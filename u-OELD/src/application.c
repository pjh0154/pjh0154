#include "../include/application.h"
#include "../include/pca9665.h"
#include "../include/gpio-dev.h"
#include "../exprtk_library/exprtk.h"
//#include <string.h>
#include <ctype.h>

extern pthread_mutex_t mutex_lock;
extern int		pthread_end;
extern int cprf;
extern int eprf;
extern int aprf;

extern void ensis_delay(int time);

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
    };

    const char *cmd_list[MAX_CMD_COUNT] = {"AVDD","ELVSS","ELVDD","ADD8_S","ADD8_G","VGH","VGL","VINIT","ASPARE1","ASPARE2","VDD11","VDD18","DSPARE1","DSPARE2","LOGIC",
    "I2C","RSTB","TM","BISTEN","LPSPARE1","DELAY","fun","i2cset", "OFF", "SEN","OCP","PAT","OTP","I2CFRE"};
    
    if(system("ls /f0/recipe/recipe.txt"))
    {
       fopen(RECIPE_FILE_PATH,"wb");                             
    }
    else
    {
        recipe_file = fopen(RECIPE_FILE_PATH, "r+");
        usleep(100);
    }   
    memset(&srecipe,0,sizeof(srecipe));
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
            else if(strncasecmp(func_name,"KEY",3)==0)
            {
                key_count_flag++;
                if(key_count_flag>1)    srecipe.key_count++;
                memcpy(srecipe.recipe_key[srecipe.key_count].name, func_name,strlen(func_name));
                func_flag=RCP_FLAG_KEY;
                if(cprf) printf("key_name = %s\r\n", func_name);
            }
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
                if(strstr(strline,"="))
                {
                    cmd_name =  strtok(strline, "=");
                    parm = strtok(NULL, " ");
                    parm_cnt = strlen(parm);   
                    for(int i =0 ; i <29 ; i++)
                    {
                        if(!strcasecmp(strline,cmd_list[i]))
                        {
                            cmd_code=i;	
                            break;
                        }    
                    }
                    if(cmd_code >=0)
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

                            case    RCP_FLAG_KEY:
                                srecipe.recipe_key[srecipe.key_count].cmd[srecipe.recipe_key[srecipe.key_count].cmd_cnt].code = cmd_code;
                                memcpy(srecipe.recipe_key[srecipe.key_count].cmd[srecipe.recipe_key[srecipe.key_count].cmd_cnt].parm, parm, parm_cnt); 
                                srecipe.recipe_key[srecipe.key_count].cmd_cnt++;
                                break;           
                        }
                        //printf("FUNC CNT = %d\r\n", srecipe.recipe_func.func_cnt);
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

                        case    RCP_FLAG_KEY:
                            srecipe.recipe_key[srecipe.key_count].cmd[srecipe.recipe_key[srecipe.key_count].cmd_cnt].code = cmd_code;
                            memcpy(srecipe.recipe_key[srecipe.key_count].cmd[srecipe.recipe_key[srecipe.key_count].cmd_cnt].parm, parm, parm_cnt);
                             if(cprf) printf("KEY_DATA = %d / %s\r\n", srecipe.recipe_key[srecipe.key_count].cmd[srecipe.recipe_key[srecipe.key_count].cmd_cnt].code,srecipe.recipe_key[srecipe.key_count].cmd[srecipe.recipe_key[srecipe.key_count].cmd_cnt].parm); 
                            srecipe.recipe_key[srecipe.key_count].cmd_cnt++;
                            break;           
                    }                          
                }
            }
        }    
    }
 }

