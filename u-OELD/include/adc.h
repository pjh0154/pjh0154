#ifndef adc_H
#define adc_H

#include "global.h"
#include "ep.h"

void ads124s08_sensing_init(void);
void ads124s08_ads124s08_rm_init(void);
unsigned short ADC_CRC8(unsigned char *data, int size);
void ADC_AUTO_DATA_READ(void);
void ADC_AUTO_DATA_READ_FOR_CAL(void);
void ADC_DATA_FOR_CAL_PRINT(void);
void ADC_DATA_PRINT(void);
void resistance_measurement_1k(void);
void resistance_measurement_1(void);
void ads124_cal_load();
char adc_sen_monitoring(char *cmd, short data);
void ads124s08_ads124s08_rm_test(char data);

#define MUX_SEL_AINCOM 12


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