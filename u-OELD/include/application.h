#ifndef application_HEADER_H
#define application_HEADER_H

#include "global.h"
#include "fpga_reg.h"
#include "ep.h"
#include "tcp_server.h"
#include "serial-dev.h"
#include "dac.h"
#include "gpio.h"
#include "adc.h"
#include "ocp.h"

///////////////////////////////////////////////////////////////
int getch(void);
void dac_power_off(void);
void model_recipe_read(void);
//void recipe_funtion_load(void);
void recipe_funtion_load(char* func);
void recipe_key_load(char* func);
void power_off(void);
void i2c_com_bist_pattern(char *index, int data, char mode);
void i2c_com_otp_loading(void);
void i2c_com_gma_block(unsigned short reg_address_ex, unsigned short* data);
void i2c_logic_defalut(void);
void relay_init(void);
void mux_init(void);
void i2c_frequency_set(char index);
#endif