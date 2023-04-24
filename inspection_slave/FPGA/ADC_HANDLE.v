module ADC_HANDLE
(
	input					CLK_100MHz,
	input					nRESET,
	input		[7:1]		ADDRESS,
	inout		[15:0]	DATA,
	input					nCS,
	input					nRE,
	input					nWE,
	
	input					ADC_REF_CLK,
	output				ADC_SCLK,
	output				ADC_nCS,
	output				ADC_MOSI,
	input					ADC_MISO,
	input					ADC_nDRDY,
	output				ADC_START,
	output				ADC_nRST,
	output	[2:0]		ADC_SEL
);

parameter	BASE_ADDRESS					=		7'd30,

				ADC_STATUS_OFFSET				=		7'd0,
				ADC_CONTROL_OFFSET			=		7'd1,
				ADC_REG_ADDRESS_OFFSET		=		7'd2,
				ADC_REG_DATA_OFFSET			=		7'd3,
				ADC_REG_READ_OFFSET			=		7'd4,
				ADC_READ_DATA_L_OFFSET		=		7'd5,
				ADC_READ_DATA_H_OFFSET		=		7'd6,
				ADC_AUTO_DATA_MUX_OFFSET	=		7'd7,
				ADC_AUTO_DATA_L_OFFSET		=		7'd8,
				ADC_AUTO_DATA_H_OFFSET		=		7'd9,
				ADC_AUTO_DELAY_L_OFFSET		=		7'd10,
				ADC_AUTO_DELAY_H_OFFSET		=		7'd11;
				
/*
ADC_STATUS
bit 3~15 Reserved
bit 2 ADC_AUTO_READ_DONE
bit 1 ADC_DRDY
bit 0 ADC_READY
*/
reg		[15:0]			REG_ADC_STATUS;
/*
ADC_CONTROL
bit 10~15 Reseved
bit 9 ADC_AUTO_READ_DONE_CLEAR
bit 8 ADC_AUTO_READ_EN
bit 5~7 ADC_SEL
bit 4 ADC_START_EN
bit 3 ADC_READ_DATA_EN
bit 2 ADC_READ_REGISTER_EN
bit 1 ADC_WRITE_REGISTER_EN
bit 0 ADC_RESET
*/
reg		[15:0]			REG_ADC_CONTROL;

wire							WIRE_ADC_READY;
reg		[4:0]				REG_ADC_RW_REG_ADDRESS;
reg		[7:0]				REG_ADC_RW_REG_DATA;
wire							WIRE_ADC_DONE;
wire		[7:0]				WIRE_ADC_READ_BUFFER;
wire		[15:0]			WIRE_ADC_READ_DATA_BUFFER_L;
wire		[15:0]			WIRE_ADC_READ_DATA_BUFFER_H;

reg							REG_AUTO_START;
reg		[2:0]				REG_AUTO_SEL;

reg		[15:0]			REG_AUTO_DATA_MUX;

