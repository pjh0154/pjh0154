#include <stdio.h>
#include "../include/global.h"
#include "../include/serial-dev.h"
#include "../include/fpga_reg.h"
#include "../include/application.h"
#include "../include/ep.h"
#include "../include/timer.h"
#include "../include/serial-dev.h"
#include "../include/gpio-dev.h"
#include "../include/spi.h"
#include "../include/adc.h"
#include "../include/gpio.h"
#include "../include/signal_power.h"
#include "../include/dac.h"
#include "../include/ocp.h"
#include "../include/pca9665.h"
#include "../include/serial_dev_2.h"
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <time.h>
#include <sys/sysinfo.h>

#define Cprintf if(cprf) printf
#define Tprintf if(tprf) printf
#define Eprintf if(eprf) printf
#define Sprintf if(sprf) printf
#define Dprintf if(dprf) printf
#define Vprintf if(vprf) printf
#define ELprintf if(elrf) printf
#define AVprintf if(avrf) printf
#define Rprintf if(rprf) printf
#define Nprintf if(nprf) printf

int cprf=0;
int tprf=0;
int eprf=0;
int aprf=0;
int sprf=0;
int dprf=0;
int vprf=0;
int elrf=0;
int avrf=0;
int rprf=0;
int nprf=0;
int pprf=0;	//231206 Modify

//struct SpiDevice dev_0;
//struct SpiDevice dev_1;
//struct SpiDevice dev_2;	

void system_init(void);
void dac_init(void);
void musleep(int time);
void uwait(int time);
void ensis_delay(int time);

#define DEBUG_BUFFER_SIZE 64
#define PROMPT "[ENSIS]"
int taskSpawn(int priority,int taskfunc);
static long get_uptime(void);

unsigned char readch(void);
static int peek_character = -1;
static struct termios initial_settings;
static struct termios new_settings;
static int fd = -1;

unsigned char com_put = 0;

unsigned char ocp_test = 0;

//unsigned char key_index = 0;
extern unsigned char com_buffer_recive[COM_BUFFER_SIZE];
extern int com_buffer_recive_cnt;
extern unsigned char pat_index;

pthread_mutex_t mutex_lock;
pthread_t TaskID[20];
int TaskCnt = 0;
	
pthread_t	pthread_tcp;
pthread_t	pthread_shell;
pthread_t	pthread_ocp;
pthread_t	pthread_fifo_0_empty;
pthread_t	pthread_fifo_1_empty;
pthread_t	pthread_fifo_2_empty;
pthread_t	rs232_read;
int		pthread_end	= 0;
int		set_link	= 0;
bool		connect_flag	= false;

total_status_t *total_status;  
ring_q_t *queue_cal_result;  

//#define ADC_UNIT ((float)0.000000298023224)
//#define ADC_UNIT ((float)0.000000357627911)
//#define ADC_REG_1K 1000

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

void rcb_cmd_analysis(char *cmd)
{
    extern unsigned char pat_index;
    //extern unsigned char pat_string_index[1];
	extern unsigned char	pat_string_index[10];
	static unsigned char auto_manual_mode;
	unsigned char	stats_data[MIN_DATA_LEN];
	unsigned char fw_ver_index[MIN_DATA_LEN];   		//231027 Modify

    if(!strcmp(cmd, "[0B]")){ //ON,OFF 버튼 클릭
        if(pg_on_flag == PG_OFF){ //PG ON 동작 하기
			//rcb_mode;
			ts_rcb_mode_ocp_flag_on_set(total_status,RCB_OCP_MODE_ON);			
			if(auto_manual_mode == MANUAL_MODE)
			{
				cur_sensing_reset_flag = 1;	
				pg_on_flag = PG_ON;
				pat_index = 0;
				if(zone_select == 0x01) sprintf(pat_string_index, "T%d",pat_index);
				else if(zone_select == 0x02)    sprintf(pat_string_index, "H%d",pat_index);
				else if(zone_select == 0x03)    sprintf(pat_string_index, "M%d",pat_index);
				else sprintf(pat_string_index, "%d",pat_index);
				if(system_load_flag==0)
				{		
					system_load_flag = 1;			
					recipe_system_load();					
				}
				if(pg_on_flag != PG_OFF)    //OCP 발생 시 진행하지 않음
                { 
					recipe_funtion_load(pat_string_index);
					display_task(model_name,pattern_name);       //230929 Modify
				}
				else	system_load_flag = 0;							//231027 Modify	
				if(pg_on_flag == PG_OFF)	system_load_flag = 0; 		//231027 Modify
			}
			else
			{
				#ifdef RELIABILITY_ROOM	//231027 Modify	
				auto_run();	
				#endif							//231027 Modify	
			}
        }
        else{ //PG OFF 동작 하기
			#ifdef RELIABILITY_ROOM	//231027 Modify 
			if(pg_on_flag == AUTO_RUN)	snprintf(aging_result_string,sizeof(aging_result_string),"AGING_FAILED"); 
			auto_log_time_start = 2;
			auto_log_time();
			#endif	  
			ts_rcb_mode_ocp_flag_on_set(total_status,RCB_OCP_MODE_OFF);	//231027 Modify	
            recipe_funtion_load("OFF");
			display_task(model_name,"Ready");       //230929 Modify
			//pg_on_flag = PG_OFF;
        }
    }
    else if(!strcmp(cmd, "[DN]")){ // << 왼쪽 버튼 클릭, PREVIOUS PATTERN 동작
        if(pg_on_flag == PG_ON){
            cur_sensing_reset_flag = 1;
            if(zone_select == 0x01)
            {
                if(pat_index == 0)  pat_index = t_count-1;
                else    pat_index--;
                sprintf(pat_string_index, "T%d",pat_index);     
            }
            else if(zone_select == 0x02)
            {
                if(pat_index == 0)  pat_index = h_count-1;
                else    pat_index--;
                sprintf(pat_string_index, "H%d",pat_index);                     
            }
            else if(zone_select == 0x03)
            {
                if(pat_index == 0)  pat_index = m_count-1;
                else    pat_index--;
                sprintf(pat_string_index, "M%d",pat_index);                     
            }
            else
            {
                if(pat_index == 0)  pat_index = n_count-2;  
                else pat_index--;
                sprintf(pat_string_index, "%d",pat_index);              
            }
			if(pg_on_flag != PG_OFF)    //OCP 발생 시 진행하지 않음
			{ 			
				recipe_funtion_load(pat_string_index);
				display_task(NULL,pattern_name);       //230929 Modify
			}
			if(pg_on_flag == PG_OFF)	system_load_flag = 0; 
        }
		#ifdef RELIABILITY_ROOM	//231027 Modify 
		else if(pg_on_flag == AUTO_RUN)
		{
			memset(stats_data, 0, sizeof(stats_data));      
			snprintf(stats_data,sizeof(stats_data), "CYCLE=%d/%d",cycle_count,(signal_group.signal_config.display_cycle)); 
			display_task(NULL,stats_data);
		}
		else if(pg_on_flag == PG_OFF)
		{
			memset(stats_data, 0, sizeof(stats_data));      
			snprintf(stats_data,sizeof(stats_data), "%s",aging_result_string); 
			display_task(NULL,stats_data);			
		}
		#endif	
    }
    else if(!strcmp(cmd, "[UP]")){ // >> 오른쪽 버튼 클릭, NEXT PATTERN 동작
        if(pg_on_flag == PG_ON){
            cur_sensing_reset_flag = 1;
            if(zone_select == 0x01)
            {
                if(pat_index >= t_count-1)  pat_index = 0;  
                else    pat_index++; 
                sprintf(pat_string_index, "T%d",pat_index);
            }
            else if(zone_select == 0x02)
            {
                if(pat_index >= h_count-1)  pat_index = 0;  
                else    pat_index++; 
                sprintf(pat_string_index, "H%d",pat_index);         
            }
            else if(zone_select == 0x03)
            {
                if(pat_index >= m_count-1)  pat_index = 0;  
                else    pat_index++;
                sprintf(pat_string_index, "M%d",pat_index);                 
            }
            else 
            {
                if(pat_index >= n_count-2)  pat_index = 0;
                else pat_index++;   
                sprintf(pat_string_index, "%d",pat_index);          
            }

			if(pg_on_flag != PG_OFF)    //OCP 발생 시 진행하지 않음
			{ 			
				recipe_funtion_load(pat_string_index);    
				display_task(NULL,pattern_name);       //230929 Modify
			}  
			if(pg_on_flag == PG_OFF)	system_load_flag = 0; 
        }
		#ifdef RELIABILITY_ROOM	//231027 Modify 
		else if((pg_on_flag == AUTO_RUN) | (pg_on_flag == PG_OFF))
		{
			memset(fw_ver_index, 0, sizeof(fw_ver_index));      
			snprintf(fw_ver_index,sizeof(fw_ver_index), "FW=%d/%d",ep970_ver,pattern_generator->PG_STATUS & 0x000000ff); 
			memset(stats_data, 0, sizeof(stats_data));      
			snprintf(stats_data,sizeof(stats_data), "FILE=%s",file_cnt_string); 
			display_task(fw_ver_index,stats_data);
		}
		#endif		
    }
    else if(!strcmp(cmd, "[05]")) // AUTO / MANUAL MODE
	{
		if(auto_manual_mode == MANUAL_MODE)
		{
			auto_manual_mode = AUTO_MODE;			
			if(rcb_mode == TRUE)	display_task(NULL,"Auto_mode");       //230929 Modify
			else	printf("current_mode = Auto_mode\r\n");	
			//rcb_ack();
		}	
		else 
		{
			auto_manual_mode = MANUAL_MODE;	
			if(rcb_mode == TRUE)	display_task(NULL,"Manual_mode");       //230929 Modify	
			else	printf("current_mode = Manual_mode\r\n");
			//rcb_ack();
		}
    }	

    if((!strcmp(cmd, "[IN]")) || (!strcmp(cmd, "[0B]")) || (!strcmp(cmd, "[DN]")) || (!strcmp(cmd, "[UP]"))){ // 정상적인 명령어 일때 현재 PG의 상태를 RCB LCD에 업데이트
        char temp_str[64];
        char temp_name[16];
        int temp_str_tx_cnt;
        int i;
	
		if(!strcmp(cmd, "[0B]"))
		{
			//ON_OFF LED 동작 하는 곳
			memset(temp_str, 0, sizeof(temp_str));
			temp_str_tx_cnt = 0;
			if(pg_on_flag == PG_ON){ //ON_OFF LED ON, 프로토콜 아직 정의되지 않음
				temp_str[temp_str_tx_cnt++] = 0x02; //STX
				temp_str[temp_str_tx_cnt++] = '2'; //Mode
				temp_str[temp_str_tx_cnt++] = 'K'; //Command
				temp_str[temp_str_tx_cnt++] = 'E'; //Command
				temp_str[temp_str_tx_cnt++] = 'Y'; //Command
				temp_str[temp_str_tx_cnt++] = 'U'; //Command
				temp_str[temp_str_tx_cnt++] = 'P'; //Command			
				temp_str[temp_str_tx_cnt++] = ' '; //Command
				temp_str[temp_str_tx_cnt++] = 'O'; //Command
				temp_str[temp_str_tx_cnt++] = 'N'; //Command			
				temp_str[temp_str_tx_cnt++] = 0x03; //ETX
			}
			else if(pg_on_flag == AUTO_RUN){ //ON_OFF LED ON, 프로토콜 아직 정의되지 않음
				temp_str[temp_str_tx_cnt++] = 0x02; //STX
				temp_str[temp_str_tx_cnt++] = '2'; //Mode
				temp_str[temp_str_tx_cnt++] = 'K'; //Command
				temp_str[temp_str_tx_cnt++] = 'E'; //Command
				temp_str[temp_str_tx_cnt++] = 'Y'; //Command
				temp_str[temp_str_tx_cnt++] = 'U'; //Command
				temp_str[temp_str_tx_cnt++] = 'P'; //Command			
				temp_str[temp_str_tx_cnt++] = ' '; //Command
				temp_str[temp_str_tx_cnt++] = 'O'; //Command
				temp_str[temp_str_tx_cnt++] = 'N'; //Command
				temp_str[temp_str_tx_cnt++] = 0x03; //ETX      
			}
			else { //ON_OFF LED OFF, 프로토콜 아직 정의되지 않음
				temp_str[temp_str_tx_cnt++] = 0x02; //STX
				temp_str[temp_str_tx_cnt++] = '2'; //Mode
				temp_str[temp_str_tx_cnt++] = 'K'; //Command
				temp_str[temp_str_tx_cnt++] = 'E'; //Command
				temp_str[temp_str_tx_cnt++] = 'Y'; //Command
				temp_str[temp_str_tx_cnt++] = 'U'; //Command
				temp_str[temp_str_tx_cnt++] = 'P'; //Command			
				temp_str[temp_str_tx_cnt++] = ' '; //Command
				temp_str[temp_str_tx_cnt++] = 'O'; //Command
				temp_str[temp_str_tx_cnt++] = 'F'; //Command
				temp_str[temp_str_tx_cnt++] = 'F'; //Command			
				temp_str[temp_str_tx_cnt++] = 0x03; //ETX
			}
			for(i=0; ((i<temp_str_tx_cnt) && (i<sizeof(temp_str))); i++) putchar(temp_str[i]);
		}
		if(!strcmp(cmd, "[IN]"))
		{
			if((pg_on_flag == PG_ON) || (pg_on_flag == AUTO_RUN))	display_task(model_name,pattern_name);       //230929 Modify
			else 	display_task(model_name,"Ready");       //230929 Modify			
		}
		//rcb_ack();
    }
} 

