
`timescale 1 ns / 1 ps

	module adc #
	(
		// Users to add parameters here
		parameter integer ADC7656_DATA_WIDTH	    = 16,
		// User parameters ends
		// Do not modify the parameters beyond this line


		// Parameters of Axi Slave Bus Interface S00_AXI
		parameter integer C_S00_AXI_DATA_WIDTH	= 32,
		parameter integer C_S00_AXI_ADDR_WIDTH	= 7
	)
	(
		// Users to add ports here
		input     wire    [ADC7656_DATA_WIDTH-1 : 0]  adc_data,
		input     wire    busy,	
		input     wire    trigger,			
		output    wire    [2:0]   convst,
		output    wire    cs,
		output    wire    rd,
		output    wire    [ADC7656_DATA_WIDTH-1 : 0]  fifo_wdata_0,
		output    wire    [ADC7656_DATA_WIDTH-1 : 0]  fifo_wdata_1,		
		output    wire    [ADC7656_DATA_WIDTH-1 : 0]  fifo_wdata_2,	
		output    wire    [ADC7656_DATA_WIDTH-1 : 0]  fifo_wdata_3,		
		output    wire    [ADC7656_DATA_WIDTH-1 : 0]  fifo_wdata_4,	
		output    wire		wr_en_0,
		output    wire		wr_en_1,	
		output    wire		wr_en_2,	
		output    wire		wr_en_3,	
		output    wire		wr_en_4,
        output    wire     adc_reset,	
        output    wire     internal_trigger,
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
		input wire  s00_axi_rready
	);
// Instantiation of Axi Bus Interface S00_AXI
	adc_S00_AXI # ( 
		.C_S_AXI_DATA_WIDTH(C_S00_AXI_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_S00_AXI_ADDR_WIDTH)
	) adc_S00_AXI_inst (
	    .ADC_DATA(adc_data),
	    .BUSY(busy),
	    .TRIGGER(trigger),
	    .CONVST(convst),
	    .CS(cs),
	    .RD(rd),
	    .FIFO_WDATA_0(fifo_wdata_0),
	    .FIFO_WDATA_1(fifo_wdata_1),
	    .FIFO_WDATA_2(fifo_wdata_2),
	    .FIFO_WDATA_3(fifo_wdata_3),
	    .FIFO_WDATA_4(fifo_wdata_4),
	    .WR_EN_0(wr_en_0),
	    .WR_EN_1(wr_en_1),
	    .WR_EN_2(wr_en_2),
	    .WR_EN_3(wr_en_3),
	    .WR_EN_4(wr_en_4),
	    .ADC_RESET(adc_reset),
	    .INTERNAL_TRIGGER(internal_trigger),		
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

	// Add user logic here

	// User logic ends

	endmodule
