
`timescale 1 ns / 1 ps

	module adc_S00_AXI #
	(
		// Users to add parameters here
        parameter	    STATE_START		=	4'd0,
                        STATE_ADC_START =   4'd1,
                        STATE_CONVSET_DELAY =   4'd2,
                        STATE_DATA_OBTIAN   =   4'd3,                                                                                              
                        STATE_ADC_END 	= 	4'd4,
                        TRIGGER_STATE_IDLE = 4'd10,
                        TRIGGER_STATE_READY = 4'd11,
                        TRIGGER_STATE_EN_DETECT = 4'd12,
                        TRIGGER_INTERVAL = 100000,
                        TRIGGER_INTERVAL_HALF = 50000,
                        VER             =   17,   
		parameter integer ADC7656_DATA_WIDTH	    = 16,   
		// User parameters ends
		// Do not modify the parameters beyond this line

		// Width of S_AXI data bus
		parameter integer C_S_AXI_DATA_WIDTH	= 32,
		// Width of S_AXI address bus
		parameter integer C_S_AXI_ADDR_WIDTH	= 7
	)
	(
		// Users to add ports here
		input     wire    [ADC7656_DATA_WIDTH-1 : 0]  ADC_DATA,
		input     wire    BUSY,	
		input     wire	   TRIGGER,		
		output    wire    [2:0]   CONVST,
		output    wire    CS,
		output    wire    RD,
		output    wire    [ADC7656_DATA_WIDTH-1 : 0]  FIFO_WDATA_0,
		output    wire    [ADC7656_DATA_WIDTH-1 : 0]  FIFO_WDATA_1,		
		output    wire    [ADC7656_DATA_WIDTH-1 : 0]  FIFO_WDATA_2,	
		output    wire    [ADC7656_DATA_WIDTH-1 : 0]  FIFO_WDATA_3,		
		output    wire    [ADC7656_DATA_WIDTH-1 : 0]  FIFO_WDATA_4,	
		output    wire		WR_EN_0,
		output    wire		WR_EN_1,	
		output    wire		WR_EN_2,	
		output    wire		WR_EN_3,	
		output    wire		WR_EN_4,
        output    wire     ADC_RESET,
        output    wire     INTERNAL_TRIGGER,
		// User ports ends
		// Do not modify the ports beyond this line

		// Global Clock Signal
		input wire  S_AXI_ACLK,
		// Global Reset Signal. This Signal is Active LOW
		input wire  S_AXI_ARESETN,
		// Write address (issued by master, acceped by Slave)
		input wire [C_S_AXI_ADDR_WIDTH-1 : 0] S_AXI_AWADDR,
		// Write channel Protection type. This signal indicates the
    		// privilege and security level of the transaction, and whether
    		// the transaction is a data access or an instruction access.
		input wire [2 : 0] S_AXI_AWPROT,
		// Write address valid. This signal indicates that the master signaling
    		// valid write address and control information.
		input wire  S_AXI_AWVALID,
		// Write address ready. This signal indicates that the slave is ready
    		// to accept an address and associated control signals.
		output wire  S_AXI_AWREADY,
		// Write data (issued by master, acceped by Slave) 
		input wire [C_S_AXI_DATA_WIDTH-1 : 0] S_AXI_WDATA,
		// Write strobes. This signal indicates which byte lanes hold
    		// valid data. There is one write strobe bit for each eight
    		// bits of the write data bus.    
		input wire [(C_S_AXI_DATA_WIDTH/8)-1 : 0] S_AXI_WSTRB,
		// Write valid. This signal indicates that valid write
    		// data and strobes are available.
		input wire  S_AXI_WVALID,
		// Write ready. This signal indicates that the slave
    		// can accept the write data.
		output wire  S_AXI_WREADY,
		// Write response. This signal indicates the status
    		// of the write transaction.
		output wire [1 : 0] S_AXI_BRESP,
		// Write response valid. This signal indicates that the channel
    		// is signaling a valid write response.
		output wire  S_AXI_BVALID,
		// Response ready. This signal indicates that the master
    		// can accept a write response.
		input wire  S_AXI_BREADY,
		// Read address (issued by master, acceped by Slave)
		input wire [C_S_AXI_ADDR_WIDTH-1 : 0] S_AXI_ARADDR,
		// Protection type. This signal indicates the privilege
    		// and security level of the transaction, and whether the
    		// transaction is a data access or an instruction access.
		input wire [2 : 0] S_AXI_ARPROT,
		// Read address valid. This signal indicates that the channel
    		// is signaling valid read address and control information.
		input wire  S_AXI_ARVALID,
		// Read address ready. This signal indicates that the slave is
    		// ready to accept an address and associated control signals.
		output wire  S_AXI_ARREADY,
		// Read data (issued by slave)
		output wire [C_S_AXI_DATA_WIDTH-1 : 0] S_AXI_RDATA,
		// Read response. This signal indicates the status of the
    		// read transfer.
		output wire [1 : 0] S_AXI_RRESP,
		// Read valid. This signal indicates that the channel is
    		// signaling the required read data.
		output wire  S_AXI_RVALID,
		// Read ready. This signal indicates that the master can
    		// accept the read data and response information.
		input wire  S_AXI_RREADY
	);

	// AXI4LITE signals
	reg [C_S_AXI_ADDR_WIDTH-1 : 0] 	axi_awaddr;
	reg  	axi_awready;
	reg  	axi_wready;
	reg [1 : 0] 	axi_bresp;
	reg  	axi_bvalid;
	reg [C_S_AXI_ADDR_WIDTH-1 : 0] 	axi_araddr;
	reg  	axi_arready;
	reg [C_S_AXI_DATA_WIDTH-1 : 0] 	axi_rdata;
	reg [1 : 0] 	axi_rresp;
	reg  	axi_rvalid;

	// Example-specific design signals
	// local parameter for addressing 32 bit / 64 bit C_S_AXI_DATA_WIDTH
	// ADDR_LSB is used for addressing 32/64 bit registers/memories
	// ADDR_LSB = 2 for 32 bits (n downto 2)
	// ADDR_LSB = 3 for 64 bits (n downto 3)
	localparam integer ADDR_LSB = (C_S_AXI_DATA_WIDTH/32) + 1;
	localparam integer OPT_MEM_ADDR_BITS = 4;
	//----------------------------------------------
	//-- Signals for user logic register space example
	//------------------------------------------------
	//-- Number of Slave Registers 32
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg0;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg1;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg2;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg3;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg4;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg5;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg6;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg7;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg8;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg9;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg10;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg11;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg12;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg13;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg14;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg15;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg16;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg17;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg18;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg19;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg20;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg21;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg22;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg23;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg24;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg25;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg26;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg27;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg28;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg29;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg30;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg31;
	wire	 slv_reg_rden;
	wire	 slv_reg_wren;
	reg [C_S_AXI_DATA_WIDTH-1:0]	 reg_data_out;
	integer	 byte_index;
	reg	 aw_en;
	
    reg    [C_S_AXI_DATA_WIDTH-1:0]  manual_trigger_count;
	reg    [C_S_AXI_DATA_WIDTH-1:0]  auto_trigger_count;
	reg    [C_S_AXI_DATA_WIDTH-1:0]  end_count;	
    reg    [15:0]  reg_fifo_wdata_3;
	reg    [15:0]  reg_fifo_wdata_4;				

	// I/O Connections assignments

	assign S_AXI_AWREADY	= axi_awready;
	assign S_AXI_WREADY	= axi_wready;
	assign S_AXI_BRESP	= axi_bresp;
	assign S_AXI_BVALID	= axi_bvalid;
	assign S_AXI_ARREADY	= axi_arready;
	assign S_AXI_RDATA	= axi_rdata;
	assign S_AXI_RRESP	= axi_rresp;
	assign S_AXI_RVALID	= axi_rvalid;
	// Implement axi_awready generation
	// axi_awready is asserted for one S_AXI_ACLK clock cycle when both
	// S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_awready is
	// de-asserted when reset is low.

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_awready <= 1'b0;
	      aw_en <= 1'b1;
	    end 
	  else
	    begin    
	      if (~axi_awready && S_AXI_AWVALID && S_AXI_WVALID && aw_en)
	        begin
	          // slave is ready to accept write address when 
	          // there is a valid write address and write data
	          // on the write address and data bus. This design 
	          // expects no outstanding transactions. 
	          axi_awready <= 1'b1;
	          aw_en <= 1'b0;
	        end
	        else if (S_AXI_BREADY && axi_bvalid)
	            begin
	              aw_en <= 1'b1;
	              axi_awready <= 1'b0;
	            end
	      else           
	        begin
	          axi_awready <= 1'b0;
	        end
	    end 
	end       

	// Implement axi_awaddr latching
	// This process is used to latch the address when both 
	// S_AXI_AWVALID and S_AXI_WVALID are valid. 

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_awaddr <= 0;
	    end 
	  else
	    begin    
	      if (~axi_awready && S_AXI_AWVALID && S_AXI_WVALID && aw_en)
	        begin
	          // Write Address latching 
	          axi_awaddr <= S_AXI_AWADDR;
	        end
	    end 
	end       

	// Implement axi_wready generation
	// axi_wready is asserted for one S_AXI_ACLK clock cycle when both
	// S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_wready is 
	// de-asserted when reset is low. 

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_wready <= 1'b0;
	    end 
	  else
	    begin    
	      if (~axi_wready && S_AXI_WVALID && S_AXI_AWVALID && aw_en )
	        begin
	          // slave is ready to accept write data when 
	          // there is a valid write address and write data
	          // on the write address and data bus. This design 
	          // expects no outstanding transactions. 
	          axi_wready <= 1'b1;
	        end
	      else
	        begin
	          axi_wready <= 1'b0;
	        end
	    end 
	end       

	// Implement memory mapped register select and write logic generation
	// The write data is accepted and written to memory mapped registers when
	// axi_awready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted. Write strobes are used to
	// select byte enables of slave registers while writing.
	// These registers are cleared when reset (active low) is applied.
	// Slave register write enable is asserted when valid address and data are available
	// and the slave is ready to accept the write address and write data.
	assign slv_reg_wren = axi_wready && S_AXI_WVALID && axi_awready && S_AXI_AWVALID;

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      slv_reg0 <= 0;
	      slv_reg1 <= 0;
	      slv_reg2 <= 0;
	      slv_reg3 <= 0;
	      slv_reg4 <= 0;
	      slv_reg5 <= 0;
	      slv_reg6 <= 0;
	      slv_reg7 <= 0;
	      slv_reg8 <= 0;
	      slv_reg9 <= 0;
	      slv_reg10 <= 0;
	      slv_reg11 <= 0;
	      slv_reg12 <= 0;
	      slv_reg13 <= 0;
	      slv_reg14 <= 0;
	      slv_reg15 <= 0;
	      slv_reg16 <= 0;
	      slv_reg17 <= 0;
	      slv_reg18 <= 0;
	      slv_reg19 <= 0;
	      slv_reg20 <= 0;
	      slv_reg21 <= 0;
	      slv_reg22 <= 0;
	      slv_reg23 <= 0;
	      slv_reg24 <= 0;
	      slv_reg25 <= 0;
	      slv_reg26 <= 0;
	      slv_reg27 <= 0;
	      slv_reg28 <= 0;
	      slv_reg29 <= 0;
	      slv_reg30 <= 0;
	      slv_reg31 <= 0;
	    end 
	  else begin
	    if (slv_reg_wren)
	      begin
	        case ( axi_awaddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] )
	          5'h00:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 0
	                slv_reg0[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h01:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 1
	                slv_reg1[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h02:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 2
	                slv_reg2[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h03:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 3
	                slv_reg3[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h04:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 4
	                slv_reg4[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h05:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 5
	                slv_reg5[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h06:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 6
	                slv_reg6[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h07:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 7
	                slv_reg7[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h08:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 8
	                slv_reg8[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h09:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 9
	                slv_reg9[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h0A:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 10
	                slv_reg10[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h0B:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 11
	                slv_reg11[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h0C:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 12
	                slv_reg12[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h0D:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 13
	                slv_reg13[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h0E:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 14
	                slv_reg14[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h0F:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 15
	                slv_reg15[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h10:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 16
	                slv_reg16[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h11:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 17
	                slv_reg17[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h12:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 18
	                slv_reg18[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h13:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 19
	                slv_reg19[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h14:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 20
	                slv_reg20[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h15:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 21
	                slv_reg21[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h16:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 22
	                slv_reg22[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h17:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 23
	                slv_reg23[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h18:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 24
	                slv_reg24[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h19:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 25
	                slv_reg25[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h1A:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 26
	                slv_reg26[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h1B:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 27
	                slv_reg27[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h1C:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 28
	                slv_reg28[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h1D:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 29
	                slv_reg29[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h1E:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 30
	                slv_reg30[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          5'h1F:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 31
	                slv_reg31[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          default : begin
	                      slv_reg0 <= slv_reg0;
	                      slv_reg1 <= slv_reg1;
	                      slv_reg2 <= slv_reg2;
	                      slv_reg3 <= slv_reg3;
	                      slv_reg4 <= slv_reg4;
	                      slv_reg5 <= slv_reg5;
	                      slv_reg6 <= slv_reg6;
	                      slv_reg7 <= slv_reg7;
	                      slv_reg8 <= slv_reg8;
	                      slv_reg9 <= slv_reg9;
	                      slv_reg10 <= slv_reg10;
	                      slv_reg11 <= slv_reg11;
	                      slv_reg12 <= slv_reg12;
	                      slv_reg13 <= slv_reg13;
	                      slv_reg14 <= slv_reg14;
	                      slv_reg15 <= slv_reg15;
	                      slv_reg16 <= slv_reg16;
	                      slv_reg17 <= slv_reg17;
	                      slv_reg18 <= slv_reg18;
	                      slv_reg19 <= slv_reg19;
	                      slv_reg20 <= slv_reg20;
	                      slv_reg21 <= slv_reg21;
	                      slv_reg22 <= slv_reg22;
	                      slv_reg23 <= slv_reg23;
	                      slv_reg24 <= slv_reg24;
	                      slv_reg25 <= slv_reg25;
	                      slv_reg26 <= slv_reg26;
	                      slv_reg27 <= slv_reg27;
	                      slv_reg28 <= slv_reg28;
	                      slv_reg29 <= slv_reg29;
	                      slv_reg30 <= slv_reg30;
	                      slv_reg31 <= slv_reg31;
	                    end
	        endcase
	      end
	  end
	end    

	// Implement write response logic generation
	// The write response and response valid signals are asserted by the slave 
	// when axi_wready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted.  
	// This marks the acceptance of address and indicates the status of 
	// write transaction.

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_bvalid  <= 0;
	      axi_bresp   <= 2'b0;
	    end 
	  else
	    begin    
	      if (axi_awready && S_AXI_AWVALID && ~axi_bvalid && axi_wready && S_AXI_WVALID)
	        begin
	          // indicates a valid write response is available
	          axi_bvalid <= 1'b1;
	          axi_bresp  <= 2'b0; // 'OKAY' response 
	        end                   // work error responses in future
	      else
	        begin
	          if (S_AXI_BREADY && axi_bvalid) 
	            //check if bready is asserted while bvalid is high) 
	            //(there is a possibility that bready is always asserted high)   
	            begin
	              axi_bvalid <= 1'b0; 
	            end  
	        end
	    end
	end   

	// Implement axi_arready generation
	// axi_arready is asserted for one S_AXI_ACLK clock cycle when
	// S_AXI_ARVALID is asserted. axi_awready is 
	// de-asserted when reset (active low) is asserted. 
	// The read address is also latched when S_AXI_ARVALID is 
	// asserted. axi_araddr is reset to zero on reset assertion.

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_arready <= 1'b0;
	      axi_araddr  <= 32'b0;
	    end 
	  else
	    begin    
	      if (~axi_arready && S_AXI_ARVALID)
	        begin
	          // indicates that the slave has acceped the valid read address
	          axi_arready <= 1'b1;
	          // Read address latching
	          axi_araddr  <= S_AXI_ARADDR;
	        end
	      else
	        begin
	          axi_arready <= 1'b0;
	        end
	    end 
	end       

	// Implement axi_arvalid generation
	// axi_rvalid is asserted for one S_AXI_ACLK clock cycle when both 
	// S_AXI_ARVALID and axi_arready are asserted. The slave registers 
	// data are available on the axi_rdata bus at this instance. The 
	// assertion of axi_rvalid marks the validity of read data on the 
	// bus and axi_rresp indicates the status of read transaction.axi_rvalid 
	// is deasserted on reset (active low). axi_rresp and axi_rdata are 
	// cleared to zero on reset (active low).  
	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_rvalid <= 0;
	      axi_rresp  <= 0;
	    end 
	  else
	    begin    
	      if (axi_arready && S_AXI_ARVALID && ~axi_rvalid)
	        begin
	          // Valid read data is available at the read data bus
	          axi_rvalid <= 1'b1;
	          axi_rresp  <= 2'b0; // 'OKAY' response
	        end   
	      else if (axi_rvalid && S_AXI_RREADY)
	        begin
	          // Read data is accepted by the master
	          axi_rvalid <= 1'b0;
	        end                
	    end
	end    

	// Implement memory mapped register select and read logic generation
	// Slave register read enable is asserted when valid address is available
	// and the slave is ready to accept the read address.
	assign slv_reg_rden = axi_arready & S_AXI_ARVALID & ~axi_rvalid;
	always @(*)
	begin
	      // Address decoding for reading registers
	      case ( axi_araddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] )
	        5'h00   : reg_data_out <= slv_reg0;
	        5'h01   : reg_data_out <= slv_reg1;
	        5'h02   : reg_data_out <= slv_reg2;
	        5'h03   : reg_data_out <= slv_reg3;
	        5'h04   : reg_data_out <= slv_reg4;
	        5'h05   : reg_data_out <= slv_reg5;
	        5'h06   : reg_data_out <= slv_reg6;
	        5'h07   : reg_data_out <= slv_reg7;
	        5'h08   : reg_data_out <= slv_reg8;
	        5'h09   : reg_data_out <= slv_reg9;
	        5'h0A   : reg_data_out <= VER;
	        5'h0B   : reg_data_out <= manual_trigger_count;
	        5'h0C   : reg_data_out <= auto_trigger_count;
	        5'h0D   : reg_data_out <= end_count;
	        5'h0E   : reg_data_out <= {16'd0,FIFO_WDATA_3[ADC7656_DATA_WIDTH-1 : 0]};
	        5'h0F   : reg_data_out <= {16'd0,FIFO_WDATA_4[ADC7656_DATA_WIDTH-1 : 0]};
	        5'h10   : reg_data_out <= slv_reg16;
	        5'h11   : reg_data_out <= slv_reg17;
	        5'h12   : reg_data_out <= slv_reg18;
	        5'h13   : reg_data_out <= slv_reg19;
	        5'h14   : reg_data_out <= slv_reg20;
	        5'h15   : reg_data_out <= slv_reg21;
	        5'h16   : reg_data_out <= slv_reg22;
	        5'h17   : reg_data_out <= slv_reg23;
	        5'h18   : reg_data_out <= slv_reg24;
	        5'h19   : reg_data_out <= slv_reg25;
	        5'h1A   : reg_data_out <= slv_reg26;
	        5'h1B   : reg_data_out <= slv_reg27;
	        5'h1C   : reg_data_out <= slv_reg28;
	        5'h1D   : reg_data_out <= slv_reg29;
	        5'h1E   : reg_data_out <= slv_reg30;
	        5'h1F   : reg_data_out <= slv_reg31;
	        default : reg_data_out <= 0;
	      endcase
	end

	// Output register or memory read data
	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_rdata  <= 0;
	    end 
	  else
	    begin    
	      // When there is a valid read address (S_AXI_ARVALID) with 
	      // acceptance of read address by the slave (axi_arready), 
	      // output the read dada 
	      if (slv_reg_rden)
	        begin
	          axi_rdata <= reg_data_out;     // register read data
	        end   
	    end
	end    

	// Add user logic here
	reg    reg_sync_operation;
	reg    [7:0]   reg_sensing_count;	
    reg    [31:0]    interval_cnt_reg;	
	reg    [3:0]   STATE_MACHINE;  
	reg    [2:0]   convst_reg;  
	reg    [1:0]   convset_enble;
	reg    [15:0]  obtian_count;
	reg    [15:0]  reg_fifo_wdata_0;
	reg    [15:0]  reg_fifo_wdata_1;
	reg    [15:0]  reg_fifo_wdata_2;		
    reg     reg_cs;
    reg     reg_rd;
    reg     wr_en_0;
    reg     wr_en_1;
    reg     wr_en_2;
    reg     wr_en_3;
    reg     wr_en_4;    
    
    reg    [31:0]   TRIGGER_CNT;
    reg    [31:0]   INTERVAL_CNT;   
    reg    [1:0]    TRIGGER_EN_BUFFER;
 	reg    [3:0]    TRIGGER_STATE_MACHINE;
 	reg    trigger_out;
    reg    [1:0]    trigger_enable;   
    reg    [1:0]    reg_trigger;
    reg    reg_adc_reset;	
 	               
	wire   [C_S_AXI_DATA_WIDTH-1:0]    reg_reset;
	wire   [C_S_AXI_DATA_WIDTH-1:0]    sensing_count;
	wire   [C_S_AXI_DATA_WIDTH-1:0]    sens_interval;
	wire   [C_S_AXI_DATA_WIDTH-1:0]    auto_onoff;
	wire   [C_S_AXI_DATA_WIDTH-1:0]    total_count;		
	wire   [C_S_AXI_DATA_WIDTH-1:0]    reg_count_reset;				
	
	assign ADC_RESET = reg_adc_reset;
	assign reg_reset = slv_reg0;
	assign reg_count_reset =   slv_reg1;   	
	assign sensing_count = slv_reg2;
	assign sens_interval = slv_reg3;
	assign total_count  =  slv_reg4;  	
	assign auto_onoff  =   slv_reg5;   
	assign CS = reg_cs;
	assign RD = reg_rd;
	assign CONVST = convst_reg; 
	assign FIFO_WDATA_0 = reg_fifo_wdata_0;
	assign FIFO_WDATA_1 = reg_fifo_wdata_1;	
	assign FIFO_WDATA_2 = reg_fifo_wdata_2;	
	assign FIFO_WDATA_3 = reg_fifo_wdata_3;	
	assign FIFO_WDATA_4 = reg_fifo_wdata_4;	
	assign WR_EN_0 = wr_en_0;
	assign WR_EN_1 = wr_en_1;
	assign WR_EN_2 = wr_en_2;
	assign WR_EN_3 = wr_en_3;
	assign WR_EN_4 = wr_en_4;
	assign INTERNAL_TRIGGER = trigger_out;
	
    always @( posedge S_AXI_ACLK ) begin
         if ( reg_reset[0] == 1'b0 ) begin
             TRIGGER_CNT <= 32'd0;
             INTERVAL_CNT <= 32'd0;
             trigger_out <= 1'b0;
             TRIGGER_EN_BUFFER <= 2'd0;
             TRIGGER_STATE_MACHINE <= TRIGGER_STATE_IDLE;       
        end
        else begin
           case(TRIGGER_STATE_MACHINE)
                TRIGGER_STATE_IDLE:begin
                    TRIGGER_CNT <= 32'd0;
                    INTERVAL_CNT <= 32'd0;
                    trigger_out <= 1'b0;
                    TRIGGER_STATE_MACHINE <= TRIGGER_STATE_READY;                
                end
                TRIGGER_STATE_READY:begin
                    TRIGGER_EN_BUFFER[0] <= auto_onoff[0];
                    TRIGGER_EN_BUFFER[1] <= TRIGGER_EN_BUFFER[0];
                    if(TRIGGER_EN_BUFFER == 2'b01)  TRIGGER_STATE_MACHINE <= TRIGGER_STATE_EN_DETECT;                
                end
                TRIGGER_STATE_EN_DETECT:begin
                    if(TRIGGER_CNT < total_count) begin
                        if(INTERVAL_CNT == TRIGGER_INTERVAL-32'd1) begin
                                TRIGGER_CNT <= TRIGGER_CNT + 32'd1;
                                INTERVAL_CNT <= 32'd0;
                        end
                        else begin
                            //trigger_out <= 1'b0;
                            INTERVAL_CNT <= INTERVAL_CNT + 32'd1;                       
                        end
                        if(INTERVAL_CNT < TRIGGER_INTERVAL_HALF) trigger_out <= 1'b1;
                        else trigger_out <= 1'b0;                                            
                    end
                    else begin
                        TRIGGER_STATE_MACHINE <= TRIGGER_STATE_IDLE;
                    end
                end
                default TRIGGER_STATE_MACHINE <= TRIGGER_STATE_IDLE;
           endcase 
        end   
    end 	
    
    always @( posedge S_AXI_ACLK ) begin
        if ( reg_reset[0] == 1'b0 ) begin
            trigger_enable <=  2'b00;          
        end
        else    begin      
            trigger_enable  <=  {trigger_enable[0],   trigger_out};
        end
	end  
	
    always @( posedge S_AXI_ACLK ) begin
        if ( reg_reset[0] == 1'b0 ) begin
            reg_trigger <=  2'b00;          
        end
        else    begin
            reg_trigger  <=  {reg_trigger[0],   TRIGGER};
        end
	end	 
	
   always @( posedge S_AXI_ACLK ) begin
        if ( reg_reset[0] == 1'b0 ) begin
            reg_adc_reset            <=  1'b1;         
        end
        else    begin     
            reg_adc_reset            <=  1'b0;    
        end
	end  
	
    always @( posedge S_AXI_ACLK )  begin
        if (reg_reset[0] == 1'b0 ) begin
            convst_reg                                                  <=  3'b111;
            wr_en_0                                                     <=  1'b0;
            wr_en_1                                                     <=  1'b0; 
            wr_en_2                                                     <=  1'b0;
            wr_en_3                                                     <=  1'b0; 
            wr_en_4                                                     <=  1'b0;
            reg_cs							                            <=	1'b1;
            reg_rd							                            <=	1'b1;                    
            obtian_count			                                    <= 16'd0;
            manual_trigger_count[C_S_AXI_DATA_WIDTH-1:0]                <= 32'd0;
            auto_trigger_count[C_S_AXI_DATA_WIDTH-1:0]                  <= 32'd0; 
			reg_sync_operation                                          <=  1'b0;
			STATE_MACHINE                                               <=  STATE_START;                       
        end
        else begin   
            if(reg_count_reset[0] ==  1'b1)  begin           
                manual_trigger_count[C_S_AXI_DATA_WIDTH-1:0]            <= 32'd0;
                auto_trigger_count[C_S_AXI_DATA_WIDTH-1:0]              <= 32'd0; 
                end_count[C_S_AXI_DATA_WIDTH-1:0]                       <= 32'd0;                			    			                   
            end                     
            if(reg_trigger[1:0] ==  2'b01)  begin           
                reg_sync_operation                                      <=  1'b1; 
                reg_sensing_count                                       <=  8'd0; 
                interval_cnt_reg                                        <= 32'd0;                
			    STATE_MACHINE                                           <=  STATE_START;
			    auto_trigger_count[C_S_AXI_DATA_WIDTH-1:0]              <=  auto_trigger_count[C_S_AXI_DATA_WIDTH-1:0] + 32'd1;				    			                   
            end        
            if(trigger_enable[1:0] ==  2'b01)  begin           
                reg_sync_operation                                      <=  1'b1; 
                reg_sensing_count                                       <=  8'd0;	
                interval_cnt_reg                                        <= 32'd0;                	      
			    STATE_MACHINE                                           <=  STATE_START;	
			    manual_trigger_count[C_S_AXI_DATA_WIDTH-1:0]            <=  manual_trigger_count[C_S_AXI_DATA_WIDTH-1:0] + 32'd1;		                   
            end                
            if(reg_sync_operation)  begin
                if(reg_sensing_count <  sensing_count)  begin
                    if(interval_cnt_reg == sens_interval-1)    begin
                        case(STATE_MACHINE)
                            STATE_START : begin
                                obtian_count			                <= 16'd0;
                                reg_cs							        <=	1'b1;
                                reg_rd							        <=	1'b1;
                                convst_reg                              <=  3'b000;
                                convset_enble                           <=  2'b00;
                                wr_en_0                                 <=  1'b0;
                                wr_en_1                                 <=  1'b0; 
                                wr_en_2                                 <=  1'b0;
                                wr_en_3                                 <=  1'b0; 
                                wr_en_4                                 <=  1'b0;                                                                                                                            
                                STATE_MACHINE 				            <= STATE_ADC_START;                               
                            end  
                            STATE_ADC_START : begin    
                                if(BUSY == 0)   begin
                                    convst_reg                          <=  3'b111;   
                                    STATE_MACHINE 				        <= STATE_CONVSET_DELAY;
                                end                                   
                            end  
                             STATE_CONVSET_DELAY : begin
                                 if(convset_enble[1:0]  ==  2'b10)  begin
                                    STATE_MACHINE                       <=  STATE_DATA_OBTIAN;  
                                 end
                                 else begin
                                    convset_enble[1:0]   <=  {convset_enble[0],   BUSY};    
                                 end      
                             end 
                             STATE_DATA_OBTIAN : begin    
				                if(obtian_count < 16'd1) begin  
                                    reg_rd							     <=	1'b0;
                                    reg_cs                               <= 1'b0;		
                                    obtian_count	                     <= obtian_count + 16'd1;		                    
				                end 
				                else if(obtian_count < 16'd6) begin
                                  obtian_count	                         <= obtian_count + 16'd1;					                  
				                end 
				                else if(obtian_count < 16'd7) begin
                                    wr_en_0                              <=  1'b1;
                                    reg_fifo_wdata_0                     <=  ADC_DATA;                                  
                                    obtian_count	                     <= obtian_count + 16'd1;					                  
				                end 
				                else if(obtian_count < 16'd8) begin
                                    wr_en_0                              <=  1'b0;
                                    reg_rd							     <=	 1'b1;   				                
                                    obtian_count	                     <= obtian_count + 16'd1;					                  
				                end 
				                else if(obtian_count < 16'd9) begin  
                                    reg_rd							     <=	1'b0;		
                                    obtian_count	                     <= obtian_count + 16'd1;		                    
				                end 
				                else if(obtian_count < 16'd14) begin
                                    obtian_count	                     <= obtian_count + 16'd1;					                  
				                end 
				                else if(obtian_count < 16'd15) begin
                                  wr_en_1                                <=  1'b1;
				                  reg_fifo_wdata_1                       <=  ADC_DATA;                                   
                                  obtian_count	                         <= obtian_count + 16'd1;					                  
				                end 
				                else if(obtian_count < 16'd16) begin
                                  wr_en_1                                <=  1'b0;
                                  reg_rd							     <=	 1'b1;                                 
                                  obtian_count	                         <= obtian_count + 16'd1;					                  
				                end 
				                else if(obtian_count < 16'd17) begin  
                                  reg_rd							     <=	1'b0;		
                                  obtian_count	                         <= obtian_count + 16'd1;		                    
				                end 
				                else if(obtian_count < 16'd22) begin
                                  obtian_count	                         <= obtian_count + 16'd1;					                  
				                end 
				                else if(obtian_count < 16'd23) begin
                                  wr_en_2                                <=  1'b1;
 				                  reg_fifo_wdata_2                       <=  ADC_DATA;                                  
                                  obtian_count	                         <= obtian_count + 16'd1;					                  
				                end 
				                else if(obtian_count < 16'd24) begin
                                    wr_en_2                              <=  1'b0;
                                    reg_rd							     <=	 1'b1;                                 
                                    obtian_count	                     <= obtian_count + 16'd1;					                  
				                end
				                else if(obtian_count < 16'd25) begin  
                                    reg_rd							     <=	1'b0;		
                                    obtian_count	                     <= obtian_count + 16'd1;		                    
				                end 
				                else if(obtian_count < 16'd30) begin
                                    obtian_count	                     <= obtian_count + 16'd1;					                  
				                end 
				                else if(obtian_count < 16'd31) begin
                                    wr_en_3                              <=  1'b1;
                                    reg_fifo_wdata_3                     <=  ADC_DATA;                                   
                                    obtian_count	                     <= obtian_count + 16'd1;					                  
				                end 
				                else if(obtian_count < 16'd32) begin
                                    wr_en_3                              <=  1'b0;
                                    reg_rd							     <=	 1'b1;                                 
                                    obtian_count	                     <= obtian_count + 16'd1;					                  
				                end
				                else if(obtian_count < 16'd33) begin  
                                    reg_rd							     <=	1'b0;		
                                    obtian_count	                     <= obtian_count + 16'd1;		                    
				                end 
				                else if(obtian_count < 16'd38) begin
                                  obtian_count	                         <= obtian_count + 16'd1;					                  
				                end 
				                else if(obtian_count < 16'd39) begin
                                  wr_en_4                                <=  1'b1;
				                  reg_fifo_wdata_4                       <=  ADC_DATA;                                   
                                  obtian_count	                         <= obtian_count + 16'd1;					                  
				                end 
				                else if(obtian_count < 16'd40) begin
                                  wr_en_4                                <=  1'b0;				                
                                  reg_rd							     <=	 1'b1;                                 
                                  obtian_count	                         <= obtian_count + 16'd1;					                  
				                end 
				                else if(obtian_count < 16'd41) begin  
                                    reg_rd							     <=	1'b0;		
                                    obtian_count	                     <= obtian_count + 16'd1;		                    
				                end 
				                else if(obtian_count < 16'd46) begin
                                  reg_rd							     <=	 1'b1; 
                                  reg_cs							     <=	 1'b1;                               
                                  obtian_count	                         <= obtian_count + 16'd1;					                  
				                end 
				                else if(obtian_count < 16'd59) begin                             
                                  obtian_count	                         <= obtian_count + 16'd1;					                  
				                end 				                
				                else if(obtian_count < 16'd62) begin
                                  convst_reg                             <=  3'b000;                                  
                                  obtian_count	                         <= obtian_count + 16'd1;
                                  STATE_MACHINE                          <=  STATE_ADC_END;  					                  
				                end				                			                				                				                 				                 				                				                				                				                				                                        
                             end
                             STATE_ADC_END : begin 
                                end_count                                <= end_count + 32'd1;
                                reg_sensing_count			             <=	reg_sensing_count	+	8'd1;
                                interval_cnt_reg                         <= 32'd0; 
                                STATE_MACHINE 				             <= STATE_START;                                                                	                             
                             end                                                                                                   
                        endcase
                    end
                    else begin
                        interval_cnt_reg <= interval_cnt_reg + 32'd1;   
                    end                     
                end
                else    begin
                        reg_sync_operation                               <=  1'b0;
                end                	    
            end
        end
    end			    
	// User logic ends

	endmodule
