#pragma once

#include "ep.h"
#include "cantus.h"

#pragma pack(1)
typedef struct{
	unsigned short status;
	unsigned short control;
	short dac_out0;
	short dac_out1;
	short dac_out2;
	short dac_out3;
	short dac_out4;
	short dac_out5;
	short dac_out6;
	short dac_out7;	
	unsigned	short dac_select;	
	unsigned short setting_control;
	unsigned short setting_buffer0;
	unsigned short setting_buffer1;
}DAC_HANDLE;
DAC_HANDLE *DAC;
#define dac_handle *(unsigned short *)&DAC
#pragma pack(4)
#define DAC_BASE			(FPGA_BASE_ADDR_1 + 0)
//#define DAC_BASE			(FPGA_BASE_ADDR + 0)
//STATUS//
#define DAC_READY		(1<<0)
//CONTROL//
#define DAC_LOAD			(1<<2)
#define DAC_RST				(1<<1)
#define DAC_TX_START	(1<<0)
//SETTING//
//TBD

#define DAC_0_STAT_WRITE	(1<<0)
#define DAC_1_STAT_WRITE	(1<<1)
#define DAC_2_STAT_WRITE	(1<<2)
#define DAC_3_STAT_WRITE	(1<<3)
#define DAC_4_STAT_WRITE	(1<<4)
#define DAC_5_STAT_WRITE	(1<<5)
#define DAC_6_STAT_WRITE	(1<<6)
#define DAC_7_STAT_WRITE	(1<<7)
#define DAC_8_STAT_WRITE	(1<<8)
#define DAC_9_STAT_WRITE	(1<<9)
#define DAC_10_STAT_WRITE	(1<<10)

typedef enum {
	NOP=0,
	DEVICEID,
	DAC_STATUS,
	SPICONFIG,
	GENCONFIG,
	BRDCONFIG,
	SYNCCONFIG,
	TOGGCONFIG0,
	TOGGCONFIG1,
	DACPWDWN,
	NC,
	DACRANGE0,
	DACRANGE1,
	TRIGGER,
	BRDCAST
}DAC_REG_ADDRESS;

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
BOOL DAC_INIT(void);
void DAC_0V_SETTING(void);
