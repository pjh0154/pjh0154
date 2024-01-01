#ifndef ep
#define ep

struct timespec 	func_time;
struct timespec 	end_time;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "global.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DAC_CAL_FILE_PATH "/f0/config/dac_cal.info"
#define RECIPE_FILE_PATH "/f0/recipe/recipe.txt"
#define TEMPORARY_RECIPE_FILE_PATH "/f0/recipe/temporary_recipe.txt"
#define MODEL_NAME_FILE_PATH "/f0/recipe/model.txt" 
#define FW_FPGA_FILE_PATH "/f0/fw_fpga/ep970.zip"
#define LAST_INDEX_FILE_PATH "/f0/log/_LAST_INDEX[0]_.txt" 
#define ADS124_0_CAL_FILE_PATH "/f0/config/ads124_cal0.info"
#define ADS124_1_CAL_FILE_PATH "/f0/config/ads124_cal1.info"
#define ADS124_2_CAL_FILE_PATH "/f0/config/ads124_cal2.info"
#define ADS124_3_CAL_FILE_PATH "/f0/config/ads124_cal3.info"
#define ADS124_4_CAL_FILE_PATH "/f0/config/ads124_cal4.info"
#define AVDD_CUR_CAL_FILE_PATH "/f0/config/avdd_cur_cal.info"
#define ELVSS_CUR_CAL_FILE_PATH "/f0/config/elvss_cur_cal.info"
#define N_AVDD_CUR_CAL_FILE_PATH "/f0/config/n_avdd_cur_cal.info"
#define N_ELVSS_CUR_CAL_FILE_PATH "/f0/config/n_elvss_cur_cal.info"
#define ADC_OFFSET_FILE_PATH "/f0/config/adc_offset_cal.info"				//231206 Modify

#define ELVSS_CUR_OFFSET_FILE_PATH "/f0/config/elvss_offset_cal.info"
#define AVDD_CUR_OFFSET_FILE_PATH "/f0/config/avdd_offset_cal.info"

#define REGISTER_OFFSET_FILE_PATH "/f0/config/eregister_offset.info"


#define RM_INIT_ERROR_CNT_FILE_PATH "/f0/error/rm_init_error_cnt.txt"		//231117 Modify	
#define RM_INIT_ERROR_READY_FILE_PATH "/f0/error/rm_init_error_ready.txt"	//231117 Modify	

int dev_0_fd;
int dev_1_fd;
int dev_2_fd;

#define RM_ERROR_INDEX 0
#define RM_ERROR_CLEAR_INDEX 1

#define DAC0 0
#define DAC1 1
#define DAC2 2

#define	DAC_nCLR_HIGH 1<<3
#define	DAC_nCLR_LOW 0<<3

#define OCP_CLEAR 1<<0

#define OUT_EN 1<<0

#define ADC_Control_START_PIN_HIGH 1<<4
#define ADC_Control_START_PIN_LOW 0<<4
#define ADC_Control_RESET 1<<0
#define ADC_Control_REG_WRITE 1<<1
#define ADC_Control_REG_READ 1<<2
#define ADC_Control_DATA_READ 1<<3
#define ADC_Control_AUTO_READ_ENABLE 1<<8
#define ADC_Control_AUTO_READ_CLEAR 1<<9
#define ADC_STATUS_AUTO_READ_DONE 1<<2

#define ADC_SEL_PIN_0 7<<5
#define ADC_SEL_PIN_1 6<<5
#define ADC_SEL_PIN_2 5<<5
#define ADC_SEL_PIN_3 4<<5
#define ADC_SEL_PIN_4 3<<5
#define ADC_SEL_PIN_5 2<<5
#define ADC_SEL_PIN_6 1<<5
#define ADC_SEL_PIN_7 0<<5

#define ADC_AIN0 0
#define ADC_AIN1 1
#define ADC_AIN2 2
#define ADC_AIN3 3
#define ADC_AIN4 4

#define OUT_EN_ON 0x03
#define OUT_EN_OFF 0<<0

/////////////////////////////// 레시피 커맨드/////////////////////////////////////////////////////
#define AVDD 0		//E970_R11(0.0.0.2)에서 AVDD로 변경
#define ELVSS 1
#define ELVDD 2		//E970_R11(0.0.0.2)에서 ELVDD로 변경
#define ADD8_S 3
#define ADD8_G 4
#define VGH 5
#define VGL 6
#define VINIT 7
#define APSPARE1 8
#define APSPARE2 9
#define VDD11 10
#define VDD18 11
#define DSPARE1 12
#define DSAPRE2 13
#define LOGIC 14
#define I2C 15
#define RSTB 16
#define TM 17
#define	BISTEN 18
#define LPSPARE1 19
#define DELAY 20
#define I2CDELAY 21
#define OFF 23
#define SEN 24
#define OCP_ON_OFF 25
#define PAT 26
#define OTP 27
#define I2CFRE 28
#define TEMP 29
#define SEND 30
#define PATTERN 31
#define GMA_0	32
#define GMA_TOP 33
#define GMA_BOT 34
#define GMA_PRE 35
#define GMA_VREFH 36
#define GMA_VREFL 37
#define TIME 38
#define CYCLE 39
#define MODEL 40
#define INTERVAL 41
#define EL_OCP 42
#define AC_OCP 43
#define DC_OCP 44
#define LT 45
#define LB 46
#define RT 47
#define RB 48
#define TEMPSEN 49
#define ROLL 50
#define COMPARE 51
#define READ 52
#define ELVSSCUR 53
#define AVDDCUR 54
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define COMD_COUNT 55

#define RELAY_ELVDD 28
#define RELAY_VDD8_S 29
#define RELAY_VDD8_G 30
#define RELAY_VGH 31
#define RELAY_VGL 32
#define RELAY_VINIT 33

#define RELAY_ON 1
#define RELAY_OFF 0

#define ADC_ELVDD 19
#define ADC_ELVSS 14
#define ADC_AVDD 38
#define ADC_ADD8_S 33
#define ADC_ADD8_G 28
#define ADC_VGH 23
#define ADC_VGL 18
#define ADC_VINT 13
#define ADC_APSPARE1 8
#define ADC_APSPARE2 3
#define ADC_VDD11 39
#define ADC_VDD18 34
#define ADC_DPSPARE1 29
#define ADC_DPSAPRE2 24
#define ADC_LOGIC 22
#define ADC_I2C 27
#define ADC_LDO_ELVDD 35 
#define ADC_LDO_OSC 30
#define ADC_LDO_VGH 25
#define ADC_LDO_VGL 20
#define ADC_LDO_VINT 15
#define ADC_VCIR 10
#define ADC_VREF1 5
#define ADC_VREG1 0
#define ADC_VOTP50 36
#define ADC_PM_SPARE1 31
#define ADC_MON1 26
#define ADC_MON2 21
#define ADC_MON3 16
#define ADC_MON4 11
#define ADC_MON5 6
#define ADC_MON6 1
#define ADC_LM_SPARE1 37
#define ADC_LM_SPARE2 32
#define ADC_IVDD 17
#define ADC_IVSS 7
#define ADC_IVDD_100mA 12
#define ADC_IVSS_100mA 2

