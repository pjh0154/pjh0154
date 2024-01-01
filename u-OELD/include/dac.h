#ifndef dac_H
#define dac_H

#include "global.h"
#include "ep.h"
#include "../include/signal_power.h"
#include "../include/spi.h" 
#include "application.h" 
#include <linux/spi/spidev.h>

#define power_vol_28 10
#define power_vol_25 25
#define power_vol_20 40
#define power_vol_15 80
#define power_vol_10 180
#define power_vol_min 255

void Power_Supply_Voltage_load(void);
void Power_Supply_Voltage_load_pjh(short *vol_data);
void dac_init(void);
void dac_set(int dac, uint8_t *write_buf, uint32_t buf_len);
//void dac_Manual_cal0(void);
void dac_Manual_cal(int outch);
void SET_DAC_OUTPUT_VALUE(int outch);
void dac_set_for_logic(void);
void dac_set_for_i2c(void);
//void SET_DAC_OUTPUT_VALUE(void);
//int msclock(void);
void dac_cal_load();
void dac_MTP(int outch);
//void dac_auto_cal(int outch);   //231122 Modify
unsigned char dac_auto_cal(int outch);   //231122 Modify
//void dac_auto_offset_cal(int outch); //231122 Modify
unsigned char dac_auto_offset_cal(int outch); //231122 Modify


#endif