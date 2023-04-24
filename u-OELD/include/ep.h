#ifndef ep
#define ep

#define MAX_PKT_SIZE	4096
#define TCP_QSIZE		150
#define UDP_QSIZE		1024
#define CMD_QSIZE		100
#define CMD_SIZE		260

#define IDLE 		0
#define BUSY 		1

#define REQ_FILE_RCV				0x001a

#define FOLDER_NAME_SIZE	256

//#define Tprintf		 if(TPRF) printf
//#define Aprintf		 if(APRF) printf

#define TEST				0x0001

#define ADC_0_CAL_FILE_PATH "/run/media/mmcblk0p2/f0/config/adc0_cal.info"
#define ADC_1_CAL_FILE_PATH "/run/media/mmcblk0p2/f0/config/adc1_cal.info"
#define ADC_2_CAL_FILE_PATH "/run/media/mmcblk0p2/f0/config/adc2_cal.info"
#define ADC_3_CAL_FILE_PATH "/run/media/mmcblk0p2/f0/config/adc3_cal.info"
#define ADC_4_CAL_FILE_PATH "/run/media/mmcblk0p2/f0/config/adc4_cal.info"
#define TEST_FILE_PATH "/run/media/mmcblk0p2/f0/config/cal_test.txt"

#define LD_1D_VALUE_FILE_PATH "/run/media/mmcblk0p2/f0/bin/ld_1d_value_file.txt" 
#define LD_2D_VALUE_FILE_PATH "/run/media/mmcblk0p2/f0/bin/ld_2d_value_file.txt" 

#define Z_FILE_PATH "/run/media/mmcblk0p2/f0/config/z_file.txt"
#define X_FILE_PATH "/run/media/mmcblk0p2/f0/config/x_file.txt"
#define Y_FILE_PATH "/run/media/mmcblk0p2/f0/config/y_file.txt"


#define LD_ON_OFF 0x0001
#define LD_ON_OFF_ACK 0x0002
#define ADC_READ_COUNT_TRIGGER_COUNT 0x0003
#define ADC_READ_COUNT_TRIGGER_COUNT_ACK 0x0004
#define ADC_SENSING_START 0x0005
#define ADC_SENSING_END 0x0006
#define REBOOT_REQUEST 0x0007
#define FORMULA_CALCULATION_INIT 0x0009
#define VERSION_REQUEST 0x000b
#define VERSION_REQUEST_ACK 0x000c
#define LD_CURRENT_CTRL 0x000d
#define	LD_CURRENT_VALUE_SAVE 0x000f
#define	LD_STATE_REQUEST 0x0011
#define LD_STATE_ACK 0x0012

#define DATA_REQUEST 0x00aa
#define DATA_REQUEST_ACK 0x00bb
#define DATA_SEND 0x00FF

#define ACK_OK 0x0001
#define ACK_NG 0x0000

#define SEND_DATA_SIZE 507
#define SEND_TOTAL_DATA_SIZE 1024
//#define MAX_SENSING_DATA_CNT 681 //4086(data size) / 6(x(2byte)+y(2byte)+z(2byte)) = 681
//#define MAX_PACKET_DATA_SIZE 4086

//#define MAX_SENSING_DATA_CNT 170 // 4080(data size) / 24(x(8byte)+y(8byte)+z(8byte)) = 170
#define MAX_SENSING_DATA_CNT 102 // 4080(data size) / 40(x(8byte)+y(8byte)+z(8byte)+z_sum(8byte)+xy_sum(8byte)) = 102
#define MAX_PACKET_DATA_SIZE 4080
//#define	MAX_SENSING_DATA_FORMULA_CALCULATION_CNT 170 // 4080(data size) / 24(x(8byte)+y(8byte)+z(8byte)) = 170
#define	MAX_SENSING_DATA_FORMULA_CALCULATION_CNT 102 // 4080(data size) / 40(x(8byte)+y(8byte)+z(8byte)+z_sum(8byte)+xy_sum(8byte)) = 102
#define	MAX_SENSING_DATA_FORMULA_CALCULATION_SIZE 4080

#define TOTAL_DATA_SIZE 40

#define GPIO_0 0x01
#define GPIO_1 0x02
#define GPIO_2 0x04

#define FIXED_COUNT 120	// 22.11.17

#pragma pack(1)
typedef struct {
	short data_0[15000];
	short data_1[15000];
	short data_2[15000];
}ADC; 
ADC adc;

typedef struct {
	double z;
	double x;
	double y;
	//double z_sum;
	//double xy_sum;		
}ADC_FORMULA_CALCULATION_EXT; 
ADC_FORMULA_CALCULATION_EXT adc__formula_calculation_ext[200];
ADC_FORMULA_CALCULATION_EXT adc_formula[1];

