
`timescale 1 ns / 1 ps

	module stream_v10_v1_0 #
	(
		// Users to add parameters here

		// User parameters ends
		// Do not modify the parameters beyond this line


		// Parameters of Axi Slave Bus Interface S00_AXI
		parameter integer C_S00_AXI_DATA_WIDTH	= 32,
		parameter integer C_S00_AXI_ADDR_WIDTH	= 8,

		// Parameters of Axi Master Bus Interface M00_AXIS
		parameter integer C_M00_AXIS_TDATA_WIDTH	= 64,
		parameter integer C_M00_AXIS_START_COUNT	= 32
	)
	(
		// Users to add ports here 
		//input wire enable_external,
		input wire  [40:0] eqed_re_0 ,
		input wire  [40:0] eqed_re_1 ,
		input wire  [40:0] eqed_re_2 ,
		input wire  [40:0] eqed_re_3 ,
		input wire  [40:0] eqed_re_4 ,
		input wire  [40:0] eqed_re_5 ,
		input wire  [40:0] eqed_re_6 ,
		input wire  [40:0] eqed_re_7 ,
		input wire  [40:0] eqed_re_8 ,
		input wire  [40:0] eqed_re_9 ,
		input wire  [40:0] eqed_re_10 ,
		input wire  [40:0] eqed_re_11 ,
		input wire  [40:0] eqed_re_12 ,
		input wire  [40:0] eqed_re_13 ,
		input wire  [40:0] eqed_re_14 ,
		input wire  [40:0] eqed_re_15 ,
		                
		input wire  [40:0] eqed_im_0 , 
		input wire  [40:0] eqed_im_1 , 
		input wire  [40:0] eqed_im_2 , 
		input wire  [40:0] eqed_im_3 , 
		input wire  [40:0] eqed_im_4 , 
		input wire  [40:0] eqed_im_5 , 
		input wire  [40:0] eqed_im_6 , 
		input wire  [40:0] eqed_im_7 , 
		input wire  [40:0] eqed_im_8 , 
		input wire  [40:0] eqed_im_9 , 
		input wire  [40:0] eqed_im_10 ,
		input wire  [40:0] eqed_im_11 ,
		input wire  [40:0] eqed_im_12 ,
		input wire  [40:0] eqed_im_13 ,
		input wire  [40:0] eqed_im_14 ,
		input wire  [40:0] eqed_im_15 ,
	
        input wire [15:0] eqed_valid,
		// User ports ends
		// Do not modify the ports beyond this line


		// Ports of Axi Slave Bus Interface S00_AXI
		input wire  s00_axi_aclk,
		input wire  s00_axi_aresetn,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_awaddr,
		input wire [2 : 0] s00_axi_awprot,
		input wire  s00_axi_awvalid,
		output wire  s00_axi_awready,
		input wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_wdata,
		input wire [(C_S00_AXI_DATA_WIDTH/8)-1 : 0] s00_axi_wstrb,
		input wire  s00_axi_wvalid,
		output wire  s00_axi_wready,
		output wire [1 : 0] s00_axi_bresp,
		output wire  s00_axi_bvalid,
		input wire  s00_axi_bready,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_araddr,
		input wire [2 : 0] s00_axi_arprot,
		input wire  s00_axi_arvalid,
		output wire  s00_axi_arready,
		output wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_rdata,
		output wire [1 : 0] s00_axi_rresp,
		output wire  s00_axi_rvalid,
		input wire  s00_axi_rready,

		// Ports of Axi Master Bus Interface M00_AXIS
		input wire  m00_axis_aclk,
		input wire  m00_axis_aresetn,
		output wire  m00_axis_tvalid,
		output wire [C_M00_AXIS_TDATA_WIDTH-1 : 0] m00_axis_tdata,
		output wire [(C_M00_AXIS_TDATA_WIDTH/8)-1 : 0] m00_axis_tstrb,
		output wire  m00_axis_tlast,
		input wire  m00_axis_tready
	);
	
	wire enable_generator;
	wire reset_buffer;
	wire data_valid;
	wire [31:0] buffer_width;

	
// Instantiation of Axi Bus Interface S00_AXI
	stream_v10_v1_0_S00_AXI # ( 
		.C_S_AXI_DATA_WIDTH(C_S00_AXI_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_S00_AXI_ADDR_WIDTH)
	) stream_v10_v1_0_S00_AXI_inst (
	    .buffer_width(buffer_width),
	    .enable_generator(enable_generator),
	    .reset_buffer(reset_buffer), 
		.S_AXI_ACLK(s00_axi_aclk),
		.S_AXI_ARESETN(s00_axi_aresetn),
		.S_AXI_AWADDR(s00_axi_awaddr),
		.S_AXI_AWPROT(s00_axi_awprot),
		.S_AXI_AWVALID(s00_axi_awvalid),
		.S_AXI_AWREADY(s00_axi_awready),
		.S_AXI_WDATA(s00_axi_wdata),
		.S_AXI_WSTRB(s00_axi_wstrb),
		.S_AXI_WVALID(s00_axi_wvalid),
		.S_AXI_WREADY(s00_axi_wready),
		.S_AXI_BRESP(s00_axi_bresp),
		.S_AXI_BVALID(s00_axi_bvalid),
		.S_AXI_BREADY(s00_axi_bready),
		.S_AXI_ARADDR(s00_axi_araddr),
		.S_AXI_ARPROT(s00_axi_arprot),
		.S_AXI_ARVALID(s00_axi_arvalid),
		.S_AXI_ARREADY(s00_axi_arready),
		.S_AXI_RDATA(s00_axi_rdata),
		.S_AXI_RRESP(s00_axi_rresp),
		.S_AXI_RVALID(s00_axi_rvalid),
		.S_AXI_RREADY(s00_axi_rready)
	);
/*
input wire [40:0] eqed_re [0:15],      
      input wire [40:0] eqed_im [0:15],
      input wire [15:0] eqed_valid,    
*/
/**/
// Instantiation of Axi Bus Interface M00_AXIS
	stream_v10_v1_0_M00_AXIS # ( 
		.C_M_AXIS_TDATA_WIDTH(C_M00_AXIS_TDATA_WIDTH),
		.C_M_START_COUNT(C_M00_AXIS_START_COUNT)
	) stream_v10_v1_0_M00_AXIS_inst (
	    .enable_generator(enable_generator),   
        .reset_buffer(reset_buffer),   
        .buffer_width(buffer_width),        
        .eqed_re_0(eqed_re_0),
        .eqed_re_1(eqed_re_1),
        .eqed_re_2(eqed_re_2),
        .eqed_re_3(eqed_re_3),
        .eqed_re_4(eqed_re_4),
        .eqed_re_5(eqed_re_5),
        .eqed_re_6(eqed_re_6),
        .eqed_re_7(eqed_re_7),
        .eqed_re_8(eqed_re_8),
        .eqed_re_9(eqed_re_9),
        .eqed_re_10(eqed_re_10),
        .eqed_re_11(eqed_re_11),
        .eqed_re_12(eqed_re_12),
        .eqed_re_13(eqed_re_13),
        .eqed_re_14(eqed_re_14),
        .eqed_re_15(eqed_re_15),     
        .eqed_im_0(eqed_im_0),
        .eqed_im_1(eqed_im_1),
        .eqed_im_2(eqed_im_2),
        .eqed_im_3(eqed_im_3),
        .eqed_im_4(eqed_im_4),
        .eqed_im_5(eqed_im_5),
        .eqed_im_6(eqed_im_6),
        .eqed_im_7(eqed_im_7),
        .eqed_im_8(eqed_im_8),
        .eqed_im_9(eqed_im_9),
        .eqed_im_10(eqed_im_10),
        .eqed_im_11(eqed_im_11),
        .eqed_im_12(eqed_im_12),
        .eqed_im_13(eqed_im_13),
        .eqed_im_14(eqed_im_14),
        .eqed_im_15(eqed_im_15),
        .eqed_valid(eqed_valid),  
		.M_AXIS_ACLK(m00_axis_aclk),
		.M_AXIS_ARESETN(m00_axis_aresetn),
		.M_AXIS_TVALID(m00_axis_tvalid),
		.M_AXIS_TDATA(m00_axis_tdata),
		.M_AXIS_TSTRB(m00_axis_tstrb),
		.M_AXIS_TLAST(m00_axis_tlast),
		.M_AXIS_TREADY(m00_axis_tready)
	);

	// Add user logic here

	// User logic ends

	endmodule
