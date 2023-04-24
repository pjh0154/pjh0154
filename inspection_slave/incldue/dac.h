#pragma once

#include "ep.h"
#include "cantus.h"

#pragma pack(1)
typedef struct{
	unsigned short status;
	unsigned short control;
	short dac0_out0;
	short dac0_out1;
	short dac0_out2;
	short dac0_out3;
	short dac1_out0;
	short dac1_out1;
	short dac1_out2;
	short dac1_out3;
	short dac2_out0;
	short dac2_out1;
	short dac2_out2;
	short dac2_out3;
	short dac3_out0;
	short dac3_out1;
	short dac3_out2;
	short dac3_out3;
	short dac4_out0;
	short dac4_out1;
	short dac4_out2;
	short dac4_out3;
	unsigned short setting_control;
	unsigned short setting_buffer0;
	unsigned short setting_buffer1;
}DAC_HANDLE;
DAC_HANDLE *DAC;
#define dac_handle *(unsigned short *)&DAC
#pragma pack(4)
#define DAC_BASE			(FPGA_BASE_ADDR + 0)

//STATUS//
#define DAC_READY		(1<<0)
//CONTROL//
#define DAC_LOAD			(1<<2)
#define DAC_RST				(1<<1)
#define DAC_TX_START	(1<<0)
//SETTING//
//TBD

typedef struct{
	unsigned short device_num;
	unsigned short address;
	unsigned short data;
}DAC_SETTING;

/*
typedef struct{
	short max;
	short zero;
	short min;
}CAL_SIGNAL;
*/
typedef struct{
	short high_15V;
	short high_12V;
	short high_9V;
	short high_6V;
	short high_3V;
	short zero;
	short low_3V;
	short low_6V;
	short low_9V;
	short low_12V;
	short low_15V;
}CAL_SIGNAL;

typedef struct{
	CAL_SIGNAL dc[9];
	CAL_SIGNAL ac[4];
}CALIBRATION;
CALIBRATION cal;

unsigned short GET_DAC_STATUS(void);
BOOL SET_DAC_CONTROL(unsigned short *data);
BOOL SET_DAC_SETTING_VALUE(DAC_SETTING *dac_setting);
BOOL SET_DAC_OUTPUT_VALUE(void);
BOOL VARIABLE_RGB(void);
void DAC_CALIBRATION(void);
BOOL CALIBRATION_LOAD(void);
void CALIBRATION_PRINT(void);