typedef struct {
	double z;
	double x;
	double y;
	double z_sum;
	double xy_sum;		
}ADC_EXT; 
ADC_EXT adc_ext[200];

typedef struct {
	double z_sum;
	double xy_sum;		
}Z_XY_SUM; 
Z_XY_SUM z_xy_sum[200];

typedef struct {
	char myip[16];
	char serverip[16];
	char gateway[16];
	char netmask[16];
	char mac[18];
} NET_CONFIG;

/*typedef struct {
	int *port_id;
	int sig_id;
	unsigned short len;
	unsigned char info[MAX_PKT_SIZE];
} TCP_QUEUE;*/

typedef struct {
	int *port_id;
	unsigned short sig_id;
	unsigned short len;
	unsigned short info[MAX_PKT_SIZE];
} TCP_QUEUE;

typedef struct {
	unsigned short sig_id;
	unsigned short len;
} TCP_PACKET;

/*typedef struct {
	unsigned short sig_id;
	unsigned short len;
	unsigned char info[MAX_PKT_SIZE];
} TCP_SEND_PACKET;*/

typedef struct {
	unsigned short sig_id;
	unsigned short len;
	short info[MAX_PKT_SIZE];
} TCP_SEND_PACKET;


typedef struct {
	unsigned short sig_id;
	unsigned short len;
	unsigned short packet_count;
	unsigned short packet_index;
	short info[2044];
} TCP_SEND_DATA_EXT;

typedef struct {
	unsigned short sig_id;
	unsigned short len;
	unsigned short packet_count;
	unsigned short packet_index;
	double info[511];
} TCP_SEND_DATA_FORMULA_EXT;

typedef struct {
	unsigned short sig_id;
	unsigned short len;
	unsigned int data;
} TCP_VER_ACK_SEND;

typedef struct {
	unsigned short sig_id;
	unsigned short len;
	unsigned short packet_count;
	unsigned short packet_index;
	short info[SEND_DATA_SIZE];
	unsigned short checksum;
} TCP_SEND_DATA_0;

typedef struct {
	unsigned short sig_id;
	unsigned short len;
	unsigned short packet_count;
	unsigned short packet_index;
	short info[SEND_DATA_SIZE];
	unsigned short checksum;
} TCP_SEND_DATA_1;

typedef struct {
	unsigned short sig_id;
	unsigned short len;
	unsigned short packet_count;
	unsigned short packet_index;
	short info[SEND_DATA_SIZE];
	unsigned short checksum;
} TCP_SEND_DATA_2;

typedef struct {
	unsigned short sig_id;
	unsigned short len;
	unsigned short data;
	unsigned int data_size;
} TCP_ACK_SEND;

typedef struct{
	double adc_0v_value;
	double adc_10v_value;	
	double adc_n10v_value;
	double adc_0v_step;
	double adc_10v_step;
	double adc_n10v_step;
	double adc_p_ratio;
	double adc_p_offset;
	double adc_n_ratio;
	double adc_n_offset;			
}ADC_CAL;
ADC_CAL adc0_cal;
ADC_CAL adc1_cal;
ADC_CAL adc2_cal;
ADC_CAL adc3_cal;
ADC_CAL adc4_cal;

typedef struct{
	unsigned short ld_on_off_state;
	unsigned char ld_id;
	unsigned char ld_value;		
}LD_DATA;
LD_DATA ld_1d_data;
LD_DATA ld_2d_data;

typedef struct{
	unsigned short read;
	unsigned short trigger;		
}COUNT_DATA;
COUNT_DATA count_data;

#pragma pack(push)
unsigned int read_count;
unsigned int read_count1;
unsigned int read_count2;
unsigned int read_count3;
unsigned int read_count4;

unsigned int real_count_0;
unsigned int real_count_1;
unsigned int real_count_2;
unsigned int real_count_3;
unsigned int real_count_4;

unsigned int adc_trigger_count_0;
unsigned int adc_trigger_count_1;
unsigned int adc_trigger_count_2;
unsigned int adc_trigger_count_3;
unsigned int adc_trigger_count_4;

unsigned int auto_adc_trigger_count_0;
unsigned int auto_adc_trigger_count_1;
unsigned int auto_adc_trigger_count_2;
unsigned int auto_adc_trigger_count_3;
unsigned int auto_adc_trigger_count_4;

unsigned int trigger;

unsigned char adc0_0v_cal_flag;
unsigned char adc0_10v_cal_flag;
unsigned char adc0_n10v_cal_flag;

unsigned char adc1_0v_cal_flag;
unsigned char adc1_10v_cal_flag;
unsigned char adc1_n10v_cal_flag;

