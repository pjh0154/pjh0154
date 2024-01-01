#include "../include/application.h"
#include "../include/pca9665.h"
#include "../include/gpio-dev.h"
#include "../include/serial_dev_2.h"
//#include <string.h>
#include <ctype.h>

extern pthread_mutex_t mutex_lock;
extern int		pthread_end;
extern int cprf;
extern int eprf;
extern int aprf;
extern int sprf;
extern int vprf;
extern int elrf;
extern int avrf;
extern int nprf;
extern int pprf;

extern void ensis_delay(int time);
extern unsigned char pat_index;
extern int kbhit(void);
extern unsigned char readch(void);

extern total_status_t *total_status;  
double lpf_tau = 0.00012;
double lpf_time = 0.00001;
#define LPF_TAU lpf_tau
#define LPF_TS  lpf_time

 int getch(void)
 {
     int ch;
     struct termios buf, save;
     tcgetattr(0,&save);
     buf = save;
     buf.c_lflag &= ~(ICANON|ECHO);
     buf.c_cc[VMIN] = 1;
     buf.c_cc[VTIME] = 0;
     tcsetattr(0, TCSAFLUSH, &buf);
     ch = getchar();
     tcsetattr(0, TCSAFLUSH, &save);
     return ch;
 }

 void dac_power_off(void)
 {
    pvsig_voltage_set(power_vol_min);
    nvsig_voltage_set(power_vol_min); 
    pvsig_onoff(false);
    nvsig_onoff(false);
    pattern_generator->PG_CONTROL = DAC_nCLR_LOW;
    usleep(1);
    pattern_generator->PG_CONTROL = DAC_nCLR_HIGH;	
    usleep(1);	
    pattern_generator->PG_CONTROL = DAC_nCLR_LOW;	   
 }

  void model_recipe_read(void)
 {
    FILE *recipe_file;
    char strline[100];
    char func_count_flag = 0;
    char key_count_flag = 0;
    int i = 0;
    int func_flag = 0;
    char *func_name;
    char *func_name1;   
    char* cmd;
    char* para;
    char* c;
    char *cmd_name;
    char *parm;
    int parm_1;
    int parm_cnt = 0;
    int cmd_code=-1;  
    char recipe_exceptions = 0;

	t_count = 0;
	h_count = 0;
	m_count = 0;
    n_count = 0;
    
    enum
    {
        RCP_FLAG_NONE=0,
        RCP_FLAG_SYSTEM,
        RCP_FLAG_FUNC,
        RCP_FLAG_KEY,
        RCP_FLAG_COMPARE,
        RCP_FLAG_TEMP,
        //RCP_FLAG_OTP_CHECK,
        //RCP_FLAG_OTP_TASK,
        //RCP_FLAG_OTP_POWER,
    };

    const char *cmd_list[MAX_CMD_COUNT] = {"AVDD","ELVSS","ELVDD","ADD8_S","ADD8_G","VGH","VGL","VINIT","ASPARE1","ASPARE2","VDD11","VDD18","DSPARE1","DSPARE2","LOGIC",
    "I2C","RSTB","TM","BISTEN","LPSPARE1","DELAY","I2CDELAY","i2cset", "OFF", "SEN","OCP","PAT","OTP","I2CFRE","TEMP","SEND","PATTERN","_0","TOP","BOT","PRE","VREFH","VREFL",
    "TIME","CYCLE","MODEL","INTERVAL","ELOCP","ACOCP","DCOCP","LT","LB","RT","RB","TEMPSEN","ROLL","COMPARE","READ","ELVSSCUR","AVDDCUR", NULL};
    
    #ifdef RELIABILITY_ROOM	//231027 Modify
    const char *compare_cmd_list[MIN_CMD_COUNT] = {"AVDD","ELVSS","ELVDD","ADD8_S","ADD8_G","VGH","VGL","VINIT","ASPARE1","ASPARE2","VDD11","VDD18","DSPARE1","DSPARE2","LDOELVDD","LDOOSC","LDOVGH",
    "LDOVGL","LDOVINT","VCIR","VREF1","VREG1","VOTP50","LSPARE","AVDDCUR","ELVSSCUR", NULL}; 

    const char *temp_cmd_list[MIN_CMD_COUNT] = {"LT","LB","RT","RB", NULL};     
    #endif					//231027 Modify  
    
    //if(system("ls /f0/recipe/recipe.txt"))
    if(access(RECIPE_FILE_PATH,F_OK) != 0)	//230929 Modify
    {
       recipe_file = fopen(RECIPE_FILE_PATH,"wb");                             
    }
    else
    {  
        recipe_file = fopen(RECIPE_FILE_PATH, "r+");
        usleep(100);
    }    
    if(nprf) printf("recipe_file = %d\r\n", recipe_file->_fileno);

    if(recipe_file != NULL)
    {
        memset(&srecipe,0,sizeof(srecipe));
        memset(&signal_group.signal_config.sequence_timing,0,sizeof(signal_group.signal_config.sequence_timing)+sizeof(signal_group.signal_config.display_time)+sizeof(signal_group.signal_config.display_interval)
            +sizeof(signal_group.signal_config.display_cycle)+sizeof(signal_group.signal_config.pattern_sel));
        memset(&compare_set, 0, sizeof(compare_set));
        func_flag=RCP_FLAG_NONE;

        while(fgets(strline, sizeof(strline),recipe_file))
        {
            if(strstr(strline,"[")&&strstr(strline,"]"))
            {  
                func_flag=RCP_FLAG_NONE;
                func_name =  strtok(strline, "[");
                func_name = strtok(func_name, "]");

                if(strcasecmp(func_name,"SYSTEM")==0)
                {
                    func_flag=RCP_FLAG_SYSTEM;
                }  
                #ifdef RELIABILITY_ROOM	//231027 Modify
                else if(strcasecmp(func_name,"COMPARE")==0)
                {                    
                    func_flag=RCP_FLAG_COMPARE;
                } 
                else if(strcasecmp(func_name,"TEMP")==0)
                {                    
                    func_flag=RCP_FLAG_TEMP;
                }                     
                #endif					//231027 Modify	               
                /*else if(strncasecmp(func_name,"KEY",3)==0)
                {
                    key_count_flag++;
                    if(key_count_flag>1)    srecipe.key_count++;
                    memcpy(srecipe.recipe_key[srecipe.key_count].name, func_name,strlen(func_name));
                    func_flag=RCP_FLAG_KEY;
                    if(cprf) printf("key_name = %s\r\n", func_name);
                }*/
                /*else if(strcasecmp(func_name,"OTP_CHECK")==0)
                {
                    func_flag=RCP_FLAG_OTP_CHECK;
                }
                else if(strcasecmp(func_name,"OTP_TASK")==0)
                {
                    func_flag=RCP_FLAG_OTP_TASK;
                }   
                else if(strcasecmp(func_name,"OTP_POWER")==0)
                {
                    func_flag=RCP_FLAG_OTP_POWER;
                }*/                                         
                else // USER FUNC
                {
                    func_count_flag ++;
                    if(func_count_flag>1)   srecipe.recipe_func.func_cnt++; 
                    func_flag=RCP_FLAG_FUNC;
                    if(strncasecmp(func_name,"T",1)==0) t_count++;
                    else if (strncasecmp(func_name,"H",1)==0) h_count++;
                    else if (strncasecmp(func_name,"M",1)==0) m_count++;
                    else    n_count++; 
                    memcpy(srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].name, func_name,strlen(func_name));
                    if(cprf) printf("func_name = %s / T = %d / H = %d / M = %d / N = %d\r\n", func_name, t_count, h_count,m_count,n_count);
                }                     
            }
            else
            {
                cmd_code=-1;  
                if((!isspace(strline[0])) || (!isspace(strline[1])) || (!isspace(strline[2])) || (!isspace(strline[3]))|| (!isspace(strline[4])))
                {   
                    if(strstr(strline,"//"))    ;
                    else if(strstr(strline,"/*"))   recipe_exceptions = 1;
                    else if(strstr(strline,"*/"))   recipe_exceptions = 0;   
                    else if(!recipe_exceptions)
                    {
                        if(strstr(strline,"="))
                        {   
                            cmd_name =  strtok(strline, "=");
                            if((cmd_name != NULL))
                            {
                                parm = strtok(NULL, " ");
                                if(parm != NULL)
                                {
                                    parm_cnt = strlen(parm); 
                                    #ifdef EXCEPT_RELIABILITY_ROOM	//231027 Modify  
                                    for(int i = 0; ; i++)
                                    {
                                        if(cmd_list[i] == NULL) break;  //230929 Modify	
                                        if(!strcasecmp(strline, cmd_list[i]))
                                        {
                                            cmd_code = i;	
                                            break;
                                        }
                                    }
                                    #endif					//231027 Modify
                                    #ifdef RELIABILITY_ROOM	//231027 Modify  
                                    for(int i = 0; ; i++)
                                    {
                                        if(func_flag == RCP_FLAG_COMPARE)
                                        {
                                            if(compare_cmd_list[i] == NULL) break;  //230929 Modify	
                                            if(!strcasecmp(strline, compare_cmd_list[i]))
                                            {
                                                cmd_code = i;	
                                                break;
                                            }                                            
                                        }
                                        else if(func_flag == RCP_FLAG_TEMP)
                                        {
                                            if(temp_cmd_list[i] == NULL) break;  //230929 Modify	
                                            if(!strcasecmp(strline, temp_cmd_list[i]))
                                            {
                                                cmd_code = i;	
                                                break;
                                            }                                             
                                        }
                                        else
                                        {
                                            if(cmd_list[i] == NULL) break;  //230929 Modify	
                                            if(!strcasecmp(strline, cmd_list[i]))
                                            {
                                                cmd_code = i;	
                                                break;
                                            }
                                        }
                                    }
                                    #endif					//231027 Modify                                    
                                    if(cmd_code == MODEL)   model_name_write(parm);
                                    else if(cmd_code == ROLL)   signal_group.signal_config.pattern_sel[srecipe.recipe_func.func_cnt] = atoi(parm);        
                                    else if(cmd_code >=0)
                                    {
                                        switch(func_flag)
                                        {
                                            case    RCP_FLAG_SYSTEM:
                                                srecipe.recipe_sys.cmd[srecipe.recipe_sys.cmd_cnt].code = cmd_code;
                                                memcpy(srecipe.recipe_sys.cmd[srecipe.recipe_sys.cmd_cnt].parm,parm,parm_cnt);
                                                if(cprf) printf("SYSTEM = %d / %s\r\n", srecipe.recipe_sys.cmd[srecipe.recipe_sys.cmd_cnt].code,srecipe.recipe_sys.cmd[srecipe.recipe_sys.cmd_cnt].parm);
                                                srecipe.recipe_sys.cmd_cnt++;                     
                                                break;
                                            
                                            case    RCP_FLAG_FUNC:
                                                srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd[srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd_cnt].code = cmd_code;
                                                memcpy(srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd[srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd_cnt].parm,parm,parm_cnt);
                                                if(cprf) printf("FUNC_DATA = %d / %s\r\n", srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd[srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd_cnt].code,srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd[srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd_cnt].parm);  
                                                srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd_cnt++;
                                                break;

                                            #ifdef RELIABILITY_ROOM	//231027 Modify
                                            case    RCP_FLAG_COMPARE:
                                                compare_value_setting_task(cmd_code,parm);  
                                                break;                                                 

                                            case    RCP_FLAG_TEMP:
                                                temp_value_setting_task(cmd_code,parm);
                                                break;                                                  
                                            #endif					//231027 Modify    

                                            /*case    RCP_FLAG_KEY:
                                                srecipe.recipe_key[srecipe.key_count].cmd[srecipe.recipe_key[srecipe.key_count].cmd_cnt].code = cmd_code;
                                                memcpy(srecipe.recipe_key[srecipe.key_count].cmd[srecipe.recipe_key[srecipe.key_count].cmd_cnt].parm, parm, parm_cnt); 
                                                srecipe.recipe_key[srecipe.key_count].cmd_cnt++;
                                                break;*/ 

                                            /*case    RCP_FLAG_OTP_CHECK:
                                                srecipe.recipe_otpcheck.cmd[srecipe.recipe_otpcheck.cmd_cnt].code = cmd_code;
                                                memcpy(srecipe.recipe_otpcheck.cmd[srecipe.recipe_otpcheck.cmd_cnt].parm,parm,parm_cnt); 
                                                if(cprf) printf("OTP_CHECK = %d / %s\r\n", srecipe.recipe_otpcheck.cmd[srecipe.recipe_otpcheck.cmd_cnt].code, srecipe.recipe_otpcheck.cmd[srecipe.recipe_otpcheck.cmd_cnt].parm);
                                                srecipe.recipe_otpcheck.cmd_cnt++;

                                            case    RCP_FLAG_OTP_TASK:
                                                srecipe.recipe_otptesk.cmd[srecipe.recipe_otptesk.cmd_cnt].code = cmd_code;
                                                memcpy(srecipe.recipe_otptesk.cmd[srecipe.recipe_otptesk.cmd_cnt].parm,parm,parm_cnt); 
                                                if(cprf) printf("OTP_TASK = %d / %s\r\n", srecipe.recipe_otptesk.cmd[srecipe.recipe_otptesk.cmd_cnt].code, srecipe.recipe_otptesk.cmd[srecipe.recipe_otptesk.cmd_cnt].parm);
                                                srecipe.recipe_otptesk.cmd_cnt++;     

                                            case    RCP_FLAG_OTP_POWER:
                                                srecipe.recipe_otppower.cmd[srecipe.recipe_otppower.cmd_cnt].code = cmd_code;
                                                memcpy(srecipe.recipe_otppower.cmd[srecipe.recipe_otppower.cmd_cnt].parm,parm,parm_cnt); 
                                                if(cprf) printf("OTP_POWER = %d / %s\r\n", srecipe.recipe_otppower.cmd[srecipe.recipe_otppower.cmd_cnt].code, srecipe.recipe_otppower.cmd[srecipe.recipe_otppower.cmd_cnt].parm);
                                                srecipe.recipe_otppower.cmd_cnt++;*/                                                                       
                                        }
                                        //printf("FUNC CNT = %d\r\n", srecipe.recipe_func.func_cnt);
                                    }
                                }
                                else
                                {
                                    system_error.recipe = ERROR_NG;   
                                }
                            } 
                            else
                            {
                                system_error.recipe = ERROR_NG;
                            }
                        }
                        else if(strncasecmp(strline,"OFF",3)==0)
                        {
                            cmd_code = OFF;
                            parm = 0;
                            parm_cnt = 0;
                            switch(func_flag)
                            {
                                case    RCP_FLAG_SYSTEM:
                                    srecipe.recipe_sys.cmd[srecipe.recipe_sys.cmd_cnt].code = cmd_code;
                                    memcpy(srecipe.recipe_sys.cmd[srecipe.recipe_sys.cmd_cnt].parm,parm,parm_cnt);
                                    if(cprf) printf("SYSTEM = %d / %s\r\n", srecipe.recipe_sys.cmd[srecipe.recipe_sys.cmd_cnt].code,srecipe.recipe_sys.cmd[srecipe.recipe_sys.cmd_cnt].parm);
                                    srecipe.recipe_sys.cmd_cnt++;                     
                                    break;
                                
                                case    RCP_FLAG_FUNC:
                                    srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd[srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd_cnt].code = cmd_code;
                                    memcpy(srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd[srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd_cnt].parm,parm,parm_cnt);
                                    if(cprf) printf("FUNC_DATA = %d / %s\r\n", srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd[srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd_cnt].code,srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd[srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd_cnt].parm);  
                                    srecipe.recipe_func.func[srecipe.recipe_func.func_cnt].cmd_cnt++;
                                    break;

                                /*case    RCP_FLAG_KEY:
                                    srecipe.recipe_key[srecipe.key_count].cmd[srecipe.recipe_key[srecipe.key_count].cmd_cnt].code = cmd_code;
                                    memcpy(srecipe.recipe_key[srecipe.key_count].cmd[srecipe.recipe_key[srecipe.key_count].cmd_cnt].parm, parm, parm_cnt);
                                    if(cprf) printf("KEY_DATA = %d / %s\r\n", srecipe.recipe_key[srecipe.key_count].cmd[srecipe.recipe_key[srecipe.key_count].cmd_cnt].code,srecipe.recipe_key[srecipe.key_count].cmd[srecipe.recipe_key[srecipe.key_count].cmd_cnt].parm); 
                                    srecipe.recipe_key[srecipe.key_count].cmd_cnt++;
                                    break;*/          
                            }                          
                        }
                    }
                }
            }    
        }
        fclose(recipe_file);
    }
    else    
    {
        system_error.model_download_file_open = ERROR_NG;
        printf("MODEL_DOWNLOAD_FILE_OPEN_ERROR\r\n");
    }
 }

void recipe_system_load(void)
{
    int i = 0;
    int j = 0;
    int cnt = 0;
    char *cmd_name;
    char *parm;
    int parm_data = 0; 
   // char *parm_1;
    //int parm_data_1 = 0; 
    int ii = 0;  
            
    //Power_Supply_Voltage_load(); 
    memset(&signal_group.signal_config.dc_voltage[0], 0, sizeof(signal_group.signal_config.dc_voltage[0])); 

    for(j = 0 ; j<srecipe.recipe_sys.cmd_cnt ; j++)
    {
        if(pg_on_flag != PG_OFF)
        {   
            if(srecipe.recipe_sys.cmd[j].code < 14)
            {  
                if(strncasecmp(srecipe.recipe_sys.cmd[j].parm,"ON",2)==0) 
                {                
                    if(srecipe.recipe_sys.cmd[j].code ==ELVDD)	gpiops_set_value(RELAY_ELVDD,RELAY_ON);
                    else if(srecipe.recipe_sys.cmd[j].code ==ADD8_S)	gpiops_set_value(RELAY_VDD8_S,RELAY_ON);
                    else if(srecipe.recipe_sys.cmd[j].code ==ADD8_G)	gpiops_set_value(RELAY_VDD8_G,RELAY_ON);
                    else if(srecipe.recipe_sys.cmd[j].code ==VGH)	gpiops_set_value(RELAY_VGH,RELAY_ON);
                    else if(srecipe.recipe_sys.cmd[j].code ==VGL)	gpiops_set_value(RELAY_VGL,RELAY_ON);	
                    else if(srecipe.recipe_sys.cmd[j].code ==VINIT)	gpiops_set_value(RELAY_VINIT,RELAY_ON);	
                    usleep(5);
                    gpio_enble->GPIO_DATA |= (0x00000001<<(srecipe.recipe_sys.cmd[j].code&0x3fff));             
                    if(cprf) printf("signal_out[%d] = %d / gpio_enble->GPIO_DATA = %x\r\n", j, srecipe.recipe_sys.cmd[j].code,gpio_enble->GPIO_DATA);  
                } 
                else if(strncasecmp(srecipe.recipe_sys.cmd[j].parm,"OFF",3)==0) 
                {                      
                    //SET_DAC_OUTPUT_VALUE(srecipe.recipe_func.func[i].cmd[j].code);
                    gpio_enble->GPIO_DATA ^= (0x00000001<<(srecipe.recipe_sys.cmd[j].code&0x3fff)); 
                    usleep(5);
                    if(srecipe.recipe_sys.cmd[j].code ==ELVDD)	gpiops_set_value(RELAY_ELVDD,RELAY_OFF);
                    else if(srecipe.recipe_sys.cmd[j].code ==ADD8_S)	gpiops_set_value(RELAY_VDD8_S,RELAY_OFF);
                    else if(srecipe.recipe_sys.cmd[j].code ==ADD8_G)	gpiops_set_value(RELAY_VDD8_G,RELAY_OFF);
                    else if(srecipe.recipe_sys.cmd[j].code ==VGH)	gpiops_set_value(RELAY_VGH,RELAY_OFF);
                    else if(srecipe.recipe_sys.cmd[j].code ==VGL)	gpiops_set_value(RELAY_VGL,RELAY_OFF);	
                    else if(srecipe.recipe_sys.cmd[j].code ==VINIT)	gpiops_set_value(RELAY_VINIT,RELAY_OFF);	                       
                    if(cprf) printf("signal_out[%d] = %d / gpio_enble->GPIO_DATA = %x\r\n", j, srecipe.recipe_sys.cmd[j].code, gpio_enble->GPIO_DATA);    
                }
                else
                {
                    signal_group.signal_config.dc_voltage[srecipe.recipe_sys.cmd[j].code] = atoi(srecipe.recipe_sys.cmd[j].parm);
                    if(signal_group.signal_config.dc_voltage[srecipe.recipe_sys.cmd[j].code] >= 0 )
                    {
                        if(signal_group.signal_config.dc_voltage[srecipe.recipe_sys.cmd[j].code] > signal_group.signal_config.dc_voltage[srecipe.recipe_sys.cmd[j-1].code]) 
                        {               
                            Power_Supply_Voltage_load();     
                        }   
                    }
                    else
                    {
                        if(signal_group.signal_config.dc_voltage[srecipe.recipe_sys.cmd[j].code] < signal_group.signal_config.dc_voltage[srecipe.recipe_sys.cmd[j-1].code]) 
                        {
                                                            
                            Power_Supply_Voltage_load();     
                        } 
                    } 
                        if(cprf) printf("signal[%d] = %d\r\n", j,signal_group.signal_config.dc_voltage[srecipe.recipe_sys.cmd[j].code]);   
                    SET_DAC_OUTPUT_VALUE(srecipe.recipe_sys.cmd[j].code);                                        
                }                                    
            }
            else
            {
                if(srecipe.recipe_sys.cmd[j].code == LOGIC)
                {
                    signal_group.signal_config.dc_voltage[LOGIC] = atoi(srecipe.recipe_sys.cmd[j].parm);
                    dac_set_for_logic();
                    if(cprf) printf("LOGIC_VOLTAGE_SET = %d\r\n", signal_group.signal_config.dc_voltage[LOGIC]);
                } 
                else if(srecipe.recipe_sys.cmd[j].code == I2C)
                {
                    signal_group.signal_config.dc_voltage[I2C] = atoi(srecipe.recipe_sys.cmd[j].parm);
                    dac_set_for_i2c();
                    if(cprf) printf("I2C_VOLTAGE_SET = %d\r\n", signal_group.signal_config.dc_voltage[I2C]);                        
                }
                else if(srecipe.recipe_sys.cmd[j].code == OCP_ON_OFF)
                {
                    if(strncasecmp(srecipe.recipe_sys.cmd[j].parm,"ON",2)==0) 
                    { 
                        ocp->OCP_CONTROL = 1<<0;	
                        pattern_generator->PG_OCP_DELAY = 10;
                        pattern_generator->PG_CONTROL = 1<<0;
                        ocp->OCP_EN_MASK = 	0x3fff;		
                        if(cprf)    printf("ocp_set!_ok\r\n");                            
                        //ocp_flag_on = 1;                                     
                        ts_ocp_flag_on_set(total_status,OCP_ON);
                    }
                    else if(strncasecmp(srecipe.recipe_sys.cmd[j].parm,"OFF",3)==0) 
                    {
                        //ocp_flag_on = 0; 
                        ocp->OCP_CONTROL = 1<<0;
                        ocp->OCP_EN_MASK = 	0x0000;	
                        //ocp_flag_on = 0;               
                        ts_ocp_flag_on_set(total_status,OCP_OFF);
                    }
                }
                else if(srecipe.recipe_sys.cmd[j].code == I2CFRE)
                {  
                    int frequency_index = 0;
                    frequency_index = atoi(srecipe.recipe_sys.cmd[j].parm);
                    if((frequency_index < 5) && (frequency_index > -1))  i2c_frequency_set(frequency_index);
                }
                //if(srecipe.recipe_sys.cmd[j].code == DELAY)
                else if(srecipe.recipe_sys.cmd[j].code == DELAY)    		//230929 Modify
                {
                    if(cprf) printf("delay = %d\r\n",atoi(srecipe.recipe_sys.cmd[j].parm));
                    ensis_delay(atoi(srecipe.recipe_sys.cmd[j].parm));      
                }
                else if((srecipe.recipe_sys.cmd[j].code == RSTB)||(srecipe.recipe_sys.cmd[j].code == TM) || (srecipe.recipe_sys.cmd[j].code == BISTEN)|| (srecipe.recipe_sys.cmd[j].code == LPSPARE1))
                {
                    if(srecipe.recipe_sys.cmd[j].code == RSTB) logic_gpio_ctl("RSTB",atoi(srecipe.recipe_sys.cmd[j].parm));   
                    else if(srecipe.recipe_sys.cmd[j].code == TM)  logic_gpio_ctl("TM",atoi(srecipe.recipe_sys.cmd[j].parm)); 
                    else if(srecipe.recipe_sys.cmd[j].code == BISTEN)  logic_gpio_ctl("BISTEN",atoi(srecipe.recipe_sys.cmd[j].parm)); 
                    else if(srecipe.recipe_sys.cmd[j].code == LPSPARE1)  logic_gpio_ctl("LPSPARE1",atoi(srecipe.recipe_sys.cmd[j].parm));                           
                }
                else if(srecipe.recipe_sys.cmd[j].code == I2CDELAY)
                {
                    USER_DELAY = atoi(srecipe.recipe_sys.cmd[j].parm);

                    if(cprf) printf("USER_DELAY_SETTING = %d\r\n", USER_DELAY);                        
                }   
                else if(srecipe.recipe_sys.cmd[j].code == SEN) 
                {
                    memset(parm_copy, 0, sizeof(parm_copy));
                    memcpy(parm_copy,srecipe.recipe_sys.cmd[j].parm,strlen(srecipe.recipe_sys.cmd[j].parm));
                    cmd_name =  strtok(parm_copy, ",");
                    if(cmd_name != NULL)
                    {
                        parm = strtok(NULL, " ");
                        if(parm != NULL)
                        {
                            parm_data = atoi(parm);
                            if(!(adc_sen_monitoring(cmd_name,parm_data)))    
                            {   
                                j = srecipe.recipe_sys.cmd_cnt;
                                power_off();
                            }
                        } 
                    } 
                    cmd_name=NULL; 
                    parm=NULL;
                }
                else if(srecipe.recipe_sys.cmd[j].code == SEND)
                { 
                    memset(parm_copy, 0, sizeof(parm_copy));
                    memcpy(parm_copy,srecipe.recipe_sys.cmd[j].parm,strlen(srecipe.recipe_sys.cmd[j].parm));
                    send_command_task();
                    usleep(USER_DELAY);                                                                                   
                }
                else if(srecipe.recipe_sys.cmd[j].code == CYCLE)
                {
                    signal_group.signal_config.display_cycle = atoi(srecipe.recipe_sys.cmd[j].parm);  
                    if(cprf) printf("CYCLE = %d\r\n",signal_group.signal_config.display_cycle);                    
                }
                else if(srecipe.recipe_sys.cmd[j].code == EL_OCP)
                { 
                    el_ocp_set(atoi(srecipe.recipe_sys.cmd[j].parm));
                    if(cprf) printf("EL_OCP_VALUE = %d\r\n",atoi(srecipe.recipe_sys.cmd[j].parm));                    
                }
                else if(srecipe.recipe_sys.cmd[j].code == AC_OCP)
                {  
                    ap_ocp_set(atoi(srecipe.recipe_sys.cmd[j].parm));
                    if(cprf) printf("AC_OCP_VALUE = %d\r\n",atoi(srecipe.recipe_sys.cmd[j].parm));                    
                }
                else if(srecipe.recipe_sys.cmd[j].code == DC_OCP)
                {
                    dp_ocp_set(atoi(srecipe.recipe_sys.cmd[j].parm));
                    if(cprf) printf("DC_OCP_VALUE = %d\r\n",atoi(srecipe.recipe_sys.cmd[j].parm));                    
                }
                else if(srecipe.recipe_sys.cmd[j].code == LT)
                {
                    /*memset(parm_copy, 0, sizeof(parm_copy));
                    memcpy(parm_copy,srecipe.recipe_sys.cmd[j].parm,strlen(srecipe.recipe_sys.cmd[j].parm));

                    temperature_data.lt_min =   strtol(strtok(parm_copy, ","),NULL,10);
                    temperature_data.lt_max =   strtol(strtok(NULL, ","),NULL,10);   
                    if(cprf) printf(" temperature_data.lt_min = %d / temperature_data.lt_max = %d\r\n",temperature_data.lt_min,temperature_data.lt_max);*/                
                } 
                else if(srecipe.recipe_sys.cmd[j].code == LB)   	//231101 Modify
                {
                    /*memset(parm_copy, 0, sizeof(parm_copy));
                    memcpy(parm_copy,srecipe.recipe_sys.cmd[j].parm,strlen(srecipe.recipe_sys.cmd[j].parm));

                    temperature_data.lb_min =   strtol(strtok(parm_copy, ","),NULL,10);
                    temperature_data.lb_max =   strtol(strtok(NULL, ","),NULL,10);   
                    if(cprf) printf(" temperature_data.lb_min = %d / temperature_data.lb_max = %d\r\n",temperature_data.lb_min,temperature_data.lb_max);*/                 
                }
                else if(srecipe.recipe_sys.cmd[j].code == RT)   	//231101 Modify
                {
                    /*memset(parm_copy, 0, sizeof(parm_copy));
                    memcpy(parm_copy,srecipe.recipe_sys.cmd[j].parm,strlen(srecipe.recipe_sys.cmd[j].parm));

                    temperature_data.rt_min =   strtol(strtok(parm_copy, ","),NULL,10);
                    temperature_data.rt_max =   strtol(strtok(NULL, ","),NULL,10);   
                    if(cprf) printf(" temperature_data.rt_min = %d / temperature_data.rt_max = %d\r\n",temperature_data.rt_min,temperature_data.rt_max);*/                 
                } 
                else if(srecipe.recipe_sys.cmd[j].code == RB)   	//231101 Modify
                {
                    /*memset(parm_copy, 0, sizeof(parm_copy));
                    memcpy(parm_copy,srecipe.recipe_sys.cmd[j].parm,strlen(srecipe.recipe_sys.cmd[j].parm));

                    temperature_data.rb_min =   strtol(strtok(parm_copy, ","),NULL,10);
                    temperature_data.rb_max =   strtol(strtok(NULL, ","),NULL,10);   
                    if(cprf) printf(" temperature_data.rb_min = %d / temperature_data.rb_max = %d\r\n",temperature_data.rb_min,temperature_data.rb_max);*/                  
                }
                else if(srecipe.recipe_sys.cmd[j].code == TEMPSEN)  	//231101 Modify
                {
                    /*if(strncasecmp(srecipe.recipe_sys.cmd[j].parm,"ON",2)==0) 
                    { 	
                        if(cprf)    printf("otp_set!_ok\r\n");                            
                        otp_flag_on = 1;                                     
                    }
                    else if(strncasecmp(srecipe.recipe_sys.cmd[j].parm,"OFF",3)==0) 
                    {
                        if(cprf)    printf("otp_set!_off\r\n");  
                        otp_flag_on = 0;     
                    }*/                
                }                                                                                                                                                                                                                                   
            }             
        }
    }  
}

