
/*****************************************************************************/
/**
 *
 * @file DMA-FFT.c
 *
 * This File demonstrates the DMA IP-Core in conjunction with a FFT IP-Core. 
 * The DMA passes a memory Buffer onto the FFT core and recollects it once the FFT has been computed.
 * Transform length of FFT in Vivado: 64
 * This Project relies on examples provided by AMD (namely xaxidma_example_simple_intr AND xilffs_polled_example) [You can access these in the Board support Package of the Platform]
 
 *
 * ***************************************************************************
 */

/***************************** Include Files *********************************/


#include "xaxidma.h"
#include "xparameters.h"
#include "xplatform_info.h"
#include "xil_exception.h"
#include "xdebug.h"
#include "xil_util.h"
#include <complex.h>
#include <math.h>   
#include <stdio.h>
#include <xil_cache.h>
#include <xil_types.h>
#include "xsdps.h"		/* SD device driver */
#include "ff.h"         //throws error that certain flags should be set, works regardless though

#ifdef SDT
#include "xinterrupt_wrap.h"
#endif
#ifdef XPAR_UARTNS550_0_BASEADDR
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif
#endif

/************************** Constant Definitions *****************************/

/*
 * Device hardware build related constants.
 */

#ifndef SDT
#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID

#ifdef XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR		XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif defined (XPAR_MIG7SERIES_0_BASEADDR)
#define DDR_BASE_ADDR	XPAR_MIG7SERIES_0_BASEADDR
#elif defined (XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR)
#define DDR_BASE_ADDR	XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR
#elif defined (XPAR_PSU_DDR_0_S_AXI_BASEADDR)
#define DDR_BASE_ADDR	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#endif

#else

#ifdef XPAR_MEM0_BASEADDRESS
#define DDR_BASE_ADDR		XPAR_MEM0_BASEADDRESS
#endif

#endif

#ifndef DDR_BASE_ADDR
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
DEFAULT SET TO 0x01000000
#define MEM_BASE_ADDR		0x01000000
#else
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x1000000)
#endif

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#define RX_INTR_ID		XPAR_INTC_0_AXIDMA_0_S2MM_INTROUT_VEC_ID
#define TX_INTR_ID		XPAR_INTC_0_AXIDMA_0_MM2S_INTROUT_VEC_ID
#else
#define RX_INTR_ID		XPAR_FABRIC_AXIDMA_0_S2MM_INTROUT_VEC_ID
#define TX_INTR_ID		XPAR_FABRIC_AXIDMA_0_MM2S_INTROUT_VEC_ID
#endif
#endif

#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#else
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC		XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#endif
#endif

/* Timeout loop counter for reset
 */
#define RESET_TIMEOUT_COUNTER	10000


/*
 * Buffer and Buffer Descriptor related constant definition
 */
#define MAX_PKT_LEN		0x40        // corresponds to FFT Transform length; max right now: 64 , to increase this increase the transform length in the Vivado FFT IP Core

#define NUMBER_OF_TRANSFERS	1
#define POLL_TIMEOUT_COUNTER    1000000U
#define NUMBER_OF_EVENTS	1

/* The interrupt coalescing threshold and delay timer threshold
 * Valid range is 1 to 255
 *
 * We set the coalescing threshold to be the total number of packets.
 * The receive side will only get one completion interrupt for this example.
 */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#ifndef DEBUG
extern void xil_printf(const char *format, ...);
#endif

#ifdef XPAR_UARTNS550_0_BASEADDR
static void Uart550_Setup(void);
#endif

static int CheckData(int Length);
static void TxIntrHandler(void *Callback);
static void RxIntrHandler(void *Callback);
void print_complex(const char* note, float complex z);

int FfsReadPolled (float complex* data_buffer, u32 bytes_to_read, char* FileName);
int FfsWritePolled (float complex* data_buffer, u32 bytes_to_write, char* FileName);
#ifndef SDT
static int SetupIntrSystem(INTC *IntcInstancePtr,
			   XAxiDma *AxiDmaPtr, u16 TxIntrId, u16 RxIntrId);