unsigned char adc2_0v_cal_flag;
unsigned char adc2_10v_cal_flag;
unsigned char adc2_n10v_cal_flag;

unsigned char adc3_0v_cal_flag;
unsigned char adc3_10v_cal_flag;
unsigned char adc3_n10v_cal_flag;

unsigned char adc4_0v_cal_flag;
unsigned char adc4_10v_cal_flag;
unsigned char adc4_n10v_cal_flag;

unsigned char auto_trigger_start_flag_0;
unsigned char auto_trigger_start_flag_1;
unsigned char auto_trigger_start_flag_2;

unsigned int trigger_end_flag;
unsigned int read_end_flag;
unsigned int pro_flag;
unsigned int read_pro_flag;
unsigned int formula_calculation_flag;
unsigned int adc0_formula_flag;
unsigned int adc1_formula_flag;
unsigned int adc2_formula_flag;

unsigned char real_time_sensing_flag;

unsigned char timer_end_flag;

unsigned char linearty_flag;

int *port_id_copy;
int *port_id_read_copy;
int real_data_all_puls_data_0;
int real_data_all_puls_data_1;
int real_data_all_puls_data_2;
int real_data_average_data_0;
int real_data_average_data_1;
int real_data_average_data_2; 
int real_data_count;
int real_count;
unsigned char gpio_reg;
unsigned int ld_state;

char* z_data;
char* x_data;
char* y_data;

extern double lpf_tau;
extern double lpf_time;
//unsigned char TPRF, APRF;

struct timespec 	func_time;
struct timespec 	end_time;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DAC_CAL_FILE_PATH "/f0/config/dac_cal.info"
#define RECIPE_FILE_PATH "/f0/recipe/recipe.txt"
#define FW_FPGA_FILE_PATH "/f0/fw_fpga/ep970.zip"
#define ADS124_0_CAL_FILE_PATH "/f0/config/ads124_cal0.info"
#define ADS124_1_CAL_FILE_PATH "/f0/config/ads124_cal1.info"
#define ADS124_2_CAL_FILE_PATH "/f0/config/ads124_cal2.info"
#define ADS124_3_CAL_FILE_PATH "/f0/config/ads124_cal3.info"
#define ADS124_4_CAL_FILE_PATH "/f0/config/ads124_cal4.info"
#define AVDD_CUR_CAL_FILE_PATH "/f0/config/avdd_cur_cal.info"
#define ELVSS_CUR_CAL_FILE_PATH "/f0/config/elvss_cur_cal.info"

int dev_0_fd;
int dev_1_fd;
int dev_2_fd;

#define DAC0 0
#define DAC1 1
#define DAC2 2

#define	DAC_nCLR_HIGH 1<<3
#define	DAC_nCLR_LOW 0<<3

#define ADC_Control_START_PIN_HIGH 1<<4
#define ADC_Control_START_PIN_LOW 0<<4
#define ADC_Control_RESET 1<<0
#define ADC_Control_REG_WRITE 1<<1
#define ADC_Control_REG_READ 1<<2
#define ADC_Control_DATA_READ 1<<3
#define ADC_Control_AUTO_READ_ENABLE 1<<8
#define ADC_Control_AUTO_READ_CLEAR 1<<9
#define ADC_STATUS_AUTO_READ_DONE 1<<2


#define OUT_EN_ON 0x03
#define OUT_EN_OFF 0<<0


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
#define OFF 23
#define SEN 24
#define OCP_ON_OFF 25
#define PAT 26
#define OTP 27
#define I2CFRE 28

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

#define SEN_ELVDD 20
#define SEN_ELVSS 25
#define SEN_AVDD 1
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
#define SEN_ELIDD 22
#define SEN_ELISS 32
#define SEN_ELIDD_100mA 27
#define SEN_ELISS_100mA 37


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
#define MAX_CMD_COUNT 100
#define MAX_SENSING_COUNT 40
#define MAX_FUNC_NAME_LENGTH 32
#define MAX_FUNC_COUNT 1024

#define timing_offset 10

#define ADC_CHANNEL_COUNT 40

//#define COM_BUFFER_SIZE 5000000
#define COM_BUFFER_SIZE 4096

//#define ADC_UNIT ((float)0.000000357627911)
#define ADC_UNIT ((float)0.000000298023224)
#define ADC_UNIT_1 ((float)0.000000298023224)
//#define ADC_UNIT_1 ((float)0.0000035836)
//#define offset ((float)0.0241221983)

#define ADC_REG_1K 1000

//#define ADC_REG_OFFSET 0.004 
#define ADC_REG_R_OFFSET 0.020 
#define ADC_REG_L_OFFSET 0.027 

#define CHECKSUM_or_ETX 2

#define BIST_PATTERN0_ADDR 0x028c         //652
#define VREFH_ADDR	0x02c1			//705
#define VREFL_ADDR	0x02c2			//706
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


