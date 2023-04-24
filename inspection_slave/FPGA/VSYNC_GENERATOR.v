module VSYNC_GENERATOR
(
	input					nRESET,
	input					REF_CLK,
	input					SYNC,
	input		[31:0]	VTOTAL,
	input		[31:0]	DELAY,
	output				VSYNC
);

reg		[31:0]		REG_VCOUNT;
reg		[1:0]			REG_SYNC_BUFFER;
wire						WIRE_VCOUNT_RESET;
reg						REG_VSYNC;

always @ (negedge REF_CLK) begin
	if(!nRESET) begin
		REG_SYNC_BUFFER <= 2'b11;
	end
	else begin
		REG_SYNC_BUFFER[0] <= SYNC;
		REG_SYNC_BUFFER[1] <= REG_SYNC_BUFFER[0];
	end
end

assign WIRE_VCOUNT_RESET = (REG_SYNC_BUFFER == 2'b10) ? 1'b0 : 1'b1;

//always @ (posedge REF_CLK or negedge WIRE_VCOUNT_RESET) begin
always @ (posedge REF_CLK) begin
	if(!WIRE_VCOUNT_RESET) REG_VCOUNT <= 32'd0;
	else if(!nRESET) REG_VCOUNT <= 32'd0;
	else begin
		if(REG_VCOUNT < (VTOTAL - 32'd1)) REG_VCOUNT <= REG_VCOUNT + 32'd1;
		else REG_VCOUNT <= 32'd0;
		
		//if(REG_VCOUNT < 32'd212) REG_VSYNC <= 1'b0;
		//if(REG_VCOUNT < (32'd302 + DELAY)) REG_VSYNC <= 1'b0;
		if(REG_VCOUNT <= DELAY) REG_VSYNC <= 1'b0;
		else REG_VSYNC <= 1'b1;
	end
end

assign VSYNC = REG_VSYNC;

endmodule