static void DisableIntrSystem(INTC *IntcInstancePtr,
			      u16 TxIntrId, u16 RxIntrId);

#endif

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */


static XAxiDma AxiDma;		/* Instance of the XAxiDma */
#ifndef SDT
static INTC Intc;	/* Instance of the Interrupt Controller */
#endif
static FIL fil;		/* File object */
static FATFS fatfs;
static char WriteFileName[32] = "FFT.bin";
static char ReadFileName[32] = "SIGNAL.BIN";
static char *SD_File;



//for SD-Card volumes
MKFS_PARM mkfs_parm;
/*
 * Flags interrupt handlers use to notify the application context the events.
 */
volatile u32 TxDone;
volatile u32 RxDone;
volatile u32 Error;

/*****************************************************************************/
/**
*
* Main function
*
* This function is the main entry of the interrupt test. It does the following:
*	Set up the output terminal if UART16550 is in the hardware build
*	Initialize the DMA engine
*	Set up Tx and Rx channels
*	Set up the interrupt system for the Tx and Rx interrupts
*	Submit a transfer
*	Wait for the transfer to finish
*	Check transfer status
*	Disable Tx and Rx interrupts
*	Print test status and exit
*
* @param	None
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if example fails.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;
	XAxiDma_Config *Config;
	int Tries = NUMBER_OF_TRANSFERS;
	int Index;
	float complex *TxBufferPtr;
	float complex *RxBufferPtr;
    

	TxBufferPtr = (float complex *)TX_BUFFER_BASE ;
	RxBufferPtr = (float complex *)RX_BUFFER_BASE;
	/* Initial setup for Uart16550 */
#ifdef XPAR_UARTNS550_0_BASEADDR

	Uart550_Setup();

#endif
    printf("\e[1;1H\e[2J"); // clearing out console can be removed if it causes issues
	xil_printf("\r\n--- Entering main() --- \r\n");

//look up DMA hardware configuration
#ifndef SDT
	Config = XAxiDma_LookupConfig(DMA_DEV_ID);
	if (!Config) {
		xil_printf("No config found for %d\r\n", DMA_DEV_ID);

		return XST_FAILURE;
	}
#else
	Config = XAxiDma_LookupConfig(XPAR_XAXIDMA_0_BASEADDR);
	if (!Config) {
		xil_printf("No config found for %d\r\n", XPAR_XAXIDMA_0_BASEADDR);

		return XST_FAILURE;
	}
#endif

	/* Initialize DMA engine */
	Status = XAxiDma_CfgInitialize(&AxiDma, Config);

	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}
    /* DMA should not be configured in SG mode (unceck in vivado)*/
	if (XAxiDma_HasSg(&AxiDma)) {
		xil_printf("Device configured as SG mode \r\n");
		return XST_FAILURE;
	}

	/* Set up Interrupt system  */
#ifndef SDT
	Status = SetupIntrSystem(&Intc, &AxiDma, TX_INTR_ID, RX_INTR_ID);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed intr setup\r\n");
		return XST_FAILURE;
	}