void recipe_funtion_load(char* func)
 {
    int i = 0;
    int j = 0;
    int cnt = 0;
    char *cmd_name;
    char *parm;
    int parm_data = 0; 
    char *parm_1;
    int parm_data_1 = 0; 
    int ii = 0;  
    short dc_voltage[20] = {0,};
            
    //Power_Supply_Voltage_load(); 
    //memset(&signal_group.signal_config.dc_voltage[0], 0, 40);  

    for(i = 0 ; i<= srecipe.recipe_func.func_cnt ; i++)
    {
        if(cprf) printf("func[%d].name = %s / func = %s\r\n", i, srecipe.recipe_func.func[i].name, func);
        if(strcasecmp(srecipe.recipe_func.func[i].name,func)==0)
        {
            for(j = 0 ; j<srecipe.recipe_func.func[i].cmd_cnt ; j++)
            {
                if(pg_on_flag != PG_OFF)
                {                 
                    if(srecipe.recipe_func.func[i].cmd[j].code < 14)
                    {
                        if(strncasecmp(srecipe.recipe_func.func[i].cmd[j].parm,"ON",2)==0) 
                        {                        
                            //SET_DAC_OUTPUT_VALUE(srecipe.recipe_func.func[i].cmd[j].code);                        
                            if(srecipe.recipe_func.func[i].cmd[j].code ==ELVDD)	gpiops_set_value(RELAY_ELVDD,RELAY_ON);
                            else if(srecipe.recipe_func.func[i].cmd[j].code ==ADD8_S)	gpiops_set_value(RELAY_VDD8_S,RELAY_ON);
                            else if(srecipe.recipe_func.func[i].cmd[j].code ==ADD8_G)	gpiops_set_value(RELAY_VDD8_G,RELAY_ON);
                            else if(srecipe.recipe_func.func[i].cmd[j].code ==VGH)	gpiops_set_value(RELAY_VGH,RELAY_ON);
                            else if(srecipe.recipe_func.func[i].cmd[j].code ==VGL)	gpiops_set_value(RELAY_VGL,RELAY_ON);	
                            else if(srecipe.recipe_func.func[i].cmd[j].code ==VINIT)	gpiops_set_value(RELAY_VINIT,RELAY_ON);	
                            usleep(5);
                            gpio_enble->GPIO_DATA |= (0x00000001<<(srecipe.recipe_func.func[i].cmd[j].code&0x3fff));             
                            if(cprf) printf("signal_out[%d] = %d / gpio_enble->GPIO_DATA = %x\r\n", j, srecipe.recipe_func.func[i].cmd[j].code,gpio_enble->GPIO_DATA);   
                        }
                        else if(strncasecmp(srecipe.recipe_func.func[i].cmd[j].parm,"OFF",3)==0) 
                        {                       
                            //SET_DAC_OUTPUT_VALUE(srecipe.recipe_func.func[i].cmd[j].code);
                            gpio_enble->GPIO_DATA ^= (0x00000001<<(srecipe.recipe_func.func[i].cmd[j].code&0x3fff)); 
                            usleep(5);
                            if(srecipe.recipe_func.func[i].cmd[j].code ==ELVDD)	gpiops_set_value(RELAY_ELVDD,RELAY_OFF);
                            else if(srecipe.recipe_func.func[i].cmd[j].code ==ADD8_S)	gpiops_set_value(RELAY_VDD8_S,RELAY_OFF);
                            else if(srecipe.recipe_func.func[i].cmd[j].code ==ADD8_G)	gpiops_set_value(RELAY_VDD8_G,RELAY_OFF);
                            else if(srecipe.recipe_func.func[i].cmd[j].code ==VGH)	gpiops_set_value(RELAY_VGH,RELAY_OFF);
                            else if(srecipe.recipe_func.func[i].cmd[j].code ==VGL)	gpiops_set_value(RELAY_VGL,RELAY_OFF);	
                            else if(srecipe.recipe_func.func[i].cmd[j].code ==VINIT)	gpiops_set_value(RELAY_VINIT,RELAY_OFF);	                       
                            if(cprf) printf("signal_out[%d] = %d / gpio_enble->GPIO_DATA = %x\r\n", j, srecipe.recipe_func.func[i].cmd[j].code,gpio_enble->GPIO_DATA);    
                        }                    
                        else
                        {
                            signal_group.signal_config.dc_voltage[srecipe.recipe_func.func[i].cmd[j].code] = atoi(srecipe.recipe_func.func[i].cmd[j].parm);
                            dc_voltage[srecipe.recipe_func.func[i].cmd[j].code] = atoi(srecipe.recipe_func.func[i].cmd[j].parm); 
                            if(signal_group.signal_config.dc_voltage[srecipe.recipe_func.func[i].cmd[j].code] >= 0 )
                            //if(dc_voltage[srecipe.recipe_func.func[i].cmd[j].code] >= 0 )
                            {
                                if(signal_group.signal_config.dc_voltage[srecipe.recipe_func.func[i].cmd[j].code] > signal_group.signal_config.dc_voltage[srecipe.recipe_func.func[i].cmd[j-1].code]) 
                                //if(dc_voltage[srecipe.recipe_func.func[i].cmd[j].code] > dc_voltage[srecipe.recipe_func.func[i].cmd[j-1].code]) 
                                {
                                    //Power_Supply_Voltage_load_pjh(dc_voltage);                
                                    Power_Supply_Voltage_load();     
                                }   
                            }
                            else
                            {
                                if(signal_group.signal_config.dc_voltage[srecipe.recipe_func.func[i].cmd[j].code] < signal_group.signal_config.dc_voltage[srecipe.recipe_func.func[i].cmd[j-1].code]) 
                                {
                                                                
                                Power_Supply_Voltage_load();     
                                } 
                            } 
                                if(cprf) printf("signal[%d] = %d\r\n", j,signal_group.signal_config.dc_voltage[srecipe.recipe_func.func[i].cmd[j].code]);   
                            SET_DAC_OUTPUT_VALUE(srecipe.recipe_func.func[i].cmd[j].code);                     
                        }                                    
                    }
                    else
                    {
                        if(srecipe.recipe_func.func[i].cmd[j].code == DELAY)
                        {
                        //ensis_delay(atoi(srecipe.recipe_func.func[i].cmd[j].parm));
                            if(cprf) printf("delay = %d\r\n",atoi(srecipe.recipe_func.func[i].cmd[j].parm));
                            ensis_delay(atoi(srecipe.recipe_func.func[i].cmd[j].parm));      
                        }
                        else if(srecipe.recipe_func.func[i].cmd[j].code == LOGIC)
                        {
                            signal_group.signal_config.dc_voltage[LOGIC] = atoi(srecipe.recipe_func.func[i].cmd[j].parm);
                            dac_set_for_logic();
                            if(cprf) printf("LOGIC_VOLTAGE_SET = %d\r\n", signal_group.signal_config.dc_voltage[LOGIC]);
                        } 
                        else if(srecipe.recipe_func.func[i].cmd[j].code == I2C)
                        {
                            signal_group.signal_config.dc_voltage[I2C] = atoi(srecipe.recipe_func.func[i].cmd[j].parm);
                            dac_set_for_i2c();
                            if(cprf) printf("I2C_VOLTAGE_SET = %d\r\n", signal_group.signal_config.dc_voltage[I2C]);                        
                        }
                        else if(srecipe.recipe_func.func[i].cmd[j].code == OCP_ON_OFF)
                        {
                            if(strncasecmp(srecipe.recipe_func.func[i].cmd[j].parm,"ON",2)==0) 
                            { 
                                ocp->OCP_CONTROL = 1<<0;	
                                pattern_generator->PG_OCP_DELAY = 10;
                                pattern_generator->PG_CONTROL = 1<<0;
                                ocp->OCP_EN_MASK = 	0x3fff;			
                                if(cprf)    printf("ocp_set!_ok\r\n");                            
                                ocp_flag_on = 1;                                     
                            }
                            else if(strncasecmp(srecipe.recipe_func.func[i].cmd[j].parm,"OFF",3)==0) 
                            {
                                ocp->OCP_CONTROL = 1<<0;
                                ocp->OCP_EN_MASK = 	0x0000;	
                                ocp_flag_on = 0;     
                            }
                        }                    
                        else if((srecipe.recipe_func.func[i].cmd[j].code == RSTB)||(srecipe.recipe_func.func[i].cmd[j].code == TM) || (srecipe.recipe_func.func[i].cmd[j].code == BISTEN)|| (srecipe.recipe_func.func[i].cmd[j].code == LPSPARE1))
                        {
                            if(srecipe.recipe_func.func[i].cmd[j].code == RSTB) logic_gpio_ctl("RSTB",atoi(srecipe.recipe_func.func[i].cmd[j].parm));   
                            else if(srecipe.recipe_func.func[i].cmd[j].code == TM)  logic_gpio_ctl("TM",atoi(srecipe.recipe_func.func[i].cmd[j].parm)); 
                            else if(srecipe.recipe_func.func[i].cmd[j].code == BISTEN)  logic_gpio_ctl("BISTEN",atoi(srecipe.recipe_func.func[i].cmd[j].parm)); 
                            else if(srecipe.recipe_func.func[i].cmd[j].code == LPSPARE1)  logic_gpio_ctl("LPSPARE1",atoi(srecipe.recipe_func.func[i].cmd[j].parm));                           
                        } 
                        else if(srecipe.recipe_func.func[i].cmd[j].code == OFF)
                        {
                            if(interval_start_flag) interval_off(); 
                            else power_off(); 
                            interval_start_flag = 0;                 
                        }
                        else if(srecipe.recipe_func.func[i].cmd[j].code == SEN) 
                        {
                            memset(parm_copy, 0, sizeof(parm_copy));
                            memcpy(parm_copy,srecipe.recipe_func.func[i].cmd[j].parm,strlen(srecipe.recipe_func.func[i].cmd[j].parm));
                            cmd_name =  strtok(parm_copy, ",");
                            parm = strtok(NULL, " ");
                            parm_data = atoi(parm);
                            if(!(adc_sen_monitoring(cmd_name,parm_data)))    
                            {   
                                j = srecipe.recipe_func.func[i].cmd_cnt;
                                power_off();
                            }   
                        } 
                        else if(srecipe.recipe_func.func[i].cmd[j].code == PAT)
                        {
                            memset(parm_copy, 0, sizeof(parm_copy));
                            memcpy(parm_copy,srecipe.recipe_func.func[i].cmd[j].parm,strlen(srecipe.recipe_func.func[i].cmd[j].parm));
                            cmd_name =  strtok(parm_copy, ",");
                            parm = strtok(NULL, " ");
                            parm_data = atoi(parm); 
                            i2c_com_bist_pattern(cmd_name,parm_data,FULL_COLOR_MODE1); 
                            //printf("cmd_name = %s / parm = %s / parm_data = %d\r\n", cmd_name, parm, parm_data);                                             
                        } 
                        else if(srecipe.recipe_func.func[i].cmd[j].code == OTP)
                        {
                            i2c_com_otp_loading();
                        }
                        else if(srecipe.recipe_func.func[i].cmd[j].code == I2CFRE)
                        {  
                            int frequency_index = 0;
                            frequency_index = atoi(srecipe.recipe_func.func[i].cmd[j].parm);
                            if((frequency_index < 5) && (frequency_index > -1))  i2c_frequency_set(frequency_index);
                        }
                        else if(srecipe.recipe_func.func[i].cmd[j].code == TEMP)
                        { 

                        } 
                        else if(srecipe.recipe_func.func[i].cmd[j].code == SEND)
                        { 
                            memset(parm_copy, 0, sizeof(parm_copy));
                            memcpy(parm_copy,srecipe.recipe_func.func[i].cmd[j].parm,strlen(srecipe.recipe_func.func[i].cmd[j].parm));
                            send_command_task();    //230929 Modify	
                            usleep(USER_DELAY);                                                                                   
                        }
                        else if(srecipe.recipe_func.func[i].cmd[j].code == PATTERN)
                        { 
                            memset(pattern_name, 0, sizeof(pattern_name));    //231027 Modify
                            //memcpy(pattern_name,srecipe.recipe_func.func[i].cmd[j].parm,sizeof(pattern_name));  //230929 Modify
                            memcpy(pattern_name,srecipe.recipe_func.func[i].cmd[j].parm,(strlen(srecipe.recipe_func.func[i].cmd[j].parm))-1);  //231027 Modify
                        }                     
                        else if(srecipe.recipe_func.func[i].cmd[j].code == GMA_0)
                        { 
                            unsigned char slave_address = i2c_wr_addr>>1;
                            int reg_address = 0;
                            int write_byte = 0;    
                            int count_i = 0;
                            unsigned short data_buffer[3] = {0};                    
                            unsigned char write_buffer[10] = {0};
                            unsigned char com_cnt = 0;

                            memset(parm_copy, 0, sizeof(parm_copy));
                            memcpy(parm_copy,srecipe.recipe_func.func[i].cmd[j].parm,strlen(srecipe.recipe_func.func[i].cmd[j].parm));

                            reg_address = _0_ADDR;
                            write_byte = 3;
                            if(cprf)    printf("slave_address = %x / reg_address = %x / com_cnt = %d\r\n", slave_address, reg_address, write_byte);
                            for(count_i = 0 ; count_i < write_byte ; count_i++)
                            {
                                if(count_i == 0)    data_buffer[count_i] =  strtol(strtok(parm_copy, ","),NULL,10);    
                                else data_buffer[count_i] = strtol(strtok(NULL, ","),NULL,10);
                                if(cprf)    printf("data_buffer[%d] = %d\r\n", count_i,data_buffer[count_i]);
                            }
                            
                            write_byte = 4;
                            write_buffer[0] = MAN_GMA_EN | (((data_buffer[0]&MAN_R_G_B_9BIT_10BIT)>>8)<<4) | (((data_buffer[1]&MAN_R_G_B_9BIT_10BIT)>>8)<<2) | (((data_buffer[2]&MAN_R_G_B_9BIT_10BIT)>>8));
                            write_buffer[1] = data_buffer[0]&0xff;
                            write_buffer[2] = data_buffer[1]&0xff;
                            write_buffer[3] = data_buffer[2]&0xff;          

                            i2c_write(slave_address,reg_address,write_buffer,write_byte); 
                            usleep(USER_DELAY);                                                                                   
                        }
                        else if(srecipe.recipe_func.func[i].cmd[j].code == GMA_TOP)
                        {
                            unsigned char slave_address = i2c_wr_addr>>1;
                            int reg_address = 0;
                            int write_byte = 0;    
                            int count_i = 0;
                            unsigned short data_buffer[3] = {0};                    
                            unsigned char write_buffer[10] = {0};
                            unsigned char com_cnt = 0;

                            memset(parm_copy, 0, sizeof(parm_copy));
                            memcpy(parm_copy,srecipe.recipe_func.func[i].cmd[j].parm,strlen(srecipe.recipe_func.func[i].cmd[j].parm));

                            reg_address = TOP_ADDR;
                            write_byte = 3;
                            for(count_i = 0 ; count_i < write_byte ; count_i++)
                            {
                                if(count_i == 0)    data_buffer[count_i] =  strtol(strtok(parm_copy, ","),NULL,10);    
                                else data_buffer[count_i] = strtol(strtok(NULL, ","),NULL,10);
                                if(cprf)    printf("data_buffer[%d] = %d\r\n", count_i,data_buffer[count_i]);
                            } 
                            
                            write_byte = 4;
                            write_buffer[0] = (((data_buffer[0]&MAN_R_G_B_9BIT_10BIT)>>8)<<4) | (((data_buffer[1]&MAN_R_G_B_9BIT_10BIT)>>8)<<2) | (((data_buffer[2]&MAN_R_G_B_9BIT_10BIT)>>8));
                            write_buffer[1] = data_buffer[0]&0xff;
                            write_buffer[2] = data_buffer[1]&0xff;
                            write_buffer[3] = data_buffer[2]&0xff;          

                            i2c_write(slave_address,reg_address,write_buffer,write_byte);  
                            usleep(USER_DELAY);                                                  
                        } 
                        else if(srecipe.recipe_func.func[i].cmd[j].code == GMA_BOT)
                        {
                            unsigned char slave_address = i2c_wr_addr>>1;
                            int reg_address = 0;
                            int write_byte = 0;    
                            int count_i = 0;
                            unsigned short data_buffer[3] = {0};                    
                            unsigned char write_buffer[10] = {0};
                            unsigned char com_cnt = 0;

                            memset(parm_copy, 0, sizeof(parm_copy));
                            memcpy(parm_copy,srecipe.recipe_func.func[i].cmd[j].parm,strlen(srecipe.recipe_func.func[i].cmd[j].parm));

                            reg_address = BOT_ADDR;
                            write_byte = 3;
                            for(count_i = 0 ; count_i < write_byte ; count_i++)
                            {
                                if(count_i == 0)    data_buffer[count_i] =  strtol(strtok(parm_copy, ","),NULL,10);    
                                else data_buffer[count_i] = strtol(strtok(NULL, ","),NULL,10);
                                if(cprf)    printf("data_buffer[%d] = %d\r\n", count_i,data_buffer[count_i]);
                            } 
                            
                            write_byte = 4;
                            write_buffer[0] = (((data_buffer[0]&MAN_R_G_B_9BIT_10BIT)>>8)<<4) | (((data_buffer[1]&MAN_R_G_B_9BIT_10BIT)>>8)<<2) | (((data_buffer[2]&MAN_R_G_B_9BIT_10BIT)>>8));
                            write_buffer[1] = data_buffer[0]&0xff;
                            write_buffer[2] = data_buffer[1]&0xff;
                            write_buffer[3] = data_buffer[2]&0xff;          

                            i2c_write(slave_address,reg_address,write_buffer,write_byte);  
                            usleep(USER_DELAY);                                                  
                        } 
                        else if(srecipe.recipe_func.func[i].cmd[j].code == GMA_PRE)
                        {
                            unsigned char slave_address = i2c_wr_addr>>1;
                            int reg_address = 0;
                            int write_byte = 0;    
                            int count_i = 0;
                            unsigned char data_buffer[3] = {0};                    
                            unsigned char write_buffer[10] = {0};
                            unsigned char com_cnt = 0;

                            memset(parm_copy, 0, sizeof(parm_copy));
                            memcpy(parm_copy,srecipe.recipe_func.func[i].cmd[j].parm,strlen(srecipe.recipe_func.func[i].cmd[j].parm));

                            reg_address = PRE_ADDR;
                            write_byte = 3;
                            for(count_i = 0 ; count_i < write_byte ; count_i++)
                            {
                                if(count_i == 0)    data_buffer[count_i] =  strtol(strtok(parm_copy, ","),NULL,10);    
                                else data_buffer[count_i] = strtol(strtok(NULL, ","),NULL,10);
                                if(cprf)    printf("data_buffer[%d] = %d\r\n", count_i,data_buffer[count_i]);
                            } 
                            
                            write_byte = 3;
                            write_buffer[0] = data_buffer[0]&0xff;
                            write_buffer[1] = data_buffer[1]&0xff;
                            write_buffer[2] = data_buffer[2]&0xff;          

                            i2c_write(slave_address,reg_address,write_buffer,write_byte);  
                            usleep(USER_DELAY);                                                  
                        }
                        else if(srecipe.recipe_func.func[i].cmd[j].code == GMA_VREFH)
                        {
                            unsigned char slave_address = i2c_wr_addr>>1;
                            int reg_address = 0;
                            int write_byte = 0;    
                            unsigned char data_buffer[3] = {0};                    
                            unsigned char write_buffer[10] = {0};

                            data_buffer[0] = atoi(srecipe.recipe_func.func[i].cmd[j].parm);
                            reg_address = VREFH_ADDR;
                            write_byte = 1;
                            if(data_buffer[0] > 0x1F) data_buffer[0] = 0x1F;
                            write_buffer[0] = 0x40 | (data_buffer[0]&0xff);

                            i2c_write(slave_address,reg_address,write_buffer,write_byte);  
                            usleep(USER_DELAY);                          
                        }
                        else if(srecipe.recipe_func.func[i].cmd[j].code == GMA_VREFL)
                        {
                            unsigned char slave_address = i2c_wr_addr>>1;
                            int reg_address = 0;
                            int write_byte = 0;    
                            unsigned char data_buffer[3] = {0};                    
                            unsigned char write_buffer[10] = {0};

                            data_buffer[0] = atoi(srecipe.recipe_func.func[i].cmd[j].parm);
                            reg_address = VREFL_ADDR;
                            write_byte = 1;
                            if(data_buffer[0] > 0x1F) data_buffer[0] = 0x1F; 
                            write_buffer[0] = 0x40 | (data_buffer[0]&0xff);

                            i2c_write(slave_address,reg_address,write_buffer,write_byte);  
                            usleep(USER_DELAY);                          
                        } 
                        else if(srecipe.recipe_func.func[i].cmd[j].code == I2CDELAY)
                        {
                            USER_DELAY = atoi(srecipe.recipe_func.func[i].cmd[j].parm);
                            if(cprf) printf("USER_DELAY_SETTING = %d\r\n", USER_DELAY);                        
                        }   
                        else if(srecipe.recipe_func.func[i].cmd[j].code == TIME)
                        {
                            signal_group.signal_config.display_time[i] = (atoi(srecipe.recipe_func.func[i].cmd[j].parm))*1000;
                            if(cprf) printf("signal_group.signal_config.display_time[%d] = %d\r\n",i, signal_group.signal_config.display_time[i]);                           
                        } 
                        else if(srecipe.recipe_func.func[i].cmd[j].code == INTERVAL)
                        {
                            signal_group.signal_config.display_interval[i] = (atoi(srecipe.recipe_func.func[i].cmd[j].parm))*1000;
                            if(cprf) printf("signal_group.signal_config.display_interval[%d] = %d\r\n",i, signal_group.signal_config.display_interval[i]);                               
                        }
                        else if(srecipe.recipe_func.func[i].cmd[j].code == EL_OCP)
                        { 
                            el_ocp_set(atoi(srecipe.recipe_func.func[i].cmd[j].parm));
                            if(cprf) printf("EL_OCP_VALUE = %d\r\n",atoi(srecipe.recipe_func.func[i].cmd[j].parm));                    
                        }
                        else if(srecipe.recipe_func.func[i].cmd[j].code == AC_OCP)
                        {  
                            ap_ocp_set(atoi(srecipe.recipe_func.func[i].cmd[j].parm));
                            if(cprf) printf("AC_OCP_VALUE = %d\r\n",atoi(srecipe.recipe_func.func[i].cmd[j].parm));                    
                        }
                        else if(srecipe.recipe_func.func[i].cmd[j].code == DC_OCP)
                        {
                            dp_ocp_set(atoi(srecipe.recipe_func.func[i].cmd[j].parm));
                            if(cprf) printf("DC_OCP_VALUE = %d\r\n",atoi(srecipe.recipe_func.func[i].cmd[j].parm));                    
                        }
                        else if(srecipe.recipe_func.func[i].cmd[j].code == READ)
                        { 
                            memset(parm_copy, 0, sizeof(parm_copy));
                            memcpy(parm_copy,srecipe.recipe_func.func[i].cmd[j].parm,strlen(srecipe.recipe_func.func[i].cmd[j].parm));
                            read_command_task();
                            usleep(USER_DELAY);                                                                               
                        }
                        else if(srecipe.recipe_func.func[i].cmd[j].code == ELVSSCUR)
                        { 
                            memset(parm_copy, 0, sizeof(parm_copy));
                            memcpy(parm_copy,srecipe.recipe_func.func[i].cmd[j].parm,strlen(srecipe.recipe_func.func[i].cmd[j].parm));
                            elvss_cur_command_task();                   
                        }
                        else if(srecipe.recipe_func.func[i].cmd[j].code == AVDDCUR)
                        {                  
                            memset(parm_copy, 0, sizeof(parm_copy));
                            memcpy(parm_copy,srecipe.recipe_func.func[i].cmd[j].parm,strlen(srecipe.recipe_func.func[i].cmd[j].parm));
                            avdd_cur_command_task();
                        }                                                                        
                    }
                }
            }     
        } 
    }      
 }

void power_off(void)
 {
    pg_on_flag = PG_OFF;   
    ts_ocp_flag_on_set(total_status,OCP_OFF);   //231013 Modify 
    //ocp_flag_on = 0;                          //231013 Modify
    ts_rcb_mode_ocp_flag_on_set(total_status,RCB_OCP_MODE_OFF); //231027 Modify   
    ocp->OCP_CONTROL = 1<<0;     
    mux_init();
    usleep(5);
    relay_init();       
    pattern_generator->PG_CONTROL = DAC_nCLR_LOW; 
    usleep(1);                       
    pattern_generator->PG_CONTROL = DAC_nCLR_HIGH;	
    usleep(1);	
    pattern_generator->PG_CONTROL = DAC_nCLR_LOW;
    usleep(1);	
    pattern_generator->PG_CONTROL = OUT_EN_ON;  //231027 Modify    
    //system_load_flag = 0;                   
    //pvsig_onoff(false);
    //nvsig_onoff(false); 
    usleep(1);                                               
    pvsig_voltage_set(power_vol_min);
    nvsig_voltage_set(power_vol_min);                       
    usleep(1);
    logic_gpio_ctl("ALL",0);
    i2c_logic_defalut();    	                        
    el_ocp_set(1100);
    ap_ocp_set(300);
    dp_ocp_set(300);
    //button_led_off();        //230929 Modify    
    system_load_flag = 0; 
    pat_index = 0xff;           //230929 Modify          
 }

void interval_off(void)
 {
    mux_init();
    usleep(5);
    relay_init();    
    /*pattern_generator->PG_CONTROL = DAC_nCLR_LOW; 
    usleep(1);                       
    pattern_generator->PG_CONTROL = DAC_nCLR_HIGH;	
    usleep(1);	
    pattern_generator->PG_CONTROL = DAC_nCLR_LOW;                    
    usleep(1);                                               
    pvsig_voltage_set(power_vol_min);
    nvsig_voltage_set(power_vol_min);*/                       
    usleep(1);
    logic_gpio_ctl("ALL",0);
    i2c_logic_defalut(); 
    el_ocp_set(1100);
    ap_ocp_set(300);
    dp_ocp_set(300);   	                         
 }

void i2c_com_otp_loading(void)
 {
    unsigned char slave_address = i2c_wr_addr>>1;
    unsigned short reg_address = OTP_LOADING_CMD;
    unsigned char write_buffer[3] = {0};  
    int write_byte = 3;  
    write_buffer[0] = 0x00;
    write_buffer[1] = 0x00;
    write_buffer[2] = 0x01;

    i2c_write(slave_address,reg_address,write_buffer,write_byte);
 }

 void i2c_com_bist_pattern(char *index, int data, char mode)
 {
    short index_num = 0;
    unsigned char slave_address = i2c_wr_addr>>1;
    unsigned short reg_address = BIST_PATTERN0_ADDR;
    unsigned char write_buffer[4] = {0};
    int write_byte = 4;
    unsigned char bist_flag = 0;

    if(strcasecmp(index,"R")==0)    index_num = RED;
    else if(strcasecmp(index,"G")==0)   index_num = GREEN;
    else if(strcasecmp(index,"B")==0)   index_num = BLUE;
    else if(strcasecmp(index,"W")==0)   index_num = WHITE;
    else    bist_flag = 1;         
    
    if(!bist_flag)
    {
        write_buffer[0] = 0x00;
        write_buffer[1] = 0x00;
        
        write_buffer[2] = index_num<<4;
        write_buffer[2] = (write_buffer[2]) | ((data >> 4) & 0x0f);  

        write_buffer[3] = (data & 0x0f);
        write_buffer[3] = write_buffer[3] << 4;
        write_buffer[3] = ((write_buffer[3]) | mode);                      

        if(cprf)
        {
            printf("index_num = %x / data = %x / mode = %x\r\n", index_num, data, mode);
            //printf("reg_address = %x / write_buffer[0] = %x / write_buffer[1] = %x / write_buffer[2] = %x / write_buffer[3] = %x\r\n",  reg_address, write_buffer[0], write_buffer[1],  write_buffer[2], write_buffer[3]);
        }
        
        if(0 <= i2c_write(slave_address,reg_address,write_buffer,write_byte))
        {
            if(cprf)    printf("BIST_PATTERN__WRITE_OK\r\n");
            usleep(10000);
            uart_ack();
        }
        else 
        {
            if(cprf)   printf("BIST_PATTERN_WRITE_NG\r\n");
            error_data.i2c_communication = ERROR_NG; 
			usleep(10000);
			uart_nack();    	//230929 Modify	            
        }
    }
    else    if(eprf)    printf("bist_pattern index not found = [%c]\r\n", index); 
 }

void i2c_com_gma_block(unsigned short reg_address_ex, unsigned short* data)
{
    unsigned char slave_address = i2c_wr_addr>>1; 
    unsigned short reg_address = reg_address_ex;
    unsigned char write_buffer[4] = {0};    
    short index_num = 0;  
    int write_byte = 0;    
    
    if(reg_address == PRE_ADDR)   
    {
        write_byte = 3;
        write_buffer[0] = data[0]&0xff;
        write_buffer[1] = data[1]&0xff;
        write_buffer[2] = data[2]&0xff;        
    }
    else if(reg_address == _0_ADDR) 
    {
        write_byte = 4;
        //write_buffer[0] = MAN_GMA_EN | (((data[0]&MAN_R_G_B_9BIT_10BIT)>>8)<<4) | (((data[1]&MAN_R_G_B_9BIT_10BIT)>>8)<<2) | (((data[2]&MAN_R_G_B_9BIT_10BIT)>>8));
        write_buffer[0] = (((data[0]&MAN_R_G_B_9BIT_10BIT)>>8)<<4) | (((data[1]&MAN_R_G_B_9BIT_10BIT)>>8)<<2) | (((data[2]&MAN_R_G_B_9BIT_10BIT)>>8));
        write_buffer[1] = data[0]&0xff;
        write_buffer[2] = data[1]&0xff;
        write_buffer[3] = data[2]&0xff;
    }
    else 
    {
        write_byte = 4;
        write_buffer[0] = (((data[0]&MAN_R_G_B_9BIT_10BIT)>>8)<<4) | (((data[1]&MAN_R_G_B_9BIT_10BIT)>>8)<<2) | (((data[2]&MAN_R_G_B_9BIT_10BIT)>>8));
        write_buffer[1] = data[0]&0xff;
        write_buffer[2] = data[1]&0xff;
        write_buffer[3] = data[2]&0xff;
    }
    
    if(cprf)
    {
         printf("data[0] = %x / data[1] = %x / data[2] = %x / data[3] = %x\r\n", data[0], data[1], data[2], data[3]);
         //printf("reg_address = %x / write_buffer[0] = %x / write_buffer[1] = %x / write_buffer[2] = %x / write_buffer[3] = %x\r\n",  reg_address, write_buffer[0], write_buffer[1],  write_buffer[2], write_buffer[3]);
    }
    //i2c_write(slave_address,reg_address,write_buffer,write_byte);
    if(0 <= i2c_write(slave_address,reg_address,write_buffer,write_byte))
    {
        if(cprf)    printf("GMA_BLOCK_WRITE_OK\r\n");
        usleep(10000);
        uart_ack();   
        //uart_index_ack(GMA_BLOCK_REQUEST);
    } 
    else    
    {
        if(cprf)    printf("GMA_BLOCK_WRITE_NG\r\n"); 
        error_data.i2c_communication = ERROR_NG;   
        usleep(10000);
        uart_nack();    	//231012 Modify               
    } 
}

void i2c_logic_defalut(void)
{
	signal_group.signal_config.dc_voltage[LOGIC] = 1800;
	dac_set_for_logic();

	signal_group.signal_config.dc_voltage[I2C] = 1800;
	dac_set_for_i2c();	    
}

void relay_init(void)
{
    gpiops_set_value(RELAY_ELVDD,RELAY_OFF);
    gpiops_set_value(RELAY_VDD8_S,RELAY_OFF);
    gpiops_set_value(RELAY_VDD8_G,RELAY_OFF);
    gpiops_set_value(RELAY_VGH,RELAY_OFF);
    gpiops_set_value(RELAY_VGL,RELAY_OFF);
    gpiops_set_value(RELAY_VINIT,RELAY_OFF);    
    
    if(cprf)    printf("RELAY_INIT_OK\r\n");
}

void mux_init(void)
{
    gpio_enble->GPIO_DATA = 0x00000000; 
    if(cprf)    printf("MUX_INIT_OK\r\n");
}

