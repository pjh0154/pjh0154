#include <stdio.h>
#include "../exprtk_library/exprtk.h"
#include "../include/global.h"
#include "../include/tcp_client.h"
#include "../include/serial-dev.h"
#include "../include/pollmanager.h"
#include "../include/can-dev.h"
#include "../include/fpga_reg.h"
#include "../include/application.h"
#include "../include/ep.h"
#include "../include/tcp_server.h"
#include "../include/timer.h"
#include "../include/serial-dev.h"
#include "../include/gpio-dev.h"

#define Cprintf if(cprf) printf
#define Tprintf if(tprf) printf
#define Eprintf if(eprf) printf

int cprf=0;
int tprf=0;
int eprf=0;
int aprf=0;

void system_init(void);

#define DEBUG_BUFFER_SIZE 64
#define PROMPT "[ENSIS]"
int taskSpawn(int priority,int taskfunc);

unsigned char readch(void);
static int peek_character = -1;
static struct termios initial_settings;
static struct termios new_settings;
static int fd = -1;

pthread_mutex_t mutex_lock;
pthread_t TaskID[20];
int TaskCnt = 0;
	
pthread_t	pthread_tcp;
pthread_t	pthread_shell;
pthread_t	pthread_fifo_0_empty;
pthread_t	pthread_fifo_1_empty;
pthread_t	pthread_fifo_2_empty;
int		pthread_end	= 0;
int		set_link	= 0;
bool		connect_flag	= false;

static void set_start_time(struct timespec  *t)
{
    clock_gettime(CLOCK_MONOTONIC, t);
}

static uint32_t get_elapse_time(struct timespec *t)
{
    struct timespec 	end_time={0,};
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    return  (((end_time.tv_sec - t->tv_sec)*1000000000) + (end_time.tv_nsec - t->tv_nsec))/1000000;//ms
}

static int CheckLink()
{
    int fdin;
    char val = 0;

    fdin = open("/sys/class/net/eth0/carrier", O_RDONLY);
    if(fdin >=0){
        read(fdin, &val, 1);
        close(fdin);
        if(val=='1') return 1;
    }
    return 0;
}

