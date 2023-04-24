#pragma once

#include "ep.h"
#include "cantus.h"

#define NUMBER_OF_SIGNAL	36

typedef struct{
	unsigned short status;
	unsigned short control;
	unsigned short vtotal_l;
	unsigned short vtotal_h;
	unsigned short inversion_l;
	unsigned short inversion_h;
	unsigned short set_l;
	unsigned short set_h;
	unsigned short delay_l;
	unsigned short delay_h;
	unsigned short width_l;
	unsigned short width_h;
	unsigned short period_l;
	unsigned short period_h;
	unsigned short start_l;
	unsigned short start_h;
	unsigned short end_l;
	unsigned short end_h;
	unsigned short output_delay_l;
	unsigned short output_delay_h;
}PG_HANDLE;
PG_HANDLE *PG;
#define pg_handle *(unsigned short *)&PG

typedef struct{
	short red;
	short green;
	short blue;
}RGB_HANDLE;
RGB_HANDLE rgb_handle;

#define PG_BASE			(FPGA_BASE_ADDR + 50)

//STATUS//
#define PG_EXTERNAL_INPUT_PATTERN_ON			(1<<0)
#define PG_ANALOG_MUX_ENABLE_ON					(1<<1)
//CONTROL//
#define PG_ANALOG_MUX_ENABLE							(1<<0)
#define PG_DAC_LOAD_OPERATION						(1<<1)
#define PG_SIGNAL_INVERSION_CHANGE				(1<<2)
#define PG_TIMING_CHANGE									(1<<3)
#define PG_INTERRUPT_PIN_LEVEL_CONTROL		(1<<4)
#define PG_SYNC_PIN_LEVEL_CONTROL					(1<<5)
#define PG_FORCED_ON											(1<<6) //Just Use TEST
#define PG_VR_RGB_ON											(1<<7)

unsigned int output_delay;
#define OUTPUT_DELAY_DEFAULT	(unsigned int)238//239 // 1=100ns, 1CLK

void system_init(void);
unsigned short GET_PG_STATUS(void);
BOOL SET_PG_CONTROL(unsigned short *data);
BOOL SET_PG_SIGNAL(void);
BOOL VARIABLE_RGB(void);
