#include "../include/ocp.h"
#include "../include/dac.h"
#include "../include/fpga_reg.h"
#include "../include/serial-dev.h"

extern pthread_mutex_t mutex_lock;

int alarm_led_flag = 0;
extern unsigned char ocp_test; 
extern unsigned char pat_index;

void *ocp_task()
{
    int i;
    unsigned int ocp_flag;
    
    //memset(ocp_status, 0, sizeof(ocp_status));
    while(1)
    {
        if(ocp_flag_on)
        {
            pthread_mutex_lock(&mutex_lock);
            if(ocp->OCP_STATUS)
            //if(ocp_test)
            {
                if(!alarm_led_flag)
                {
                    power_off();
                    pg_on_flag = 0;                
                    alarm_led_flag = 1;
                    each_channel_ocp_value = ocp->EACH_CHANNEL_OCP_FLAG; 
                    com_task_ack(OCP_STATUS_ACK);
                    printf("OCP_CH = %x\r\n", ocp->EACH_CHANNEL_OCP_FLAG);
                }
            }
            else
            {
                if(alarm_led_flag)
                {                  
                    alarm_led_flag = 0;
                }
            }
            pthread_mutex_unlock(&mutex_lock);
        }
        usleep(100000);
    }
}
 void el_ocp_set(int data)
 {
    int i = 0;
    short a = 0;
    short b = 0;
    char c[3]  = {0,};				
    signal_group.signal_config->el_over_current_value = data;
    a = (65535/10)*(((((float)signal_group.signal_config->el_over_current_value/1000)*0.25)*((1+(820000/13000)*0.1)))+5);
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
    char c[3]  = {0,};				
    signal_group.signal_config->ap_over_current_value = data;
    a = (65535/10)*(((((float)signal_group.signal_config->ap_over_current_value/1000)*0.5)*((1+(1000000/10200)*0.1)))+5);
    b = -a;
    //printf("AP_OCP_PO = %x / AP_OCP_NE = %x\r\n", a,b);			
    c[2] = a &0xff;
    c[1] = (a>>8) &0xff;
    c[0] = 24;
    dac_set(DAC2,c,3);				
    ////////////////////////////////////////////	
    c[2] = b &0xff;
    c[1] = (b>>8) &0xff;
    c[0] = 25;
    dac_set(DAC2,c,3);
 }

 void dp_ocp_set(int data)
 {
    int i = 0;
    short a = 0;
    short b = 0;
    char c[3]  = {0,};				
    signal_group.signal_config->ap_over_current_value = data;
    a = (65535/10)*(((((float)signal_group.signal_config->ap_over_current_value/1000)*0.5)*((1+(1000000/10200)*0.1)))+5);
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