#else
	Status = XSetupInterruptSystem(&AxiDma, &TxIntrHandler,
				       Config->IntrId[0], Config->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSetupInterruptSystem(&AxiDma, &RxIntrHandler,
				       Config->IntrId[1], Config->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/* Disable all interrupts before setup */

	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
			    XAXIDMA_DMA_TO_DEVICE);

	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
			    XAXIDMA_DEVICE_TO_DMA);

	/* Enable all interrupts */
	XAxiDma_IntrEnable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
			   XAXIDMA_DMA_TO_DEVICE);


	XAxiDma_IntrEnable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
			   XAXIDMA_DEVICE_TO_DMA);

	/* Initialize flags before start transfer test  */
	TxDone = 0;
	RxDone = 0;
	Error = 0;

	
    
    /* Read signal data from SD-card and pass it into TxBuffer for later use in DMA */
    /* alternatively manually fill the buffer with some loop or through some header file */
    Status = FfsReadPolled (TxBufferPtr, MAX_PKT_LEN*sizeof(float complex), ReadFileName); 
    
    
	/* Flush the buffers before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
    //FfsWritePolled(TxBufferPtr, MAX_PKT_LEN*sizeof(float complex), ReadFileName); 
	Xil_DCacheFlushRange((UINTPTR)TxBufferPtr, MAX_PKT_LEN*sizeof(float complex));
	Xil_DCacheFlushRange((UINTPTR)RxBufferPtr, MAX_PKT_LEN*sizeof(float complex));

	/* Send a packet */
	for (Index = 0; Index < Tries; Index ++) {
        //actual dma transfer function with pointer to AxiDma Instance the Receiver Buffer in case of device to dma and the number of bytes to write into the buffer
		//Device to DMA aka zynq mapped memory buffer to dma axi stream
        Status = XAxiDma_SimpleTransfer(&AxiDma, (UINTPTR) RxBufferPtr, 
						MAX_PKT_LEN*sizeof(float complex), XAXIDMA_DEVICE_TO_DMA);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
        //DMA to Zynq (stream to mapped memory)
		Status = XAxiDma_SimpleTransfer(&AxiDma, (UINTPTR) TxBufferPtr,
						MAX_PKT_LEN*sizeof(float complex), XAXIDMA_DMA_TO_DEVICE);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = Xil_WaitForEventSet(POLL_TIMEOUT_COUNTER, NUMBER_OF_EVENTS, &Error);
		if (Status == XST_SUCCESS) {
			if (!TxDone) {
				xil_printf("Transmit error %d\r\n", Status);
				goto Done;
			} else if (Status == XST_SUCCESS && !RxDone) {
				xil_printf("Receive error %d\r\n", Status);
				goto Done;
			}
		}

		/*
		 * Wait for TX done or timeout
		 */
		Status = Xil_WaitForEventSet(POLL_TIMEOUT_COUNTER, NUMBER_OF_EVENTS, &TxDone);
		if (Status != XST_SUCCESS) {
			xil_printf("Transmit failed %d\r\n", Status);
			goto Done;

		}
        
		/*
		 * Wait for RX done or timeout
		 */
		Status = Xil_WaitForEventSet(POLL_TIMEOUT_COUNTER, NUMBER_OF_EVENTS, &RxDone);
		if (Status != XST_SUCCESS) {
			xil_printf("Receive failed %d\r\n", Status);
			goto Done;
		}

		/*
		 * Test finished, check data; originally from dma loop back where the dma would send data to a FIFO and receive it back. Here this only prints the FFT data
		 */
		Status = CheckData(MAX_PKT_LEN);
		if (Status != XST_SUCCESS) {
			xil_printf("Data check failed\r\n");
			goto Done;
		}
	}


	xil_printf("Successfully ran AXI DMA interrupt Data Exchange\r\n");
    //Write FFT Data to SD card
    FfsWritePolled(RxBufferPtr, MAX_PKT_LEN*sizeof(float complex), WriteFileName);
    
    // might want to unmount sd card use : f_mount(0, Path, 1)

	/* Disable TX and RX Ring interrupts and return success */
#ifndef SDT
	DisableIntrSystem(&Intc, TX_INTR_ID, RX_INTR_ID);
#else
	XDisconnectInterruptCntrl(Config->IntrId[0], Config->IntrParent);
	XDisconnectInterruptCntrl(Config->IntrId[1], Config->IntrParent);
#endif

Done:
	xil_printf("--- Exiting main() --- \r\n");

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

#ifdef XPAR_UARTNS550_0_BASEADDR
/*****************************************************************************/
/*
*
* Uart16550 setup routine, need to set baudrate to 9600 and data bits to 8
*
* @param	None
*
* @return	None
*
* @note		None.
*
******************************************************************************/
static void Uart550_Setup(void)
{

	XUartNs550_SetBaud(XPAR_UARTNS550_0_BASEADDR,
			   XPAR_XUARTNS550_CLOCK_HZ, 9600);

	XUartNs550_SetLineControlReg(XPAR_UARTNS550_0_BASEADDR,
				     XUN_LCR_8_DATA_BITS);
}
#endif