//void recipe_funtion_load(void)
void recipe_funtion_load(char* func)
 {
    int i = 0;
    int j = 0;
    int cnt = 0;
    char *cmd_name;
    char *parm;
    int parm_data = 0; 
    int ii = 0;  
    short dc_voltage[20] = {0,};
            
    Power_Supply_Voltage_load(); 
    memset(&signal_group.signal_config->dc_voltage[0], 0, 40);  
    for(i = 0 ; i<= srecipe.recipe_func.func_cnt ; i++)
    {
        if(cprf) printf("func[%d].name = %s / func = %s\r\n", i, srecipe.recipe_func.func[i].name, func);
        if(strcasecmp(srecipe.recipe_func.func[i].name,func)==0)
        {
            for(j = 0 ; j<srecipe.recipe_func.func[i].cmd_cnt ; j++)
            {
                if(srecipe.recipe_func.func[i].cmd[j].code < 14)
                {
                    if(strncasecmp(srecipe.recipe_func.func[i].cmd[j].parm,"ON",2)==0) 
                    {
                          
                        //SET_DAC_OUTPUT_VALUE(srecipe.recipe_func.func[i].cmd[j].code);
                        gpio_enble->GPIO_DATA |= (0x00000001<<(srecipe.recipe_func.func[i].cmd[j].code&0x3fff));
                        if(srecipe.recipe_func.func[i].cmd[j].code ==ELVDD)	gpiops_set_value(RELAY_ELVDD,RELAY_ON);
                        else if(srecipe.recipe_func.func[i].cmd[j].code ==ADD8_S)	gpiops_set_value(RELAY_VDD8_S,RELAY_ON);
                        else if(srecipe.recipe_func.func[i].cmd[j].code ==ADD8_G)	gpiops_set_value(RELAY_VDD8_G,RELAY_ON);
                        else if(srecipe.recipe_func.func[i].cmd[j].code ==VGH)	gpiops_set_value(RELAY_VGH,RELAY_ON);
                        else if(srecipe.recipe_func.func[i].cmd[j].code ==VGL)	gpiops_set_value(RELAY_VGL,RELAY_ON);	
                        else if(srecipe.recipe_func.func[i].cmd[j].code ==VINIT)	gpiops_set_value(RELAY_VINIT,RELAY_ON);	                       
                        if(cprf) printf("signal_out[%d] = %d\r\n", j, srecipe.recipe_func.func[i].cmd[j].code);   
                    }
                    else
                    {
                        signal_group.signal_config->dc_voltage[srecipe.recipe_func.func[i].cmd[j].code] = atoi(srecipe.recipe_func.func[i].cmd[j].parm);
                        dc_voltage[srecipe.recipe_func.func[i].cmd[j].code] = atoi(srecipe.recipe_func.func[i].cmd[j].parm); 
                        if(signal_group.signal_config->dc_voltage[srecipe.recipe_func.func[i].cmd[j].code] >= 0 )
                        //if(dc_voltage[srecipe.recipe_func.func[i].cmd[j].code] >= 0 )
                        {
                            if(signal_group.signal_config->dc_voltage[srecipe.recipe_func.func[i].cmd[j].code] > signal_group.signal_config->dc_voltage[srecipe.recipe_func.func[i].cmd[j-1].code]) 
                            //if(dc_voltage[srecipe.recipe_func.func[i].cmd[j].code] > dc_voltage[srecipe.recipe_func.func[i].cmd[j-1].code]) 
                            {
                                //Power_Supply_Voltage_load_pjh(dc_voltage);                
                                Power_Supply_Voltage_load();     
                            }   
                        }
                        else
                        {
                            if(signal_group.signal_config->dc_voltage[srecipe.recipe_func.func[i].cmd[j].code] < signal_group.signal_config->dc_voltage[srecipe.recipe_func.func[i].cmd[j-1].code]) 
                            {
                                                               
                               Power_Supply_Voltage_load();     
                            } 
                        } 
                            if(cprf) printf("signal[%d] = %d\r\n", j,signal_group.signal_config->dc_voltage[srecipe.recipe_func.func[i].cmd[j].code]);   
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
                        signal_group.signal_config->dc_voltage[LOGIC] = atoi(srecipe.recipe_func.func[i].cmd[j].parm);
                        dac_set_for_logic();
                        if(cprf) printf("LOGIC_VOLTAGE_SET = %d\r\n", signal_group.signal_config->dc_voltage[LOGIC]);
                    } 
                    else if(srecipe.recipe_func.func[i].cmd[j].code == I2C)
                    {
                        signal_group.signal_config->dc_voltage[I2C] = atoi(srecipe.recipe_func.func[i].cmd[j].parm);
                        dac_set_for_i2c();
                        if(cprf) printf("I2C_VOLTAGE_SET = %d\r\n", signal_group.signal_config->dc_voltage[I2C]);                        
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
                        power_off();                  
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
                        int frequency = 0;
                        frequency = atoi(srecipe.recipe_func.func[i].cmd[j].parm);
                        if(frequency > 1190000)  frequency = 1100000; 
                        i2c_set_frequency(frequency);   

                    }                                                                                                                                                                
                }
            }
        } 
    }      
 }

 void recipe_key_load(char* func)
 {
    int i = 0;
    int j = 0;
    int cnt = 0;
    char *cmd_name;
    char *parm;
    int parm_data = 0;   
     
    Power_Supply_Voltage_load(); 
    for(i = 0 ; i<= srecipe.key_count ; i++)
    {
        if(strcasecmp(srecipe.recipe_key[i].name,func)==0)
        {
            for(j = 0 ; j<srecipe.recipe_key[i].cmd_cnt ; j++)
            {
                if(srecipe.recipe_key[i].cmd[j].code < 14)
                {
                    if(strncasecmp(srecipe.recipe_key[i].cmd[j].parm,"ON",2)==0) 
                    {
                        SET_DAC_OUTPUT_VALUE(srecipe.recipe_key[i].cmd[j].code);
                         if(cprf) printf("signal_out[%d] = %d\r\n", j, srecipe.recipe_key[i].cmd[j].code);   
                    }
                    else
                    {
                        signal_group.signal_config->dc_voltage[srecipe.recipe_key[i].cmd[j].code] = atoi(srecipe.recipe_key[i].cmd[j].parm);  
                         if(cprf) printf("signal[%d] = %d\r\n", j,signal_group.signal_config->dc_voltage[srecipe.recipe_key[i].cmd[j].code]);                  
                    }                    
                }
                else
                {
                    if(srecipe.recipe_key[i].cmd[j].code == DELAY)
                    {
                       //ensis_delay(atoi(srecipe.recipe_func.func[i].cmd[j].parm));
                        if(cprf) printf("delay = %d\r\n",atoi(srecipe.recipe_key[i].cmd[j].parm));
                       ensis_delay(atoi(srecipe.recipe_key[i].cmd[j].parm));      
                    }
                    else if(srecipe.recipe_key[i].cmd[j].code == LOGIC)
                    {
                        signal_group.signal_config->dc_voltage[LOGIC] = atoi(srecipe.recipe_key[i].cmd[j].parm);
                        dac_set_for_logic();
                        if(cprf) printf("LOGIC_VOLTAGE_SET = %d\r\n", signal_group.signal_config->dc_voltage[LOGIC]);
                    } 
                    else if(srecipe.recipe_key[i].cmd[j].code == I2C)
                    {
                        signal_group.signal_config->dc_voltage[I2C] = atoi(srecipe.recipe_key[i].cmd[j].parm);
                        dac_set_for_i2c();
                        if(cprf) printf("I2C_VOLTAGE_SET = %d\r\n", signal_group.signal_config->dc_voltage[I2C]);                        
                    }
                    else if(srecipe.recipe_key[i].cmd[j].code == OCP_ON_OFF)
                    {
                        if(strncasecmp(srecipe.recipe_key[i].cmd[j].parm,"ON",2)==0) 
                        {
                            ocp_flag_on = 1;   
                        }
                        else if(strncasecmp(srecipe.recipe_key[i].cmd[j].parm,"OFF",3)==0)
                        {
                            ocp_flag_on = 0; 
                        }
                    }                    
                    else if((srecipe.recipe_key[i].cmd[j].code == RSTB)||(srecipe.recipe_key[i].cmd[j].code == TM) || (srecipe.recipe_key[i].cmd[j].code == BISTEN)|| (srecipe.recipe_key[i].cmd[j].code == LPSPARE1))
                    {
                        if(srecipe.recipe_key[i].cmd[j].code == RSTB) logic_gpio_ctl("RSTB",atoi(srecipe.recipe_key[i].cmd[j].parm));   
                        else if(srecipe.recipe_key[i].cmd[j].code == TM)  logic_gpio_ctl("TM",atoi(srecipe.recipe_key[i].cmd[j].parm)); 
                        else if(srecipe.recipe_key[i].cmd[j].code == BISTEN)  logic_gpio_ctl("BISTEN",atoi(srecipe.recipe_key[i].cmd[j].parm)); 
                        else if(srecipe.recipe_key[i].cmd[j].code == LPSPARE1)  logic_gpio_ctl("LPSPARE1",atoi(srecipe.recipe_key[i].cmd[j].parm));                           
                    }                
                    else if(srecipe.recipe_key[i].cmd[j].code == OFF)
                    {
                        power_off();
                    }   
                    else if(srecipe.recipe_key[i].cmd[j].code == SEN) 
                    {
                        memset(parm_copy, 0, sizeof(parm_copy));
                        memcpy(parm_copy,srecipe.recipe_key[i].cmd[j].parm,strlen(srecipe.recipe_key[i].cmd[j].parm));
                        cmd_name =  strtok(parm_copy, ",");
                        parm = strtok(NULL, " ");
                        parm_data = atoi(parm);
                        if(!(adc_sen_monitoring(cmd_name,(float)parm_data)))    
                        {   
                            j = srecipe.recipe_key[i].cmd_cnt;
                            power_off();
                        }                  
                    } 
                    else if(srecipe.recipe_key[i].cmd[j].code == PAT) 
                    {
                        memset(parm_copy, 0, sizeof(parm_copy));
                        memcpy(parm_copy,srecipe.recipe_key[i].cmd[j].parm,strlen(srecipe.recipe_key[i].cmd[j].parm));
                        cmd_name =  strtok(parm_copy, ",");
                        parm = strtok(NULL, " ");
                        parm_data = atoi(parm);
                        i2c_com_bist_pattern(cmd_name,parm_data,FULL_COLOR_MODE1);                         
                    } 
                    else if(srecipe.recipe_key[i].cmd[j].code == OTP) 
                    {
                        i2c_com_otp_loading();
                    }  
                    else if(srecipe.recipe_key[i].cmd[j].code == I2CFRE)
                    {  
                        int frequency = 0;
                        frequency = atoi(srecipe.recipe_key[i].cmd[j].parm);
                        if(frequency <1190000)  frequency = 1100000; 
                        if(cprf) printf("frequency = %d\r\n",frequency );
                        i2c_set_frequency(frequency);                      
                    }                                                                                                                            
                }
            }
        }
    }      
 }

