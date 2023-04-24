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
#include "../include/spi.h"
#include "../include/adc.h"
#include "../include/gpio.h"
#include "../include/signal_power.h"
#include "../include/dac.h"
#include "../include/ocp.h"
#include "../include/pca9665.h"
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#define Cprintf if(cprf) printf
#define Tprintf if(tprf) printf
#define Eprintf if(eprf) printf

int cprf=0;
int tprf=0;
int eprf=0;
int aprf=0;

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

unsigned char readch(void);
static int peek_character = -1;
static struct termios initial_settings;
static struct termios new_settings;
static int fd = -1;

unsigned char com_put = 0;

unsigned char ocp_test = 0;

unsigned char key_index = 0;
extern unsigned char com_buffer_recive[COM_BUFFER_SIZE];
extern int com_buffer_recive_cnt;

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

void *shell_cmd_thread(char *cmd)
{
	char key_flag = 0;

	if(!strcmp(cmd, "[00]"))
	{
		if(key_flag)	
		{
			key_flag = 0;
			key_index = 0;
			recipe_key_load("KEY0");
		}
		else
		{
			key_flag = 1;
			key_index = 1;
			recipe_key_load("KEY1");
		}				
	}
	else if(!strcmp(cmd, "[05]"))
	{
		char cmd_name[10];
		if(key_flag)
		{
			if(key_index >= srecipe.key_count)	key_index = 1;
			else	key_index++; 	
			sprintf(cmd_name, "KEY%d",key_index);
			recipe_key_load(cmd_name);
		}		
	}
	else if(!strcmp(cmd, "[0B]"))
	{
		char cmd_name[10];
		if(key_flag)
		{
			if(key_index == 1)	key_index = srecipe.key_count;
			else  key_index--;
			sprintf(cmd_name, "KEY%d",key_index);
			recipe_key_load(cmd_name);
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
		printf("ZONE_SELECT =  address,cnt ex)comw 0260,1\r\n");
		printf("---------------recipe------------------------------\r\n");
		printf("ELVDD,ELVSS,AVDD,ADD8_S,ADD8_G,VGH,VGL,VINIT,ASPARE1,ASPARE2,VDD11,VDD18,DSPARE1,DSPARE2,LOGIC,I2C,RSTB,TM,BISTEN,LPSPARE1,DELAY,fun,i2cset,OFF,SEN,OCP,PAT,OTP,I2CFRE\r\n");
		printf("--------------------TEST---------------------------\r\n");
		printf("RELAY_ON_TEST = relayn x ex)relayn 0(ELVDD), relayn 1(VDD8_S), relayn 2(VDD8_G), relayn 3(VGH), relayn 4(VGL), relayn 5(VINT)\r\n");
		printf("RELAY_OFF_TEST = relayf x ex)relayf 0(ELVDD), relayf 1(VDD8_S), relayf 2(VDD8_G), relayf 3(VGH), relayf 4(VGL), relayf 5(VINT)\r\n");
		printf("MUX_ON_TEST = muxn\r\n");
		printf("MUX_OFF_TEST = muxf\r\n");

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
		unsigned char slave_address_w = 0xa2>>1;
		unsigned short reg_address_w = 0x0000;
		unsigned char write_buffer[100] = {0};
		int write_byte = 2;

		unsigned char slave_address = 0xa3>>1;
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
		unsigned char slave_address = 0xa2>>1;
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

	}
	else if((!strncmp(cmd, "e2pr ",5)))
	{
		unsigned char slave_address = 0xa3>>1;
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
	}					
	else if(!strncmp(cmd, "comw ",5))
	{
		unsigned char slave_address = 0xa0>>1;
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

	}
	else if((!strncmp(cmd, "comr ",5)))
	{
		unsigned char slave_address = 0xa1>>1;
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
			printf("reg_address = %x / READ_DATA = [%x]\r\n", reg_address, read_buffer[i]);
		}
	}						
	else if(!strcmp(cmd, "ocptest"))
	{
		ocp_test = 1;
	}
	else if(!strcmp(cmd, "ocpoff"))
	{
		ocp_test = 0;
	}							
	else if(!strncmp(cmd, "adrg ",5))
	{
		char adc_reg = 0;
		adc_reg = atoi(cmd+5);
		ads124s08_ads124s08_rm_test(adc_reg);					
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
	else if(!strcmp(cmd, "ss"))
	{
		gpio->GPIO_DATA = 0x1aa;	
		gpio->GPIO_DATA = 0x1ff;
		printf("TEST_OK\r\n");
	}							
	else if(!strcmp(cmd, "reg"))
	{
		//resistance_measurement_1k();
		//usleep(5);
		int t, dt;
		t = msclock();
		resistance_measurement_1();
		gpio->GPIO_DATA = 0x100;
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
		ADC_AUTO_DATA_READ();
		dt = msclock() - t;	
		printf("time_data = %dms\r\n", dt);				
		ADC_DATA_PRINT();
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
	else if(!strncmp(cmd, "i2",2))
	{
		unsigned char slave_address_w = 0x90>>1;
		unsigned short reg_address_w = 0x0001;
		unsigned char write_buffer[100] = {0};
		int write_byte = 2;

		unsigned char slave_address = 0x91>>1;
		unsigned short reg_address = 0x0001;
		unsigned char read_buffer[100] = {0};
		int read_byte = 2;

		int i = 0;
		for(i = 0 ; i < 100 ; i++)
		{
			//write_buffer[0] +=1;
			//write_buffer[1] +=1;
			write_buffer[0] =0x00;
			write_buffer[1] =0x00;					
			i2c_write_test(slave_address_w, reg_address_w, write_buffer, write_byte);
			i2c_read_test(slave_address, reg_address, read_buffer, read_byte);					
			printf("---------------------i2c_data[%d] = %x / %x\r\n", i,read_buffer[0], read_buffer[1]);										
			write_buffer[0] =0x56;
			write_buffer[1] =0x78;					
			i2c_write_test(slave_address_w, reg_address_w, write_buffer, write_byte);
			i2c_read_test(slave_address, reg_address, read_buffer, read_byte);
			printf("i2c_data[%d] = %x / %x\r\n", i,read_buffer[0], read_buffer[1]);
			
		}								
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
			signal_group.signal_config->dc_voltage[i] = atof(cmd+5);
			printf("TP = %d\r\n", signal_group.signal_config->dc_voltage[i]);
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
				
		signal_group.signal_config->dc_voltage[0] = atof(cmd+3);							
	
		a = ((65535/10)*(((double)signal_group.signal_config->dc_voltage[0]/1000)+5));
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
		signal_group.signal_config->sequence_timing[0] = 5000;
		delay = (unsigned short *)&signal_group.signal_config->sequence_timing[0];				
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
		signal_group.signal_config->el_over_current_value = atof(cmd+6);
		a = (65535/10)*(((((float)signal_group.signal_config->el_over_current_value/1000)*0.25)*((1+(820000/13000)*0.1)))+5);
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
		signal_group.signal_config->ap_over_current_value = atof(cmd+6);
		a = (65535/10)*(((((float)signal_group.signal_config->ap_over_current_value/1000)*0.5)*((1+(1000000/10200)*0.1)))+5);
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
			ads124s08_sensing->ADC_CONTROL = ADC_Control_START_PIN_LOW;						
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
	}																																
	else if(!strcmp(cmd, "adscalpri"))
	{
		ADC_AUTO_DATA_READ_FOR_CAL();
		usleep(10);
		ADC_DATA_FOR_CAL_PRINT();	
	}																																																																				
	else if(!strcmp(cmd, "adsinit"))
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
			signal_group.signal_config->dc_voltage[i] = 15000;
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
			signal_group.signal_config->dc_voltage[i] = 0;
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
	else if(!strncmp(cmd, "cur1a ", 6))
	{
		ADC_AUTO_DATA_READ_FOR_CAL();
		avdd_cur_cal.cur_1a_value = atof(cmd+6);
		avdd_cur_cal.cur_1a_step = adc_sensing_value_for_cal[SEN_ELIDD];
		printf("cur_1a = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELIDD], avdd_cur_cal.cur_1a_step, avdd_cur_cal.cur_1a_value);								
	}
	else if(!strncmp(cmd, "vsscur1a ", 9))
	{
		ADC_AUTO_DATA_READ_FOR_CAL();
		elvss_cur_cal.cur_1a_value = atof(cmd+9);
		elvss_cur_cal.cur_1a_step = adc_sensing_value_for_cal[SEN_ELISS];
		printf("elvss_cur_1a = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELISS], elvss_cur_cal.cur_1a_step, elvss_cur_cal.cur_1a_value);								
	}		
	else if(!strncmp(cmd, "cur50ma ", 8))
	{
		ADC_AUTO_DATA_READ_FOR_CAL();
		avdd_cur_cal.cur_100ma_value = atof(cmd+8);
		avdd_cur_cal.cur_100ma_step = adc_sensing_value_for_cal[SEN_ELIDD_100mA];
		printf("ads124[1] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELIDD_100mA], avdd_cur_cal.cur_100ma_step, avdd_cur_cal.cur_100ma_value);								
	}
	else if(!strncmp(cmd, "vsscur50ma ", 11))
	{
		ADC_AUTO_DATA_READ_FOR_CAL();
		elvss_cur_cal.cur_100ma_value = atof(cmd+11);
		elvss_cur_cal.cur_100ma_step = adc_sensing_value_for_cal[SEN_ELISS_100mA];
		printf("ads124[1] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELISS_100mA], elvss_cur_cal.cur_100ma_step, elvss_cur_cal.cur_100ma_value);								
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
	char thr_id;	
	int t, dt, i;
	system_init();	
	relay_init();		
	signal_power_device_init();		
	Power_Supply_Voltage_load();	
	dac_init();
	mux_init();
	usleep(100000);
	ads124s08_sensing_init();
	usleep(100000);
	ads124s08_ads124s08_rm_init();
	gpio_reg_init();
	ads124_cal_load();
	i2c_init();	
	el_ocp_set(1100);
	ap_ocp_set(300);
	dp_ocp_set(300);
	//struct can_frame frame;
	pthread_mutex_init(&mutex_lock,NULL);
	poll_init();
	sleep(1);
	//net_config_init();
	//TcpServerInit();
	sdcd_serial_open(38400, 8, 1, 0);
	i2c_frequency_set(I2C_RATE_100KHZ);
	timer_t timerID;
	createTimer(&timerID,0, 200);

    thr_id = pthread_create(&pthread_shell, NULL, (void *(*)())debug_task, 0);
	if(thr_id < 0)
	{
		LOGE("shell_cmd_thread create error");
	}

	taskSpawn(1,(int)ocp_task);	  
	while(!pthread_end)
	{ 	
		/*if(com_buffer_recive_cnt != com_buffer_cnt)
		{				
			
			rs232_read_flag = 1;
			i=0;
			com_put = com_buffer_recive[com_buffer_cnt++];
			//data_check();
			if(com_buffer_cnt >= COM_BUFFER_SIZE) com_buffer_cnt = 0;
			printf("com_buffer_recive_cnt = %d / com_buffer_cnt = %d\r\n", com_buffer_recive_cnt, com_buffer_cnt);
		}
		if(rs232_read_flag)
		{
			//if(i==0)	t = msclock();
			if(i > 25)
			{
				i = 0;
				rs232_read_flag = 0;				
				com_read_data_init();
				//dt = msclock() - t;	
				//printf("rs232_rx_ng = %d\r\n", dt);
			}
			i++;
		}*/
		poll_loop(2);
		usleep(1);
	}

	pthread_end=1;
	printf("exit encore\n");
	//pthread_join(pthread_tcp,0);
	pthread_join(pthread_shell,0);
			
	sdcd_serial_close();			
	return 1;
}

void system_init(void)
{
	int mem_fd;
	void *map_base;

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
	pg_on_flag = 0;
	ocp_flag_on = 0;
	each_channel_ocp_value = 0;
	t_count = 0;
	h_count = 0;
	m_count = 0;
	zone_select = 0;
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
	signal_group.signal_config->sequence_timing[0] = time;
	delay = (unsigned short *)&signal_group.signal_config->sequence_timing[0];					
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

	signal_group.signal_config->sequence_timing[0] = time; // backup

	start = msclock();
	while(1){
		end = msclock() - start;
		if(end >= time) break;
	}
}

