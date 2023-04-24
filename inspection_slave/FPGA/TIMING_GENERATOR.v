module TIMING_GENERATOR
(
	//input					OUT_CLK,
	input					REF_CLK,
	input					nRESET,

	input					SET,
	input		[31:0]	DELAY,
	input		[31:0]	WIDTH,
	input		[31:0]	PERIOD,
	input		[31:0]	START,
	input		[31:0]	END,
	
	input		[1:0]		SETTING_EN,
	output	[1:0]		SETTING_DONE,
	
	input					VSYNC,
	input					INVERSION_EN,
	output	reg		LEVEL_SEL
);

reg		[1:0]			REG_SET_DETECT;
wire						WIRE_SET_LOAD;
reg		[1:0]			REG_VSYNC_DETECT;
reg		[31:0]		BUF_DELAY;
reg		[31:0]		BUF_WIDTH;
reg		[31:0]		BUF_PERIOD;
reg		[31:0]		BUF_START;
reg		[31:0]		BUF_END;
reg		[31:0]		REG_DELAY;
reg		[31:0]		REG_WIDTH;
reg		[31:0]		REG_PERIOD;
reg		[31:0]		REG_START;
reg		[31:0]		REG_END;

reg		[31:0]		REG_TIMING_COUNT;
reg						REG_VSYNC_LOAD;

reg						REG_LEVEL_SEL;
reg						REG_INVERSION_EN;
reg		[1:0]			REG_SETTING_DONE;
assign SETTING_DONE = REG_SETTING_DONE;

always @ (posedge REF_CLK) begin
	if(!nRESET) begin
		REG_SET_DETECT			<=			2'b11;
	end
	else begin
		REG_SET_DETECT[0] <= SET;
		REG_SET_DETECT[1] <= REG_SET_DETECT[0];
	end
end

assign	WIRE_SET_LOAD = (REG_SET_DETECT[0]) & (!REG_SET_DETECT[1]);

always @ (posedge WIRE_SET_LOAD or negedge nRESET) begin
	if(!nRESET) begin
		BUF_DELAY <= 32'd0;
		BUF_WIDTH <= 32'd0;
		BUF_PERIOD <= 32'd0;
		BUF_START <= 32'd0;
		BUF_END <= 32'd0;
	end
	else begin
		BUF_DELAY <= DELAY;
		BUF_WIDTH <= WIDTH;
		BUF_PERIOD <= PERIOD;
		BUF_START <= START;
		BUF_END <= END;
	end
end

always @ (negedge REF_CLK) begin
	if(!nRESET) begin
		REG_VSYNC_DETECT		<=			2'b11;
	end
	else begin
		REG_VSYNC_DETECT[0] <= VSYNC;
		REG_VSYNC_DETECT[1] <= REG_VSYNC_DETECT[0];
		REG_VSYNC_LOAD <= ((!REG_VSYNC_DETECT[1]) & (REG_VSYNC_DETECT[0]));
	end
end
/*
always @ (posedge REG_VSYNC_LOAD) begin
	if(!nRESET) begin
		REG_DELAY				<=			32'd0;
		REG_WIDTH				<=			32'd0;
		REG_PERIOD				<=			32'd0;
		REG_START				<=			32'd0;
		REG_END					<=			32'd0;
	end
	else begin
		REG_DELAY <= BUF_DELAY;
		REG_WIDTH <= BUF_WIDTH;
		REG_PERIOD <= BUF_PERIOD;
		REG_START <= BUF_START;
		REG_END <= BUF_END;
	end
end
*/
reg REG_END_FLAG;
reg	[31:0] REG_END_COUNT;

always @ (posedge REF_CLK) begin
	if(!nRESET) begin
		REG_END_FLAG <= 1'b0;
		REG_END_COUNT <= 32'd0;
	end
	else begin
		if (REG_VSYNC_LOAD) begin
			REG_END_COUNT <= 32'd0;
			REG_END_FLAG <= 1'd0;
		end
		else begin
			if(REG_END > 32'd0) begin
				if((REG_END > 32'd0) && (REG_END_COUNT < (REG_END - 32'd1))) begin
					REG_END_COUNT <= REG_END_COUNT + 32'd1;
					REG_END_FLAG <= 1'd0;
				end
				else begin
					REG_END_FLAG <= 1'd1;
				end
			end
			else begin
				REG_END_COUNT <= 32'd0;
				REG_END_FLAG <= 1'd1;
			end
		end
	end
end

always @ (posedge REF_CLK) begin
	if(!nRESET) begin
		REG_TIMING_COUNT	<= 32'd0;
		REG_DELAY			<=	32'd0;
		REG_WIDTH			<=	32'd0;
		REG_PERIOD			<=	32'd0;
		REG_START			<=	32'd0;
		REG_END				<=	32'd0;
		REG_INVERSION_EN  <= 1'd0;
		REG_SETTING_DONE	<= 2'b00;
	end
	else begin
		if (REG_VSYNC_LOAD) begin
			REG_TIMING_COUNT <= 32'd0;
			if(SETTING_EN[0]) begin
				REG_INVERSION_EN <= INVERSION_EN;
				REG_SETTING_DONE[0] <= 1'd1;
			end
			else REG_SETTING_DONE[0] <= 1'd0;
			if(SETTING_EN[1]) begin
				REG_DELAY <= BUF_DELAY;
				REG_WIDTH <= BUF_WIDTH;
				REG_PERIOD <= BUF_PERIOD;
				REG_START <= BUF_START;
				REG_END <= BUF_END;
				REG_SETTING_DONE[1] <= 1'd1;
			end
			else REG_SETTING_DONE[1] <= 1'd0;
		end
		else begin
			if((REG_PERIOD > 32'd0) && (REG_TIMING_COUNT < (REG_START + REG_PERIOD - 32'd1))) begin
				REG_TIMING_COUNT <= REG_TIMING_COUNT + 32'd1;
			end
			else REG_TIMING_COUNT <= REG_START;
		end
	end
end

always @ (negedge REF_CLK) begin
	if(!nRESET) REG_LEVEL_SEL <= 1'b0;
	else begin
		if			((REG_START > 32'd0) && (REG_TIMING_COUNT < (REG_START))) REG_LEVEL_SEL <= 1'b0;
		else if	((REG_DELAY > 32'd0) && (REG_TIMING_COUNT < (REG_START + REG_DELAY))) REG_LEVEL_SEL <= 1'b0;
		else if	((REG_WIDTH > 32'd0) && (REG_TIMING_COUNT < (REG_START + REG_DELAY + REG_WIDTH))) REG_LEVEL_SEL <= 1'b1;
		else REG_LEVEL_SEL <= 1'b0;
	end
end

always @ (posedge REF_CLK) begin
	if(!nRESET) begin
		LEVEL_SEL <= 1'd0;
	end
	else begin
		if(REG_INVERSION_EN) begin
			if(REG_END_FLAG) LEVEL_SEL <= 1'b1;
			else LEVEL_SEL <= !REG_LEVEL_SEL;
		end
		else begin
			if(REG_END_FLAG) LEVEL_SEL <= 1'b0;
			else LEVEL_SEL <= REG_LEVEL_SEL;
		end
	end
end

endmodule