#define SEN_ELVDD 1	//E970_R11_PCB에서 AVDD로 변경
#define SEN_ELVSS 25
#define SEN_AVDD 20		//E970_R11_PCB에서 ELVDD로 변경
#define SEN_ADD8_S 6
#define SEN_ADD8_G 11
#define SEN_VGH 16
#define SEN_VGL 21
#define SEN_VINT 26
#define SEN_APSPARE1 31 
#define SEN_APSPARE2 36
#define SEN_VDD11 0
#define SEN_VDD18 5
#define SEN_DPSPARE1 10 
#define SEN_DPSAPRE2 15
#define SEN_LOGIC 17
#define SEN_I2C 12
#define SEN_LDO_ELVDD 4 
#define SEN_LDO_OSC 9
#define SEN_LDO_VGH 14
#define SEN_LDO_VGL 19
#define SEN_LDO_VINT 24
#define SEN_VCIR 29
#define SEN_VREF1 34
#define SEN_VREG1 39
#define SEN_VOTP50 3
#define SEN_PM_SPARE1 8 
#define SEN_MON1 13
#define SEN_MON2 18
#define SEN_MON3 23
#define SEN_MON4 28
#define SEN_MON5 33
#define SEN_MON6 38
#define SEN_LM_SPARE1 2 
#define SEN_LM_SPARE2 7 
#define SEN_ELIDD 22
#define SEN_ELISS 32
#define SEN_ELIDD_100mA 27
#define SEN_ELISS_100mA 37

/////////////////////////////////////////COMPARE_COMMAND///////////////////////////////////////////////////////////////
#define COMPARE_AVDD 0
#define COMPARE_ELVSS 1
#define COMPARE_ELVDD 2
#define COMPARE_AVDD8_S 3
#define COMPARE_AVDD8_G 4
#define COMPARE_VGH 5
#define COMPARE_VGL 6
#define COMPARE_VINT 7
#define COMPARE_ASPARE1 8
#define COMPARE_ASPARE2 9
#define COMPARE_VDD11 10
#define COMPARE_VDD18 11
#define COMPARE_DSPARE1 12
#define COMPARE_DSPARE2 13
#define COMPARE_LDOELVDD 14
#define COMPARE_LDOOSC 15
#define COMPARE_LDOVGH 16
#define COMPARE_LDOVGL 17
#define COMPARE_LDOVINT 18
#define COMPARE_VCIR 19
#define COMPARE_VREF1 20
#define COMPARE_VREG1 21
#define COMPARE_VOTP50 22
#define COMPARE_LSPARE 23
#define COMPARE_AVDDCUR 24
#define COMPARE_ELVSSCUR 25
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DAC0_ADDR 20
#define DAC1_ADDR 21
#define DAC2_ADDR 22
#define DAC3_ADDR 23
#define DAC4_ADDR 24
#define DAC5_ADDR 25
#define DAC6_ADDR 26
#define DAC7_ADDR 27

#define MAX_CMD_LIST_COUNT 128
#define MAX_KEY_COUNT 128
#define MAX_PARM_COUNT 128
//#define MAX_CMD_COUNT 1024
#define MAX_CMD_COUNT 512		//230929 Modify	
#define MIN_CMD_COUNT 64		//231027 Modify	
#define MAX_SENSING_COUNT 40
#define MAX_FUNC_NAME_LENGTH 32
//#define MAX_FUNC_COUNT 1024
#define MAX_FUNC_COUNT 256		//230929 Modify	
#define LCD_MAX_LEN 16
#define PATTERN_NAME_LEN 16		//230929 Modify
#define MODEL_NAME_LEN 50		//230929 Modify
#define POWER_SET_MAX_LEN 10	//230929 Modify
#define EX_RS232_MAX_LEN 20		//230929 Modify
#define I2C_MAN_LEN 20			//230929 Modify

#define MIN_DATA_LEN 32
#define MID_DATA_LEN 64
#define MAX_DATA_LEN 128

#define LCD_SEND_BUF 19		//230929 Modify

#define timing_offset 10

#define ADC_CHANNEL_COUNT 40
#define COMPARE_SET_COUNT 26
#define TEMP_SET_COUNT 4

#define bist_val 80

//#define COM_BUFFER_SIZE 5000000
#define COM_BUFFER_SIZE 4096

//#define ADC_UNIT ((float)0.000000357627911)
#define ADC_UNIT ((float)0.000000298023224)
#define ADC_UNIT_1 ((float)0.000000298023224)
//#define ADC_UNIT_1 ((float)0.0000035836)
//#define offset ((float)0.0241221983)

#define ADC_REG_1K 1000

//#define USER_DELAY 5000

//#define ADC_REG_OFFSET 0.004 
#define ADC_REG_R_OFFSET 0.020 
#define ADC_REG_L_OFFSET 0.027 

#define ADC_100mA_VALUE	0x77a78c

#define CHECKSUM_or_ETX 2

//#define i2c_wr_addr 0xa0
//#define i2c_rd_addr 0xa1

#define i2c_wr_addr 0xa2
#define i2c_rd_addr 0xa3

#define i2c_test_wr_addr 0xa2
#define i2c_test_rd_addr 0xa3

#define BIST_PATTERN0_ADDR 0x028c         //652
#define VREFH_ADDR	0x01FE			//710
#define VREFL_ADDR	0x01FF			//711
#define OTP_LOADING_CMD 0x0020
#define FULL_COLOR_MODE1 0x00 & 0x0f

#define TURN_ON 0x81
#define TURN_OFF 0x82
#define NEXT_PATTERN 0x83
#define PREVIOUS_PATTERN 0x84
#define PATTERN_SELECT 0x85
#define RECIPE_DOWNLOAD_START 0x8b
#define RESISTANCE_REQUEST 0x90
#define RESISTANCE_REQUEST_ACK 0x91
#define SENSING_DATA_REQUEST 0x92
#define SENSING_DATA_REQUEST_ACK 0x93
#define PATTERN_R_G_B_REQUEST 0x94
#define ZONE_SELECT 0x99 
#define FW_FPGA_DOWNLOAD_START 0xd0
#define OCP_STATUS_ACK 0x8f
#define GMA_BLOCK_REQUEST 0x97
#define VREFH_VREFL_REQUEST 0x98
#define RGB_VOL_SET_REQUEST 0x96
#define FW_FPGA_VER_REQUEST 0xd1
#define RESET_REQUEST 0x55
#define I2C_WRITE_REQUEST 0x21
#define I2C_READ_REQUEST 0x22
#define I2C_READ_ACK 0x23
#define REGISTER_OFFSET_WRITE 0x24
#define REGISTER_OFFSET_SAVE 0x25
#define SHORT_CHECK_REQUEST 0x26
#define AVDD_ELVSS_VOLTAGE_REQUEST 0x20
#define AVDD_ELVSS_CURRENT_REQUEST 0x27
#define VOLTAGE_VARIABLE_REQUEST 0x28
#define BIST_PATTERN_SHTTING_REQUEST 0x29
#define AVDD_ELVSS_4BYTE_CURRENT_REQUEST 0x30
#define ELVSS_4BYTE_CURRENT_REQUEST 0x31
#define SENSING_DATA_CUR_4BYTE_REQUEST 0x32
#define TEMPERATURE_SENSING_REQUEST 0x33
#define LDO_REGISTER_RAED_REQUEST 0x34
#define LDO_REGISTER_WRITE_REQUEST 0x35
#define I2C_TEST_REQUEST 0x36
#define BIST_PATTERN_DATA_READ_REQUEST 0x37
#define ERROR_DATA_REQUEST 0x40
#define AVDD_ELVSS_CUR_OFFSET_REQUEST 0x51
#define AVDD_ELVSS_CUR_OFFSET_SAVE_REQUEST 0x52
#define ERROR_DATA_RESET_REQUEST 0x44	//230929 Modify
#define CURRENT_MODEL_INDEX_REQUEST 0x41	//230929 Modify	
#define REBOOT_ALARM 0x42				//230929 Modify
#define PROBE_BLOCK_CONTACT_CHECK_REQUEST 0x43	//230929 Modify
#define RM_ADC_INIT_NG_ALARM 0x45	//231117 Modify


