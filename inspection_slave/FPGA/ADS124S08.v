module ADS124S08
(
	input					nRESET,

	output				READY,
	
	input		[4:0]		ADC_ADDRESS,
	input		[7:0]		ADC_REG_WRITE_DATA,
	input					ADC_WRITE_REG_EN,
	input					ADC_READ_REG_EN,
	input					ADC_READ_DATA_EN,
	output				ADC_DONE,
	output	[7:0]		ADC_READ_REG_BUFFER,
	output	[31:0]	ADC_READ_DATA_BUFFER,
	
	input					ADC_REF_CLK,
	output				ADC_SCLK,
	output				ADC_nCS,
	output				ADC_MOSI,
	input					ADC_MISO,
	input					ADC_nDRDY
);

reg			[3:0]			STATE_MACHINE;
reg			[31:0]		REG_COUNT;

reg							REG_READY;
reg			[31:0]		REG_BUFFER;
reg			[7:0]			REG_READ_REG_BUFFER;
reg							REG_READ_REG_ON;
reg							REG_READ_REG_CLEAR;
reg			[31:0]		REG_READ_DATA_BUFFER;
reg							REG_READ_DATA_ON;
reg							REG_READ_DATA_CLEAR;
reg							REG_DONE;

reg							REG_nCS;
reg							REG_SCLK_ON;
reg							REG_MOSI;

parameter	STATE_IDLE				=		4'd0,
				STATE_READY				=		4'd1,
				STATE_READ_REG			=		4'd2,
				STATE_WRITE_REG		=		4'd3,
				STATE_READ_DATA		=		4'd4,
				STATE_DONE				=		4'd5;
				