#define TURN_ON_LEN 0x02
#define TURN_OFF_LEN 0x02
#define NEXT_PATTERN_LEN 0x02
#define PREVIOUS_PATTERN_LEN 0x02
#define RESISTANCE_REQUEST_LEN 0x02
//#define RESISTANCE_REQUEST_ACK_LEN 0x09
#define RESISTANCE_REQUEST_ACK_LEN 0x08
#define SENSING_DATA_REQUEST_LEN 0x02
#define PATTERN_R_G_B_REQUEST_LEN 0x02
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
#define I2C_READ_ACK_LEN 0x07
#define RGB_VOL_SET_REQUEST_LEN 0x03

#define RECIPE_DOWNLOAD_SIZE 1024
#define RECIPE_DOWNLOAD_USER_DATA_SIZE 1025

#define FW_FPGA_DOWNLOAD_SIZE 1024
#define FW_FPGA_DOWNLOAD_USER_DATA_SIZE 1025

#define PG_ON 1
#define PG_OFF 0

#define _0 0x00
#define TOP 0x01
#define BOT 0x02
#define PRE 0x03

#define VREFH 0x00
#define VREFL 0x01

#define _0_ADDR 0x0206
#define TOP_ADDR 0x020A
#define BOT_ADDR 0x020E
#define PRE_ADDR 0x0212

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


#define reg_value_size 2

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
	unsigned char onid[18];
	short dc_voltage[20];
	unsigned short sequence_timing[18];
	unsigned int dc_use_ch;
	unsigned short resistance_determination_ref_value[2];   
}SIGNAL_CONFIG;

typedef struct{
	unsigned char signal_ver;
	unsigned char signal_count;
	SIGNAL_CONFIG signal_config[10];
}SIGNAL_GROUP;
SIGNAL_GROUP signal_group;

/*typedef struct{
	short dac_0v_value;
	short dac_10v_value;	
	short dac_25v_value;
	short dac_n10v_value;	
	short dac_n25v_value;	
	short dac_0v_step;
	short dac_n0v_step;	
	short dac_10v_step;	
	short dac_25v_step;
	short dac_n10v_step;
	short dac_n25v_step;	
	double dac_0to10_ratio;
	double dac_10to25_ratio;
	double dac_0ton10_ratio;
	double dac_n10ton25_ratio;				
	double dac__0to10v_offset;	
	double dac__10vto25voffset;
	double dac__0ton10_offset;
	double dac__n10to_n25_offset;									
}DAC_CAL;
DAC_CAL dac_cal[15];*/

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
}CUR_CAL;
CUR_CAL avdd_cur_cal;
CUR_CAL elvss_cur_cal;

typedef struct{
	int code;
	char parm[MAX_PARM_COUNT];	
}COMMAND;
COMMAND command;

typedef struct{
	char name[MAX_FUNC_NAME_LENGTH];
	unsigned char cmd_cnt;
	COMMAND cmd[MAX_CMD_COUNT];
}FUNCTION;
FUNCTION function;

typedef struct{
	unsigned int func_cnt;
	FUNCTION  func[MAX_FUNC_COUNT];	
}FUNC_LIST;
FUNC_LIST func_fist;

typedef struct{
	char name[MAX_FUNC_NAME_LENGTH];
	unsigned int cmd_cnt;
	COMMAND cmd[MAX_CMD_COUNT];	
}SKEY;
SKEY skey;

typedef struct{
	unsigned int cmd_cnt;
	COMMAND cmd[MAX_CMD_COUNT];	
}SSYSTEM;
SSYSTEM ssystem;

typedef struct{
	int key_count;
	 SSYSTEM recipe_sys;
	 SKEY recipe_key[MAX_KEY_COUNT];
	 FUNC_LIST recipe_func;
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

#pragma pack(push)

unsigned char ocp_flag_on;
unsigned char pg_on_flag;
//float adc_sensing_value[ADC_CHANNEL_COUNT];
short adc_sensing_value[ADC_CHANNEL_COUNT];
int adc_sensing_value_for_cal[ADC_CHANNEL_COUNT];
double dac_data_backup[14];
char parm_copy[MAX_PARM_COUNT];	
int cur_sensing_value[2];

char rs232_read_flag;

//int adc_res_value_r_1;
//int adc_res_value_l_1;

int adc_res_value_r_1;
int adc_res_value_l_1;

unsigned int each_channel_ocp_value;

unsigned short t_count;
unsigned short h_count;
unsigned short m_count;
unsigned short n_count;
unsigned short zone_select;

#include <sys/time.h>

static inline int msclock(void)
{
	struct timeval tv;
	gettimeofday (&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

#endif
