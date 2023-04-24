#pragma once

#include "ep.h"
#include "cantus.h"
typedef struct{
	float value[56];
}ADC_SENSING_VALUE;
ADC_SENSING_VALUE adc_sensing_value;

typedef struct{
	unsigned short status;
	unsigned short control;
	unsigned short reg_address;
	unsigned short reg_write_data;
	unsigned short reg_read_data;
	unsigned short conversion_data_l;
	unsigned short conversion_data_h;
	unsigned short auto_mux_select;
	unsigned short auto_conversion_data_l;
	unsigned short auto_conversion_data_h;
	unsigned short auto_conversion_delay_l;
	unsigned short auto_conversion_delay_h;
}ADC_HANDLE;
ADC_HANDLE *ADC;
#define adc_handle *(unsigned short *)&ADC

typedef enum {
	ID=0,
	STATUS,
	INPMUX,
	PGA,
	DATARATE,
	REF,
	IDACMAG,
	IDACMUX,
	VBIAS,
	SYS,
	OFCAL0,
	OFCAL1,
	OFCAL2,
	FSCAL0,
	FSCAL1,
	FSCAL2,
	GPIODAT,
	GPIOCON
}ADC_REG_ADDRESS;

#define ADC_BASE			(FPGA_BASE_ADDR + 30)

//STATUS//
#define ADC_READY										(1<<0)
#define ADC_DRDY										(1<<1)
#define ADC_AUTO_READ_DONE					(1<<2)
//CONTROL//
#define ADC_RESET										(1<<0)
#define ADC_REG_WRITE_EN						(1<<1)
#define ADC_REG_READ_EN						(1<<2)
#define ADC_READ_DATA_EN						(1<<3)
#define ADC_START_EN								(1<<4)
#define ADC_SEL_0										(1<<5)
#define ADC_SEL_1										(1<<6)
#define ADC_SEL_2										(1<<7)
#define ADC_AUTO_READ_EN						(1<<8)
#define ADC_AUTO_READ_DONE_CLEAR		(1<<9)

#define ADC_AUTO_5MHz_CLOCK_DELAY	((U32)0)//1 = 10ns

BOOL ADC_INIT(void);
BOOL ADC_REG_READ(ADC_REG_ADDRESS *address, unsigned short *read_data);
BOOL ADC_REG_WRITE(ADC_REG_ADDRESS *address, unsigned short *write_data);
void ADC_REG_PRINT(void);
void ADC_AUTO_DATA_READ(void);