#define TURN_ON_LEN 0x02
#define TURN_OFF_LEN 0x02
#define NEXT_PATTERN_LEN 0x02
#define PREVIOUS_PATTERN_LEN 0x02
#define RESISTANCE_REQUEST_LEN 0x02
//#define RESISTANCE_REQUEST_ACK_LEN 0x09
#define RESISTANCE_REQUEST_ACK_LEN 0x08
#define SENSING_DATA_REQUEST_LEN 0x02
#define PATTERN_R_G_B_REQUEST_LEN 0x02
#define PATTERN_R_G_B_REQUEST__AC_LEN 0x20
//#define SENSING_DATA_REQUEST_ACK_LEN 0x51
#define SENSING_DATA_REQUEST_ACK_LEN 0x50
#define ZONE_SELECT_REQUEST_LEN 0x02
#define RECIPE_DOWNLOAD_START_LEN 0x05
#define RECIPE_DOWNLOAD_LEN 0x00
#define FW_FPGA_DOWNLOAD_START_LEN 0x05
#define FW_FPGA_DOWNLOAD_LEN 0x00
#define OCP_LEN 0x02
#define GMA_BLOCK_REQUEST_LEN 0x08
#define VREFH_VREFL_REQUEST_LEN 0x03
#define FW_FPGA_VER_REQUEST_LEN 0x02
#define RESET_REQUEST_LEN 0x02
#define I2C_READ_REQUEST_LEN 0x04
#define I2C_READ_ACK_LEN 0x0b
#define RGB_VOL_SET_REQUEST_LEN 0x03
#define PATTERN_SELECT_LEN 0x02
#define REGISTER_OFFSET_WRITE_LEN 0x04
#define REGISTER_OFFSET_SAVE_LEN 0x04
#define SHORT_CHECK_REQUEST_LEN 0x04
#define SHORT_CHECK_REQUEST_ACK_LEN 0x01
#define AVDD_ELVSS_VOLTAGE_REQUEST_LEN 0x02
#define AVDD_ELVSS_VOLTAGE_REQUEST_ACK_LEN 0x04
#define AVDD_ELVSS_CURRENT_REQUEST_LEN 0x02
#define AVDD_ELVSS_CURRENT_REQUEST_ACK_LEN 0x04
#define VOLTAGE_VARIABLE_REQUEST_LEN 0x03
#define BIST_PATTERN_SHTTING_REQUEST_LEN 0x03
#define AVDD_ELVSS_4BYTE_CURRENT_REQUEST_LEN 0x02
#define AVDD_ELVSS_4BYTE_CURRENT_REQUEST_ACK_LEN 0x08
#define ELVSS_4BYTE_CURRENT_REQUEST_LEN 0x02
#define ELVSS_4BYTE_CURRENT_REQUEST_ACK_LEN 0x04
#define SENSING_DATA_CUR_4BYTE_REQUEST_LEN 0x02
#define SENSING_DATA_CUR_4BYTE_REQUEST_ACK_LEN 0x54
#define TEMPERATURE_SENSING_REQUEST_LEN 0x02
#define TEMPERATURE_SENSING_REQUEST_ACK_LEN 0x08
#define LDO_REGISTER_RAED_REQUEST_LEN 0x02
#define LDO_REGISTER_RAED_REQUEST_ACK_LEN 0x02
#define LDO_REGISTER_WRITE_REQUEST_LEN 0x03
#define I2C_TEST_REQUEST_LEN 0x02
#define AVDD_ELVSS_CUR_OFFSET_REQUEST_LEN 0x04
#define AVDD_ELVSS_CUR_OFFSET_SAVE_REQUEST_LEN 0x02
#define BIST_PATTERN_DATA_READ_REQUEST_LEN 0x02
#define BIST_PATTERN_DATA_READ_REQUEST_ACK_LEN 0x02
#define ERROR_DATA_REQUEST_LEN 0x02
#define ERROR_DATA_REQUEST_ACK_LEN 0x07
#define CURRENT_MODEL_INDEX_REQUEST_LEN 0x02
#define PROBE_BLOCK_CONTACT_CHECK_REQUEST_LEN 0x02
#define ERROR_DATA_RESET_REQUEST_LEN 0x02	//230929 Modify


#define UNCONTACT_MODE 0x00
#define CONTACT_MODE 0x01

#define RECIPE_DOWNLOAD_SIZE 1024
#define RECIPE_DOWNLOAD_USER_DATA_SIZE 1025

#define FW_FPGA_DOWNLOAD_SIZE 1024
#define FW_FPGA_DOWNLOAD_USER_DATA_SIZE 1025

#define CURRENT_INDEX_SIZE 1

#define AUTO_SYSTEM_RUN 4
#define CAL_RUN 3
#define AUTO_RUN 2
#define PG_ON 1
#define PG_OFF 0

#define _0 0x00
#define TOP 0x01
#define BOT 0x02
#define PRE 0x03

#define VREFH 0x00
#define VREFL 0x01

#define LDO_VREG 0x03
#define LDO_VREF 0x04
#define LDO_VGH 0x05
#define LDO_VGL 0x06
#define LDO_VINT 0x07

//-------------------------------I2C ADDRESS-------------------------------------//
#define _0_ADDR 0x0206
#define TOP_ADDR 0x020A
#define BOT_ADDR 0x020E
#define PRE_ADDR 0x0212
#define VREFH_ADDR	0x01FE
#define VREFL_ADDR	0x01FF
#define VGH_ADDR	0x0200
#define VGL_ADDR	0x0201
#define VINT_ADDR	0x0202

#define LT_BPRD_1ST_ADDR 0x025E
#define LB_BPRD_2ND_ADDR 0x025F
#define RT_BPRD_3RD_ADDR 0x0260
#define RB_BPRD_4TH_ADDR 0x0261
//--------------------------------------------------------------------------------//

#define AVDD_POSITIVE 0
#define AVDD_NEGATIVE 1
#define ELVSS_POSITIVE 2
#define ELVSS_NEGATIVE 3

#define EP971_FLASH 0x00
#define CELL 0x01

#define I2C_TEST_OK 0x00
#define I2C_TEST_NG 0x01

#define AVDD_OFFSET_SAVE 0x00
#define ELVSS_OFFSET_SAVE 0x01

#define I2C_RATE_100KHZ 0
#define I2C_RATE_125KHZ 1
#define I2C_RATE_250KHZ 2
#define I2C_RATE_500KHZ 3
#define I2C_RATE_1MHZ 4

#define SPEED_0 26
#define SPEED_1 27

#define MAN_R_G_B_9BIT_10BIT 0x300
#define MAN_GMA_EN 0x80

#define WHITE 0x0000
#define RED 0x0001
#define GREEN 0x0002
#define BLUE 0x0003

#define REG_ADDRESS_518 0
#define REG_ADDRESS_519 1
#define REG_ADDRESS_520 2
#define REG_ADDRESS_521 3
#define REG_ADDRESS_522 4
#define REG_ADDRESS_523 5
#define REG_ADDRESS_524 6
#define REG_ADDRESS_525 7
#define REG_ADDRESS_526 8
#define REG_ADDRESS_527 9
#define REG_ADDRESS_528 10
#define REG_ADDRESS_529 11
#define REG_ADDRESS_530 12
#define REG_ADDRESS_531 13
#define REG_ADDRESS_532 14