/*****************************************************************************/
/*
*
* This function checks data buffer after the DMA transfer is finished.
*
* We use the static tx/rx buffers.
*
* @param	Length is the length to check
* @param	StartValue is the starting value of the first byte
*
* @return
*		- XST_SUCCESS if validation is successful
*		- XST_FAILURE if validation is failure.
*
* @note		None.
*
******************************************************************************/
static int CheckData(int Length)
{
    /*There is a Option for the FFT output to be in bitreversed order (in Vivado), this however did not work in this particular case.
      A forum answer by AMD staff suggested remaking the project and initially leaving the output in natural order. Might want to try that.
    */
    int bit_reversal_index[64] = {0, 32, 16, 48, 8, 40, 24, 56, 4, 36, 20, 52, 12, 44, 28, 60, 2, 34, 18, 50, 10, 42, 26, 58, 6, 38, 22, 54, 14, 46, 30, 62, 1, 33, 17, 49, 9, 41, 25, 57, 5, 37, 21, 53, 13, 45, 29, 61, 3, 35, 19, 51, 11, 43, 27, 59, 7, 39, 23, 55, 15, 47, 31, 63};

	float complex *RxPacket;
	int Index = 0;
	
	RxPacket = (float complex *) RX_BUFFER_BASE;
	

	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
	Xil_DCacheInvalidateRange((UINTPTR)RxPacket, Length*sizeof(float complex));

	for (Index = 0; Index < Length; Index++) {
		xil_printf("FFT Data at %d: ", Index);
        print_complex("", (float complex) RxPacket[bit_reversal_index[Index]]);
			
	}

	return XST_SUCCESS;
}
int FfsReadPolled (float complex* data_buffer, u32 bytes_to_read, char* FileName){
    FRESULT Res;
	UINT NumBytesRead;
	//BYTE work[FF_MAX_SS];
	
    
    float complex read_data[bytes_to_read/sizeof(float complex)]; //bytes_to_read/sizeof(float complex)

    TCHAR *Path = "0:/";
    Res = f_mount(&fatfs, Path, 0);

    if (Res != FR_OK) {
		return XST_FAILURE;
	}
    mkfs_parm.fmt = FM_FAT32;
    // creating volumes is not necessary here and actually breaks the reading operation
    /*
    Res = f_mkfs(Path, &mkfs_parm , work, sizeof work);
	if (Res != FR_OK) {
		return XST_FAILURE;
	}
    */
    SD_File = (char *)FileName;

	Res = f_open(&fil, SD_File,  FA_READ); //FA_OPEN_EXISTING |
	if (Res) {
		return XST_FAILURE;
	}
    
	/*
	 * Pointer to beginning of file .
	 */
	Res = f_lseek(&fil, 0);
	if (Res) {
		return XST_FAILURE;
	}
    /*
	 * Read data from file.
	 */
	Res = f_read(&fil, (void*)read_data, bytes_to_read,
			&NumBytesRead);
    for(u32 i = 0; i< bytes_to_read/sizeof(float complex); i++){
        printf("Data read from SD-card at index %i: ", i);
        print_complex("", read_data[i]);
    }
    xil_printf("Number of Bytes read: %i\r\n", NumBytesRead);
	if (Res) {
		return XST_FAILURE;
	}
    //write local buffer to dma memory Tx area
    for(u32 i = 0; i< bytes_to_read/sizeof(float complex); i++){
        data_buffer[i] = read_data[i];
        
    }

    Res = f_close(&fil);
	if (Res) {
		return XST_FAILURE;
	}
    printf("Successfully Read from SD-Card\r\n");
    Xil_DCacheFlush();
	return XST_SUCCESS;

}
int FfsWritePolled (float complex* data_buffer, u32 bytes_to_write, char* FileName){
    FRESULT Res;
	UINT NumBytesWritten;
	//BYTE work[FF_MAX_SS];
	
    
    float complex write_data[bytes_to_write/sizeof(float complex)];
    //write Rx memory area to local buffer 
    for(u32 i = 0; i< bytes_to_write/sizeof(float complex); i++){
          write_data[i] = data_buffer[i];
    }

    TCHAR *Path = "0:/";
    Res = f_mount(&fatfs, Path, 0);

    if (Res != FR_OK) {
		return XST_FAILURE;
	}
    mkfs_parm.fmt = FM_FAT32;

    /*
    Res = f_mkfs(Path, &mkfs_parm , work, sizeof work);
	if (Res != FR_OK) {
		return XST_FAILURE;
	}
    */
    SD_File = (char *)FileName;

	Res = f_open(&fil, SD_File, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	if (Res) {
		return XST_FAILURE;
	}

	/*
	 * Pointer to beginning of file .
	 */
	Res = f_lseek(&fil, 0);
	if (Res) {
		return XST_FAILURE;
    }
    /*
	 * Write data to file. (const void*)SourceAddress, FileSize
	 */
	Res = f_write(&fil, (const void*)write_data, bytes_to_write,
			&NumBytesWritten);
	if (Res) {
		return XST_FAILURE;
	}
    
    /*
	 * Close file.
	 */
	Res = f_close(&fil);
	if (Res) {
		return XST_FAILURE;
	}
    //Flush Cache
    Xil_DCacheFlush();
    printf("Successfully wrote to SD-Card\r\n");
	return XST_SUCCESS;

}

void print_complex(const char* note, float complex z)
{
    printf("%s %f%+f*i\n", note, creal(z), cimag(z));
}

/*****************************************************************************/
/*
*
* This is the DMA TX Interrupt handler function.
*
* It gets the interrupt status from the hardware, acknowledges it, and if any
* error happens, it resets the hardware. Otherwise, if a completion interrupt
* is present, then sets the TxDone.flag
*
* @param	Callback is a pointer to TX channel of the DMA engine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void TxIntrHandler(void *Callback)
{

	u32 IrqStatus;
	int TimeOut;
	XAxiDma *AxiDmaInst = (XAxiDma *)Callback;

	/* Read pending interrupts */
	IrqStatus = XAxiDma_IntrGetIrq(AxiDmaInst, XAXIDMA_DMA_TO_DEVICE);

	/* Acknowledge pending interrupts */


	XAxiDma_IntrAckIrq(AxiDmaInst, IrqStatus, XAXIDMA_DMA_TO_DEVICE);

	/*
	 * If no interrupt is asserted, we do not do anything
	 */
	if (!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)) {

		return;
	}

	/*
	 * If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {

		Error = 1;

		/*
		 * Reset should never fail for transmit channel
		 */
		XAxiDma_Reset(AxiDmaInst);

		TimeOut = RESET_TIMEOUT_COUNTER;

		while (TimeOut) {
			if (XAxiDma_ResetIsDone(AxiDmaInst)) {
				break;
			}

			TimeOut -= 1;
		}

		return;
	}

	/*
	 * If Completion interrupt is asserted, then set the TxDone flag
	 */
	if ((IrqStatus & XAXIDMA_IRQ_IOC_MASK)) {

		TxDone = 1;
	}
}

