#ifndef adc_H
#define adc_H

#include "global.h"
#include "ep.h"

void ads124s08_sensing_init(void);
void ads124s08_ads124s08_rm_init(void);
unsigned short ADC_CRC8(unsigned char *data, int size);
//void ADC_AUTO_DATA_READ(void);
unsigned char ADC_AUTO_DATA_READ(void);
void ADC_AUTO_DATA_READ_FOR_CAL(void);
void ADC_DATA_FOR_CAL_PRINT(void);
void ADC_DATA_PRINT(void);
//void resistance_measurement_1k(void);
void resistance_measurement_1(void);
void ads124_cal_load();
char adc_sen_monitoring(char *cmd, short data);
void register_offset_load();
void ads124s08_short_check_set(int address, int data);
void ADC_SELECT_DATA_READ_AVG(unsigned char ch);
void ADC_AUTO_DATA_READ_FOR_CAL_N(void);
void ads124s08_ads124s08_rm_re_init(void);

#define MUX_SEL_AINCOM 12

#define DATARATE_ADDRESS 4
#define DATARATE_4000SPS 0x3E
#define DATARATE_SINC3_16_6SPS 0x23
#define DATARATE_SINC3_200SPS 0x38

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
}ADC_REG_ADDRESS_NUM;


#endif