void *shell_cmd_thread()
{
	char 	cmd[MAX_PATH], prompt[20];
	int		size = 0;
	//int i=0;
	//int cnt=0;
   	int mipi_fd;
	char str[MAX_PATH];
	
	memset(prompt, 0, sizeof(prompt));
	sprintf(prompt, "[%s_%s]", "ENSIS", FW_VERSION);

	while(!pthread_end)
	{
		//usleep(50000);	
		memset(cmd, 0, MAX_PATH);
		if( NULL != fgets(cmd, sizeof(cmd), stdin) )
		{
			size = strlen(cmd);
			if(size>0) cmd[size-1] = '\0';

			if(size==0) {}
			else if(!strcmp(cmd, "help"))
			{
				LOGE("ADC_REGISTER = adcreg\r\n");
				LOGE("ADC_COUNT_REG_RESET = clear\r\n");
				LOGE("FIFO_EMPTY / FULL CHECK = fifore\r\n");	
				LOGE("ADC_0_1_2_3_4_INIT = calinit\r\n");
				LOGE("ADC_0_INIT = adc0calinit\r\n");	
				LOGE("ADC_1_INIT = adc1calinit\r\n");
				LOGE("ADC_2_INIT = adc2calinit\r\n");
				LOGE("ADC_3_INIT = adc3calinit\r\n");
				LOGE("ADC_4_INIT = adc4calinit\r\n");				
				LOGE("ADC_0_0V_CAL = adccal00v 0.0023\r\n");
				LOGE("ADC_1_0V_CAL = adccal10v 0.0011\r\n");	
				LOGE("ADC_2_0V_CAL = adccal20v 0.0023\r\n");
				LOGE("ADC_1_0V_CAL = adccal30v 0.0011\r\n");	
				LOGE("ADC_2_0V_CAL = adccal40v 0.0023\r\n");					
				LOGE("ADC_0_P10V_CAL = adccalp010v 10.00\r\n");
				LOGE("ADC_1_P10V_CAL = adccalp110v 10.00\r\n");	
				LOGE("ADC_2_P10V_CAL = adccalp210v 10.00\r\n");
				LOGE("ADC_3_P10V_CAL = adccalp310v 10.00\r\n");	
				LOGE("ADC_4_P10V_CAL = adccalp410v 10.00\r\n");				
				LOGE("ADC_0_N10V_CAL = adccaln010v 10.00\r\n");
				LOGE("ADC_1_N10V_CAL = adccaln110v 10.00\r\n");	
				LOGE("ADC_2_N10V_CAL = adccaln210v 10.00\r\n");	
				LOGE("ADC_3_N10V_CAL = adccaln310v 10.00\r\n");	
				LOGE("ADC_4_N10V_CAL = adccaln410v 10.00\r\n");						
				LOGE("ADC_0_P_CAL_SLOPE_OFFSET_CALCULATION = adc0pcal\r\n");
				LOGE("ADC_1_P_CAL_SLOPE_OFFSET_CALCULATION = adc1pcal\r\n");		
				LOGE("ADC_2_P_CAL_SLOPE_OFFSET_CALCULATION = adc2pcal\r\n");
				LOGE("ADC_3_P_CAL_SLOPE_OFFSET_CALCULATION = adc3pcal\r\n");		
				LOGE("ADC_4_P_CAL_SLOPE_OFFSET_CALCULATION = adc4pcal\r\n");					
				LOGE("ADC_0_N_CAL_SLOPE_OFFSET_CALCULATION = adc0ncal\r\n");
				LOGE("ADC_1_N_CAL_SLOPE_OFFSET_CALCULATION = adc1ncal\r\n");		
				LOGE("ADC_2_N_CAL_SLOPE_OFFSET_CALCULATION = adc2ncal\r\n");
				LOGE("ADC_3_N_CAL_SLOPE_OFFSET_CALCULATION = adc3ncal\r\n");		
				LOGE("ADC_4_N_CAL_SLOPE_OFFSET_CALCULATION = adc4ncal\r\n");				
				LOGE("ADC_0__CAL_DATA_SAVE = adc0calsave\r\n");
				LOGE("ADC_1__CAL_DATA_SAVE = adc1calsave\r\n");
				LOGE("ADC_2__CAL_DATA_SAVE = adc2calsave\r\n");
				LOGE("ADC_3__CAL_DATA_SAVE = adc3calsave\r\n");
				LOGE("ADC_4__CAL_DATA_SAVE = adc4calsave\r\n");				
				LOGE("ADC_0_P_CAL_USER_OFFSET_SETTING = adc0poffset\r\n");
				LOGE("ADC_1_P_CAL_USER_OFFSET_SETTING = adc1poffset\r\n");																																																				
				LOGE("ADC_2_P_CAL_USER_OFFSET_SETTING = adc2poffset\r\n");
				LOGE("ADC_3_P_CAL_USER_OFFSET_SETTING = adc3poffset\r\n");																																																				
				LOGE("ADC_4_P_CAL_USER_OFFSET_SETTING = adc4poffset\r\n");				
				LOGE("ADC_0_N_CAL_USER_OFFSET_SETTING = adc0noffset\r\n");
				LOGE("ADC_1_N_CAL_USER_OFFSET_SETTING = adc1noffset\r\n");																																																				
				LOGE("ADC_2_N_CAL_USER_OFFSET_SETTING = adc2noffset\r\n");	
				LOGE("ADC_3_N_CAL_USER_OFFSET_SETTING = adc3noffset\r\n");																																																				
				LOGE("ADC_4_N_CAL_USER_OFFSET_SETTING = adc4noffset\r\n");					
				LOGE("ADC_0_CAL_DATA_LOAD = adc0load\r\n");
				LOGE("ADC_1_CAL_DATA_LOAD = adc1load\r\n");	
				LOGE("ADC_2_CAL_DATA_LOAD = adc2load\r\n");	
				LOGE("ADC_3_CAL_DATA_LOAD = adc3load\r\n");	
				LOGE("ADC_4_CAL_DATA_LOAD = adc4load\r\n");					
			}
			else if(!strcmp(cmd, "cprf"))
			{
				if(cprf == 0)
				{
					cprf = 1;
					printf("Cprf Print ON\r\n");
				}
				else
				{
					cprf = 0;
					printf("Cprf Print OFF\r\n");
				}				
			}
			else if(!strcmp(cmd, "tprf"))
			{
				if(tprf == 0)
				{
					tprf = 1;
					printf("Tprf Print ON\r\n");
				}
				else
				{
					tprf = 0;
					printf("Tprf Print OFF\r\n");
				}				
			}
			else if(!strcmp(cmd, "eprf"))
			{
				if(eprf == 0)
				{
					eprf = 1;
					printf("Eprf Print ON\r\n");
				}
				else
				{
					eprf = 0;
					printf("Eprf Print OFF\r\n");
				}				
			}
			else if(!strcmp(cmd, "aprf"))
			{
				if(aprf == 0)
				{
					aprf = 1;
					printf("Aprf Print ON\r\n");
				}
				else
				{
					aprf = 0;
					printf("Aprf Print OFF\r\n");
				}				
			}	
			else if(!strncmp(cmd, "tau ",4))
			{
				lpf_tau = atof(cmd+4); 
				printf("lpf_tau = %lf\r\n", lpf_tau);				
			}
			else if(!strncmp(cmd, "time ",5))
			{
				lpf_time  = atof(cmd+5); 
				printf("lpf_time = %lf\r\n", lpf_time);									
			}																
			else if(!strcmp(cmd, "qw"))
			{
				printf("z_file = %s / x_file = %s / y_file = %s\r\n", z_data, x_data, y_data);
			}			
			else if(!strcmp(cmd, "as"))
			{
				trigger_count_clear();														
			}					
			else if(!strcmp(cmd, "uartvp"))
			{
				//sdcd_serial_write( 2,"VP");
				printf("UART_1_TX_OK\r\n");
			}
			else if(!strcmp(cmd, "uartrd"))
			{
				//sdcd_serial_write( 2,"IP");
				printf("UART_1_READ_TX_OK\r\n");
			}
		
			else if(!strcmp(cmd, "tp"))
			{
				struct timespec 	func_time;
				struct timespec 	end_time;
				clock_gettime(CLOCK_MONOTONIC, &func_time);
				usleep(100);
				clock_gettime(CLOCK_MONOTONIC, &end_time);
				printf("TP1 = %ld\r\n", ((end_time.tv_sec - func_time.tv_sec)*1000000000) + (end_time.tv_nsec - func_time.tv_nsec));
			}											
			else if(!strcmp(cmd, "realtime"))
			{
				if(real_time_sensing_flag == 0)
				{			
					ad7656->total_count = 1;
					real_time_sensing_flag = 1;
					timer_end_flag = 1;
					real_count = 0;
					printf("REAL_TIME_SENSING_START\r\n");
				}
				else
				{
					real_time_sensing_flag = 0;
					timer_end_flag = 0;
					printf("REAL_TIME_SENSING_OFF\r\n");
				}					
			}
			else if(!strcmp(cmd, "linearty"))
			{
				linearty_flag = 1;
				ad7656->total_count = 1;
				real_time_sensing_flag = 1;
				timer_end_flag = 1;				
			}																
			else if(!strcmp(cmd, "g1on"))
			{
				uart_gpio_0_on();
				printf("GPIO0_ON\r\n");				
			}	
			else if(!strcmp(cmd, "g1off"))
			{
				uart_gpio_0_off();
				printf("GPIO0_OFF\r\n");												
			}					
			else if(!strcmp(cmd, "g2on"))
			{
				uart_gpio_1_on();
				printf("GPIO1_ON\r\n");				
			}	
			else if(!strcmp(cmd, "g2off"))
			{
				uart_gpio_1_off();	
				printf("GPIO1_OFF\r\n");											
			}
			else if(!strcmp(cmd, "g3on"))
			{
				uart_gpio_2_on();
				printf("GPIO2_ON\r\n");				
			}	
			else if(!strcmp(cmd, "g3off"))
			{
				uart_gpio_2_off();	
				printf("GPIO2_OFF\r\n");											
			}	
			else if(!strncmp(cmd, "ld2 ",4))
			{
				unsigned char value = atof(cmd+4); 
				uart_to_i2c_ctl(0x00,value);
			}	
			else if(!strncmp(cmd, "ld3 ",4))
			{
				unsigned char value = atof(cmd+4); 
				uart_to_i2c_ctl(0x80,value);
			}					
			else if(!strcmp(cmd, "triclear"))
			{
				trigger_count_clear();
			}		
			else if(!strcmp(cmd, "tridata"))
			{
				LOGE("TRIGGER COUNT = %d / READ COUNT = %d\r\n", count_data.trigger, count_data.read);
				LOGE("AUTO_TRIGGER COUNT = %d\r\n", ad7656->total_count);		
				LOGE("CURRENT TTRIGGER_0 COUNT = %d\r\n", adc_trigger_count_0);									
				LOGE("CURRENT TTRIGGER_1 COUNT = %d\r\n", adc_trigger_count_1);
				LOGE("CURRENT TTRIGGER_2 COUNT = %d\r\n", adc_trigger_count_2);
				LOGE("CURRENT TTRIGGER_3 COUNT = %d\r\n", adc_trigger_count_3);
				LOGE("CURRENT TTRIGGER_4 COUNT = %d\r\n", adc_trigger_count_4);									
				LOGE("CURRENT AUTO_TTRIGGER_0 COUNT = %d\r\n", auto_adc_trigger_count_0);	
				LOGE("CURRENT AUTO_TTRIGGER_1 COUNT = %d\r\n", auto_adc_trigger_count_1);
				LOGE("CURRENT AUTO_TTRIGGER_2 COUNT = %d\r\n", auto_adc_trigger_count_2);
				LOGE("CURRENT AUTO_TTRIGGER_3 COUNT = %d\r\n", auto_adc_trigger_count_3);
				LOGE("CURRENT AUTO_TTRIGGER_4 COUNT = %d\r\n", auto_adc_trigger_count_4);																									
			}							
			else if(!strcmp(cmd, "trion"))
			{
				ad7656->auto_onoff = 0;
				ad7656->auto_onoff = 1;
				ad7656->auto_onoff = 0;
				LOGE("trigger on\r\n");		
			}																																			
			else if(!strcmp(cmd, "adcreg"))
			{
				LOGE("-----------------------------------------------------------\r\n");
				LOGE("----------------------AD7656_REG---------------------------\r\n");				
				LOGE("reg_reset =%x\r\n", ad7656->reg_reset);
				LOGE("reg_count_reset =%x\r\n", ad7656->reg_count_reset);
				LOGE("sensing_count =%x\r\n", ad7656->sensing_count);
				LOGE("sens_interval =%x\r\n", ad7656->sens_interval);
				LOGE("total_count =%x\r\n", ad7656->total_count);
				LOGE("auto_onoff =%x\r\n", ad7656->auto_onoff);
				LOGE("dummy_6 =%x\r\n", ad7656->dummy_6);
				LOGE("dummy_7 =%x\r\n", ad7656->dummy_7);
				LOGE("dummy_8 =%x\r\n", ad7656->dummy_8);		
				LOGE("dummy_9 =%x\r\n", ad7656->dummy_9);											
				LOGE("ver =%x\r\n", ad7656->ver);
				LOGE("manual_trigger_count =%x\r\n", ad7656->manual_trigger_count);
				LOGE("auto_trigger_count =%x\r\n", ad7656->auto_trigger_count);		
				LOGE("end_count =%x\r\n", ad7656->end_count);
				LOGE("z_sum =%x\r\n", ad7656->z_sum);
				LOGE("xy_sum =%x\r\n", ad7656->xy_sum);
				LOGE("dummy_16 =%x\r\n", ad7656->dummy_16);
				LOGE("dummy_17 =%x\r\n", ad7656->dummy_17);
				LOGE("dummy_18 =%x\r\n", ad7656->dummy_18);
				LOGE("dummy_19 =%x\r\n", ad7656->dummy_19);
				LOGE("dummy_20 =%x\r\n", ad7656->dummy_20);	
				LOGE("-----------------------------------------------------------\r\n");																																																													
			}
			else if(!strcmp(cmd, "fifore"))
			{								
				LOGE("FIFO0_EMPTY = %x\n", fifo_read_0->empty);								
				LOGE("FIFO1_EMPTY = %x\n", fifo_read_1->empty);
				LOGE("FIFO2_EMPTY = %x\n", fifo_read_2->empty);	
				//LOGE("FIFO3_EMPTY = %x\n", fifo_read_3->empty);								
				//LOGE("FIFO4_EMPTY = %x\n", fifo_read_4->empty);				
				LOGE("FIFO0_FULL = %x\n", fifo_read_0->full);								
				LOGE("FIFO1_FULL = %x\n", fifo_read_1->full);
				LOGE("FIFO2_FULL = %x\n", fifo_read_2->full);
				//LOGE("FIFO3_FULL = %x\n", fifo_read_3->full);
				//LOGE("FIFO4_FULL = %x\n", fifo_read_4->full);								
			}	
			else if(!strncmp(cmd, "co ", 3))
			{
				count_data.read = atof(cmd+3);
				ad7656->sensing_count = atof(cmd+3);																		
			}	
			else if(!strncmp(cmd, "in ", 3))
			{
				ad7656->sens_interval = atof(cmd+3);																		
			}					
			else if(!strcmp(cmd, "calinit"))
			{
				memset(&adc0_cal, 0, sizeof(adc0_cal));
				memset(&adc1_cal, 0, sizeof(adc1_cal));
				memset(&adc2_cal, 0, sizeof(adc2_cal));
				memset(&adc3_cal, 0, sizeof(adc3_cal));
				memset(&adc4_cal, 0, sizeof(adc4_cal));				
				LOGE("--------------------------------------------------ADC0--------------------------------------------------\r\n");
				LOGE("adc_0_cal_0v_OK = value = %f / step = %f\n", adc0_cal.adc_0v_value, adc0_cal.adc_0v_step);
				LOGE("adc_0_cal_p10v_OK = value = %f / step = %f\n", adc0_cal.adc_10v_value, adc0_cal.adc_10v_step);
				LOGE("adc_0_cal_n10v_OK = value = %f / step = %f\n", adc0_cal.adc_n10v_value, adc0_cal.adc_n10v_step);				
				LOGE("adc_0_p_ratio value = %f\r\n", adc0_cal.adc_p_ratio);
				LOGE("adc_0_p_offset value = %f\r\n", adc0_cal.adc_p_offset);
				LOGE("adc_0_n_ratio value = %f\r\n", adc0_cal.adc_n_ratio);
				LOGE("adc_0_n_offset value = %f\r\n", adc0_cal.adc_n_offset);
				LOGE("adc_0_cal.user_p_offset value = %f\r\n", adc0_cal.user_p_offset);
				LOGE("adc_0_cal.user_n_offset value = %f\r\n", adc0_cal.user_n_offset);										
				LOGE("--------------------------------------------------ADC1--------------------------------------------------\r\n");
				LOGE("adc_1_cal_0v_OK = value = %f / step = %f\n", adc1_cal.adc_0v_value, adc1_cal.adc_0v_step);
				LOGE("adc_1_cal_p10v_OK = value = %f / step = %f\n", adc1_cal.adc_10v_value, adc1_cal.adc_10v_step);
				LOGE("adc_1_cal_n10v_OK = value = %f / step = %f\n", adc1_cal.adc_n10v_value, adc1_cal.adc_n10v_step);				
				LOGE("adc_1_p_ratio value = %f\r\n", adc1_cal.adc_p_ratio);
				LOGE("adc_1_p_offset value = %f\r\n", adc1_cal.adc_p_offset);
				LOGE("adc_1_n_ratio value = %f\r\n", adc1_cal.adc_n_ratio);
				LOGE("adc_1_n_offset value = %f\r\n", adc1_cal.adc_n_offset);
				LOGE("adc_1_cal.user_p_offset value = %f\r\n", adc1_cal.user_p_offset);
				LOGE("adc_1_cal.user_n_offset value = %f\r\n", adc1_cal.user_n_offset);					
				LOGE("--------------------------------------------------ADC2--------------------------------------------------\r\n");
				LOGE("adc_2_cal_0v_OK = value = %f / step = %f\n", adc2_cal.adc_0v_value, adc2_cal.adc_0v_step);
				LOGE("adc_2_cal_p10v_OK = value = %f / step = %f\n", adc2_cal.adc_10v_value, adc2_cal.adc_10v_step);
				LOGE("adc_2_cal_n10v_OK = value = %f / step = %f\n", adc2_cal.adc_n10v_value, adc2_cal.adc_n10v_step);				
				LOGE("adc_2_p_ratio value = %f\r\n", adc2_cal.adc_p_ratio);
				LOGE("adc_2_p_offset value = %f\r\n", adc2_cal.adc_p_offset);
				LOGE("adc_2_n_ratio value = %f\r\n", adc2_cal.adc_n_ratio);
				LOGE("adc_2_n_offset value = %f\r\n", adc2_cal.adc_n_offset);	
				LOGE("adc_2_cal.user_p_offset value = %f\r\n", adc2_cal.user_p_offset);
				LOGE("adc_2_cal.user_n_offset value = %f\r\n", adc2_cal.user_n_offset);	
				LOGE("--------------------------------------------------ADC3--------------------------------------------------\r\n");
				LOGE("adc_3_cal_0v_OK = value = %f / step = %f\n", adc3_cal.adc_0v_value, adc3_cal.adc_0v_step);
				LOGE("adc_3_cal_p10v_OK = value = %f / step = %f\n", adc3_cal.adc_10v_value, adc3_cal.adc_10v_step);
				LOGE("adc_3_cal_n10v_OK = value = %f / step = %f\n", adc3_cal.adc_n10v_value, adc3_cal.adc_n10v_step);				
				LOGE("adc_3_p_ratio value = %f\r\n", adc3_cal.adc_p_ratio);
				LOGE("adc_3_p_offset value = %f\r\n", adc3_cal.adc_p_offset);
				LOGE("adc_3_n_ratio value = %f\r\n", adc3_cal.adc_n_ratio);
				LOGE("adc_3_n_offset value = %f\r\n", adc3_cal.adc_n_offset);
				LOGE("adc_3_cal.user_p_offset value = %f\r\n", adc3_cal.user_p_offset);
				LOGE("adc_3_cal.user_n_offset value = %f\r\n", adc3_cal.user_n_offset);					
				LOGE("--------------------------------------------------ADC4--------------------------------------------------\r\n");
				LOGE("adc_4_cal_0v_OK = value = %f / step = %f\n", adc4_cal.adc_0v_value, adc4_cal.adc_0v_step);
				LOGE("adc_4_cal_p10v_OK = value = %f / step = %f\n", adc4_cal.adc_10v_value, adc4_cal.adc_10v_step);
				LOGE("adc_4_cal_n10v_OK = value = %f / step = %f\n", adc4_cal.adc_n10v_value, adc4_cal.adc_n10v_step);				
				LOGE("adc_4_p_ratio value = %f\r\n", adc4_cal.adc_p_ratio);
				LOGE("adc_4_p_offset value = %f\r\n", adc4_cal.adc_p_offset);
				LOGE("adc_4_n_ratio value = %f\r\n", adc4_cal.adc_n_ratio);
				LOGE("adc_4_n_offset value = %f\r\n", adc4_cal.adc_n_offset);	
				LOGE("adc_4_cal.user_p_offset value = %f\r\n", adc4_cal.user_p_offset);
				LOGE("adc_4_cal.user_n_offset value = %f\r\n", adc4_cal.user_n_offset);									
			}			
			else if(!strcmp(cmd, "adc0calinit"))
			{
				memset(&adc0_cal, 0, sizeof(adc0_cal));
				printf("adc_0_0v_value = %f\r\n", adc0_cal.adc_0v_value);
				printf("adc_0_0v_step = %f\r\n", adc0_cal.adc_0v_step);
				printf("adc_0_10v_value = %f\r\n", adc0_cal.adc_10v_value);
				printf("adc_0_10v_step = %f\r\n", adc0_cal.adc_10v_step);
				printf("adc_0_ratio = %f\r\n", adc0_cal.adc_p_ratio);
				printf("adc_0_offset = %f\r\n", adc0_cal.adc_p_offset);		
			}
			else if(!strcmp(cmd, "adc1calinit"))
			{
				memset(&adc1_cal, 0, sizeof(adc1_cal));
				printf("adc_1_0v_value = %f\r\n", adc1_cal.adc_0v_value);
				printf("adc_1_0v_step = %f\r\n", adc1_cal.adc_0v_step);
				printf("adc_1_10v_value = %f\r\n", adc1_cal.adc_10v_value);
				printf("adc_1_10v_step = %f\r\n", adc1_cal.adc_10v_step);
				printf("adc_1_ratio = %f\r\n", adc1_cal.adc_p_ratio);
				printf("adc_1_offset = %f\r\n", adc1_cal.adc_p_offset);		
			}
			else if(!strcmp(cmd, "adc2calinit"))
			{
				memset(&adc2_cal, 0, sizeof(adc2_cal));
				printf("adc_2_0v_value = %f\r\n", adc2_cal.adc_0v_value);
				printf("adc_2_0v_step = %f\r\n", adc2_cal.adc_0v_step);
				printf("adc_2_10v_value = %f\r\n", adc2_cal.adc_10v_value);
				printf("adc_2_10v_step = %f\r\n", adc2_cal.adc_10v_step);
				printf("adc_2_ratio = %f\r\n", adc2_cal.adc_p_ratio);
				printf("adc_2_offset = %f\r\n", adc2_cal.adc_p_offset);		
			}
			else if(!strcmp(cmd, "adc3calinit"))
			{
				memset(&adc3_cal, 0, sizeof(adc3_cal));
				printf("adc_3_0v_value = %f\r\n", adc3_cal.adc_0v_value);
				printf("adc_3_0v_step = %f\r\n", adc3_cal.adc_0v_step);
				printf("adc_3_10v_value = %f\r\n", adc3_cal.adc_10v_value);
				printf("adc_3_10v_step = %f\r\n", adc3_cal.adc_10v_step);
				printf("adc_3_ratio = %f\r\n", adc3_cal.adc_p_ratio);
				printf("adc_3_offset = %f\r\n", adc3_cal.adc_p_offset);		
			}
			else if(!strcmp(cmd, "adc4calinit"))
			{
				memset(&adc4_cal, 0, sizeof(adc3_cal));
				printf("adc_4_0v_value = %f\r\n", adc4_cal.adc_0v_value);
				printf("adc_4_0v_step = %f\r\n", adc4_cal.adc_0v_step);
				printf("adc_4_10v_value = %f\r\n", adc4_cal.adc_10v_value);
				printf("adc_4_10v_step = %f\r\n", adc4_cal.adc_10v_step);
				printf("adc_4_ratio = %f\r\n", adc4_cal.adc_p_ratio);
				printf("adc_4_offset = %f\r\n", adc4_cal.adc_p_offset);		
			}														
			else if(!strncmp(cmd, "adccal00v ", 10))
			{
				read_count = FIXED_COUNT;
				adc0_0v_cal_flag = 1;
				ad7656->total_count = 1;				
				ad7656->sensing_count = read_count;
				real_count_0 = 0;
				trigger_auto_run();
				usleep(5000);				
				adc0_cal.adc_0v_value = atof(cmd+10);												
				adc0_0v_cal_flag = 0;	
				LOGE("adc_0_cal_0v_OK = value = %f / step = %f\n", adc0_cal.adc_0v_value, adc0_cal.adc_0v_step);								
			}
			else if(!strncmp(cmd, "adccal10v ", 10))
			{
				read_count = FIXED_COUNT;
				adc1_0v_cal_flag = 1;
				ad7656->total_count = 1;				
				ad7656->sensing_count = read_count;
				real_count_1 = 0;
				trigger_auto_run();
				usleep(5000);	
				adc1_cal.adc_0v_value = atof(cmd+10);												
				adc1_0v_cal_flag = 0;	
				LOGE("adc_1_cal_0v_OK = value = %f / step = %f\n", adc1_cal.adc_0v_value, adc1_cal.adc_0v_step);								
			}
			else if(!strncmp(cmd, "adccal20v ", 10))
			{
				read_count = FIXED_COUNT;
				adc2_0v_cal_flag = 1;
				ad7656->total_count = 1;				
				ad7656->sensing_count = read_count;
				real_count_2 = 0;
				trigger_auto_run();
				usleep(5000);	
				adc2_cal.adc_0v_value = atof(cmd+10);												
				adc2_0v_cal_flag = 0;	
				LOGE("adc_2_cal_0v_OK = value = %f / step = %f\n", adc2_cal.adc_0v_value, adc2_cal.adc_0v_step);								
			}
			else if(!strncmp(cmd, "adccal30v ", 10))
			{
				read_count = FIXED_COUNT;
				adc3_0v_cal_flag = 1;
				ad7656->total_count = 1;				
				ad7656->sensing_count = read_count;
				real_count_3 = 0;
				trigger_auto_run();
				usleep(5000);	
				adc3_cal.adc_0v_value = atof(cmd+10);												
				adc3_0v_cal_flag = 0;	
				LOGE("adc_3_cal_0v_OK = value = %f / step = %f\n", adc3_cal.adc_0v_value, adc3_cal.adc_0v_step);								
			}
			else if(!strncmp(cmd, "adccal40v ", 10))
			{
				read_count = FIXED_COUNT;
				adc4_0v_cal_flag = 1;
				ad7656->total_count = 1;				
				ad7656->sensing_count = read_count;
				real_count_4 = 0;
				trigger_auto_run();
				usleep(5000);	
				adc4_cal.adc_0v_value = atof(cmd+10);												
				adc4_0v_cal_flag = 0;	
				LOGE("adc_4_cal_0v_OK = value = %f / step = %f\n", adc4_cal.adc_0v_value, adc4_cal.adc_0v_step);								
			}												
			else if(!strncmp(cmd, "adccalp010v ", 12))
			{
				read_count = FIXED_COUNT;
				adc0_10v_cal_flag = 1;
				ad7656->total_count = 1;				
				ad7656->sensing_count = read_count;				
				real_count_0 = 0;											
				trigger_auto_run();
				usleep(5000);
				adc0_cal.adc_10v_value = atof(cmd+12);		
				adc0_10v_cal_flag = 0;	
				LOGE("adc_0_cal_p10v_OK = value = %f / step = %f\n", adc0_cal.adc_10v_value, adc0_cal.adc_10v_step);								
			}
			else if(!strncmp(cmd, "adccalp110v ", 12))
			{
				read_count = FIXED_COUNT;
				adc1_10v_cal_flag = 1;
				ad7656->total_count = 1;				
				ad7656->sensing_count = read_count;				
				real_count_1 = 0;											
				trigger_auto_run();
				usleep(5000);
				adc1_cal.adc_10v_value = atof(cmd+12);		
				adc1_10v_cal_flag = 0;	
				LOGE("adc_1_cal_p10v_OK = value = %f / step = %f\n", adc1_cal.adc_10v_value, adc1_cal.adc_10v_step);									
			}
			else if(!strncmp(cmd, "adccalp210v ", 12))
			{
				read_count = FIXED_COUNT;
				adc2_10v_cal_flag = 1;
				ad7656->total_count = 1;				
				ad7656->sensing_count = read_count;				
				real_count_2 = 0;											
				trigger_auto_run();
				usleep(5000);
				adc2_cal.adc_10v_value = atof(cmd+12);		
				adc2_10v_cal_flag = 0;	
				LOGE("adc_2_cal_p10v_OK = value = %f / step = %f\n", adc2_cal.adc_10v_value, adc2_cal.adc_10v_step);								
			}
			else if(!strncmp(cmd, "adccalp310v ", 12))
			{
				read_count = FIXED_COUNT;
				adc3_10v_cal_flag = 1;
				ad7656->total_count = 1;				
				ad7656->sensing_count = read_count;				
				real_count_3 = 0;											
				trigger_auto_run();
				usleep(5000);
				adc3_cal.adc_10v_value = atof(cmd+12);		
				adc3_10v_cal_flag = 0;	
				LOGE("adc_3_cal_p10v_OK = value = %f / step = %f\n", adc3_cal.adc_10v_value, adc3_cal.adc_10v_step);								
			}	
			else if(!strncmp(cmd, "adccalp410v ", 12))
			{
				read_count = FIXED_COUNT;
				adc4_10v_cal_flag = 1;
				ad7656->total_count = 1;				
				ad7656->sensing_count = read_count;				
				real_count_4 = 0;											
				trigger_auto_run();
				usleep(5000);
				adc4_cal.adc_10v_value = atof(cmd+12);		
				adc4_10v_cal_flag = 0;	
				LOGE("adc_4_cal_p10v_OK = value = %f / step = %f\n", adc4_cal.adc_10v_value, adc4_cal.adc_10v_step);								
			}													
			else if(!strncmp(cmd, "adccaln010v ", 12))
			{
				read_count = FIXED_COUNT;
				adc0_n10v_cal_flag = 1;	
				ad7656->total_count = 1;														
				ad7656->sensing_count = read_count;
				real_count_0 = 0;
				trigger_auto_run();
				usleep(5000);				
				adc0_cal.adc_n10v_value = atof(cmd+12);					
				adc0_n10v_cal_flag = 0;	
				LOGE("adc_0_cal_n10v_OK = value = %f / step = %f\n", adc0_cal.adc_n10v_value, adc0_cal.adc_n10v_step);								
			}
			else if(!strncmp(cmd, "adccaln110v ", 12))
			{
				read_count = FIXED_COUNT;
				adc1_n10v_cal_flag = 1;		
				ad7656->total_count = 1;													
				ad7656->sensing_count = read_count;
				real_count_1 = 0;
				trigger_auto_run();
				usleep(5000);				
				adc1_cal.adc_n10v_value = atof(cmd+12);					
				adc1_n10v_cal_flag = 0;	
				LOGE("adc_1_cal_n10v_OK = value = %f / step = %f\n", adc1_cal.adc_n10v_value, adc1_cal.adc_n10v_step);								
			}	
			else if(!strncmp(cmd, "adccaln210v ", 12))
			{
				read_count = FIXED_COUNT;
				adc2_n10v_cal_flag = 1;		
				ad7656->total_count = 1;													
				ad7656->sensing_count = read_count;
				real_count_2 = 0;
				trigger_auto_run();
				usleep(5000);				
				adc2_cal.adc_n10v_value = atof(cmd+12);					
				adc2_n10v_cal_flag = 0;	
				LOGE("adc_2_cal_n10v_OK = value = %f / step = %f\n", adc2_cal.adc_n10v_value, adc2_cal.adc_n10v_step);								
			}
			else if(!strncmp(cmd, "adccaln310v ", 12))
			{
				read_count = FIXED_COUNT;
				adc3_n10v_cal_flag = 1;	
				ad7656->total_count = 1;														
				ad7656->sensing_count = read_count;
				real_count_3 = 0;
				trigger_auto_run();
				usleep(5000);				
				adc3_cal.adc_n10v_value = atof(cmd+12);					
				adc3_n10v_cal_flag = 0;	
				LOGE("adc_3_cal_n10v_OK = value = %f / step = %f\n", adc3_cal.adc_n10v_value, adc3_cal.adc_n10v_step);								
			}	
			else if(!strncmp(cmd, "adccaln410v ", 12))
			{
				read_count = FIXED_COUNT;
				adc4_n10v_cal_flag = 1;	
				ad7656->total_count = 1;														
				ad7656->sensing_count = read_count;
				real_count_4 = 0;
				trigger_auto_run();
				usleep(5000);				
				adc4_cal.adc_n10v_value = atof(cmd+12);					
				adc4_n10v_cal_flag = 0;	
				LOGE("adc_4_cal_n10v_OK = value = %f / step = %f\n", adc4_cal.adc_n10v_value, adc4_cal.adc_n10v_step);								
			}							
			else if(!strncmp(cmd, "adc0poffset ", 11))
			{
				adc0_cal.user_p_offset = atof(cmd+11);
				LOGE("adc0_cal.user_p_offset value = %f\r\n", adc0_cal.user_p_offset);						
			}
			else if(!strncmp(cmd, "adc0noffset ", 11))
			{
				adc0_cal.user_n_offset = atof(cmd+11);
				LOGE("adc0_cal.user_n_offset value = %f\r\n", adc0_cal.user_n_offset);							
			}
			else if(!strncmp(cmd, "adc1poffset ", 11))
			{
				adc1_cal.user_p_offset = atof(cmd+11);
				LOGE("adc1_cal.user_p_offset value = %f\r\n", adc1_cal.user_p_offset);							
			}
			else if(!strncmp(cmd, "adc1noffset ", 11))
			{
				adc1_cal.user_n_offset = atof(cmd+11);
				LOGE("adc1_cal.user_p_offset value = %f\r\n", adc1_cal.user_p_offset);										
			}
			else if(!strncmp(cmd, "adc2poffset ", 11))
			{
				adc2_cal.user_p_offset = atof(cmd+11);
				LOGE("adc2_cal.user_p_offset value = %f\r\n", adc2_cal.user_p_offset);											
			}
			else if(!strncmp(cmd, "adc2noffset ", 11))
			{
				adc2_cal.user_n_offset = atof(cmd+11);
				LOGE("adc2_cal.user_p_offset value = %f\r\n", adc2_cal.user_p_offset);											
			}
			else if(!strncmp(cmd, "adc3poffset ", 11))
			{
				adc3_cal.user_p_offset = atof(cmd+11);
				LOGE("adc3_cal.user_p_offset value = %f\r\n", adc3_cal.user_p_offset);											
			}
			else if(!strncmp(cmd, "adc3noffset ", 11))
			{
				adc3_cal.user_n_offset = atof(cmd+11);
				LOGE("adc3_cal.user_p_offset value = %f\r\n", adc3_cal.user_p_offset);											
			}
			else if(!strncmp(cmd, "adc4poffset ", 11))
			{
				adc4_cal.user_p_offset = atof(cmd+11);
				LOGE("adc4_cal.user_p_offset value = %f\r\n", adc4_cal.user_p_offset);											
			}
			else if(!strncmp(cmd, "adc4noffset ", 11))
			{
				adc4_cal.user_n_offset = atof(cmd+11);
				LOGE("adc4_cal.user_p_offset value = %f\r\n", adc4_cal.user_p_offset);											
			}																													
			else if(!strcmp(cmd, "adc0pcal"))
			{
				adc0_cal.adc_p_ratio = (adc0_cal.adc_10v_value-adc0_cal.adc_0v_value) / (adc0_cal.adc_10v_step-adc0_cal.adc_0v_step);
				adc0_cal.adc_p_offset = adc0_cal.adc_0v_value - (adc0_cal.adc_p_ratio * adc0_cal.adc_0v_step);
				LOGE("adc_0_p_ratio value = %f\r\n", adc0_cal.adc_p_ratio);
				LOGE("adc_0_p_offset value = %f\r\n", adc0_cal.adc_p_offset);			
			}
			else if(!strcmp(cmd, "adc1pcal"))
			{
				adc1_cal.adc_p_ratio = (adc1_cal.adc_10v_value-adc1_cal.adc_0v_value) / (adc1_cal.adc_10v_step-adc1_cal.adc_0v_step);
				adc1_cal.adc_p_offset = adc1_cal.adc_0v_value - (adc1_cal.adc_p_ratio * adc1_cal.adc_0v_step);
				LOGE("adc_1_p_ratio value = %f\r\n", adc1_cal.adc_p_ratio);
				LOGE("adc_1_p_offset value = %f\r\n", adc1_cal.adc_p_offset);			
			}
			else if(!strcmp(cmd, "adc2pcal"))
			{
				adc2_cal.adc_p_ratio = (adc2_cal.adc_10v_value-adc2_cal.adc_0v_value) / (adc2_cal.adc_10v_step-adc2_cal.adc_0v_step);
				adc2_cal.adc_p_offset = adc2_cal.adc_0v_value - (adc2_cal.adc_p_ratio * adc2_cal.adc_0v_step);
				LOGE("adc_2_p_ratio value = %f\r\n", adc2_cal.adc_p_ratio);
				LOGE("adc_2_p_offset value = %f\r\n", adc2_cal.adc_p_offset);			
			}	
			else if(!strcmp(cmd, "adc3pcal"))
			{
				adc3_cal.adc_p_ratio = (adc3_cal.adc_10v_value-adc3_cal.adc_0v_value) / (adc3_cal.adc_10v_step-adc3_cal.adc_0v_step);
				adc3_cal.adc_p_offset = adc3_cal.adc_0v_value - (adc3_cal.adc_p_ratio * adc3_cal.adc_0v_step);
				LOGE("adc_3_p_ratio value = %f\r\n", adc3_cal.adc_p_ratio);
				LOGE("adc_3_p_offset value = %f\r\n", adc3_cal.adc_p_offset);			
			}
			else if(!strcmp(cmd, "adc4pcal"))
			{
				adc4_cal.adc_p_ratio = (adc4_cal.adc_10v_value-adc4_cal.adc_0v_value) / (adc4_cal.adc_10v_step-adc4_cal.adc_0v_step);
				adc4_cal.adc_p_offset = adc4_cal.adc_0v_value - (adc4_cal.adc_p_ratio * adc4_cal.adc_0v_step);
				LOGE("adc_4_p_ratio value = %f\r\n", adc4_cal.adc_p_ratio);
				LOGE("adc_4_p_offset value = %f\r\n", adc4_cal.adc_p_offset);			
			}													
			else if(!strcmp(cmd, "adc0ncal"))
			{
				adc0_cal.adc_n_ratio = (adc0_cal.adc_0v_value-adc0_cal.adc_n10v_value) / (adc0_cal.adc_0v_step-adc0_cal.adc_n10v_step);
				adc0_cal.adc_n_offset = adc0_cal.adc_n10v_value - (adc0_cal.adc_n_ratio * adc0_cal.adc_n10v_step);
				LOGE("adc_0_n_ratio value = %f\r\n", adc0_cal.adc_n_ratio);
				LOGE("adc_0_n_offset value = %f\r\n", adc0_cal.adc_n_offset);			
			}	
			else if(!strcmp(cmd, "adc1ncal"))
			{
				adc1_cal.adc_n_ratio = (adc1_cal.adc_0v_value-adc1_cal.adc_n10v_value) / (adc1_cal.adc_0v_step-adc1_cal.adc_n10v_step);
				adc1_cal.adc_n_offset = adc1_cal.adc_n10v_value - (adc1_cal.adc_n_ratio * adc1_cal.adc_n10v_step);
				LOGE("adc_1_n_ratio value = %f\r\n", adc1_cal.adc_n_ratio);
				LOGE("adc_1_n_offset value = %f\r\n", adc1_cal.adc_n_offset);			
			}
			else if(!strcmp(cmd, "adc2ncal"))
			{
				adc2_cal.adc_n_ratio = (adc2_cal.adc_0v_value-adc2_cal.adc_n10v_value) / (adc2_cal.adc_0v_step-adc2_cal.adc_n10v_step);
				adc2_cal.adc_n_offset = adc2_cal.adc_n10v_value - (adc2_cal.adc_n_ratio * adc2_cal.adc_n10v_step);
				LOGE("adc_2_n_ratio value = %f\r\n", adc2_cal.adc_n_ratio);
				LOGE("adc_2_n_offset value = %f\r\n", adc2_cal.adc_n_offset);			
			}
			else if(!strcmp(cmd, "adc3ncal"))
			{
				adc3_cal.adc_n_ratio = (adc3_cal.adc_0v_value-adc3_cal.adc_n10v_value) / (adc3_cal.adc_0v_step-adc3_cal.adc_n10v_step);
				adc3_cal.adc_n_offset = adc3_cal.adc_n10v_value - (adc3_cal.adc_n_ratio * adc3_cal.adc_n10v_step);
				LOGE("adc_3_n_ratio value = %f\r\n", adc3_cal.adc_n_ratio);
				LOGE("adc_3_n_offset value = %f\r\n", adc3_cal.adc_n_offset);			
			}
			else if(!strcmp(cmd, "adc4ncal"))
			{
				adc4_cal.adc_n_ratio = (adc4_cal.adc_0v_value-adc4_cal.adc_n10v_value) / (adc4_cal.adc_0v_step-adc4_cal.adc_n10v_step);
				adc4_cal.adc_n_offset = adc4_cal.adc_n10v_value - (adc4_cal.adc_n_ratio * adc4_cal.adc_n10v_step);
				LOGE("adc_4_n_ratio value = %f\r\n", adc4_cal.adc_n_ratio);
				LOGE("adc_4_n_offset value = %f\r\n", adc4_cal.adc_n_offset);			
			}																			
			else if(!strcmp(cmd, "adc0calsave"))
			{
				FILE *adc_file;

				adc_file = fopen(ADC_0_CAL_FILE_PATH, "w+b");
 				fwrite(&adc0_cal, sizeof(adc0_cal), 1, adc_file);
				fclose(adc_file);
				system("sync");	
				LOGE("adc0_data_save_OK\r\n");			
			}
			else if(!strcmp(cmd, "adc1calsave"))
			{
				FILE *adc_file;

				adc_file = fopen(ADC_1_CAL_FILE_PATH, "w+b");
 				fwrite(&adc1_cal, sizeof(adc1_cal), 1, adc_file);
				fclose(adc_file);
				system("sync");	
				LOGE("adc1_data_save_OK\r\n");			
			}
			else if(!strcmp(cmd, "adc2calsave"))
			{
				FILE *adc_file;

				adc_file = fopen(ADC_2_CAL_FILE_PATH, "w+b");
 				fwrite(&adc2_cal, sizeof(adc2_cal), 1, adc_file);
				fclose(adc_file);
				system("sync");	
				LOGE("adc2_data_save_OK\r\n");			
			}	
			else if(!strcmp(cmd, "adc3calsave"))
			{
				FILE *adc_file;

				adc_file = fopen(ADC_3_CAL_FILE_PATH, "w+b");
 				fwrite(&adc3_cal, sizeof(adc3_cal), 1, adc_file);
				fclose(adc_file);
				system("sync");	
				LOGE("adc3_data_save_OK\r\n");			
			}	
			else if(!strcmp(cmd, "adc4calsave"))
			{
				FILE *adc_file;

				adc_file = fopen(ADC_4_CAL_FILE_PATH, "w+b");
 				fwrite(&adc4_cal, sizeof(adc4_cal), 1, adc_file);
				fclose(adc_file);
				system("sync");	
				LOGE("adc4_data_save_OK\r\n");			
			}												
			else if(!strcmp(cmd, "adc0load"))
			{
				FILE *adc_file;		
				adc_file = fopen(ADC_0_CAL_FILE_PATH, "r");
				fread(&adc0_cal, sizeof(adc0_cal), 1, adc_file);
			}
			else if(!strcmp(cmd, "adc1load"))
			{
				FILE *adc_file;		
				adc_file = fopen(ADC_1_CAL_FILE_PATH, "r");
				fread(&adc1_cal, sizeof(adc1_cal), 1, adc_file);
			}	
			else if(!strcmp(cmd, "adc2load"))
			{
				FILE *adc_file;		
				adc_file = fopen(ADC_2_CAL_FILE_PATH, "r");
				fread(&adc2_cal, sizeof(adc2_cal), 1, adc_file);
			}
			else if(!strcmp(cmd, "adc3load"))
			{
				FILE *adc_file;		
				adc_file = fopen(ADC_3_CAL_FILE_PATH, "r");
				fread(&adc3_cal, sizeof(adc3_cal), 1, adc_file);
			}	
			else if(!strcmp(cmd, "adc4load"))
			{
				FILE *adc_file;		
				adc_file = fopen(ADC_4_CAL_FILE_PATH, "r");
				fread(&adc4_cal, sizeof(adc4_cal), 1, adc_file);
			}																			
			else if(!strcmp(cmd, "calprint"))
			{
				LOGE("--------------------------------------------------ADC0--------------------------------------------------\r\n");
				LOGE("adc_0_cal_0v_OK = value = %f / step = %f\n", adc0_cal.adc_0v_value, adc0_cal.adc_0v_step);
				LOGE("adc_0_cal_p10v_OK = value = %f / step = %f\n", adc0_cal.adc_10v_value, adc0_cal.adc_10v_step);
				LOGE("adc_0_cal_n10v_OK = value = %f / step = %f\n", adc0_cal.adc_n10v_value, adc0_cal.adc_n10v_step);				
				LOGE("adc_0_p_ratio value = %f\r\n", adc0_cal.adc_p_ratio);
				LOGE("adc_0_p_offset value = %f\r\n", adc0_cal.adc_p_offset);
				LOGE("adc_0_n_ratio value = %f\r\n", adc0_cal.adc_n_ratio);
				LOGE("adc_0_n_offset value = %f\r\n", adc0_cal.adc_n_offset);
				LOGE("adc_0_cal.user_p_offset value = %f\r\n", adc0_cal.user_p_offset);
				LOGE("adc_0_cal.user_n_offset value = %f\r\n", adc0_cal.user_n_offset);										
				LOGE("--------------------------------------------------ADC1--------------------------------------------------\r\n");
				LOGE("adc_1_cal_0v_OK = value = %f / step = %f\n", adc1_cal.adc_0v_value, adc1_cal.adc_0v_step);
				LOGE("adc_1_cal_p10v_OK = value = %f / step = %f\n", adc1_cal.adc_10v_value, adc1_cal.adc_10v_step);
				LOGE("adc_1_cal_n10v_OK = value = %f / step = %f\n", adc1_cal.adc_n10v_value, adc1_cal.adc_n10v_step);				
				LOGE("adc_1_p_ratio value = %f\r\n", adc1_cal.adc_p_ratio);
				LOGE("adc_1_p_offset value = %f\r\n", adc1_cal.adc_p_offset);
				LOGE("adc_1_n_ratio value = %f\r\n", adc1_cal.adc_n_ratio);
				LOGE("adc_1_n_offset value = %f\r\n", adc1_cal.adc_n_offset);
				LOGE("adc_1_cal.user_p_offset value = %f\r\n", adc1_cal.user_p_offset);
				LOGE("adc_1_cal.user_n_offset value = %f\r\n", adc1_cal.user_n_offset);					
				LOGE("--------------------------------------------------ADC2--------------------------------------------------\r\n");
				LOGE("adc_2_cal_0v_OK = value = %f / step = %f\n", adc2_cal.adc_0v_value, adc2_cal.adc_0v_step);
				LOGE("adc_2_cal_p10v_OK = value = %f / step = %f\n", adc2_cal.adc_10v_value, adc2_cal.adc_10v_step);
				LOGE("adc_2_cal_n10v_OK = value = %f / step = %f\n", adc2_cal.adc_n10v_value, adc2_cal.adc_n10v_step);				
				LOGE("adc_2_p_ratio value = %f\r\n", adc2_cal.adc_p_ratio);
				LOGE("adc_2_p_offset value = %f\r\n", adc2_cal.adc_p_offset);
				LOGE("adc_2_n_ratio value = %f\r\n", adc2_cal.adc_n_ratio);
				LOGE("adc_2_n_offset value = %f\r\n", adc2_cal.adc_n_offset);	
				LOGE("adc_2_cal.user_p_offset value = %f\r\n", adc2_cal.user_p_offset);
				LOGE("adc_2_cal.user_n_offset value = %f\r\n", adc2_cal.user_n_offset);	
				LOGE("--------------------------------------------------ADC3--------------------------------------------------\r\n");
				LOGE("adc_3_cal_0v_OK = value = %f / step = %f\n", adc3_cal.adc_0v_value, adc3_cal.adc_0v_step);
				LOGE("adc_3_cal_p10v_OK = value = %f / step = %f\n", adc3_cal.adc_10v_value, adc3_cal.adc_10v_step);
				LOGE("adc_3_cal_n10v_OK = value = %f / step = %f\n", adc3_cal.adc_n10v_value, adc3_cal.adc_n10v_step);				
				LOGE("adc_3_p_ratio value = %f\r\n", adc3_cal.adc_p_ratio);
				LOGE("adc_3_p_offset value = %f\r\n", adc3_cal.adc_p_offset);
				LOGE("adc_3_n_ratio value = %f\r\n", adc3_cal.adc_n_ratio);
				LOGE("adc_3_n_offset value = %f\r\n", adc3_cal.adc_n_offset);	
				LOGE("adc_3_cal.user_p_offset value = %f\r\n", adc3_cal.user_p_offset);
				LOGE("adc_3_cal.user_n_offset value = %f\r\n", adc3_cal.user_n_offset);
				LOGE("--------------------------------------------------ADC4--------------------------------------------------\r\n");
				LOGE("adc_4_cal_0v_OK = value = %f / step = %f\n", adc4_cal.adc_0v_value, adc4_cal.adc_0v_step);
				LOGE("adc_4_cal_p10v_OK = value = %f / step = %f\n", adc4_cal.adc_10v_value, adc4_cal.adc_10v_step);
				LOGE("adc_4_cal_n10v_OK = value = %f / step = %f\n", adc4_cal.adc_n10v_value, adc4_cal.adc_n10v_step);				
				LOGE("adc_4_p_ratio value = %f\r\n", adc4_cal.adc_p_ratio);
				LOGE("adc_4_p_offset value = %f\r\n", adc4_cal.adc_p_offset);
				LOGE("adc_4_n_ratio value = %f\r\n", adc4_cal.adc_n_ratio);
				LOGE("adc_4_n_offset value = %f\r\n", adc4_cal.adc_n_offset);	
				LOGE("adc_4_cal.user_p_offset value = %f\r\n", adc4_cal.user_p_offset);
				LOGE("adc_4_cal.user_n_offset value = %f\r\n", adc4_cal.user_n_offset);																	

			}											
			else if(!strcmp(cmd, "calfilecreate"))
			{
				unsigned char adc_file = 0;
				if(access("/run/media/mmcblk0p2/f0/config", F_OK))
				{
					system("/run/media/mmcblk0p2/f0/config");
					printf("/run/media/mmcblk0p2/f0/config Folder Create\r\n");
				}
				usleep(1000);
				fopen(ADC_0_CAL_FILE_PATH, "w+b");
				if(adc_file < 0)
				{
					printf("%s Open Fail\r\n", ADC_0_CAL_FILE_PATH);
					printf("adc_0 cal value default\r\n");
    			}
				usleep(1000);
				adc_file = 0;
				fopen(ADC_1_CAL_FILE_PATH, "w+b");	
				{
					printf("%s Open Fail\r\n", ADC_1_CAL_FILE_PATH);
					printf("adc_1 cal value default\r\n");
    			}	
				usleep(1000);
				adc_file = 0;
				fopen(ADC_2_CAL_FILE_PATH, "w+b");	
				{
					printf("%s Open Fail\r\n", ADC_2_CAL_FILE_PATH);
					printf("adc_2 cal value default\r\n");
    			}										
			}																																														
			//else if(!strcmp(cmd, "exit")) pthread_end = 1;
			else if(!strcmp(cmd, "exit")) exit(0);
			else if(!strcmp(cmd, "s_tty"))
			{
				sdcd_serial_open(115200, 8, 1, 0);
			}
			else if(!strcmp(cmd, "e_tty"))
			{
				sdcd_serial_close();
			}
			else system(cmd);
			printf("%s", prompt);
		}
	}
	return 0;
}

