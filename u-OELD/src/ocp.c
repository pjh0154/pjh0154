#include "../include/ocp.h"
#include "../include/dac.h"
#include "../include/fpga_reg.h"
#include "../include/serial-dev.h"

extern pthread_mutex_t mutex_lock;

int alarm_led_flag = 0;
extern unsigned char ocp_test; 
extern unsigned char pat_index;
extern int nprf;
extern total_status_t *total_status;  

void *ocp_task()
{
    int i;
    unsigned int ocp_flag;
    //unsigned char ocp_string_index[5] = {0};
    unsigned char ocp_string_index[128] = {0};   		//230929 Modify
    unsigned char state=0;
    unsigned char rcb_mode_state=0;
    //memset(ocp_status, 0, sizeof(ocp_status));
    while(1)
    {
        ts_ocp_flag_on_get(total_status,&state);
        if(state)
        {
            //pthread_mutex_lock(&mutex_lock);
            if(ocp->OCP_STATUS)
            //if(ocp_test)
            {
                if(!alarm_led_flag)
                {
                    #ifdef RELIABILITY_ROOM	//231027 Modify 
                    if(auto_ocp_flag_on == 1)
                    {
                        auto_ocp_flag_on = 0;
                        log_task(OCP_EVENT,(unsigned short)0);
                        log_task(AUTO_RUN_OFF,(unsigned short)aging_pat_index);
                        //display_task(model_name,"OCP!");
                        //pg_on_flag = PG_OFF;  
                    }     
                    if(pg_on_flag == AUTO_RUN)	snprintf(aging_result_string,sizeof(aging_result_string),"AGING_FAILED");
                    #endif	
                    
                    ts_rcb_mode_ocp_flag_on_get(total_status, &rcb_mode_state);  //231027 Modify                    
                    each_channel_ocp_value = ocp->EACH_CHANNEL_OCP_FLAG;                    
                    if(rcb_mode_state)  snprintf(ocp_string_index,sizeof(ocp_string_index),"OCP[%x] Occur",each_channel_ocp_value);
                    if(rcb_mode_state)  display_task(NULL,ocp_string_index);       //230929 Modify               
                    //ocp_flag_on = 0;          //231013 Modify  
                    if(rcb_mode_state)  button_led_off();          //230929 Modify 

                    power_off();                  
                    //pg_on_flag = PG_OFF;                
                    alarm_led_flag = 1;
                    error_data.ocp = ERROR_NG;
                    com_task_ack(OCP_STATUS_ACK);
                    if(nprf)    printf("OCP_CH = %x\r\n", ocp->EACH_CHANNEL_OCP_FLAG);
                }
            }
            else
            {
                if(alarm_led_flag)
                {                  
                    alarm_led_flag = 0;
                }
            }
            //pthread_mutex_unlock(&mutex_lock);
        }
        else    alarm_led_flag = 0;    
        /*if(otp_flag_on)   	//231101 Modify
        {
           if(!temp_sen_task())
           {
                power_off();
                //pg_on_flag = PG_OFF;               
           } 
        }*/
        //usleep(1);
        usleep(10);
    }
}
 void el_ocp_set(int data)
 {
    int i = 0;
    short a = 0;
    short b = 0;
    char c[4]  = {0,};				
    signal_group.signal_config.el_over_current_value = data;
    a = (65535/10)*(((((float)signal_group.signal_config.el_over_current_value/1000)*0.25)*((1+(820000/13000)*0.1)))+5);
    b = -a;
    //printf("EL_OCP_PO = %x / EL_OCP_NE = %x\r\n", a,b);			
    c[2] = a &0xff;
    c[1] = (a>>8) &0xff;
    c[0] = 22;	
    dac_set(DAC0,c,3);
    ////////////////////////////////////////////
    c[2] = b &0xff;
    c[1] = (b>>8) &0xff;
    c[0] = 23;	
    dac_set(DAC0,c,3);
 }

void ap_ocp_set(int data)
 {
    int i = 0;
    short a = 0;
    short b = 0;
    char c[4]  = {0,};				
    signal_group.signal_config.ap_over_current_value = data;
    a = (65535/10)*(((((float)signal_group.signal_config.ap_over_current_value/1000)*0.5)*((1+(1000000/10200)*0.1)))+5);
    b = -a;
    //printf("AP_OCP_PO = %x / AP_OCP_NE = %x\r\n", a,b);			
    c[2] = a &0xff;
    c[1] = (a>>8) &0xff;
    c[0] = 24;
    dac_set(DAC2,c,3);				
    ////////////////////////////////////////////	
    c[2] = b &0xff;
    c[1] = (b>>8    ) &0xff;
    c[0] = 25;
    dac_set(DAC2,c,3);
 }

 void dp_ocp_set(int data)
 {
    int i = 0;
    short a = 0;
    short b = 0;
    char c[4]  = {0,};				
    signal_group.signal_config.ap_over_current_value = data;
    a = (65535/10)*(((((float)signal_group.signal_config.ap_over_current_value/1000)*0.5)*((1+(1000000/10200)*0.1)))+5);
    b = -a;
    //printf("DP_OCP_PO = %x / DP_OCP_NE = %x\r\n", a,b);			
    c[2] = a &0xff;
    c[1] = (a>>8) &0xff;
    c[0] = 26;
    dac_set(DAC2,c,3);	
    /////////////////////////////////////////////				
    c[2] = b &0xff;
    c[1] = (b>>8) &0xff;
    c[0] = 27;
    dac_set(DAC2,c,3);
 }