#define ADC_TIMEOUT 2000
#define POWER_SET_TIMEOUT 2000

#define RM_ADC_NG 1

#define AUTO_TASK_OK 0x00
#define AUTO_TASK_NG 0x01
#define LDO_CONNECT_ON 0x01
#define LDO_CONNECT_OFF 0x00

//------------------------------------BIST_PATTERN_REG-------------------------------------------//
//#define REG_ADDRESS_652 0
//#define REG_ADDRESS_653 1
//#define REG_ADDRESS_654 2
//#define REG_ADDRESS_655 3
#define REG_ADDRESS_654 0
#define REG_ADDRESS_655 1

//----------------------------------------------------------------------------------------------//

#define ADC_RESULT_OK 1
#define ADC_RESULT_NG 0

//-------------------------------------AUTO_RUN_INDEX--------------------------------------------//
#define AUTO_RUN_START 0
#define AUTO_NEXT_PATTERN 1
#define AUTO_RUN_OFF 2
#define CYCLE_COMPLETE 3 
#define OCP_EVENT 4
#define INTERVAL_START 5
#define INTERVAL_END 6
#define CUR_RECORD	7
#define COMPARE_VOL_CUR_ERROR 8
#define COMPARE_TEMP_ERROR 9
//----------------------------------------------------------------------------------------------//
#define reg_value_size 2
#define MANUAL_MODE 0
#define AUTO_MODE 1

//-------------------------------------INTERVAL_MODE_USE---------------------------------------//
#define INTERVA_START_FLAG 0
#define INTERVAL_END_FLAG 1
//----------------------------------------------------------------------------------------------//

//-------------------------------------TEMPERATURE_STTING_USE---------------------------------------//
#define big_than 1
#define small_than 0
#define COMPARE_LT 0
#define COMPARE_LB 1
#define COMPARE_RT 2
#define COMPARE_RB 3
//--------------------------------------------------------------------------------------------------//

//-------------------------------------ERROR_DATA_USE---------------------------------------//
#define ERROR_NG 1
#define ERROR_OK 0
//--------------------------------------------------------------------------------------------------//

//-------------------------------------OCP_USE---------------------------------------//
#define OCP_ON 1
#define OCP_OFF 0
#define RCB_OCP_MODE_ON 1
#define RCB_OCP_MODE_OFF 0
//--------------------------------------------------------------------------------------------------//
//#define log_index_count 500
#define log_index_count 200
#define CACHMEM_INIT_24HOURS	86400 

//-------------------------------------EX_SERIAL_PORT_USE--------------------------------------------//
#define EX_PORT_STX 0x37a4c295
#define EX_PORT_ETX 0x592c4a0d
#define EX_PORT_BUFFER_SIZE 256
#define EX_PORT_CHECKSUM_CNT 1
#define EX_PORT_OUTPUT_CNT 18
#define EX_PORT_INPUT_CNT 18
#define EX_PORT_TOTAL_CNT 36
#define EX_ADC_SEN 0xb0
#define EX_ADC_SEN_LEN 0x02
#define EX_ADC_DATA_CHECK 0xb1
#define EX_ADC_DATA_CHECK_LEN 0x02 	
//#define EX_ADC_DATA_CHECK_REQUEST_LEN 0x04
#define EX_ADC_DATA_CHECK_REQUEST_LEN 0x01
#define EX_LDO_SELECT 0xb2
#define EX_LDO_SELECT_LEN 0x02 	
#define EX_LDO_TASK_END 0xb3
#define EX_LDO_TASK_END_LEN 0x02 	
#define EX_REG_TASK 0xb4
#define EX_REG_TASK_LEN 0x02 
#define AUTO_CAL_TASK 0xb5
#define AUTO_CAL_TASK_LEN 0x02 
#define AUTO_CAL_ACK_LEN 0x04
#define LDO_ON_FOR_VOL_CHECK 0xb6
#define LDO_ON_FOR_VOL_LEN 0x02 
#define TASK_FOR_VOL_CHECK 0xb7
#define TASK_FOR_VOL_CHECK_LEN 0x02 
#define CAL_END 0xb8
#define CAL_END_LEN 0x02 
#define KEITHLEY_INIT 0xb9
#define KEITHLEY_INIT_LEN 0x02 
#define AUTO_CAL_CUR_TASK 0xba
#define AUTO_CAL_CUR_TASK_LEN 0x03 
#define VOL_CUR_SELECT 0xbb
#define VOL_CUR_SELECT_LEN 0x02

//------------------DAC-----------------------//
#define DAC_DATA_INIT_REQUEST 0xc2
#define DAC_DATA_INIT_REQUEST_LEN 0x02
#define DAC_AUTO_CAL_REQUEST 0xc3
#define DAC_AUTO_CAL_REQUEST_LEN 0x02
#define DAC_AUTO_OFFSET_REQUEST 0xc4
#define DAC_AUTO_OFFSET_REQUEST_LEN 0x02
#define DAC_CHECK_REQUEST 0xc5
#define DAC_CHECK_REQUEST_LEN 0x02
#define DAC_DATA_SAVE_REQUEST 0xc6
#define DAC_DATA_SAVE_REQUEST_LEN 0x02
//------------------ADC----------------------//
#define ADC_DATA_INIT_REQUEST 0xc7
#define ADC_DATA_INIT_REQUEST_LEN 0x02
#define ADC_AUTO_CAL_REQUEST 0xc8
#define ADC_AUTO_CAL_REQUEST_LEN 0x02
#define ADC_AUTO_OFFSET_REQUEST 0xc9
#define ADC_AUTO_OFFSET_REQUEST_LEN 0x02
#define ADC_DATA_SAVE_REQUEST 0xca
#define ADC_DATA_SAVE_REQUEST_LEN 0x02
//#define LDO_ON_OFF_REQUEST 0xcb
//#define LDO_ON_OFF_REQUEST_LEN 0x02
#define CUR_DATA_INIT_REQUEST 0xcc
#define CUR_DATA_INIT_REQUEST_LEN 0x02
#define CUR_AUTO_CAL_REQUEST 0xcd
#define CUR_AUTO_CAL_REQUEST_LEN 0x02
#define CUR_DATA_SAVE_REQUEST 0xce
#define CUR_DATA_SAVE_REQUEST_LEN 0x02
#define CUR_CHECK_REQUEST 0xcf
#define CURCHECK_REQUEST_LEN 0x05
#define CURCHECK_REQUEST_ACK_LEN 0x04
#define RES_SELECT_REQUEST 0xd0
#define RES_SELECT_REQUEST_LEN 0x02
#define RELAY_INIT_REQUEST 0xd1
#define RELAY_INIT_REQUEST_LEN 0x02
//-----------------ETC---------------------//
#define OUTPUT_CHECK_REQUEST 0xc0
#define OUTPUT_CHECK_REQUEST_LEN 0x02
#define OUTPUT_CHECK_REQUEST_ACK_LEN 0x24
#define KEITHLEY_CONNECTION_CHECK_REQUEST 0xc1
#define KEITHLEY_CONNECTION_CHECK_REQUEST_LEN 0x02
//#define MEASUREMENT_RES_REQUEST 0xd2
//#define MEASUREMENT_RES_REQUEST_LEN 0x02
#define KEITHLEY_CONNECTION_REQUEST 0xd3
#define KEITHLEY_CONNECTION_REQUEST_LEN 0x02

