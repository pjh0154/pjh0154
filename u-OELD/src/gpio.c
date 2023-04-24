#include "../include/gpio.h"
#include "../include/fpga_reg.h"
#include <strings.h>

extern int cprf;

void gpio_reg_init(void)
{  
    gpio->GPIO_DATA ^= 0xff;   
    if(cprf) printf("GPIO_INIT_OK : %x\r\n", gpio->GPIO_DATA); 
}

void logic_gpio_ctl(char *data, char stat)
{

    if(strcasecmp(data,"RSTB")==0)
    {
        if(stat)    
        {
            pattern_generator->PG_LP_SEL |= 0x01;
            if(cprf) printf("RSTB_ON\r\n");
        }
        else    
        {
            pattern_generator->PG_LP_SEL &= 0x0E;
            if(cprf) printf("RSTB_OFF\r\n");            
        }           
    }
    else if(strcasecmp(data,"TM")==0)
    {
        if(stat)    
        {
            pattern_generator->PG_LP_SEL |= 0x02;
            if(cprf) printf("TM_ON\r\n");
        }
        else    
        {
           pattern_generator->PG_LP_SEL &= 0x0D; 
            if(cprf) printf("TM_OFF\r\n"); 
        }           
    }  
    else if(strcasecmp(data,"BISTEN")==0)
    {
        if(stat)
        {
            pattern_generator->PG_LP_SEL |= 0x04;
            if(cprf) printf("BISTEN_ON\r\n");
        }    
        else           
        {
            pattern_generator->PG_LP_SEL &= 0x0B;
            if(cprf) printf("BISTEN_OFF\r\n");  
        }           
    } 
    else if(strcasecmp(data,"LPSPARE1")==0)
    {
        if(stat)      
        {    
            pattern_generator->PG_LP_SEL |= 0x08;
             if(cprf) printf("LPSPARE1_ON\r\n");           
        }
        else    
        {
            pattern_generator->PG_LP_SEL &= 0x07;
            if(cprf) printf("LPSPARE1_OFF\r\n");            
        }            
    }
    else if(strcasecmp(data,"ALL")==0)
    {
        if(stat)    
        {
            pattern_generator->PG_LP_SEL |= 0x0F;
            if(cprf) printf("ALL_ON\r\n");           
        }
        else    
        {
            pattern_generator->PG_LP_SEL &= 0x00;
            if(cprf) printf("ALL_OFF\r\n");              
        }           
    }        
}