void power_off(void)
 {
    relay_init();
    mux_init();
    pattern_generator->PG_CONTROL = DAC_nCLR_LOW; 
    usleep(1);                       
    pattern_generator->PG_CONTROL = DAC_nCLR_HIGH;	
    usleep(1);	
    pattern_generator->PG_CONTROL = DAC_nCLR_LOW;                       
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
 }

void i2c_com_otp_loading(void)
 {
    unsigned char slave_address = 0xa0>>1;
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
    unsigned char slave_address = 0xa0>>1;
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
       /*write_buffer[0] = (data & 0x0f);
        write_buffer[0] = write_buffer[0] << 4;
        write_buffer[0] = ((write_buffer[0]) | mode);

        write_buffer[1] = index_num<<4;
        write_buffer[1] = (write_buffer[1]) | ((data >> 4) & 0x0f);

        write_buffer[2] = 0x00;
        write_buffer[3] = 0x00;*/

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
            printf("reg_address = %x / write_buffer[0] = %x / write_buffer[1] = %x / write_buffer[2] = %x / write_buffer[3] = %x\r\n",  reg_address, write_buffer[0], write_buffer[1],  write_buffer[2], write_buffer[3]);
        }
        
        if(0 <= i2c_write(slave_address,reg_address,write_buffer,write_byte))
        {
            if(cprf)    printf("BIST_PATTERN__WRITE_OK\r\n");
            uart_ack();
        }
        else if(cprf)   printf("BIST_PATTERN_WRITE_NG\r\n");
    }
    else    if(eprf)    printf("bist_pattern index not found = [%c]\r\n", index); 
 }

