#pragma once

void Interrupt_disable(void);
void Interrupt_enable(void);
void swreset(void);
BOOL fpga_loading(void);
void nconfig_low(void);
void nconfig_high(void);
unsigned int nstatus(void);
unsigned int conf_done(void);
void fpga_reset(void);
void red_led(char *str);
