#ifndef application_HEADER_H
#define application_HEADER_H

#include "global.h"
#include "fpga_reg.h"
#include "ep.h"
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
void power_off(void);
void i2c_com_bist_pattern(char *index, int data, char mode);
void i2c_com_otp_loading(void);
void i2c_com_gma_block(unsigned short reg_address_ex, unsigned short* data);
void i2c_logic_defalut(void);
void relay_init(void);
void mux_init(void);
void i2c_frequency_set(char index);
//void bist(void);
void bist_test(unsigned char ch, short vol);
void rgb_voltage_request(void);
void recipe_system_load(void);
void *sensing_task();
void model_name_check(void);
void aging_mode_task(void);
static bool ms_timecheck_array(unsigned int ms);
int auto_log_time(void);
int create_file(void);
void auto_run(void);
void log_task(unsigned char index, unsigned short num); //231027 Modify  
void display_task(char * line_1, char * line_2);
void model_name_write(char * name);
void aging_off_task(void);
void rcb_ack(void);
void log_index_file_create(void);
void log_index_file_change(unsigned int index);
void delete_log_file(void);
void interval_check(unsigned char index);
static bool ms_timecheck_array_2(unsigned int ms);
bool temp_sen_task(void);
void interval_off(void);
void button_led_off(void);
void button_led_on(void);
void bist_pattern_read_data_request(void);
void nvss_cur_offset(void);
void vss_cur_offset_select(void);
void pvss_cur_offset(void);
void nvdd_cur_offset(void);
void pvdd_cur_offset(void);
void vdd_cur_offset_select(void);
void read_command_task(void);
void send_command_task(void);
void elvss_cur_command_task(void);
void avdd_cur_command_task(void);
void recipe_download_task(void);
void file_count(unsigned short cnt);  //231027 Modify
void compare_value_setting_task(int code, char *data);  //231027 Modify
unsigned char compare_vol_cur_task(void);   //231027 Modify
unsigned char compare_sensing_select_task(unsigned char ch);    //231027 Modify
void temp_value_setting_task(int code, char *data);  //231027 Modify 
unsigned char compare_temp_task(void);  //231027 Modify
unsigned char ex_voltage_sensing_task(unsigned char ch);    //231101 Modify
unsigned char ex_voltage__sensing_select_task(unsigned char ch);    //231101 Modify
void ex_vol_sensing_task(void);    //231101 Modify
unsigned int ex_vol_sensing_task_2(void);    //231101 Modify 
unsigned int ads_rm_init_data_compare_task(int cnt, unsigned int data); 	        //231101 Modify 
void ads_rm_init_data_set_task(void);   //231101 Modify  	
//unsigned char rm_error_cnt_write(unsigned char index);  //231117 Modify  	
//unsigned char rm_error_ready_write(unsigned char index);    //231117 Modify  
void rm_adc_init_ng_ack(void);  //231117 Modify 
void ex_vol_set_task(unsigned char index); //231122 Modify 
unsigned char ex_input_vol_sensing_task(unsigned char ch); //231122 Modify
unsigned char ex_input_vol_compare_task(void); 	 //231122 Modify
//void ex_auto_cal_task(void); 	 //231122 Modify 
unsigned char ex_auto_cal_task(void); 	 //231122 Modify 
void auto_cal_apply_task(unsigned index);   //231122 Modify
void ldo_all_on_off_task(unsigned index); //231122 Modify
unsigned char auto_cal_data_compare_task(unsigned char index); //231122 Modify
unsigned char auto_cal_data_compare_sensing_select_task(unsigned char ch); //231122 Modify
void auto_cal_successfully_check_task(void);    //231122 Modify
void ADC_SELECT_DATA_READ_FOR_CAL(unsigned char ch);   //231122 Modify
void keithley_init_task(void); //231122 Modify 	
unsigned char dac_vol_check(int outch); //231122 Modify
void cal_end_task(void);    //231122 Modify
unsigned char vol_cur_select_task(unsigned char index); //231122 Modify
//void auto_cal_cur_task(unsigned char index); //231122 Modify
unsigned char auto_cal_cur_task(unsigned char index); //231122 Modify
void cur_sensing_compare_task(unsigned char index, unsigned char res_index, short avdd_data, short elvss_data); //23112 Modify 	
unsigned char adc_offset_task(unsigned char outch); //231206 Modify
unsigned char adc_auto_offset_select_task(unsigned char ch); //231206 Modify
unsigned char adc_select_offset_select_task(unsigned char ch); //231206 Modify 	
void pg_check_task(void);   //231206 Modify 
void serial_dev_ack_nack_task(unsigned char cmd, unsigned char len, unsigned char data); //231206 Modify
void adc_cal_offset_data_save_task(void); //231206 Modify
void cur_auto_cal_task(void); //231206 Modify
void cur_cal_offset_save_task(void); //231206 Modify
void res_select_task(unsigned char ch, unsigned char sel); //231206 Modify
void measurement_res_select_task(unsigned char index); //231206 Modify
void output_to_keithley_connect_task(unsigned char ch); //231206 Modify
void dac_cal_task(unsigned char count); //231206 Modify
unsigned char adc_cal_task(unsigned char ch);  //231206 Modify 
void category_task(void);    //231206 Modify      
short *cal_dac_vol_save_task(unsigned char index,unsigned char outch);  //231206 Modify  
void result_ring_q_save_task(unsigned char ch, unsigned char index, ES975_STATE ring_q);  //231206 Modify    
void result_ring_q_save_end_task(unsigned char index);  //231206 Modify   
unsigned char ex_recive_data_delay_task(int time); //231206 Modify
unsigned char dac_cal_save_task(void); //231206 Modify
unsigned char pre_dac_cal_vol_check_task(short *vol); //231206 Modify
unsigned char pre_adc_cal_vol_check_task(unsigned char index, unsigned char outch); //231206 Modify
unsigned char cur_cal_task(unsigned char ch);  //231206 Modify   
void cur_cal_check(unsigned char ch);  //231206 Modify  
#endif