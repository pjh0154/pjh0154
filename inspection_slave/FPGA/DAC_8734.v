module DAC_8734
(
	input					nRESET,
	
	input					DAC_REF_CLK,
	output	[4:0]		DAC_SCLK,
	output	[4:0]		DAC_nCS,
	output				DAC_SDI,
	output				DAC_READY,
	
	output				DAC_OUTPUT_DONE,
	output				DAC_SET_DONE,
	
	input		[15:0]	REG_DAC_CONTROL,
	input		[15:0]	REG_DAC0_OUTPUT0,
	input		[15:0]	REG_DAC0_OUTPUT1,
	input		[15:0]	REG_DAC0_OUTPUT2,
	input		[15:0]	REG_DAC0_OUTPUT3,
	input		[15:0]	REG_DAC1_OUTPUT0,
	input		[15:0]	REG_DAC1_OUTPUT1,
	input		[15:0]	REG_DAC1_OUTPUT2,
	input		[15:0]	REG_DAC1_OUTPUT3,
	input		[15:0]	REG_DAC2_OUTPUT0,
	input		[15:0]	REG_DAC2_OUTPUT1,
	input		[15:0]	REG_DAC2_OUTPUT2,
	input		[15:0]	REG_DAC2_OUTPUT3,
	input		[15:0]	REG_DAC3_OUTPUT0,
	input		[15:0]	REG_DAC3_OUTPUT1,
	input		[15:0]	REG_DAC3_OUTPUT2,
	input		[15:0]	REG_DAC3_OUTPUT3,
	input		[15:0]	REG_DAC4_OUTPUT0,
	input		[15:0]	REG_DAC4_OUTPUT1,
	input		[15:0]	REG_DAC4_OUTPUT2,
	input		[15:0]	REG_DAC4_OUTPUT3,
	
	input		[15:0]	REG_DAC_SETCONTROL,
	input		[15:0]	REG_DAC_SETBUFFER0,
	input		[15:0]	REG_DAC_SETBUFFER1
);

reg			[4:0]			REG_DAC_nCS;
reg							REG_DAC_SDI;

reg							REG_DAC_OUTPUT_DONE;
reg							REG_DAC_SET_DONE;
reg							REG_DAC_READY;

reg			[8:0]			DAC_TX_COUNT;
reg			[23:0]		DAC_TX_BUFFER;
reg			[3:0]			STATE_MACHINE;

parameter	STATE_IDLE				=		4'd0,
				STATE_READY				=		4'd1,
				STATE_OUT_TX			=		4'd2,
				STATE_SET_TX			=		4'd3,
				STATE_OUT_WAIT			=		4'd4,
				STATE_SET_WAIT			=		4'd5;