void *shell_cmd_thread(char *cmd)
{

	if(!strcmp(cmd, "nprf"))
	{
		if(nprf == 0)
		{
			nprf = 1;
			printf("Nprf Print ON\r\n");
		}
		else
		{
			nprf = 0;
			printf("Nprf Print OFF\r\n");
		}				
	}	
	else if(!strcmp(cmd, "rprf"))
	{
		if(rprf == 0)
		{
			rprf = 1;
			printf("Rprf Print ON\r\n");
		}
		else
		{
			rprf = 0;
			printf("Rprf Print OFF\r\n");
		}				
	}	
	else if(!strcmp(cmd, "avrf"))
	{
		if(avrf == 0)
		{
			avrf = 1;
			printf("AVrf Print ON\r\n");
		}
		else
		{
			avrf = 0;
			printf("AVrf Print OFF\r\n");
		}				
	}	
	else if(!strcmp(cmd, "elrf"))
	{
		if(elrf == 0)
		{
			elrf = 1;
			printf("ELrf Print ON\r\n");
		}
		else
		{
			elrf = 0;
			printf("ELrf Print OFF\r\n");
		}				
	}		
	else if(!strcmp(cmd, "vprf"))
	{
		if(vprf == 0)
		{
			vprf = 1;
			printf("Vprf Print ON\r\n");
		}
		else
		{
			vprf = 0;
			printf("Vprf Print OFF\r\n");
		}				
	}	
	else if(!strcmp(cmd, "sprf"))
	{
		if(sprf == 0)
		{
			sprf = 1;
			printf("Sprf Print ON\r\n");
		}
		else
		{
			sprf = 0;
			printf("Sprf Print OFF\r\n");
		}				
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
	else if(!strcmp(cmd, "dprf"))
	{
		if(dprf == 0)
		{
			dprf = 1;
			printf("Dprf Print ON\r\n");
		}
		else
		{
			dprf = 0;
			printf("Dprf Print OFF\r\n");
		}				
	}	
	else if(!strcmp(cmd, "pprf"))	//231206 Modify
	{
		if(pprf == 0)
		{
			pprf = 1;
			printf("Pprf Print ON\r\n");
		}
		else
		{
			pprf = 0;
			printf("Pprf Print OFF\r\n");
		}				
	}	
	else if(!strcmp(cmd, "help"))
	{
		printf("---------------ADC--------------------------------\r\n");
		printf("ADC_INIT = adcinit\r\n");
		printf("DC15V_FOR_ADC_CAL = ads15v\r\n");
		printf("DC0V_FOR_ADC_CAL = ads0v\r\n");
		printf("ADS124_0_15V_INPUT(LDO_ELVDD)= ads015v 15.xxx\r\n");
		printf("ADS124_0_0V_INPUT(LDO_ELVDD)= ads000v 0.xxx\r\n");
		printf("ADS124_1_15V_INPUT(VOTP50)= ads115v 15.xxx\r\n");
		printf("ADS124_1_0V_INPUT(VOTP50)= ads100v 0.xxx\r\n");
		printf("ADS124_2_15V_INPUT(LM_SPARE1)= ads215v 15.xxx\r\n");
		printf("ADS124_2_0V_INPUT(LM_SPARE1)= ads200v 0.xxx\r\n");
		printf("ADS124_3_15V_INPUT(AVDD)= ads315v 15.xxx\r\n");
		printf("ADS124_3_0V_INPUT(AVDD)= ads300v 0.xxx\r\n");	
		printf("ADS124_4_15V_INPUT(ELVDD)= ads415v 15.xxx\r\n");
		printf("ADS124_4_0V_INPUT(ELVDD)= ads400v 0.xxx\r\n");		
		printf("ADS124_0_CAL_Calculation = ads0cal\r\n");	
		printf("ADS124_1_CAL_Calculation = ads1cal\r\n");
		printf("ADS124_2_CAL_Calculation = ads2cal\r\n");
		printf("ADS124_3_CAL_Calculation = ads3cal\r\n");
		printf("ADS124_4_CAL_Calculation = ads4cal\r\n");	
		printf("ADS124_0_CAL_VALUE_SAVE = ads0calsave\r\n");
		printf("ADS124_1_CAL_VALUE_SAVE = ads1calsave\r\n");
		printf("ADS124_2_CAL_VALUE_SAVE = ads2calsave\r\n");
		printf("ADS124_3_CAL_VALUE_SAVE = ads3calsave\r\n");
		printf("ADS124_4_CAL_VALUE_SAVE = ads4calsave\r\n");			
		printf("ADS124_4_CAL_VALUE_LOAD = adcload\r\n");
		printf("ADC_CAL_VALUE_PRINT = adprint\r\n");
		printf("---------------DAC--------------------------------\r\n");
		printf("CH DAC CAL = dcal 0(ch num)\r\n");
		printf("CH DAC OFFSET 0V~10V offset = offset0 0,x.xxxx  ex)0ffset0 0(ch num), x.xxx(value)\r\n");
		printf("CH DAC OFFSET 11V~25V offset = offset1 0,x.xxxx  ex)0ffset0 0(ch num), x.xxx(value)\r\n");	
		printf("CH DAC OFFSET 0V~-10V offset = offset2 0,x.xxxx  ex)0ffset0 0(ch num), x.xxx(value)\r\n");
		printf("CH DAC OFFSET -11V~-25V offset = offset3 0,x.xxxx  ex)0ffset0 0(ch num), x.xxx(value)\r\n");	
		printf("DAC CAL SAVE = daccalsave\r\n");
		printf("---------------I2C--------------------------------\r\n");
		printf("i2c_write = comw address,cnt,data0,data1,data2,data3 ex)comw 0260,4,02,04,05,06\r\n");
		printf("i2c_read = comr address,cnt ex) comr 0260,2\r\n");
		printf("i2c_eeprom_write_test = e2pw address,cnt,data0,data1,data2,data3 ex)e2pw 0000,4,02,04,05,06\r\n");
		printf("i2c_eeprom_read_test = e2pr address,cnt ex)e2pr 0000,4");
		printf("i2c_frequency_set = i2cfre num ex)i2cfre 0(100khz), i2cfre 1(125Khz), i2cfre 2(250Khz), i2cfre 3(500Khz), i2cfre 4(1Mhz)");
		printf("---------------ZONE_SELECT--------------------------------\r\n");
		printf("ZONE_SELECT =  zone num ex)zone 0(none),zone 1(T), zone 2(H), zone 3(M) \r\n");
		printf("---------------recipe------------------------------\r\n");
		printf("ELVDD,ELVSS,AVDD,ADD8_S,ADD8_G,VGH,VGL,VINIT,ASPARE1,ASPARE2,VDD11,VDD18,DSPARE1,DSPARE2,LOGIC,I2C,RSTB,TM,BISTEN,LPSPARE1,DELAY,fun,i2cset,OFF,SEN,OCP,PAT,OTP,I2CFRE\r\n");
		printf("--------------------TEST---------------------------\r\n");
		printf("RELAY_ON_TEST = relayn x ex)relayn 0(ELVDD), relayn 1(VDD8_S), relayn 2(VDD8_G), relayn 3(VGH), relayn 4(VGL), relayn 5(VINT)\r\n");
		printf("RELAY_OFF_TEST = relayf x ex)relayf 0(ELVDD), relayf 1(VDD8_S), relayf 2(VDD8_G), relayf 3(VGH), relayf 4(VGL), relayf 5(VINT)\r\n");
		printf("MUX_ON_TEST = muxn\r\n");
		printf("MUX_OFF_TEST = muxf\r\n");
		printf("REGISTER_Measurement_TEST = reg\r\n");
		printf("Measure all voltage current = sen\r\n");
		printf("GPIO_ON_OFF_TEST = gp0on / gp0of: RSTB_PIN, gp1on / gp1of: TM_PIN, gp2on / gp2of: BISTEN_PIN, gp3on / gp3of: LPSPARE1_PIN\r\n");
		printf("MTP_Variable_TEST = mtp num ex)mtp 0(AVDD) -> enter -> +- Adjust\r\n");
		printf("--------------------REGISTER_OFFSET---------------------------\r\n");
		printf("R_REGISTER_OFFSET = regr x ex)regr 0.002\r\n");
		printf("L_REGISTER_OFFSET = regl x ex)regl 0.002\r\n");
		printf("REGISTER_OFFSET_SAVE = regsave\r\n");	
		printf("--------------------AVDD_ELVSS_CUR_OFFSET---------------------------\r\n");	
		printf("AVDD_POSITIVE_CUR_CAL_OFFSET(mA) = cur1aoffset x.xxx\r\n");	
		printf("AVDD_NEGATIVE_CUR_CAL_OFFSET(mA) = ncur1aoffset x.xxx\r\n");	
		printf("ELVSS_POSITIVE_CUR_CAL_OFFSET(mA) = vsscur1aoffset x.xxx\r\n");	
		printf("ELVSS_NEGATIVE_CUR_CAL_OFFSET(mA) = nvsscur1aoffset x.xxx\r\n");
		printf("AVDD_CUR_OFFSET_INIT = vddoffsetinit\r\n");
		printf("AVDD_NEGATIVE_CUR_OFFSET_AUTO_SET = nvddoffset\r\n");
		printf("AVDD_POSITIVE_CUR_OFFSET_AUTO_SET = pvddoffset\r\n");				
		printf("AVDD_CUR_OFFSET_SAVE = vddoffsetsave\r\n");	
		printf("ELVSS_CUR_OFFSET_INIT = vssoffsetinit\r\n");
		printf("ELVSS_NEGATIVE_CUR_OFFSET_AUTO_SET = nvssoffset\r\n");
		printf("ELVSS_POSITIVE_CUR_OFFSET_AUTO_SET = pvssoffset\r\n");				
		printf("ELVSS_CUR_OFFSET_SAVE = vssoffsetsave\r\n");
		printf("--------------------ERROR_CODE_CHECK---------------------------\r\n");	
		printf("ERROR_CODE_CHECK = errorcode\r\n");
		printf("ERROR_CODE_INIT = errorinit\r\n");
		printf("SYSSTEM_ERROR_CODE_CHECK = syserrorcode\r\n");
		printf("SYSSTEM_ERROR_CODE_INIT = syserrorinit\r\n");
	}
	else if(!strcmp(cmd, "ep975help"))
	{
		printf("--------------------ADC TASK---------------------------\r\n");	
		printf("adc cal data init : adcinit\r\n");
		printf("adc cal auto task : adcautocal\r\n");
		printf("adc cal save task : ads0calsave / ads1calsave / ads2calsave / ads3calsave / ads4calsave\r\n");

		printf("adcoffset data print : adcoffsetprint\r\n");
		printf("adcoffset data init : adcoffsetinit\r\n");
		printf("adcoffset auto_task : adcoffsetauto\r\n");
		printf("adcoffset select task : adcoffset x ex)adcoffset 0(AVDD), adcoffset 1(ELVSS)\r\n");
		printf("adcoffset data save : adcoffsetsave\r\n");

		printf("adc cur cal data init : adcoffsetinit\r\n");
		printf("adc cur auto cal task : curautocal x ex) curautocal 0(AVDD), curautocal 1(ELVSS)\r\n");
		printf("adc cur avdd cal data save : curcalsave\r\n");
		printf("adc cur elvss cal data save : vsscurcalsave\r\n");
		printf("adc cur auto check task : curautocheck\r\n");
		printf("adc cur select check task : curcheck x,x,x,x  ex)curcheck 0,1,8000,1\r\n");
		printf("curcheck 0(AVDD),1(10ohm),8000(AVDD 8V),1(ELVSS 0.001V)\r\n");
		printf("0: 10ohm, 1: 25ohm, 2: 200ohm, 3: 50ohm, 4: 100ohm, 5: 400ohm, 6: 1Kohm, 7: OPEN\r\n");
		printf("--------------------DAC TASK---------------------------\r\n");	
		printf("dac cal data init : daccalinit\r\n");
		printf("dac auto cal task : dacautocal\r\n");
		printf("dac select cal task : autocal x ex)autocal 0(AVDD)\r\n");
		printf("dac all offset data init : dacalloffsetinit\r\n");
		printf("dac select offset data init : dacoffsetinit x ex)dacoffsetinit 0(AVDD)\r\n");
		printf("dac offset auto task : dacoffsetauto\r\n");
		printf("dac select offset task : caloffset x ex)caloffset 0(AVDD\r\n)");
		printf("--------------------ETC TASK---------------------------\r\n");	
		printf("ALL LDO CH CONNECT task : ldoon\r\n");
		printf("ALL LDO CH UNCONNECT task : ldooff\r\n");
		printf("MEASUREMENT RES SELECT task : ressen x ex)ressen 0(open), 1(10ohm), 2(short)\r\n");
		printf("OUTPUT to KEITHLEY CONNECT task : volchsel x ex)volchsel 0(AVDD)\r\n");		
	}		
	else if(!strcmp(cmd, "dacprint"))
	{
		int i = 0;
		for(i =0 ; i<14 ; i++)	
		{
			printf("dac_cal[%d].dac_0v_value = %f / dac_0v_step = %f\r\n", i, dac_cal[i].dac_0v_value, dac_cal[i].dac_0v_step);
			printf("dac_cal[%d].dac_10v_value = %f / dac_10v_step = %f\r\n", i, dac_cal[i].dac_10v_value, dac_cal[i].dac_10v_step);	
			printf("dac_cal[%d].dac_25v_value = %f / dac_25v_step = %f\r\n", i, dac_cal[i].dac_25v_value, dac_cal[i].dac_25v_step);	
			printf("dac_cal[%d].dac_n10v_value = %f / dac_n10v_step = %f\r\n", i, dac_cal[i].dac_n10v_value, dac_cal[i].dac_n10v_step);	
			printf("dac_cal[%d].dac_n25v_value = %f / dac_n25v_step = %f\r\n", i, dac_cal[i].dac_n25v_value, dac_cal[i].dac_n25v_step);			
			printf("dac_cal[%d].dac_0to10_ratio = %f / dac__0to10v_offset = %f\r\n", i, dac_cal[i].dac_0to10_ratio, dac_cal[i].dac_0to10_offset);
			printf("dac_cal[%d].dac_10to25_ratio = %f / dac__10vto25voffset = %f\r\n", i, dac_cal[i].dac_10to25_ratio, dac_cal[i].dac_10to25_offset);
			printf("dac_cal[%d].dac_0ton10_ratio = %f / dac__0ton10_offset = %f\r\n", i, dac_cal[i].dac_0ton10_ratio, dac_cal[i].dac_0ton10_offset);	
			printf("dac_cal[%d].dac_n10ton25_ratio = %f / dac__n10to_n25_offset = %f\r\n", i, dac_cal[i].dac_n10ton25_ratio, dac_cal[i].dac_n10to_n25_offset);		
		}		
	}	
	else if(!strcmp(cmd, "adcprint"))
	{
		int i = 0;
		printf("ads124_cal0.adc_15v_value = %f / ads124_cal0.adc_0v_value = %f\r\n", ads124_cal0.adc_15v_value, ads124_cal0.adc_0v_value);
		printf("ads124_cal0.adc_15v_step = %f / ads124_cal0.adc_0v_step = %f\r\n", ads124_cal0.adc_15v_step, ads124_cal0.adc_0v_step);
		printf("ads124_cal0.adc_ratio = %.10f / ads124_cal0.offset = %.10f\r\n", ads124_cal0.adc_p_ratio, ads124_cal0.adc_p_offset);

		printf("ads124_cal1.adc_15v_value = %f / ads124_cal1.adc_0v_value = %f\r\n", ads124_cal1.adc_15v_value, ads124_cal1.adc_0v_value);
		printf("ads124_cal1.adc_15v_step = %f / ads124_cal1.adc_0v_step = %f\r\n", ads124_cal1.adc_15v_step, ads124_cal1.adc_0v_step);
		printf("ads124_cal1.adc_ratio = %.10f / ads124_cal1.offset = %.10f\r\n", ads124_cal1.adc_p_ratio, ads124_cal1.adc_p_offset);

		printf("ads124_cal2.adc_15v_value = %f / ads124_cal2.adc_0v_value = %f\r\n", ads124_cal2.adc_15v_value, ads124_cal2.adc_0v_value);
		printf("ads124_cal2.adc_15v_step = %f / ads124_cal2.adc_0v_step = %f\r\n", ads124_cal2.adc_15v_step, ads124_cal2.adc_0v_step);
		printf("ads124_cal2.adc_ratio = %.10f / ads124_cal2.offset = %.10f\r\n", ads124_cal2.adc_p_ratio, ads124_cal2.adc_p_offset);

		printf("ads124_cal3.adc_15v_value = %f / ads124_cal3.adc_0v_value = %f\r\n", ads124_cal3.adc_15v_value, ads124_cal3.adc_0v_value);
		printf("ads124_cal3.adc_15v_step = %f / ads124_cal3.adc_0v_step = %f\r\n", ads124_cal3.adc_15v_step, ads124_cal3.adc_0v_step);
		printf("ads124_cal3.adc_ratio = %.10f / ads124_cal3.offset = %.10f\r\n", ads124_cal3.adc_p_ratio, ads124_cal3.adc_p_offset);		

		printf("ads124_cal4.adc_15v_value = %f / ads124_cal4.adc_0v_value = %f\r\n", ads124_cal4.adc_15v_value, ads124_cal4.adc_0v_value);
		printf("ads124_cal4.adc_15v_step = %f / ads124_cal4.adc_0v_step = %f\r\n", ads124_cal4.adc_15v_step, ads124_cal4.adc_0v_step);
		printf("ads124_cal4.adc_ratio = %.10f / ads124_cal4.offset = %.10f\r\n", ads124_cal4.adc_p_ratio, ads124_cal4.adc_p_offset);	

		printf("avdd_cur_cal.cur_100ma_value = %f / avdd_cur_cal.cur_0ma_value = %f\r\n", avdd_cur_cal.cur_100ma_value, avdd_cur_cal.cur_0ma_value);
		printf("avdd_cur_cal.cur_100ma_step = %f / avdd_cur_cal.cur_0ma_step = %f\r\n", avdd_cur_cal.cur_100ma_step, avdd_cur_cal.cur_0ma_step);
		printf("avdd_cur_cal.cur_100ma_ratio = %f / avdd_cur_cal.cur_100ma_offset = %f\r\n", avdd_cur_cal.cur_100ma_ratio, avdd_cur_cal.cur_100ma_offset);		
		printf("avdd_cur_cal.cur_100ma_user_offset = %f\r\n", avdd_cur_cal.cur_100ma_user_offset);

		printf("avdd_cur_cal.cur_1a_value = %f / avdd_cur_cal.cur_0ma_value = %f\r\n", avdd_cur_cal.cur_1a_value, avdd_cur_cal.cur_0a_value);
		printf("avdd_cur_cal.cur_1a_step = %f / avdd_cur_cal.cur_0ma_step = %f\r\n", avdd_cur_cal.cur_1a_step, avdd_cur_cal.cur_0a_step);
		printf("avdd_cur_cal.cur_1a_ratio = %f / avdd_cur_cal.cur_1a_offset = %f\r\n", avdd_cur_cal.cur_1a_ratio, avdd_cur_cal.cur_1a_offset);		
		printf("avdd_cur_cal.cur_1a_user_offset = %f\r\n", avdd_cur_cal.cur_1a_user_offset);
		printf("avdd_cur_cal.n_cur_1a_user_offset = %f\r\n", avdd_cur_cal.n_cur_1a_user_offset);

		printf("elvss_cur_cal.cur_100ma_value = %f / elvss_cur_cal.cur_0ma_value = %f\r\n", elvss_cur_cal.cur_100ma_value, elvss_cur_cal.cur_0ma_value);
		printf("elvss_cur_cal.cur_100ma_step = %f / elvss_cur_cal.cur_0ma_step = %f\r\n", elvss_cur_cal.cur_100ma_step, elvss_cur_cal.cur_0ma_step);
		printf("elvss_cur_cal.cur_100ma_ratio = %f / elvss_cur_cal.cur_100ma_offset = %f\r\n", elvss_cur_cal.cur_100ma_ratio, elvss_cur_cal.cur_100ma_offset);		
		printf("elvss_cur_cal.cur_100ma_user_offset = %f\r\n", elvss_cur_cal.cur_100ma_user_offset);

		printf("elvss_cur_cal.cur_1a_value = %f / elvss_cur_cal.cur_0a_value = %f\r\n", elvss_cur_cal.cur_1a_value, elvss_cur_cal.cur_0a_value);
		printf("elvss_cur_cal.cur_1a_step = %f / elvss_cur_cal.cur_0a_step = %f\r\n", elvss_cur_cal.cur_1a_step, elvss_cur_cal.cur_0a_step);
		printf("elvss_cur_cal.cur_1a_ratio = %f / elvss_cur_cal.cur_1a_offset = %f\r\n", elvss_cur_cal.cur_1a_ratio, elvss_cur_cal.cur_1a_offset);		
		printf("elvss_cur_cal.cur_1a_user_offset = %f\r\n", elvss_cur_cal.cur_1a_user_offset);	
		printf("elvss_cur_cal.n_cur_1a_user_offset = %f\r\n", elvss_cur_cal.n_cur_1a_user_offset);	

		for(i = 0 ; i < 25 ; i++)	printf("elvss_cur_offset_cal.p_offset[%d] = %f / elvss_cur_offset_cal.n_offset[%d] = %f\r\n", i,elvss_cur_offset_cal.p_offset[i], i, elvss_cur_offset_cal.n_offset[i]);	
		for(i = 0 ; i < 25 ; i++)	printf("avdd_cur_offset_cal.p_offset[%d] = %f / avdd_cur_offset_cal.n_offset[%d] = %f\r\n", i,avdd_cur_offset_cal.p_offset[i], i, avdd_cur_offset_cal.n_offset[i]);			
	}	
	else if(!strcmp(cmd, "adcoffsetprint"))			//231206 Modify
	{
		int i = 0;
		int ii = 0;
		for(i = 0 ; i < EX_CAL_LM_SPARE2+1 ; i++)
		{
			for(ii = 0 ; ii < ADC_CAL_N25V+1 ; ii++)	printf("vol_offset[%d].user_offset[%d] = %f\r\n", i,ii,vol_offset[i].user_offset[ii]);
		}
	}			
	else if(!strcmp(cmd, "errorinit"))
	{
		memset(&error_data, 0, sizeof(error_data));
		printf("error_data.i2c_communication = %d\r\n",error_data.i2c_communication);
		printf("error_data.i2c_read = %d\r\n",error_data.i2c_read);
		printf("error_data.avdd_cur = %d\r\n",error_data.avdd_cur);
		printf("error_data.elvss_cur = %d\r\n",error_data.elvss_cur);
		printf("error_data.ocp = %d\r\n",error_data.ocp);
		printf("error_data.vol_cur_adc = %d\r\n",error_data.vol_cur_adc);	
		printf("error_data.resistance_adc = %d\r\n",error_data.resistance_adc);			
	}	
	else if(!strcmp(cmd, "curcalinit"))
	{
		memset(&avdd_cur_cal, 0, sizeof(avdd_cur_cal));
		memset(&elvss_cur_cal, 0, sizeof(elvss_cur_cal));
		memset(&n_avdd_cur_cal, 0, sizeof(n_avdd_cur_cal));
		memset(&n_elvss_cur_cal, 0, sizeof(n_elvss_cur_cal));														
		printf("cur cal data init\r\n");	
	}
	else if(!strcmp(cmd, "adcoffsetinit"))	//231206 Modify
	{
		memset(&vol_offset, 0, sizeof(vol_offset)); 													
		printf("adc_offset data init\r\n");	
	}	
	else if(!strncmp(cmd, "keiinit",7))
	{
		keithley_init_task();
		printf("EP975 KEITHLEY_INIT_OK\r\n");	
	}	
	else if(!strcmp(cmd, "adcoffsetauto"))
	{
		int i = 0;
		unsigned char result = 0;
		usleep(100000);
		for(i = 0; i < EX_CAL_LM_SPARE2+1 ; i++)
		{
			result = adc_offset_task(i);
			printf("result = %x\r\n",result);
			//if(result>0)	break;
			if(result==DAC_AUTOCHECK_CANCEL)	break;
		}
		power_off(); 	
		cal_end_task();
	}	
	else if(!strcmp(cmd, "adcoffsetsave"))
	{
		FILE *adc_file;

		adc_file = fopen(ADC_OFFSET_FILE_PATH, "w+b");
		fwrite(&vol_offset, sizeof(vol_offset), 1, adc_file);
		fclose(adc_file);
		system("sync");	
		printf("adc_offset_data_save_OK\r\n");										
	}	
	else if(!strcmp(cmd, "123456"))
	{
		avdd_vol_2byte = 0;
		elvss_vol_2byte = 0;
		ADC_SELECT_DATA_READ_AVG(SEN_AVDD);
		avdd_vol_2byte = adc_sensing_value[SEN_AVDD];	
		if(vprf)	printf("AVDD = %d\r\n", avdd_vol_2byte);
		ADC_SELECT_DATA_READ_AVG(SEN_ELVSS);
		elvss_vol_2byte = adc_sensing_value[SEN_ELVSS];	
		if(vprf) 	printf("ELVSS = %d\r\n", elvss_vol_2byte);	
		com_task_ack(AVDD_ELVSS_VOLTAGE_REQUEST); 		
	}		
	else if(!strncmp(cmd, "adcoffset " ,10))
	{
		usleep(100000);
		memset(&vol_offset[atoi(cmd+10)], 0, sizeof(vol_offset[atoi(cmd+10)]));
		adc_offset_task(atoi(cmd+10));
		power_off(); 	
		cal_end_task();
	}
	else if(!strcmp(cmd, "123123"))
	{		
		short a = 0x0102;
		int i = 0;
		for(i = 0 ; i < 14 ; i++)
		{
			if((a>>i)&0x01 == 1)	printf("1\r\n");
			else	printf("0\r\n");
		}
	}	
	else if(!strcmp(cmd, "234234"))
	{
		int i = 0;
		for(i = 0 ; i<10 ; i++)
		{
			printf("es975_state.result_data.data_0[%d] = %d / es975_state.result_data.data_1[%d] = %d\r\n", i,es975_state.result_data.data_0[i],i,es975_state.result_data.data_1[i]);
		}
	}	
	else if(!strcmp(cmd, "123qwe"))
	{
		//memset(&category, 0, sizeof(category));
		//memset(&es975_state, 0, sizeof(es975_state));
		//unsigned char com_buffer[COM_BUFFER_SIZE] = {0};
		//memcpy(&category,&com_buffer[1],sizeof(category));	
		printf("category.dac_cal = %x\r\n",category.dac_cal);
		printf("category.adc_cal = %x\r\n",category.adc_cal);
		printf("category.cur_cal = %x\r\n",category.cur_cal);
		printf("category.vol_check = %x\r\n",category.vol_check);
		printf("category.cur_check.avdd_elvss_select = %x\r\n",category.cur_check.avdd_elvss_select);
		printf("category.cur_check.res_select = %x\r\n",category.cur_check.res_select);
		printf("category.res_select.avdd_elvss_select = %x\r\n",category.res_select.avdd_elvss_select);
		printf("category.res_select.res_select = %x\r\n",category.res_select.res_select);
	}	
	else if(!strncmp(cmd, "idea",4))
	{
		cal_end_task();
		printf("EP975 READY_MODE\r\n");	
	}
	else if(!strncmp(cmd, "curautocal ",11))
	{
		auto_cal_cur_task(atoi(cmd+11));
		power_off(); 
		cal_end_task();
	}		
	else if(!strncmp(cmd, "999 ",4))
	{
		printf("TP =%d\r\n", vol_cur_select_task(atoi(cmd+4)));
	}	
	else if(!strcmp(cmd, "curautocheck"))
	{	
		unsigned char vol_cur_select_task_result = 0; 
		vol_cur_select_task_result = vol_cur_select_task(SELECT_CUR);
		if(!vol_cur_select_task_result)
		{		
			ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);
			cur_sensing_compare_task(AVDD_CUR_SELECT, SELECT_25OHM, 8000, 1);	//AVDD : 8V / 0.32A
			usleep(100000);
			cur_sensing_compare_task(AVDD_CUR_SELECT, SELECT_25OHM, 4000, 1);	//AVDD : 4V / 0.16A
			usleep(100000);		
			cur_sensing_compare_task(AVDD_CUR_SELECT, SELECT_100OHM, 8000, 1);	//AVDD : 8V / 0.08A
			usleep(100000);
			cur_sensing_compare_task(AVDD_CUR_SELECT, SELECT_200OHM, 8000, 1);	//AVDD : 8V / 0.04A
			usleep(100000);
			cur_sensing_compare_task(AVDD_CUR_SELECT, SELECT_400OHM, 8000, 1);	//AVDD : 8V / 0.02A
			usleep(100000);
			cur_sensing_compare_task(ELVSS_CUR_SELECT, SELECT_25OHM, 1, -2500);	//ELVSS : -2.5V / -0.1A
			usleep(100000);
			cur_sensing_compare_task(ELVSS_CUR_SELECT, SELECT_100OHM, 1, -5000);	//ELVSS : -5V / -0.05A
			usleep(100000);	
			cur_sensing_compare_task(ELVSS_CUR_SELECT, SELECT_200OHM, 1, -2500);	//ELVSS : -2.5V / -0.0125A
			usleep(100000);		
			cur_sensing_compare_task(ELVSS_CUR_SELECT, SELECT_400OHM, 1, -2500);	//ELVSS : -2.5V / -0.00625A
			usleep(100000);		
			cur_sensing_compare_task(ELVSS_CUR_SELECT, SELECT_400OHM, 1, -1200);	//ELVSS : -1.2V / -0.003A
			usleep(100000);
			cur_sensing_compare_task(ELVSS_CUR_SELECT, SELECT_400OHM, 1, -500);	//ELVSS : -0.5V / -0.00125A
			usleep(100000);									
			power_off(); 
			cal_end_task();
			ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);  
		}
	}				
	else if(!strncmp(cmd, "curcheck ",9))
	{
		char num = 0;	
		char sel = 0;
		unsigned char data[100];
		short avdd = 0;
		short elvss = 0;
		double cur_data = 0;
		memset(&data,0, sizeof(data));
		strtok(cmd, " ");
		num = atoi(strtok(NULL, ","));
		sel = atoi(strtok(NULL, ","));	
		avdd = atoi(strtok(NULL, ","));	
		elvss = atoi(strtok(NULL, ","));	
		cur_sensing_compare_task(num, sel, avdd, elvss);
		power_off(); 
   	 	cal_end_task();
	}	
	else if(!strncmp(cmd, "ressel ",7))
	{
		char num = 0;	
		char sel = 0;
		unsigned char data[100];
		double cur_data = 0;
		memset(&data,0, sizeof(data));
		strtok(cmd, " ");
		num = atoi(strtok(NULL, ","));
		sel = atoi(strtok(NULL, ","));		

		unsigned char vol_cur_select_task_result = 0; 
		memset(&data,0, sizeof(data));
		vol_cur_select_task_result = vol_cur_select_task(SELECT_CUR);
		if(!vol_cur_select_task_result)
		{  
			data[0] = AUTO_CAL_CUR_TASK;	
			data[1] = num;	            // AVDD or ELVSS SELECT
			data[2] = sel;            // Resistance SELECT
			ex_port_send_task(data,AUTO_CAL_CUR_TASK_LEN);	
			ex_port_serial_task();				
		}	
	}
	else if(!strncmp(cmd, "daccheck ",9))
	{
		usleep(100000);
		printf("DAC_CAL_CHECK_START\r\n");
		printf("result[%d] = %d\r\n", atoi(cmd+9),dac_vol_check(atoi(cmd+9)));
		power_off(); 
		cal_end_task();
	}
	else if(!strcmp(cmd, "dacautocheck"))
	{
		int i = 0;
		unsigned char result = 0;
		printf("DAC_CAL_CHECK_START\r\n");
		for(i = 0; i < 14 ; i++)
		{		
			result = dac_vol_check(i);
			printf("result[%d] = %x\r\n", i,result);
			usleep(100000);
			if(result==DAC_AUTOCHECK_CANCEL)	break;
		}
		power_off(); 
		cal_end_task();
		printf("DAC_CAL_CHECK_END\r\n");
	}				
	else if(!strncmp(cmd, "dacoffsetauto",13))
	{
		unsigned char result = 0;
		int i = 0;
		for(i = 0; i < 14 ; i++)
		{
			dac_cal[i].dac_10to25_offset = 0;	
			dac_cal[i].dac_0to10_offset = 0; 
			dac_cal[i].dac_0ton10_offset = 0; 
			dac_cal[i].dac_n10to_n25_offset = 0; 				
		}
		usleep(100000);
		for(i = 0; i < 14 ; i++)	
		{
			result = dac_auto_offset_cal(i);
			printf("result[%d] = %x\r\n", i,result);
			usleep(100000);
			if(result==DAC_AUTOCHECK_CANCEL)	break;			
		}
		power_off(); 
		cal_end_task();
	}
	else if(!strncmp(cmd, "dacautocal",10))
	{
		int i = 0;
		unsigned char result = 0;
		usleep(100000);
		for(i = 0; i < 14 ; i++)
		{
			result = dac_auto_cal(i);
			printf("result = %x\r\n",result);
			//if(result>0)	break;
			if(result==DAC_AUTOCHECK_CANCEL)	break;
		}
		power_off(); 	
		cal_end_task();
	}		
	else if(!strncmp(cmd, "autocal ",8))
	{
		//dac_Manual_cal0();	
		//keithley_init_task();
		usleep(100000);
		dac_auto_cal(atoi(cmd+8));
		power_off(); 	
		cal_end_task();
	}	
	else if(!strncmp(cmd, "caloffset ",10))
	{
		int i = 0;		
		i = atoi(cmd+10);
		dac_cal[i].dac_10to25_offset = 0;	
		dac_cal[i].dac_0to10_offset = 0; 
		dac_cal[i].dac_0ton10_offset = 0; 
		dac_cal[i].dac_n10to_n25_offset = 0; 			
		//keithley_init_task();
		usleep(100000);
		dac_auto_offset_cal(atoi(cmd+10));
		power_off(); 	
		cal_end_task();
	}
	else if(!strncmp(cmd, "dacoffsetinit ",14))
	{
		int i = 0;		
		i = atoi(cmd+14);
		dac_cal[i].dac_10to25_offset = 0;	
		dac_cal[i].dac_0to10_offset = 0; 
		dac_cal[i].dac_0ton10_offset = 0; 
		dac_cal[i].dac_n10to_n25_offset = 0; 		
	}	
	else if(!strcmp(cmd, "dacalloffsetinit"))
	{
		int i = 0;		
		i = atoi(cmd+10);
		dac_cal[i].dac_10to25_offset = 0;	
		dac_cal[i].dac_0to10_offset = 0; 
		dac_cal[i].dac_0ton10_offset = 0; 
		dac_cal[i].dac_n10to_n25_offset = 0; 		
	}		
	else if(!strncmp(cmd, "ldoon",5))
	{
		ldo_all_on_off_task(EX_LDO_ALL_ON); 			
	}	
	else if(!strncmp(cmd, "ldooff",6))
	{
		ldo_all_on_off_task(EX_LDO_ALL_OFF); 			
	}	
	else if(!strncmp(cmd, "adcautocal",10))
	{
		usleep(100000);
		ex_auto_cal_task();
		power_off(); 	
		cal_end_task();
	}	
	else if(!strncmp(cmd, "ressen ",7))
	{
		char num = 0;	
		unsigned char data[100];
		memset(&data,0, sizeof(data));
		num = atoi(cmd+7);	
		data[0] = EX_REG_TASK;	
    	data[1] = num;	
    	ex_port_send_task(data,EX_REG_TASK_LEN);	
		printf("REG_SET_OK\r\n");	
	}		
	else if(!strncmp(cmd, "volchsel ",9))
	{
		char num = 0;	
		unsigned char data[100];
		unsigned vol_cur_select_task_result = 0;
		memset(&data,0, sizeof(data));
		num = atoi(cmd+9);			
		vol_cur_select_task_result = vol_cur_select_task(SELECT_VOL);
		if(!vol_cur_select_task_result)
		{			
			cal_end_task();
			data[0] = AUTO_CAL_TASK;	
			data[1] = num;	
			ex_port_send_task(data,AUTO_CAL_TASK_LEN);	
			printf("AUTO_CAL_VOL_SET_OK\r\n");	
			ex_port_serial_task();		
		}
	}			
	else if(!strcmp(cmd, "pgcheck"))
	{
		memset(&ex_input_vol_adc_result, 0, sizeof(ex_input_vol_adc_result));	
		memset(&ex_adc_total_result, 0, sizeof(ex_adc_total_result));
		ex_adc_sen_index =	EX_AVDD; 
		ex_vol_set_task(ex_adc_sen_index);
    	ex_input_vol_compare_task();	

	}			
	else if(!strcmp(cmd, "ccc"))
	{
		int i = 0;
		int j = 0;
		char ready = 0;
		unsigned char write_data;    
		int rm_init_start = 0;	//231015 Modify
		int rm_init_end = 0;	//231015 Modify
		int rm_init_time = ADC_TIMEOUT;	//231015 Modify

		ads124s08_rm->ADC_CONTROL = ADC_Control_RESET;
		rm_init_start = timeout_msclock();	//231015 Modify
		while(1)
		{
			if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_RESET))
			{
				ready = 1;
				break;
			}				
			//usleep(5000);	//231013 Modify
			//j++;
			
			//if(j > 5) return 0;
			//if(j > 5000000) break;	//231013 Modify
			usleep(1);	//231013 Modify
			rm_init_end = timeout_msclock() - rm_init_start;	//231015 Modify
			if(rm_init_end >= rm_init_time) break;	//231015 Modify			
		}
		
		usleep(50000);	
		if(!ready)	
		{
			printf("ADS124_RESET_FAIL\r\n");
		}		
	}
	else if(!strcmp(cmd, "eee"))
	{
		int i = 0;
		int j = 0;
		char ready = 0;
		unsigned char write_data;    
		int rm_init_start = 0;	//231015 Modify
		int rm_init_end = 0;	//231015 Modify
		int rm_init_time = ADC_TIMEOUT;	//231015 Modify		
		for(i = 0 ; i <= 18 ; i++)
		{
			ads124s08_rm->ADC_REG_ADDRESS = i;
			//ads124s08_rm->ADC_CONTROL = ADC_Control_START_PIN_HIGH;
			ads124s08_rm->ADC_CONTROL = ADC_Control_REG_READ;
			j=0;
			ready = 0;
			rm_init_start = 0;	//231015 Modify
			rm_init_end = 0;	//231015 Modify
			rm_init_start = timeout_msclock();	//231015 Modify			
			while(1)
			{
				if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_REG_READ))
				{
					ready = 1;
					break;
				}				
				//usleep(5000);	//231013 Modify	
				/*j++;
				usleep(1);	//231013 Modify	
				//if(j > 5) return 0;
				if(j > 5000000) break;	//231013 Modify*/
				usleep(1);	//231015 Modify
				rm_init_end = timeout_msclock() - rm_init_start;	//231015 Modify
				if(rm_init_end >= rm_init_time) break;	//231015 Modify					
			}
			if(!ready)	
			{
				printf("ADS124_RM_READ_FAIL\r\n");
			}			
			//data = ads124s08_sensing->ADC_REG_READ;
			printf("REG%d = %x\r\n", i, ads124s08_rm->ADC_REG_READ);
			usleep(10000);	
		}			
	}	
	else if(!strcmp(cmd, "rrr"))
	{	
		extern int ADC_RM_REG_SETTING_VALUE_LOAD(ADC_REG_ADDRESS_NUM address, unsigned char *init_value);
		int i = 0;
		int j = 0;
		char ready = 0;
		unsigned char write_data;    
		int rm_init_start = 0;	//231015 Modify
		int rm_init_end = 0;	//231015 Modify
		int rm_init_time = ADC_TIMEOUT;	//231015 Modify			
		for(i = 0 ; i <= 18 ; i++)
		{
			if(ADC_RM_REG_SETTING_VALUE_LOAD((ADC_REG_ADDRESS_NUM)i, &write_data))
			{	
				printf("i = %d / write_data = %x\r\n", i, write_data);
				ads124s08_rm->ADC_REG_ADDRESS = i;
				ads124s08_rm->ADC_REG_WRITE = write_data;
				//ads124s08_rm->ADC_CONTROL = ADC_Control_START_PIN_HIGH;
				ads124s08_rm->ADC_CONTROL = ADC_Control_REG_WRITE; 
				j=0;
				ready = 0;
				rm_init_start = 0;	//231015 Modify
				rm_init_end = 0;	//231015 Modify
				rm_init_start = timeout_msclock();	//231015 Modify				
				while(1)
				{
					if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_REG_WRITE))
					{
						ready = 1;
						break;
					}				
					//usleep(5000);	//231013 Modify	
					/*j++;
					usleep(1);	//231013 Modify				
					//if(j > 5) return 0;
					if(j > 5000000) break;	//231013 Modify*/
					usleep(1);	//231015 Modify
					rm_init_end = timeout_msclock() - rm_init_start;	//231015 Modify
					if(rm_init_end >= rm_init_time) break;	//231015 Modify						
				}
				if(!ready)	
				{
					printf("ADS124_RM_WRITE_FAIL\r\n");
				}								          
			} 
			usleep(10000);	
		}
	}
	else if(!strcmp(cmd, "bbb"))
	{
		printf("gpio->GPIO_DATA = %x\r\n",gpio->GPIO_DATA);		
	}			
	else if(!strcmp(cmd, "aaa"))
	{
		printf("pattern_name = %s\r\n",pattern_name);
		printf("model_name = %s\r\n",model_name);		
	}	
	else if(!strcmp(cmd, "www"))
	{
		printf("srecipe_size = %d\r\n", sizeof(srecipe));
		model_name_check();
	}	
	else if(!strcmp(cmd, "syserrorinit"))
	{
		memset(&system_error, 0, sizeof(system_error));
		printf("system_error.recipe = %d\r\n",system_error.recipe);
		printf("system_error.recipe_copy = %d\r\n",system_error.recipe_copy);
		printf("system_error.pvssig_rs232 = %d\r\n",system_error.pvssig_rs232);
		printf("system_error.nvssig_rs232 = %d\r\n",system_error.nvssig_rs232);
		printf("system_error.delay_task = %d\r\n",system_error.delay_task);
		printf("system_error.log_index_file_create = %d\r\n",system_error.log_index_file_create);	
		printf("system_error.model_download_file_open = %d\r\n",system_error.model_download_file_open);		
				
	}	
	else if(!strcmp(cmd, "syserrorcode"))
	{
		printf("system_error.recipe = %d\r\n",system_error.recipe);
		printf("system_error.recipe_copy = %d\r\n",system_error.recipe_copy);
		printf("system_error.pvssig_rs232 = %d\r\n",system_error.pvssig_rs232);
		printf("system_error.nvssig_rs232 = %d\r\n",system_error.nvssig_rs232);
		printf("system_error.delay_task = %d\r\n",system_error.delay_task);
		printf("system_error.log_index_file_create = %d\r\n",system_error.log_index_file_create);	
		printf("system_error.model_download_file_open = %d\r\n",system_error.model_download_file_open);				
	}			
	else if(!strcmp(cmd, "vddoffsetinit"))
	{
		memset(&avdd_cur_offset_cal, 0, sizeof(avdd_cur_offset_cal));	
		printf("avddoffsetinit_ok\r\n");
	}	
	else if(!strcmp(cmd, "nvddoffset"))
	{
		nvdd_cur_offset();
	}	
	else if(!strcmp(cmd, "pvddoffset"))
	{
		pvdd_cur_offset();	
	}		
	else if(!strcmp(cmd, "vddoffsetsave"))
	{
		FILE *vdd_offset_file;

		vdd_offset_file = fopen(AVDD_CUR_OFFSET_FILE_PATH, "w+b");
		fwrite(&avdd_cur_offset_cal, sizeof(avdd_cur_offset_cal), 1, vdd_offset_file);
		fclose(vdd_offset_file);
		system("sync");	
		printf("avdd_cur_offset_data_save_OK\r\n");	
	}

	else if(!strcmp(cmd, "vssoffsetinit"))
	{
		memset(&elvss_cur_offset_cal, 0, sizeof(elvss_cur_offset_cal));	
		printf("vssoffsetinit_ok\r\n");
	}	
	else if(!strcmp(cmd, "nvssoffset"))
	{
		nvss_cur_offset();
	}	
	else if(!strcmp(cmd, "pvssoffset"))
	{
		pvss_cur_offset();	
	}		
	else if(!strcmp(cmd, "vssoffsetsave"))
	{
		FILE *vss_offset_file;

		vss_offset_file = fopen(ELVSS_CUR_OFFSET_FILE_PATH, "w+b");
		fwrite(&elvss_cur_offset_cal, sizeof(elvss_cur_offset_cal), 1, vss_offset_file);
		fclose(vss_offset_file);
		system("sync");	
		printf("elvss_cur_offset_data_save_OK\r\n");	
	}	
	else if(!strcmp(cmd, "errorcode"))
	{
		printf("error_data.avdd_cur = %d\r\n", error_data.avdd_cur);
		printf("error_data.elvss_cur = %d\r\n", error_data.elvss_cur);
		printf("error_data.i2c_communication = %d\r\n", error_data.i2c_communication);
		printf("error_data.i2c_read = %d\r\n", error_data.i2c_read);
		printf("error_data.ocp = %d\r\n", error_data.ocp);
	}	
	else if(!strcmp(cmd, "qqq"))
	{
		int i = 0;
		for(i = 0 ; i < 5000 ; i++)
		{
			model_recipe_read();
			printf("COUNT = %d\r\n",i);
		}
	}					
	else if(!strcmp(cmd, "usedelaystate"))
	{
		printf("USE_DELAY=%d\r\n", USER_DELAY);
	}
	else if(!strncmp(cmd, "usedelay ",8))
	{
		USER_DELAY = atof(cmd+8);
		printf("USE_DELAY=%d\r\n", USER_DELAY);
	}					 
	else if(!strcmp(cmd, "ff"))
	{
		rgb_voltage_request();
	}		
	else if(!strcmp(cmd, "mtptest"))
	{
		rgb_voltage_request();		
	}			
	else if(!strcmp(cmd, "adcreset"))
	{
		ads124s08_sensing_init();
		ads124s08_ads124s08_rm_init();		
	}
	else if(!strncmp(cmd, "i2cfre ",7))
	{
		char num = 0;
		num = atof(cmd+7);
		i2c_frequency_set(num);
	}	
	else if(!strcmp(cmd, "muxn"))
	{
		gpio_enble->GPIO_DATA = 0xffffffff;
		printf("GPIO_ENBLE ON\r\n");
	}	
	else if(!strcmp(cmd, "muxf"))
	{
		gpio_enble->GPIO_DATA = 0x00000000;
		printf("GPIO_ENBLE OFF\r\n");
	}
	else if(!strncmp(cmd, "relayn ",7))
	{
		int num = 0;
		num = atof(cmd+7);
		num += 28;
		gpiops_set_value(num,1);
		if(num == 28)	printf("ELVDD RELAY ON\r\n");
		else if(num == 29)	printf("VDD8_S RELAY ON\r\n");
		else if(num == 30)	printf("VDD8_G RELAY ON\r\n");
		else if(num == 31)	printf("VGH RELAY ON\r\n");
		else if(num == 32)	printf("VGL RELAY ON\r\n");
		else if(num == 33)	printf("VINT RELAY ON\r\n");
	}
	else if(!strncmp(cmd, "relayf ",7))
	{
		int num = 0;
		num = atof(cmd+7);
		num += 28;
		gpiops_set_value(num,0);
		if(num == 28)	printf("ELVDD RELAY OFF\r\n");
		else if(num == 29)	printf("VDD8_S RELAY OFF\r\n");
		else if(num == 30)	printf("VDD8_G RELAY OFF\r\n");
		else if(num == 31)	printf("VGH RELAY OFF\r\n");
		else if(num == 32)	printf("VGL RELAY OFF\r\n");
		else if(num == 33)	printf("VINT RELAY OFF\r\n");
	}																	
	else if(!strncmp(cmd, "zone ",5))
	{
		zone_select = atof(cmd+5);	
		if(cprf)	printf("ZONE = 	[%d]\r\n", zone_select);					
	}
	else if(!strcmp(cmd, "e2ptest"))
	{
		unsigned char slave_address_w = i2c_test_wr_addr>>1;
		unsigned short reg_address_w = 0x0000;
		unsigned char write_buffer[100] = {0};
		int write_byte = 2;

		unsigned char slave_address = i2c_test_rd_addr>>1;
		unsigned short reg_address = 0x0000;
		unsigned char read_buffer[100] = {0};
		int read_byte = 2;

		int i = 0;
		for(i = 0 ; i < 10 ; i++)
		{
			write_buffer[0] =0x00;
			write_buffer[1] =0x00;					
			i2c_write(slave_address_w, reg_address_w, write_buffer, write_byte);
			i2c_read(slave_address, reg_address, read_buffer, read_byte);					
			printf("i2c_data[%d] = [0x00] : [0x%x] / [0x00] : [0x%x]\r\n", i,read_buffer[0], read_buffer[1]);										
			write_buffer[0] =0x12;
			write_buffer[1] =0x34;					
			i2c_write(slave_address_w, reg_address_w, write_buffer, write_byte);
			i2c_read(slave_address, reg_address, read_buffer, read_byte);
			printf("i2c_data[%d] = [0x12] : [0x%x] / [0x34] : [0x%x]\r\n", i,read_buffer[0], read_buffer[1]);
			
		}								
	}	
	else if(!strncmp(cmd, "e2pw ",5))
	{
		unsigned char slave_address = i2c_test_wr_addr>>1;
		//unsigned char slave_address = 0x90>>1;
		int reg_address = 0;
		int write_byte = 0;
		char write_buffer[10] = {0};  
		int i = 0;
		
		strtok(cmd, " ");
		reg_address = strtol(strtok(NULL, ","),NULL,16);
		write_byte = atoi(strtok(NULL, ","));
		write_buffer[0] = strtol(strtok(NULL, ","),NULL,16);
		if(write_byte > 1) write_buffer[1] = strtol(strtok(NULL, ","),NULL,16);
		if(write_byte > 2) write_buffer[2] = strtol(strtok(NULL, ","),NULL,16);
		if(write_byte > 3) write_buffer[3] = strtol(strtok(NULL, ","),NULL,16);
		if(write_byte > 4) write_buffer[4] = strtol(strtok(NULL, ","),NULL,16);
		if(write_byte > 5) write_buffer[5] = strtol(strtok(NULL, ","),NULL,16);
		if(write_byte > 6) write_buffer[6] = strtol(strtok(NULL, ","),NULL,16);
		if(write_byte > 7) write_buffer[7] = strtol(strtok(NULL, ","),NULL,16);
			
		if(cprf) printf("reg_address = %x / write_byte = %d\r\nwite_buffer[0] = %x\r\nwite_buffer[1] = %x\r\nwite_buffer[2] = %x\r\nwite_buffer[3] = %x\r\nwite_buffer[4] = %x\r\nwite_buffer[5] = %x\r\nwite_buffer[6] = %x\r\nwite_buffer[7] = %x\r\n"
		, reg_address, write_byte, write_buffer[0],write_buffer[1],write_buffer[2],write_buffer[3],write_buffer[4],write_buffer[5],write_buffer[6],write_buffer[7]);
		i2c_write(slave_address,reg_address,write_buffer,write_byte);	
		usleep(USER_DELAY);   
	}
	else if((!strncmp(cmd, "e2pr ",5)))
	{
		unsigned char slave_address = i2c_test_rd_addr>>1;
		//unsigned char slave_address = 0x91>>1;	
		int reg_address = 0;
		int read_byte = 0;
		char read_buffer[10] = {0};
		int i = 0;

		strtok(cmd, " ");
		reg_address = strtol(strtok(NULL, ","),NULL,16);
		read_byte = atoi(strtok(NULL, ","));				
		i2c_read(slave_address,reg_address,read_buffer,read_byte);
		for(i = 0 ; i < read_byte ; i++)
		{
			printf("reg_address = %x / READ_DATA = [%x]\r\n", reg_address++, read_buffer[i]);
		}
		usleep(USER_DELAY);   
	}						
	else if(!strncmp(cmd, "comw ",5))
	{
		unsigned char slave_address = i2c_wr_addr>>1;
		//unsigned char slave_address = 0x90>>1;
		int reg_address = 0;
		int write_byte = 0;
		char write_buffer[10] = {0};  
		int i = 0;
		
		strtok(cmd, " ");
		reg_address = strtol(strtok(NULL, ","),NULL,16);
		write_byte = atoi(strtok(NULL, ","));
		write_buffer[0] = strtol(strtok(NULL, ","),NULL,16);
		if(write_byte > 1) write_buffer[1] = strtol(strtok(NULL, ","),NULL,16);
		if(write_byte > 2) write_buffer[2] = strtol(strtok(NULL, ","),NULL,16);
		if(write_byte > 3) write_buffer[3] = strtol(strtok(NULL, ","),NULL,16);
		if(write_byte > 4) write_buffer[4] = strtol(strtok(NULL, ","),NULL,16);
		if(write_byte > 5) write_buffer[5] = strtol(strtok(NULL, ","),NULL,16);
		if(write_byte > 6) write_buffer[6] = strtol(strtok(NULL, ","),NULL,16);
		if(write_byte > 7) write_buffer[7] = strtol(strtok(NULL, ","),NULL,16);
			
		if(cprf) printf("reg_address = %x / write_byte = %d\r\nwite_buffer[0] = %x\r\nwite_buffer[1] = %x\r\nwite_buffer[2] = %x\r\nwite_buffer[3] = %x\r\nwite_buffer[4] = %x\r\nwite_buffer[5] = %x\r\nwite_buffer[6] = %x\r\nwite_buffer[7] = %x\r\n"
		, reg_address, write_byte, write_buffer[0],write_buffer[1],write_buffer[2],write_buffer[3],write_buffer[4],write_buffer[5],write_buffer[6],write_buffer[7]);
		i2c_write(slave_address,reg_address,write_buffer,write_byte);	
		usleep(USER_DELAY);    	
	}
	else if((!strncmp(cmd, "comr ",5)))
	{
		unsigned char slave_address = i2c_rd_addr>>1;
		//unsigned char slave_address = 0x91>>1;	
		int reg_address = 0;
		int read_byte = 0;
		char read_buffer[10] = {0};
		int i = 0;

		strtok(cmd, " ");
		reg_address = strtol(strtok(NULL, ","),NULL,16);
		read_byte = atoi(strtok(NULL, ","));				
		i2c_read(slave_address,reg_address,read_buffer,read_byte);
		for(i = 0 ; i < read_byte ; i++)
		{
			printf("reg_address = %x / READ_DATA = [%x]\r\n", reg_address++, read_buffer[i]);
		}
		usleep(USER_DELAY);   
	}						
	else if(!strcmp(cmd, "ocptest"))
	{
		ocp_test = 1;
	}
	else if(!strcmp(cmd, "ocpoff"))
	{
		ocp_test = 0;
	}							
	else if(!strncmp(cmd, "rr ",3))
	{
		int adc_reg = 0;
		adc_reg = atoi(cmd+3);
		gpio->GPIO_DATA = 	adc_reg;
		printf("gpio->GPIO_DATA = %x\r\n", gpio->GPIO_DATA);	
	}	
	else if(!strcmp(cmd, "gpionum"))
	{
		printf("gpio->GPIO_DATA = %x\r\n", gpio->GPIO_DATA);
	}						
	else if(!strcmp(cmd, "reg"))
	{
		//resistance_measurement_1k();
		//usleep(5);
		int t, dt;
		t = msclock();
		resistance_measurement_1();
		gpio->GPIO_DATA = 0x100;	//231027 Modify 
		dt = msclock() - t;	
		//gpio_reg_init();
		printf("time_data = %dms\r\n", dt);
		//printf("reg_value_r = %dohm\r\n", adc_res_value_r_1);
		//printf("reg_value_l = %dohm\r\n", adc_res_value_l_1); 	
		printf("reg_value_r = %.4fohm\r\n", (float)adc_res_value_r_1/10000);
		printf("reg_value_l = %.4fohm\r\n", (float)adc_res_value_l_1/10000); 								
	}				
	else if(!strcmp(cmd, "sen"))
	{
		int t, dt;
		t = msclock();				
		//ADC_AUTO_DATA_READ();
		total_sen_flag = 1;
		dt = msclock() - t;	
		//printf("time_data = %dus\r\n", dt);				
		//ADC_DATA_PRINT();
	}					
	else if(!strcmp(cmd, "daccalinit"))
	{
		memset(&dac_cal, 0, sizeof(dac_cal));
		printf("DAC_CAL_INIT_OK!!\r\n");
		printf("dac_cal[0].dac_25v_value = %f / dac_cal[0].dac_25v_step = %f\r\n", dac_cal[0].dac_25v_value, dac_cal[0].dac_25v_step);	
		printf("dac_cal[0].dac_10v_value = %f / dac_cal[0].dac_10v_step = %f\r\n", dac_cal[0].dac_10v_value, dac_cal[0].dac_10v_step);
		printf("dac_cal[0].dac_0v_value = %f / dac_cal[0].dac_0v_step = %f\r\n", dac_cal[0].dac_0v_value, dac_cal[0].dac_0v_step);	
		printf("dac_cal[0].dac_n10v_value = %f / dac_cal[0].dac_n10v_step = %f\r\n", dac_cal[0].dac_n10v_value, dac_cal[0].dac_n10v_step);	
		printf("dac_cal[0].dac_n25v_value = %f / dac_cal[0].dac_n25v_step = %f\r\n", dac_cal[0].dac_n25v_value, dac_cal[0].dac_n25v_step);
		printf("/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////\r\n");										

		printf("dac_cal[0].dac_0to10_ratio = %f\r\n", dac_cal[0].dac_0to10_ratio);
		printf("dac_cal[0].dac_10to25_ratio = %f\r\n", dac_cal[0].dac_10to25_ratio);
		printf("dac_cal[0].dac_0ton10_ratio = %f\r\n", dac_cal[0].dac_0ton10_ratio);
		printf("dac_cal[0].dac_n10ton25_ratio = %f\r\n", dac_cal[0].dac_n10ton25_ratio);						
	}
	else if(!strcmp(cmd, "ocpon"))
	{
		ocp_flag_on=1;
		printf("OCP_FLAG ON\r\n");
	}
	else if(!strcmp(cmd, "ocpof"))
	{
		ocp_flag_on=0;
		printf("OCP_FLAG OFF\r\n");
	}				
	else if(!strcmp(cmd, "reci"))
	{
		model_recipe_read();
	}	
	else if(!strcmp(cmd, "adsre"))
	{
		ads124s08_sensing_init();
		ads124s08_ads124s08_rm_init();
		printf("ads124s08 reset_ok\r\n");
	}	
	else if(!strcmp(cmd, "gp0on"))
	{
		logic_gpio_ctl("RSTB",1);	
	}
	else if(!strcmp(cmd, "gp0of"))
	{
		logic_gpio_ctl("RSTB",0);	
	}	
	else if(!strcmp(cmd, "gp1on"))
	{
		logic_gpio_ctl("TM",1);	
	}
	else if(!strcmp(cmd, "gp1of"))
	{
		logic_gpio_ctl("TM",0);	
	}
	else if(!strcmp(cmd, "gp2on"))
	{
		logic_gpio_ctl("BISTEN",1);	
	}
	else if(!strcmp(cmd, "gp2of"))
	{
		logic_gpio_ctl("BISTEN",0);	
	}	
	else if(!strcmp(cmd, "gp3on"))
	{
		logic_gpio_ctl("LPSPARE1",1);	
	}
	else if(!strcmp(cmd, "gp3of"))
	{
		logic_gpio_ctl("LPSPARE1",0);	
	}	
	else if(!strcmp(cmd, "gp4on"))
	{
		logic_gpio_ctl("ALL",1);	
	}
	else if(!strcmp(cmd, "gp4of"))
	{
		logic_gpio_ctl("ALL",0);	
	}	
	else if(!strcmp(cmd, "otc"))
	{
		i2c_com_otp_loading();				
	}			
	else if(!strncmp(cmd, "i2cf ",5))
	{
		int frequency = 0;
		frequency = atof(cmd+5);
		printf("frequency = %d\r\n",frequency );
		i2c_set_frequency(frequency);			
	}									
	else if(!strncmp(cmd, "vi2c ",5))
	{
		short a = 0;
		char c[3]  = {0,};
		float vol = 0;
		int t, dt;
		unsigned char i=0,*id;
		
		vol = atof(cmd+5);									
		//pattern_generator->PG_CONTROL = OUT_EN_ON;
		a = (short)(((65535/10)*(((float)(vol/1000))+4.8)));	
		printf("!dac = %x\r\n", (short)a);	
		Power_Supply_Voltage_load();
		c[2] = a &0xff;
		c[1] = (a>>8) &0xff;
		c[0] = 24;	
		dac_set(DAC0,c,3);	
	}				
	else if(!strncmp(cmd, "logic ",6))
	{
		short a = 0;
		char c[3]  = {0,};
		float vol = 0;
		int t, dt;
		unsigned char i=0,*id;
		
		vol = atof(cmd+6);									
		//pattern_generator->PG_CONTROL = OUT_EN_ON;

		a = (short)(((65535/10)*(((vol/1000))+4.8)));	
		printf("!dac = %x\r\n",a);	
		Power_Supply_Voltage_load();
		c[2] = a &0xff;
		c[1] = (a>>8) &0xff;
		c[0] = 25;	
		dac_set(DAC0,c,3);	
	}
	else if(!strncmp(cmd, "sp0 ",4))
	{
		short a = 0;
		char c[3]  = {0,};
		float vol = 0;
		int t, dt;
		unsigned char i=0,*id;
		
		vol = atof(cmd+4);									
		//pattern_generator->PG_CONTROL = OUT_EN_ON;

		a = (short)(((65535/10)*(((vol/1000))+4.8)));	
		printf("!dac = %x\r\n",a);	
		c[2] = a &0xff;
		c[1] = (a>>8) &0xff;
		c[0] = 26;	
		dac_set(DAC0,c,3);	
	}
	else if(!strncmp(cmd, "sp1 ",4))
	{
		short a = 0;
		char c[3]  = {0,};
		float vol = 0;
		int t, dt;
		unsigned char i=0,*id;
		
		vol = atof(cmd+4);									
		//pattern_generator->PG_CONTROL = OUT_EN_ON;

		a = (short)(((65535/10)*(((vol/1000))+4.8)));	
		printf("!dac = %x\r\n",a);	
		c[2] = a &0xff;
		c[1] = (a>>8) &0xff;
		c[0] = 27;	
		dac_set(DAC0,c,3);	
	}																				
	else if(!strncmp(cmd, "else ",5))
	{
		int a = 0;
		int i = 0;
		a = atof(cmd+5);
		for(i = 0 ; i<14 ; i++)	
		{
			signal_group.signal_config.dc_voltage[i] = atof(cmd+5);
			printf("TP = %d\r\n", signal_group.signal_config.dc_voltage[i]);
		}
		//SET_DAC_OUTPUT_VALUE(a);				
	}
	else if(!strcmp(cmd, "alon"))
	{
		Power_Supply_Voltage_load();
		int i = 0;
		for(i=0 ; i< 14 ; i++)	
		{
			SET_DAC_OUTPUT_VALUE(i);

			printf("TP1 = %d\r\n", i);
		}	
		gpio_enble->GPIO_DATA = 0xffffffff;
		printf("GPIO_ENBLE ON\r\n");

		gpiops_set_value(RELAY_ELVDD,RELAY_ON);
		gpiops_set_value(RELAY_VDD8_S,RELAY_ON);
		gpiops_set_value(RELAY_VDD8_G,RELAY_ON);
		gpiops_set_value(RELAY_VGH,RELAY_ON);
		gpiops_set_value(RELAY_VGL,RELAY_ON);
		gpiops_set_value(RELAY_VINIT,RELAY_ON);  				

		ocp->OCP_CONTROL = 1<<0;	
		pattern_generator->PG_OCP_DELAY = 10;
		ocp->OCP_EN_MASK = 	0x3fff;	 
	}			
	else if(!strncmp(cmd, "elon ",5))
	{
		int a = 0;
		a = atof(cmd+5);
		Power_Supply_Voltage_load();
		SET_DAC_OUTPUT_VALUE(a);
		gpio_enble->GPIO_DATA |= (0x00000001<<(a&0x3fff));
		if(a ==ELVDD)	gpiops_set_value(RELAY_ELVDD,RELAY_ON);
		else if(a ==ADD8_S)	gpiops_set_value(RELAY_VDD8_S,RELAY_ON);
		else if(a ==ADD8_G)	gpiops_set_value(RELAY_VDD8_G,RELAY_ON);
		else if(a ==VGH)	gpiops_set_value(RELAY_VGH,RELAY_ON);
		else if(a ==VGL)	gpiops_set_value(RELAY_VGL,RELAY_ON);	
		else if(a ==VINIT)	gpiops_set_value(RELAY_VINIT,RELAY_ON);	
		printf("TP1 = %d\r\n", a);				
	}
	else if(!strncmp(cmd, "i2c ",4))
	{
		short a = 0;
		char c[3]  = {0,};
		int t, dt;
				
		signal_group.signal_config.dc_voltage[0] = atof(cmd+3);							
	
		a = ((65535/10)*(((double)signal_group.signal_config.dc_voltage[0]/1000)+5));
		printf("!dac = %x\r\n", (short)a);	
		c[2] = a &0xff;
		c[1] = (a>>8) &0xff;
		c[0] = 24;
		//transfer_spi_dac(dev_0.fd,c,3);	
		dac_set(DAC0,c,3);								
	}			
	else if(!strcmp(cmd, "daccalsave"))
	{
		FILE *dac_file;

		dac_file = fopen(DAC_CAL_FILE_PATH, "w+b");
		fwrite(&dac_cal, sizeof(dac_cal), 1, dac_file);
		fclose(dac_file);
		system("sync");	
		LOGE("dac_data_save_OK\r\n");										
	}	
	else if(!strcmp(cmd, "daccalload"))
	{
		dac_cal_load();	
	}	
	else if(!strncmp(cmd, "mtp ",4))
	{
		dac_MTP(atoi(cmd+4));
	}										
	else if(!strncmp(cmd, "dcal ",5))
	{
		//dac_Manual_cal0();	
		dac_Manual_cal(atoi(cmd+4));
	}
	else if(!strncmp(cmd, "offset0 ",8))
	{
		char *dac_num = 0;
		char *dac_value = 0;
		int dca_num_int = 0;
		double dac_value_double = 0;
		dac_num = strtok(cmd, " ");
		dac_num = strtok(NULL, ",");
		dac_value = strtok(NULL, " ");
		dca_num_int = atoi(dac_num);
		dac_value_double = atof(dac_value);
		
		dac_cal[dca_num_int].dac_0to10_offset = (dac_value_double*1000) * dac_cal[dca_num_int].dac_0to10_ratio;				
		printf("dac_cal[%d].dac_0to10_offset = %f\r\n", dca_num_int, dac_cal[dca_num_int].dac_0to10_offset);
	}
	else if(!strncmp(cmd, "offset1 ",8))
	{
		char *dac_num = 0;
		char *dac_value = 0;
		int dca_num_int = 0;
		double dac_value_double = 0;
		dac_num = strtok(cmd, " ");
		dac_num = strtok(NULL, ",");
		dac_value = strtok(NULL, " ");
		dca_num_int = atoi(dac_num);
		dac_value_double = atof(dac_value);
		
		dac_cal[dca_num_int].dac_10to25_offset = (dac_value_double*1000)*dac_cal[dca_num_int].dac_10to25_ratio;				
		printf("dac_cal[%d].dac_10to25_offset = %f\r\n", dca_num_int, dac_cal[dca_num_int].dac_10to25_offset);
	}
	else if(!strncmp(cmd, "offset2 ",8))
	{
		char *dac_num = 0;
		char *dac_value = 0;
		int dca_num_int = 0;
		double dac_value_double = 0;
		dac_num = strtok(cmd, " ");
		dac_num = strtok(NULL, ",");
		dac_value = strtok(NULL, " ");
		dca_num_int = atoi(dac_num);
		dac_value_double = atof(dac_value);
		
		dac_cal[dca_num_int].dac_0ton10_offset = (dac_value_double*1000)* dac_cal[dca_num_int].dac_0ton10_ratio;				
		printf("dac_cal[%d].dac_0ton10_offset = %f\r\n", dca_num_int, dac_cal[dca_num_int].dac_0ton10_offset);
	}
	else if(!strncmp(cmd, "offset3 ",8))
	{
		char *dac_num = 0;
		char *dac_value = 0;
		int dca_num_int = 0;
		double dac_value_double = 0;
		dac_num = strtok(cmd, " ");
		dac_num = strtok(NULL, ",");
		dac_value = strtok(NULL, " ");
		dca_num_int = atoi(dac_num);
		dac_value_double = atof(dac_value);
		
		dac_cal[dca_num_int].dac_n10to_n25_offset = (dac_value_double*1000)* dac_cal[dca_num_int].dac_n10ton25_ratio;				
		printf("dac_cal[%d].dac_n10to_n25_offset = %f\r\n", dca_num_int, dac_cal[dca_num_int].dac_n10to_n25_offset);
	}															
	else if(!strcmp(cmd, "cc"))
	{
		int t, dt;
		unsigned char i=0,*id;
		unsigned short *delay;
		signal_group.signal_config.sequence_timing[0] = 5000;
		delay = (unsigned short *)&signal_group.signal_config.sequence_timing[0];				
		for(i=0; i<4; i++) {
			t = msclock();
			usleep(10000);										
			dt = msclock() - t;					
			if(*delay<dt) dt=*delay;
			musleep((*delay-dt) * 1000);
			//printf("seq delay ms =%d  / %d\n",dt, *delay);
			//delay++;
		}				
	}			
	else if(!strcmp(cmd, "cl"))
	{
		ocp->OCP_CONTROL = 1<<0;	
						
		printf("ocp_clean_ok\r\n");
	}						
	else if(!strcmp(cmd, "ocpre"))
	{
		printf("OCP_STATUS = %x\r\n", ocp->OCP_STATUS);				
		printf("OCP_CONTROL = %x\r\n", ocp->OCP_CONTROL);
		printf("EACH_CHANNEL_OCP_FLAG = %x\r\n", ocp->EACH_CHANNEL_OCP_FLAG);				
		printf("OCP_EN_MASK = %x\r\n", ocp->OCP_EN_MASK);
	}						
	else if(!strcmp(cmd, "pp"))
	{
		pattern_generator->PG_OCP_DELAY = 10;
		pattern_generator->PG_CONTROL = 1<<0;
		ocp->OCP_EN_MASK = 	0x3fff;			
		printf("ocp_set!_ok\r\n");
	}			
	else if(!strcmp(cmd, "ch"))
	{
		printf("ocp = %x / ocp_ch = %x\r\n", ocp->OCP_STATUS, ocp->EACH_CHANNEL_OCP_FLAG);
	}			
	else if(!strncmp(cmd, "elocp ",6))
	{
		int i = 0;
		short a = 0;
		short b = 0;
		char c[3]  = {0,};				
		signal_group.signal_config.el_over_current_value = atof(cmd+6);
		a = (65535/10)*(((((float)signal_group.signal_config.el_over_current_value/1000)*0.25)*((1+(820000/13000)*0.1)))+5);
		b = -a;
		printf("EL_OCP_PO = %x / EL_OCP_NE = %x\r\n", a,b);			
		c[2] = a &0xff;
		c[1] = (a>>8) &0xff;
		c[0] = 22;
		//transfer_spi_dac(dev_0.fd,c,3);	
		dac_set(DAC0,c,3);
		c[2] = b &0xff;
		c[1] = (b>>8) &0xff;
		c[0] = 23;
		//transfer_spi_dac(dev_0.fd,c,3);	
		dac_set(DAC0,c,3);				
	}
	else if(!strncmp(cmd, "apocp ",6))
	{
		int i = 0;
		short a = 0;
		short b = 0;
		char c[3]  = {0,};				
		signal_group.signal_config.ap_over_current_value = atof(cmd+6);
		a = (65535/10)*(((((float)signal_group.signal_config.ap_over_current_value/1000)*0.5)*((1+(1000000/10200)*0.1)))+5);
		b = -a;
		printf("AP_OCP_PO = %x / AP_OCP_NE = %x\r\n", a,b);			
		c[2] = a &0xff;
		c[1] = (a>>8) &0xff;
		c[0] = 24;
		dac_set(DAC2,c,3);				
		////////////////////////////////////////////	
		c[2] = b &0xff;
		c[1] = (b>>8) &0xff;
		c[0] = 25;
		dac_set(DAC2,c,3);
		////////////////////////////////////////////
		c[2] = a &0xff;
		c[1] = (a>>8) &0xff;
		c[0] = 26;
		dac_set(DAC2,c,3);
		////////////////////////////////////////////
		c[2] = b &0xff;
		c[1] = (b>>8) &0xff;
		c[0] = 27;
		dac_set(DAC2,c,3);
		////////////////////////////////////////////												
	}											
	else if(!strcmp(cmd, "poff"))
	{
		power_off();  						
	}																																																															
	else if(!strncmp(cmd, "aa1",3))
	{
		int i = 0;
		int j = 0;
		int ready = 0;
		unsigned int adc_read_data;
		unsigned short crc;	
		for(i = 0 ; i <5 ; i++)
		{
			ads124s08_sensing->ADC_CONTROL = ADC_Control_START_PIN_HIGH;
			//ads124s08_sensing->ADC_CONTROL = ADC_Control_START_PIN_LOW;						
			ads124s08_sensing->ADC_REG_ADDRESS = 2;
			ads124s08_sensing->ADC_REG_WRITE = (((i << 4) & 0xf0) | (MUX_SEL_AINCOM & 0x0f));
			ads124s08_sensing->ADC_CONTROL = ADC_Control_REG_WRITE;  													
			j = 0;
			while(1)
			{
				if((ads124s08_sensing->ADC_STATUS & (1<<1)))
				{
					//ready = 1;
					break;
				}				
				usleep(1000);
				j++;
				//if(j > 5) return 0;
				if(j > 5) break;
			}
			ads124s08_sensing->ADC_CONTROL = ADC_Control_DATA_READ; 
			j = 0;	
			ready = 0;
			while(1)
			{
				if((ads124s08_sensing->ADC_STATUS & (1<<0)))
				{						
					ready = 1;
					break;
				}				
				usleep(1000);
				j++;
				//if(j > 5) return 0;
				if(j > 5) break;
			}														
			if(ready)	
			{
				adc_read_data = ads124s08_sensing->ADC_DATA_READ;
				crc = ADC_CRC8((unsigned char *)&adc_read_data, sizeof(adc_read_data));	 
				if(!crc)
				{
					adc_read_data = (adc_read_data >>8);
					printf("crc_ok = %x\r\n", adc_read_data);
				}
				else	printf("crc_ng\r\n"); 	
			}
			else	printf("READY NG\r\n");					
		} 
		ads124s08_sensing->ADC_CONTROL = ADC_Control_START_PIN_LOW;
	}																																
	else if(!strcmp(cmd, "adscalpri"))
	{
		ADC_AUTO_DATA_READ_FOR_CAL();
		usleep(10);
		ADC_DATA_FOR_CAL_PRINT();	
	}																																																																				
	else if(!strcmp(cmd, "adcinit"))
	{
		memset(&ads124_cal0, 0, sizeof(ads124_cal0));
		memset(&ads124_cal1, 0, sizeof(ads124_cal1));
		memset(&ads124_cal2, 0, sizeof(ads124_cal2));
		memset(&ads124_cal3, 0, sizeof(ads124_cal3));
		memset(&ads124_cal4, 0, sizeof(ads124_cal4));																
		printf("ads124 cal data init\r\n");
	}															
	else if(!strcmp(cmd, "ads15v"))
	{
		int a = 0;
		int i = 0;
		for(i = 0 ; i<14 ; i++)	
		{
			signal_group.signal_config.dc_voltage[i] = 15000;
		}			
		Power_Supply_Voltage_load();
		for(i=0 ; i< 14 ; i++)	
		{
			SET_DAC_OUTPUT_VALUE(i);
		}					

	}
	else if(!strcmp(cmd, "ads0v"))
	{
		int a = 0;
		int i = 0;
		for(i = 0 ; i<14 ; i++)	
		{
			signal_group.signal_config.dc_voltage[i] = 0;
		}			
		Power_Supply_Voltage_load();
		for(i=0 ; i< 14 ; i++)	
		{
			SET_DAC_OUTPUT_VALUE(i);
		}			
	}
	else if(!strncmp(cmd, "cur0a ", 6))
	{
		ADC_AUTO_DATA_READ_FOR_CAL();
		avdd_cur_cal.cur_0ma_value = atof(cmd+6);
		avdd_cur_cal.cur_0ma_step = adc_sensing_value_for_cal[SEN_ELIDD_100mA];
		avdd_cur_cal.cur_0a_value = atof(cmd+6);
		avdd_cur_cal.cur_0a_step = adc_sensing_value_for_cal[SEN_ELIDD];		
		printf("cur_mA = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELIDD_100mA], avdd_cur_cal.cur_0a_step, avdd_cur_cal.cur_0a_value);		
		printf("cur_A = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELIDD], avdd_cur_cal.cur_0ma_step, avdd_cur_cal.cur_0ma_value);								
	}
	else if(!strncmp(cmd, "ncur0a ", 7))
	{
		//ADC_AUTO_DATA_READ_FOR_CAL();
		ADC_AUTO_DATA_READ_FOR_CAL_N();
		n_avdd_cur_cal.cur_0ma_value = atof(cmd+6);
		n_avdd_cur_cal.cur_0ma_step = adc_sensing_value_for_cal[SEN_ELIDD_100mA];
		n_avdd_cur_cal.cur_0a_value = atof(cmd+6);
		n_avdd_cur_cal.cur_0a_step = adc_sensing_value_for_cal[SEN_ELIDD];		
		printf("cur_mA = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELIDD_100mA], n_avdd_cur_cal.cur_0a_step, n_avdd_cur_cal.cur_0a_value);		
		printf("cur_A = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELIDD], n_avdd_cur_cal.cur_0ma_step, n_avdd_cur_cal.cur_0ma_value);								
	}	
	else if(!strncmp(cmd, "vsscur0a ", 9))
	{
		ADC_AUTO_DATA_READ_FOR_CAL();
		elvss_cur_cal.cur_0ma_value = atof(cmd+9);
		elvss_cur_cal.cur_0ma_step = adc_sensing_value_for_cal[SEN_ELISS_100mA];
		elvss_cur_cal.cur_0a_value = atof(cmd+9);
		elvss_cur_cal.cur_0a_step = adc_sensing_value_for_cal[SEN_ELISS];		
		printf("elvss_cur_mA = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELISS_100mA], elvss_cur_cal.cur_0a_step, elvss_cur_cal.cur_0a_value);		
		printf("elvss_cur_A = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELISS], elvss_cur_cal.cur_0ma_step, elvss_cur_cal.cur_0ma_value);								
	}	
	else if(!strncmp(cmd, "nvsscur0a ", 10))
	{
		//ADC_AUTO_DATA_READ_FOR_CAL();
		ADC_AUTO_DATA_READ_FOR_CAL_N();
		n_elvss_cur_cal.cur_0ma_value = atof(cmd+10);
		n_elvss_cur_cal.cur_0ma_step = abs(adc_sensing_value_for_cal[SEN_ELISS_100mA]);
		n_elvss_cur_cal.cur_0a_value = atof(cmd+10);
		n_elvss_cur_cal.cur_0a_step = abs(adc_sensing_value_for_cal[SEN_ELISS]);		
		printf("n_elvss_cur_mA = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELISS_100mA], n_elvss_cur_cal.cur_0a_step, n_elvss_cur_cal.cur_0a_value);		
		printf("n_elvss_cur_A = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELISS], n_elvss_cur_cal.cur_0ma_step, n_elvss_cur_cal.cur_0ma_value);								
	}		
	else if(!strncmp(cmd, "cur1a ", 6))
	{
		ADC_AUTO_DATA_READ_FOR_CAL();
		avdd_cur_cal.cur_1a_value = atof(cmd+6);
		avdd_cur_cal.cur_1a_step = adc_sensing_value_for_cal[SEN_ELIDD];
		printf("cur_1a = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELIDD], avdd_cur_cal.cur_1a_step, avdd_cur_cal.cur_1a_value);								
	}
	else if(!strncmp(cmd, "ncur1a ", 7))
	{
		//ADC_AUTO_DATA_READ_FOR_CAL();
		ADC_AUTO_DATA_READ_FOR_CAL_N();
		n_avdd_cur_cal.cur_1a_value = atof(cmd+6);
		n_avdd_cur_cal.cur_1a_step = adc_sensing_value_for_cal[SEN_ELIDD];
		printf("cur_1a = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELIDD], n_avdd_cur_cal.cur_1a_step, n_avdd_cur_cal.cur_1a_value);								
	}	
	else if(!strncmp(cmd, "vsscur1a ", 9))
	{
		ADC_AUTO_DATA_READ_FOR_CAL();
		elvss_cur_cal.cur_1a_value = atof(cmd+9);
		elvss_cur_cal.cur_1a_step = adc_sensing_value_for_cal[SEN_ELISS];
		printf("elvss_cur_1a = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELISS], elvss_cur_cal.cur_1a_step, elvss_cur_cal.cur_1a_value);								
	}
	else if(!strncmp(cmd, "nvsscur1a ", 10))
	{
		//ADC_AUTO_DATA_READ_FOR_CAL();
		ADC_AUTO_DATA_READ_FOR_CAL_N();
		n_elvss_cur_cal.cur_1a_value = atof(cmd+10);
		n_elvss_cur_cal.cur_1a_step = abs(adc_sensing_value_for_cal[SEN_ELISS]);
		printf("elvss_cur_1a = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELISS], n_elvss_cur_cal.cur_1a_step, n_elvss_cur_cal.cur_1a_value);								
	}			
	else if(!strncmp(cmd, "cur50ma ", 8))
	{
		ADC_AUTO_DATA_READ_FOR_CAL();
		avdd_cur_cal.cur_100ma_value = atof(cmd+8);
		avdd_cur_cal.cur_100ma_step = adc_sensing_value_for_cal[SEN_ELIDD_100mA];
		printf("ads124[1] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELIDD_100mA], avdd_cur_cal.cur_100ma_step, avdd_cur_cal.cur_100ma_value);								
	}
	else if(!strncmp(cmd, "ncur50ma ", 9))
	{
		//ADC_AUTO_DATA_READ_FOR_CAL();
		ADC_AUTO_DATA_READ_FOR_CAL_N();
		n_avdd_cur_cal.cur_100ma_value = atof(cmd+8);
		n_avdd_cur_cal.cur_100ma_step = adc_sensing_value_for_cal[SEN_ELIDD_100mA];
		printf("ads124[1] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELIDD_100mA], n_avdd_cur_cal.cur_100ma_step, n_avdd_cur_cal.cur_100ma_value);								
	}	
	else if(!strncmp(cmd, "vsscur50ma ", 11))
	{
		ADC_AUTO_DATA_READ_FOR_CAL();
		elvss_cur_cal.cur_100ma_value = atof(cmd+11);
		elvss_cur_cal.cur_100ma_step = adc_sensing_value_for_cal[SEN_ELISS_100mA];
		printf("ads124[1] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELISS_100mA], elvss_cur_cal.cur_100ma_step, elvss_cur_cal.cur_100ma_value);								
	}
	else if(!strncmp(cmd, "nvsscur50ma ", 12))
	{
		//ADC_AUTO_DATA_READ_FOR_CAL();
		ADC_AUTO_DATA_READ_FOR_CAL_N();
		n_elvss_cur_cal.cur_100ma_value = atof(cmd+12);
		n_elvss_cur_cal.cur_100ma_step = adc_sensing_value_for_cal[SEN_ELISS_100mA];
		printf("ads124[1] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELISS_100mA], n_elvss_cur_cal.cur_100ma_step, n_elvss_cur_cal.cur_100ma_value);								
	}		
	else if(!strcmp(cmd, "curcal"))
	{
		avdd_cur_cal.cur_100ma_ratio = (avdd_cur_cal.cur_100ma_value-avdd_cur_cal.cur_0ma_value) / (avdd_cur_cal.cur_100ma_step - avdd_cur_cal.cur_0ma_step);
		avdd_cur_cal.cur_100ma_offset = avdd_cur_cal.cur_0ma_value - (avdd_cur_cal.cur_100ma_ratio * avdd_cur_cal.cur_0ma_step);
		avdd_cur_cal.cur_1a_ratio = (avdd_cur_cal.cur_1a_value-avdd_cur_cal.cur_0a_value) / (avdd_cur_cal.cur_1a_step - avdd_cur_cal.cur_0a_step);
		avdd_cur_cal.cur_1a_offset = avdd_cur_cal.cur_0a_value - (avdd_cur_cal.cur_1a_ratio * avdd_cur_cal.cur_0a_step);		
		printf("avdd_cur_cal.cur_100ma_ratio = %.10f\r\n", avdd_cur_cal.cur_100ma_ratio);
		printf("avdd_cur_cal.cur_100ma_offset = %.10f\r\n", avdd_cur_cal.cur_100ma_offset);
		printf("avdd_cur_cal.cur_1a_ratio = %.10f\r\n", avdd_cur_cal.cur_1a_ratio);
		printf("avdd_cur_cal.cur_1a_offset = %.10f\r\n", avdd_cur_cal.cur_1a_offset);				
	}
	else if(!strcmp(cmd, "nvsscurcal"))
	{
		n_avdd_cur_cal.cur_100ma_ratio = (n_avdd_cur_cal.cur_100ma_value-n_avdd_cur_cal.cur_0ma_value) / (n_avdd_cur_cal.cur_100ma_step - n_avdd_cur_cal.cur_0ma_step);
		n_avdd_cur_cal.cur_100ma_offset = n_avdd_cur_cal.cur_0ma_value - (n_avdd_cur_cal.cur_100ma_ratio * n_avdd_cur_cal.cur_0ma_step);
		n_avdd_cur_cal.cur_1a_ratio = (n_avdd_cur_cal.cur_1a_value-n_avdd_cur_cal.cur_0a_value) / (n_avdd_cur_cal.cur_1a_step - n_avdd_cur_cal.cur_0a_step);
		n_avdd_cur_cal.cur_1a_offset = n_avdd_cur_cal.cur_0a_value - (n_avdd_cur_cal.cur_1a_ratio * n_avdd_cur_cal.cur_0a_step);				
		printf("n_avdd_cur_cal.cur_100ma_ratio = %.10f\r\n", n_avdd_cur_cal.cur_100ma_ratio);
		printf("n_avdd_cur_cal.cur_100ma_offset = %.10f\r\n", n_avdd_cur_cal.cur_100ma_offset);
		printf("n_avdd_cur_cal.cur_1a_ratio = %.10f\r\n", n_avdd_cur_cal.cur_1a_ratio);
		printf("n_avdd_cur_cal.cur_1a_offset = %.10f\r\n", n_avdd_cur_cal.cur_1a_offset);	
		printf("n_avdd_cur_cal.cur_1a_offset = %.10f\r\n", n_avdd_cur_cal.n_cur_1a_user_offset);				
	}	
	else if(!strcmp(cmd, "vsscurcal"))
	{
		elvss_cur_cal.cur_100ma_ratio = (elvss_cur_cal.cur_100ma_value-elvss_cur_cal.cur_0ma_value) / (elvss_cur_cal.cur_100ma_step - elvss_cur_cal.cur_0ma_step);
		elvss_cur_cal.cur_100ma_offset = elvss_cur_cal.cur_0ma_value - (elvss_cur_cal.cur_100ma_ratio * elvss_cur_cal.cur_0ma_step);
		elvss_cur_cal.cur_1a_ratio = (elvss_cur_cal.cur_1a_value-elvss_cur_cal.cur_0a_value) / (elvss_cur_cal.cur_1a_step - elvss_cur_cal.cur_0a_step);
		elvss_cur_cal.cur_1a_offset = elvss_cur_cal.cur_0a_value - (elvss_cur_cal.cur_1a_ratio * elvss_cur_cal.cur_0a_step);		
		printf("elvss_cur_cal.cur_100ma_ratio = %.10f\r\n", elvss_cur_cal.cur_100ma_ratio);
		printf("elvss_cur_cal.cur_100ma_offset = %.10f\r\n", elvss_cur_cal.cur_100ma_offset);
		printf("elvss_cur_cal.cur_1a_ratio = %.10f\r\n", elvss_cur_cal.cur_1a_ratio);
		printf("elvss_cur_cal.cur_1a_offset = %.10f\r\n", elvss_cur_cal.cur_1a_offset);				
	}
	else if(!strcmp(cmd, "nvsscurcal"))
	{
		n_elvss_cur_cal.cur_100ma_ratio = (n_elvss_cur_cal.cur_100ma_value-n_elvss_cur_cal.cur_0ma_value) / (n_elvss_cur_cal.cur_100ma_step - n_elvss_cur_cal.cur_0ma_step);
		n_elvss_cur_cal.cur_100ma_offset = n_elvss_cur_cal.cur_0ma_value - (n_elvss_cur_cal.cur_100ma_ratio * n_elvss_cur_cal.cur_0ma_step);
		n_elvss_cur_cal.cur_1a_ratio = (n_elvss_cur_cal.cur_1a_value-n_elvss_cur_cal.cur_0a_value) / (n_elvss_cur_cal.cur_1a_step - n_elvss_cur_cal.cur_0a_step);
		n_elvss_cur_cal.cur_1a_offset = n_elvss_cur_cal.cur_0a_value - (n_elvss_cur_cal.cur_1a_ratio * n_elvss_cur_cal.cur_0a_step);				
		printf("n_elvss_cur_cal.cur_100ma_ratio = %.10f\r\n", n_elvss_cur_cal.cur_100ma_ratio);
		printf("n_elvss_cur_cal.cur_100ma_offset = %.10f\r\n", n_elvss_cur_cal.cur_100ma_offset);
		printf("n_elvss_cur_cal.cur_1a_ratio = %.10f\r\n", n_elvss_cur_cal.cur_1a_ratio);
		printf("n_elvss_cur_cal.cur_1a_offset = %.10f\r\n", n_elvss_cur_cal.cur_1a_offset);	
		printf("nn_elvss_cur_cal.cur_1a_offset = %.10f\r\n", n_elvss_cur_cal.n_cur_1a_user_offset);				
	}	
	else if(!strncmp(cmd, "cur1aoffset ",12))
	{
		char *dac_value = 0;
		int dca_num_int = 0;
		double adc_value_double = 0;
		strtok(cmd, " ");
		dac_value = strtok(NULL, " ");
		adc_value_double = atof(dac_value);
		
		avdd_cur_cal.cur_1a_user_offset = adc_value_double;			
		printf("cur_cal.cur_1a_user_offset = %f\r\n", avdd_cur_cal.cur_1a_user_offset);		
	}
	else if(!strncmp(cmd, "ncur1aoffset ",13))
	{
		char *dac_value = 0;
		int dca_num_int = 0;
		double adc_value_double = 0;
		strtok(cmd, " ");
		dac_value = strtok(NULL, " ");
		adc_value_double = atof(dac_value);
		
		avdd_cur_cal.n_cur_1a_user_offset = adc_value_double;			
		printf("avdd_cur_cal.n_cur_1a_user_offset = %f\r\n", avdd_cur_cal.n_cur_1a_user_offset);		
	}		
	else if(!strncmp(cmd, "vsscur1aoffset ",15))
	{
		char *dac_value = 0;
		int dca_num_int = 0;
		double adc_value_double = 0;
		strtok(cmd, " ");
		dac_value = strtok(NULL, " ");
		adc_value_double = atof(dac_value);
		
		elvss_cur_cal.cur_1a_user_offset = adc_value_double;			
		printf("cur_cal.cur_1a_user_offset = %f\r\n", elvss_cur_cal.cur_1a_user_offset);		
	}
	else if(!strncmp(cmd, "nvsscur1aoffset ",16))
	{
		char *dac_value = 0;
		int dca_num_int = 0;
		double adc_value_double = 0;
		strtok(cmd, " ");
		dac_value = strtok(NULL, " ");
		adc_value_double = atof(dac_value);
		
		elvss_cur_cal.n_cur_1a_user_offset = adc_value_double;			
		printf("elvss_cur_cal.n_cur_1a_user_offset = %f\r\n", elvss_cur_cal.n_cur_1a_user_offset);		
	}					
	else if(!strncmp(cmd, "cur50maoffset ",14))
	{
		char *dac_value = 0;
		int dca_num_int = 0;
		double adc_value_double = 0;
		strtok(cmd, " ");
		dac_value = strtok(NULL, " ");
		adc_value_double = atof(dac_value);
		
		avdd_cur_cal.cur_100ma_user_offset = adc_value_double;			
		printf("cur_cal.cur_100ma_user_offset = %f\r\n", avdd_cur_cal.cur_100ma_user_offset);		
	}
	else if(!strncmp(cmd, "vsscur50maoffset ",17))
	{
		char *dac_value = 0;
		int dca_num_int = 0;
		double adc_value_double = 0;
		strtok(cmd, " ");
		dac_value = strtok(NULL, " ");
		adc_value_double = atof(dac_value);
		
		elvss_cur_cal.cur_100ma_user_offset = adc_value_double;			
		printf("cur_cal.cur_100ma_user_offset = %f\r\n", elvss_cur_cal.cur_100ma_user_offset);		
	}			
	else if(!strcmp(cmd, "curcalsave"))
	{
		FILE *adc_file;

		adc_file = fopen(AVDD_CUR_CAL_FILE_PATH, "w+b");
		fwrite(&avdd_cur_cal, sizeof(avdd_cur_cal), 1, adc_file);
		fclose(adc_file);
		system("sync");	
		printf("cur_cal_data_save_OK\r\n");			
	}
	else if(!strcmp(cmd, "vsscurcalsave"))
	{
		FILE *adc_file;

		adc_file = fopen(ELVSS_CUR_CAL_FILE_PATH, "w+b");
		fwrite(&elvss_cur_cal, sizeof(elvss_cur_cal), 1, adc_file);
		fclose(adc_file);
		system("sync");	
		printf("elvss_cur_cal_data_save_OK\r\n");			
	}
	else if(!strcmp(cmd, "ncurcalsave"))
	{
		FILE *adc_file;

		adc_file = fopen(N_AVDD_CUR_CAL_FILE_PATH, "w+b");
		fwrite(&n_avdd_cur_cal, sizeof(n_avdd_cur_cal), 1, adc_file);
		fclose(adc_file);
		system("sync");	
		printf("n_avdd_cur_cal_data_save_OK\r\n");			
	}	
	else if(!strcmp(cmd, "nvsscurcalsave"))
	{
		FILE *adc_file;

		adc_file = fopen(N_ELVSS_CUR_CAL_FILE_PATH, "w+b");
		fwrite(&n_elvss_cur_cal, sizeof(n_elvss_cur_cal), 1, adc_file);
		fclose(adc_file);
		system("sync");	
		printf("n_elvss_cur_cal_data_save_OK\r\n");			
	}						
	else if(!strncmp(cmd, "ads015v ", 8))
	{
		ads124_cal0.adc_15v_value = atof(cmd+8);
		ADC_AUTO_DATA_READ_FOR_CAL();
		ads124_cal0.adc_15v_step = adc_sensing_value_for_cal[SEN_LDO_ELVDD];
		printf("ads124[0] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_LDO_ELVDD], ads124_cal0.adc_15v_step, ads124_cal0.adc_15v_value);								
	}				
	else if(!strncmp(cmd, "ads000v ", 8))
	{
		ads124_cal0.adc_0v_value = atof(cmd+8);
		ADC_AUTO_DATA_READ_FOR_CAL();
		ads124_cal0.adc_0v_step = adc_sensing_value_for_cal[SEN_LDO_ELVDD];
		printf("ads124[0] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_LDO_ELVDD], ads124_cal0.adc_0v_step, ads124_cal0.adc_0v_value);								
	}			
	else if(!strncmp(cmd, "ads115v ", 8))
	{
		ads124_cal1.adc_15v_value = atof(cmd+8);
		ADC_AUTO_DATA_READ_FOR_CAL();
		ads124_cal1.adc_15v_step = adc_sensing_value_for_cal[SEN_VOTP50];
		printf("ads124[1] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_VOTP50], ads124_cal1.adc_15v_step, ads124_cal1.adc_15v_value);								
	}				
	else if(!strncmp(cmd, "ads100v ", 8))
	{
		ads124_cal1.adc_0v_value = atof(cmd+8);
		ADC_AUTO_DATA_READ_FOR_CAL();
		ads124_cal1.adc_0v_step = adc_sensing_value_for_cal[SEN_VOTP50];
		printf("ads124[1] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_VOTP50], ads124_cal1.adc_0v_step, ads124_cal1.adc_0v_value);								
	}			
	else if(!strncmp(cmd, "ads215v ", 8))
	{
		ads124_cal2.adc_15v_value = atof(cmd+8);
		ADC_AUTO_DATA_READ_FOR_CAL();
		ads124_cal2.adc_15v_step = adc_sensing_value_for_cal[SEN_LM_SPARE1];
		printf("ads124[2] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_LM_SPARE1], ads124_cal2.adc_15v_step, ads124_cal2.adc_15v_value);								
	}			
	else if(!strncmp(cmd, "ads200v ", 8))
	{
		ads124_cal2.adc_0v_value = atof(cmd+8);
		ADC_AUTO_DATA_READ_FOR_CAL();
		ads124_cal2.adc_0v_step = adc_sensing_value_for_cal[SEN_LM_SPARE1];
		printf("ads124[2] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_LM_SPARE1], ads124_cal2.adc_0v_step, ads124_cal2.adc_0v_value);								
	}										
	else if(!strncmp(cmd, "ads315v ", 8))
	{
		ads124_cal3.adc_15v_value = atof(cmd+8);
		ADC_AUTO_DATA_READ_FOR_CAL();
		ads124_cal3.adc_15v_step = adc_sensing_value_for_cal[SEN_AVDD];
		printf("ads124[3] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_AVDD], ads124_cal3.adc_15v_step, ads124_cal3.adc_15v_value);								
	}
	else if(!strncmp(cmd, "ads300v ", 8))
	{
		ads124_cal3.adc_0v_value = atof(cmd+8);
		ADC_AUTO_DATA_READ_FOR_CAL();
		ads124_cal3.adc_0v_step = adc_sensing_value_for_cal[SEN_AVDD];
		printf("ads124[3] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_AVDD], ads124_cal3.adc_0v_step, ads124_cal3.adc_0v_value);								
	}			
	else if(!strncmp(cmd, "ads415v ", 8))
	{
		ads124_cal4.adc_15v_value = atof(cmd+8);
		ADC_AUTO_DATA_READ_FOR_CAL();
		ads124_cal4.adc_15v_step = adc_sensing_value_for_cal[SEN_ELVDD];
		printf("ads124[4] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELVDD], ads124_cal4.adc_15v_step, ads124_cal4.adc_15v_value);								
	}
	else if(!strncmp(cmd, "ads400v ", 8))
	{
		ads124_cal4.adc_0v_value = atof(cmd+8);
		ADC_AUTO_DATA_READ_FOR_CAL();
		ads124_cal4.adc_0v_step = adc_sensing_value_for_cal[SEN_ELVDD];
		printf("ads124[4] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELVDD], ads124_cal4.adc_0v_step / ads124_cal4.adc_0v_value);								
	}
	else if(!strcmp(cmd, "ads0cal"))
	{
		printf("ads124_cal0.adc_15v_value = %f / ads124_cal0.adc_0v_value = %f\r\n", ads124_cal0.adc_15v_value, ads124_cal0.adc_0v_value);
		printf("ads124_cal0.adc_15v_step = %f / ads124_cal0.adc_0v_step = %f\r\n", ads124_cal0.adc_15v_step, ads124_cal0.adc_0v_step);
		ads124_cal0.adc_p_ratio = (ads124_cal0.adc_15v_value-ads124_cal0.adc_0v_value) / (ads124_cal0.adc_15v_step - ads124_cal0.adc_0v_step);
		ads124_cal0.adc_p_offset = ads124_cal0.adc_0v_value - (ads124_cal0.adc_p_ratio * ads124_cal0.adc_0v_step);
		printf("ads124_cal0.adc_p_ratio = %.10f\r\n", ads124_cal0.adc_p_ratio);
		printf("ads124_cal0.adc_p_offset = %.10f\r\n", ads124_cal0.adc_p_offset);
	}				
	else if(!strcmp(cmd, "ads1cal"))
	{
		printf("ads124_cal1.adc_15v_value = %f / ads124_cal1.adc_0v_value = %f\r\n", ads124_cal1.adc_15v_value, ads124_cal1.adc_0v_value);
		printf("ads124_cal1.adc_15v_step = %f / ads124_cal1.adc_0v_step = %f\r\n", ads124_cal1.adc_15v_step, ads124_cal1.adc_0v_step);
		ads124_cal1.adc_p_ratio = (ads124_cal1.adc_15v_value-ads124_cal1.adc_0v_value) / (ads124_cal1.adc_15v_step - ads124_cal1.adc_0v_step);
		ads124_cal1.adc_p_offset = ads124_cal1.adc_0v_value - (ads124_cal1.adc_p_ratio * ads124_cal1.adc_0v_step);
		printf("ads124_cal1.adc_p_ratio = %.10f\r\n", ads124_cal1.adc_p_ratio);
		printf("ads124_cal1.adc_p_offset = %.10f\r\n", ads124_cal1.adc_p_offset);
	}				
	else if(!strcmp(cmd, "ads2cal"))
	{
		printf("ads124_cal2.adc_15v_value = %f / ads124_cal2.adc_0v_value = %f\r\n", ads124_cal2.adc_15v_value, ads124_cal2.adc_0v_value);
		printf("ads124_cal2.adc_15v_step = %f / ads124_cal2.adc_0v_step = %f\r\n", ads124_cal2.adc_15v_step, ads124_cal2.adc_0v_step);
		ads124_cal2.adc_p_ratio = (ads124_cal2.adc_15v_value-ads124_cal2.adc_0v_value) / (ads124_cal2.adc_15v_step - ads124_cal2.adc_0v_step);
		ads124_cal2.adc_p_offset = ads124_cal2.adc_0v_value - (ads124_cal2.adc_p_ratio * ads124_cal2.adc_0v_step);
		printf("ads124_cal2.adc_p_ratio = %.10f\r\n", ads124_cal2.adc_p_ratio);
		printf("ads124_cal2.adc_p_offset = %.10f\r\n", ads124_cal2.adc_p_offset);
	}			
	else if(!strcmp(cmd, "ads3cal"))
	{
		printf("ads124_cal3.adc_15v_value = %f / ads124_cal3.adc_0v_value = %f\r\n", ads124_cal3.adc_15v_value, ads124_cal3.adc_0v_value);
		printf("ads124_cal3.adc_15v_step = %f / ads124_cal3.adc_0v_step = %f\r\n", ads124_cal3.adc_15v_step, ads124_cal3.adc_0v_step);
		ads124_cal3.adc_p_ratio = (ads124_cal3.adc_15v_value-ads124_cal3.adc_0v_value) / (ads124_cal3.adc_15v_step - ads124_cal3.adc_0v_step);
		ads124_cal3.adc_p_offset = ads124_cal3.adc_0v_value - (ads124_cal3.adc_p_ratio * ads124_cal3.adc_0v_step);
		printf("ads124_cal3.adc_p_ratio = %.10f\r\n", ads124_cal3.adc_p_ratio);
		printf("ads124_cal3.adc_p_offset = %.10f\r\n", ads124_cal3.adc_p_offset);
	}
	else if(!strcmp(cmd, "ads4cal"))
	{
		printf("ads124_cal4.adc_15v_value = %f / ads124_cal4.adc_0v_value = %f\r\n", ads124_cal4.adc_15v_value, ads124_cal4.adc_0v_value);
		printf("ads124_cal4.adc_15v_step = %f / ads124_cal4.adc_0v_step = %f\r\n", ads124_cal4.adc_15v_step, ads124_cal4.adc_0v_step);				
		ads124_cal4.adc_p_ratio = (ads124_cal4.adc_15v_value-ads124_cal4.adc_0v_value) / (ads124_cal4.adc_15v_step - ads124_cal4.adc_0v_step);
		ads124_cal4.adc_p_offset = ads124_cal4.adc_0v_value - (ads124_cal4.adc_p_ratio * ads124_cal4.adc_0v_step);
		printf("ads124_cal4.adc_p_ratio = %.10f\r\n", ads124_cal4.adc_p_ratio);
		printf("ads124_cal4.adc_p_offset = %.10f\r\n", ads124_cal4.adc_p_offset);
	}
	else if(!strcmp(cmd, "ads0calsave"))
	{
		FILE *adc_file;

		adc_file = fopen(ADS124_0_CAL_FILE_PATH, "w+b");
		fwrite(&ads124_cal0, sizeof(ads124_cal0), 1, adc_file);
		fclose(adc_file);
		system("sync");	
		printf("ads124_cal0_data_save_OK\r\n");			
	}			
	else if(!strcmp(cmd, "ads1calsave"))
	{
		FILE *adc_file;

		adc_file = fopen(ADS124_1_CAL_FILE_PATH, "w+b");
		fwrite(&ads124_cal1, sizeof(ads124_cal1), 1, adc_file);
		fclose(adc_file);
		system("sync");	
		printf("ads124_cal1_data_save_OK\r\n");			
	}					
	else if(!strcmp(cmd, "ads2calsave"))
	{
		FILE *adc_file;

		adc_file = fopen(ADS124_2_CAL_FILE_PATH, "w+b");
		fwrite(&ads124_cal2, sizeof(ads124_cal2), 1, adc_file);
		fclose(adc_file);
		system("sync");	
		printf("ads124_cal2_data_save_OK\r\n");			
	}			
	else if(!strcmp(cmd, "ads3calsave"))
	{
		FILE *adc_file;

		adc_file = fopen(ADS124_3_CAL_FILE_PATH, "w+b");
		fwrite(&ads124_cal3, sizeof(ads124_cal3), 1, adc_file);
		fclose(adc_file);
		system("sync");	
		printf("ads124_cal3_data_save_OK\r\n");			
	}
	else if(!strcmp(cmd, "ads4calsave"))
	{
		FILE *adc_file;

		adc_file = fopen(ADS124_4_CAL_FILE_PATH, "w+b");
		fwrite(&ads124_cal4, sizeof(ads124_cal4), 1, adc_file);
		fclose(adc_file);
		system("sync");	
		printf("ads124_cal4_data_save_OK\r\n");			
	}																																																																			
	else if(!strcmp(cmd, "adcload"))
	{
		ads124_cal_load();
	}
	else if(!strncmp(cmd, "regr ",5))
	{
		char *reg_value = 0;
		int reg_num_int = 0;
		double adc_value_double = 0;
		strtok(cmd, " ");
		reg_value = strtok(NULL, " ");
		adc_value_double = atof(reg_value);
		
		register_offset.register_r = adc_value_double;			
		printf("register_offset.register_r = %f\r\n", register_offset.register_r);		
	}
	else if(!strncmp(cmd, "regl ",5))
	{
		char *reg_value = 0;
		int reg_num_int = 0;
		double adc_value_double = 0;
		strtok(cmd, " ");
		reg_value = strtok(NULL, " ");
		adc_value_double = atof(reg_value);
		
		register_offset.register_l = adc_value_double;			
		printf("register_offset.register_l = %f\r\n", register_offset.register_l);		
	}
	else if(!strcmp(cmd, "regsave"))
	{
		FILE *adc_file;

		adc_file = fopen(REGISTER_OFFSET_FILE_PATH, "w+b");
		fwrite(&register_offset, sizeof(register_offset), 1, adc_file);
		fclose(adc_file);
		system("sync");	
		printf("register_offset_data_save_OK\r\n");			
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
}


void close_keyboard(void)
{
    tcsetattr(0, TCSANOW, &initial_settings);
}

void init_keyboard(void)
{
    tcgetattr(0, &initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
}

int kbhit(void)
{
    unsigned char ch;
    int nread;

    if(peek_character != -1) return 1;
    new_settings.c_cc[VMIN] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0, &ch, 1);
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &new_settings);
    if(nread == 1){
        peek_character = (int)ch;
        return 1;
    }
    return 0;
}


unsigned char readch(void)
{
    unsigned char ch;

    if (peek_character != -1){
        ch = (unsigned char)peek_character;
        peek_character = -1;
        return ch;
    }
    read (0, &ch, 1);

    return ch;
}


void debug_task(void)
{
    unsigned char ch;
    char debug_buffer[DEBUG_BUFFER_SIZE];
    int debug_put_cnt;
    //BOOL rcb_mode = FALSE; //RCB 모드 구분 변수 추가
	rcb_mode = FALSE;
    int rcb_time_start, rcb_time_end;

    setvbuf(stdout, NULL, _IONBF, 0);
    init_keyboard();
	fflush(stdin);fflush(stdout);
    memset(debug_buffer, 0, sizeof(debug_buffer));
    debug_put_cnt = 0;
    
    printf("[ENSIS_"FW_VERSION"]");

    while(1){
        pthread_mutex_lock(&mutex_lock);
        if(rcb_mode){ // RCB 모드인데 100ms 동안 통신이 오지 않거나 동작이 안되면 돌아오기위해 타임아웃 기능 추가
            rcb_time_end = msclock();
            if((rcb_time_end - rcb_time_start) > 100000){ //100ms 넘었는지 체크
                rcb_mode = FALSE;
                debug_put_cnt = 0;
                memset(debug_buffer, 0, DEBUG_BUFFER_SIZE);
            }
        }
        while(kbhit()){
            ch = readch();
            if(debug_put_cnt == 0){ //첫글자로 RCB인지 디버그인지 확인, 첫글자가 '[' 괄호이면 RCB 모드
                if(ch == '[') rcb_mode = TRUE;
                else 
				{
					rcb_mode = FALSE;
				}
            }
            if(rcb_mode){ //RCB 사용
                if(ch == ']'){ //RCB에서 보내는 데이터 끝 확인
                    debug_buffer[debug_put_cnt++] = ch; //버퍼에 ']' 글자 추가
                    rcb_cmd_analysis(debug_buffer);
                    rcb_mode = FALSE;				
                    debug_put_cnt = 0;
                    memset(debug_buffer, 0, DEBUG_BUFFER_SIZE);
                }
                else{
                    if(debug_put_cnt < (DEBUG_BUFFER_SIZE-1)){ //버퍼사이즈 넘지않게,, 뒤에 -1은 ']'끝 괄호를 바로위 if문에서 추가할것이므로
                        debug_buffer[debug_put_cnt++] = ch;
                        rcb_time_start = msclock();
                    }
                    else{ //버퍼사이즈가 넘었다는 것은 통신이 에러임, 초기화
                        rcb_mode = FALSE;
                        debug_put_cnt = 0;
                        memset(debug_buffer, 0, DEBUG_BUFFER_SIZE);
                    }
                }
            }
            else{ //디버그 콘솔 사용
                if((ch == '\r') || (ch == '\n')){
                    printf("\r\n");
                    if(debug_put_cnt != 0) shell_cmd_thread(debug_buffer);
                    debug_put_cnt = 0;
                    memset(debug_buffer, 0, sizeof(debug_buffer));
                    printf("[ENSIS_"FW_VERSION"]");
                }
                else if(ch == 0x7f){
                    if(debug_put_cnt > 0){
                        char temp[3] = {0x08, 0x20, 0x08};
                        putchar(temp[2]);putchar(temp[1]);putchar(temp[0]);
                        debug_buffer[--debug_put_cnt] = '\0';
                    }
                }
                else{
                    if(debug_put_cnt < DEBUG_BUFFER_SIZE){
                        debug_buffer[debug_put_cnt++] = ch;
                        putchar(ch);
                    }
                }
            }
        }
        pthread_mutex_unlock(&mutex_lock);
        usleep(1000);
    }
}

/*void debug_task(void)
{
    unsigned char ch;
    char debug_buffer[DEBUG_BUFFER_SIZE];
    int debug_put_cnt;

    setvbuf(stdout, NULL, _IONBF, 0);
    init_keyboard();
    memset(debug_buffer, 0, sizeof(debug_buffer));
    debug_put_cnt = 0;
    
    printf("[ENSIS_"FW_VERSION"]");

    while(1){
        pthread_mutex_lock(&mutex_lock);
        while(kbhit()){
            ch = readch();
            if((ch == '\r') || (ch == '\n')){
                printf("\r\n");
                if(debug_put_cnt != 0) shell_cmd_thread(debug_buffer);
                debug_put_cnt = 0;
                memset(debug_buffer, 0, sizeof(debug_buffer));
                printf("[ENSIS_"FW_VERSION"]");
            }
            else if(ch == 0x7f){
                if(debug_put_cnt > 0){
                    char temp[3] = {0x08, 0x20, 0x08};
                    putchar(temp[2]);putchar(temp[1]);putchar(temp[0]);
                    debug_buffer[--debug_put_cnt] = '\0';
                }
            }
            else{
                if(debug_put_cnt < DEBUG_BUFFER_SIZE){
                    debug_buffer[debug_put_cnt++] = ch;
                    putchar(ch);
                }
            }
        }
        pthread_mutex_unlock(&mutex_lock);
        usleep(1000);
    }
}*/


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
	char thr_id;	
	int t, dt, i;
	
	total_status=ts_init();   

	queue_cal_result=ring_q_init(42*10);  
	system_init();	
	relay_init();		
	signal_power_device_init();	
	//signal_device_init();	
	ex_port_serial_open(115200, 8, 1, 0);
	Power_Supply_Voltage_load();	
	dac_init();
	mux_init();
	usleep(100000);
	ads124s08_sensing_init();
	usleep(100000);
	ads124s08_ads124s08_rm_init();
	gpio_reg_init();
	ads124_cal_load();
	register_offset_load();
	i2c_init();
	el_ocp_set(1100);
	ap_ocp_set(300);
	dp_ocp_set(300);
	//struct can_frame frame;
	pthread_mutex_init(&mutex_lock,NULL);
	sleep(1);
	//net_config_init();
	//TcpServerInit();
	sdcd_serial_open(38400, 8, 1, 0);
	//sdcd_serial_open(115200, 8, 1, 0);
	i2c_frequency_set(I2C_RATE_100KHZ);
	if((rm_error_cnt_result) || rm_error_ready_result)	//231117 Modify
	{
		rm_adc_init_ng_ack();
		timer_t timerID;
		createTimer(&timerID,0, 50);		
	}
	else
	{
		timer_t timerID;
		createTimer(&timerID,0, 200);
		pg_reboot_ack();		//230929 Modify
	}
    thr_id = pthread_create(&pthread_shell, NULL, (void *(*)())debug_task, 0);
	if(thr_id < 0)
	{
		LOGE("shell_cmd_thread create error");
	}

	taskSpawn(1,(int)ocp_task);	
	taskSpawn(1,(int)category_task);	
	#ifdef RELIABILITY_ROOM	//231027 Modify
	taskSpawn(1,(int)sensing_task);	  	
	#endif					//231027 Modify		
	
	long check_time=0;
	uint32_t check_24h_time=0;
	while(!pthread_end)
	{ 	
		usleep(10);
		//usleep(1);
		#ifdef RELIABILITY_ROOM				//231027 Modify
		aging_mode_task();		     				

		if(check_time!=get_uptime())		//231027 Modify
		{
			if(check_24h_time>=(CACHMEM_INIT_24HOURS))
			{
				system("echo 3 > /proc/sys/vm/drop_caches");
				//system("cat /dev/null > /var/volatile/log/boot");
				check_24h_time=0;
			}
			else check_24h_time++;
			check_time=get_uptime();
		}
		#endif					//231027 Modify
	}

	pthread_end=1;
	printf("exit encore\n");
	//pthread_join(pthread_tcp,0);
	pthread_join(pthread_shell,0);
			
	sdcd_serial_close();
	ts_deinit(total_status);		
	ring_q_deinit(queue_cal_result);		
		
	return 1;
}

void system_init(void)
{
	int mem_fd;
	void *map_base;
	FILE *recipe_check_fd;
	FILE *rminit_error_cnt_check_fd;
	FILE *rminit_error_ready_check_fd;

	mem_fd = open("/dev/mem", O_RDWR);
	if(mem_fd < 0){
		LOGE("/dev/mem open error (OCP)\r\n");
		return;
	}
	map_base = (void *)mmap((void *)0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, REGISTER_BASE_OCP);
	ocp = (OCP *)map_base;
	close(mem_fd);

	mem_fd = open("/dev/mem", O_RDWR);
	if(mem_fd < 0){
		LOGE("/dev/mem open error (PATTERN_GENERATOR)\r\n");
		return;
	}
	map_base = (void *)mmap((void *)0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, REGISTER_BASE_PATTERN_GENERATOR);
	pattern_generator = (PATTERN_GENERATOR *)map_base;
	close(mem_fd);

	mem_fd = open("/dev/mem", O_RDWR);
	if(mem_fd < 0){
		LOGE("/dev/mem open error (ADS124S08)\r\n");
		return;
	}
	map_base = (void *)mmap((void *)0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, REGISTER_BASE_ADS124S08);
	ads124s08_sensing = (ADS124S08_SENSING *)map_base;
	close(mem_fd);

	mem_fd = open("/dev/mem", O_RDWR);
	if(mem_fd < 0){
		LOGE("/dev/mem open error (ADS124S08_RM)\r\n");
		return;
	}
	map_base = (void *)mmap((void *)0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, REGISTER_BASE_ADS124S08_RM);
	ads124s08_rm = (ADS124S08_SENSING *)map_base;
	close(mem_fd);	

	mem_fd = open("/dev/mem", O_RDWR);
	if(mem_fd < 0){
		LOGE("/dev/mem open error (GPIO)\r\n");
		return;
	}
	map_base = (void *)mmap((void *)0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, REGISTER_BASE_AX_GPIO_0);
	gpio = (GPIO_REG *)map_base;
	close(mem_fd);	

	mem_fd = open("/dev/mem", O_RDWR);
	if(mem_fd < 0){
		LOGE("/dev/mem open error (GPIO_ENBLE)\r\n");
		return;
	}
	map_base = (void *)mmap((void *)0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, REGISTER_BASE_AX_GPIO_ENBLE);
	gpio_enble = (GPIO_REG *)map_base;
	close(mem_fd);		

	mem_fd = open("/dev/mem", O_RDWR);
	if(mem_fd < 0){
		LOGE("/dev/mem open error (PCA9665)\r\n");
		return;
	}
	map_base = (void *)mmap((void *)0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, REGISTER_BASE_PCA9665);
	i2c_bridge = (I2C_BRIDGE *)map_base;
	close(mem_fd);		

	LOGE("fpga register mapping ok\r\n");
	
    memset(&signal_group, 0, sizeof(signal_group)); 
	memset(&time_count, 0, sizeof(time_count)); 
	memset(&error_data, 0, sizeof(error_data));
 	//uart_gpio_init();
	gpiops_init();
	gpiops_export(0);
	gpiops_dir_out(0);
	gpiops_export(RELAY_ELVDD);
	gpiops_dir_out(RELAY_ELVDD);	
	gpiops_export(RELAY_VDD8_S);
	gpiops_dir_out(RELAY_VDD8_S);	
	gpiops_export(RELAY_VDD8_G);
	gpiops_dir_out(RELAY_VDD8_G);	
	gpiops_export(RELAY_VGH);
	gpiops_dir_out(RELAY_VGH);				
	gpiops_export(RELAY_VGL);
	gpiops_dir_out(RELAY_VGL);	
	gpiops_export(RELAY_VINIT);
	gpiops_dir_out(RELAY_VINIT);	
	rs232_read_flag = 0;
	pg_on_flag = PG_OFF;
	ocp_flag_on = 0;
	//otp_flag_on= 0;		//231101 Modify
	each_channel_ocp_value = 0;
	auto_ocp_flag_on = 0;
	t_count = 0;
	h_count = 0;
	m_count = 0;
	zone_select = 0;
	USER_DELAY = 5000;
	 
	total_sen_flag = 0;
	//elvss_cur_sen_flag = 0;
	//avdd_elvss_cur_sen_flag = 0;
	//avdd_elvss_sen_flag = 0;
	//avdd_elvss_cur_2byte_sen_flag = 0;
	//vrefh_vrefl_sen_flag = 0;
	cur_sensing_reset_flag = 0;
	total_sen_cur_4byte_flag = 0;
	system_load_flag = 0;
	aging_pat_index = 0;
	aging_pattern_start = 0;
	aging_pat_change_flag = 0;
	aging_pat_interval_change_flag = 0;
	interval_start_flag = 0;
	pat_index = 0xff;
	memset(&system_error, 0, sizeof(system_error));	//231013 Modify
	memset(&aging_result_string, 0, sizeof(aging_result_string));	//231027 Modify
	memset(&file_cnt_string, 0, sizeof(file_cnt_string));			//231027 Modify
	if(access("/f0/config", F_OK) != 0) system("mkdir /f0/config");
	usleep(100000);	
	log_index_file_create();
    if(access("/f0/recipe", F_OK) != 0) system("mkdir /f0/recipe");
    usleep(100000);	
	ts_dac_adc_auto_task_set(total_status,DAC_ADC_AUTO_STOP);
    /*if(access("/f0/error", F_OK) != 0) system("mkdir /f0/error");
    usleep(100000);	
	
	if(access(RM_INIT_ERROR_CNT_FILE_PATH,F_OK)!= 0)	//231117 Modify		
	{
       rminit_error_cnt_check_fd = fopen(RM_INIT_ERROR_CNT_FILE_PATH,"wb");   
	   usleep(1000);
	   if(rminit_error_cnt_check_fd != NULL)
	   {
			fwrite("0", 1, 1, rminit_error_cnt_check_fd);    
			fclose(rminit_error_cnt_check_fd); 
	   }	   		
	}	
	usleep(100000);	
	if(access(RM_INIT_ERROR_READY_FILE_PATH,F_OK)!= 0)	//231117 Modify		
	{
       rminit_error_ready_check_fd = fopen(RM_INIT_ERROR_READY_FILE_PATH,"wb");   
	   usleep(1000);
	   if(rminit_error_ready_check_fd != NULL)
	   {
			fwrite("0", 1, 1, rminit_error_ready_check_fd);    
			fclose(rminit_error_ready_check_fd); 
	   }	   		
	}	
	usleep(100000);*/		
    //if(system("ls /f0/recipe/recipe.txt"))
	if(access(RECIPE_FILE_PATH,F_OK)!= 0)	//230929 Modify
    {
       recipe_check_fd = fopen(RECIPE_FILE_PATH,"wb");   
	   usleep(1000);
	   if(recipe_check_fd != NULL)	fclose(recipe_check_fd);                       
    }
	else 	
	{
		model_recipe_read();
		model_name_check();		
	}
}

void musleep(int time)
{
	int mok,nam;

	mok = time / 10000;
	nam = time % 10000;
	if(mok) 
	{
		usleep((mok) * 10000);
	}
	if(nam) 
	{
		uwait(nam);
	}
}

void uwait(int time)
{
	int i;

	for(i=0;i<time*100;i++)
	{
		asm("nop");
	}
}

/*void ensis_delay(int time)
{
	int t, dt;
	unsigned char i=0,*id;
	unsigned short *delay;
	//delay = (unsigned short *)time;
	signal_group.signal_config.sequence_timing[0] = time;
	delay = (unsigned short *)&signal_group.signal_config.sequence_timing[0];					
	//printf("DELAY = %d\r\n", time);
	t = msclock();
	usleep(500);										
	dt = msclock() - t;
	//printf("TP1 = %d\r\n", dt);					
	if(*delay<dt) dt=*delay;
	musleep((*delay-dt) * 1000);
}*/

void ensis_delay(int time)
{
	int start, end;
	int i = 0;

	signal_group.signal_config.sequence_timing[0] = time; // backup

	start = ensis_msclock();
	while(1){
		end = ensis_msclock() - start;
		if(end >= time) break;
		i++;
		if(i > 5000000) 
		{
			printf("DELAY_COMMAND_ERROR\r\n");
			system_error.delay_task = ERROR_NG;
			break;
		}
		//usleep(1);
	}
}

static long get_uptime(void)
{
	struct sysinfo s_info;
	int error = sysinfo(&s_info);
	if(error != 0)
	{
		//DERROR("code error = %d\n", error);
		printf("code error = %d\n", error);
	}
	return s_info.uptime;
}


