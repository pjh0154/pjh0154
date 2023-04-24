module EP261_EP262
(
	ADDRESS,
	ADC_MISO,
	ADC_MOSI,
	ADC_SCLK,
	ADC_SEL,
	ADC_START,
	ADC_nCS,
	ADC_nDRDY,
	ADC_nRST,
	CLK10M,
	DATA,
	FDAC_CS,
	FDAC_LD,
	FDAC_RST,
	FDAC_SCLK,
	FDAC_SDI,
	FLEVEL_SEL,
	FOUT_nEN,
	FPAT_ON,
	FPGA_nCS,
	FRESET,
	INTERRUPT,
	RX_SYNC,
	TP,
	nRE,
	nWE
);

input	[7:1]	ADDRESS;
input			ADC_MISO;
output			ADC_MOSI;
output			ADC_SCLK;
output	[2:0]	ADC_SEL;
output			ADC_START;
output			ADC_nCS;
input			ADC_nDRDY;
output			ADC_nRST;
input			CLK10M;
inout	[15:0]	DATA;
output	[4:0]	FDAC_CS;
output			FDAC_LD;
output			FDAC_RST;
output	[4:0]	FDAC_SCLK;
output			FDAC_SDI;
output	[23:0]	FLEVEL_SEL;
output			FOUT_nEN;
input			FPAT_ON;
input			FPGA_nCS;
input			FRESET;
output			INTERRUPT;
input			RX_SYNC;
output [4:0]	TP;
input			nRE;
input			nWE;

assign TP[0] = FPAT_ON;

wire						PLL_10MHz;
wire						PLL_100MHz;
wire						PLL_5MHz;

PLL	PLL_inst (
	.inclk0 ( CLK10M ),
	.c0 ( PLL_100MHz ),
	.c1 ( PLL_5MHz ),
	.c2 ( PLL_10MHz )
	);
	
reg	[1:0]		MASTER_10MHz_DETECT_BUF;
always @ (negedge PLL_100MHz) begin
	if(!FRESET) MASTER_10MHz_DETECT_BUF <= 2'b11;
	else MASTER_10MHz_DETECT_BUF <= {MASTER_10MHz_DETECT_BUF[0], RX_SYNC};
end

reg	[31:0]	PHASE_DECTECTOR_COUNT;
reg	MASTER_10MHz_DETECT_FLAG;
always @ (posedge PLL_100MHz) begin
	if(!FRESET) begin
		PHASE_DECTECTOR_COUNT <= 32'd0;
		MASTER_10MHz_DETECT_FLAG <= 1'd0;
	end
	else begin
		if(MASTER_10MHz_DETECT_BUF == 2'b01) begin
			PHASE_DECTECTOR_COUNT <= 32'd0;
			MASTER_10MHz_DETECT_FLAG <= 1'd1;
		end
		else begin
			if(PHASE_DECTECTOR_COUNT >= 32'd100) begin
				MASTER_10MHz_DETECT_FLAG <= 1'd0;
			end
			else begin
				PHASE_DECTECTOR_COUNT <= PHASE_DECTECTOR_COUNT + 32'd1;
			end
		end
	end
end

wire	PG_10MHz;
assign	PG_10MHz = (MASTER_10MHz_DETECT_FLAG) ? RX_SYNC : PLL_10MHz;
	
DAC_HANDLE DAC_HANDLE_inst
(
	.CLK_100MHz(PLL_100MHz) ,	// input  CLK_100MHz_sig
	.nRESET(FRESET) ,	// input  nRESET_sig
	.ADDRESS(ADDRESS) ,	// input [7:1] ADDRESS_sig
	.DATA(DATA) ,	// inout [15:0] DATA_sig
	.nCS(FPGA_nCS) ,	// input  nCS_sig
	.nRE(nRE) ,	// input  nRE_sig
	.nWE(nWE) ,	// input  nWE_sig
	.DAC_REF_CLK(PLL_10MHz) ,	// input  DAC_REF_CLK_sig
	.DAC_SCLK(FDAC_SCLK) ,	// output [4:0] DAC_SCLK_sig
	.DAC_nCS(FDAC_CS) ,	// output [4:0] DAC_nCS_sig
	.DAC_SDI(FDAC_SDI) ,	// output  DAC_SDI_sig
	.DAC_nRST(FDAC_RST) ,	// output  DAC_nRST_sig
	.DAC_nLOAD(FDAC_LD) ,	// output  DAC_nLOAD_sig
	.EXTERNAL_DAC_nLOAD(DAC_LOAD_CONTROL)   // input  EXTERNAL_DAC_nLOAD_sig
);

ADC_HANDLE ADC_HANDLE_inst
(
	.CLK_100MHz(PLL_100MHz) ,	// input  CLK_100MHz_sig
	.nRESET(FRESET) ,	// input  nRESET_sig
	.ADDRESS(ADDRESS) ,	// input [7:1] ADDRESS_sig
	.DATA(DATA) ,	// inout [15:0] DATA_sig
	.nCS(FPGA_nCS) ,	// input  nCS_sig
	.nRE(nRE) ,	// input  nRE_sig
	.nWE(nWE) ,	// input  nWE_sig
	.ADC_REF_CLK(PLL_5MHz) ,	// input  ADC_REF_CLK_sig
	.ADC_SCLK(ADC_SCLK) ,	// output  ADC_SCLK_sig
	.ADC_nCS(ADC_nCS) ,	// output  ADC_nCS_sig
	.ADC_MOSI(ADC_MOSI) ,	// output  ADC_MOSI_sig
	.ADC_MISO(ADC_MISO) ,	// input  ADC_MISO_sig
	.ADC_nDRDY(ADC_nDRDY) ,	// input  ADC_nDRDY_sig
	.ADC_START(ADC_START) ,	// output  ADC_START_sig
	.ADC_nRST(ADC_nRST) ,	// output  ADC_nRST_sig
	.ADC_SEL(ADC_SEL) 	// output [2:0] ADC_SEL_sig
);


wire DAC_LOAD_CONTROL;
PATTERN_GENERATOR_HANDLE PATTERN_GENERATOR_HANDLE_inst
(
	.CLK_100MHz(PLL_100MHz) ,	// input  CLK_100MHz_sig
	.nRESET(FRESET) ,	// input  nRESET_sig
	.ADDRESS(ADDRESS) ,	// input [7:1] ADDRESS_sig
	.DATA(DATA) ,	// inout [15:0] DATA_sig
	.nCS(FPGA_nCS) ,	// input  nCS_sig
	.nRE(nRE) ,	// input  nRE_sig
	.nWE(nWE) ,	// input  nWE_sig
	.REF_CLK(PG_10MHz) ,	// input  REF_CLK_sig
	.SYNC(FPAT_ON) ,	// input  SYNC_sig
	//.SYNC(RX_SYNC) ,	// input  SYNC_sig
	//.PATTERN_ON(FPAT_ON) ,	// input  PATTERN_ON_sig
	.OUTPUT_nON(FOUT_nEN) ,	// output  OUTPUT_nON_sig
	.LEVEL_SEL(FLEVEL_SEL) ,	// output [23:0] LEVEL_SEL_sig
	.INTERRUPT(INTERRUPT) ,	// output  INTERRUPT_sig
	.DAC_LOAD_CONTROL(DAC_LOAD_CONTROL) 	// output  DAC_LOAD_CONTROL_sig
);

endmodule