#define COMMAND_SETTING_REQUEST 0xa0
#define COMMAND_SETTING_REQUEST_LEN 0x11
#define COMMAND_START_REQUEST 0xa1
#define COMMAND_START_REQUEST_LEN 0x02
#define ES975_STATE_REQUEST 0xa2
#define ES975_STATE_REQUEST_LEN 0x02
#define ES975_STATE_REQUEST_ACK_LEN  0x2a
#define LDO_ON_OFF_REQUEST 0xa3
#define LDO_ON_OFF_REQUEST_LEN 0x02
#define MEASUREMENT_RES_REQUEST 0xa4
#define MEASUREMENT_RES_REQUEST_LEN 0x02
//-----------------------------------------//

 
#define EX_STX_LEN 4
#define EX_ETX_LEN 4
#define EX_LEN_LEN 1
#define EX_OK_ACK 1
#define EX_NG_ACK 2
#define EX_LDO_ELVDD 0
#define EX_LDO_OSC 1
#define EX_LDO_VGH 2
#define EX_LDO_VGL 3
#define EX_LDO_VINT 4
#define EX_VCIR 5
#define EX_VREF1 6
#define EX_VREG1 7
#define EX_VOTP50 8
#define EX_PM_SPARE 9
#define EX_MON1 10
#define EX_MON2 11
#define EX_MON3 12
#define EX_MON4 13
#define EX_MON5 14
#define EX_MON6 15
#define EX_LM_SPARE1 16
#define EX_LM_SPARE2 17

#define EX_AVDD 0		
#define EX_ELVSS 1
#define EX_ELVDD 2		
#define EX_ADD8_S 3
#define EX_ADD8_G 4
#define EX_VGH 5
#define EX_VGL 6
#define EX_VINIT 7
#define EX_APSPARE1 8
#define EX_APSPARE2 9
#define EX_VDD11 10
#define EX_VDD18 11
#define EX_DSPARE1 12
#define EX_DSAPRE2 13
#define EX_RSTB 14
#define EX_TM 15
#define EX_BISTEN 16
#define EX_LPSPARE1 17
#define EX_LDO_ELVDD_2 18

#define EX_CAL_AVDD 0		
#define EX_CAL_ELVSS 1
#define EX_CAL_ELVDD 2		
#define EX_CAL_ADD8_S 3
#define EX_CAL_ADD8_G 4
#define EX_CAL_VGH 5
#define EX_CAL_VGL 6
#define EX_CAL_VINIT 7
#define EX_CAL_APSPARE1 8
#define EX_CAL_APSPARE2 9
#define EX_CAL_VDD11 10
#define EX_CAL_VDD18 11
#define EX_CAL_DSPARE1 12
#define EX_CAL_DSAPRE2 13
#define EX_CAL_LDO_ELVDD 14
#define EX_CAL_LDO_OSC 15
#define EX_CAL_LDO_VGH 16
#define EX_CAL_LDO_VGL 17
#define EX_CAL_LDO_VINT 18
#define EX_CAL_VCIR 19
#define EX_CAL_VREF1 20
#define EX_CAL_VREG1 21
#define EX_CAL_VOTP50 22
#define EX_CAL_PM_SPARE 23
#define EX_CAL_MON1 24
#define EX_CAL_MON2 25
#define EX_CAL_MON3 26
#define EX_CAL_MON4 27
#define EX_CAL_MON5 28
#define EX_CAL_MON6 29
#define EX_CAL_LM_SPARE1 30
#define EX_CAL_LM_SPARE2 31
#define EX_CAL_END 0xff
#define EX_LDO_ALL_ON 0
#define EX_LDO_ALL_OFF 1
#define EX_25V_COMPARE 0
#define EX_15V_COMPARE 1
#define EX_10V_COMAPRE 2
#define EX_5V_COMPARE 3
#define EX_0V_COMPARE 4
#define EX_N5V_COMPARE 5
#define EX_N10V_COMAPRE 6
#define EX_N15V_COMAPRE 7
#define EX_N25V_COMPARE 8

#define AUTO_CAL_NG 1
#define AUTO_CAL_OK 0

#define ADS_INIT_DATA_CNT 18
#define STATUS_ADDR		0x01
#define INPMUX_ADDR		0x02
#define	PGA_ADDR		0x03
#define DATARATE_ADDR	0x04
#define REF_ADDR		0x05
#define IDACMAG_ADDR 	0x06
#define IDACMUX_ADDR 	0x07
#define VBIAS_ADDR		0x08
#define SYS_ADDR		0x09
#define OFCAL0_ADDR		0x0a
#define OFCAL1_ADDR		0x0b
#define OFCAL2_ADDR		0x0c
#define FSCAL0_ADDR		0x0d
#define FSCAL1_ADDR		0x0e
#define FSCAL2_ADDR		0x0f
#define GPIODAT_ADDR 	0x10
#define GPIOCON_ADDR	0x11

#define STATUS_VALUE	0x80
#define INPMUX_VALUE	0x01
#define	PGA_VALUE		0x00
#define DATARATE_VALUE	0x14
#define REF_VALUE		0x10
#define IDACMAG_VALUE 	0x00
#define IDACMUX_VALUE 	0xff
#define VBIAS_VALUE		0x00
#define SYS_VALUE		0x10
#define OFCAL0_VALUE	0x00
#define OFCAL1_VALUE	0x00
#define OFCAL2_VALUE	0x00
#define FSCAL0_VALUE	0x00
#define FSCAL1_VALUE	0x00
#define FSCAL2_VALUE	0x40
#define GPIODAT_AVALUE 	0x00
#define GPIOCON_VALUE	0x00

#define CAL_15V 0
#define CAL_0V 1
#define ADC_CH0 0
#define ADC_CH1 1
#define ADC_CH2 2
#define ADC_CH3 3
#define ADC_CH4 4
//#define ADC_CAL_25V 0
//#define ADC_CAL_20V 1
//#define ADC_CAL_10V 2
//#define ADC_CAL_N10V 3
//#define ADC_CAL_N20V 4
//#define ADC_CAL_N25V 5
#define ADC_CAL_25V 0
#define ADC_CAL_20V 1
#define ADC_CAL_15V 2
#define ADC_CAL_10V 3
#define ADC_CAL_5V 4
#define ADC_CAL_N5V 5
#define ADC_CAL_N10V 6
#define ADC_CAL_N15V 7
#define ADC_CAL_N20V 8
#define ADC_CAL_N25V 9

#define DAC_CAL_25V 0
#define DAC_CAL_10V 1
#define DAC_CAL_0V 2
#define DAC_CAL_N10V 3
#define DAC_CAL_N25V 4
#define DAC_OFFSET_11V_25V 0
#define DAC_OFFSET_0V_10V 1
#define DAC_OFFSET_0V_N10V 2
#define DAC_OFFSET_N11V_N25V 3

#define SELECT_VOL 0
#define SELECT_CUR 1

#define DAC_AUTOCAL_MAX_COUNT 50

#define DAC_AUTOCHECK_CANCEL 0xfe
#define KEITHLEY_COM_ERROR 0xff

#define DAC_CAL_CANCEL 0xfd

#define AVDD_CUR_SELECT 0
#define ELVSS_CUR_SELECT 1

#define REG_OFF_SELECT 0
#define REG_0OHM_SELECT 1
#define REG_10OHM_SELECT 2
#define MEASURING_INIT 0
#define MEASURING_VOL 1
#define MEASURING_CUR 2
#define SELECT_10OHM 0
#define SELECT_25OHM 1
#define SELECT_50OHMM 3
#define SELECT_100OHM 4
#define SELECT_200OHM 2
#define SELECT_400OHM 5
#define SELECT_1KOHM 6
#define SELECT_OPEN 7
#define SELECT_500mA 0
#define SELECT_0A 1