always @ (posedge CLK_100MHz) begin
	if(!nRESET) begin
		REG_ADC_STATUS				<=			16'b0000000000000001;
		REG_ADC_CONTROL			<=			16'b0000000000000000;
		REG_ADC_RW_REG_DATA		<=			8'd0;
		REG_AUTO_DATA_MUX			<=			16'd0;
		REG_AUTO_DELAY				<=			32'd0;
	end
	else begin
		if(!nCS) begin
			if(!nWE) begin
				if			(ADDRESS == (BASE_ADDRESS + ADC_CONTROL_OFFSET))				REG_ADC_CONTROL				<=		DATA;
				else if	(ADDRESS == (BASE_ADDRESS + ADC_REG_ADDRESS_OFFSET))			REG_ADC_RW_REG_ADDRESS		<=		DATA[4:0];
				else if	(ADDRESS == (BASE_ADDRESS + ADC_REG_DATA_OFFSET))				REG_ADC_RW_REG_DATA			<=		DATA[7:0];
				else if	(ADDRESS == (BASE_ADDRESS + ADC_AUTO_DATA_MUX_OFFSET))		REG_AUTO_DATA_MUX				<=		DATA;
				else if	(ADDRESS == (BASE_ADDRESS + ADC_AUTO_DELAY_L_OFFSET))			REG_AUTO_DELAY[15:0]			<=		DATA;
				else if	(ADDRESS == (BASE_ADDRESS + ADC_AUTO_DELAY_H_OFFSET))			REG_AUTO_DELAY[31:0]			<=		DATA;
			end
		end
		REG_ADC_STATUS[0] <= WIRE_ADC_READY;
		REG_ADC_STATUS[1] <= ~ADC_nDRDY;
		REG_ADC_STATUS[2] <= REG_AUTO_ADC_DONE;
		if(ADC_RST_BUFFER == 5'b11111) REG_ADC_CONTROL[0] <= 1'b0;
		if(WIRE_ADC_DONE) REG_ADC_CONTROL[3:1] <= 3'b000;
		if(REG_AUTO_ADC_DONE_CLEAR) REG_ADC_CONTROL[9] <= 1'b0;
	end
end

wire		[15:0]		READ_DATA;
assign	DATA			=		(!nCS & !nRE) ? READ_DATA : 16'bzzzzzzzzzzzzzzzz;
assign	READ_DATA	=		(ADDRESS == (BASE_ADDRESS + ADC_STATUS_OFFSET))				?		REG_ADC_STATUS :
									(ADDRESS == (BASE_ADDRESS + ADC_CONTROL_OFFSET))			?		REG_ADC_CONTROL :
									(ADDRESS == (BASE_ADDRESS + ADC_REG_ADDRESS_OFFSET))		?		{11'd0, REG_ADC_RW_REG_ADDRESS} :
									(ADDRESS == (BASE_ADDRESS + ADC_REG_DATA_OFFSET))			?		{8'd0, REG_ADC_RW_REG_DATA} :
									(ADDRESS == (BASE_ADDRESS + ADC_REG_READ_OFFSET))			?		{8'd0, WIRE_ADC_READ_BUFFER} :
									(ADDRESS == (BASE_ADDRESS + ADC_READ_DATA_L_OFFSET))		?		WIRE_ADC_READ_DATA_BUFFER_L:
									(ADDRESS == (BASE_ADDRESS + ADC_READ_DATA_H_OFFSET))		?		WIRE_ADC_READ_DATA_BUFFER_H:
									(ADDRESS == (BASE_ADDRESS + ADC_AUTO_DATA_MUX_OFFSET))	?		REG_AUTO_DATA_MUX:
									(ADDRESS == (BASE_ADDRESS + ADC_AUTO_DATA_L_OFFSET))		?		WIRE_AUDO_ADC_DATA_SEL[15:0]:
									(ADDRESS == (BASE_ADDRESS + ADC_AUTO_DATA_H_OFFSET))		?		WIRE_AUDO_ADC_DATA_SEL[31:16]:
									(ADDRESS == (BASE_ADDRESS + ADC_AUTO_DELAY_L_OFFSET))		?		REG_AUTO_DELAY[15:0]:
									(ADDRESS == (BASE_ADDRESS + ADC_AUTO_DELAY_H_OFFSET))		?		REG_AUTO_DELAY[31:16] : 16'bzzzzzzzzzzzzzzzz;

reg [4:0] ADC_RST_BUFFER;
always @ (posedge ADC_REF_CLK) begin
	if(!nRESET) ADC_RST_BUFFER <= 5'd0;
	else begin
		if(REG_ADC_CONTROL[0]) ADC_RST_BUFFER <= {ADC_RST_BUFFER[3:0], 1'b1};
		else ADC_RST_BUFFER <= 5'd0;
	end
end

assign	ADC_nRST		=	(!ADC_RST_BUFFER[0]) & nRESET;
assign	ADC_START	=	(!REG_ADC_CONTROL[8]) ? ~REG_ADC_CONTROL[4] : REG_AUTO_START;
assign	ADC_SEL		=	(!REG_ADC_CONTROL[8]) ? REG_ADC_CONTROL[7:5] : REG_AUTO_SEL;

wire						READ_DATA_EN;
assign	READ_DATA_EN = (!REG_ADC_CONTROL[8]) ? REG_ADC_CONTROL[3] : REG_AUTO_READ_EN;
wire						WIRE_REG_EN;
assign	WIRE_REG_EN = (!REG_ADC_CONTROL[8]) ? REG_ADC_CONTROL[1] : REG_AUTO_WRITE_EN;
wire		[4:0]			WIRE_ADC_RW_REG_ADDRESS;
assign	WIRE_ADC_RW_REG_ADDRESS = (!REG_ADC_CONTROL[8]) ? REG_ADC_RW_REG_ADDRESS : REG_AUTO_ADDRESS;
wire		[7:0]			WIRE_ADC_RW_REG_DATA;
assign	WIRE_ADC_RW_REG_DATA = (!REG_ADC_CONTROL[8]) ? REG_ADC_RW_REG_DATA : REG_AUTO_DATA;

ADS124S08 ADS124S08_inst
(
	.nRESET(nRESET) ,	// input  nRESET_sig
	.READY(WIRE_ADC_READY) ,	// output  nREADY_sig
	.ADC_ADDRESS(WIRE_ADC_RW_REG_ADDRESS) ,	// input [4:0] ADC_ADDRESS_sig
	.ADC_REG_WRITE_DATA(WIRE_ADC_RW_REG_DATA) ,	// input [7:0] ADC_REG_WRITE_DATA_sig
	.ADC_WRITE_REG_EN(WIRE_REG_EN) ,	// input  ADC_WRITE_REG_EN_sig
	.ADC_READ_REG_EN(REG_ADC_CONTROL[2]) ,	// input  ADC_READ_REG_EN_sig
	.ADC_READ_DATA_EN(READ_DATA_EN) ,	// input  ADC_READ_DATA_EN_sig
	.ADC_DONE(WIRE_ADC_DONE) ,	// output  ADC_DONE_sig
	.ADC_READ_REG_BUFFER(WIRE_ADC_READ_BUFFER) ,	// output [7:0] ADC_READ_REG_BUFFER_sig
	.ADC_READ_DATA_BUFFER({WIRE_ADC_READ_DATA_BUFFER_H, WIRE_ADC_READ_DATA_BUFFER_L}) ,	// output [31:0] ADC_READ_DATA_BUFFER_sig
	.ADC_REF_CLK(ADC_REF_CLK) ,	// input  ADC_REF_CLK_sig
	.ADC_SCLK(ADC_SCLK) ,	// output  ADC_SCLK_sig
	.ADC_nCS(ADC_nCS) ,	// output  ADC_nCS_sig
	.ADC_MOSI(ADC_MOSI) ,	// output  ADC_MOSI_sig
	.ADC_MISO(ADC_MISO) 	// input  ADC_MISO_sig
);

reg		[7:0]				STATE_MACHINE;
reg		[31:0]			STATE_MACHINE_COUNT;
reg		[4:0]				REG_AUTO_CH_COUNT;
reg							REG_AUTO_READ_EN;
reg							REG_AUTO_WRITE_EN;
reg		[4:0]				REG_AUTO_ADDRESS;
reg		[7:0]				REG_AUTO_DATA;
reg							REG_AUTO_ADC_DONE;
reg							REG_AUTO_ADC_DONE_CLEAR;

reg		[7:0]				REG_AUTO_ADC_DATA_COUNT;
reg		[31:0]			REG_AUTO_ADC_DATA[55:0];
wire		[31:0]			WIRE_AUDO_ADC_DATA_SEL;
reg		[31:0]			REG_AUTO_DELAY;
assign WIRE_AUDO_ADC_DATA_SEL = REG_AUTO_ADC_DATA[REG_AUTO_DATA_MUX];

parameter	STATE_IDLE						=		8'd0,
				STATE_ADC_READY				=		8'd1,
				STATE_REG_CHANNEL				=		8'd2,
				STATE_REG_DONE					=		8'd3,
				STATE_MUX_SEL_DELAY			=		8'd4,
				STATE_START						=		8'd5,
				STATE_nDRDY						=		8'd6,
				STATE_READ						=		8'd7,
				STATE_DONE						=		8'd8,
				STATE_MCU						=		8'd9;

always @ (posedge CLK_100MHz) begin
	if((!nRESET) || (!REG_ADC_CONTROL[8])) begin
		REG_AUTO_READ_EN <= 1'd0;
		REG_AUTO_WRITE_EN <= 1'd0;
		REG_AUTO_CH_COUNT <= 5'd0;
		REG_AUTO_SEL <= 3'd0;
		REG_AUTO_START <= 1'd1;
		STATE_MACHINE <= STATE_IDLE;
		STATE_MACHINE_COUNT <= 32'd0;
		REG_AUTO_ADC_DATA_COUNT <= 8'd0;
		REG_AUTO_ADC_DONE <= 1'd0;
		REG_AUTO_ADC_DONE_CLEAR <= 1'd0;
	end
	else begin
		case(STATE_MACHINE)
			STATE_IDLE:begin
				if(STATE_MACHINE_COUNT < 32'd100) begin
					STATE_MACHINE_COUNT <= STATE_MACHINE_COUNT + 32'd1;
					REG_AUTO_READ_EN <= 1'd0;
					REG_AUTO_WRITE_EN <= 1'd0;
					REG_AUTO_CH_COUNT <= 5'd0;
					REG_AUTO_SEL <= 3'd0;
					REG_AUTO_START <= 1'd1;
					REG_AUTO_ADC_DATA_COUNT <= 8'd0;
					REG_AUTO_ADC_DONE <= 1'd0;
					REG_AUTO_ADC_DONE_CLEAR <= 1'd0;
				end
				else begin
					STATE_MACHINE <= STATE_ADC_READY;
					STATE_MACHINE_COUNT <= 32'd0;
				end
			end
			STATE_ADC_READY:begin
				if(WIRE_ADC_READY) begin
					STATE_MACHINE_COUNT <= 32'd0;
					STATE_MACHINE <= STATE_REG_CHANNEL;
				end
			end
			STATE_REG_CHANNEL:begin
				REG_AUTO_ADDRESS <= 5'b00010;
				REG_AUTO_DATA <= {REG_AUTO_CH_COUNT[3:0], 4'b1100};
				REG_AUTO_WRITE_EN <= 1'd1;
				STATE_MACHINE <= STATE_REG_DONE;
			end
			STATE_REG_DONE:begin
				if(WIRE_ADC_DONE) begin
					REG_AUTO_WRITE_EN <= 1'd0;
					STATE_MACHINE <= STATE_MUX_SEL_DELAY;
				end
			end
			STATE_MUX_SEL_DELAY:begin
				if(STATE_MACHINE_COUNT < REG_AUTO_DELAY) STATE_MACHINE_COUNT <= STATE_MACHINE_COUNT + 32'd1;
				else begin
					STATE_MACHINE_COUNT <= 32'd0;
					STATE_MACHINE <= STATE_START;
				end
			end
			STATE_START:begin
				if(STATE_MACHINE_COUNT < 32'd50) begin
					STATE_MACHINE_COUNT <= STATE_MACHINE_COUNT + 32'd1;
					REG_AUTO_START <= 1'd0;
				end
				else begin
					STATE_MACHINE_COUNT <= 32'd0;
					REG_AUTO_START <= 1'd1;
					STATE_MACHINE <= STATE_nDRDY;
				end
			end
			STATE_nDRDY:begin
				if(!ADC_nDRDY) begin
					STATE_MACHINE_COUNT <= 32'd0;
					STATE_MACHINE <= STATE_READ;
				end
			end
			STATE_READ:begin
				if(WIRE_ADC_DONE) begin
					STATE_MACHINE <= STATE_DONE;
					REG_AUTO_READ_EN <= 1'd0;
				end
				else REG_AUTO_READ_EN <= 1'd1;
			end
			STATE_DONE:begin
				if(REG_AUTO_CH_COUNT >= 5'd6) begin
					if(REG_AUTO_SEL == 3'b111) begin
						REG_AUTO_SEL <= 3'd0;
						REG_AUTO_CH_COUNT <= 5'd0;
						//WAIT for ADC DATA READ at MCU
						REG_AUTO_ADC_DONE <= 1'd1;
						STATE_MACHINE <= STATE_MCU;
					end
					else begin
						REG_AUTO_SEL <= REG_AUTO_SEL + 3'd1;
						REG_AUTO_CH_COUNT <= 5'd0;
						STATE_MACHINE <= STATE_ADC_READY;
					end
				end
				else begin
					REG_AUTO_CH_COUNT <= REG_AUTO_CH_COUNT + 5'd1;
					STATE_MACHINE <= STATE_ADC_READY;
				end
				//ADC VALUE SAVE
				REG_AUTO_ADC_DATA[REG_AUTO_ADC_DATA_COUNT] <= {WIRE_ADC_READ_DATA_BUFFER_H, WIRE_ADC_READ_DATA_BUFFER_L};
				REG_AUTO_ADC_DATA_COUNT <= REG_AUTO_ADC_DATA_COUNT + 8'd1;
			end
			STATE_MCU:begin
				if(REG_ADC_CONTROL[9]) begin
					STATE_MACHINE <= STATE_ADC_READY;
					REG_AUTO_ADC_DONE <= 1'd0;
					REG_AUTO_ADC_DONE_CLEAR <= 1'd1;
					REG_AUTO_ADC_DATA_COUNT <= 8'd0;
				end
				else begin
					REG_AUTO_ADC_DONE_CLEAR <= 1'd0;
				end
			end
			default STATE_MACHINE <= STATE_IDLE;
		endcase
	end
end

				
endmodule