void i2c_frequency_set(char index)
{
    short a = 0;
    char c[3]  = {0,};
    float vol_0 = 0;
    float vol_1 = 0;
    unsigned char i=0,*id;   
    if(index == I2C_RATE_100KHZ)
    {
        vol_0 = 0;
        vol_1 = signal_group.signal_config.dc_voltage[I2C]; 
        i2c_set_frequency(100000); 
        if(cprf) printf("I2C_Frequency : 100KHZ\r\n");      
    }

    else if(index == I2C_RATE_125KHZ)
    {
        vol_0 = 0;
        vol_1 = 900; 
        i2c_set_frequency(125000);
        if(cprf) printf("I2C_Frequency : 125KHZ\r\n");             
    }
    else if(index == I2C_RATE_250KHZ)
    {
        vol_0 = signal_group.signal_config.dc_voltage[I2C]; 
        vol_1 = 0; 
        i2c_set_frequency(250000); 
        if(cprf) printf("I2C_Frequency : 250KHZ\r\n");           
    }
    else if(index == I2C_RATE_500KHZ)
    {
        vol_0 = 900; 
        vol_1 = 0; 
        i2c_set_frequency(500000);  
        if(cprf) printf("I2C_Frequency : 500KHZ\r\n");          
    }
    else if(index == I2C_RATE_1MHZ)
    {
        vol_0 = 0; 
        vol_1 = 0; 
        i2c_set_frequency(1100000); 
        if(cprf) printf("I2C_Frequency : 1MHZ\r\n");           
    }    

    if(vol_0 > 5000) vol_0 = 5000;
    if(vol_0 < 0) vol_0 = 0;
    if(vol_1 > 5000) vol_1 = 5000;
    if(vol_1 < 0) vol_1 = 0;
    a = (short)(((65535/10)*(((vol_0/1000))+4.85)));	
    c[2] = a &0xff;
    c[1] = (a>>8) &0xff;
    c[0] = SPEED_0;	
    dac_set(DAC0,c,3);	

    a = (short)(((65535/10)*(((vol_1/1000))+4.85)));	
    c[2] = a &0xff;
    c[1] = (a>>8) &0xff;
    c[0] = SPEED_1;	
    dac_set(DAC0,c,3);	        
}
				
void bist_test(unsigned char ch, short vol)
{
    int i = 0;
    int ii = 0;
    int count = 0;

    if(vol > 3000)  vol = 3000;

    gpiops_set_value(RELAY_ELVDD,RELAY_ON);  
    gpiops_set_value(RELAY_VDD8_S,RELAY_ON);
    gpiops_set_value(RELAY_VDD8_G,RELAY_ON);
    gpiops_set_value(RELAY_VGH,RELAY_ON);
    gpiops_set_value(RELAY_VGL,RELAY_ON); 
    gpiops_set_value(RELAY_VINIT,RELAY_ON); 

    signal_group.signal_config.dc_voltage[ch] = vol;
    Power_Supply_Voltage_load();   
    SET_DAC_OUTPUT_VALUE(ch);
    gpio_enble->GPIO_DATA = 0x00000001 <<ch; 
    usleep(1000);  
    ADC_AUTO_DATA_READ(); 
    memset(&signal_group.signal_config.dc_voltage[0], 0, 40);  
    power_off();       			
}

void rgb_voltage_request(void)
{
    unsigned char slave_address = i2c_wr_addr>>1;
    unsigned short checksum = 0;
    int reg_address = 0;
    char read_buffer[15] = {0};
    char read_buffer_vref[2] = {0}; 
    char send_buffer[40] = {0}; 
    unsigned char send_len = 0;
    int read_byte = 0;
    int i = 0;

    send_buffer[send_len++] = 0x02;
    send_buffer[send_len++] = PATTERN_R_G_B_REQUEST;
    send_buffer[send_len++] = 0x80;
    send_buffer[send_len++] = PATTERN_R_G_B_REQUEST__AC_LEN;

    reg_address = 0x0206;
    read_byte = 15;				
    i2c_read(slave_address,reg_address,read_buffer,read_byte);
    if(aprf)
    {
        for(i = 0 ; i < read_byte ; i++)
        {
            printf("reg_address = %x / READ_DATA = [%x]\r\n", reg_address++, read_buffer[i]);
        }
    }

    send_buffer[send_len++] = (read_buffer[REG_ADDRESS_518] >> 4)&0x03; 
    send_buffer[send_len++] = read_buffer[REG_ADDRESS_519]; 

    send_buffer[send_len++] = (read_buffer[REG_ADDRESS_518] >> 2)&0x03; 
    send_buffer[send_len++] = read_buffer[REG_ADDRESS_520];

    send_buffer[send_len++] = (read_buffer[REG_ADDRESS_518] >> 0)&0x03; 
    send_buffer[send_len++] = read_buffer[REG_ADDRESS_521]; 

    send_buffer[send_len++] = (read_buffer[REG_ADDRESS_522] >> 4)&0x03; 
    send_buffer[send_len++] = read_buffer[REG_ADDRESS_523]; 

    send_buffer[send_len++] = (read_buffer[REG_ADDRESS_522] >> 2)&0x03; 
    send_buffer[send_len++] = read_buffer[REG_ADDRESS_524];

    send_buffer[send_len++] = (read_buffer[REG_ADDRESS_522] >> 0)&0x03; 
    send_buffer[send_len++] = read_buffer[REG_ADDRESS_525];  

    send_buffer[send_len++] = (read_buffer[REG_ADDRESS_526] >> 4)&0x03; 
    send_buffer[send_len++] = read_buffer[REG_ADDRESS_527]; 

    send_buffer[send_len++] = (read_buffer[REG_ADDRESS_526] >> 2)&0x03; 
    send_buffer[send_len++] = read_buffer[REG_ADDRESS_528];

    send_buffer[send_len++] = (read_buffer[REG_ADDRESS_526] >> 0)&0x03; 
    send_buffer[send_len++] = read_buffer[REG_ADDRESS_529];  

    send_buffer[send_len++] = 0x00;
    send_buffer[send_len++] = read_buffer[REG_ADDRESS_530];
    send_buffer[send_len++] = 0x00;    
    send_buffer[send_len++] = read_buffer[REG_ADDRESS_531];
    send_buffer[send_len++] = 0x00;
    send_buffer[send_len++] = read_buffer[REG_ADDRESS_532]; 

    reg_address = 0x01FE;
    read_byte = 2;				
    i2c_read(slave_address,reg_address,read_buffer_vref,read_byte); 
    send_buffer[send_len++] = 0x00;
    send_buffer[send_len++] = read_buffer_vref[0];   
    send_buffer[send_len++] = 0x00;
    send_buffer[send_len++] = read_buffer_vref[1];

    if(aprf)
    {
        for(i = 0 ; i < send_len ; i++)
        {
            printf("SEND_DATA[%d] = [%x] \r\n", i, send_buffer[i]);
        }
    }

    ADC_SELECT_DATA_READ_AVG(SEN_VREG1);
    if(aprf)	printf("VREG = %d\r\n", adc_sensing_value[SEN_VREG1]);	
    send_buffer[send_len++] = (adc_sensing_value[SEN_VREG1]>>8)&0xff;	   
    send_buffer[send_len++] = adc_sensing_value[SEN_VREG1]&0xff;
        
    ADC_SELECT_DATA_READ_AVG(SEN_VREF1);
    if(aprf) 	printf("VREF = %d\r\n", adc_sensing_value[SEN_VREF1]);            
    send_buffer[send_len++] = (adc_sensing_value[SEN_VREF1]>>8)&0xff;	   
    send_buffer[send_len++] = adc_sensing_value[SEN_VREF1]&0xff;   

    for(i=1; i<send_len; i++)	checksum ^= *(send_buffer+i);	
    send_buffer[send_len++] = checksum;
    send_buffer[send_len++] = 0x03;
    sdcd_serial_write(send_len, send_buffer);
    memset(&send_buffer, 0, sizeof(send_buffer));
}

void bist_pattern_read_data_request(void)
{
    unsigned char slave_address = i2c_wr_addr>>1;
    unsigned short checksum = 0;
    int reg_address = 0;
    char read_buffer[15] = {0};
    char read_buffer_vref[2] = {0}; 
    char send_buffer[40] = {0}; 
    unsigned char send_len = 0;
    int read_byte = 0;
    int i = 0;

    send_buffer[send_len++] = 0x02;
    send_buffer[send_len++] = BIST_PATTERN_DATA_READ_REQUEST;
    send_buffer[send_len++] = 0x80;
    send_buffer[send_len++] = BIST_PATTERN_DATA_READ_REQUEST_ACK_LEN;

    reg_address = 0x028e;
    read_byte = 2;				
    i2c_read(slave_address,reg_address,read_buffer,read_byte);
    if(aprf)
    {
        for(i = 0 ; i < read_byte ; i++)
        {
            printf("reg_address = %x / READ_DATA = [%x]\r\n", reg_address++, read_buffer[i]);
        }
    }
        
    send_buffer[send_len++] = (read_buffer[REG_ADDRESS_654] >> 4)&0x03;
    send_buffer[send_len++] = (read_buffer[REG_ADDRESS_654] << 4)&0xf0 | (read_buffer[REG_ADDRESS_655] >> 4)&0x0f;

    if(aprf)
    {
        for(i = 0 ; i < send_len ; i++)
        {
            printf("SEND_DATA[%d] = [%x] \r\n", i, send_buffer[i]);
        }
    }  

    for(i=1; i<send_len; i++)	checksum ^= *(send_buffer+i);	
    send_buffer[send_len++] = checksum;
    send_buffer[send_len++] = 0x03;
    sdcd_serial_write(send_len, send_buffer);
    memset(&send_buffer, 0, sizeof(send_buffer));
}

#ifdef RELIABILITY_ROOM	//231027 Modify
void *sensing_task()
{
    double elvss_real_value = 0;
    double elvss_filter_value = 0;
    double elvss_filter_value_pre = 0;
    double elvss_result = 0;
    static int elvss_i;

    double avdd_real_value = 0;
    double avdd_filter_value = 0;
    double avdd_filter_value_pre = 0;
    double avdd_result = 0;
    static int avdd_i;

    double lpf_tau;
    double lpt_time;

    int elvss_4byte = 0;
    short elvss_2byte = 0;
    int avdd_4byte = 0;
    short avdd_2byte = 0;

    elvss_filter_value = 0.0;
    avdd_filter_value = 0.0;
			
    while(1)
    {
        if((pg_on_flag == AUTO_RUN) || (pg_on_flag == CAL_RUN)) 		//230929 Modify	
        {
            ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);

            ADC_SELECT_DATA_READ_AVG(SEN_ELISS);
            if(cur_sensing_reset_flag)  //Initializing the event occurrence flag
            {
                cur_sensing_reset_flag = 0;
                elvss_i = 0;
                avdd_i = 0;
            }
            if(elvss_i == 0)
            {
                elvss_filter_value = elvss_cur_sensing_data;
                elvss_filter_value_pre = elvss_cur_sensing_data;
                elvss_i = 1;
            }            
            else
            {
                lpf_tau = 0.001;
                lpt_time = 0.00005;
                elvss_real_value = ((lpf_tau/(lpf_tau+lpt_time))*elvss_filter_value) + ((lpt_time/(lpf_tau+lpt_time)) * ((double)elvss_cur_sensing_data));

                elvss_filter_value = ((lpf_tau/(lpf_tau+lpt_time))*elvss_filter_value) + ((lpt_time/(lpf_tau+lpt_time)) * ((double)elvss_real_value));
            }
            elvss_result = fabs(elvss_filter_value_pre-elvss_cur_sensing_data);
            if(elrf)	printf("ELVSS_filter_value[%02d] = %f \t real = %f \t elvss_cur_sensing_data = %f \t result = %f\r\n", elvss_i, elvss_filter_value,elvss_real_value,elvss_cur_sensing_data,elvss_result);
            elvss_4byte = fabs(elvss_filter_value*1000);  //4byte_data
            elvss_2byte = fabs(elvss_filter_value*10);    //2byte_data
            elvss_4byte_cur_ex = elvss_4byte;
            elvss_2byte_cur_ex = elvss_2byte;
            elvss_2byte_offset = elvss_filter_value;
            elvss_filter_value_pre = elvss_cur_sensing_data;
            if(elvss_result> 0.1)	//Reset if 0.1 difference from previous value
            {
                elvss_i =0;
            } 
            //---------------------------------------------------------------------------------------------------------------------------------//
              
            ADC_SELECT_DATA_READ_AVG(SEN_ELIDD);
            if(avdd_i == 0)
            {
                avdd_filter_value = avdd_cur_sensing_data;
                avdd_filter_value_pre = avdd_cur_sensing_data;
                avdd_i = 1;
            }            
            else
            {
                lpf_tau = 0.001;
                lpt_time = 0.00005;
                avdd_real_value = ((lpf_tau/(lpf_tau+lpt_time))*avdd_filter_value) + ((lpt_time/(lpf_tau+lpt_time)) * ((double)avdd_cur_sensing_data));

                avdd_filter_value = ((lpf_tau/(lpf_tau+lpt_time))*avdd_filter_value) + ((lpt_time/(lpf_tau+lpt_time)) * ((double)avdd_real_value));
            }
            avdd_result = fabs(avdd_filter_value_pre-avdd_cur_sensing_data);
            if(avrf)	printf("AVDD_filter_value[%02d] = %f \t real = %f \t avdd_cur_sensing_data = %f \t result = %f\r\n", avdd_i, avdd_filter_value,avdd_real_value,avdd_cur_sensing_data,avdd_result);
            avdd_4byte = fabs(avdd_filter_value*1000);
            avdd_2byte = fabs(avdd_filter_value*10);
            avdd_4byte_cur_ex = avdd_4byte;
            avdd_2byte_cur_ex = avdd_2byte;
            avdd_filter_value_pre = avdd_cur_sensing_data;
            avdd_2byte_offset = avdd_filter_value;
            if(avdd_result> 0.1)	
            {
                avdd_i =0;
            } 
            ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);
            ADC_AUTO_DATA_READ();
            memcpy(compare_adc_sensing_value,adc_sensing_value,sizeof(adc_sensing_value));             
            if(aprf) ADC_DATA_PRINT();  
        }
        else
        {
            if(total_sen_flag)
            {
                total_sen_flag = 0;
                ADC_AUTO_DATA_READ();               
                if(aprf) ADC_DATA_PRINT();        
            }
        }
        //usleep(1);
        usleep(10);
    }
}
#endif					//231027 Modify	

void model_name_check(void)
{
    FILE *name_file;
    int file_size = 0;

    model_name_size = 0;

    //if(system("ls /f0/recipe/model.txt"))
    if(access(MODEL_NAME_FILE_PATH,F_OK) != 0)	//230929 Modify
    {
        name_file = fopen(MODEL_NAME_FILE_PATH,"wb");                          
    }
    else
    {
        memset(&model_name, 0, sizeof(model_name));
        name_file = fopen(MODEL_NAME_FILE_PATH, "r");
        if(name_file != NULL)
        {
            fseek(name_file, 0, SEEK_END);  
            file_size = ftell(name_file); 
            model_name_size = file_size; 
            fseek(name_file, 0, SEEK_SET); 
            if(file_size >= MODEL_NAME_LEN)   file_size = MODEL_NAME_LEN;   //230929 Modify 
            fread(model_name, file_size, 1, name_file);            
        }
    }
    if(name_file != NULL)   fclose(name_file);    
}

#ifdef RELIABILITY_ROOM	//231027 Modify
void aging_mode_task(void)
{
    static unsigned char flag;
    //static unsigned int cycle_count;
    static unsigned char interval_flag;
    unsigned char aging_string_index[5] = {0};
    static unsigned char compare_vol_cur_result_flag;
    static unsigned char compare_temp_result_flag;

    if(pg_on_flag == AUTO_RUN)
    {
        if(cycle_count <= (signal_group.signal_config.display_cycle)-1)
        {
            if((n_count-1) >= aging_pat_index)
            {
                if(aging_pat_index == 0) 
                {
                    if(flag == 0)
                    {
                        //if(cycle_count == 0)    recipe_system_load();          //230929 Modify                                          
                        if(pg_on_flag != PG_OFF)    //OCP 발생 시 진행하지 않음
                        {
                            if(signal_group.signal_config.pattern_sel[aging_pat_index])    sprintf(aging_string_index, "%d",aging_pat_index);      //ROLL값이 0이면 다음패턴으로 넘어감 //231117 Modify  
                            if(signal_group.signal_config.pattern_sel[aging_pat_index])    recipe_funtion_load(aging_string_index);    //ROLL값이 0이면 다음패턴으로 넘어감             //231117 Modify 
                            if(signal_group.signal_config.pattern_sel[aging_pat_index])    display_task(NULL,pattern_name);   //ROLL값이 0이면 다음패턴으로 넘어감 //230929 Modify      //231117 Modify	
                            flag = 1; 
                            if(cycle_count == 0)   auto_log_time_start = 1; 
                            if(cycle_count == 0)   auto_log_time();    
                            if(cycle_count == 0)   log_task(AUTO_RUN_START,(unsigned short)aging_pat_index);
                            else    
                            {
                                if(signal_group.signal_config.pattern_sel[aging_pat_index])    auto_log_time();    //ROLL값이 0이면 다음패턴으로 넘어감 
                                if(signal_group.signal_config.pattern_sel[aging_pat_index])    log_task(AUTO_NEXT_PATTERN,(unsigned short)aging_pat_index);    //ROLL값이 0이면 다음패턴으로 넘어감      
                            }
                            //if(signal_group.signal_config.pattern_sel[aging_pat_index])    sprintf(aging_string_index, "%d",aging_pat_index);      //ROLL값이 0이면 다음패턴으로 넘어감 
                            //if(signal_group.signal_config.pattern_sel[aging_pat_index])    recipe_funtion_load(aging_string_index);    //ROLL값이 0이면 다음패턴으로 넘어감 
                            //if(signal_group.signal_config.pattern_sel[aging_pat_index])    display_task(NULL,pattern_name);   //ROLL값이 0이면 다음패턴으로 넘어감 //230929 Modify	                            
                        }                                                 
                    } 
                }
                else if(aging_pat_index <= (n_count-1))
                {
                    if(flag == 0)
                    {                    
                        if(aging_pat_index == (n_count-1))  ;
                        else
                        {
                            if(signal_group.signal_config.pattern_sel[aging_pat_index])    sprintf(aging_string_index, "%d",aging_pat_index);  //ROLL값이 0이면 다음패턴으로 넘어감 
                            if(signal_group.signal_config.pattern_sel[aging_pat_index])    recipe_funtion_load(aging_string_index);    //ROLL값이 0이면 다음패턴으로 넘어감 
                            if(signal_group.signal_config.pattern_sel[aging_pat_index])    display_task(NULL,pattern_name);   //ROLL값이 0이면 다음패턴으로 넘어감       //230929 Modify 	
                            flag = 1; 
                            if(signal_group.signal_config.pattern_sel[aging_pat_index])    log_task(AUTO_NEXT_PATTERN,(unsigned short)aging_pat_index);    //ROLL값이 0이면 다음패턴으로 넘어감 
                        }                       
                    }             
                }
                if(pg_on_flag != PG_OFF)    //OCP 발생 시 진행하지 않음
                { 
                    if(signal_group.signal_config.pattern_sel[aging_pat_index])    //ROLL값이 0이면 다음패턴으로 넘어감
                    {
                        if(signal_group.signal_config.display_time[aging_pat_index] != 0)     //time값이 0이면 다음패턴으로 넘어감
                        {              
                            if(ms_timecheck_array(signal_group.signal_config.display_time[aging_pat_index]) == true)   
                            {                                                      
                                if((cycle_count == 0) & (aging_pat_index == 0)) ;
                                else    
                                {
                                    compare_vol_cur_result_flag = compare_vol_cur_task(); 
                                    compare_temp_result_flag = compare_temp_task();
                                }

                                if((!compare_vol_cur_result_flag) & (!compare_temp_result_flag))
                                {                                   
                                    log_task(CUR_RECORD,(unsigned short)aging_pat_index);
                                    if(signal_group.signal_config.display_interval[aging_pat_index] != 0)
                                    {
                                        if(interval_flag == 0)  interval_flag = 1;
                                    }
                                    else
                                    {
                                        interval_flag = 0;
                                        aging_pat_index++;
                                        aging_pat_change_flag = 1;
                                        flag = 0;
                                    }
                                }
                                else    
                                {
                                    compare_vol_cur_result_flag = 0;  
                                    compare_temp_result_flag = 0;
                                }  
                            }
                            if(interval_flag > 0)
                            {
                                if(pg_on_flag != PG_OFF) 
                                {
                                    if(interval_flag == 1)  
                                    {
                                        interval_check(INTERVA_START_FLAG);
                                        interval_flag = 2;
                                    }
                                    else if(interval_flag == 2)
                                    {
                                        if(ms_timecheck_array_2(signal_group.signal_config.display_interval[aging_pat_index]) == true)
                                        {
                                            interval_check(INTERVAL_END_FLAG);
                                            interval_flag = 0;
                                            flag = 0;                                                   
                                        }                         
                                    } 
                                }
                                else    interval_flag = 0;                      
                            }
                        }
                    }
                    else
                    {
                        interval_flag = 0;
                        aging_pat_index++;
                        aging_pat_change_flag = 1;
                        flag = 0;                        
                    }
                }                            
            }
            else if((n_count-1) < aging_pat_index)
            {
                //pg_on_flag = PG_OFF;
                interval_flag = 0;
                aging_pat_index = 0;
                flag = 0; 
                log_task(CYCLE_COMPLETE,(unsigned short)cycle_count+1);  
                cycle_count++;         
            }
        }
        else
        {            
            memset(&aging_result_string, 0, sizeof(aging_result_string));
            if(cycle_count == (signal_group.signal_config.display_cycle)) snprintf(aging_result_string,sizeof(aging_result_string),"AGING_COMPLETE"); 
            else    snprintf(aging_result_string,sizeof(aging_result_string),"AGING_FAILED");    
            aging_off_task();
        }
    }
    else
    {          
        aging_pat_change_flag = 1;
        interval_flag = 0;
        cycle_count = 0;  
        flag = 0;  
        compare_vol_cur_result_flag = 0;  
        compare_temp_result_flag = 0;        
    }
}


static bool ms_timecheck_array(unsigned int ms)
{
    static int state = 0; //0 : Check Start, 1 : Check End, 2 : Clear Stanby
    static struct timeval start,end;
    static int delay;

    bool ret = false;
    int diff_time;
    if(aging_pat_change_flag)   
    {
        aging_pat_change_flag = 0;
        state = 0;
    }

    switch(state){
        case 0:
        gettimeofday(&start, NULL);
        delay = ms;
        state = 1;
        break;
        case 1:       
        gettimeofday(&end, NULL);
        diff_time = ((end.tv_sec - start.tv_sec) * 1000) + ((end.tv_usec - start.tv_usec) / 1000);
        if(diff_time >= delay){
            ret = true;
            state = 2;
        }
        break;
        case 2: // Clear Standby
        break;
        default:
        state = 0;
        ret = false;
        break;
    }
            //printf("diff_time = %d / delay = %d / state = %d / ret = %d\r\n",diff_time, delay, state, ret);
    return ret;
}

static bool ms_timecheck_array_2(unsigned int ms)
{
    static int state = 0; //0 : Check Start, 1 : Check End, 2 : Clear Stanby
    static struct timeval start,end;
    static int delay;

    bool ret = false;
    int diff_time;
    if(aging_pat_interval_change_flag)   
    {
        aging_pat_interval_change_flag = 0;
        state = 0;
    }

    switch(state){
        case 0:
        gettimeofday(&start, NULL);
        delay = ms;
        state = 1;
        break;
        case 1:       
        gettimeofday(&end, NULL);
        diff_time = ((end.tv_sec - start.tv_sec) * 1000) + ((end.tv_usec - start.tv_usec) / 1000);
        if(diff_time >= delay){
            ret = true;
            state = 2;
        }
        break;
        case 2: // Clear Standby
        break;
        default:
        state = 0;
        ret = false;
        break;
    }
            //printf("diff_time = %d / delay = %d / state = %d / ret = %d\r\n",diff_time, delay, state, ret);
    return ret;
}

int auto_log_time(void)
{
    static int state = 2; //0 : Check Start, 1 : Check End, 2 : Clear Stanby
    static struct timeval start,end;
    static int delay;

    int ret = 0;
    unsigned int diff_time;
    unsigned int sec_time;
    unsigned int usec_time;

    if(auto_log_time_start == 1) 
    {
        state = 0;
        auto_log_time_start = 0;
    }
    else if(auto_log_time_start == 2)
    {
        state = 2;
    }    

    switch(state){
        case 0:
        gettimeofday(&start, NULL);
        //delay = ms;
        state = 1;
        break;
        case 1:
        gettimeofday(&end, NULL);
        sec_time = end.tv_sec - start.tv_sec;
        //usec_time = end.tv_usec - start.tv_usec;
        if(end.tv_usec >= start.tv_usec) usec_time = end.tv_usec - start.tv_usec;
        else    
        {
            sec_time = sec_time - 1;
            usec_time = end.tv_usec+ (1000000 - start.tv_usec);  
        }      
        time_count.hour = sec_time/3600;
        time_count.min = (sec_time%3600)/60;
        time_count.sec = (sec_time%3600)%60;
        time_count.msec = usec_time/1000;
        //printf("%04d:%02d:%02d:%03d\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec);
        break;
        case 2: // Clear Standby
        break;
        default:
        state = 0;
        ret = 0;
        break;
    }
    return ret;
}
#endif					//231027 Modify	

int create_file(void)
{
    FILE *fp = NULL;
    DIR *dir_ptr = NULL;
    struct dirent *file = NULL;
    int i = 0;
    int temp_base_number;
    char pre_index_name[50] = {0,};
    char index_name[50] = {0,};

    if((dir_ptr = opendir("/f0/log")) == NULL)
    {
        if(nprf)    printf("directory find fail\r\n");
        return -1;
    }
    else
    {
        while((file = readdir(dir_ptr)) != NULL)
        {
            i++;        
        }
        closedir(dir_ptr);    
        return i-2;
    }
}

#ifdef RELIABILITY_ROOM	//231027 Modify
void auto_run(void)
{
    aging_pat_index = 0;  
    pg_on_flag = AUTO_SYSTEM_RUN;          //230929 Modify
    //pg_on_flag = AUTO_RUN; 
    recipe_system_load();  
    if(pg_on_flag == AUTO_SYSTEM_RUN)   pg_on_flag = AUTO_RUN; //231027 Modify     
}