#define EX_UART_RX_TIMEOUT 2000

#define CATEGORY_DAC_CAL 0
#define CATEGORY_ADC_CAL 1
#define CATEGORY_CUR_CAL 2
#define CATEGORY_VOL_CHECK 3
#define CATEGORY_CUR_CHECK 4
#define CATEGORY_RES_SELECT 5
#define CATEGORY_RES_IDEA 6

#define CATEGORY_STATE_IDEL 0
#define CATEGORY_STATE_NG 1
#define CATEGORY_STATE_KEITHLEY_COMMUNICATION_FAIL 2
#define CATEGORY_STATE_EXECUTE 3
#define CATEGORY_STATE_DAC_CAL_RESULT 4
#define CATEGORY_STATE_ADC_CAL_RESULT 5
#define CATEGORY_STATE_CUR_CAL_RESULT 6
#define CATEGORY_STATE_CUR_CHECK_RESULT 7
#define CATEGORY_STATE_VOL_CHECK_RESULT 8
#define CATEGORY_STATE_ALLEND 9

#define DAC_ADC_AUTO_START 0
#define DAC_ADC_AUTO_STOP 1
#define TASK_PROCESSING 1

#define TASK_END 1
#define TASK_IDEA 0

#define SET_VOL_25V 0
#define SET_VOL_15V	1
#define SET_VOL_8V	2
#define SET_VOL_5V	3
#define SET_VOL_2_5V 4
#define SET_VOL_N2_5V 5
#define SET_VOL_N5V	6
#define SET_VOL_N8V	7
#define SET_VOL_N15V 8
#define SET_VOL_N25V 9

#define PRE_DAC_CAL 0
#define AFTER_DAC_CAL 1

#define PRE_ADC_CAL 0
#define AFTER_ADC_CAL 1

#define PRE_ADC_CAL 0
#define AFTER_ADC_CAL 1

#define EX_REVICE_DELAY_START 0
#define EX_REVICE_DELAY_OK 1
#define EX_REVICE_DELAY_NG 2

#define ADC_ALL_VOL_CAL 0xffffffff

//--------------------------------------------------------------------------------------------------//

//--------------------CAL_TASK_USE------------------------------//
#define TASK_OK 0
#define TASK_FAIL 1
#define CAL_PASS 0
#define CAL_NEED 1
#define CAL_CANCEL 2
//--------------------------------------------------------------//
typedef enum {
	NOP=0,
	DEVICEID,
	DAC_STATUS,
	SPICONFIG,
	GENCONFIG,
	BRDCONFIG,
	SYNCCONFIG,
	TOGGCONFIG0,
	TOGGCONFIG1,
	DACPWDWN,
	NC,
	DACRANGE0,
	DACRANGE1,
	TRIGGER,
	BRDCAST
}DAC_REG_ADDRESS;

typedef struct{
	unsigned short device_num;
	unsigned short address;
	unsigned short data;
}DAC_SETTING;

typedef enum {
	FALSE=0,
	TRUE,
}BOOL;

#pragma pack(1)
typedef struct{
	unsigned int el_over_current_value;
	unsigned int ap_over_current_value;
	unsigned int dp_over_current_value;	
	//unsigned char onid[18];
	short dc_voltage[20];
	unsigned short sequence_timing[18];
	unsigned short display_time[100];
	unsigned short display_interval[100];
	unsigned short display_cycle;
	unsigned short pattern_sel[100];

	//unsigned int dc_use_ch;
	//unsigned short resistance_determination_ref_value[2];   
}SIGNAL_CONFIG;

typedef struct{
	unsigned char signal_ver;
	unsigned char signal_count;
	SIGNAL_CONFIG signal_config;
}SIGNAL_GROUP;
SIGNAL_GROUP signal_group;

typedef struct{
	double register_r;
	double register_l;
}REGISTER_OFFSET;
REGISTER_OFFSET register_offset;

typedef struct{
	double dac_0v_value;
	double dac_10v_value;	
	double dac_25v_value;
	double dac_n10v_value;	
	double dac_n25v_value;	
	double dac_0v_step;
	double dac_n0v_step;	
	double dac_10v_step;	
	double dac_25v_step;
	double dac_n10v_step;
	double dac_n25v_step;	
	double dac_0to10_ratio;
	double dac_10to25_ratio;
	double dac_0ton10_ratio;
	double dac_n10ton25_ratio;				
	double dac_0to10_offset;	
	double dac_10to25_offset;
	double dac_0ton10_offset;
	double dac_n10to_n25_offset;									
}DAC_CAL;
DAC_CAL dac_cal[15];

typedef struct{
	double adc_0v_value;
	double adc_15v_value;			
	double adc_n15v_value;
	double adc_0v_step;
	double adc_15v_step;
	double adc_n15v_step;
	double adc_p_ratio;
	double adc_p_offset;
	double adc_n_ratio;
	double adc_n_offset;
	double user_p_offset;		
	double user_n_offset;			
}ADS124_CAL;
ADS124_CAL ads124_cal0;
ADS124_CAL ads124_cal1;
ADS124_CAL ads124_cal2;
ADS124_CAL ads124_cal3;
ADS124_CAL ads124_cal4;
ADS124_CAL adc_cal[32];		//231206 Modify

typedef struct{
	//double user_offset[6];
	double user_offset[10];		
}VOL_OFFSET_CAL;
VOL_OFFSET_CAL vol_offset[32];	//231206 Modify

typedef struct{
	double cur_0a_value;
	double cur_0ma_value;
	double cur_100ma_value;			
	double cur_1a_value;
	double cur_0a_step;
	double cur_0ma_step;
	double cur_100ma_step;
	double cur_1a_step;	
	double cur_100ma_ratio;
	double cur_100ma_offset;
	double cur_1a_ratio;
	double cur_1a_offset;	
	double cur_100ma_user_offset;
	double cur_1a_user_offset;
	double n_cur_1a_user_offset;		
}CUR_CAL;
CUR_CAL avdd_cur_cal;
CUR_CAL elvss_cur_cal;
CUR_CAL n_avdd_cur_cal;
CUR_CAL n_elvss_cur_cal;

typedef struct{		
	double n_offset[25];
	double p_offset[25];										
}CUR_OFFSET_CAL;
CUR_OFFSET_CAL elvss_cur_offset_cal;
CUR_OFFSET_CAL avdd_cur_offset_cal;

typedef struct{
	int code;
	char parm[MAX_PARM_COUNT];	
}COMMAND;
COMMAND command;

typedef struct{
	char name[MAX_FUNC_NAME_LENGTH];
	//unsigned char cmd_cnt;
	unsigned int cmd_cnt;
	COMMAND cmd[MAX_CMD_COUNT];
}FUNCTION;
FUNCTION function;

typedef struct{
	unsigned int func_cnt;
	FUNCTION  func[MAX_FUNC_COUNT];	
}FUNC_LIST;
FUNC_LIST func_fist;

/*typedef struct{
	char name[MAX_FUNC_NAME_LENGTH];
	unsigned int cmd_cnt;
	COMMAND cmd[MAX_CMD_COUNT];	
}SKEY;
SKEY skey;*/

typedef struct{
	unsigned int cmd_cnt;
	COMMAND cmd[MAX_CMD_COUNT];	
}SSYSTEM;
SSYSTEM ssystem;