always @ (posedge DAC_REF_CLK) begin
	if(!nRESET) begin
		REG_DAC_nCS <= 5'b11111;
		REG_DAC_SDI <= 1'b1;
		REG_DAC_OUTPUT_DONE <= 1'b0;
		REG_DAC_SET_DONE <= 1'b0;
		REG_DAC_READY <= 1'b0;
		STATE_MACHINE <= STATE_IDLE;
	end
	else begin
		case(STATE_MACHINE)
			STATE_IDLE:begin
				REG_DAC_nCS <= 5'b11111;
				REG_DAC_SDI <= 1'b1;
				REG_DAC_OUTPUT_DONE <= 1'b0;
				REG_DAC_SET_DONE <= 1'b0;
				REG_DAC_READY <= 1'b0;
				STATE_MACHINE <= STATE_READY;
			end
			STATE_READY:begin
				if(REG_DAC_CONTROL[0] == 1'b1) begin
					REG_DAC_READY <= 1'b0;
					DAC_TX_COUNT <= 9'd0;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b00, REG_DAC0_OUTPUT0};
					STATE_MACHINE <= STATE_OUT_TX;
				end
				else if(REG_DAC_SETCONTROL[4:0] != 5'd0) begin
					REG_DAC_READY <= 1'b0;
					DAC_TX_COUNT <= 9'd0;
					DAC_TX_BUFFER <= {REG_DAC_SETBUFFER1[7:0], REG_DAC_SETBUFFER0[15:0]};
					STATE_MACHINE <= STATE_SET_TX;
				end
				else begin
					REG_DAC_READY <= 1'b1;
					REG_DAC_SDI <= 1'b1;
				end
			end
			STATE_OUT_TX:begin
				if(DAC_TX_COUNT < 9'd24) begin
					REG_DAC_nCS <= 5'b11110;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd25) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b01, REG_DAC0_OUTPUT1};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd49) begin
					REG_DAC_nCS <= 5'b11110;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd50) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b10, REG_DAC0_OUTPUT2};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd74) begin
					REG_DAC_nCS <= 5'b11110;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd75) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b11, REG_DAC0_OUTPUT3};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd99) begin
					REG_DAC_nCS <= 5'b11110;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd100) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b00, REG_DAC1_OUTPUT0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd124) begin
					REG_DAC_nCS <= 5'b11101;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd125) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b01, REG_DAC1_OUTPUT1};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd149) begin
					REG_DAC_nCS <= 5'b11101;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd150) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b10, REG_DAC1_OUTPUT2};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd174) begin
					REG_DAC_nCS <= 5'b11101;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd175) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b11, REG_DAC1_OUTPUT3};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd199) begin
					REG_DAC_nCS <= 5'b11101;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd200) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b00, REG_DAC2_OUTPUT0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd224) begin
					REG_DAC_nCS <= 5'b11011;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd225) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b01, REG_DAC2_OUTPUT1};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd249) begin
					REG_DAC_nCS <= 5'b11011;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd250) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b10, REG_DAC2_OUTPUT2};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd274) begin
					REG_DAC_nCS <= 5'b11011;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd275) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b11, REG_DAC2_OUTPUT3};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd299) begin
					REG_DAC_nCS <= 5'b11011;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd300) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b00, REG_DAC3_OUTPUT0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd324) begin
					REG_DAC_nCS <= 5'b10111;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd325) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b01, REG_DAC3_OUTPUT1};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd349) begin
					REG_DAC_nCS <= 5'b10111;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd350) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b10, REG_DAC3_OUTPUT2};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd374) begin
					REG_DAC_nCS <= 5'b10111;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd375) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b11, REG_DAC3_OUTPUT3};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd399) begin
					REG_DAC_nCS <= 5'b10111;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd400) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b00, REG_DAC4_OUTPUT0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd424) begin
					REG_DAC_nCS <= 5'b01111;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd425) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b01, REG_DAC4_OUTPUT1};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd449) begin
					REG_DAC_nCS <= 5'b01111;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd450) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b10, REG_DAC4_OUTPUT2};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd474) begin
					REG_DAC_nCS <= 5'b01111;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd475) begin
					REG_DAC_nCS <= 5'b11111;
					DAC_TX_BUFFER <= {1'b0, 3'b000, 2'b01, 2'b11, REG_DAC4_OUTPUT3};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd499) begin
					REG_DAC_nCS <= 5'b01111;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd500) begin
					REG_DAC_nCS <= 5'b11111;
					STATE_MACHINE <= STATE_OUT_WAIT;
				end
			end
			STATE_SET_TX:begin
				if(DAC_TX_COUNT < 9'd24) begin
					if(REG_DAC_SETCONTROL[0] == 1'b1) REG_DAC_nCS <= 5'b11110;
					else if(REG_DAC_SETCONTROL[1] == 1'b1) REG_DAC_nCS <= 5'b11101;
					else if(REG_DAC_SETCONTROL[2] == 1'b1) REG_DAC_nCS <= 5'b11011;
					else if(REG_DAC_SETCONTROL[3] == 1'b1) REG_DAC_nCS <= 5'b10111;
					else if(REG_DAC_SETCONTROL[4] == 1'b1) REG_DAC_nCS <= 5'b01111;
					REG_DAC_SDI <= DAC_TX_BUFFER[23];
					DAC_TX_BUFFER <= {DAC_TX_BUFFER[22:0], 1'b0};
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end
				else if(DAC_TX_COUNT < 9'd25) begin
					REG_DAC_nCS <= 5'b11111;
					STATE_MACHINE <= STATE_SET_WAIT;
					DAC_TX_COUNT <= DAC_TX_COUNT + 9'd1;
				end		
			end
			STATE_OUT_WAIT:begin
				if(REG_DAC_CONTROL[0] == 1'b0) begin
					REG_DAC_OUTPUT_DONE <= 1'b0;
					STATE_MACHINE <= STATE_READY;
				end
				else REG_DAC_OUTPUT_DONE <= 1'b1;
			end
			STATE_SET_WAIT:begin
				if(REG_DAC_SETCONTROL[4:0] == 5'd0) begin
					REG_DAC_SET_DONE <= 1'b0;
					STATE_MACHINE <= STATE_READY;
				end
				else REG_DAC_SET_DONE <= 1'b1;
			end
			default STATE_MACHINE <= STATE_IDLE;
		endcase
	end
end

assign	DAC_SCLK[0]	=		(!REG_DAC_nCS[0]) ? DAC_REF_CLK : 1'b1;
assign	DAC_SCLK[1]	=		(!REG_DAC_nCS[1]) ? DAC_REF_CLK : 1'b1;
assign	DAC_SCLK[2]	=		(!REG_DAC_nCS[2]) ? DAC_REF_CLK : 1'b1;
assign	DAC_SCLK[3]	=		(!REG_DAC_nCS[3]) ? DAC_REF_CLK : 1'b1;
assign	DAC_SCLK[4]	=		(!REG_DAC_nCS[4]) ? DAC_REF_CLK : 1'b1;
assign	DAC_nCS = REG_DAC_nCS;
assign	DAC_SDI = REG_DAC_SDI;
assign	DAC_OUTPUT_DONE = REG_DAC_OUTPUT_DONE;
assign	DAC_SET_DONE = REG_DAC_SET_DONE;
assign	DAC_READY = REG_DAC_READY;

endmodule