//void log_task(unsigned char index, unsigned char num)
void log_task(unsigned char index, unsigned short num)  //231027 Modify  
{
    FILE *adc_file;
    unsigned int log_count;
    static char cmd[30];
    char log[100] = {0,};
    //char log_index_1[30] = {0,};
    //char log_index_2[30] = {0,};
    int i = 0;

    if(index == AUTO_RUN_START)
    {
        log_count = create_file();
        file_count(log_count-1);
        if(log_count != -1)
        {
            if(log_count > (log_index_count+1))     //231027 Modify   
            {
                delete_log_file();
                log_count = 1;
            }
            memset(&cmd,0,sizeof(cmd));
            log_index_file_change(log_count-1);  
            snprintf(cmd, sizeof(cmd), "/f0/log/auto_run[%d].txt", log_count-1);   //LAST_INDEX.txt FILE Exception
            //fopen(cmd,"wb");
            adc_file = fopen(cmd, "w+b");

            if(adc_file != NULL)
            {
                auto_log_time(); 
                memset(&log,0,sizeof(log));     
                snprintf(log, sizeof(log), "%04d:%02d:%02d:%03d  ****************AUTO RUN Start****************\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec);
                fwrite(&log, strlen(log), 1, adc_file);

                auto_log_time();
                memset(&log,0,sizeof(log));  
                snprintf(log, sizeof(log), "%04d:%02d:%02d:%03d  MODEL_NAME = %s\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec,model_name);
                fwrite(&log, strlen(log), 1, adc_file);

                auto_log_time();
                memset(&log,0,sizeof(log));  
                snprintf(log, sizeof(log), "%04d:%02d:%02d:%03d  PATTERN_TOTAL_COUNT = %d\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec,srecipe.recipe_func.func_cnt);
                fwrite(&log, strlen(log), 1, adc_file);

                auto_log_time();
                memset(&log,0,sizeof(log));  
                snprintf(log, sizeof(log), "%04d:%02d:%02d:%03d  AUTO_RUN_CYCLE = %d\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec,signal_group.signal_config.display_cycle);
                fwrite(&log, strlen(log), 1, adc_file);           

                auto_log_time(); 
                memset(&log,0,sizeof(log));     
                snprintf(log, sizeof(log), "%04d:%02d:%02d:%03d  **********************************************\r\n\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec);
                fwrite(&log, strlen(log), 1, adc_file);

                if(signal_group.signal_config.pattern_sel[aging_pat_index])
                {
                    auto_log_time();
                    memset(&log,0,sizeof(log));  
                    snprintf(log, sizeof(log), "%04d:%02d:%02d:%03d  PATTERN[%d]_NAME = %s / PATTERN[%d]_RUN_TIME = %dsec\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec,num, pattern_name, num, signal_group.signal_config.display_time[num]/1000);
                    fwrite(&log, strlen(log), 1, adc_file);

                    /*auto_log_time();
                    memset(&log,0,sizeof(log));  
                    snprintf(log, sizeof(log), "%04d:%02d:%02d:%03d  AVDD_CUR = %.3fmA / ELVSS_CUR = %.3fmA\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)avdd_4byte_cur_ex/1000, (float)elvss_4byte_cur_ex/1000);
                    fwrite(&log, strlen(log), 1, adc_file);*/                  
                }

                auto_ocp_flag_on = 1;
                fclose(adc_file);
            }

            /*auto_log_time();
            memset(&log,0,sizeof(log));  
            snprintf(log, sizeof(log), "%04d:%02d:%02d:%03d  PATTERN[%d]_RUN\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec,num);
            fwrite(&log, strlen(log), 1, adc_file);*/                                              
        }
        else    printf("LOG_FILE_CREATE_NG\r\n"); 
    }
    else if(index == AUTO_NEXT_PATTERN)
    {
        adc_file = fopen(cmd, "a");

        if(adc_file != NULL)
        {
            auto_log_time();
            memset(&log,0,sizeof(log));  
            snprintf(log, sizeof(log), "%04d:%02d:%02d:%03d  PATTERN_CHANGE\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec);
            fwrite(&log, strlen(log), 1, adc_file); 

            auto_log_time();
            memset(&log,0,sizeof(log));  
            snprintf(log, sizeof(log), "%04d:%02d:%02d:%03d  PATTERN[%d]_NAME = %s / PATTERN[%d]_RUN_TIME = %dsec\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec,num, pattern_name, num, signal_group.signal_config.display_time[num]/1000);
            fwrite(&log, strlen(log), 1, adc_file);

            /*auto_log_time();
            memset(&log,0,sizeof(log));  
            snprintf(log, sizeof(log), "%04d:%02d:%02d:%03d  AVDD_CUR = %.3fmA / ELVSS_CUR = %.3fmA\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)avdd_4byte_cur_ex/1000, (float)elvss_4byte_cur_ex/1000);
            fwrite(&log, strlen(log), 1, adc_file);*/    

            /*auto_log_time();
            memset(&log,0,sizeof(log));  
            snprintf(log, sizeof(log), "%04d:%02d:%02d:%03d  PATTERN[%d]_RUN\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec,num);
            fwrite(&log, strlen(log), 1, adc_file);*/ 
            fclose(adc_file);
        }                                      
    } 
    else if(index == AUTO_RUN_OFF)
    {
        adc_file = fopen(cmd, "a");

        if(adc_file != NULL)
        {
            auto_log_time(); 
            memset(&log,0,sizeof(log));     
            snprintf(log, sizeof(log), "%04d:%02d:%02d:%03d  ****************AUTO RUN END****************\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec);
            fwrite(&log, strlen(log), 1, adc_file);   
            fclose(adc_file);      
        }         
    }
    else if(index == CYCLE_COMPLETE)
    {
        adc_file = fopen(cmd, "a");

        if(adc_file != NULL)
        {
            auto_log_time(); 
            memset(&log,0,sizeof(log));     
            snprintf(log, sizeof(log), "%04d:%02d:%02d:%03d  **************CYCLE[%d]_COMPLETE************\r\n\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec,num);
            fwrite(&log, strlen(log), 1, adc_file);     
            fclose(adc_file);      
        }       
    }  
    else if(index == OCP_EVENT)
    {
        adc_file = fopen(cmd, "a");

        if(adc_file != NULL)
        {
            auto_log_time(); 
            memset(&log,0,sizeof(log));     
            snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  OCP_EVENT[%x]\r\n\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec,ocp->EACH_CHANNEL_OCP_FLAG);
            fwrite(&log, strlen(log), 1, adc_file);   
            fclose(adc_file);     
        }          
    } 
    else if(index == INTERVAL_START)
    {
        adc_file = fopen(cmd, "a");

        if(adc_file != NULL)
        {
            auto_log_time(); 
            memset(&log,0,sizeof(log));     
            snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  INTERVAL_START = %dsec\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec,signal_group.signal_config.display_interval[num]/1000);
            fwrite(&log, strlen(log), 1, adc_file);   
            fclose(adc_file);
        }               
    } 
   else if(index == INTERVAL_END)
    {
        adc_file = fopen(cmd, "a");

        if(adc_file != NULL)
        {
            auto_log_time(); 
            memset(&log,0,sizeof(log));     
            snprintf(log, sizeof(log), "%04d:%02d:%02d:%03d  INTERVAL_END = %dsec\r\n\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec,signal_group.signal_config.display_interval[num]/1000);
            fwrite(&log, strlen(log), 1, adc_file);   
            fclose(adc_file);    
        }           
    }  
    else if(index == CUR_RECORD)
    {
        adc_file = fopen(cmd, "a");

        if(adc_file != NULL)
        {
            auto_log_time();
            memset(&log,0,sizeof(log));  
            snprintf(log, sizeof(log), "%04d:%02d:%02d:%03d  AVDD_CUR = %.3fmA / ELVSS_CUR = %.3fmA\r\n",time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)avdd_4byte_cur_ex/1000, (float)elvss_4byte_cur_ex/1000);
            fwrite(&log, strlen(log), 1, adc_file);    
            fclose(adc_file);    
        }             
    }  
    else if(index == COMPARE_VOL_CUR_ERROR)
    {
        adc_file = fopen(cmd, "a");

        if(adc_file != NULL)
        {
            auto_log_time();
            memset(&log,0,sizeof(log));  
            if(num == COMPARE_AVDD) snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / AVDD = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_AVDD]/1000,(float)compare_adc_sensing_value[SEN_AVDD]/1000,(float)compare_set.min_set[COMPARE_AVDD]/1000);
            
            else if(num == COMPARE_ELVSS)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / ELVSS = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_ELVSS]/1000,(float)compare_adc_sensing_value[SEN_ELVSS]/1000,(float)compare_set.min_set[COMPARE_ELVSS]/1000);
            
            else if(num == COMPARE_ELVDD)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / ELVDD = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_ELVDD]/1000,(float)compare_adc_sensing_value[SEN_ELVDD]/1000,(float)compare_set.min_set[COMPARE_ELVDD]/1000);            
           
            else if(num == COMPARE_AVDD8_S)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / AVDD8_S = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_AVDD8_S]/1000,(float)compare_adc_sensing_value[SEN_ADD8_S]/1000,(float)compare_set.min_set[COMPARE_AVDD8_S]/1000);  
            
            else if(num == COMPARE_AVDD8_G)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / AVDD8_G = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_AVDD8_G]/1000,(float)compare_adc_sensing_value[SEN_ADD8_G]/1000,(float)compare_set.min_set[COMPARE_AVDD8_G]/1000);  
            
            else if(num == COMPARE_VGH)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / VGH = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_VGH]/1000,(float)compare_adc_sensing_value[SEN_VGH]/1000,(float)compare_set.min_set[COMPARE_VGH]/1000);                                      
            
            else if(num == COMPARE_VGL)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / VGL = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_VGL]/1000,(float)compare_adc_sensing_value[SEN_VGL]/1000,(float)compare_set.min_set[COMPARE_VGL]/1000);  
            
            else if(num == COMPARE_VINT)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / VINT = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_VINT]/1000,(float)compare_adc_sensing_value[SEN_VINT]/1000,(float)compare_set.min_set[COMPARE_VINT]/1000);  
            
            else if(num == COMPARE_ASPARE1)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / ASPARE1 = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_ASPARE1]/1000,(float)compare_adc_sensing_value[SEN_APSPARE1]/1000,(float)compare_set.min_set[COMPARE_ASPARE1]/1000);  
            
            else if(num == COMPARE_ASPARE2)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / ASPARE2 = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_ASPARE2]/1000,(float)compare_adc_sensing_value[SEN_APSPARE2]/1000,(float)compare_set.min_set[COMPARE_ASPARE2]/1000);              
            
            else if(num == COMPARE_VDD11)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / VDD11 = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_VDD11]/1000,(float)compare_adc_sensing_value[SEN_VDD11]/1000,(float)compare_set.min_set[COMPARE_VDD11]/1000);              
            
            else if(num == COMPARE_VDD18)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / VDD18 = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_VDD18]/1000,(float)compare_adc_sensing_value[SEN_VDD18]/1000,(float)compare_set.min_set[COMPARE_VDD18]/1000);              
            
            else if(num == COMPARE_DSPARE1)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / DSPARE1 = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_DSPARE1]/1000,(float)compare_adc_sensing_value[SEN_DPSPARE1]/1000,(float)compare_set.min_set[COMPARE_DSPARE1]/1000);              
            
            else if(num == COMPARE_DSPARE2)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / DSAPRE2 = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_DSPARE2]/1000,(float)compare_adc_sensing_value[SEN_DPSAPRE2]/1000,(float)compare_set.min_set[COMPARE_DSPARE2]/1000); 
            
            else if(num == COMPARE_LDOELVDD)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / LDOELVDD = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_LDOELVDD]/1000,(float)compare_adc_sensing_value[SEN_LDO_ELVDD]/1000,(float)compare_set.min_set[COMPARE_LDOELVDD]/1000); 
            
            else if(num == COMPARE_LDOOSC)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / LDOOSC = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_LDOOSC]/1000,(float)compare_adc_sensing_value[SEN_LDO_OSC]/1000,(float)compare_set.min_set[COMPARE_LDOOSC]/1000); 
            
            else if(num == COMPARE_LDOVGH)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / LDOVGH = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_LDOVGH]/1000,(float)compare_adc_sensing_value[SEN_LDO_VGH]/1000,(float)compare_set.min_set[COMPARE_LDOVGH]/1000); 
            
            else if(num == COMPARE_LDOVGL)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / LDOVGL = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_LDOVGL]/1000,(float)compare_adc_sensing_value[SEN_LDO_VGL]/1000,(float)compare_set.min_set[COMPARE_LDOVGL]/1000); 
            
            else if(num == COMPARE_LDOVINT)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / LDOVINT = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_LDOVINT]/1000,(float)compare_adc_sensing_value[SEN_LDO_VINT]/1000,(float)compare_set.min_set[COMPARE_LDOVINT]/1000); 
            
            else if(num == COMPARE_VCIR)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / VCIR = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_VCIR]/1000,(float)compare_adc_sensing_value[SEN_VCIR]/1000,(float)compare_set.min_set[COMPARE_VCIR]/1000); 
            
            else if(num == COMPARE_VREF1)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / VREF1 = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_VREF1]/1000,(float)compare_adc_sensing_value[SEN_VREF1]/1000,(float)compare_set.min_set[COMPARE_VREF1]/1000);                                     
            
            else if(num == COMPARE_VREG1)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / VREG1 = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_VREG1]/1000,(float)compare_adc_sensing_value[SEN_VREG1]/1000,(float)compare_set.min_set[COMPARE_VREG1]/1000); 
            
            else if(num == COMPARE_VOTP50)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / VOTP50 = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_VOTP50]/1000,(float)compare_adc_sensing_value[SEN_VOTP50]/1000,(float)compare_set.min_set[COMPARE_VOTP50]/1000); 
            
            else if(num == COMPARE_LSPARE)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_VOL = %.3fV / LSPARE = %.3fV / MIN_VOL = %.3fV\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_LSPARE]/1000,(float)compare_adc_sensing_value[SEN_PM_SPARE1]/1000,(float)compare_set.min_set[COMPARE_LSPARE]/1000); 
            
            else if(num == COMPARE_AVDDCUR)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_CUR = %.3fmA / ADDCUR = %.3fmA / MIN_CUR = %.3fmA\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_AVDDCUR],(float)avdd_4byte_cur_ex/1000,(float)compare_set.min_set[COMPARE_AVDDCUR]); 
            
            else if(num == COMPARE_ELVSSCUR)   snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_CUR = %.3fmA / ELVSSCUR = %.3fmA / MIN_CUR = %.3fmA\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)compare_set.max_set[COMPARE_ELVSSCUR],(float)elvss_4byte_cur_ex/1000,(float)compare_set.min_set[COMPARE_ELVSSCUR]); 
            fwrite(&log, strlen(log), 1, adc_file);    
            fclose(adc_file);    
        }             
    }
    else if(index == COMPARE_TEMP_ERROR)
    {
        adc_file = fopen(cmd, "a");

        if(adc_file != NULL)
        {
            auto_log_time(); 
            memset(&log,0,sizeof(log)); 

            if(num == COMPARE_LT) snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_TEMP = %.1fºC / TEMP(LT) = %.1fºC / MIN_TEMP = %.1fºC\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)temp_set.max_set[COMPARE_LT],(float)temp_data[COMPARE_LT],(float)temp_set.min_set[COMPARE_LT]);
            
            else if(num == COMPARE_LB) snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_TEMP = %.1fºC / TEMP(LB) = %.1fºC / MIN_TEMP = %.1fºC\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)temp_set.max_set[COMPARE_LB],(float)temp_data[COMPARE_LB],(float)temp_set.min_set[COMPARE_LB]);   

            else if(num == COMPARE_RT) snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_TEMP = %.1fºC / TEMP(RT) = %.1fºC / MIN_TEMP = %.1fºC\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)temp_set.max_set[COMPARE_RT],(float)temp_data[COMPARE_RT],(float)temp_set.min_set[COMPARE_RT]);   
            
            else if(num == COMPARE_RB) snprintf(log, sizeof(log), "\r\n%04d:%02d:%02d:%03d  MAX_TEMP = %.1fºC / TEMP(RB) = %.1fºC / MIN_TEMP = %.1fºC\r\n",
            time_count.hour, time_count.min,time_count.sec,time_count.msec,(float)temp_set.max_set[COMPARE_RB],(float)temp_data[COMPARE_RB],(float)temp_set.min_set[COMPARE_RB]);    

            fwrite(&log, strlen(log), 1, adc_file);   
            fclose(adc_file);  
        }
    }                         
    //fclose(adc_file);
    system("sync");	      
}
#endif					//231027 Modify	

void display_task(char* line_1, char* line_2)   		//230929 Modify	
{
    char put_buf[128];
    char pat_name_buf[10];

    if(line_1 != NULL)
    {
        //len = strlen(line_1);
        if(strlen(line_1) > 0)
        {         
            memset(put_buf, 0, sizeof(put_buf));
            put_buf[0] = 0x02;
            put_buf[1] = '0';
            snprintf(put_buf+2,sizeof(put_buf), "%s",line_1);
            put_buf[strlen(put_buf)] = 0x03;
            printf("%s",put_buf);
        }
    }

    if(line_2 != NULL)
    {
        if(strlen(line_2) > 0)
        {
            memset(put_buf, 0, sizeof(put_buf));
            put_buf[0] = 0x02;
            put_buf[1] = '1';
            snprintf(put_buf+2,sizeof(put_buf), "%s",line_2);
            put_buf[strlen(put_buf)] = 0x03;
            printf("%s",put_buf);         
        }
    }
}


void button_led_off(void)
{
    char temp_str[64];
    int temp_str_tx_cnt;
    int i;

    //ON_OFF LED 동작 하는 곳
    memset(temp_str, 0, sizeof(temp_str));
    temp_str_tx_cnt = 0;

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

    for(i=0; ((i<temp_str_tx_cnt) && (i<sizeof(temp_str))); i++) putchar(temp_str[i]);
}

void button_led_on(void)
{
    char temp_str[64];
    int temp_str_tx_cnt;
    int i;

    //ON_OFF LED 동작 하는 곳
    memset(temp_str, 0, sizeof(temp_str));
    temp_str_tx_cnt = 0;

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

    for(i=0; ((i<temp_str_tx_cnt) && (i<sizeof(temp_str))); i++) putchar(temp_str[i]);
}

void model_name_write(char * name)
{
    FILE *adc_file;
    unsigned char model_name_write[MODEL_NAME_LEN] = {0,};    //230929 Modify
    adc_file = fopen(MODEL_NAME_FILE_PATH, "w+b");
    int model_name_size = 0;    //230929 Modify

    if(adc_file != NULL)
    {
        model_name_size = strlen(name); //230929 Modify

        if(model_name_size >= MODEL_NAME_LEN)   model_name_size = MODEL_NAME_LEN;   //230929 Modify 
        memset(&model_name_write,0,sizeof(model_name_write));     
        //snprintf(model_name, sizeof(model_name), "%s",name);
        //fwrite(&model_name, (strlen(model_name))-1, 1, adc_file);
        snprintf(model_name_write, model_name_size, "%s",name);   //230929 Modify
        fwrite(&model_name_write, model_name_size-1, 1, adc_file);    //230929 Modify
        fclose(adc_file);
        system("sync");	    
    }     
}

#ifdef RELIABILITY_ROOM	//231027 Modify
void aging_off_task(void)
{
    recipe_funtion_load("OFF");
    display_task(model_name,"Ready");       //230929 Modify
    pg_on_flag = PG_OFF;
    log_task(AUTO_RUN_OFF,(unsigned short)aging_pat_index);
    auto_log_time_start = 2;
    auto_log_time();
    button_led_off();
    //display_task(model_name,"Ready");    
}
#endif					//231027 Modify	

void rcb_ack(void)
{
    char temp_str[64];
    char temp_name[16];
    int temp_str_tx_cnt;
    int i;

    //LCD 첫번째 라인 글씨 변경하는 곳
    memset(temp_str, 0, sizeof(temp_str));
    memset(temp_name, 0, sizeof(temp_name));
    temp_str_tx_cnt = 0;
    temp_str[temp_str_tx_cnt++] = 0x02; //STX
    temp_str[temp_str_tx_cnt++] = '3'; //Mode
    sprintf(temp_name, "%.*s", 16, "ACK");
    strcat(temp_str, temp_name);
    temp_str_tx_cnt += strlen(temp_name);
    temp_str[temp_str_tx_cnt++] = 0x03;
    for(i=0; (i<temp_str_tx_cnt) && (i<(16+3)); i++) putchar(temp_str[i]);
}

void log_index_file_create(void)
{
    FILE *log_index_file;
    unsigned int log_count;

    if(access("/f0/log", F_OK)) system("mkdir /f0/log");
    usleep(100000);
    log_count = create_file();

    if(log_count == 0)
    {
        //if(system("ls /f0/log/_LAST_INDEX[0]_.txt"))
        if(access(LAST_INDEX_FILE_PATH,F_OK))	//230929 Modify
        {
            log_index_file = fopen(LAST_INDEX_FILE_PATH,"wb"); 
            usleep(1000);
           if(log_index_file != NULL)   fclose(log_index_file);  
           else system_error.log_index_file_create = ERROR_NG;                             
        }
    }
    else if(log_count == -1)    
    {
        system_error.log_index_file_create = ERROR_NG;  
    }  
}

#ifdef RELIABILITY_ROOM	//231027 Modify
void log_index_file_change(unsigned int index)
{
    FILE *fp = NULL;
    DIR *dir_ptr = NULL;
    struct dirent *file = NULL;
    int i = 0;
    int temp_base_number;
    char pre_index_name[50] = {0,};
    char index_name[50] = {0,};

    if((dir_ptr = opendir("/f0/log")) == NULL){
        if(nprf)    printf("directory find fail\r\n");
    }

    while((file = readdir(dir_ptr)) != NULL)
    {
        if(strstr(file->d_name, "INDEX") != NULL)
        { 
            break;
        }
        if(i > log_index_count+10)
        {           
            break;       
        }
        i++;        
    }
    sprintf(pre_index_name, "/f0/log/%s",file->d_name);
    sprintf(index_name, "/f0/log/_LAST_INDEX[%d]_.txt",index);    
    rename(pre_index_name, index_name); 
    closedir(dir_ptr);   
}

void delete_log_file(void)
{
    DIR *dir_ptr = NULL;
    struct dirent *file = NULL;
    FILE *log_file;
    int result = 0;
    char log_name[50] = {0,};
    if((dir_ptr = opendir("/f0/log")) == NULL){
        if(nprf)    printf("directory find fail\r\n");
    }
    else
    {
        while((file = readdir(dir_ptr)) != NULL)
        {
            if(strstr(file->d_name, "INDEX") != NULL)
            {
                sprintf(log_name, "/f0/log/%s",file->d_name);
                if(!remove(log_name))	
                {
                    log_file = fopen(LAST_INDEX_FILE_PATH,"wb"); 
                    if(log_file != NULL)    fclose(log_file);    
                }
                memset(log_name, 0, sizeof(log_name));                            			
            }
            //else if(strstr(file->d_name, "auto_run") != NULL)
            else
            {
                sprintf(log_name, "/f0/log/%s",file->d_name);
                result = remove(log_name);
                memset(log_name, 0, sizeof(log_name));  
            }	
        }
        closedir(dir_ptr);  
    }  
}

void interval_check(unsigned char index)
{
    static char interval_index_flag;
    switch(index)
    {
        case 0:
            interval_start_flag = 1;
            recipe_funtion_load("OFF");
            display_task(NULL,"Interval");       //230929 Modify
            aging_pat_interval_change_flag = 1;
            log_task(INTERVAL_START,(unsigned short)aging_pat_index);
        break;
        case 1:
            log_task(INTERVAL_END,(unsigned short)aging_pat_index);
            recipe_system_load();            
            aging_pat_index++;
            aging_pat_change_flag = 1;
        break;
        case 2:
        break;
        default:
        index = 0;
        break;
    }   
}
#endif					//231027 Modify	

bool temp_sen_task(void)
{
    unsigned char slave_address = i2c_rd_addr>>1;
    int read_byte = 0;
    int reg_address = 0;
    int i = 0;
    char read_buffer[10] = {0};
    float temp[4] = {0};
    char str[20]={0};
    unsigned short checksum=0;
    unsigned short tx_len = 0;

    bool ret = false;

    read_byte = 4;
    reg_address = LT_BPRD_1ST_ADDR;

    i2c_read(slave_address,reg_address,read_buffer,read_byte);

    temp[0] = (read_buffer[0]-15)*0.75-40;
    temp[1] = (read_buffer[1]-15)*0.75-40;
    temp[2] = (read_buffer[2]-15)*0.75-40;
    temp[3] = (read_buffer[3]-15)*0.75-40; 

    if((temperature_data.lt_min < temp[0]) && (temperature_data.lt_max > temp[0]))  ret = true;
    else 
    {
        ret = false;
    }
    if((temperature_data.lb_min < temp[1]) && (temperature_data.lb_max > temp[1]))  ret = true;
    else 
    {
        ret = false;        
    } 
    if((temperature_data.rt_min < temp[2]) && (temperature_data.rt_max > temp[2]))  ret = true;
    else 
    {
        ret = false;        
    }
    if((temperature_data.rb_min < temp[3]) && (temperature_data.rb_max > temp[3]))  ret = true;
    else 
    {
        ret = false;            
    }                 

}

void nvss_cur_offset(void)
{
    int a = 0;
    int i = 0;
    short data =500;
    a = ELVSS;
    unsigned char category_start_flag = 0xff;

    //pg_on_flag = CAL_RUN;		//230929 Modify	
    //usleep(2000000);		//230929 Modify	
    for(i = 0 ; i < 25 ; i++)
    {
        ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
        if(category_start_flag == DAC_ADC_AUTO_STOP)
        {
        	return;
        }        
        //pg_on_flag = CAL_RUN;		//230929 Modify	  
        //usleep(100000);		//230929 Modify	   
        signal_group.signal_config.dc_voltage[ELVSS] = - (data + (i*1000)); 
        Power_Supply_Voltage_load();
        SET_DAC_OUTPUT_VALUE(ELVSS);  
        gpio_enble->GPIO_DATA |= (0x00000001<<(a&0x3fff));  
        usleep(1000000);      

        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);     //231013 Modify	
        ADC_SELECT_DATA_READ_AVG(SEN_ELISS);                                    //231013 Modify	
        elvss_2byte_offset = elvss_cur_sensing_data;                            //231013 Modify	 
        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);      //231013 Modify	       

        if(elvss_2byte_offset > 0)   elvss_cur_offset_cal.n_offset[i] = -elvss_2byte_offset;
        else    elvss_cur_offset_cal.n_offset[i] = -(elvss_2byte_offset);    
        if(pprf)    printf("elvss_2byte_offset = %f / elvss_cur_offset_cal.n_offset[%d] =%f\r\n", elvss_2byte_offset, i, elvss_cur_offset_cal.n_offset[i]); 
        power_off();                      
    }    
}

void pvss_cur_offset(void)
{
    int a = 0;
    int i = 0;
    short data =500;
    a = ELVSS;
    unsigned char category_start_flag = 0xff;

    //pg_on_flag = CAL_RUN;		//230929 Modify	
    //usleep(2000000);		//230929 Modify	
    for(i = 0 ; i < 25 ; i++)
    {
        ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
        if(category_start_flag == DAC_ADC_AUTO_STOP)
        {
        	return;
        }
        //pg_on_flag = CAL_RUN; 		//230929 Modify	 
        //usleep(100000);  		//230929 Modify	       
        signal_group.signal_config.dc_voltage[ELVSS] = (data + (i*1000)); 
        Power_Supply_Voltage_load();
        SET_DAC_OUTPUT_VALUE(ELVSS);  
        gpio_enble->GPIO_DATA |= (0x00000001<<(a&0x3fff));  
        usleep(1000000);

        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);     //231013 Modify	
        ADC_SELECT_DATA_READ_AVG(SEN_ELISS);                                    //231013 Modify	
        elvss_2byte_offset = elvss_cur_sensing_data;                            //231013 Modify	 
        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);      //231013 Modify	  

        if(elvss_2byte_offset > 0)   elvss_cur_offset_cal.p_offset[i] = -elvss_2byte_offset;
        else    elvss_cur_offset_cal.p_offset[i] = -(elvss_2byte_offset);    
        if(pprf)    printf("elvss_2byte_offset = %f / elvss_cur_offset_cal.p_offset[%d] =%f\r\n", elvss_2byte_offset, i, elvss_cur_offset_cal.p_offset[i]); 
        power_off();                      
    }    
}

void vss_cur_offset_select(void)
{
    if((signal_group.signal_config.dc_voltage[ELVSS] <= 0) && (signal_group.signal_config.dc_voltage[ELVSS] > -1000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[0];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[0];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -1000) && (signal_group.signal_config.dc_voltage[ELVSS] > -2000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[1];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[1];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -2000) && (signal_group.signal_config.dc_voltage[ELVSS] > -3000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[2];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[2];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -3000) && (signal_group.signal_config.dc_voltage[ELVSS] > -4000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[3];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[3];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -4000) && (signal_group.signal_config.dc_voltage[ELVSS] > -5000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[4];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[4];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -5000) && (signal_group.signal_config.dc_voltage[ELVSS] > -6000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[5];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[5];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -6000) && (signal_group.signal_config.dc_voltage[ELVSS] > -7000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[6];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[6];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -7000) && (signal_group.signal_config.dc_voltage[ELVSS] > -8000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[7];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[7];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -8000) && (signal_group.signal_config.dc_voltage[ELVSS] > -9000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[8];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[8];	
    }
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -9000) && (signal_group.signal_config.dc_voltage[ELVSS] > -10000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[9];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[9];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -10000) && (signal_group.signal_config.dc_voltage[ELVSS] > -11000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[10];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[10];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -11000) && (signal_group.signal_config.dc_voltage[ELVSS] > -12000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[11];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[11];	
    }
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -12000) && (signal_group.signal_config.dc_voltage[ELVSS] > -13000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[12];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[12];	
    }
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -13000) && (signal_group.signal_config.dc_voltage[ELVSS] > -14000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[13];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[13];	
    }  
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -14000) && (signal_group.signal_config.dc_voltage[ELVSS] > -15000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[14];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[14];	
    }  
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -15000) && (signal_group.signal_config.dc_voltage[ELVSS] > -16000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[15];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[15];	
    }  
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -16000) && (signal_group.signal_config.dc_voltage[ELVSS] > -17000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[16];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[16];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -17000) && (signal_group.signal_config.dc_voltage[ELVSS] > -18000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[17];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[17];	
    }
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -18000) && (signal_group.signal_config.dc_voltage[ELVSS] > -19000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[18];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[18];	
    }
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -19000) && (signal_group.signal_config.dc_voltage[ELVSS] > -20000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[19];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[19];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -20000) && (signal_group.signal_config.dc_voltage[ELVSS] > -21000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[20];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[20];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -21000) && (signal_group.signal_config.dc_voltage[ELVSS] > -22000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[21];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[21];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -22000) && (signal_group.signal_config.dc_voltage[ELVSS] > -23000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[22];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[22];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -23000) && (signal_group.signal_config.dc_voltage[ELVSS] > -24000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[23];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[23];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] <= -24000) && (signal_group.signal_config.dc_voltage[ELVSS] >= -25000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.n_offset[24];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.n_offset[24];	
    }

    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 0) && (signal_group.signal_config.dc_voltage[ELVSS] < 1000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[0];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[0];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 1000) && (signal_group.signal_config.dc_voltage[ELVSS] < 2000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[1];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[1];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 2000) && (signal_group.signal_config.dc_voltage[ELVSS] < 3000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[2];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[2];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 3000) && (signal_group.signal_config.dc_voltage[ELVSS] < 4000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[3];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[3];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 4000) && (signal_group.signal_config.dc_voltage[ELVSS] < 5000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[4];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[4];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 5000) && (signal_group.signal_config.dc_voltage[ELVSS] < 6000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[5];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[5];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 6000) && (signal_group.signal_config.dc_voltage[ELVSS] < 7000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[6];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[6];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 7000) && (signal_group.signal_config.dc_voltage[ELVSS] < 8000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[7];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[7];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 8000) && (signal_group.signal_config.dc_voltage[ELVSS] < 9000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[8];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[8];	
    }
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 9000) && (signal_group.signal_config.dc_voltage[ELVSS] < 10000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[9];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[9];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 10000) && (signal_group.signal_config.dc_voltage[ELVSS] < 11000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[10];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[10];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 11000) && (signal_group.signal_config.dc_voltage[ELVSS] < 12000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[11];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[11];	
    }
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 12000) && (signal_group.signal_config.dc_voltage[ELVSS] < 13000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[12];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[12];	
    }
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 13000) && (signal_group.signal_config.dc_voltage[ELVSS] < 14000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[13];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[13];	
    }  
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 14000) && (signal_group.signal_config.dc_voltage[ELVSS] < 15000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[14];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[14];	
    }  
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 15000) && (signal_group.signal_config.dc_voltage[ELVSS] < 16000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[15];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[15];	
    }  
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 16000) && (signal_group.signal_config.dc_voltage[ELVSS] < 17000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[16];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[16];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 17000) && (signal_group.signal_config.dc_voltage[ELVSS] < 18000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[17];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[17];	
    }
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 18000) && (signal_group.signal_config.dc_voltage[ELVSS] < 19000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[18];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[18];	
    }
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 19000) && (signal_group.signal_config.dc_voltage[ELVSS] < 20000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[19];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[19];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 20000) && (signal_group.signal_config.dc_voltage[ELVSS] < 21000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[20];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[20];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 21000) && (signal_group.signal_config.dc_voltage[ELVSS] < 22000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[21];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[21];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 22000) && (signal_group.signal_config.dc_voltage[ELVSS] < 23000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[22];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[22];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 23000) && (signal_group.signal_config.dc_voltage[ELVSS] < 24000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[23];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[23];	
    } 
    else if((signal_group.signal_config.dc_voltage[ELVSS] >= 24000) && (signal_group.signal_config.dc_voltage[ELVSS] <= 25000))	
    {
        elvss_cur_cal.n_cur_1a_user_offset = elvss_cur_offset_cal.p_offset[24];	
        elvss_cur_cal.cur_1a_user_offset = elvss_cur_offset_cal.p_offset[24];	
    }                                                                                                                
}

void nvdd_cur_offset(void)
{
    int a = 0;
    int i = 0;
    short data =500;
    a = AVDD;
    unsigned char category_start_flag = 0xff;

    //pg_on_flag = CAL_RUN;		//230929 Modify	
    //usleep(2000000);		//230929 Modify	
    for(i = 0 ; i < 25 ; i++)
    {
        ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
        if(category_start_flag == DAC_ADC_AUTO_STOP)
        {
        	return;
        }        
        //pg_on_flag = CAL_RUN;		//230929 Modify	  
        //usleep(100000);		//230929 Modify	
        signal_group.signal_config.dc_voltage[AVDD] = - (data + (i*1000)); 
        Power_Supply_Voltage_load();
        SET_DAC_OUTPUT_VALUE(AVDD);  
        gpio_enble->GPIO_DATA |= (0x00000001<<(a&0x3fff));  
        usleep(1000000);

        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);     //231013 Modify	
        ADC_SELECT_DATA_READ_AVG(SEN_ELIDD);                                    //231013 Modify	
        avdd_2byte_offset = avdd_cur_sensing_data;                            //231013 Modify	 
        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);      //231013 Modify	 

        if(avdd_2byte_offset > 0)   avdd_cur_offset_cal.n_offset[i] = -avdd_2byte_offset;
        else    avdd_cur_offset_cal.n_offset[i] = -(avdd_2byte_offset);    
        if(pprf)    printf("avdd_2byte_offset = %f / avdd_cur_offset_cal.n_offset[%d] =%f\r\n", avdd_2byte_offset, i, avdd_cur_offset_cal.n_offset[i]); 
        power_off();                      
    }    
}

void pvdd_cur_offset(void)
{
    int a = 0;
    int i = 0;
    short data =500;
    a = AVDD;
    unsigned char category_start_flag = 0xff;

    //pg_on_flag = CAL_RUN;   		//230929 Modify	
    //usleep(2000000);    		//230929 Modify	
    for(i = 0 ; i < 25 ; i++)
    {
        ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
        if(category_start_flag == DAC_ADC_AUTO_STOP)
        {
        	return;
        }        
        //pg_on_flag = CAL_RUN;		//230929 Modify	  
        //usleep(100000);		//230929 Modify	
        signal_group.signal_config.dc_voltage[AVDD] = (data + (i*1000)); 
        Power_Supply_Voltage_load();
        SET_DAC_OUTPUT_VALUE(AVDD);  
        gpio_enble->GPIO_DATA |= (0x00000001<<(a&0x3fff));  
        usleep(1000000);

        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);     //231013 Modify	
        ADC_SELECT_DATA_READ_AVG(SEN_ELIDD);                                    //231013 Modify	
        avdd_2byte_offset = avdd_cur_sensing_data;                            //231013 Modify	 
        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);      //231013 Modify	 
                
        if(avdd_2byte_offset > 0)   avdd_cur_offset_cal.p_offset[i] = -avdd_2byte_offset;
        else    avdd_cur_offset_cal.p_offset[i] = -(avdd_2byte_offset);    
        if(pprf)    printf("avdd_2byte_offset = %f / avdd_cur_offset_cal.p_offset[%d] =%f\r\n", avdd_2byte_offset, i, avdd_cur_offset_cal.p_offset[i]); 
        power_off();                      
    }    
}

