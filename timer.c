#include "../include/timer.h"
#include "../include/application.h"
#include "../include/gpio-dev.h"

unsigned int count = 0;
void timer()
{
    count++;
    if(count == 2)  gpiops_set_value(0,1);
    else if(count == 4)   
    {
        gpiops_set_value(0,0);
        count = 0; 
    }
    if(real_time_sensing_flag)
    {
        //if((adc_trigger_count_0 == 1) && (adc_trigger_count_1 == 1) && (adc_trigger_count_2 == 1)&& (adc_trigger_count_3 == 1)&& (adc_trigger_count_4 == 1))
        if((adc_trigger_count_0 == 1) && (adc_trigger_count_1 == 1) && (adc_trigger_count_2 == 1))
        {
            //conversion_fifo_read_sum_value(3,&adc3_cal,adc3_0v_cal_flag,adc3_10v_cal_flag,adc3_n10v_cal_flag);
            //conversion_fifo_read_sum_value(4,&adc3_cal,adc4_0v_cal_flag,adc4_10v_cal_flag,adc4_n10v_cal_flag);
            
            printf("Z = %lf  /  TX = %lf  /  TY = %lf / Z_SUM = %lf / XY_SUM = %lf\r\n",adc_ext[0].z , adc_ext[0].x, adc_ext[0].y,adc_ext[0].z_sum, adc_ext[0].xy_sum);
            //printf("Z = %lf  /  TX = %lf  /  TY = %lf / Z_SUM = %lf / XY_SUM = %lf\r\n",adc_ext[0].z , adc_ext[0].x, adc_ext[0].y,z_xy_sum[0].z_sum, z_xy_sum[0].xy_sum);
            //printf("%d\r\n",adc.data_0[0]);
            if(linearty_flag) 
            {
                linearty_flag = 0;
                timer_end_flag = 0;
                real_time_sensing_flag = 0;
            }
            else    timer_end_flag = 1;
 
            adc_trigger_count_0 = 0;
            adc_trigger_count_1 = 0;
            adc_trigger_count_2 = 0; 
            adc_trigger_count_3 = 0;
            adc_trigger_count_4 = 0;             
            trigger_count_clear();                     
        }
    }
}
 
int createTimer( timer_t *timerID, int sec, int msec )  
{  
    struct sigevent         te;  
    struct itimerspec       its;  
    struct sigaction        sa;  
    int                     sigNo = SIGRTMIN;  
   
    /* Set up signal handler. */  
    sa.sa_flags = SA_SIGINFO;  
    sa.sa_sigaction = timer;     // 타이머 호출시 호출할 함수 
    sigemptyset(&sa.sa_mask);  
  
    if (sigaction(sigNo, &sa, NULL) == -1)  
    {  
        printf("sigaction error\n");
        return -1;  
    }  
   
    /* Set and enable alarm */  
    te.sigev_notify = SIGEV_SIGNAL;  
    te.sigev_signo = sigNo;  
    te.sigev_value.sival_ptr = timerID;  
    timer_create(CLOCK_REALTIME, &te, timerID);  
   
    its.it_interval.tv_sec = sec;
    its.it_interval.tv_nsec = msec * 1000000;  
    its.it_value.tv_sec = sec;
    
    its.it_value.tv_nsec = msec * 1000000;
    timer_settime(*timerID, 0, &its, NULL);  
   
    return 0;  
}