int taskSpawn(int priority,int taskfunc)
{
	int ret_val,scope;
	pthread_attr_t attr;
	struct sched_param param;
	size_t stacksize; //2097152 -> 4194304
	
	pthread_attr_init(&attr);
    stacksize = 1024*1024*4;
    pthread_attr_setstacksize(&attr, stacksize);
	ret_val = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	ret_val |= pthread_attr_getscope(&attr, &scope);
	if(scope != PTHREAD_SCOPE_SYSTEM){
		scope = PTHREAD_SCOPE_SYSTEM;
		ret_val |= pthread_attr_setscope(&attr, scope);
	}
	ret_val |= pthread_attr_setschedpolicy(&attr, SCHED_RR);
	param.sched_priority = sched_get_priority_min(SCHED_RR) + priority;
	ret_val |= pthread_attr_setschedparam(&attr, &param);
	ret_val |= pthread_create(&TaskID[TaskCnt], NULL, (void *(*)())taskfunc, (void *)TaskCnt);
	TaskCnt++;

	return ret_val;
}

int main()
{
	int thr_id;
	char p_name[] = "thread";	
	char p_name0[] = "thread_0";
	char p_name1[] = "thread_1";
	char p_name2[] = "thread_2";	
	struct can_frame frame;
	pthread_mutex_init(&mutex_lock,NULL);
	poll_init();
	sleep(1);
	net_config_init();
	TcpServerInit();
	sdcd_serial_open(9600, 8, 1, 0);
	system_init();
	timer_t timerID;
	createTimer(&timerID,0, 200);

    thr_id = pthread_create(&pthread_shell, NULL, shell_cmd_thread, (void*)p_name);
	if(thr_id < 0)
	{
		LOGE("shell_cmd_thread create error");
	}

	taskSpawn(1,(int)fifo_0_empty);	  

	/*int	file_size;
	FILE *adc_file;

    adc_file = fopen(Z_FILE_PATH, "r");
	fseek(adc_file, 0, SEEK_END);
	file_size = ftell(adc_file);
	z_data = (char *)malloc(sizeof(char)*	(file_size+1));
	z_data[file_size] = '\0';
	fseek(adc_file, 0, SEEK_SET);
    fread(z_data, file_size, 1, adc_file); 	
	set_math_expression_0(z_data);

    adc_file = fopen(X_FILE_PATH, "r");
	fseek(adc_file, 0, SEEK_END);
	file_size = ftell(adc_file);
	x_data = (char *)malloc(sizeof(char)*	(file_size+1));
	x_data[file_size] = '\0';
	fseek(adc_file, 0, SEEK_SET);
    fread(x_data, file_size, 1, adc_file); 	
	set_math_expression_1(x_data);

    adc_file = fopen(Y_FILE_PATH, "r");
	fseek(adc_file, 0, SEEK_END);
	file_size = ftell(adc_file);
	y_data = (char *)malloc(sizeof(char)*	(file_size+1));
	y_data[file_size] = '\0';
	fseek(adc_file, 0, SEEK_SET);
    fread(y_data, file_size, 1, adc_file); 	
	set_math_expression_2(y_data);

	add_symbol("lfz");
	add_symbol("lfx");
	add_symbol("lfy");
	add_symbol("lftx");
	add_symbol("lfty");
				
	set_register_symbol();*/

	while(!pthread_end)
	{ 		
		poll_loop(2);
		usleep(5);
	}
	
	pthread_end=1;
	printf("exit encore\n");
	//pthread_join(pthread_tcp,0);
	pthread_join(pthread_shell,0);
	pthread_join(pthread_fifo_0_empty,0);
	pthread_join(pthread_fifo_1_empty,0);
	pthread_join(pthread_fifo_2_empty,0);
	free(z_data);	
	free(x_data);	
	free(y_data);				
	sdcd_serial_close();			
	return 1;
}