void vdd_cur_offset_select(void)
{
    if((signal_group.signal_config.dc_voltage[AVDD] <= 0) && (signal_group.signal_config.dc_voltage[AVDD] > -1000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[0];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[0];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -1000) && (signal_group.signal_config.dc_voltage[AVDD] > -2000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[1];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[1];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -2000) && (signal_group.signal_config.dc_voltage[AVDD] > -3000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[2];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[2];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -3000) && (signal_group.signal_config.dc_voltage[AVDD] > -4000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[3];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[3];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -4000) && (signal_group.signal_config.dc_voltage[AVDD] > -5000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[4];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[4];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -5000) && (signal_group.signal_config.dc_voltage[AVDD] > -6000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[5];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[5];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -6000) && (signal_group.signal_config.dc_voltage[AVDD] > -7000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[6];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[6];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -7000) && (signal_group.signal_config.dc_voltage[AVDD] > -8000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[7];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[7];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -8000) && (signal_group.signal_config.dc_voltage[AVDD] > -9000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[8];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[8];	
    }
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -9000) && (signal_group.signal_config.dc_voltage[AVDD] > -10000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[9];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[9];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -10000) && (signal_group.signal_config.dc_voltage[AVDD] > -11000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[10];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[10];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -11000) && (signal_group.signal_config.dc_voltage[AVDD] > -12000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[11];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[11];	
    }
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -12000) && (signal_group.signal_config.dc_voltage[AVDD] > -13000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[12];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[12];	
    }
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -13000) && (signal_group.signal_config.dc_voltage[AVDD] > -14000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[13];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[13];	
    }  
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -14000) && (signal_group.signal_config.dc_voltage[AVDD] > -15000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[14];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[14];	
    }  
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -15000) && (signal_group.signal_config.dc_voltage[AVDD] > -16000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[15];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[15];	
    }  
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -16000) && (signal_group.signal_config.dc_voltage[AVDD] > -17000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[16];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[16];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -17000) && (signal_group.signal_config.dc_voltage[AVDD] > -18000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[17];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[17];	
    }
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -18000) && (signal_group.signal_config.dc_voltage[AVDD] > -19000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[18];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[18];	
    }
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -19000) && (signal_group.signal_config.dc_voltage[AVDD] > -20000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[19];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[19];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -20000) && (signal_group.signal_config.dc_voltage[AVDD] > -21000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[20];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[20];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -21000) && (signal_group.signal_config.dc_voltage[AVDD] > -22000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[21];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[21];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -22000) && (signal_group.signal_config.dc_voltage[AVDD] > -23000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[22];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[22];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -23000) && (signal_group.signal_config.dc_voltage[AVDD] > -24000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[23];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[23];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] <= -24000) && (signal_group.signal_config.dc_voltage[AVDD] >= -25000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.n_offset[24];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.n_offset[24];	
    }

    else if((signal_group.signal_config.dc_voltage[AVDD] >= 0) && (signal_group.signal_config.dc_voltage[AVDD] < 1000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[0];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[0];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 1000) && (signal_group.signal_config.dc_voltage[AVDD] < 2000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[1];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[1];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 2000) && (signal_group.signal_config.dc_voltage[AVDD] < 3000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[2];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[2];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 3000) && (signal_group.signal_config.dc_voltage[AVDD] < 4000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[3];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[3];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 4000) && (signal_group.signal_config.dc_voltage[AVDD] < 5000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[4];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[4];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 5000) && (signal_group.signal_config.dc_voltage[AVDD] < 6000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[5];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[5];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 6000) && (signal_group.signal_config.dc_voltage[AVDD] < 7000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[6];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[6];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 7000) && (signal_group.signal_config.dc_voltage[AVDD] < 8000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[7];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[7];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 8000) && (signal_group.signal_config.dc_voltage[AVDD] < 9000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[8];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[8];	
    }
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 9000) && (signal_group.signal_config.dc_voltage[AVDD] < 10000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[9];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[9];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 10000) && (signal_group.signal_config.dc_voltage[AVDD] < 11000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[10];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[10];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 11000) && (signal_group.signal_config.dc_voltage[AVDD] < 12000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[11];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[11];	
    }
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 12000) && (signal_group.signal_config.dc_voltage[AVDD] < 13000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[12];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[12];	
    }
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 13000) && (signal_group.signal_config.dc_voltage[AVDD] < 14000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[13];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[13];	
    }  
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 14000) && (signal_group.signal_config.dc_voltage[AVDD] < 15000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[14];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[14];	
    }  
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 15000) && (signal_group.signal_config.dc_voltage[AVDD] < 16000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[15];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[15];	
    }  
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 16000) && (signal_group.signal_config.dc_voltage[AVDD] < 17000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[16];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[16];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 17000) && (signal_group.signal_config.dc_voltage[AVDD] < 18000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[17];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[17];	
    }
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 18000) && (signal_group.signal_config.dc_voltage[AVDD] < 19000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[18];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[18];	
    }
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 19000) && (signal_group.signal_config.dc_voltage[AVDD] < 20000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[19];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[19];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 20000) && (signal_group.signal_config.dc_voltage[AVDD] < 21000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[20];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[20];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 21000) && (signal_group.signal_config.dc_voltage[AVDD] < 22000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[21];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[21];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 22000) && (signal_group.signal_config.dc_voltage[AVDD] < 23000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[22];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[22];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 23000) && (signal_group.signal_config.dc_voltage[AVDD] < 24000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[23];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[23];	
    } 
    else if((signal_group.signal_config.dc_voltage[AVDD] >= 24000) && (signal_group.signal_config.dc_voltage[AVDD] <= 25000))	
    {
        avdd_cur_cal.n_cur_1a_user_offset = avdd_cur_offset_cal.p_offset[24];	
        avdd_cur_cal.cur_1a_user_offset = avdd_cur_offset_cal.p_offset[24];	
    }                                                                                                                
}

void read_command_task(void)
{
    unsigned char slave_address = i2c_rd_addr>>1;
    int reg_address = 0;
    int read_byte = 0;    
    int count_i = 0;                    
    char read_buffer[15] = {0};
    char compare_buffer[15] = {0};
    unsigned char com_cnt = 0;
    char i2c_read_communication_result = 0;
    char *reg_address_char;
    char *read_byte_char;
    char *compare_buffer_char;
    char null_flag = 0;

    reg_address_char = strtok(parm_copy, ",");
    if(strcmp(reg_address_char, "\n") == 0)
    {
        system_error.recipe = ERROR_NG; 
        printf("READ_COMMAND_ADDRESS_NG\r\n");
    }
    else
    {
        reg_address = strtol(reg_address_char,NULL,16); 
        read_byte_char = strtok(NULL, ",");
        if((read_byte_char == NULL) || (strcmp(read_byte_char,"\n") == 0))  
        {
            system_error.recipe = ERROR_NG;
            printf("READ_COMMAND_COUNT_NG\r\n"); 
        }   
        else
        {
            read_byte = strtol(read_byte_char,NULL,10);
            if(read_byte >= 10)  read_byte = 10;
            for(count_i = 0 ; count_i < read_byte ; count_i++)  
            {
                compare_buffer_char =   strtok(NULL, ","); 
                if((compare_buffer_char == NULL) || (strcmp(read_byte_char,"\n") == 0))
                {
                    system_error.recipe = ERROR_NG;  
                    null_flag = 1; 
                    printf("READ_COMMAND_DATA_NG\r\n");  
                    break;
                }
                else    compare_buffer[count_i] = strtol(compare_buffer_char,NULL,16); 
            }
            if(!null_flag)
            {
                i2c_read_communication_result = i2c_read(slave_address,reg_address,read_buffer,read_byte);

                if(i2c_read_communication_result == 0)
                {
                    if(cprf)    printf("slave_address = %x / reg_address = %x / com_cnt = %d\r\n", slave_address, reg_address, read_byte);

                    for(count_i = 0 ; count_i < read_byte ; count_i++)
                    {                               
                        if(cprf)    printf("Before_ compare_buffer[%d] = %x : read_buffer[%d] = %x\r\n", count_i,compare_buffer[count_i],count_i,read_buffer[count_i]);

                        if(compare_buffer[count_i] != read_buffer[count_i])
                        {
                            error_data.i2c_read = ERROR_NG;
                            break;
                        }
                    }
                }
                else    error_data.i2c_communication = ERROR_NG;   
            }                                                                   
        }          
    }     
    null_flag = 0; 
    reg_address_char=NULL;
    read_byte_char=NULL;
    compare_buffer_char=NULL;       
}

void send_command_task(void)
{
    unsigned char slave_address = i2c_wr_addr>>1;
    int reg_address = 0;
    int write_byte = 0;    
    int count_i = 0;                    
    char write_buffer[32] = {0};
    unsigned char com_cnt = 0;
    char i2c_send_communication_result = 0;    

    char *reg_address_char;
    char *send_byte_char;
    char *data_buffer_char;
    char null_flag = 0;      

    reg_address_char = strtok(parm_copy, ",");  

    if(strcmp(reg_address_char, "\n") == 0)
    {
        system_error.recipe = ERROR_NG; 
        printf("SEND_COMMAND_ADDRESS_NG\r\n");
        power_off();
    } 
    else
    {
        reg_address = strtol(reg_address_char,NULL,16); 
        send_byte_char = strtok(NULL, ",");
        if((send_byte_char == NULL) || (strcmp(send_byte_char,"\n") == 0))  
        {
            system_error.recipe = ERROR_NG;
            printf("SEND_COMMAND_COUNT_NG\r\n"); 
            power_off();
        }
        else
        {
            write_byte = strtol(send_byte_char,NULL,10);
            if(write_byte >= 10)  write_byte = 10; 
            for(count_i = 0 ; count_i < write_byte ; count_i++)  
            {
                data_buffer_char =   strtok(NULL, ","); 
                if((data_buffer_char == NULL) || (strcmp(data_buffer_char,"\n") == 0))
                {
                    system_error.recipe = ERROR_NG;  
                    null_flag = 1; 
                    printf("SNED_COMMAND_DATA_NG\r\n");  
                    power_off();
                    break;
                }
                else    write_buffer[count_i] = strtol(data_buffer_char,NULL,16); 
            } 
            if(!null_flag)
            {
                i2c_send_communication_result = i2c_write(slave_address,reg_address,write_buffer,write_byte);
                if(i2c_send_communication_result == 0);
                else    error_data.i2c_communication = ERROR_NG;
            }                                  
        }        
    } 
    reg_address_char=NULL;
    send_byte_char=NULL;
    data_buffer_char=NULL;      
}

void elvss_cur_command_task(void)
{
    int elvsscur_max = 0;
    int elvsscur_min = 0;
    int elvsscur_real = 0;   		//230929 Modify	

    char *elvsscur_max_char;
    char *elvsscur_min_char; 

    elvsscur_max_char = strtok(parm_copy, ",");  

    if(strcmp(elvsscur_max_char, "\n") == 0)
    {
        system_error.recipe = ERROR_NG; 
        printf("ELVSS_CUR_COMMAND_MAXDATA_NG\r\n");
    }
    else
    {
        elvsscur_max = strtol(elvsscur_max_char,NULL,10); 
        elvsscur_min_char = strtok(NULL, ",");
        if((elvsscur_min_char == NULL) || (strcmp(elvsscur_min_char,"\n") == 0))  
        {
            system_error.recipe = ERROR_NG;
            printf("ELVSS_CUR_COMMAND_MINDATA_NG\r\n"); 
        }
        else
        {
            elvsscur_min = strtol(elvsscur_min_char,NULL,10);    		//230929 Modify	
            ADC_SELECT_DATA_READ_AVG(SEN_ELISS);     		            //230929 Modify	
            elvsscur_real = fabs(elvss_cur_sensing_data*10);              
            if(cprf)    printf("elvsscur_max = %d / elvsscur_min = %d / elvsscur_real = %d\r\n", elvsscur_max*10,elvsscur_min*10,elvsscur_real);     		//230929 Modify	
            if((elvsscur_max*10 >= elvsscur_real) && (elvsscur_min*10 <= elvsscur_real))  ;  		//230929 Modify	    
            else    error_data.elvss_cur = ERROR_NG;               
        }        
    } 
    elvsscur_max_char=NULL;
    elvsscur_min_char=NULL;              
}

void avdd_cur_command_task(void)    
{
    int avddcur_max = 0;
    int avddcur_min = 0;
    int avddcur_real = 0;    		//230929 Modify	 

    char *avddcur_max_char;
    char *avddcur_min_char; 

    avddcur_max_char = strtok(parm_copy, ",");  

    if(strcmp(avddcur_max_char, "\n") == 0)
    {
        system_error.recipe = ERROR_NG; 
        printf("AVDD_CUR_COMMAND_MAXDATA_NG\r\n");
    }
    else
    {
        avddcur_max = strtol(avddcur_max_char,NULL,10); 
        avddcur_min_char = strtok(NULL, ",");
        if((avddcur_min_char == NULL) || (strcmp(avddcur_min_char,"\n") == 0))  
        {
            system_error.recipe = ERROR_NG;
            printf("AVDD_CUR_COMMAND_MINDATA_NG\r\n"); 
        }
        else
        {
            avddcur_min = strtol(avddcur_min_char,NULL,10);
            ADC_SELECT_DATA_READ_AVG(SEN_ELIDD);     		        //230929 Modify	
            avddcur_real = fabs(avdd_cur_sensing_data*10);   		//230929 Modify	             
             if(cprf)    printf("avddcur_max = %d / avddcur_min = %d / avddcur_real = %d\r\n", avddcur_max*10,avddcur_min*10,avddcur_real);  		//230929 Modify	
            if((avddcur_max*10 >= avddcur_real) && (avddcur_min*10 <= avddcur_real))  ;  		//230929 Modify	    
            else    error_data.avdd_cur = ERROR_NG;               
        }        
    }          
}

void recipe_download_task(void) 		//230929 Modify	
{
    FILE *temporary_file = NULL;
    FILE *recipe_file = NULL;	
    char file_data;
    int i = 0;

    //temporary_file = fopen(TEMPORARY_RECIPE_FILE_PATH, "rb");
    temporary_file = fopen(TEMPORARY_RECIPE_FILE_PATH, "r");
    if (temporary_file == NULL) 
    {
        printf("TEMPORARY FILE OPEN NOT FOUND\r\n");
        system_error.recipe_copy = ERROR_NG;
    }
    else
    {
        //recipe_file = fopen(RECIPE_FILE_PATH, "wb");
        recipe_file = fopen(RECIPE_FILE_PATH, "w");
        if(recipe_file == NULL)
        {
            printf("RECIPE FILE OPEN  NOT FOUND\r\n");
            system_error.recipe_copy = ERROR_NG;
            fclose(temporary_file);
        }
        else
        {
            while (1)
            {
                fread(&file_data, sizeof(char), 1, temporary_file);
                if(feof(temporary_file))
                {
                    break;
                }
                fwrite(&file_data, sizeof(char), 1, recipe_file);
                i++;
                if(i > 5000000) 
                {
                    printf("RECIPE_DOWNLOAD_COPY_ERROR\r\n");
                    system_error.recipe_copy = ERROR_NG;
                    break;
                }				
            }
            fclose(temporary_file);
            fclose(recipe_file);    
            if(system_error.recipe_copy != ERROR_NG)    
            {
                model_recipe_read();
                model_name_check();
                if(system_error.model_download_file_open != ERROR_NG)    recipe.download_mode = 0;            
                if(nprf)	printf("Recipe_download_ok\r\n");
            }	            
        }							
    }						
}

unsigned char ex_voltage_sensing_task(unsigned char ch) 	
{
    unsigned char ch_index = 0;
    unsigned char other_ch_index = 0;
    unsigned char result = 0;
    int i = 0;

    ch_index = ex_voltage__sensing_select_task(ch);  
    ADC_SELECT_DATA_READ_AVG(ch_index);       
   // ADC_AUTO_DATA_READ();
    if(cprf)    printf("adc_sensing_value[%d] = %d\r\n", ch_index, adc_sensing_value[ch_index]);
    if((adc_sensing_value[ch_index] >950)  &   (adc_sensing_value[ch_index] < 1050))  ;
    else    
    {
        result = 1;
    }

    for(i = 0 ; i < 18 ; i++)
    {
        if(cprf)    printf("other_adc_sensing_value[%d] = %d\r\n", other_ch_index, adc_sensing_value[other_ch_index]);
        other_ch_index = ex_voltage__sensing_select_task(i); 
        ADC_SELECT_DATA_READ_AVG(other_ch_index);   
        if(ch_index != other_ch_index)
        {
            if((adc_sensing_value[other_ch_index] >-30)  &   (adc_sensing_value[other_ch_index] < 30))  ;
            else    
            {
                result = 1;
            }   
        }
    }
    return result;          
}

unsigned char ex_voltage__sensing_select_task(unsigned char ch) 	
{
    unsigned char index_result = 0; 

    if(ch == EX_LDO_ELVDD)  index_result = SEN_LDO_ELVDD;
    else if(ch == EX_LDO_OSC)   index_result = SEN_LDO_OSC;
    else if(ch == EX_LDO_VGH)   index_result = SEN_LDO_VGH; 
    else if(ch == EX_LDO_VGL)   index_result = SEN_LDO_VGL; 
    else if(ch == EX_LDO_VINT)   index_result = SEN_LDO_VINT; 
    else if(ch == EX_VCIR)   index_result = SEN_VCIR; 
    else if(ch == EX_VREF1)   index_result = SEN_VREF1; 
    else if(ch == EX_VREG1)   index_result = SEN_VREG1; 
    else if(ch == EX_VOTP50)   index_result = SEN_VOTP50; 
    else if(ch == EX_PM_SPARE)   index_result = SEN_PM_SPARE1; 
    else if(ch == EX_MON1)   index_result = SEN_MON1;  
    else if(ch == EX_MON2)   index_result = SEN_MON2; 
    else if(ch == EX_MON3)   index_result = SEN_MON3; 
    else if(ch == EX_MON4)   index_result = SEN_MON4; 
    else if(ch == EX_MON5)   index_result = SEN_MON5; 
    else if(ch == EX_MON6)   index_result = SEN_MON6;  
    else if(ch == EX_LM_SPARE1)   index_result = SEN_LM_SPARE1;   
    else if(ch == EX_LM_SPARE2)   index_result = SEN_LM_SPARE2;      

    return index_result;
}

void ex_vol_sensing_task(void) 	
{
    //int adc_sensing_ch = 0;
    extern unsigned char adc_sensing_ch;
    unsigned char data[100] = {0,};
    //unsigned char ex_vol_sensing_task_result[100] = {0, };
    //unsigned int ex_vol_sensing_task_result = 0; 
    int value = 1800;

    for(adc_sensing_ch = 0 ; adc_sensing_ch < 18 ; adc_sensing_ch++)
    {
        if(adc_sensing_ch	==	14)	
        {
            logic_gpio_ctl("RSTB",1);	
        }
        else if(adc_sensing_ch	==	15)	
        {
            logic_gpio_ctl("TM",1);	
        }
        else if(adc_sensing_ch	==	16)	
        {
            logic_gpio_ctl("BISTEN",1);
        }
        else if(adc_sensing_ch	==	17)	
        {
            logic_gpio_ctl("LPSPARE1",1);
        }
        else
        {
            memset(&signal_group.signal_config.dc_voltage[0], 0, sizeof(signal_group.signal_config.dc_voltage[0])); 
            signal_group.signal_config.dc_voltage[adc_sensing_ch] = value;	
            Power_Supply_Voltage_load();
            SET_DAC_OUTPUT_VALUE(adc_sensing_ch);
            if(adc_sensing_ch ==ELVDD)	gpiops_set_value(RELAY_ELVDD,RELAY_ON);
            else if(adc_sensing_ch ==ADD8_S)	gpiops_set_value(RELAY_VDD8_S,RELAY_ON);
            else if(adc_sensing_ch ==ADD8_G)	gpiops_set_value(RELAY_VDD8_G,RELAY_ON);
            else if(adc_sensing_ch ==VGH)	gpiops_set_value(RELAY_VGH,RELAY_ON);
            else if(adc_sensing_ch ==VGL)	gpiops_set_value(RELAY_VGL,RELAY_ON);	
            else if(adc_sensing_ch ==VINIT)	gpiops_set_value(RELAY_VINIT,RELAY_ON);				
            gpio_enble->GPIO_DATA |= (0x00000001<<(adc_sensing_ch&0x3fff));
        }
        ex_vol_sensing_task_result[adc_sensing_ch] = ex_vol_sensing_task_2();
        printf("ex_vol_sensing_task_result[%d] = %x\r\n",adc_sensing_ch,ex_vol_sensing_task_result[adc_sensing_ch]);
        data[0] = EX_ADC_SEN;	
        data[1] = adc_sensing_ch;	
        ex_port_send_task(data,EX_ADC_SEN_LEN);		
        ex_port_serial_task();  
    }  
}

unsigned int ex_vol_sensing_task_2(void) 	
{
    unsigned char ch_index = 0; 
    unsigned int ex_vol_sensing_task_2_result = 0; 
    int i = 0;
    
    for(i = 0 ; i < 18 ; i++)
    {
        ch_index = ex_voltage__sensing_select_task(i); 
        ADC_SELECT_DATA_READ_AVG(ch_index);  
        if((adc_sensing_value[ch_index] >-30)  &   (adc_sensing_value[ch_index] < 30))  ;
        else    
        {
            ex_vol_sensing_task_2_result |= 1<<i;
        }         
    }
    return ex_vol_sensing_task_2_result;   
}

unsigned int ads_rm_init_data_compare_task(int cnt, unsigned int data) 	
{
    unsigned char rm_init_data_result = 0;
     
    if(cnt != 0)
    {      
        if(data == rm_init_data[cnt])   ;
        else    rm_init_data_result = 1;
    }
    return  rm_init_data_result;           
}

void ads_rm_init_data_set_task(void) 	
{
    memset(&rm_init_data, 0, sizeof(rm_init_data));
    rm_init_data[STATUS_ADDR] = STATUS_VALUE; 
    rm_init_data[INPMUX_ADDR] = INPMUX_VALUE; 
    rm_init_data[PGA_ADDR] = PGA_VALUE; 
    rm_init_data[DATARATE_ADDR] = DATARATE_VALUE; 
    rm_init_data[REF_ADDR] = REF_VALUE; 
    rm_init_data[IDACMAG_ADDR] = IDACMAG_VALUE; 
    rm_init_data[IDACMUX_ADDR] = IDACMUX_VALUE; 
    rm_init_data[VBIAS_ADDR] = VBIAS_VALUE; 
    rm_init_data[SYS_ADDR] = SYS_VALUE; 
    rm_init_data[OFCAL0_ADDR] = OFCAL0_VALUE; 
    rm_init_data[OFCAL1_ADDR] = OFCAL1_VALUE; 
    rm_init_data[OFCAL2_ADDR] = OFCAL2_VALUE; 
    rm_init_data[FSCAL0_ADDR] = FSCAL0_VALUE; 
    rm_init_data[FSCAL1_ADDR] = FSCAL1_VALUE; 
    rm_init_data[FSCAL2_ADDR] = FSCAL2_VALUE; 
    rm_init_data[GPIODAT_ADDR] = GPIODAT_AVALUE; 
    rm_init_data[GPIOCON_ADDR] = GPIOCON_VALUE; 
}

#ifdef RELIABILITY_ROOM	//231027 Modify
void file_count(unsigned short cnt)  //231027 Modify  
{
    memset(&file_cnt_string, 0, sizeof(file_cnt_string));
    snprintf(file_cnt_string,sizeof(file_cnt_string), "%d/%d",cnt,log_index_count);
}

void compare_value_setting_task(int code, char *data)  //231027 Modify 
{
    char *max_char;
    char *min_char; 
 
    max_char = strtok(data, ",");  
    if(strcmp(max_char, "\n") == 0)
    {
        printf("COMPARE_MAX_DATA_NG\r\n");
    }
    else
    {
        compare_set.max_set[code] = strtol(max_char,NULL,10); 
        min_char = strtok(NULL, ",");
        if((min_char == NULL) || (strcmp(min_char,"\n") == 0))  
        {
            printf("COMPARE_MIN_DATA_NG\r\n");
        } 
        else
        {
            compare_set.min_set[code] = strtol(min_char,NULL,10);    
        }       
    }
    if(cprf)    printf("compare_set.max_set[%d] = %d / compare_set.min_set[%d] = %d\r\n", code, compare_set.max_set[code], code,compare_set.min_set[code]);
}

unsigned char compare_vol_cur_task(void)  //231027 Modify 
{
    int i = 0 ;
    unsigned char sen_index = 0;
    unsigned char compare_result = 0;

    for(i = 0 ; i < COMPARE_SET_COUNT ; i++)
    {
        if((compare_set.max_set[i] != 0) & (compare_set.min_set[i] != 0))
        {
            if(i == COMPARE_AVDDCUR)
            {
                if((avdd_2byte_cur_ex > compare_set.min_set[i]*10)  &   (avdd_2byte_cur_ex < compare_set.max_set[i]*10))  ; 
                else
                {
                    compare_result = 1;
                    log_task(COMPARE_VOL_CUR_ERROR,(unsigned short)i);
                    display_task(NULL,"AVDD_CUR_ERROR");
                    power_off(); 
                    button_led_off();  
                    snprintf(aging_result_string,sizeof(aging_result_string),"AGING_FAILED");  
                    if(cprf)    
                    {
                        printf("compare_set.max_set[%d] = %d\r\n",i, compare_set.max_set[i]*10);
                        printf("compare_set.min_set[%d] = %d\r\n",i, compare_set.min_set[i]*10);
                        printf("avdd_2byte_cur_ex = %d\r\n",avdd_2byte_cur_ex);
                    }
                }               
            }
            else if(i == COMPARE_ELVSSCUR)
            {
                if((elvss_2byte_cur_ex > compare_set.min_set[i]*10)  &   (elvss_2byte_cur_ex < compare_set.max_set[i]*10))  ; 
                else
                {
                    compare_result = 1;
                    log_task(COMPARE_VOL_CUR_ERROR,(unsigned short)i);
                    display_task(NULL,"ELVSS_CUR_ERROR");
                    power_off(); 
                    button_led_off();  
                    snprintf(aging_result_string,sizeof(aging_result_string),"AGING_FAILED");  
                    if(cprf)
                    {
                        printf("compare_set.max_set[%d] = %d\r\n",i, compare_set.max_set[i]*10);
                        printf("compare_set.min_set[%d] = %d\r\n",i, compare_set.min_set[i]*10);
                        printf("elvss_2byte_cur_ex = %d\r\n",elvss_2byte_cur_ex);
                    }
                }  
            }
            else
            {                
                sen_index = compare_sensing_select_task(i);              
                if((compare_adc_sensing_value[sen_index] > compare_set.min_set[i])  &   (compare_adc_sensing_value[sen_index] < compare_set.max_set[i]))  ; 
                else
                {
                    compare_result = 1;
                    log_task(COMPARE_VOL_CUR_ERROR,(unsigned short)i);
                    display_task(NULL,"VOLTAGE_ERROR");
                    power_off(); 
                    button_led_off();   
                    snprintf(aging_result_string,sizeof(aging_result_string),"AGING_FAILED"); 
                    if(cprf)
                    {
                        printf("compare_set.max_set[%d] = %d\r\n",i, compare_set.max_set[i]);
                        printf("compare_set.min_set[%d] = %d\r\n",i, compare_set.min_set[i]);
                        printf("adc_sensing_value[%d] = %d\r\n",sen_index,compare_adc_sensing_value[sen_index]);
                    }
                }
            }
        }
    }
    return compare_result;
}

unsigned char compare_sensing_select_task(unsigned char ch) 	
{
    unsigned char index_result = 0; 

    if(ch == COMPARE_AVDD)  index_result = SEN_AVDD;
    else if(ch == COMPARE_ELVSS)  index_result = SEN_ELVSS;
    else if(ch == COMPARE_ELVDD)  index_result = SEN_ELVDD ;
    else if(ch == COMPARE_AVDD8_S)  index_result = SEN_ADD8_S;
    else if(ch == COMPARE_AVDD8_G)  index_result = SEN_ADD8_G;
    else if(ch == COMPARE_VGH)  index_result = SEN_VGH;
    else if(ch == COMPARE_VGL)  index_result = SEN_VGL;
    else if(ch == COMPARE_VINT)  index_result = SEN_VINT;
    else if(ch == COMPARE_ASPARE1)  index_result = SEN_APSPARE1;
    else if(ch == COMPARE_ASPARE2)  index_result = SEN_APSPARE2;
    else if(ch == COMPARE_VDD11)  index_result = SEN_VDD11;
    else if(ch == COMPARE_VDD18)  index_result = SEN_VDD18;
    else if(ch == COMPARE_DSPARE1)  index_result = SEN_DPSPARE1;
    else if(ch == COMPARE_DSPARE2)  index_result = SEN_DPSAPRE2;
    else if(ch == COMPARE_LDOELVDD)  index_result = SEN_LDO_ELVDD;
    else if(ch == COMPARE_LDOOSC)   index_result = SEN_LDO_OSC;
    else if(ch == COMPARE_LDOVGH)   index_result = SEN_LDO_VGH; 
    else if(ch == COMPARE_LDOVGL)   index_result = SEN_LDO_VGL; 
    else if(ch == COMPARE_LDOVINT)   index_result = SEN_LDO_VINT; 
    else if(ch == COMPARE_VCIR)   index_result = SEN_VCIR; 
    else if(ch == COMPARE_VREF1)   index_result = SEN_VREF1; 
    else if(ch == COMPARE_VREG1)   index_result = SEN_VREG1; 
    else if(ch == COMPARE_VOTP50)   index_result = SEN_VOTP50; 
    else if(ch == COMPARE_LSPARE)   index_result = SEN_PM_SPARE1;  

    return index_result;
}

void temp_value_setting_task(int code, char *data)  //231027 Modify 
{
    char *max_char;
    char *min_char; 
 
    max_char = strtok(data, ",");  
    if(strcmp(max_char, "\n") == 0)
    {
        printf("TEMP_MAX_DATA_NG\r\n");
    }
    else
    {
        temp_set.max_set[code] = strtol(max_char,NULL,10); 
        min_char = strtok(NULL, ",");
        if((min_char == NULL) || (strcmp(min_char,"\n") == 0))  
        {
            printf("TEMP_MIN_DATA_NG\r\n");
        } 
        else
        {
            temp_set.min_set[code] = strtol(min_char,NULL,10);    
        }       
    }
    if(cprf)    printf("temp_set.max_set[%d] = %d / temp_set.min_set[%d] = %d\r\n", code, temp_set.max_set[code], code,temp_set.min_set[code]);
}

