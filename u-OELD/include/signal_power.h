#ifndef __SIGNAL_POWER__
#define __SIGNAL_POWER__

#include <stdbool.h>
#include "global.h"
#include "ep.h"

extern void signal_power_device_init(void);
extern void pvsig_rs232_send(char *tx_buffer, int tx_len);
extern void nvsig_rs232_send(char *tx_buffer, int tx_len);
extern void pvsig_rs232_test(void);
extern void nvsig_rs232_test(void);
extern void signal_power_rs232_rx_task(void);
extern void pvsig_onoff(bool on);
extern void nvsig_onoff(bool on);
extern void pvsig_voltage_set(double voltage);
extern void nvsig_voltage_set(double voltage);

#endif