always @ (posedge ADC_REF_CLK) begin
	if(!nRESET) begin
		REG_COUNT <= 32'd0;
		REG_nCS <= 1'b1;
		REG_SCLK_ON <= 1'b0;
		REG_MOSI <= 1'b0;
		REG_READ_REG_ON <= 1'b0;
		REG_READ_REG_CLEAR <= 1'b0;
		REG_READ_DATA_ON <= 1'b0;
		REG_READ_DATA_CLEAR <= 1'b0;
		REG_READY <= 1'b0;
		REG_DONE <= 1'b0;
		STATE_MACHINE <= STATE_IDLE;
	end
	else begin
		case(STATE_MACHINE)
			STATE_IDLE:begin
				REG_COUNT <= 32'd0;
				REG_nCS <= 1'b1;
				REG_SCLK_ON <= 1'b0;
				REG_MOSI <= 1'b0;
				REG_READ_REG_ON <= 1'b0;
				REG_READ_REG_CLEAR <= 1'b0;
				REG_READ_DATA_ON <= 1'b0;
				REG_READ_DATA_CLEAR <= 1'b0;
				REG_READY <= 1'b0;
				REG_DONE <= 1'b0;
				STATE_MACHINE <= STATE_READY;
			end
			STATE_READY:begin
				if(ADC_WRITE_REG_EN) begin
					REG_BUFFER <= {3'b010, ADC_ADDRESS, 8'b00000000, ADC_REG_WRITE_DATA, 8'b00000000};
					STATE_MACHINE <= STATE_WRITE_REG;
					REG_READY <= 1'b0;
				end
				else if(ADC_READ_REG_EN) begin
					REG_READ_REG_CLEAR <= 1'b1;
					REG_BUFFER <= {3'b001, ADC_ADDRESS, 8'b00000000, 16'h0000};
					STATE_MACHINE <= STATE_READ_REG;
					REG_READY <= 1'b0;
				end
				else if(ADC_READ_DATA_EN) begin
					REG_READ_DATA_CLEAR <= 1'b1;
					//REG_BUFFER <= {8'b00010010, 24'd0}; //RDATA
					REG_BUFFER <= 32'd0; //READ DATA DIRECT
					STATE_MACHINE <= STATE_READ_DATA;
					REG_READY <= 1'b0;
				end
				else REG_READY <= 1'b1;
			end
			STATE_WRITE_REG:begin
				if(REG_COUNT < 32'd1) begin
					REG_COUNT <= REG_COUNT + 32'd1;
					REG_nCS <= 1'b0;
				end
				else if(REG_COUNT < 32'd25) begin
					REG_COUNT <= REG_COUNT + 32'd1;
					REG_SCLK_ON <= 1'b1;
					REG_MOSI <= REG_BUFFER[31];
					REG_BUFFER <= {REG_BUFFER[30:0], 1'b0};
				end
				else if(REG_COUNT < 32'd26) begin
					REG_COUNT <= REG_COUNT + 32'd1;
					REG_SCLK_ON <= 1'b0;
					REG_nCS <= 1'b1;
				end
				else begin
					STATE_MACHINE <= STATE_DONE;
				end
			end
			STATE_READ_REG:begin
				if(REG_COUNT < 32'd1) begin
					REG_READ_REG_CLEAR <= 1'b0;
					REG_COUNT <= REG_COUNT + 32'd1;
					REG_nCS <= 1'b0;
				end
				else if(REG_COUNT < 32'd17) begin
					REG_COUNT <= REG_COUNT + 32'd1;
					REG_SCLK_ON <= 1'b1;
					REG_MOSI <= REG_BUFFER[31];
					REG_BUFFER <= {REG_BUFFER[30:0], 1'b0};
				end
				else if(REG_COUNT < 32'd25) begin
					REG_COUNT <= REG_COUNT + 32'd1;
					REG_SCLK_ON <= 1'b1;
					REG_MOSI <= REG_BUFFER[31];
					REG_BUFFER <= {REG_BUFFER[30:0], 1'b0};
					REG_READ_REG_ON <= 1'b1;
				end
				else if(REG_COUNT < 32'd26) begin
					REG_COUNT <= REG_COUNT + 32'd1;
					REG_SCLK_ON <= 1'b0;
					REG_READ_REG_ON <= 1'b0;
					REG_nCS <= 1'b1;
				end
				else begin
					STATE_MACHINE <= STATE_DONE;
				end
			end
			STATE_READ_DATA:begin
				/* RDATA
				if(REG_COUNT < 32'd1) begin
					REG_READ_DATA_CLEAR <= 1'b0;
					REG_COUNT <= REG_COUNT + 32'd1;
					REG_nCS <= 1'b0;
				end
				else if(REG_COUNT < 32'd2) begin
					if(!ADC_nDRDY) REG_COUNT <= REG_COUNT + 32'd1;
				end
				else if(REG_COUNT < 32'd10) begin
					REG_COUNT <= REG_COUNT + 32'd1;
					REG_SCLK_ON <= 1'b1;
					REG_MOSI <= REG_BUFFER[31];
					REG_BUFFER <= {REG_BUFFER[30:0], 1'b0};
				end
				else if(REG_COUNT < 32'd42) begin
					REG_COUNT <= REG_COUNT + 32'd1;
					REG_SCLK_ON <= 1'b1;
					REG_READ_DATA_ON <= 1'b1;
				end
				else if(REG_COUNT < 32'd43) begin
					REG_COUNT <= REG_COUNT + 32'd1;
					REG_SCLK_ON <= 1'b0;
					REG_READ_DATA_ON <= 1'b0;
					REG_nCS <= 1'b1;
				end
				else begin
					STATE_MACHINE <= STATE_DONE;
				end
				*/
				/*READ DATA DIRECT*/
				if(REG_COUNT < 32'd1) begin
					REG_READ_DATA_CLEAR <= 1'b0;
					REG_COUNT <= REG_COUNT + 32'd1;
					REG_nCS <= 1'b0;
				end
				else if(REG_COUNT < 32'd33) begin
					REG_COUNT <= REG_COUNT + 32'd1;
					REG_SCLK_ON <= 1'b1;
					REG_MOSI <= REG_BUFFER[31];
					REG_BUFFER <= {REG_BUFFER[30:0], 1'b0};
					REG_READ_DATA_ON <= 1'b1;
				end
				else if(REG_COUNT < 32'd34) begin
					REG_COUNT <= REG_COUNT + 32'd1;
					REG_SCLK_ON <= 1'b0;
					REG_READ_DATA_ON <= 1'b0;
					REG_nCS <= 1'b1;
				end
				else begin
					STATE_MACHINE <= STATE_DONE;
				end
			end
			STATE_DONE:begin
				if((!ADC_WRITE_REG_EN) && (!ADC_READ_REG_EN) && (!ADC_READ_DATA_EN)) begin
					REG_DONE <= 1'b0;
					STATE_MACHINE <= STATE_READY;
				end
				else begin
					REG_DONE <= 1'b1;
					REG_COUNT <= 32'd0;
				end
			end
			default STATE_MACHINE <= STATE_IDLE;
		endcase
	end
end

always @ (negedge ADC_REF_CLK) begin
	if(!nRESET) begin
		REG_READ_REG_BUFFER <= 8'd0;
		REG_READ_DATA_BUFFER <= 32'd0;
	end
	else begin
		if(REG_READ_REG_CLEAR) REG_READ_REG_BUFFER <= 8'd0;
		else begin
			if(REG_READ_REG_ON) REG_READ_REG_BUFFER <= {REG_READ_REG_BUFFER[6:0], ADC_MISO};
		end
		if(REG_READ_DATA_CLEAR) REG_READ_DATA_BUFFER <= 32'd0;
		else begin
			if(REG_READ_DATA_ON) REG_READ_DATA_BUFFER <= {REG_READ_DATA_BUFFER[30:0], ADC_MISO}; // include CRC
		end
	end
end

assign ADC_READ_DATA_BUFFER = REG_READ_DATA_BUFFER;
assign ADC_READ_REG_BUFFER = REG_READ_REG_BUFFER;
assign ADC_DONE = REG_DONE;
assign READY = REG_READY;
assign ADC_nCS = REG_nCS;
assign ADC_SCLK = REG_SCLK_ON & ADC_REF_CLK;
assign ADC_MOSI = REG_MOSI;

endmodule