unsigned char compare_temp_task(void)  //231027 Modify 
{
    unsigned char slave_address = i2c_rd_addr>>1;
    int read_byte = 0;
    int reg_address = 0;
    int i = 0;
    char read_buffer[10] = {0};
    float temp[4] = {0};
    char str[20]={0};
    unsigned short checksum=0;
    unsigned short tx_len = 0;

    unsigned char sen_index = 0;
    unsigned char temp_result = 0;

    memset(&temp_data, 0, sizeof(temp_data));
    read_byte = 4;
    reg_address = LT_BPRD_1ST_ADDR;

    i2c_read(slave_address,reg_address,read_buffer,read_byte);

    temp_data[COMPARE_LT] = (read_buffer[0]-15)*0.75-40;
    temp_data[COMPARE_LB] = (read_buffer[1]-15)*0.75-40;
    temp_data[COMPARE_RT] = (read_buffer[2]-15)*0.75-40;
    temp_data[COMPARE_RB] = (read_buffer[3]-15)*0.75-40; 

    for(i = 0 ; i < TEMP_SET_COUNT ; i++)
    {
        if((temp_set.max_set[i] != 0) & (temp_set.min_set[i] != 0))
        {                         
            if((temp_data[i] > (float)temp_set.min_set[i])  &   (temp_data[i] < (float)temp_set.max_set[i]))  ; 
            else
            {
                temp_result = 1;
                log_task(COMPARE_TEMP_ERROR,(unsigned short)i);
                if(i == COMPARE_LT)  display_task(NULL,"TEMP(LT)_ERROR");
                else if(i == COMPARE_LB)    display_task(NULL,"TEMP(LB)_ERROR");
                else if(i == COMPARE_RT)    display_task(NULL,"TEMP(RT)_ERROR");  
                else if(i == COMPARE_RB)    display_task(NULL,"TEMP(RB)_ERROR");                  
                power_off(); 
                button_led_off();   
                snprintf(aging_result_string,sizeof(aging_result_string),"AGING_FAILED"); 
                if(cprf)
                {
                    printf("temp_set.max_set[%d] = %d\r\n",i, temp_set.max_set[i]);
                    printf("temp_set.min_set[%d] = %d\r\n",i, temp_set.min_set[i]);
                    if(i == COMPARE_LT)  printf("TEMP(LT) = %d\r\n",temp_data[i]);
                    else if(i == COMPARE_LB)    printf("TEMP(LB) = %d\r\n",temp_data[i]); 
                    else if(i == COMPARE_RT)    printf("TEMP(RT) = %d\r\n",temp_data[i]);    
                    else if(i == COMPARE_RB)    printf("TEMP(RB) = %d\r\n",temp_data[i]);       
                }
            }
        }
    }
    return temp_result;
}
#endif  //231027 Modify	

/*unsigned char rm_error_cnt_write(unsigned char index)
{
    FILE *rm_error_cnt_file;
    unsigned char rm_error_cnt_write[MODEL_NAME_LEN] = {0,};    //230929 Modify
    unsigned char rm_error_cnt_result = 0;

    if(index == RM_ERROR_INDEX)
    {
        rm_error_cnt_file = fopen(RM_INIT_ERROR_CNT_FILE_PATH, "r");

        if(rm_error_cnt_file != NULL)
        {
            fread(&rm_error_cnt_write, sizeof(rm_error_cnt_write), 1, rm_error_cnt_file);  
            fclose(rm_error_cnt_file);  
            usleep(1000);
            if(strncasecmp(rm_error_cnt_write,"0",1)==0)    
            {
                printf("RM_INIT_ERROR_CNT_COUNT = 1\r\n");
                rm_error_cnt_file = fopen(RM_INIT_ERROR_CNT_FILE_PATH, "w");
                fwrite("1", 1, 1, rm_error_cnt_file); 
                usleep(50000);
                fclose(rm_error_cnt_file);
                system("sync");	
                system("reboot");
                usleep(100000);	
            }
            else if(strncasecmp(rm_error_cnt_write,"1",1)==0)    
            {
                printf("RM_INIT_ERROR_CNT_COUNT = 2\r\n");
                rm_error_cnt_file = fopen(RM_INIT_ERROR_CNT_FILE_PATH, "w");
                fwrite("2", 1, 1, rm_error_cnt_file);
                usleep(50000);
                fclose(rm_error_cnt_file);  
                system("sync");	
                system("reboot");	
                usleep(100000);
            }
            else if(strncasecmp(rm_error_cnt_write,"2",1)==0)    
            {
                printf("RM_INIT_ERROR_CNT_COUNT = 3\r\n");
                rm_error_cnt_file = fopen(RM_INIT_ERROR_CNT_FILE_PATH, "w");
                fwrite("3", 1, 1, rm_error_cnt_file);
                usleep(50000);
                fclose(rm_error_cnt_file);  
                system("sync");	
                system("reboot");	
                usleep(100000);
            }        
            else if(strncasecmp(rm_error_cnt_write,"3",1)==0)    
            {
                printf("RM_INIT_ERROR_CNT_COUNT = 4\r\n");
                rm_error_cnt_file = fopen(RM_INIT_ERROR_CNT_FILE_PATH, "w");
                fwrite("0", 1, 1, rm_error_cnt_file); 
                usleep(50000);
                fclose(rm_error_cnt_file); 
                system("sync");	          
                rm_error_cnt_result = 1; 
                usleep(100000);
            } 
        }
    }  
    else if(index == RM_ERROR_CLEAR_INDEX)  
    {
       rm_error_cnt_file = fopen(RM_INIT_ERROR_CNT_FILE_PATH,"w");   
	   usleep(1000);
	   if(rm_error_cnt_file != NULL)
	   {
            printf("RM_INIT_ERROR_CNT_CLEAR\r\n");
			fwrite("0", 1, 1, rm_error_cnt_file);  
            usleep(50000);  
			fclose(rm_error_cnt_file); 
	   }	        
    }
     return rm_error_cnt_result;  
}    

unsigned char rm_error_ready_write(unsigned char index)
{
    FILE *rm_error_ready_file;
    unsigned char rm_error_ready_write[MODEL_NAME_LEN] = {0,};  
    unsigned char rm_error_ready_result = 0;

    if(index == RM_ERROR_INDEX)
    {
        rm_error_ready_file = fopen(RM_INIT_ERROR_READY_FILE_PATH, "r");

        if(rm_error_ready_file != NULL)
        {
            fread(&rm_error_ready_write, sizeof(rm_error_ready_write), 1, rm_error_ready_file);  
            fclose(rm_error_ready_file);  
            usleep(1000);
            if(strncasecmp(rm_error_ready_write,"0",1)==0)    
            {
                printf("RM_INIT_ERROR_REDY_COUNT = 1\r\n");
                rm_error_ready_file = fopen(RM_INIT_ERROR_READY_FILE_PATH, "w");
                fwrite("1", 1, 1, rm_error_ready_file); 
                usleep(50000);
                fclose(rm_error_ready_file);
                system("sync");	
                system("reboot");	
                usleep(100000);
            }
            else if(strncasecmp(rm_error_ready_write,"1",1)==0)    
            {
                printf("RM_INIT_ERROR_REDY_COUNT = 2\r\n");
                rm_error_ready_file = fopen(RM_INIT_ERROR_READY_FILE_PATH, "w");
                fwrite("2", 1, 1, rm_error_ready_file);
                usleep(50000);
                fclose(rm_error_ready_file);  
                system("sync");	
                system("reboot");	
                usleep(100000);
            }
            else if(strncasecmp(rm_error_ready_write,"2",1)==0)    
            {
                printf("RM_INIT_ERROR_REDY_COUNT = 3\r\n");
                rm_error_ready_file = fopen(RM_INIT_ERROR_READY_FILE_PATH, "w");
                fwrite("3", 1, 1, rm_error_ready_file);
                usleep(50000);
                fclose(rm_error_ready_file);  
                system("sync");	
                system("reboot");	
                usleep(100000);
            }        
            else if(strncasecmp(rm_error_ready_write,"3",1)==0)    
            {
                printf("RM_INIT_ERROR_REDY_COUNT = 4\r\n");
                rm_error_ready_file = fopen(RM_INIT_ERROR_READY_FILE_PATH, "w");
                fwrite("0", 1, 1, rm_error_ready_file); 
                usleep(50000);
                fclose(rm_error_ready_file); 
                system("sync");	          
                rm_error_ready_result = 1; 
                usleep(100000);
            } 
        }
    }  
    else if(index == RM_ERROR_CLEAR_INDEX)  
    {
       rm_error_ready_file = fopen(RM_INIT_ERROR_READY_FILE_PATH,"w");   
	   usleep(1000);
	   if(rm_error_ready_file != NULL)
	   {
            printf("RM_INIT_ERROR_REDY_COUNT_CLEAR\r\n");
			fwrite("0", 1, 1, rm_error_ready_file); 
            usleep(50000);   
			fclose(rm_error_ready_file); 
	   }	        
    }
    return rm_error_ready_result;  
} */ 

void ex_vol_set_task(unsigned char index)
{
    int i = 0;
    char data[100] = {0};
    int value = 1800;

    power_off(); 
    usleep(10000);
    if(index < 14)
    {
        memset(&signal_group.signal_config.dc_voltage[0], 0, sizeof(signal_group.signal_config.dc_voltage[0])); 

        signal_group.signal_config.dc_voltage[index] = value;	
        Power_Supply_Voltage_load();
        SET_DAC_OUTPUT_VALUE(index);
        if(index ==ELVDD)	gpiops_set_value(RELAY_ELVDD,RELAY_ON);
        else if(index ==ADD8_S)	gpiops_set_value(RELAY_VDD8_S,RELAY_ON);
        else if(index ==ADD8_G)	gpiops_set_value(RELAY_VDD8_G,RELAY_ON);
        else if(index ==VGH)	gpiops_set_value(RELAY_VGH,RELAY_ON);
        else if(index ==VGL)	gpiops_set_value(RELAY_VGL,RELAY_ON);	
        else if(index ==VINIT)	gpiops_set_value(RELAY_VINIT,RELAY_ON);				
        gpio_enble->GPIO_DATA |= (0x00000001<<(index&0x3fff));
    }
    else if(index == EX_RSTB)   logic_gpio_ctl("RSTB",1);	
    else if(index == EX_TM)   logic_gpio_ctl("TM",1);	
    else if(index == EX_BISTEN)   logic_gpio_ctl("BISTEN",1);	
    else if(index == EX_LPSPARE1)   logic_gpio_ctl("LPSPARE1",1);	 
    usleep(1000);
    ex_input_vol_adc_result[index] =    ex_input_vol_sensing_task(index); 
    data[0] = EX_ADC_SEN;	
    data[1] = index;						
    ex_port_send_task(data,EX_ADC_SEN_LEN);
    ex_port_serial_task();     
}

unsigned char ex_input_vol_sensing_task(unsigned char ch) 	
{
    unsigned char other_ch_index = 0;
    unsigned char result = 0;
    int i = 0;

    for(i = 0 ; i < 18 ; i++)
    {
        if(cprf)    printf("ex_input_vol_sensing_value[%d] = %d\r\n", other_ch_index, adc_sensing_value[other_ch_index]);
        other_ch_index = ex_voltage__sensing_select_task(i); 
        ADC_SELECT_DATA_READ_AVG(other_ch_index);   

        if((adc_sensing_value[other_ch_index] >-30)  &   (adc_sensing_value[other_ch_index] < 30))  ;
        else    
        {
            result = 1;
        }   
    }
    return result;          
}

unsigned char ex_input_vol_compare_task(void) 	
{
    int i = 0;
    int result = 0;
    int total_result = 0;
    char data[100] = {0};

    memset(&signal_group.signal_config.dc_voltage[0], 0, sizeof(signal_group.signal_config.dc_voltage[0])); 
    signal_group.signal_config.dc_voltage[ELVDD] = 1000;	
    Power_Supply_Voltage_load();
    SET_DAC_OUTPUT_VALUE(ELVDD);
    gpiops_set_value(RELAY_ELVDD,RELAY_ON);				
    gpio_enble->GPIO_DATA |= (0x00000001<<(ELVDD&0x3fff));	
    //ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_4000SPS);

    for(i = 0 ; i < 18 ; i++)
    {
        result = 0;
        data[0] = EX_LDO_SELECT;	
        data[1] = i;	
        ex_port_send_task(data,EX_LDO_SELECT_LEN);	
        usleep(10000);			
        ex_adc_total_result[EX_LDO_ELVDD_2+i] = ex_voltage_sensing_task(i);
        //result = ex_voltage_sensing_task(i);
        //if(result)	total_result |= 1<<i;
        printf("ex_adc_total_result[%d] = %x\r\n", EX_LDO_ELVDD_2+i,ex_adc_total_result[EX_LDO_ELVDD_2+i]);
    }
    power_off();
    data[0] = EX_LDO_TASK_END;	
    data[1] = 0x00;		 
    ex_port_send_task(data,EX_LDO_TASK_END_LEN);
}