/*****************************************************************************/
/*
*
* This is the DMA RX interrupt handler function
*
* It gets the interrupt status from the hardware, acknowledges it, and if any
* error happens, it resets the hardware. Otherwise, if a completion interrupt
* is present, then it sets the RxDone flag.
*
* @param	Callback is a pointer to RX channel of the DMA engine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RxIntrHandler(void *Callback)
{
	u32 IrqStatus;
	int TimeOut;
	XAxiDma *AxiDmaInst = (XAxiDma *)Callback;

	/* Read pending interrupts */
	IrqStatus = XAxiDma_IntrGetIrq(AxiDmaInst, XAXIDMA_DEVICE_TO_DMA);

	/* Acknowledge pending interrupts */
	XAxiDma_IntrAckIrq(AxiDmaInst, IrqStatus, XAXIDMA_DEVICE_TO_DMA);

	/*
	 * If no interrupt is asserted, we do not do anything
	 */
	if (!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)) {
		return;
	}

	/*
	 * If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {

		Error = 1;

		/* Reset could fail and hang
		 * NEED a way to handle this or do not call it??
		 */
		XAxiDma_Reset(AxiDmaInst);

		TimeOut = RESET_TIMEOUT_COUNTER;

		while (TimeOut) {
			if (XAxiDma_ResetIsDone(AxiDmaInst)) {
				break;
			}

			TimeOut -= 1;
		}

		return;
	}

	/*
	 * If completion interrupt is asserted, then set RxDone flag
	 */
	if ((IrqStatus & XAXIDMA_IRQ_IOC_MASK)) {

		RxDone = 1;
	}
}
#ifndef SDT
/*****************************************************************************/
/*
*
* This function setups the interrupt system so interrupts can occur for the
* DMA, it assumes INTC component exists in the hardware system.
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC.
* @param	AxiDmaPtr is a pointer to the instance of the DMA engine
* @param	TxIntrId is the TX channel Interrupt ID.
* @param	RxIntrId is the RX channel Interrupt ID.
*
* @return
*		- XST_SUCCESS if successful,
*		- XST_FAILURE.if not successful
*
* @note		None.
*
******************************************************************************/
static int SetupIntrSystem(INTC *IntcInstancePtr,
			   XAxiDma *AxiDmaPtr, u16 TxIntrId, u16 RxIntrId)
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID

	/* Initialize the interrupt controller and connect the ISRs */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed init intc\r\n");
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstancePtr, TxIntrId,
			       (XInterruptHandler) TxIntrHandler, AxiDmaPtr);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed tx connect intc\r\n");
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstancePtr, RxIntrId,
			       (XInterruptHandler) RxIntrHandler, AxiDmaPtr);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed rx connect intc\r\n");
		return XST_FAILURE;
	}

	/* Start the interrupt controller */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed to start intc\r\n");
		return XST_FAILURE;
	}

	XIntc_Enable(IntcInstancePtr, TxIntrId);
	XIntc_Enable(IntcInstancePtr, RxIntrId);