typedef struct{
	unsigned int cmd_cnt;
	COMMAND cmd[MAX_CMD_COUNT];	
}OTPLIST;
OTPLIST otpcheck;
OTPLIST otptesk;
OTPLIST	recipe_otppower;

typedef struct{
	int key_count;
	 SSYSTEM recipe_sys;
	 //SKEY recipe_key[MAX_KEY_COUNT];
	 FUNC_LIST recipe_func;
	 OTPLIST recipe_otpcheck;
	 OTPLIST recipe_otptesk;
	 OTPLIST recipe_otppower;
}SRECIPE;
SRECIPE srecipe;

/*typedef struct{
	float adc_res_value_r_1k;
	float adc_res_value_l_1k;
	float adc_res_value_r_1;
	float adc_res_value_l_1;
}RES_VALUE;
RES_VALUE reg_value;*/

typedef struct{
    int download_ch;
    unsigned char download_mode;
    unsigned int download_length;
    unsigned int download_packetnumber;
}RS232_RECIPE;
RS232_RECIPE recipe;
RS232_RECIPE fw_fpga;

typedef struct{
    unsigned int hour;
    unsigned int min;
    unsigned int sec;
    unsigned int msec;
}TIME_COUNT;
TIME_COUNT time_count;

typedef struct{
    unsigned char lt_min;
	unsigned char lt_max;
    unsigned char lb_min;
	unsigned char lb_max;
    unsigned char rt_min;
	unsigned char rt_max;
    unsigned char rb_min;
	unsigned char rb_max;
}TEMPERATURE_DATA;
TEMPERATURE_DATA temperature_data;

typedef struct{
    unsigned char i2c_communication;
	unsigned char i2c_read;
	unsigned char avdd_cur;
    unsigned char elvss_cur;
	unsigned char ocp;
	unsigned char vol_cur_adc;
	unsigned char resistance_adc;	
}ERROR_DATA;
ERROR_DATA error_data;

typedef struct{
	unsigned char recipe;
	unsigned char recipe_copy;
	unsigned char pvssig_rs232;
	unsigned char nvssig_rs232;
	unsigned char delay_task;
	unsigned char model_download_file_open;
	unsigned char log_index_file_create;
}SYSTEM_ERROR;
SYSTEM_ERROR system_error;

typedef struct{
	short max_set[COMPARE_SET_COUNT];
	short min_set[COMPARE_SET_COUNT];
}COMOPARE_SET;
COMOPARE_SET compare_set;

typedef struct{
	short max_set[TEMP_SET_COUNT];
	short min_set[TEMP_SET_COUNT];
}TEMP_SET;
TEMP_SET temp_set;

typedef struct{
	unsigned char avdd_elvss_select;
	unsigned char res_select;
}SELECT_SEL;

typedef struct{
	unsigned short dac_cal;
	unsigned int adc_cal;
	unsigned char cur_cal;
	unsigned char vol_check;
	SELECT_SEL cur_check;
	SELECT_SEL res_select;
}CATEGORY;
CATEGORY category;

typedef struct{
	short data_0[10];
	short data_1[10];
}RESULT_DATA;

typedef struct{
	unsigned char state_index;
	unsigned char ch_index;
	RESULT_DATA result_data;
}ES975_STATE;
ES975_STATE es975_state;

typedef struct{
	RESULT_DATA result_data[32];
}TEMP_STORAGE;
TEMP_STORAGE temp_storage;

typedef struct _ring_q_t
{
	pthread_mutex_t mutex;
	char *q; /*q buffer*/
	int nqsize; /*q 버퍼 크기*/
	int ndatasize; /*저장된 데이터 크기*/
	int in;
	int out;
}ring_q_t;
     
#define safe_free(a) if(a) {free(a); a=NULL;}
#define _mlock(x) pthread_mutex_lock(&x->mutex);
#define _munlock(x) pthread_mutex_unlock(&x->mutex);
static void ring_q_deinit(ring_q_t *rq)
{
	if (rq)
	{
		pthread_mutex_destroy(&rq->mutex);
		safe_free(rq->q);
		safe_free(rq);
	}
}

static ring_q_t *ring_q_init(int size)
{
	ring_q_t *rq = (ring_q_t*)malloc(sizeof(ring_q_t));
	if (rq)
	{
		memset(rq, 0, sizeof(ring_q_t));
		pthread_mutex_init(&rq->mutex, NULL);
		rq->nqsize = size;

		rq->q = (char*)malloc(sizeof(char)*size);
		if (!rq->q)
		{
			ring_q_deinit(rq);
			return NULL;
		}

		memset(rq->q, 0, sizeof(char)*size);
	}

	return rq;
}

static void ring_q_reset(ring_q_t *rq)
{
	if (rq)
	{
		_mlock(rq);

		memset(rq->q, 0, sizeof(char)*rq->nqsize);

		rq->ndatasize = 0;
		rq->in = 0;
		rq->out = 0;

		_munlock(rq);
	}
}

static int ring_q_size(ring_q_t *rq)
{
	int size = 0;
	if (rq)
	{
		_mlock(rq);
		size = rq->ndatasize;
		_munlock(rq);

	}

	return size;
}

static int ring_q_push(ring_q_t *rq, char *data, int nsize)
{
	char *src = NULL;
	char *dst = NULL;
	int push_size1 = 0;
	int push_size2 = 0;

	if (!rq || !data) return 0;
	_mlock(rq);

	if (rq->ndatasize + nsize >= rq->nqsize)
	{
		// overflow
		printf("[%s]overflow",__func__);
		_munlock(rq);
		return -1;
	}

	src = data;
	dst = &rq->q[rq->in];
	push_size1 = rq->nqsize - rq->in;

	if (push_size1 > nsize) push_size1 = nsize;

	push_size2 = nsize - push_size1;
	memcpy(dst, src, push_size1*sizeof(char));
	rq->ndatasize += push_size1;

	if (push_size2 > 0)
	{
		int src_offset = push_size1;
		dst = rq->q;
		memcpy(dst, src + src_offset, push_size2*sizeof(char));
		rq->ndatasize += push_size2;
	}

	rq->in = (rq->in + nsize) % rq->nqsize;

	_munlock(rq);
	return nsize;
}

static int ring_q_peek(ring_q_t *rq, char *data, int nsize)
{
	int peek_size1 = 0;
	int peek_size2 = 0;

	if (!rq || !data || nsize <= 0) return 0;

	_mlock(rq);
	if (rq->ndatasize <= 0)
	{
		_munlock(rq);
		return 0;
	}

	if (rq->ndatasize < nsize)
	{
		nsize = rq->ndatasize;
	}

	peek_size1 = rq->nqsize - rq->out;
	if (peek_size1 > nsize)
	{
		peek_size1 = nsize;
	}

	peek_size2 = nsize - peek_size1;

	if (peek_size1 > 0)
	{
		memcpy(data, &rq->q[rq->out], peek_size1*sizeof(char));
	}

	if (peek_size2 > 0)
	{
		memcpy(data + peek_size1, rq->q, peek_size2*sizeof(char));
	}

	_munlock(rq);
	return peek_size1 + peek_size2;
}

static int ring_q_flush(ring_q_t *rq, int nsize)
{
	int i = 0;
	if (!rq || nsize <= 0) return -1;

	_mlock(rq);

	if (nsize > rq->ndatasize)
	{
		nsize = rq->ndatasize;
	}

	while (i<nsize)
	{
		rq->out = (rq->out + 1) % rq->nqsize;
		rq->ndatasize--;
		i++;
	}

	_munlock(rq);

	return nsize;
}