unsigned char ex_auto_cal_task(void) 	
{
    unsigned char category_start_flag = 0xff;
    int a = 0;
    int i = 0;
    int ch = 0;
    char num = 0;	
    unsigned char data[100];
    unsigned char vol_cur_select_task_result = 0; 
    
    memset(&data,0, sizeof(data));

    vol_cur_select_task_result = vol_cur_select_task(SELECT_VOL);
    if(!vol_cur_select_task_result)
	{    
        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);
        for(ch = 0; ch < 5 ; ch++)
        {
            ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
            if(category_start_flag == DAC_ADC_AUTO_STOP)    return  vol_cur_select_task_result = CAL_CANCEL;   

            usleep(100000);	
            for(i = 0; i < 2 ; i++)
            {
                ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
                if(category_start_flag == DAC_ADC_AUTO_STOP)    return  vol_cur_select_task_result = CAL_CANCEL;                  
                if(ch < ADC_CH3)
                {
                    if(i == CAL_15V)  signal_group.signal_config.dc_voltage[ELVDD] = 15000;	
                    else if(i == CAL_0V) signal_group.signal_config.dc_voltage[ELVDD] = 0;	 
                    Power_Supply_Voltage_load();
                    SET_DAC_OUTPUT_VALUE(ELVDD);
                    gpio_enble->GPIO_DATA |= (0x00000001<<(ELVDD&0x3fff));    
                    gpiops_set_value(RELAY_ELVDD,RELAY_ON);
                    usleep(10000);	

                    data[0] = EX_LDO_SELECT;	
                    if(ch == ADC_CH0)    data[1] = EX_LDO_ELVDD;	
                    else if(ch == ADC_CH1)   data[1] = EX_VOTP50;	
                    else if(ch == ADC_CH2)   data[1] = EX_LM_SPARE1;	                
                    ex_port_send_task(data,EX_LDO_SELECT_LEN);
                    usleep(100000);	
                    data[0] = AUTO_CAL_TASK;	
                    if(ch == ADC_CH0)    data[1] = EX_CAL_LDO_ELVDD;	
                    else if(ch == ADC_CH1)   data[1] = EX_CAL_VOTP50;	
                    else if(ch == ADC_CH2)   data[1] = EX_CAL_LM_SPARE1;	 
                    ex_port_send_task(data,AUTO_CAL_TASK_LEN);	
                    ex_port_serial_task();
                }
                else
                {
                    if(ch == ADC_CH3)
                    {   
                        if(i == CAL_15V)  signal_group.signal_config.dc_voltage[AVDD] = 15000;	
                        else if(i == CAL_0V) signal_group.signal_config.dc_voltage[AVDD] = 0;	
                        Power_Supply_Voltage_load();
                        SET_DAC_OUTPUT_VALUE(AVDD); 
                        gpio_enble->GPIO_DATA |= (0x00000001<<(AVDD&0x3fff)); 
                        usleep(100000);	
                        data[0] = AUTO_CAL_TASK;	
                        data[1] = EX_CAL_AVDD;	
                        ex_port_send_task(data,AUTO_CAL_TASK_LEN);	
                        ex_port_serial_task();                      
                    }
                    else 
                    {
                        if(i == CAL_15V)  signal_group.signal_config.dc_voltage[ELVDD] = 15000;	
                        else if(i == CAL_0V) signal_group.signal_config.dc_voltage[ELVDD] = 0;	
                        Power_Supply_Voltage_load();
                        SET_DAC_OUTPUT_VALUE(ELVDD); 
                        gpio_enble->GPIO_DATA |= (0x00000001<<(ELVDD&0x3fff));
                        gpiops_set_value(RELAY_ELVDD,RELAY_ON); 
                        usleep(100000);	
                        data[0] = AUTO_CAL_TASK;	
                        data[1] = EX_CAL_ELVDD;	
                        ex_port_send_task(data,AUTO_CAL_TASK_LEN);	
                        ex_port_serial_task();                         
                    }                
                }
                if(ch == ADC_CH0)
                {	
                    if(i == CAL_15V)  ads124_cal0.adc_15v_value = auto_cal_data_float;
                    else if(i == CAL_0V) ads124_cal0.adc_0v_value = auto_cal_data_float; 
                    //ADC_AUTO_DATA_READ_FOR_CAL();
                    ADC_SELECT_DATA_READ_FOR_CAL(SEN_LDO_ELVDD);
                    if(i == CAL_15V)      ads124_cal0.adc_15v_step = adc_sensing_value_for_cal[SEN_LDO_ELVDD];
                    else if(i == CAL_0V) ads124_cal0.adc_0v_step = adc_sensing_value_for_cal[SEN_LDO_ELVDD]; 
                    if(i == CAL_15V)          printf("ads124[0] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_LDO_ELVDD], ads124_cal0.adc_15v_step, ads124_cal0.adc_15v_value);
                    else if(i == CAL_0V)     printf("ads124[0] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_LDO_ELVDD], ads124_cal0.adc_0v_step, ads124_cal0.adc_0v_value);                
                    if(i==CAL_0V)    auto_cal_apply_task(ADC_CH0);
                }
                else if(ch == ADC_CH1)
                {	
                    if(i == CAL_15V)  ads124_cal1.adc_15v_value = auto_cal_data_float;
                    else if(i == CAL_0V) ads124_cal1.adc_0v_value = auto_cal_data_float; 
                    //ADC_AUTO_DATA_READ_FOR_CAL();
                    ADC_SELECT_DATA_READ_FOR_CAL(SEN_VOTP50);
                    if(i == CAL_15V)      ads124_cal1.adc_15v_step = adc_sensing_value_for_cal[SEN_VOTP50];
                    else if(i == CAL_0V) ads124_cal1.adc_0v_step = adc_sensing_value_for_cal[SEN_VOTP50]; 
                    if(i == CAL_15V)          printf("ads124[1] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_VOTP50], ads124_cal1.adc_15v_step, ads124_cal1.adc_15v_value);
                    else if(i == CAL_0V)     printf("ads124[1] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_VOTP50], ads124_cal1.adc_0v_step, ads124_cal1.adc_0v_value);                
                    if(i==CAL_0V)    auto_cal_apply_task(ADC_CH1);
                }  
                else if(ch == ADC_CH2)
                {	
                    if(i == CAL_15V)  ads124_cal2.adc_15v_value = auto_cal_data_float;
                    else if(i == CAL_0V) ads124_cal2.adc_0v_value = auto_cal_data_float; 
                    //ADC_AUTO_DATA_READ_FOR_CAL();
                    ADC_SELECT_DATA_READ_FOR_CAL(SEN_LM_SPARE1);
                    if(i == CAL_15V)      ads124_cal2.adc_15v_step = adc_sensing_value_for_cal[SEN_LM_SPARE1];
                    else if(i == CAL_0V) ads124_cal2.adc_0v_step = adc_sensing_value_for_cal[SEN_LM_SPARE1]; 
                    if(i == CAL_15V)          printf("ads124[2] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_LM_SPARE1], ads124_cal2.adc_15v_step, ads124_cal2.adc_15v_value);
                    else if(i == CAL_0V)     printf("ads124[2] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_LM_SPARE1], ads124_cal2.adc_0v_step, ads124_cal2.adc_0v_value);                 
                    if(i==CAL_0V)   auto_cal_apply_task(ADC_CH2);
                } 
                else if(ch == ADC_CH3)
                {	
                    if(i == CAL_15V)  ads124_cal3.adc_15v_value = auto_cal_data_float;
                    else if(i == CAL_0V) ads124_cal3.adc_0v_value = auto_cal_data_float; 
                    //ADC_AUTO_DATA_READ_FOR_CAL();
                    ADC_SELECT_DATA_READ_FOR_CAL(SEN_AVDD);
                    if(i == CAL_15V)      ads124_cal3.adc_15v_step = adc_sensing_value_for_cal[SEN_AVDD];
                    else if(i == CAL_0V) ads124_cal3.adc_0v_step = adc_sensing_value_for_cal[SEN_AVDD]; 
                    if(i == CAL_15V)          printf("ads124[3] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_AVDD], ads124_cal3.adc_15v_step, ads124_cal3.adc_15v_value);
                    else if(i == CAL_0V)     printf("ads124[3] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_AVDD], ads124_cal3.adc_0v_step, ads124_cal3.adc_0v_value);                 
                    if(i==CAL_0V)   auto_cal_apply_task(ADC_CH3);
                } 
                else if(ch == ADC_CH4)
                {	
                    if(i == CAL_15V)  ads124_cal4.adc_15v_value = auto_cal_data_float;
                    else if(i == CAL_0V) ads124_cal4.adc_0v_value = auto_cal_data_float; 
                    //ADC_AUTO_DATA_READ_FOR_CAL();
                    ADC_SELECT_DATA_READ_FOR_CAL(SEN_ELVDD);
                    if(i == CAL_15V)      ads124_cal4.adc_15v_step = adc_sensing_value_for_cal[SEN_ELVDD];
                    else if(i == CAL_0V) ads124_cal4.adc_0v_step = adc_sensing_value_for_cal[SEN_ELVDD]; 
                    if(i == CAL_15V)          printf("ads124[4] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELVDD], ads124_cal4.adc_15v_step, ads124_cal4.adc_15v_value);
                    else if(i == CAL_0V)     printf("ads124[4] = %x / %f / %f\r\n", adc_sensing_value_for_cal[SEN_ELVDD], ads124_cal4.adc_0v_step, ads124_cal4.adc_0v_value);                 
                    if(i==CAL_0V)   auto_cal_apply_task(ADC_CH4);
                }                                                	    
                data[0] = EX_LDO_TASK_END;	
                data[1] = 0x00;		 
                ex_port_send_task(data,EX_LDO_TASK_END_LEN);    
            }        
        }
        data[0] = AUTO_CAL_TASK;	
        data[1] = EX_CAL_END;	
        ex_port_send_task(data,AUTO_CAL_TASK_LEN);	
        ex_port_serial_task();   
        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);
    } 
    else vol_cur_select_task_result = KEITHLEY_COM_ERROR; 

    ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
    if(category_start_flag == DAC_ADC_AUTO_STOP)    return  vol_cur_select_task_result = CAL_CANCEL;  

    return vol_cur_select_task_result;
}

void auto_cal_apply_task(unsigned index) 	
{
    if(index == ADC_CH0)
    {
        printf("ads124_cal0.adc_15v_value = %f / ads124_cal0.adc_0v_value = %f\r\n", ads124_cal0.adc_15v_value, ads124_cal0.adc_0v_value);
        printf("ads124_cal0.adc_15v_step = %f / ads124_cal0.adc_0v_step = %f\r\n", ads124_cal0.adc_15v_step, ads124_cal0.adc_0v_step);
        ads124_cal0.adc_p_ratio = (ads124_cal0.adc_15v_value-ads124_cal0.adc_0v_value) / (ads124_cal0.adc_15v_step - ads124_cal0.adc_0v_step);
        ads124_cal0.adc_p_offset = ads124_cal0.adc_0v_value - (ads124_cal0.adc_p_ratio * ads124_cal0.adc_0v_step);
        printf("ads124_cal0.adc_p_ratio = %.10f\r\n", ads124_cal0.adc_p_ratio);
        printf("ads124_cal0.adc_p_offset = %.10f\r\n", ads124_cal0.adc_p_offset);    
    }
    else if(index == ADC_CH1)
    {
		printf("ads124_cal1.adc_15v_value = %f / ads124_cal1.adc_0v_value = %f\r\n", ads124_cal1.adc_15v_value, ads124_cal1.adc_0v_value);
		printf("ads124_cal1.adc_15v_step = %f / ads124_cal1.adc_0v_step = %f\r\n", ads124_cal1.adc_15v_step, ads124_cal1.adc_0v_step);
		ads124_cal1.adc_p_ratio = (ads124_cal1.adc_15v_value-ads124_cal1.adc_0v_value) / (ads124_cal1.adc_15v_step - ads124_cal1.adc_0v_step);
		ads124_cal1.adc_p_offset = ads124_cal1.adc_0v_value - (ads124_cal1.adc_p_ratio * ads124_cal1.adc_0v_step);
		printf("ads124_cal1.adc_p_ratio = %.10f\r\n", ads124_cal1.adc_p_ratio);
		printf("ads124_cal1.adc_p_offset = %.10f\r\n", ads124_cal1.adc_p_offset);        
    }
    else if(index == ADC_CH2)
    {
		printf("ads124_cal2.adc_15v_value = %f / ads124_cal2.adc_0v_value = %f\r\n", ads124_cal2.adc_15v_value, ads124_cal2.adc_0v_value);
		printf("ads124_cal2.adc_15v_step = %f / ads124_cal2.adc_0v_step = %f\r\n", ads124_cal2.adc_15v_step, ads124_cal2.adc_0v_step);
		ads124_cal2.adc_p_ratio = (ads124_cal2.adc_15v_value-ads124_cal2.adc_0v_value) / (ads124_cal2.adc_15v_step - ads124_cal2.adc_0v_step);
		ads124_cal2.adc_p_offset = ads124_cal2.adc_0v_value - (ads124_cal2.adc_p_ratio * ads124_cal2.adc_0v_step);
		printf("ads124_cal2.adc_p_ratio = %.10f\r\n", ads124_cal2.adc_p_ratio);
		printf("ads124_cal2.adc_p_offset = %.10f\r\n", ads124_cal2.adc_p_offset);     
    } 
    else if(index == ADC_CH3)
    {
		printf("ads124_cal3.adc_15v_value = %f / ads124_cal3.adc_0v_value = %f\r\n", ads124_cal3.adc_15v_value, ads124_cal3.adc_0v_value);
		printf("ads124_cal3.adc_15v_step = %f / ads124_cal3.adc_0v_step = %f\r\n", ads124_cal3.adc_15v_step, ads124_cal3.adc_0v_step);
		ads124_cal3.adc_p_ratio = (ads124_cal3.adc_15v_value-ads124_cal3.adc_0v_value) / (ads124_cal3.adc_15v_step - ads124_cal3.adc_0v_step);
		ads124_cal3.adc_p_offset = ads124_cal3.adc_0v_value - (ads124_cal3.adc_p_ratio * ads124_cal3.adc_0v_step);
		printf("ads124_cal3.adc_p_ratio = %.10f\r\n", ads124_cal3.adc_p_ratio);
		printf("ads124_cal3.adc_p_offset = %.10f\r\n", ads124_cal3.adc_p_offset);        
    }
    else if(index == ADC_CH4)
    {
		printf("ads124_cal4.adc_15v_value = %f / ads124_cal4.adc_0v_value = %f\r\n", ads124_cal4.adc_15v_value, ads124_cal4.adc_0v_value);
		printf("ads124_cal4.adc_15v_step = %f / ads124_cal4.adc_0v_step = %f\r\n", ads124_cal4.adc_15v_step, ads124_cal4.adc_0v_step);				
		ads124_cal4.adc_p_ratio = (ads124_cal4.adc_15v_value-ads124_cal4.adc_0v_value) / (ads124_cal4.adc_15v_step - ads124_cal4.adc_0v_step);
		ads124_cal4.adc_p_offset = ads124_cal4.adc_0v_value - (ads124_cal4.adc_p_ratio * ads124_cal4.adc_0v_step);
		printf("ads124_cal4.adc_p_ratio = %.10f\r\n", ads124_cal4.adc_p_ratio);
		printf("ads124_cal4.adc_p_offset = %.10f\r\n", ads124_cal4.adc_p_offset);        
    }   
}

void ldo_all_on_off_task(unsigned index) 	
{
    char num = 0;	
    unsigned char data[100];    
    memset(&data,0, sizeof(data));
    if(index == EX_LDO_ALL_ON)  data[0] = LDO_ON_FOR_VOL_CHECK;	
    else    data[0] = EX_LDO_TASK_END;	 
    data[1] = 0x00;	
    ex_port_send_task(data,AUTO_CAL_TASK_LEN);	    
}

void auto_cal_successfully_check_task(void) 	
{
    int i = 0;
    int vol_lev = 0;
    unsigned char result = 0;
    ldo_all_on_off_task(EX_LDO_ALL_ON);
    //for(vol_lev = 8 ; vol_lev < EX_N25V_COMPARE+1 ; vol_lev++)
    for(vol_lev = 0 ; vol_lev < EX_N25V_COMPARE+1 ; vol_lev++)
    {
        for(i = 0 ; i<14 ; i++)	
        {
            if(vol_lev == EX_25V_COMPARE) signal_group.signal_config.dc_voltage[i] = 25000;
            else if(vol_lev == EX_15V_COMPARE) signal_group.signal_config.dc_voltage[i] = 15000;
            else if(vol_lev == EX_10V_COMAPRE) signal_group.signal_config.dc_voltage[i] = 10000;
            else if(vol_lev == EX_5V_COMPARE) signal_group.signal_config.dc_voltage[i] = 5000;
            else if(vol_lev == EX_0V_COMPARE) signal_group.signal_config.dc_voltage[i] = 0;
            else if(vol_lev == EX_N5V_COMPARE) signal_group.signal_config.dc_voltage[i] = -5000;
            else if(vol_lev == EX_N10V_COMAPRE) signal_group.signal_config.dc_voltage[i] = -10000;
            else if(vol_lev == EX_N15V_COMAPRE) signal_group.signal_config.dc_voltage[i] = -15000;
            else if(vol_lev == EX_N25V_COMPARE) signal_group.signal_config.dc_voltage[i] = -25000;
        }			
        Power_Supply_Voltage_load();
        for(i=0 ; i< 14 ; i++)	
        {
            SET_DAC_OUTPUT_VALUE(i);
            gpio_enble->GPIO_DATA |= (0x00000001<<(i&0x3fff));
            if(i ==ELVDD)	gpiops_set_value(RELAY_ELVDD,RELAY_ON);
            else if(i ==ADD8_S)	gpiops_set_value(RELAY_VDD8_S,RELAY_ON);
            else if(i ==ADD8_G)	gpiops_set_value(RELAY_VDD8_G,RELAY_ON);
            else if(i ==VGH)	gpiops_set_value(RELAY_VGH,RELAY_ON);
            else if(i ==VGL)	gpiops_set_value(RELAY_VGL,RELAY_ON);	
            else if(i ==VINIT)	gpiops_set_value(RELAY_VINIT,RELAY_ON);	        
        }     
        usleep(100000);	
        ADC_AUTO_DATA_READ();
        usleep(100000);
        if(vol_lev == EX_25V_COMPARE)  result = auto_cal_data_compare_task(EX_25V_COMPARE);
        else if(vol_lev == EX_15V_COMPARE)  result = auto_cal_data_compare_task(EX_15V_COMPARE);
        else if(vol_lev == EX_10V_COMAPRE)  result = auto_cal_data_compare_task(EX_10V_COMAPRE);
        else if(vol_lev == EX_5V_COMPARE)  result = auto_cal_data_compare_task(EX_5V_COMPARE);
        else if(vol_lev == EX_0V_COMPARE)  result = auto_cal_data_compare_task(EX_0V_COMPARE);
        else if(vol_lev == EX_N5V_COMPARE)  result = auto_cal_data_compare_task(EX_N5V_COMPARE);
        else if(vol_lev == EX_N10V_COMAPRE)  result = auto_cal_data_compare_task(EX_N10V_COMAPRE);
        else if(vol_lev == EX_N15V_COMAPRE)  result = auto_cal_data_compare_task(EX_N15V_COMAPRE);
        else if(vol_lev == EX_N25V_COMPARE)  result = auto_cal_data_compare_task(EX_N25V_COMPARE);        
        printf("---------------------[%d]result = %d-----------------------------\r\n", vol_lev,result);
    }
    ldo_all_on_off_task(EX_LDO_ALL_OFF); 
}
unsigned char auto_cal_data_compare_task(unsigned char index) 	
{
    unsigned char result = 0;
    unsigned char ch_index = 0;
    unsigned char data[100];   
    short compare_data = 0;
    short result_value = 0;
    int i = 0;
    memset(&data,0, sizeof(data));
    for(i = EX_CAL_LM_SPARE2 ; i >= 0 ; i--)
    {
        usleep(50000);   
        data[0] = TASK_FOR_VOL_CHECK;	
        data[1] = i;	
        ex_port_send_task(data,AUTO_CAL_TASK_LEN);	
        ex_port_serial_task();
        usleep(50000);   
        compare_data = auto_cal_data_float * 1000;
        ch_index = auto_cal_data_compare_sensing_select_task(i);
        result_value = compare_data - adc_sensing_value[ch_index];
        printf("result = keithley - adc_sensing_value[%d]\r\n",ch_index);
        printf("[%d] = [%d] - [%d]\r\n",result_value,compare_data,adc_sensing_value[ch_index]);                                                        
    }
    return  result; 
}

unsigned char auto_cal_data_compare_sensing_select_task(unsigned char ch) 	
{
    unsigned char index_result = 0; 

    if(ch == EX_CAL_AVDD)  index_result = SEN_AVDD;
    else if(ch == EX_CAL_ELVSS)  index_result = SEN_ELVSS;
    else if(ch == EX_CAL_ELVDD)  index_result = SEN_ELVDD ;
    else if(ch == EX_CAL_ADD8_S)  index_result = SEN_ADD8_S;
    else if(ch == EX_CAL_ADD8_G)  index_result = SEN_ADD8_G;
    else if(ch == EX_CAL_VGH)  index_result = SEN_VGH;
    else if(ch == EX_CAL_VGL)  index_result = SEN_VGL;
    else if(ch == EX_CAL_VINIT)  index_result = SEN_VINT;
    else if(ch == EX_CAL_APSPARE1)  index_result = SEN_APSPARE1;
    else if(ch == EX_CAL_APSPARE2)  index_result = SEN_APSPARE2;
    else if(ch == EX_CAL_VDD11)  index_result = SEN_VDD11;
    else if(ch == EX_CAL_VDD18)  index_result = SEN_VDD18;
    else if(ch == EX_CAL_DSPARE1)  index_result = SEN_DPSPARE1;
    else if(ch == EX_CAL_DSAPRE2)  index_result = SEN_DPSAPRE2;
    else if(ch == EX_CAL_LDO_ELVDD)  index_result = SEN_LDO_ELVDD;
    else if(ch == EX_CAL_LDO_OSC)   index_result = SEN_LDO_OSC;
    else if(ch == EX_CAL_LDO_VGH)   index_result = SEN_LDO_VGH; 
    else if(ch == EX_CAL_LDO_VGL)   index_result = SEN_LDO_VGL; 
    else if(ch == EX_CAL_LDO_VINT)   index_result = SEN_LDO_VINT; 
    else if(ch == EX_CAL_VCIR)   index_result = SEN_VCIR; 
    else if(ch == EX_CAL_VREF1)   index_result = SEN_VREF1; 
    else if(ch == EX_CAL_VREG1)   index_result = SEN_VREG1; 
    else if(ch == EX_CAL_VOTP50)   index_result = SEN_VOTP50; 
    else if(ch == EX_CAL_PM_SPARE)   index_result = SEN_PM_SPARE1;  
    else if(ch == EX_CAL_MON1)   index_result = SEN_MON1;    
    else if(ch == EX_CAL_MON2)   index_result = SEN_MON2;    
    else if(ch == EX_CAL_MON3)   index_result = SEN_MON3;    
    else if(ch == EX_CAL_MON4)   index_result = SEN_MON4;              
    else if(ch == EX_CAL_MON5)   index_result = SEN_MON5;    
    else if(ch == EX_CAL_MON6)   index_result = SEN_MON6;       
    else if(ch == EX_CAL_LM_SPARE1)   index_result = SEN_LM_SPARE1;    
    else if(ch == EX_CAL_LM_SPARE2)   index_result = SEN_LM_SPARE2; 
    return index_result;
}



void keithley_init_task(void)
{
    char num = 0;	
    unsigned char data[100];    
    memset(&data,0, sizeof(data));
    data[0] = KEITHLEY_INIT;	 
    data[1] = 0x00;	
    ex_port_send_task(data,KEITHLEY_INIT_LEN);	    
}

void cal_end_task(void)
{
    char num = 0;	
    unsigned char data[100];    
    memset(&data,0, sizeof(data));
    data[0] = CAL_END;	 
    data[1] = 0x00;	
    ex_port_send_task(data,CAL_END_LEN);	    
}

unsigned char vol_cur_select_task(unsigned char index)
{
    char num = 0;	
    unsigned char data[100]; 
    unsigned char vol_cur_select_task_result = 0;
    vol_cur_select_flag = 1;
    memset(&data,0, sizeof(data));
    data[0] = VOL_CUR_SELECT;	 
    data[1] = index;	
    ex_port_send_task(data,VOL_CUR_SELECT_LEN);	
    ex_port_serial_task();	  
    usleep(10000);
    if(vol_cur_select_flag) vol_cur_select_task_result= 1;
    else    vol_cur_select_task_result = 0;
    return  vol_cur_select_task_result;         
}

unsigned char auto_cal_cur_task(unsigned char index)
{
    unsigned char vol_cur_select_task_result = 0; 
    unsigned char data[100];
    double cur_data = 0;
    int i = 0;
    memset(&data,0, sizeof(data));
    vol_cur_select_task_result = vol_cur_select_task(SELECT_CUR);
    if(!vol_cur_select_task_result)
    {
        if(index == EX_CAL_AVDD)
        {
            memset(&avdd_cur_cal, 0, sizeof(avdd_cur_cal));   
            memset(&avdd_cur_offset_cal, 0, sizeof(avdd_cur_offset_cal));
        }
        else
        {
            memset(&elvss_cur_cal, 0, sizeof(elvss_cur_cal));   
            memset(&elvss_cur_offset_cal, 0, sizeof(elvss_cur_offset_cal));            
        } 
        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);
        if(index == EX_CAL_AVDD) signal_group.signal_config.dc_voltage[EX_CAL_AVDD] = 5000;
        else    signal_group.signal_config.dc_voltage[EX_CAL_ELVSS] = 5000;
        Power_Supply_Voltage_load();
        SET_DAC_OUTPUT_VALUE(index);
        gpio_enble->GPIO_DATA |= (0x00000001<<(index&0x3fff));

        for(i = SELECT_500mA ; i<SELECT_0A+1 ; i++)
        {
            usleep(100000);	
            data[0] = AUTO_CAL_CUR_TASK;	
            data[1] = index;	
            if(i==SELECT_500mA) data[2] = SELECT_10OHM;
            else    data[2] = SELECT_OPEN; 

            ex_port_send_task(data,AUTO_CAL_CUR_TASK_LEN);	
            ex_port_serial_task();

            if(index == EX_CAL_AVDD)    ADC_SELECT_DATA_READ_FOR_CAL(SEN_ELIDD);	
            else    ADC_SELECT_DATA_READ_FOR_CAL(SEN_ELISS);   
            usleep(100000);	
            cur_data = (double)auto_cal_data/1000; 
             if(index == EX_CAL_AVDD) 
             {
                if(i==SELECT_500mA)
                {
                    avdd_cur_cal.cur_1a_value = cur_data;
                    avdd_cur_cal.cur_1a_step = adc_sensing_value_for_cal[SEN_ELIDD];
                    //printf("cur_data = %f\r\n",cur_data);
                    printf("avdd_cur_cal.cur_1a_value = %f /avdd_cur_cal.cur_1a_step = %f\r\n",avdd_cur_cal.cur_1a_value,avdd_cur_cal.cur_1a_step);
                }
                else
                {
                    avdd_cur_cal.cur_0a_value = cur_data;
                    avdd_cur_cal.cur_0a_step = adc_sensing_value_for_cal[SEN_ELIDD];
                    //printf("cur_data = %f\r\n",cur_data);
                    printf("avdd_cur_cal.cur_0a_value = %f /avdd_cur_cal.cur_0a_step = %f\r\n",avdd_cur_cal.cur_0a_value,avdd_cur_cal.cur_0a_step);               
                }
             }
             else
             {
                if(i==SELECT_500mA)
                {
                    elvss_cur_cal.cur_1a_value = cur_data;
                    elvss_cur_cal.cur_1a_step = adc_sensing_value_for_cal[SEN_ELISS];
                    //printf("cur_data = %f\r\n",cur_data);
                    printf("elvss_cur_cal.cur_1a_value = %f /elvss_cur_cal.cur_1a_step = %f\r\n",elvss_cur_cal.cur_1a_value,elvss_cur_cal.cur_1a_step);
                }
                else
                {
                    elvss_cur_cal.cur_0a_value = cur_data;
                    elvss_cur_cal.cur_0a_step = adc_sensing_value_for_cal[SEN_ELISS];
                    //printf("cur_data = %f\r\n",cur_data);
                    printf("elvss_cur_cal.cur_0a_value = %f /elvss_cur_cal.cur_0a_step = %f\r\n",elvss_cur_cal.cur_0a_value,elvss_cur_cal.cur_0a_step);               
                }                
             }
        } 
        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);    
         if(index == EX_CAL_AVDD) 
         {
            avdd_cur_cal.cur_1a_ratio = (avdd_cur_cal.cur_1a_value-avdd_cur_cal.cur_0a_value) / (avdd_cur_cal.cur_1a_step - avdd_cur_cal.cur_0a_step);
            avdd_cur_cal.cur_1a_offset = avdd_cur_cal.cur_0a_value - (avdd_cur_cal.cur_1a_ratio * avdd_cur_cal.cur_0a_step);		
            printf("avdd_cur_cal.cur_1a_ratio = %.10f\r\n", avdd_cur_cal.cur_1a_ratio);
            printf("avdd_cur_cal.cur_1a_offset = %.10f\r\n", avdd_cur_cal.cur_1a_offset);	            
         } 
         else
         {
            elvss_cur_cal.cur_1a_ratio = (elvss_cur_cal.cur_1a_value-elvss_cur_cal.cur_0a_value) / (elvss_cur_cal.cur_1a_step - elvss_cur_cal.cur_0a_step);
            elvss_cur_cal.cur_1a_offset = elvss_cur_cal.cur_0a_value - (elvss_cur_cal.cur_1a_ratio * elvss_cur_cal.cur_0a_step);				
            printf("elvss_cur_cal.cur_1a_ratio = %.10f\r\n", elvss_cur_cal.cur_1a_ratio);
            printf("elvss_cur_cal.cur_1a_offset = %.10f\r\n", elvss_cur_cal.cur_1a_offset);	       
         }
    }
    return vol_cur_select_task_result;
}

void cur_sensing_compare_task(unsigned char index, unsigned char res_index, short avdd_data, short elvss_data) 	
{
    unsigned char data[100];
    double cur_data = 0;
    int i = 0;
    memset(&data,0, sizeof(data));

    if((avdd_data != 0) && (elvss_data != 0) )
    {
        if(index == EX_CAL_AVDD) signal_group.signal_config.dc_voltage[EX_CAL_AVDD] = avdd_data;
        else    signal_group.signal_config.dc_voltage[EX_CAL_ELVSS] = elvss_data;
        Power_Supply_Voltage_load();
        SET_DAC_OUTPUT_VALUE(index);
        gpio_enble->GPIO_DATA |= (0x00000001<<(index&0x3fff)); 
    }      
    data[0] = AUTO_CAL_CUR_TASK;	
    data[1] = index;	            // AVDD or ELVSS SELECT
    data[2] = res_index;            // Resistance SELECT
    ex_port_send_task(data,AUTO_CAL_CUR_TASK_LEN);	
    ex_port_serial_task();	
    cur_data = (double)auto_cal_data/1000;
    if(index == EX_CAL_AVDD)    
    {
        ADC_SELECT_DATA_READ_AVG(SEN_ELIDD);
        printf("REAL_CUR / SEN_CUR[AVDD]\r\n");	
        printf("%f / %f\r\n", cur_data,avdd_cur_sensing_data);	         	
    }
    else    
    {
        ADC_SELECT_DATA_READ_AVG(SEN_ELISS);
        printf("REAL_CUR / SEN_CUR[ELVSS]\r\n");	
        printf("%f / %f\r\n", cur_data,elvss_cur_sensing_data);	                
    }          
}

unsigned char adc_offset_task(unsigned char outch) 	
{
    unsigned char vol_cur_select_task_result = 0; 
    unsigned char data[100];
    unsigned char result = 0;
    unsigned char mode = 0;
    unsigned char ch = 0;
    unsigned char ch_index = 0;
	int auto_cal_data_divide_1000 = 0;    
	int sum_data = 0;
	double avr_data = 0;    
    int cycle = 0;
    int i = 0;
    int ii = 0;
    short vol = 0;

    memset(&data,0, sizeof(data));
    vol_cur_select_task_result = vol_cur_select_task(SELECT_VOL);
    if(!vol_cur_select_task_result)
    {
        memset(&vol_offset[outch], 0, sizeof(vol_offset[outch])); 
        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);
        ch_index = auto_cal_data_compare_sensing_select_task(outch);
        for(i = 0; i < ADC_CAL_N25V+1 ; i++)
        {
			if(kbhit())	ch = readch();
			usleep(10000);
			if((ch == 'q') || (ch=='Q'))	
			{
				result = DAC_AUTOCHECK_CANCEL;
				return	result;			
			}		
			if(mode==ADC_CAL_25V)	
			{
				printf("[ADC_OFFSET_21V_25V_START]\r\n");
				vol = 24000;
				cycle = 3;			
				ii = 0;
			}
			else if(mode==ADC_CAL_20V) 
			{
				printf("[ADC_OFFSET_16V_20V_START]\r\n");
				vol = 20000;
				cycle = 3;			
				ii = 0;
			}
			else if(mode==ADC_CAL_15V) 
			{
				printf("[ADC_OFFSET_11V_15V_START]\r\n");
				vol = 14000;
				cycle = 3;			
				ii = 0;
			}	            
			else if(mode==ADC_CAL_10V) 
			{
				printf("[ADC_OFFSET_6V_10V_START]\r\n");
				vol = 9000;
				cycle = 3;			
				ii = 0;
			}		
			else if(mode==ADC_CAL_5V) 
			{
				printf("[ADC_OFFSET_0V_5V_START]\r\n");
				vol = 4000;
				cycle = 3;			
				ii = 0;
			}	 
			else if(mode==ADC_CAL_N5V) 
			{
				printf("[ADC_OFFSET_0V_N5V_START]\r\n");
				vol = -2000;
				cycle = 3;			
				ii = 0;
			}                       
			else if(mode==ADC_CAL_N10V) 
			{
				printf("[ADC_OFFSET_6V_N10V_START]\r\n");
				vol = -7000;
				cycle = 3;			
				ii = 0;
			}
			else if(mode==ADC_CAL_N15V) 
			{
				printf("[ADC_OFFSET_11V_N15V_START]\r\n");
				vol = -12000;
				cycle = 3;			
				ii = 0;
			}            
			else if(mode==ADC_CAL_N20V) 
			{
				printf("[ADC_OFFSET_N11V_N20V_START]\r\n");
				vol = -16000;
				cycle = 3;			
				ii = 0;
			} 
			else if(mode==ADC_CAL_N25V) 
			{
				printf("[ADC_OFFSET_N21V_N25V_START]\r\n");
				vol = -22000;
				cycle = 3;			
				ii = 0;
			}                        
            for(ii = 0 ; ii <cycle ; ii++)
			{
                if(outch > EX_CAL_DSAPRE2)
                {
                    signal_group.signal_config.dc_voltage[ELVDD] = vol;	
                    Power_Supply_Voltage_load();
                    SET_DAC_OUTPUT_VALUE(ELVDD); 
                    gpio_enble->GPIO_DATA |= (0x00000001<<(ELVDD&0x3fff)); 
                    gpiops_set_value(RELAY_ELVDD,RELAY_ON);

                    data[0] = EX_LDO_SELECT;	
                    data[1] = outch -EX_CAL_LDO_ELVDD;	
                    ex_port_send_task(data,EX_LDO_SELECT_LEN);	 
                }
                else
                {
                    signal_group.signal_config.dc_voltage[outch] = vol;	
                    Power_Supply_Voltage_load();
                    SET_DAC_OUTPUT_VALUE(outch);
                    pattern_generator->PG_CONTROL = OUT_EN_ON;
                    gpio_enble->GPIO_DATA |= (0x00000001<<(outch&0x3fff));
                    if(outch ==ELVDD)	gpiops_set_value(RELAY_ELVDD,RELAY_ON);
                    else if(outch ==ADD8_S)	gpiops_set_value(RELAY_VDD8_S,RELAY_ON);
                    else if(outch ==ADD8_G)	gpiops_set_value(RELAY_VDD8_G,RELAY_ON);
                    else if(outch ==VGH)	gpiops_set_value(RELAY_VGH,RELAY_ON);
                    else if(outch ==VGL)	gpiops_set_value(RELAY_VGL,RELAY_ON);	
                    else if(outch ==VINIT)	gpiops_set_value(RELAY_VINIT,RELAY_ON);	
                }
                data[0] = AUTO_CAL_TASK;	
                data[1] = outch;	
                usleep(100000);		
                ex_port_send_task(data,AUTO_CAL_TASK_LEN);	
                ex_port_serial_task(); 
                usleep(100000);    
				auto_cal_data_divide_1000 = auto_cal_data/1000;
                ADC_SELECT_DATA_READ_AVG(ch_index);
			    sum_data += auto_cal_data_divide_1000 - adc_sensing_value[ch_index];	              
				printf("%d = %d - %d\r\n",sum_data,adc_sensing_value[ch_index],auto_cal_data_divide_1000);
				vol -= 1000;
				if(ii == cycle-1)	
				{				
					avr_data = (double)sum_data / cycle;  
                    vol_offset[outch].user_offset[mode] = avr_data;		
                    printf("vol_offset[%d].user_offset[%d] = %f\r\n",outch,mode,vol_offset[outch].user_offset[mode]);
                    mode++;   
                    avr_data = 0;
					sum_data = 0;                  
                }              	

            }     
        } 
        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);      
    }    
	else	result = KEITHLEY_COM_ERROR; 
	return	result; 
}

unsigned char adc_auto_offset_select_task(unsigned char ch) 	
{
    unsigned char index_result = 0; 

    if(ch == ADC_ELVDD)  index_result = EX_CAL_AVDD;
    else if(ch == ADC_ELVSS)  index_result = EX_CAL_ELVSS;
    else if(ch == ADC_AVDD)  index_result = EX_CAL_ELVDD ;
    else if(ch == ADC_ADD8_S)  index_result = EX_CAL_ADD8_S;
    else if(ch == ADC_ADD8_G)  index_result = EX_CAL_ADD8_G;
    else if(ch == ADC_VGH)  index_result = EX_CAL_VGH;
    else if(ch == ADC_VGL)  index_result = EX_CAL_VGL;
    else if(ch == ADC_VINT)  index_result = EX_CAL_VINIT;
    else if(ch == ADC_APSPARE1)  index_result = EX_CAL_APSPARE1;
    else if(ch == ADC_APSPARE2)  index_result = EX_CAL_APSPARE2;
    else if(ch == ADC_VDD11)  index_result = EX_CAL_VDD11;
    else if(ch == ADC_VDD18)  index_result = EX_CAL_VDD18;
    else if(ch == ADC_DPSPARE1)  index_result = EX_CAL_DSPARE1;
    else if(ch == ADC_DPSAPRE2)  index_result = EX_CAL_DSAPRE2;
    else if(ch == ADC_LDO_ELVDD)  index_result = EX_CAL_LDO_ELVDD;
    else if(ch == ADC_LDO_OSC)   index_result = EX_CAL_LDO_OSC;
    else if(ch == ADC_LDO_VGH)   index_result = EX_CAL_LDO_VGH; 
    else if(ch == ADC_LDO_VGL)   index_result = EX_CAL_LDO_VGL; 
    else if(ch == ADC_LDO_VINT)   index_result = EX_CAL_LDO_VINT; 
    else if(ch == ADC_VCIR)   index_result = EX_CAL_VCIR; 
    else if(ch == ADC_VREF1)   index_result = EX_CAL_VREF1; 
    else if(ch == ADC_VREG1)   index_result = EX_CAL_VREG1; 
    else if(ch == ADC_VOTP50)   index_result = EX_CAL_VOTP50; 
    else if(ch == ADC_PM_SPARE1)   index_result = EX_CAL_PM_SPARE;  
    else if(ch == ADC_MON1)   index_result = EX_CAL_MON1;    
    else if(ch == ADC_MON2)   index_result = EX_CAL_MON2;    
    else if(ch == ADC_MON3)   index_result = EX_CAL_MON3;    
    else if(ch == ADC_MON4)   index_result = EX_CAL_MON4;              
    else if(ch == ADC_MON5)   index_result = EX_CAL_MON5;    
    else if(ch == ADC_MON6)   index_result = EX_CAL_MON6;       
    else if(ch == ADC_LM_SPARE1)   index_result = EX_CAL_LM_SPARE1;    
    else if(ch == ADC_LM_SPARE2)   index_result = EX_CAL_LM_SPARE2; 
    return index_result;
}

unsigned char adc_select_offset_select_task(unsigned char ch) 	
{
    unsigned char index_result = 0; 

    if(ch == SEN_AVDD)  index_result = EX_CAL_AVDD;
    else if(ch == SEN_ELVSS)  index_result = EX_CAL_ELVSS;
    else if(ch == SEN_ELVDD)  index_result = EX_CAL_ELVDD ;
    else if(ch == SEN_ADD8_S)  index_result = EX_CAL_ADD8_S;
    else if(ch == SEN_ADD8_G)  index_result = EX_CAL_ADD8_G;
    else if(ch == SEN_VGH)  index_result = EX_CAL_VGH;
    else if(ch == SEN_VGL)  index_result = EX_CAL_VGL;
    else if(ch == SEN_VINT)  index_result = EX_CAL_VINIT;
    else if(ch == SEN_APSPARE1)  index_result = EX_CAL_APSPARE1;
    else if(ch == SEN_APSPARE2)  index_result = EX_CAL_APSPARE2;
    else if(ch == SEN_VDD11)  index_result = EX_CAL_VDD11;
    else if(ch == SEN_VDD18)  index_result = EX_CAL_VDD18;
    else if(ch == SEN_DPSPARE1)  index_result = EX_CAL_DSPARE1;
    else if(ch == SEN_DPSAPRE2)  index_result = EX_CAL_DSAPRE2;
    else if(ch == SEN_LDO_ELVDD)  index_result = EX_CAL_LDO_ELVDD;
    else if(ch == SEN_LDO_OSC)   index_result = EX_CAL_LDO_OSC;
    else if(ch == SEN_LDO_VGH)   index_result = EX_CAL_LDO_VGH; 
    else if(ch == SEN_LDO_VGL)   index_result = EX_CAL_LDO_VGL; 
    else if(ch == SEN_LDO_VINT)   index_result = EX_CAL_LDO_VINT; 
    else if(ch == SEN_VCIR)   index_result = EX_CAL_VCIR; 
    else if(ch == SEN_VREF1)   index_result = EX_CAL_VREF1; 
    else if(ch == SEN_VREG1)   index_result = EX_CAL_VREG1; 
    else if(ch == SEN_VOTP50)   index_result = EX_CAL_VOTP50; 
    else if(ch == SEN_PM_SPARE1)   index_result = EX_CAL_PM_SPARE;  
    else if(ch == SEN_MON1)   index_result = EX_CAL_MON1;    
    else if(ch == SEN_MON2)   index_result = EX_CAL_MON2;    
    else if(ch == SEN_MON3)   index_result = EX_CAL_MON3;    
    else if(ch == SEN_MON4)   index_result = EX_CAL_MON4;              
    else if(ch == SEN_MON5)   index_result = EX_CAL_MON5;    
    else if(ch == SEN_MON6)   index_result = EX_CAL_MON6;       
    else if(ch == SEN_LM_SPARE1)   index_result = EX_CAL_LM_SPARE1;    
    else if(ch == SEN_LM_SPARE2)   index_result = EX_CAL_LM_SPARE2; 
    return index_result;
}

void pg_check_task(void)
{
    memset(&ex_input_vol_adc_result, 0, sizeof(ex_input_vol_adc_result));	
    memset(&ex_adc_total_result, 0, sizeof(ex_adc_total_result));
    ex_input_vol_compare_task();	
    ex_adc_sen_index =	EX_AVDD; 
    ex_vol_set_task(ex_adc_sen_index);    

    ext_com_task_ack(OUTPUT_CHECK_REQUEST, OUTPUT_CHECK_REQUEST_LEN, ex_adc_total_result);
}

void serial_dev_ack_nack_task(unsigned char cmd, unsigned char len, unsigned char data)
{
    unsigned char send_data[100];
    memset(&send_data, 0, sizeof(send_data));
    send_data[0] = data;
    send_data[1] = 0x00;
    ext_com_task_ack(cmd, len, send_data);
}

void adc_cal_offset_data_save_task(void)
{
    FILE *adc_file_0;
    FILE *adc_file_1;
    FILE *adc_file_2;
    FILE *adc_file_3;
    FILE *adc_file_4;
    FILE *adc_file_offset;
    unsigned char file_open_result = 0;

    adc_file_0 = fopen(ADS124_0_CAL_FILE_PATH, "w+b");
    if(adc_file_0!= NULL)
    {
        fwrite(&ads124_cal0, sizeof(ads124_cal0), 1, adc_file_0);
        fclose(adc_file_0);
        if(pprf)	printf("ads124_cal0_data_save_OK\r\n");					
    }
    else 
    {
        if(pprf)	printf("ads124_cal0_data_save_Fail\r\n");	
        file_open_result = 1;				
    }

    adc_file_1 = fopen(ADS124_1_CAL_FILE_PATH, "w+b");
    if(adc_file_1!= NULL)
    {
        fwrite(&ads124_cal1, sizeof(ads124_cal1), 1, adc_file_1);
        fclose(adc_file_1);
        if(pprf)	printf("ads124_cal1_data_save_OK\r\n");					
    }
    else 
    {
        if(pprf)	printf("ads124_cal1_data_save_Fail\r\n");	
        file_open_result = 1;				
    }    
	
    adc_file_2 = fopen(ADS124_2_CAL_FILE_PATH, "w+b");
    if(adc_file_2!= NULL)
    {
        fwrite(&ads124_cal2, sizeof(ads124_cal2), 1, adc_file_2);
        fclose(adc_file_2);
        if(pprf)	printf("ads124_cal2_data_save_OK\r\n");					
    }
    else 
    {
        if(pprf)	printf("ads124_cal2_data_save_Fail\r\n");	
        file_open_result = 1;				
    }   

    adc_file_3 = fopen(ADS124_3_CAL_FILE_PATH, "w+b");
    if(adc_file_3!= NULL)
    {
        fwrite(&ads124_cal3, sizeof(ads124_cal3), 1, adc_file_3);
        fclose(adc_file_3);
        if(pprf)	printf("ads124_cal3_data_save_OK\r\n");					
    }
    else 
    {
        if(pprf)	printf("ads124_cal3_data_save_Fail\r\n");	
        file_open_result = 1;				
    }  

    adc_file_4 = fopen(ADS124_4_CAL_FILE_PATH, "w+b");
    if(adc_file_4!= NULL)
    {
        fwrite(&ads124_cal4, sizeof(ads124_cal4), 1, adc_file_4);
        fclose(adc_file_4);
        if(pprf)	printf("ads124_cal4_data_save_OK\r\n");					
    }
    else 
    {
        if(pprf)	printf("ads124_cal4_data_save_Fail\r\n");	
        file_open_result = 1;				
    }

    adc_file_offset = fopen(ADC_OFFSET_FILE_PATH, "w+b");
    if(adc_file_offset!= NULL)
    {
        fwrite(&vol_offset, sizeof(vol_offset), 1, adc_file_offset);
        fclose(adc_file_offset);
        if(pprf)	printf("adc_offset_data_save_OK\r\n");					
    }
    else 
    {
        if(pprf)	printf("adc_offset_data_save_Fail\r\n");	
        file_open_result = 1;				
    }

    if(file_open_result == AUTO_TASK_OK)  
    {
        serial_dev_ack_nack_task(ADC_DATA_SAVE_REQUEST,ADC_DATA_SAVE_REQUEST_LEN,AUTO_TASK_OK);	
        system("sync");		
    }
    else    serial_dev_ack_nack_task(ADC_DATA_SAVE_REQUEST,ADC_DATA_SAVE_REQUEST_LEN,AUTO_TASK_NG);	    
}

void cur_auto_cal_task(void)
{
    auto_cal_cur_task(EX_CAL_AVDD);
    power_off(); 
    cal_end_task();	
    usleep(100000);	
    auto_cal_cur_task(EX_CAL_ELVSS);
    power_off(); 
    cal_end_task();	
    usleep(100000);		
    nvss_cur_offset();
    usleep(100000);		
    pvss_cur_offset();
    usleep(100000);	
    nvdd_cur_offset();
    usleep(100000);	
    pvdd_cur_offset();	
    usleep(100000);	
    serial_dev_ack_nack_task(CUR_AUTO_CAL_REQUEST,CUR_AUTO_CAL_REQUEST_LEN,AUTO_TASK_OK);	
}

void cur_cal_offset_save_task(void)
{
    FILE *vdd_offset_file;
    FILE *vss_offset_file;
    FILE *vdd_adc_file;
    FILE *vss_adc_file;
    unsigned char cur_cal_offset_save_task_result = 0; 
    
    vdd_offset_file = fopen(AVDD_CUR_OFFSET_FILE_PATH, "w+b");
    if(vdd_offset_file!= NULL)
    {
        fwrite(&avdd_cur_offset_cal, sizeof(avdd_cur_offset_cal), 1, vdd_offset_file);
        fclose(vdd_offset_file);
        if(pprf)    printf("avdd_cur_offset_data_save_OK\r\n");
    }	
    else
    {
        if(pprf)    printf("avdd_cur_offset_data_save_Fail\r\n");  
        cur_cal_offset_save_task_result = 1;  
    }    

    vss_offset_file = fopen(ELVSS_CUR_OFFSET_FILE_PATH, "w+b");
    if(vss_offset_file!= NULL)
    {
        fwrite(&elvss_cur_offset_cal, sizeof(elvss_cur_offset_cal), 1, vss_offset_file);
        fclose(vss_offset_file);
        if(pprf)    printf("elvss_cur_offset_data_save_OK\r\n");
    }	        
    else
    {
        if(pprf)    printf("elvss_cur_offset_data_save_Fail\r\n");
        cur_cal_offset_save_task_result = 1;      
    }

    vdd_adc_file = fopen(AVDD_CUR_CAL_FILE_PATH, "w+b");
    if(vdd_adc_file!= NULL)
    {
        fwrite(&avdd_cur_cal, sizeof(avdd_cur_cal), 1, vdd_adc_file);
        fclose(vdd_adc_file);
        if(pprf)    printf("cur_cal_data_save_OK\r\n");
    }
    else
    {
        if(pprf)    printf("cur_cal_data_save_Fail\r\n"); 
        cur_cal_offset_save_task_result = 1;    
    }

    vss_adc_file = fopen(ELVSS_CUR_CAL_FILE_PATH, "w+b");
    if(vss_adc_file!= NULL)
    {
        fwrite(&elvss_cur_cal, sizeof(elvss_cur_cal), 1, vss_adc_file);
        fclose(vss_adc_file);
        if(pprf)    printf("elvss_cur_cal_data_save_OK\r\n");	
    }    
    else
    {
        if(pprf)    printf("elvss_cur_cal_data_save_Fail\r\n");
        cur_cal_offset_save_task_result = 1;  	    
    }  

    if(cur_cal_offset_save_task_result == AUTO_TASK_OK)  
    {
        serial_dev_ack_nack_task(CUR_DATA_SAVE_REQUEST,CUR_DATA_SAVE_REQUEST_LEN,AUTO_TASK_OK);	
        system("sync");		
    }
    else    serial_dev_ack_nack_task(CUR_DATA_SAVE_REQUEST,CUR_DATA_SAVE_REQUEST_LEN,AUTO_TASK_NG);	       			        
}

void res_select_task(unsigned char ch, unsigned char sel)
{
    unsigned char data[100];
    unsigned char vol_cur_select_task_result = 0; 
    unsigned char res_select_result = 0;
    memset(&data,0, sizeof(data));
    vol_cur_select_task_result = vol_cur_select_task(SELECT_CUR);
    if(!vol_cur_select_task_result)
    {  
        data[0] = AUTO_CAL_CUR_TASK;	
        data[1] = ch;	            // AVDD or ELVSS SELECT
        data[2] = sel;            // Resistance SELECT		
        ex_port_send_task(data,AUTO_CAL_CUR_TASK_LEN);	
        ex_port_serial_task();
        if(pprf)    printf("KEITHLEY_DATA = %f\r\n",auto_cal_data_float);	
    }
    if(vol_cur_select_task_result == AUTO_TASK_OK)  
    {
        serial_dev_ack_nack_task(RES_SELECT_REQUEST,RES_SELECT_REQUEST_LEN,AUTO_TASK_OK);	
        system("sync");		
    }
    else    serial_dev_ack_nack_task(RES_SELECT_REQUEST,RES_SELECT_REQUEST_LEN,AUTO_TASK_NG);	       	
}

void measurement_res_select_task(unsigned char index)
{
    unsigned char data[100];
    memset(&data,0, sizeof(data));

    data[0] = EX_REG_TASK;	
    data[1] = index;	
    ex_port_send_task(data,EX_REG_TASK_LEN);	
    if(pprf)    printf("measurement_res_select_OK\r\n");
    usleep(100000);		
    serial_dev_ack_nack_task(MEASUREMENT_RES_REQUEST,MEASUREMENT_RES_REQUEST_LEN,AUTO_TASK_OK);
}

void output_to_keithley_connect_task(unsigned char ch)
{
    unsigned char data[100];
    unsigned vol_cur_select_task_result = 0;
    memset(&data,0, sizeof(data));		
    vol_cur_select_task_result = vol_cur_select_task(SELECT_VOL);
    if(!vol_cur_select_task_result)
    {			
        cal_end_task();
        data[0] = AUTO_CAL_TASK;	
        data[1] = ch;	
        ex_port_send_task(data,AUTO_CAL_TASK_LEN);	
        if(pprf)    printf("output_to_keithley_connect_OK\r\n");	
        ex_port_serial_task();	
        serial_dev_ack_nack_task(KEITHLEY_CONNECTION_REQUEST,KEITHLEY_CONNECTION_REQUEST_LEN,AUTO_TASK_OK);	
    } 
    else
    {
        if(pprf)    printf("output_to_keithley_connect_NG\r\n");	
        serial_dev_ack_nack_task(KEITHLEY_CONNECTION_REQUEST,KEITHLEY_CONNECTION_REQUEST_LEN,AUTO_TASK_NG);	    
    }   
}

void category_task(void)    //231206 Modify    
{
    unsigned char category_start_flag = 0xff;
    static int category_task_index = 0;
    static unsigned char category_count = 0;
    static unsigned char dac_select_ch_save = 0;
    static unsigned char adc_select_ch_save = 0;
    unsigned char ex_auto_cal_task_flag = 0;
    unsigned char adc_cal_task_result = TASK_OK;
    unsigned char dac_cal_task_result = TASK_OK;
    unsigned char cur_cal_task_result = TASK_OK;
    while(1)
    {
        ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
        if(category_start_flag == DAC_ADC_AUTO_START)
        {       
            switch(category_task_index)
            {       
                case CATEGORY_DAC_CAL:
                    if(category.dac_cal != 0)
                    {
                        for(category_count = 0 ; category_count < 14 ; category_count++)
                        {
                            if((category.dac_cal>>category_count)&0x01 != 0)
                            {
                                dac_select_ch_save = category_count;                           
                                //if(es975_state.state_index == CATEGORY_STATE_IDEL) 
                                if(categoty_state[dac_select_ch_save]  == CATEGORY_STATE_IDEL)                             
                                {                                                              
                                    dac_cal_task(dac_select_ch_save);
                                }                                                              
                            } 
                            usleep(10);                
                        } 
                    } 
                    category_task_index = CATEGORY_ADC_CAL;   
                break;
                case    CATEGORY_ADC_CAL:
                    if(category.adc_cal != 0)
                    {
                        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);
                        for(category_count = 0 ; category_count < 32 ; category_count++)
                        {
                            if((category.adc_cal>>category_count)&0x01 != 0)
                            {
                                adc_select_ch_save = category_count;   
                                if(categoty_state[adc_select_ch_save]  == CATEGORY_STATE_IDEL)                             
                                {                                                              
                                    adc_cal_task_result = adc_cal_task(adc_select_ch_save);
                                    if((adc_cal_task_result == TASK_FAIL) ||(adc_cal_task_result == CAL_CANCEL))    
                                    {
                                        power_off();
                                        category_task_index = CATEGORY_RES_IDEA;     
                                    }
                                }                                            
                            }
                        }
                        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);
                    }            
                    category_task_index = CATEGORY_CUR_CAL; 
                break; 
                case    CATEGORY_CUR_CAL:
                    if(category.cur_cal != 0)
                    {
                        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);
                        for(category_count = 0 ; category_count < 2 ; category_count++)
                        {
                            if((category.cur_cal>>category_count)&0x01 != 0)
                            {
                                cur_cal_task_result = cur_cal_task(category_count);   
                                if((cur_cal_task_result == TASK_FAIL) ||(cur_cal_task_result == CAL_CANCEL))  
                                {
                                    power_off();
                                    category_task_index = CATEGORY_RES_IDEA;     
                                }     
                            }                            
                        }
                        ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);
                    }                    
                    category_task_index = CATEGORY_VOL_CHECK; 
                break;
                case    CATEGORY_VOL_CHECK:
                    printf("CATEGORY_VOL_CHECK\r\n");
                    category_task_index = CATEGORY_CUR_CHECK;
                break;
                case    CATEGORY_CUR_CHECK:
                    printf("CATEGORY_CUR_CHECK\r\n");
                    category_task_index = CATEGORY_RES_SELECT;
                break;
                case    CATEGORY_RES_SELECT:
                    printf("CATEGORY_RES_SELECT\r\n");
                    result_ring_q_save_end_task(TASK_END); 
                    category_task_index = CATEGORY_RES_IDEA;   
                    power_off();                 
                break;
                case    CATEGORY_RES_IDEA:
                    power_off(); 
                break;    
            }
           
        }
        else if(category_start_flag == DAC_ADC_AUTO_STOP)
        {
            category_task_index = CATEGORY_DAC_CAL;   
            category_count = 0;
           // result_ring_q_save_end_task(TASK_IDEA);
        }   
        usleep(10);              
    }
}

