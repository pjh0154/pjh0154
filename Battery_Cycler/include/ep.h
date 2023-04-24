/*
 * ep.h
 *
 *  Created on: 2019. 4. 25.
 *      Author: ghkim
 */

#ifndef EP_H_
#define EP_H_

#define FW_Version "1.01"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "main.h"
#include "command.h"
#include "tcp_echoserver.h"
#include "can.h"
#include "can_queue.h"
#include "ymodem.h"

typedef struct{
	unsigned short SC_Voltage_1;
	unsigned short SC_Voltage_2;
	unsigned short SC_Voltage_3;
	unsigned short SC_Voltage_4;
	unsigned short SC_Current_1;
	unsigned short SC_Current_2;
	unsigned short SC_Current_3;
	unsigned short SC_Current_4;
	unsigned short Link_Voltage_1;
	unsigned short Link_Voltage_2;
	unsigned short Link_Voltage_3;
	unsigned short Link_Voltage_4;
	unsigned short Link_Current_1;
	unsigned short Link_Current_2;
	unsigned short Link_Current_3;
	unsigned short Link_Current_4;
	unsigned short  Temp_1;
	unsigned short  Temp_2;
	unsigned short  Temp_3;
	unsigned short  Temp_4;
	unsigned char  Firemware;
	unsigned short SOC_1;
	unsigned short SOC_2;
	unsigned short SOC_3;
	unsigned short SOC_4;
	unsigned char  Operation_State_1;
	unsigned char  Operation_State_2;
	unsigned char  Operation_State_3;
	unsigned char  Operation_State_4;
	unsigned char  Fault_State_1;
	unsigned char  Fault_State_2;
	unsigned char  Fault_State_3;
	unsigned char  Fault_State_4;
	unsigned char  Cycle_State_1;
	unsigned char  Cycle_State_2;
	unsigned char  Cycle_State_3;
	unsigned char  Cycle_State_4;
	unsigned char  Step_State_1;
	unsigned char  Step_State_2;
	unsigned char  Step_State_3;
	unsigned char  Step_State_4;
	unsigned char  Mode_ID_1;
	unsigned char  Mode_ID_2;
	unsigned char  Mode_ID_3;
	unsigned char  Mode_ID_4;
	unsigned char  Type_ID_1;
	unsigned char  Type_ID_2;
	unsigned char  Type_ID_3;
	unsigned char  Type_ID_4;
	unsigned char  Run_Stop_1;
	unsigned char  Run_Stop_2;
	unsigned char  Run_Stop_3;
	unsigned char  Run_Stop_4;
	unsigned char  Cycle_SetRepeat;
	unsigned char  step_SetRepeat;
	unsigned short SOC_SetTest;
	unsigned char  step_channel;
	unsigned char  step_step_id;
	unsigned short norminal_voltage;
	unsigned short norminal_capacity;
	unsigned short max_voltage;
	unsigned short min_voltage;
	unsigned short max_current;
	unsigned short min_current;
	unsigned short max_soc;
	unsigned short min_soc;
	unsigned char  step_mode_id;
	unsigned char  step_type_id;
	unsigned short step_voltage;
	unsigned short step_current;
	unsigned short step_time;
	unsigned short step_extra;
	unsigned short step_termination_voltage;
	unsigned short step_termination_current;
	unsigned short step_termination_soc;
	unsigned short step_termination_time;
	unsigned short step_termination_msec;
}BUCK_BOOST;
BUCK_BOOST buck_boost[4];