void i2c_com_gma_block(unsigned short reg_address_ex, unsigned short* data)
{
    unsigned char slave_address = 0xa0>>1; 
    unsigned short reg_address = reg_address_ex;
    unsigned char write_buffer[4] = {0};    
    short index_num = 0;  
    int write_byte = 4;    
    
    write_buffer[0] = MAN_GMA_EN | (((data[0]&MAN_R_G_B_9BIT_10BIT)>>8)<<4) | (((data[1]&MAN_R_G_B_9BIT_10BIT)>>8)<<2) | (((data[2]&MAN_R_G_B_9BIT_10BIT)>>8));
    write_buffer[1] = data[0]&0xff;
    write_buffer[2] = data[1]&0xff;
    write_buffer[3] = data[2]&0xff;
    
    if(cprf)
    {
         printf("data[0] = %x / data[1] = %x / data[2] = %x\r\n", data[0], data[1], data[2]);
         printf("reg_address = %x / write_buffer[0] = %x / write_buffer[1] = %x / write_buffer[2] = %x / write_buffer[3] = %x\r\n",  reg_address, write_buffer[0], write_buffer[1],  write_buffer[2], write_buffer[3]);
    }
    //i2c_write(slave_address,reg_address,write_buffer,write_byte);
    if(0 <= i2c_write(slave_address,reg_address,write_buffer,write_byte))
    {
        if(cprf)    printf("GMA_BLOCK_WRITE_OK\r\n");
        uart_ack();   
    } 
    else    if(cprf)    printf("GMA_BLOCK_WRITE_NG\r\n");  
}

void i2c_logic_defalut(void)
{
	signal_group.signal_config->dc_voltage[LOGIC] = 1800;
	dac_set_for_logic();

	signal_group.signal_config->dc_voltage[I2C] = 1800;
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
    
    printf("RELAY_INIT_OK\r\n");
}

void mux_init(void)
{
    gpio_enble->GPIO_DATA = 0x00000000; 
    printf("MUX_INIT_OK\r\n");
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
        vol_1 = signal_group.signal_config->dc_voltage[I2C]; 
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
        vol_0 = signal_group.signal_config->dc_voltage[I2C]; 
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
				