void dac_cal_task(unsigned char ch)  //231206 Modify   
{
    unsigned char category_start_flag = 0xff;
    unsigned char dac_cal_result = 0;
    unsigned char dac_offset_result = 0;
    unsigned char category_state=0;
    unsigned char dac_cal_save_task_result = TASK_OK;
    unsigned char dac_cal_pass_need_flag = CAL_PASS;
    short *dac_cal_point;
    ES975_STATE dac_cal_taske_state_result;
    
    memset(&dac_cal_taske_state_result, 0, sizeof(dac_cal_taske_state_result)); 
    
    result_ring_q_save_task(ch,CATEGORY_STATE_EXECUTE, dac_cal_taske_state_result);

    dac_cal_point = cal_dac_vol_save_task(PRE_DAC_CAL,ch);  //pre-cal voltage save
    ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
    if(category_start_flag == DAC_ADC_AUTO_STOP)	return;  

    dac_cal_pass_need_flag = pre_dac_cal_vol_check_task(dac_cal_point); // CHECK Need cal or No Need Cal
    if(dac_cal_pass_need_flag == CAL_PASS)
    {
        memcpy(dac_cal_taske_state_result.result_data.data_0,dac_cal_point,sizeof(dac_cal_taske_state_result.result_data.data_0));
        memcpy(dac_cal_taske_state_result.result_data.data_1,dac_cal_point,sizeof(dac_cal_taske_state_result.result_data.data_1));
        result_ring_q_save_task(ch,CATEGORY_STATE_DAC_CAL_RESULT, dac_cal_taske_state_result);                 
    }
    else
    {
        ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
        if(category_start_flag == DAC_ADC_AUTO_STOP)	return;  

        memcpy(dac_cal_taske_state_result.result_data.data_0,dac_cal_point,sizeof(dac_cal_taske_state_result.result_data.data_0));
        usleep(100000);	    
        dac_cal_result = dac_auto_cal(ch);                   //DAC_CAL

        ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
        if(category_start_flag == DAC_ADC_AUTO_STOP)	return; 
        
        if(dac_cal_result == AUTO_CAL_OK)   
        {      
            usleep(100000);
            dac_offset_result = dac_auto_offset_cal(ch);         //DAC_OFFSET

            ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
            if(category_start_flag == DAC_ADC_AUTO_STOP)	return;    

            if(dac_offset_result== AUTO_CAL_OK)
            {
                dac_cal_save_task_result = dac_cal_save_task();
                if(dac_cal_save_task_result == TASK_OK)
                {
                    usleep(100000);
                    dac_cal_point = cal_dac_vol_save_task(AFTER_DAC_CAL,ch);  //cal after voltage save

                    ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
                    if(category_start_flag == DAC_ADC_AUTO_STOP)	return;    

                    memcpy(dac_cal_taske_state_result.result_data.data_1,dac_cal_point,sizeof(dac_cal_taske_state_result.result_data.data_1));
                    result_ring_q_save_task(ch,CATEGORY_STATE_DAC_CAL_RESULT, dac_cal_taske_state_result);  
                }     
                else
                {
                    memcpy(dac_cal_taske_state_result.result_data.data_1,dac_cal_point,sizeof(dac_cal_taske_state_result.result_data.data_1));
                    result_ring_q_save_task(ch,CATEGORY_STATE_DAC_CAL_RESULT, dac_cal_taske_state_result);                  
                }     
            }
            else if(dac_offset_result == KEITHLEY_COM_ERROR)
            { 
                result_ring_q_save_task(ch,CATEGORY_STATE_KEITHLEY_COMMUNICATION_FAIL, dac_cal_taske_state_result);      
            }
            else
            {
                result_ring_q_save_task(ch,CATEGORY_STATE_NG, dac_cal_taske_state_result);                           
            }
        }
        else if(dac_cal_result == KEITHLEY_COM_ERROR)
        {
            result_ring_q_save_task(ch,CATEGORY_STATE_KEITHLEY_COMMUNICATION_FAIL, dac_cal_taske_state_result);       
        }
        else
        {
            result_ring_q_save_task(ch,CATEGORY_STATE_NG, dac_cal_taske_state_result);           
        }
    }
}

short *cal_dac_vol_save_task(unsigned char index,unsigned char outch)  //231206 Modify   
{
    unsigned char category_start_flag = 0xff;
	char ch = 0;
	short a = 0;
	short vol = 0;
	char c[3]  = {0,};
	int size = 0;
	int mode = 0;
	int i = 0;
	char dac_num;
	char dac_ch = 0;
    unsigned char data[100];
    static short temp_data[100];  
    unsigned char vol_cur_select_task_result = 0; 

    memset(&data,0, sizeof(data));
    memset(&temp_data,0, sizeof(temp_data));

    if(outch < 2)	
    {
        dac_num = 0;
        dac_ch = outch;
    }	
    else if(outch <10)
    {	
        dac_num = 1;
        dac_ch = outch - 2;
    }
    else if(outch <14)	
    {
        dac_num =2;
        dac_ch = outch - 10;
    }
    else dac_num = 10;

    for(i = 0 ; i < SET_VOL_N25V+1 ; i++)
    {
        ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
        if(category_start_flag == DAC_ADC_AUTO_STOP)
        {
            memset(&temp_data,0, sizeof(temp_data));
            return  temp_data;
        }

        if(i==SET_VOL_25V)	vol = 25000;
        else if(i==SET_VOL_15V) vol = 15000;
        else if(i==SET_VOL_8V) vol = 8000;		
        else if(i==SET_VOL_5V) vol = 5000;
        else if(i==SET_VOL_2_5V) vol = 2500;	
        else if(i==SET_VOL_N2_5V) vol = -2500;
        else if(i==SET_VOL_N5V) vol = -5000;		
        else if(i==SET_VOL_N8V) vol = -8000;
        else if(i==SET_VOL_N15V) vol = -15000;	       
        else if(i==SET_VOL_N25V) vol = -25000;	   

        signal_group.signal_config.dc_voltage[outch] = vol;	
        Power_Supply_Voltage_load();
        if(index == PRE_DAC_CAL)    // PRE DAC CAL VOL
        {
            if((dac_cal[outch].dac_0to10_ratio == 0) || (dac_cal[outch].dac_10to25_ratio == 0) || (dac_cal[outch].dac_n10ton25_ratio == 0) || (dac_cal[outch].dac_0ton10_ratio == 0))
            {
                pattern_generator->PG_CONTROL = OUT_EN_ON;
                a = (short)(((65535/10)*(((vol/1000)/5.3)+5)));			
                c[2] = a &0xff;
                c[1] = (a>>8) &0xff;
                c[0] = 20+dac_ch;	
                dac_set(dac_num,c,3);
                memset(&dac_cal[outch], 0, sizeof(dac_cal[outch])); 
            }
            else    
            {
                SET_DAC_OUTPUT_VALUE(outch);    
            }
        }	
        else    SET_DAC_OUTPUT_VALUE(outch);   // DAC CAL AFTER VOL 
        gpio_enble->GPIO_DATA |= (0x00000001<<(outch&0x3fff));
        if(outch ==ELVDD)	gpiops_set_value(RELAY_ELVDD,RELAY_ON);
        else if(outch ==ADD8_S)	gpiops_set_value(RELAY_VDD8_S,RELAY_ON);
        else if(outch ==ADD8_G)	gpiops_set_value(RELAY_VDD8_G,RELAY_ON);
        else if(outch ==VGH)	gpiops_set_value(RELAY_VGH,RELAY_ON);
        else if(outch ==VGL)	gpiops_set_value(RELAY_VGL,RELAY_ON);	
        else if(outch ==VINIT)	gpiops_set_value(RELAY_VINIT,RELAY_ON);	

        data[0] = AUTO_CAL_TASK;	
        data[1] = outch;
        ex_port_send_task(data,AUTO_CAL_TASK_LEN);	     
        ex_port_serial_task(); 
        //ex_recive_data_delay_task(1000);

        temp_data[i] = auto_cal_data/1000;  // DAC CAL AFTER VOL  
        usleep(200000);  		  
    }
    return temp_data;
}

void result_ring_q_save_task(unsigned char ch, unsigned char index, ES975_STATE ring_q)  //231206 Modify   
{
    ring_q.state_index = index;
    ring_q.ch_index = ch;
    if(ring_q_push(queue_cal_result, (char *)&ring_q, sizeof(ring_q))<0)
    {
        printf("%s:%d ->queue_cal_result queue error!\n",__func__,__LINE__);
    }  
}

void result_ring_q_save_end_task(unsigned char index)  //231206 Modify   
{
    ES975_STATE ring_q_end; 
    memset(&ring_q_end, 0, sizeof(ring_q_end)); 
    if(index == TASK_END)   ring_q_end.state_index = CATEGORY_STATE_ALLEND;
    else    ring_q_end.state_index = CATEGORY_STATE_IDEL; 
    ring_q_end.ch_index = 0;
    if(ring_q_push(queue_cal_result, (char *)&ring_q_end, sizeof(ring_q_end))<0)
    {
        printf("%s:%d ->queue_cal_result queue error!\n",__func__,__LINE__);
    }  
}

unsigned char ex_recive_data_delay_task(int time)
{
    int res_adc_start = 0;	
    int res_adc_adc_auto_end = 0;	
    int res_adc_adc_auto_time = time;	
    unsigned char ex_recive_data_delay_task_index = 0;
    unsigned char ex_recive_data_delay_task_state=0; 

    res_adc_start = timeout_msclock();	
    while(1)
    {
        if(ex_recive_data_delay_task_state == EX_REVICE_DELAY_OK)
        {
            ex_recive_data_delay_task_index = 0;
            break;
        }						
        res_adc_adc_auto_end = timeout_msclock() - res_adc_start;	
        if(res_adc_adc_auto_end >= res_adc_adc_auto_time) 
        {
            ex_recive_data_delay_task_index = 1;
            break;	
        }	
        usleep(10);				
    } 
    return  ex_recive_data_delay_task_index;    
}

unsigned char dac_cal_save_task(void)
{
    FILE *dac_file;
    unsigned char dac_cal_save_task_result = TASK_OK;

    dac_file = fopen(DAC_CAL_FILE_PATH, "w+b");
    if(dac_file != NULL)
    {
        fwrite(&dac_cal, sizeof(dac_cal), 1, dac_file);
        fclose(dac_file);
        system("sync");	
        LOGE("dac_data_save_OK\r\n");	
    } 
    else    dac_cal_save_task_result = TASK_FAIL; 
    return  dac_cal_save_task_result;    
}

unsigned char pre_dac_cal_vol_check_task(short *vol)
{   
    unsigned char vol_check_flag = CAL_PASS;
    if(abs(vol[SET_VOL_25V] - 25000) > 10)    vol_check_flag = CAL_NEED;   
    if(abs(vol[SET_VOL_15V] - 15000) > 10)    vol_check_flag = CAL_NEED;   
    if(abs(vol[SET_VOL_8V] - 8000) > 10)    vol_check_flag = CAL_NEED; 
    if(abs(vol[SET_VOL_5V] - 5000) > 10)    vol_check_flag = CAL_NEED; 
    if(abs(vol[SET_VOL_2_5V] - 2500) > 10)    vol_check_flag = CAL_NEED; 
    if(abs(vol[SET_VOL_N2_5V] + 2500) > 10)    vol_check_flag = CAL_NEED; 
    if(abs(vol[SET_VOL_N5V] + 5000) > 10)    vol_check_flag = CAL_NEED; 
    if(abs(vol[SET_VOL_N8V] + 8000) > 10)    vol_check_flag = CAL_NEED; 
    if(abs(vol[SET_VOL_N15V] + 15000) > 10)    vol_check_flag = CAL_NEED; 
    if(abs(vol[SET_VOL_N25V] + 25000) > 10)    vol_check_flag = CAL_NEED; 

    return  vol_check_flag;     
}
unsigned char adc_cal_task(unsigned char ch)  //231206 Modify   
{
    unsigned char category_start_flag = 0xff;
    unsigned char adc_cal_result = 0;
    unsigned char adc_offset_result = 0;
    unsigned char category_state=0;
    short *adc_cal_point;
    unsigned char vol_cur_select_task_result = 0; 
    unsigned char adc_cal_task_result = TASK_OK;
    ES975_STATE adc_cal_task_state_result;

    vol_cur_select_task_result = vol_cur_select_task(SELECT_VOL);
    ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
    if(category_start_flag == DAC_ADC_AUTO_STOP)
    {   
        result_ring_q_save_task(ch,CATEGORY_STATE_NG, adc_cal_task_state_result);    
        return  adc_cal_task_result == CAL_CANCEL;  
    }
    if(!vol_cur_select_task_result)
	{	
        memset(&adc_cal_task_state_result, 0, sizeof(adc_cal_task_state_result)); 

        result_ring_q_save_task(ch,CATEGORY_STATE_EXECUTE, adc_cal_task_state_result);
        if(pre_adc_cal_vol_check_task(PRE_ADC_CAL,ch) == CAL_NEED)
        {
            if((category.adc_cal == ADC_ALL_VOL_CAL) && (ch == EX_CAL_AVDD))  
            {
                memset(&vol_offset, 0, sizeof(vol_offset)); 
                memset(&ads124_cal0, 0, sizeof(ads124_cal0));
                memset(&ads124_cal1, 0, sizeof(ads124_cal1));
                memset(&ads124_cal2, 0, sizeof(ads124_cal2));
                memset(&ads124_cal3, 0, sizeof(ads124_cal3));
                memset(&ads124_cal4, 0, sizeof(ads124_cal4));	                
                adc_cal_result = ex_auto_cal_task();  
                if(adc_cal_result == CAL_CANCEL)    
                {
                    adc_cal_task_result =   CAL_CANCEL; 
                    return adc_cal_task_result; 
                }
                else if(adc_cal_result == KEITHLEY_COM_ERROR)  
                {
                    result_ring_q_save_task(ch,CATEGORY_STATE_KEITHLEY_COMMUNICATION_FAIL, adc_cal_task_state_result);   
                    adc_cal_task_result = TASK_FAIL;                      
                }
            }
        }  
        adc_offset_result = adc_offset_task(ch);      
        if(!adc_offset_result)
        {
            pre_adc_cal_vol_check_task(AFTER_ADC_CAL, ch);   
            adc_cal_offset_data_save_task();	
            adc_cal_task_result = TASK_OK; 
        }
        else if(adc_offset_result == KEITHLEY_COM_ERROR)
        {
            result_ring_q_save_task(ch,CATEGORY_STATE_KEITHLEY_COMMUNICATION_FAIL, adc_cal_task_state_result);   
            adc_cal_task_result = TASK_FAIL;     
        }
        else
        {
            result_ring_q_save_task(ch,CATEGORY_STATE_NG, adc_cal_task_state_result);
            adc_cal_task_result = TASK_FAIL;                            
        }        
    }
    else
    {
        result_ring_q_save_task(ch,CATEGORY_STATE_KEITHLEY_COMMUNICATION_FAIL, adc_cal_task_state_result);      
        adc_cal_task_result = TASK_FAIL;    
    }

    if(category_start_flag == DAC_ADC_AUTO_STOP)
    {   
        result_ring_q_save_task(ch,CATEGORY_STATE_NG, adc_cal_task_state_result);    
        return  adc_cal_task_result == CAL_CANCEL;  
    }    
    return adc_cal_task_result;
}

unsigned char pre_adc_cal_vol_check_task(unsigned char index,unsigned char outch) //231206 Modify  
{
    unsigned char category_start_flag = 0xff;
	char ch = 0;
	short a = 0;
	short vol = 0;
	char c[3]  = {0,};
	int size = 0;
	int mode = 0;
	int i = 0;
	char dac_num;
	char dac_ch = 0;
    unsigned char data[100];
    unsigned char ch_index = 0;
    unsigned char cal_pass_or_need_result = CAL_PASS;
    ES975_STATE pre_adc_cal_vol_check_task_result;

    ch_index = auto_cal_data_compare_sensing_select_task(outch);
    memset(&data,0, sizeof(data));
    memset(&pre_adc_cal_vol_check_task_result,0, sizeof(pre_adc_cal_vol_check_task_result));

    if(outch < 2)	
    {
        dac_num = 0;
        dac_ch = outch;
    }	
    else if(outch <10)
    {	
        dac_num = 1;
        dac_ch = outch - 2;
    }
    else if(outch <14)	
    {
        dac_num =2;
        dac_ch = outch - 10;
    }
    else dac_num = 10;

    for(i = 0 ; i < SET_VOL_N25V+1 ; i++)
    {
        ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
        if(category_start_flag == DAC_ADC_AUTO_STOP)    return  cal_pass_or_need_result = CAL_CANCEL;

        if(i==SET_VOL_25V)	vol = 25000;
        else if(i==SET_VOL_15V) vol = 15000;
        else if(i==SET_VOL_8V) vol = 8000;		
        else if(i==SET_VOL_5V) vol = 5000;
        else if(i==SET_VOL_2_5V) vol = 2500;	
        else if(i==SET_VOL_N2_5V) vol = -2500;
        else if(i==SET_VOL_N5V) vol = -5000;		
        else if(i==SET_VOL_N8V) vol = -8000;
        else if(i==SET_VOL_N15V) vol = -15000;	       
        else if(i==SET_VOL_N25V) vol = -25000;	   

        signal_group.signal_config.dc_voltage[outch] = vol;	
        Power_Supply_Voltage_load();
        SET_DAC_OUTPUT_VALUE(outch);   // DAC CAL AFTER VOL 
        gpio_enble->GPIO_DATA |= (0x00000001<<(outch&0x3fff));
        if(outch ==ELVDD)	gpiops_set_value(RELAY_ELVDD,RELAY_ON);
        else if(outch ==ADD8_S)	gpiops_set_value(RELAY_VDD8_S,RELAY_ON);
        else if(outch ==ADD8_G)	gpiops_set_value(RELAY_VDD8_G,RELAY_ON);
        else if(outch ==VGH)	gpiops_set_value(RELAY_VGH,RELAY_ON);
        else if(outch ==VGL)	gpiops_set_value(RELAY_VGL,RELAY_ON);	
        else if(outch ==VINIT)	gpiops_set_value(RELAY_VINIT,RELAY_ON);	

        data[0] = AUTO_CAL_TASK;	
        data[1] = outch;
        ex_port_send_task(data,AUTO_CAL_TASK_LEN);	     
        ex_port_serial_task(); 
        usleep(200000);         	
        ADC_SELECT_DATA_READ_AVG(ch_index);	 
        pre_adc_cal_vol_check_task_result.result_data.data_0[i]  = auto_cal_data/1000; 
        pre_adc_cal_vol_check_task_result.result_data.data_1[i]  = adc_sensing_value[ch_index];
    }
    
    if(index == PRE_ADC_CAL)
    {
        for(i = 0 ; i < SET_VOL_N25V+1 ; i++)
        {
            if(abs(pre_adc_cal_vol_check_task_result.result_data.data_0[i] - pre_adc_cal_vol_check_task_result.result_data.data_1[i]) > 10)  cal_pass_or_need_result |= CAL_NEED;   
        }

        if(cal_pass_or_need_result == CAL_PASS)
        {
            result_ring_q_save_task(outch,CATEGORY_STATE_ADC_CAL_RESULT, pre_adc_cal_vol_check_task_result);         
        }
    }
    else    result_ring_q_save_task(outch,CATEGORY_STATE_ADC_CAL_RESULT, pre_adc_cal_vol_check_task_result); 

    ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
    if(category_start_flag == DAC_ADC_AUTO_STOP)    return  cal_pass_or_need_result = CAL_CANCEL;
    
    return cal_pass_or_need_result;
}

unsigned char cur_cal_task(unsigned char ch)  //231206 Modify   
{
    unsigned char category_start_flag = 0xff;
    unsigned char cur_cal_result = 0;
    unsigned char cur_offset_result = 0;
    unsigned char category_state=0;
    short *cur_cal_point;
    unsigned char cur_cal_task_result = TASK_OK;
    ES975_STATE cur_cal_task_state_result;

    ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);
    memset(&cur_cal_task_state_result, 0, sizeof(cur_cal_task_state_result));   
    result_ring_q_save_task(ch,CATEGORY_STATE_EXECUTE, cur_cal_task_state_result);     
    cur_cal_task_result = auto_cal_cur_task(ch);        //CUR_CAL   

    ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
    if(category_start_flag == DAC_ADC_AUTO_STOP)    return  cur_cal_task_result = CAL_CANCEL;      

    if(cur_cal_task_result == TASK_OK)
    {
        if(ch == EX_CAL_AVDD)   
        {
            pvdd_cur_offset();          //AVDD P_VOL_CUR_OFFSET	    
            usleep(100000);
            nvdd_cur_offset();          //AVDD N_VOL_CUR_OFFSET	           
        }
        else
        {
            pvss_cur_offset();          //ELVSS P_VOL_CUR_OFFSET	      
            usleep(100000);
            nvss_cur_offset();          //ELVSS N_VOL_CUR_OFFSET
        }           
        cur_cal_check(ch);          
    }
    ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);       
    ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
    if(category_start_flag == DAC_ADC_AUTO_STOP)    return  cur_cal_task_result = CAL_CANCEL;    
    return cur_cal_task_result;   
}

void cur_cal_check(unsigned char ch)
{
    unsigned char category_start_flag = 0xff;
    unsigned char vol_cur_select_task_result = 0; 
    unsigned char data[100];
    unsigned char ch_index = 0;
    float cur_sensing_data = 0;;
    double cur_data = 0;
    ES975_STATE adc_cal_cur_check_task_result;
    memset(&data,0, sizeof(data));
    memset(&adc_cal_cur_check_task_result,0, sizeof(adc_cal_cur_check_task_result));
    ch_index = ch; 
    if(ch_index == EX_CAL_AVDD) cur_sensing_data = avdd_cur_sensing_data;
    else    cur_sensing_data = elvss_cur_sensing_data;   

    ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
    if(category_start_flag == DAC_ADC_AUTO_STOP)    return; 
    cur_sensing_compare_task(ch_index, SELECT_25OHM, 4000, 1);	//AVDD : 4V / 0.16A
    adc_cal_cur_check_task_result.result_data.data_0[0] = (double)auto_cal_data/1000;
    adc_cal_cur_check_task_result.result_data.data_1[0] = cur_sensing_data;    
    usleep(100000);		
    cur_sensing_compare_task(ch_index, SELECT_100OHM, 8000, 1);	//AVDD : 8V / 0.08A
    adc_cal_cur_check_task_result.result_data.data_0[1] = (double)auto_cal_data/1000;
    adc_cal_cur_check_task_result.result_data.data_1[1] = cur_sensing_data;    
    usleep(100000);
    cur_sensing_compare_task(ch_index, SELECT_200OHM, 8000, 1);	//AVDD : 8V / 0.04A
    adc_cal_cur_check_task_result.result_data.data_0[2] = (double)auto_cal_data/1000;
    adc_cal_cur_check_task_result.result_data.data_1[2] = cur_sensing_data;    
    usleep(100000);
    ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
    if(category_start_flag == DAC_ADC_AUTO_STOP)    return;     
    cur_sensing_compare_task(ch_index, SELECT_400OHM, 8000, 1);	//AVDD : 8V / 0.02A
    adc_cal_cur_check_task_result.result_data.data_0[3] = (double)auto_cal_data/1000;
    adc_cal_cur_check_task_result.result_data.data_1[3] = cur_sensing_data;    
    usleep(100000);
    cur_sensing_compare_task(ch_index, SELECT_25OHM, 1, -2500);	//ELVSS : -2.5V / -0.1A
    adc_cal_cur_check_task_result.result_data.data_0[4] = (double)auto_cal_data/1000;
    adc_cal_cur_check_task_result.result_data.data_1[4] = cur_sensing_data;    
    usleep(100000);
    ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
    if(category_start_flag == DAC_ADC_AUTO_STOP)    return;     
    cur_sensing_compare_task(ch_index, SELECT_100OHM, 1, -5000);	//ELVSS : -5V / -0.05A
    adc_cal_cur_check_task_result.result_data.data_0[5] = (double)auto_cal_data/1000;
    adc_cal_cur_check_task_result.result_data.data_1[5] = cur_sensing_data;    
    usleep(100000);	
    cur_sensing_compare_task(ch_index, SELECT_200OHM, 1, -2500);	//ELVSS : -2.5V / -0.0125A
    adc_cal_cur_check_task_result.result_data.data_0[6] = (double)auto_cal_data/1000;
    adc_cal_cur_check_task_result.result_data.data_1[6] = cur_sensing_data;    
    usleep(100000);		
    cur_sensing_compare_task(ch_index, SELECT_400OHM, 1, -2500);	//ELVSS : -2.5V / -0.00625A
    adc_cal_cur_check_task_result.result_data.data_0[7] = (double)auto_cal_data/1000;
    adc_cal_cur_check_task_result.result_data.data_1[7] = cur_sensing_data;    
    usleep(100000);		
    ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
    if(category_start_flag == DAC_ADC_AUTO_STOP)    return;     
    cur_sensing_compare_task(ch_index, SELECT_400OHM, 1, -1200);	//ELVSS : -1.2V / -0.003A
    adc_cal_cur_check_task_result.result_data.data_0[8] = (double)auto_cal_data/1000;
    adc_cal_cur_check_task_result.result_data.data_1[8] = cur_sensing_data;    
    usleep(100000);
    cur_sensing_compare_task(ch_index, SELECT_400OHM, 1, -500);	//ELVSS : -0.5V / -0.00125A
    adc_cal_cur_check_task_result.result_data.data_0[9] = (double)auto_cal_data/1000;
    adc_cal_cur_check_task_result.result_data.data_1[9] = cur_sensing_data;    
    usleep(100000);
    result_ring_q_save_task(ch,CATEGORY_STATE_CUR_CAL_RESULT, adc_cal_cur_check_task_result);  	
}