typedef struct{
	unsigned short ac_r_voltage;
	unsigned short ac_s_voltage;
	unsigned short ac_t_voltage;
	unsigned short ac_r_current;
	unsigned short ac_s_current;
	unsigned short ac_t_current;
	unsigned short ac_frequency_r;
	unsigned short dc_voltage_r;
	unsigned short dc_current;
	unsigned short temperature;
	unsigned short apparent_power;
	unsigned short active_power;
	unsigned short reactive_power;
	unsigned short power_factor;
	unsigned short dc_power;
	/*unsigned char  in_dip_sw1;
	unsigned char  in_dip_sw2;
	unsigned char  in_dip_sw3;
	unsigned char  in_dip_sw4;*/
	unsigned char  out_out_relay;
	unsigned char  out_init_relay;
	unsigned char  out_pwm12cs;
	unsigned char  out_pwm34sc;
	unsigned char  out_pwm56sc;
	unsigned char  out_fault_led;
	unsigned char  out_run_led;
	unsigned char  out_ready_led;
	unsigned char  out_fan;
	unsigned char  flag_ac_ready;
	unsigned char  flag_comm_ok;
	unsigned char  flag_dc_ready;
	unsigned char  flag_dclink_ok;
	unsigned char  flag_pll_ok;
	unsigned char  flag_zerocross;
	unsigned char  flag_run;
	unsigned char  flag_stop;
	unsigned char  flag_fault_stop;
	unsigned char  flag_pwm_on;
	unsigned char  flag_pwm_steady;
	unsigned char  flag_dummy;
	unsigned char  flag_pretest;
	unsigned char  flag_testmode;
	unsigned char  flag_gc_mode;
	unsigned char  ovr;
	unsigned char  ovs;
	unsigned char  ovt;
	unsigned char  uvr;
	unsigned char  uvs;
	unsigned char  uvt;
	unsigned char  ocr;
	unsigned char  ocs;
	unsigned char  oct;
	unsigned char  of;
	unsigned char  uf;
	unsigned char  ocdc;
	unsigned char  ovdc;
	unsigned char  uvdc;
	unsigned char  otsen;
	unsigned char  aciint;
	unsigned char  dcvint;
	unsigned char  igbt1;
	unsigned char  igbt2;
	unsigned char  igbt3;
	unsigned char  pwm_trip;
	unsigned char  temp_sw1;
	unsigned char  temp_sw2;
	unsigned char  fan1_lock;
	unsigned char  fan2_lock;
	unsigned short  test_power;
	unsigned short  pre_test_pwm_duty;
	unsigned short  dc_voltage_w;
	unsigned short  ac_voltage;
	unsigned short  ac_frequency_w;
	unsigned short  pwm_frequency;
	unsigned short  max_active_power;
	unsigned short  max_ac_current;
	unsigned short  max_ac_voltage;
	unsigned short  min_ac_voltage;
	unsigned short  max_ac_frequency;
	unsigned short  min_ac_frequency;
	unsigned short  max_dc_voltage;
	unsigned short  max_dc_current;
	unsigned short  dc_start_voltage;
	unsigned short  dc_stop_voltage;
	unsigned short  fan_start_temp;
	unsigned short  fan_stop_temp;
	unsigned short  step1_v;
	unsigned short  step1_p;
	unsigned short  step2_v;
	unsigned short  step2_p;
	unsigned short  step3_v;
	unsigned short  step3_p;
	unsigned short  step4_v;
	unsigned short  step4_p;
	unsigned short  step5_v;
	unsigned short  step5_p;
	unsigned short  memory_key;
	unsigned char run_stop;
	unsigned short  flag1;
	unsigned short  flag2;
	unsigned short  flag3;
	unsigned short  flag4;
}ACDC;
ACDC acdc;

typedef struct{
	unsigned short fb_lv_v;
	unsigned short fb_lv_i;
	unsigned short fb_hv_v;
	unsigned short fb_hv_i;
	unsigned char fb_fault_1;
	unsigned char fb_fault_2;
	unsigned char fb_fault_3;
	unsigned char fb_fault_4;
	unsigned short fb_test_hv;
	unsigned short fb_test_lv;
	unsigned char fb_buck_charging;
	unsigned char fb_buck_burst;
	unsigned char fb_buck_sr;
	unsigned char fb_discharging;
}PSFB;
PSFB psfb;

#endif /* EP_H_ */