void system_init(void)
{
	int mem_fd;
	void *map_base;

	mem_fd = open("/dev/mem", O_RDWR);
	if(mem_fd < 0){
		LOGE("/dev/mem open error (fifo_read_0)\r\n");
		return;
	}
	map_base = (void *)mmap((void *)0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, REGISTER_BASE_FIFO_READ_0);
	fifo_read_0 = (REGISTER_CONFIG_CS0 *)map_base;
	close(mem_fd);

	mem_fd = open("/dev/mem", O_RDWR);
	if(mem_fd < 0){
		LOGE("/dev/mem open error (fifo_read_1)\r\n");
		return;
	}
	map_base = (void *)mmap((void *)0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, REGISTER_BASE_FIFO_READ_1);
	fifo_read_1 = (REGISTER_CONFIG_CS0 *)map_base;
	close(mem_fd);

	mem_fd = open("/dev/mem", O_RDWR);
	if(mem_fd < 0){
		LOGE("/dev/mem open error (fifo_read_2)\r\n");
		return;
	}
	map_base = (void *)mmap((void *)0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, REGISTER_BASE_FIFO_READ_2);
	fifo_read_2 = (REGISTER_CONFIG_CS0 *)map_base;
	close(mem_fd);

	/*mem_fd = open("/dev/mem", O_RDWR);
	if(mem_fd < 0){
		LOGE("/dev/mem open error (fifo_read_3)\r\n");
		return;
	}
	map_base = (void *)mmap((void *)0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, REGISTER_BASE_FIFO_READ_3);
	fifo_read_3 = (REGISTER_CONFIG_CS0 *)map_base;
	close(mem_fd);

	mem_fd = open("/dev/mem", O_RDWR);
	if(mem_fd < 0){
		LOGE("/dev/mem open error (fifo_read_4)\r\n");
		return;
	}
	map_base = (void *)mmap((void *)0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, REGISTER_BASE_FIFO_READ_4);
	fifo_read_4 = (REGISTER_CONFIG_CS0 *)map_base;
	close(mem_fd);	*/

	mem_fd = open("/dev/mem", O_RDWR);
	if(mem_fd < 0){
		LOGE("/dev/mem open error (ad7656)\r\n");
		return;
	}
	map_base = (void *)mmap((void *)0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, REGISTER_BASE_AD7656);
	ad7656 = (REGISTER_CONFIG_AD7656 *)map_base;
	close(mem_fd);	

	LOGE("fpga register mapping ok\r\n");
	LOGE("FIFO_READ_IP0_VER = %d\n", fifo_read_0->ver);	
	LOGE("FIFO_READ_IP1_VER = %d\n", fifo_read_1->ver);
	LOGE("FIFO_READ_IP2_VER = %d\n", fifo_read_2->ver);		
	LOGE("ADC_IP_VER = %d\n", ad7656->ver);	
		
	ad7656->reg_reset = 0;
	usleep(1000);
	ad7656->reg_reset = 1;	

	fifo_read_0->reg_reset = 0;
	fifo_read_1->reg_reset = 0;
	fifo_read_2->reg_reset = 0;	
						
	usleep(1000);
	fifo_read_0->reg_reset = 1;
	fifo_read_1->reg_reset = 1;	
	fifo_read_2->reg_reset = 1;	
			
	usleep(1000);
	fifo_read_0->reg_reset = 0;	
	fifo_read_1->reg_reset = 0;	
	fifo_read_2->reg_reset = 0;	
				
	LOGE("FIFO_RESET OK\r\n");

	count_data.read = FIXED_COUNT;
	count_data.trigger = 3;
	ad7656->sensing_count = FIXED_COUNT;	
	ad7656->sens_interval = 10;		//100MHZ / 10 = 1MHZ = 0.0000001

	adc_cal_load();
	ld_current_ctrl_init();
	trigger_count_clear();

	adc0_0v_cal_flag = 0;	
	adc0_10v_cal_flag = 0;
	adc0_n10v_cal_flag = 0;

	adc1_0v_cal_flag = 0;	
	adc1_10v_cal_flag = 0;
	adc1_n10v_cal_flag = 0;	

	adc2_0v_cal_flag = 0;	
	adc2_10v_cal_flag = 0;
	adc2_n10v_cal_flag = 0;

	adc3_0v_cal_flag = 0;	
	adc3_10v_cal_flag = 0;
	adc3_n10v_cal_flag = 0;	

	adc4_0v_cal_flag = 0;	
	adc4_10v_cal_flag = 0;
	adc4_n10v_cal_flag = 0;	

	adc_trigger_count_0 = 0;
	adc_trigger_count_1 = 0;
	adc_trigger_count_2 = 0;
	adc_trigger_count_3 = 0;
	adc_trigger_count_4 = 0;		

	auto_adc_trigger_count_0 = 0;
	auto_adc_trigger_count_1 = 0;
	auto_adc_trigger_count_2 = 0;
	auto_adc_trigger_count_3 = 0;
	auto_adc_trigger_count_4 = 0;	

	trigger_end_flag = 0;
	pro_flag = 1;
	read_pro_flag = 1;
	read_end_flag = 0;
	real_time_sensing_flag = 0;
	timer_end_flag = 0;
	formula_calculation_flag = 0;
	adc0_formula_flag = 0;
	adc1_formula_flag = 0;
	adc2_formula_flag = 0;
	linearty_flag = 0;
	ld_state = 0;

 	uart_gpio_init();
	gpiops_init();
	gpiops_export(0);
	gpiops_dir_out(0);	
	formula_calculation_init();
	gpio_reg = 0;	
}