#pragma pack(push)

extern ring_q_t *queue_cal_result;  
unsigned char categoty_state[32];	//231206 Modify


//unsigned char otp_flag_on;	//231101 Modify
unsigned char ocp_flag_on;
unsigned char pg_on_flag;
unsigned char auto_ocp_flag_on;
//float adc_sensing_value[ADC_CHANNEL_COUNT];
float temp_data[TEMP_SET_COUNT];
short adc_sensing_value[ADC_CHANNEL_COUNT];
short compare_adc_sensing_value[ADC_CHANNEL_COUNT];
int adc_sensing_value_for_cal[ADC_CHANNEL_COUNT];
double dac_data_backup[14];
char parm_copy[MAX_PARM_COUNT];	
char pattern_name[PATTERN_NAME_LEN];		//230929 Modify
char model_name[MODEL_NAME_LEN];	//230929 Modify
int cur_sensing_value[4];			//230929 Modify
int model_name_size;			//230929 Modify
char rs232_read_flag;

//int adc_res_value_r_1;
//int adc_res_value_l_1;

int adc_res_value_r_1;
int adc_res_value_l_1;

short avdd_vol_2byte;
short elvss_vol_2byte;

unsigned int each_channel_ocp_value;

unsigned short t_count;
unsigned short h_count;
unsigned short m_count;
unsigned short n_count;

int elvss_4byte_cur_ex;
int elvss_2byte_cur_ex;
double elvss_2byte_offset;
int avdd_4byte_cur_ex;
int avdd_2byte_cur_ex;
double avdd_2byte_offset;
//unsigned int cycle_count;

unsigned short zone_select;

unsigned int USER_DELAY;

float read_data_avg[1000];
float read_data_avg_tt;
float avdd_cur_sensing_data;
float elvss_cur_sensing_data;

unsigned char total_sen_flag;
unsigned char total_sen_cur_4byte_flag;
//unsigned char elvss_cur_sen_flag;
//unsigned char avdd_elvss_cur_sen_flag;
//unsigned char avdd_elvss_sen_flag;
//unsigned char avdd_elvss_cur_2byte_sen_flag;
//unsigned char vrefh_vrefl_sen_flag;
unsigned char cur_sensing_reset_flag;
unsigned char system_load_flag;
unsigned char aging_pat_index;
unsigned char aging_pat_change_flag;
unsigned char aging_pat_interval_change_flag;

unsigned char auto_log_time_start;

unsigned char interval_start_flag;

int aging_pattern_start;

unsigned char aging_result_string[32];   	//231027 Modify
unsigned char file_cnt_string[32];   		//231027 Modify
unsigned int cycle_count;					//231027 Modify

int ex_vol_sensing_task_result[EX_PORT_OUTPUT_CNT];	//231027 Modify

BOOL rcb_mode;

unsigned int rm_init_data[ADS_INIT_DATA_CNT];

unsigned char rm_error_cnt_result; //231117 Modify
unsigned char rm_error_ready_result; //231117 Modify

unsigned char ex_adc_sen_index; //231122 Modify
unsigned char ex_adc_total_result[EX_PORT_TOTAL_CNT]; //231122 Modify
unsigned char ex_input_vol_adc_result[EX_PORT_INPUT_CNT]; //231122 Modify
double auto_cal_data_float;	//231122 Modify
int auto_cal_data;	//231122 Modify
unsigned char vol_cur_select_flag;

unsigned char es975_state_ch;

//unsigned char rcb_mode;
//unsigned char rcb_mode_flag;

//#define rcb_mode_on	1
//#define rcb_mode_off 0

#include <sys/time.h>

static inline int msclock(void)
{
	struct timeval tv;
	gettimeofday (&tv, NULL);
	//return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
	return (tv.tv_sec * 1000000 + tv.tv_usec);
}

static inline int ensis_msclock(void)
{
	struct timeval tv;
	gettimeofday (&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

static inline int timeout_msclock(void)	//231013 Modify
{
	struct timeval tv;
	gettimeofday (&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

#define EXT_DEVICE_NAME "/dev/ttyS3"

#pragma pack(1)

typedef struct _ts_data_t	//231013 Modify
{
       unsigned char ocp_flag_on;
	   unsigned char rcb_mode_ocp_flag_on;
	   unsigned char dac_adc_auto_taske_index;	//231206 Modify
	   unsigned char ex_recive_data_delay_task_index;	//231206 Modify
}ts_data_t;

typedef struct _total_status_t	//231013 Modify
{
       pthread_mutex_t mutex;	   
       ts_data_t *data;
	   
}total_status_t;

#pragma pack()

        #define safe_free(a) if(a) {free(a); a=NULL;}
        #define _mlock(x) pthread_mutex_lock(&x->mutex);
        #define _munlock(x) pthread_mutex_unlock(&x->mutex);

extern total_status_t *total_status;  
static void ts_deinit(total_status_t* ts)	//231013 Modify
{
       pthread_mutex_destroy(&ts->mutex);
       safe_free(ts->data);
       safe_free(ts);
}

static total_status_t* ts_init()	//231013 Modify
{
       total_status_t *ts = (total_status_t*)malloc(sizeof(total_status_t));
       memset(ts, 0, sizeof(total_status_t));
       ts->data = (ts_data_t*)malloc(sizeof(ts_data_t));
       pthread_mutex_init(&ts->mutex, NULL);
       ts_data_t *dst = ts->data;
       memset(&dst->ocp_flag_on, 0, sizeof(ts_data_t));
       return ts;
}

static void ts_ocp_flag_on_set(total_status_t* ts,unsigned char val)	//231013 Modify
{
       ts_data_t *dst = NULL;
       dst=ts->data;
       _mlock(ts);
       dst->ocp_flag_on=val;
       _munlock(ts);
}

static void ts_get(total_status_t* ts,ts_data_t *des)	//231013 Modify
{
       ts_data_t *dst = NULL;
       dst=ts->data;
       _mlock(ts);
       memcpy(des,&dst->ocp_flag_on,sizeof(ts_data_t));
       _munlock(ts);
}

static void ts_ocp_flag_on_get(total_status_t* ts,unsigned char *des)	//231013 Modify
{
       ts_data_t *dst = NULL;
       dst=ts->data;
       _mlock(ts);
       *des=dst->ocp_flag_on;
       _munlock(ts);
}

static void ts_rcb_mode_ocp_flag_on_set(total_status_t* ts,unsigned char val)	//231027 Modify
{
       ts_data_t *dst = NULL;
       dst=ts->data;
       _mlock(ts);
       dst->rcb_mode_ocp_flag_on=val;
       _munlock(ts);
}

static void ts_rcb_mode_ocp_flag_on_get(total_status_t* ts,unsigned char *des)	//231027 Modify
{
       ts_data_t *dst = NULL;
       dst=ts->data;
       _mlock(ts);
       *des=dst->rcb_mode_ocp_flag_on;
       _munlock(ts);
}

static void ts_dac_adc_auto_task_set(total_status_t* ts,unsigned char val)	//231206 Modify
{
       ts_data_t *dst = NULL;
       dst=ts->data;
       _mlock(ts);
       dst->dac_adc_auto_taske_index=val;
       _munlock(ts);
}

static void ts_dac_adc_auto_task_get(total_status_t* ts,unsigned char *des)	//231206 Modify
{
       ts_data_t *dst = NULL;
       dst=ts->data;
       _mlock(ts);
       *des=dst->dac_adc_auto_taske_index;
       _munlock(ts);
}

#endif
