-- (c) Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
-- (c) Copyright 2022-2024 Advanced Micro Devices, Inc. All rights reserved.
-- 
-- This file contains confidential and proprietary information
-- of AMD and is protected under U.S. and international copyright
-- and other intellectual property laws.
-- 
-- DISCLAIMER
-- This disclaimer is not a license and does not grant any
-- rights to the materials distributed herewith. Except as
-- otherwise provided in a valid license issued to you by
-- AMD, and to the maximum extent permitted by applicable
-- law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
-- WITH ALL FAULTS, AND AMD HEREBY DISCLAIMS ALL WARRANTIES
-- AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
-- BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
-- INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
-- (2) AMD shall not be liable (whether in contract or tort,
-- including negligence, or under any other theory of
-- liability) for any loss or damage of any kind or nature
-- related to, arising under or in connection with these
-- materials, including for any direct, or any indirect,
-- special, incidental, or consequential loss or damage
-- (including loss of data, profits, goodwill, or any type of
-- loss or damage suffered as a result of any action brought
-- by a third party) even if such damage or loss was
-- reasonably foreseeable or AMD had been advised of the
-- possibility of the same.
-- 
-- CRITICAL APPLICATIONS
-- AMD products are not designed or intended to be fail-
-- safe, or for use in any application requiring fail-safe
-- performance, such as life-support or safety devices or
-- systems, Class III medical devices, nuclear facilities,
-- applications related to the deployment of airbags, or any
-- other applications that could lead to death, personal
-- injury, or severe property or environmental damage
-- (individually and collectively, "Critical
-- Applications"). Customer assumes the sole risk and
-- liability of any use of AMD products in Critical
-- Applications, subject only to applicable laws and
-- regulations governing limitations on product liability.
-- 
-- THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
-- PART OF THIS FILE AT ALL TIMES.
-- 
-- DO NOT MODIFY THIS FILE.

-- IP VLNV: xilinx.com:ip:system_ila:1.1
-- IP Revision: 15

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

ENTITY Block_design_system_ila_0_0 IS
  PORT (
    clk : IN STD_LOGIC;
    SLOT_0_AXIS_tdata : IN STD_LOGIC_VECTOR(63 DOWNTO 0);
    SLOT_0_AXIS_tkeep : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
    SLOT_0_AXIS_tlast : IN STD_LOGIC;
    SLOT_0_AXIS_tvalid : IN STD_LOGIC;
    SLOT_0_AXIS_tready : IN STD_LOGIC;
    SLOT_1_AXIS_tdata : IN STD_LOGIC_VECTOR(63 DOWNTO 0);
    SLOT_1_AXIS_tlast : IN STD_LOGIC;
    SLOT_1_AXIS_tvalid : IN STD_LOGIC;
    SLOT_1_AXIS_tready : IN STD_LOGIC;
    resetn : IN STD_LOGIC
  );
END Block_design_system_ila_0_0;

ARCHITECTURE Block_design_system_ila_0_0_arch OF Block_design_system_ila_0_0 IS
  ATTRIBUTE DowngradeIPIdentifiedWarnings : STRING;
  ATTRIBUTE DowngradeIPIdentifiedWarnings OF Block_design_system_ila_0_0_arch: ARCHITECTURE IS "yes";
  COMPONENT bd_25ec IS
    PORT (
      clk : IN STD_LOGIC;
      SLOT_0_AXIS_tdata : IN STD_LOGIC_VECTOR(63 DOWNTO 0);
      SLOT_0_AXIS_tkeep : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
      SLOT_0_AXIS_tlast : IN STD_LOGIC;
      SLOT_0_AXIS_tvalid : IN STD_LOGIC;
      SLOT_0_AXIS_tready : IN STD_LOGIC;
      SLOT_1_AXIS_tdata : IN STD_LOGIC_VECTOR(63 DOWNTO 0);
      SLOT_1_AXIS_tlast : IN STD_LOGIC;
      SLOT_1_AXIS_tvalid : IN STD_LOGIC;
      SLOT_1_AXIS_tready : IN STD_LOGIC;
      resetn : IN STD_LOGIC
    );
  END COMPONENT bd_25ec;
  ATTRIBUTE X_INTERFACE_INFO : STRING;
  ATTRIBUTE X_INTERFACE_PARAMETER : STRING;
  ATTRIBUTE X_INTERFACE_PARAMETER OF SLOT_0_AXIS_tdata: SIGNAL IS "XIL_INTERFACENAME SLOT_0_AXIS, TDATA_NUM_BYTES 8, TDEST_WIDTH 0, TID_WIDTH 0, TUSER_WIDTH 0, HAS_TREADY 1, HAS_TSTRB 0, HAS_TKEEP 1, HAS_TLAST 1, FREQ_HZ 50000000, PHASE 0.0, CLK_DOMAIN Block_design_processing_system7_0_1_FCLK_CLK0, LAYERED_METADATA undef, INSERT_VIP 0";
  ATTRIBUTE X_INTERFACE_INFO OF SLOT_0_AXIS_tdata: SIGNAL IS "xilinx.com:interface:axis:1.0 SLOT_0_AXIS TDATA";
  ATTRIBUTE X_INTERFACE_INFO OF SLOT_0_AXIS_tkeep: SIGNAL IS "xilinx.com:interface:axis:1.0 SLOT_0_AXIS TKEEP";
  ATTRIBUTE X_INTERFACE_INFO OF SLOT_0_AXIS_tlast: SIGNAL IS "xilinx.com:interface:axis:1.0 SLOT_0_AXIS TLAST";
  ATTRIBUTE X_INTERFACE_INFO OF SLOT_0_AXIS_tready: SIGNAL IS "xilinx.com:interface:axis:1.0 SLOT_0_AXIS TREADY";
  ATTRIBUTE X_INTERFACE_INFO OF SLOT_0_AXIS_tvalid: SIGNAL IS "xilinx.com:interface:axis:1.0 SLOT_0_AXIS TVALID";
  ATTRIBUTE X_INTERFACE_PARAMETER OF SLOT_1_AXIS_tdata: SIGNAL IS "XIL_INTERFACENAME SLOT_1_AXIS, TDATA_NUM_BYTES 8, TDEST_WIDTH 0, TID_WIDTH 0, TUSER_WIDTH 0, HAS_TREADY 1, HAS_TSTRB 0, HAS_TKEEP 0, HAS_TLAST 1, FREQ_HZ 50000000, PHASE 0.0, CLK_DOMAIN Block_design_processing_system7_0_1_FCLK_CLK0, LAYERED_METADATA xilinx.com:interface:datatypes:1.0 {TDATA {datatype {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value {}} bitwidth {attribs {resolve_type automatic dependency {} format long minimum {} maximum {}} value 2" & 
"048} bitoffset {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 0} array_type {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value chan} size {attribs {resolve_type generated dependency chan_size format long minimum {} maximum {}} value 1} stride {attribs {resolve_type generated dependency chan_stride format long minimum {} maximum {}} value 64} datatype {name {attribs {resolve_type immediate dependency {} format s" & 
"tring minimum {} maximum {}} value {}} bitwidth {attribs {resolve_type automatic dependency {} format long minimum {} maximum {}} value 2048} bitoffset {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 0} struct {field_xn_re {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value xn_re} enabled {attribs {resolve_type immediate dependency {} format bool minimum {} maximum {}} value true} datatype {name {attribs {resolve" & 
"_type immediate dependency {} format string minimum {} maximum {}} value {}} bitwidth {attribs {resolve_type automatic dependency {} format long minimum {} maximum {}} value 2016} bitoffset {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 0} array_type {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value {}} size {attribs {resolve_type generated dependency frame_size format long minimum {} maximum {}} value 32} str" & 
"ide {attribs {resolve_type generated dependency frame_stride format long minimum {} maximum {}} value 64} datatype {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value {}} bitwidth {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 32} bitoffset {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 0} real {float {sigwidth {attribs {resolve_type immediate dependency {} format long mi" & 
"nimum {} maximum {}} value 24}}}}}}} field_xn_im {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value xn_im} enabled {attribs {resolve_type immediate dependency {} format bool minimum {} maximum {}} value true} datatype {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value {}} bitwidth {attribs {resolve_type automatic dependency {} format long minimum {} maximum {}} value 2016} bitoffset {attribs {resolve_type g" & 
"enerated dependency xn_im_offset format long minimum {} maximum {}} value 32} array_type {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value {}} size {attribs {resolve_type generated dependency frame_size format long minimum {} maximum {}} value 32} stride {attribs {resolve_type generated dependency frame_stride format long minimum {} maximum {}} value 64} datatype {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}" & 
"} value {}} bitwidth {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 32} bitoffset {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 0} real {float {sigwidth {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 24}}}}}}}}}}}} TDATA_WIDTH 64 TUSER {datatype {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value {}} bitwidth {attribs {resolve_typ" & 
"e automatic dependency {} format long minimum {} maximum {}} value 0} bitoffset {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 0} struct {field_xk_index {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value xk_index} enabled {attribs {resolve_type generated dependency xk_index_enabled format bool minimum {} maximum {}} value false} datatype {name {attribs {resolve_type immediate dependency {} format string minimum" & 
" {} maximum {}} value {}} bitwidth {attribs {resolve_type generated dependency xk_index_width format long minimum {} maximum {}} value 0} bitoffset {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 0} integer {signed {attribs {resolve_type immediate dependency {} format bool minimum {} maximum {}} value false}}}} field_blk_exp {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value blk_exp} enabled {attribs {resolve_ty" & 
"pe generated dependency blk_exp_enabled format bool minimum {} maximum {}} value false} datatype {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value {}} bitwidth {attribs {resolve_type automatic dependency {} format long minimum {} maximum {}} value 5} bitoffset {attribs {resolve_type generated dependency blk_exp_offset format long minimum {} maximum {}} value 0} array_type {name {attribs {resolve_type immediate dependency {} format string minimum {} m" & 
"aximum {}} value {}} size {attribs {resolve_type generated dependency chan_size format long minimum {} maximum {}} value 1} stride {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 8} datatype {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value {}} bitwidth {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 5} bitoffset {attribs {resolve_type immediate dependency {} format long " & 
"minimum {} maximum {}} value 0} integer {signed {attribs {resolve_type immediate dependency {} format bool minimum {} maximum {}} value false}}}}}} field_ovflo {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value ovflo} enabled {attribs {resolve_type generated dependency ovflo_enabled format bool minimum {} maximum {}} value false} datatype {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value {}} bitwidth {attr" & 
"ibs {resolve_type automatic dependency {} format long minimum {} maximum {}} value 1} bitoffset {attribs {resolve_type generated dependency ovflo_offset format long minimum {} maximum {}} value 0} array_type {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value {}} size {attribs {resolve_type generated dependency chan_size format long minimum {} maximum {}} value 1} stride {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} " & 
"value 1} datatype {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value {}} bitwidth {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 1} bitoffset {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 0} integer {signed {attribs {resolve_type immediate dependency {} format bool minimum {} maximum {}} value false}}}}}}}}} TUSER_WIDTH 0}, INSERT_VIP 0";
  ATTRIBUTE X_INTERFACE_INFO OF SLOT_1_AXIS_tdata: SIGNAL IS "xilinx.com:interface:axis:1.0 SLOT_1_AXIS TDATA";
  ATTRIBUTE X_INTERFACE_INFO OF SLOT_1_AXIS_tlast: SIGNAL IS "xilinx.com:interface:axis:1.0 SLOT_1_AXIS TLAST";
  ATTRIBUTE X_INTERFACE_INFO OF SLOT_1_AXIS_tready: SIGNAL IS "xilinx.com:interface:axis:1.0 SLOT_1_AXIS TREADY";
  ATTRIBUTE X_INTERFACE_INFO OF SLOT_1_AXIS_tvalid: SIGNAL IS "xilinx.com:interface:axis:1.0 SLOT_1_AXIS TVALID";
  ATTRIBUTE X_INTERFACE_PARAMETER OF clk: SIGNAL IS "XIL_INTERFACENAME CLK.clk, FREQ_HZ 50000000, FREQ_TOLERANCE_HZ 0, PHASE 0.0, CLK_DOMAIN Block_design_processing_system7_0_1_FCLK_CLK0, ASSOCIATED_BUSIF SLOT_0_AXIS:SLOT_1_AXIS, ASSOCIATED_RESET resetn, INSERT_VIP 0";
  ATTRIBUTE X_INTERFACE_INFO OF clk: SIGNAL IS "xilinx.com:signal:clock:1.0 CLK.clk CLK";
  ATTRIBUTE X_INTERFACE_PARAMETER OF resetn: SIGNAL IS "XIL_INTERFACENAME RST.resetn, POLARITY ACTIVE_LOW, INSERT_VIP 0";
  ATTRIBUTE X_INTERFACE_INFO OF resetn: SIGNAL IS "xilinx.com:signal:reset:1.0 RST.resetn RST";
BEGIN
  U0 : bd_25ec
    PORT MAP (
      clk => clk,
      SLOT_0_AXIS_tdata => SLOT_0_AXIS_tdata,
      SLOT_0_AXIS_tkeep => SLOT_0_AXIS_tkeep,
      SLOT_0_AXIS_tlast => SLOT_0_AXIS_tlast,
      SLOT_0_AXIS_tvalid => SLOT_0_AXIS_tvalid,
      SLOT_0_AXIS_tready => SLOT_0_AXIS_tready,
      SLOT_1_AXIS_tdata => SLOT_1_AXIS_tdata,
      SLOT_1_AXIS_tlast => SLOT_1_AXIS_tlast,
      SLOT_1_AXIS_tvalid => SLOT_1_AXIS_tvalid,
      SLOT_1_AXIS_tready => SLOT_1_AXIS_tready,
      resetn => resetn
    );
END Block_design_system_ila_0_0_arch;
