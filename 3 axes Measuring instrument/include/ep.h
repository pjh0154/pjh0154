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
	double user_p_offset;		
	double user_n_offset;			
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
#endif