#else

	XScuGic_Config *IntcConfig;


	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	XScuGic_SetPriorityTriggerType(IntcInstancePtr, TxIntrId, 0xA0, 0x3);

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, RxIntrId, 0xA0, 0x3);
	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, TxIntrId,
				 (Xil_InterruptHandler)TxIntrHandler,
				 AxiDmaPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XScuGic_Connect(IntcInstancePtr, RxIntrId,
				 (Xil_InterruptHandler)RxIntrHandler,
				 AxiDmaPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	XScuGic_Enable(IntcInstancePtr, TxIntrId);
	XScuGic_Enable(IntcInstancePtr, RxIntrId);


#endif

	/* Enable interrupts from the hardware */

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)INTC_HANDLER,
				     (void *)IntcInstancePtr);

	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function disables the interrupts for DMA engine.
*
* @param	IntcInstancePtr is the pointer to the INTC component instance
* @param	TxIntrId is interrupt ID associated w/ DMA TX channel
* @param	RxIntrId is interrupt ID associated w/ DMA RX channel
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DisableIntrSystem(INTC *IntcInstancePtr,
			      u16 TxIntrId, u16 RxIntrId)
{
#ifdef XPAR_INTC_0_DEVICE_ID
	/* Disconnect the interrupts for the DMA TX and RX channels */
	XIntc_Disconnect(IntcInstancePtr, TxIntrId);
	XIntc_Disconnect(IntcInstancePtr, RxIntrId);
#else
	XScuGic_Disconnect(IntcInstancePtr, TxIntrId);
	XScuGic_Disconnect(IntcInstancePtr, RxIntrId);
#endif
}
#endif
