#include "../include/application.h"
#include "../exprtk_library/exprtk.h"

extern pthread_mutex_t mutex_lock;
extern int		pthread_end;
extern int cprf;
extern int eprf;
extern int aprf;

double lpf_tau = 0.00012;
double lpf_time = 0.00001;
#define LPF_TAU lpf_tau
#define LPF_TS  lpf_time

void *fifo_0_empty()
{
    int sendflag=0;
    int sendflag_1=0;    
    int old_adc_trigger_count=0;
    int old_adc_trigger_count_1=0;    

	while(!pthread_end)
	{
        if(fifo_read_0->empty   ==  0)  conversion_fifo_read_ext(0,&real_count_0,fifo_read_0,&adc0_cal,adc0_0v_cal_flag,adc0_10v_cal_flag,adc0_n10v_cal_flag);
        if(fifo_read_1->empty   ==  0)  conversion_fifo_read_ext(1,&real_count_1,fifo_read_1,&adc1_cal,adc1_0v_cal_flag,adc1_10v_cal_flag,adc1_n10v_cal_flag);
        if(fifo_read_2->empty   ==  0)  conversion_fifo_read_ext(2,&real_count_2,fifo_read_2,&adc2_cal,adc2_0v_cal_flag,adc2_10v_cal_flag,adc2_n10v_cal_flag);      

        pthread_mutex_lock(&mutex_lock);
        sendflag=0;
        sendflag_1=0;        
        if(!pro_flag)
        {
            //if(fifo_read_0->empty   ==  1)  printf("0_fifo_read_0->empty =%d adc_trigger_count_0=%d adc_trigger_count_1=%d adc_trigger_count_2=%d\n" ,fifo_read_0->empty,adc_trigger_count_0,adc_trigger_count_1,adc_trigger_count_2 );
            //if(fifo_read_0->empty   ==  1)  printf("1_fifo_read_->empty =%d adc_trigger_count_0=%d adc_trigger_count_1=%d adc_trigger_count_2=%d\n" ,fifo_read_0->empty,adc_trigger_count_0,adc_trigger_count_1,adc_trigger_count_2 );
              //if(fifo_read_0->empty   ==  1)  printf("2_fifo_read_0->empty =%d adc_trigger_count_0=%d adc_trigger_count_1=%d adc_trigger_count_2=%d\n" ,fifo_read_0->empty,adc_trigger_count_0,adc_trigger_count_1,adc_trigger_count_2 );
              //if(fifo_read_0->full   ==  1)  printf("fifo_0_full\r\n" );
              //if(fifo_read_1->full   ==  1)  printf("fifo_1_full\r\n" );
              //if(fifo_read_2->full   ==  1)  printf("fifo_2_full\r\n" );                                            
            //if((adc_trigger_count_0 == count_data.trigger) && (adc_trigger_count_1 == count_data.trigger) && (adc_trigger_count_2 == count_data.trigger) && (adc_trigger_count_3 == count_data.trigger) && (adc_trigger_count_4 == count_data.trigger)) 
            if((adc_trigger_count_0 == count_data.trigger) && (adc_trigger_count_1 == count_data.trigger) && (adc_trigger_count_2 == count_data.trigger)) 
            {
                trigger_end_flag = 1;
            }
            //else if((adc_trigger_count_0>0)&&(adc_trigger_count_1>0)&&(adc_trigger_count_2>0)&&(adc_trigger_count_3>0)&&(adc_trigger_count_4>0))
            else if((adc_trigger_count_0>0)&&(adc_trigger_count_1>0)&&(adc_trigger_count_2>0))           
            {
                if(old_adc_trigger_count!=adc_trigger_count_0)
                {

                    //if(((adc_trigger_count_0%MAX_SENSING_DATA_FORMULA_CALCULATION_CNT)==0)&&((adc_trigger_count_1%MAX_SENSING_DATA_FORMULA_CALCULATION_CNT)==0)&&((adc_trigger_count_2%MAX_SENSING_DATA_FORMULA_CALCULATION_CNT)==0)&&((adc_trigger_count_3%MAX_SENSING_DATA_FORMULA_CALCULATION_CNT)==0)&&((adc_trigger_count_4%MAX_SENSING_DATA_FORMULA_CALCULATION_CNT)==0))
                    if(((adc_trigger_count_0%MAX_SENSING_DATA_FORMULA_CALCULATION_CNT)==0)&&((adc_trigger_count_1%MAX_SENSING_DATA_FORMULA_CALCULATION_CNT)==0)&&((adc_trigger_count_2%MAX_SENSING_DATA_FORMULA_CALCULATION_CNT)==0))
                    {
                        sendflag = 1; 
                        old_adc_trigger_count=adc_trigger_count_0;
                    }
                }
            }
        }
        if(sendflag)
        {
            data_send_ext(adc_ext,MAX_PACKET_DATA_SIZE,(adc_trigger_count_0/MAX_SENSING_DATA_CNT),(count_data.trigger/MAX_SENSING_DATA_CNT)+((count_data.trigger%MAX_SENSING_DATA_CNT)>0?1:0));
            //printf("TP1 = %d / TP2 = %d / TP3 = %d\r\n", adc_trigger_count_0, adc_trigger_count_1, adc_trigger_count_2);
        }
        if(trigger_end_flag)  
        {
             //printf("TP4 = %d / TP5 = %d / TP6 = %d\r\n", adc_trigger_count_0, adc_trigger_count_1, adc_trigger_count_2);
            if(adc_trigger_count_0!=MAX_SENSING_DATA_CNT) data_send_ext(adc_ext,(adc_trigger_count_0%MAX_SENSING_DATA_CNT)*TOTAL_DATA_SIZE,(adc_trigger_count_0/MAX_SENSING_DATA_CNT),(count_data.trigger/MAX_SENSING_DATA_CNT)+((count_data.trigger%MAX_SENSING_DATA_CNT)>0?1:0));
            else data_send_ext(adc_ext,(adc_trigger_count_0)*TOTAL_DATA_SIZE,1,1); 
            formula_calculation_flag = 0;      //formula_calculation 기능 추가  
            pro_flag = 1;
            trigger_end_flag = 0;
            if(cprf) printf("manual_end_flag_ok\r\n");  
            adc_trigger_count_0 = 0; 
            adc_trigger_count_1 = 0; 
            adc_trigger_count_2 = 0;
            adc_trigger_count_3 = 0; 
            adc_trigger_count_4 = 0;            
            old_adc_trigger_count=0;                            
            trigger_count_clear();
        }

        if(!read_pro_flag)
        {
            //if(fifo_read_0->empty   ==  1)  printf("fifo_read_0->empty =%d adc_trigger_count_0=%d adc_trigger_count_1=%d adc_trigger_count_2=%d\n" ,fifo_read_0->empty,adc_trigger_count_0,adc_trigger_count_1,adc_trigger_count_2 );
            if((adc_trigger_count_0 == count_data.trigger) && (adc_trigger_count_1 == count_data.trigger) && (adc_trigger_count_2 == count_data.trigger) && (adc_trigger_count_3 == count_data.trigger) && (adc_trigger_count_4 == count_data.trigger)) 
            {
                read_end_flag = 1;
            }
            else if((adc_trigger_count_0>0)&&(adc_trigger_count_1>0)&&(adc_trigger_count_2>0)&&(adc_trigger_count_3>0)&&(adc_trigger_count_4>0))
            {
                if(old_adc_trigger_count_1!=adc_trigger_count_0)
                {
                    if(((adc_trigger_count_0%MAX_SENSING_DATA_CNT)==0)&&((adc_trigger_count_1%MAX_SENSING_DATA_CNT)==0)&&((adc_trigger_count_2%MAX_SENSING_DATA_CNT)==0)&&((adc_trigger_count_3%MAX_SENSING_DATA_CNT)==0)&&((adc_trigger_count_4%MAX_SENSING_DATA_CNT)==0))
                    {
                        sendflag_1 = 1; 
                        old_adc_trigger_count_1=adc_trigger_count_0;
                    }
                }
            }
        }
        if(sendflag_1)
        {
            data_send_ext(adc_ext,MAX_PACKET_DATA_SIZE,(adc_trigger_count_0/MAX_SENSING_DATA_CNT),(count_data.trigger/MAX_SENSING_DATA_CNT)+((count_data.trigger%MAX_SENSING_DATA_CNT)>0?1:0));
        }        
        if(read_end_flag)  
        {
            if(adc_trigger_count_0!=MAX_SENSING_DATA_CNT) data_send_ext(adc_ext,(adc_trigger_count_0%MAX_SENSING_DATA_CNT)*TOTAL_DATA_SIZE,(adc_trigger_count_0/MAX_SENSING_DATA_CNT),(count_data.trigger/MAX_SENSING_DATA_CNT)+((count_data.trigger%MAX_SENSING_DATA_CNT)>0?1:0));
            else data_send_ext(adc_ext,(adc_trigger_count_0)*TOTAL_DATA_SIZE,1,1); 
            formula_calculation_flag = 0;      //formula_calculation 기능 추가      
            adc_trigger_count_0 = 0; 
            adc_trigger_count_1 = 0; 
            adc_trigger_count_2 = 0;
            adc_trigger_count_3 = 0; 
            adc_trigger_count_4 = 0;            
            old_adc_trigger_count=0;                            
            trigger_count_clear();           
            read_pro_flag = 1;
            read_end_flag = 0;
            if(cprf) printf("ex_end_flag_ok\r\n");
        }         

        pthread_mutex_unlock(&mutex_lock);  

        real_time_sensing();                          
        usleep(1);
	}    
}

//short adc_avg_data[3][10]={{0,},};
short adc_avg_data[3][150]={{0,},};     //22.11.17
double  adc_conver_data[3][150]={{0,},}; 
void conversion_fifo_read_ext(int sel,int *count,REGISTER_CONFIG_CS0 *fifo_read,ADC_CAL *adc_cal,u_int8_t cal_flag1,u_int8_t cal_flag2,u_int8_t cal_flag3)
{
    int i, j;
    //int flag = 0;
    double average_data = 0;
    double all_plus_data = 0;
    double real_data = 0; 
    short adc_data=0;
    register short temp=0;
    static double pre_average_data = 0;          
    static double pre_real_data[3] = {0,0,0}; 

    ///////////////////////////////////////////////////////////////////////////////
    kalman1_state state1;
    kalman2_state state2;
    double init_x[2];
    double init_p[2][2] = {{10e-6,0}, {0,10e-6}};    
    ///////////////////////////////////////////////////////////////////////////////       

    for(i =*count;i<count_data.read;i++)
    {
        if(fifo_read->empty   ==  0)
        {
            fifo_read->reg_control = 1;	
            fifo_read->reg_control = 0;	
            adc_data   =   fifo_read->fifo_data & 0x0000ffff; 

            //if((adc_data >> 12) & 0x0001)  if(eprf) printf("overflow event\r\n");
            if((adc_data >> 15) & 0x0001)
            {
                //flag = 1;
                adc_avg_data[sel][i] = -(((~adc_data) & 0x07fff) + 1);     
            } 
            else 
            {
                //flag = 0;
                adc_avg_data[sel][i] = adc_data & 0x07fff;    
            }
            if(aprf)
            { 
                printf("ADC%d = %d /  %d /read_count = %d\r\n", sel,fifo_read->fifo_data, adc_avg_data[sel][i],i);
            }
            *count+=1;
        }
        else break;
    }
    all_plus_data=0;
    if(*count == count_data.read) 
    {
    ///////////////////////////////////////////////////////////////////////////////   
    //printf("TP0 = %f / %f\r\n", (float)adc_avg_data[sel][0], (float)adc_avg_data[sel][1]);
        //state1.x = (float)adc_avg_data[sel][0];
        //kalman1_init(&state1, (double)adc_avg_data[sel][1], 5e2);
        //init_x[0] = (double)adc_avg_data[sel][0];
        //init_x[1] = (double)(adc_avg_data[sel][1] - adc_avg_data[sel][0]); 
        //kalman2_init(&state2, init_x, init_p); 
        //kalman2_init(&state2, init_x, init_p);    
        //for(i=2 ; i<count_data.read-1; i++)
        //{
            //all_plus_data += kalman1_filter(&state1, adc_avg_data[sel][i]); 
            //adc_conver_data[sel][i] = kalman1_filter(&state1, adc_avg_data[sel][i]); 
            //all_plus_data += kalman1_filter(&state1, adc_conver_data[sel][i]); 
            //all_plus_data += kalman2_filter(&state2, adc_conver_data[sel][i]);                
        //} 
        //average_data=all_plus_data /count_data.read-3 ;         
    ///////////////////////////////////////////////////////////////////////////////         
        /*for(i=0; i<count_data.read-1; i++)
        {
            for(j=0; j<count_data.read-1-i; j++)
            {
                if(adc_avg_data[sel][j] > adc_avg_data[sel][j+1]) {
                    temp = adc_avg_data[sel][j];
                    adc_avg_data[sel][j] = adc_avg_data[sel][j+1];
                    adc_avg_data[sel][j+1] = temp;
                }
            }            
        }*/                   
        all_plus_data=0;
        if(count_data.read > 4)
        {
            for(i=0;i<(count_data.read);i++)           
            //for(i=11;i<(count_data.read-9);i++)
            {
                all_plus_data+=adc_avg_data[sel][i];
            }
            //if(sel == 1)    average_data=all_plus_data /(count_data.read);        //2022.11.03기준 1EA납품분 적용 FW (VER_1.1.0)
            //else    average_data=all_plus_data /(count_data.read-2);              //2022.11.03기준 1EA납품분 적용 FW (VER_1.1.0)   
            average_data=all_plus_data /(count_data.read);                    //2022.11.03기준 기존 1EA 납품분 제외 나머지 적용 FW
            //average_data=all_plus_data /(count_data.read-20);                     //count ??利앷? 諛?怨좎젙 22.11.17  
           /*for(i=0;i<(count_data.read);i++)
            {
               // if(i = 0) pre_average_data = ((double)adc_avg_data[sel][5]);
               if(i == 0){
                    //if(pre_average_data == 0) pre_average_data = ((double)adc_avg_data[sel][5]);
                    pre_average_data = ((double)adc_avg_data[sel][20]);                   
               }
                average_data = ((LPF_TAU/(LPF_TAU+LPF_TS))*pre_average_data) + ((LPF_TS/(LPF_TAU+LPF_TS)) * ((double)adc_avg_data[sel][i]));
                pre_average_data = average_data;
                //all_plus_data+=adc_avg_data[sel][i];
            }*/
            //average_data=all_plus_data /(count_data.read);                      
        } 

        //else
        //{
            //for(i=0;i<count_data.read;i++)
            //{
            //    all_plus_data+=adc_avg_data[sel][i];
            //}
            //average_data=all_plus_data /count_data.read ;
        //}
        if(aprf) printf("AVERAGE_DATA = %f / PLUS_DATA = %d / COUNT_DATA = %d\r\n", average_data,all_plus_data, count_data.read);

        //LOGE("AVERAGE_DATA_0 = %d / %d / %d / %d\r\n", all_plus_data, average_data, real_count_0, read_count);
  
        if(cal_flag1) adc_cal->adc_0v_step = average_data;
        else if(cal_flag2) adc_cal->adc_10v_step = average_data; 
        else if(cal_flag3) adc_cal->adc_n10v_step = average_data;
        else
        {
            if(average_data<0)
            {
                real_data = ((average_data * adc_cal->adc_n_ratio)  + adc_cal->adc_n_offset)+adc_cal->user_n_offset; 
            }
            else
            {
                real_data = ((average_data * adc_cal->adc_p_ratio)  + adc_cal->adc_p_offset)+adc_cal->user_p_offset;  
            }
            switch(sel)
            {
                case 0:
                    if(formula_calculation_flag)
                    {
                        adc_formula->z = real_data;
                        if(cprf) printf("ADC%d_FORMULA_DATA = %lf\r\n",sel,adc_formula->z); 
                        adc0_formula_flag = 1;
                    }
                    else
                    {
                        /*if(adc_trigger_count_0 == 0)
                        {
                            if(pre_real_data[0] == 0)   pre_real_data[0] = real_data;   
                        }               
                        real_data = ((LPF_TAU/(LPF_TAU+LPF_TS))*pre_real_data[0]) + ((LPF_TS/(LPF_TAU+LPF_TS)) * real_data);
                        pre_real_data[0] = real_data; */                                   
                        adc_ext[adc_trigger_count_0%MAX_SENSING_DATA_CNT].z = real_data;
                        if(cprf) printf("ADC%d_DATA = %lf\r\n",sel,adc_ext[adc_trigger_count_0%MAX_SENSING_DATA_CNT].z);    
                        //adc_ext[adc_trigger_count_0%MAX_SENSING_DATA_CNT].z = real_data;                   
                        //if(cprf) printf("ADC%d_DATA = %d\r\n",sel,adc_ext[adc_trigger_count_0%MAX_SENSING_DATA_CNT].z); 
                        adc_trigger_count_0++;
                        conversion_fifo_read_sum_value(3,&adc3_cal,adc3_0v_cal_flag,adc3_10v_cal_flag,adc3_n10v_cal_flag);
                        conversion_fifo_read_sum_value(4,&adc4_cal,adc4_0v_cal_flag,adc4_10v_cal_flag,adc4_n10v_cal_flag);                        
                    }
                    break;
                    
                case 1:
                    if(formula_calculation_flag)
                    {
                        adc_formula->x = real_data;
                        if(cprf) printf("ADC%d_FORMULA_DATA = %lf\r\n",sel,adc_formula->x); 
                        adc1_formula_flag = 1;                       
                    }
                    else
                    {
                        /*if(adc_trigger_count_1 == 0)
                        {
                            if(pre_real_data[1] == 0)   pre_real_data[1] = real_data;   
                        }               
                        real_data = ((LPF_TAU/(LPF_TAU+LPF_TS))*pre_real_data[1]) + ((LPF_TS/(LPF_TAU+LPF_TS)) * real_data);
                        pre_real_data[1] = real_data;*/                          
                        adc_ext[adc_trigger_count_1%MAX_SENSING_DATA_CNT].x = real_data;                     
                        if(cprf) printf("ADC%d_DATA = %lf\r\n",sel,adc_ext[adc_trigger_count_1%MAX_SENSING_DATA_CNT].x);  
                        //adc_ext[adc_trigger_count_1%MAX_SENSING_DATA_CNT].x = real_data;                    
                        //if(cprf) printf("ADC%d_DATA = %d\r\n",sel,adc_ext[adc_trigger_count_1%MAX_SENSING_DATA_CNT].x); 
                        adc_trigger_count_1++;                        
                    }
                    break;
                    
                case 2:
                    if(formula_calculation_flag)
                    {
                        adc_formula->y = real_data;
                        if(cprf) printf("ADC%d_FORMULA_DATA = %lf\r\n",sel,adc_formula->y); 
                        adc2_formula_flag = 1;                       
                    }      
                    else
                    {
                        /*if(adc_trigger_count_2 == 0)
                        {
                            if(pre_real_data[2] == 0)   pre_real_data[2] = real_data;   
                        }               
                        real_data = ((LPF_TAU/(LPF_TAU+LPF_TS))*pre_real_data[2]) + ((LPF_TS/(LPF_TAU+LPF_TS)) * real_data);
                        pre_real_data[2] = real_data;*/                                                 
                        adc_ext[adc_trigger_count_2%MAX_SENSING_DATA_CNT].y = real_data;                         
                        if(cprf) printf("ADC%d_DATA = %lf\r\n",sel,adc_ext[adc_trigger_count_2%MAX_SENSING_DATA_CNT].y);
                        //adc_ext[adc_trigger_count_2%MAX_SENSING_DATA_CNT].y = real_data;                
                        //if(cprf) printf("ADC%d_DATA = %d\r\n",sel,adc_ext[adc_trigger_count_2%MAX_SENSING_DATA_CNT].y);
                        adc_trigger_count_2++;                        
                    } 
                    break; 
            }
            if((formula_calculation_flag == 1) && (adc0_formula_flag ==1) && (adc1_formula_flag ==1) && (adc2_formula_flag ==1)) 
            {
                double value_table[32] = {0};
				//double result;			
				value_table[0] = adc_formula->z;
				value_table[1] = adc_formula->x;
				value_table[2] = adc_formula->y;

                adc_ext[adc_trigger_count_0%MAX_SENSING_DATA_CNT].z = calculation_0(value_table);                             
                adc_ext[adc_trigger_count_1%MAX_SENSING_DATA_CNT].x = calculation_1(value_table);             
                adc_ext[adc_trigger_count_2%MAX_SENSING_DATA_CNT].y = calculation_2(value_table);
                conversion_fifo_read_sum_value(3,&adc3_cal,adc3_0v_cal_flag,adc3_10v_cal_flag,adc3_n10v_cal_flag);
                conversion_fifo_read_sum_value(4,&adc4_cal,adc4_0v_cal_flag,adc4_10v_cal_flag,adc4_n10v_cal_flag);                 

                adc0_formula_flag = 0;
                adc1_formula_flag = 0;
                adc2_formula_flag = 0;
                if(cprf) printf("FORMULA_CALCULATION z = %.20lf / x = %.20lf / y = %.20lf\r\n", adc_ext[adc_trigger_count_0].z, adc_ext[adc_trigger_count_1].x, adc_ext[adc_trigger_count_2].y);               
                adc_trigger_count_0++;
                adc_trigger_count_1++;
                adc_trigger_count_2++;                                               
            }
        }
            
        //memset(&temp_adc.data_0, 0, sizeof(temp_adc.data_0));
    }   
    if(*count == count_data.read)    *count = 0;
}

///////////////////////////////////////////////////////////////////////////////
void kalman1_init(kalman1_state *state, double init_x, double init_p)
{
    state->x = init_x;
    state->p = init_p;
    state->A = 1;
    state->H = 1;
    state->q = 2e2;//10e-6;  /* predict noise convariance */
    state->r = 5e2;//10e-5;  /* measure error convariance */
}

float kalman1_filter(kalman1_state *state, double z_measure)
{
    /* Predict */
    state->x = state->A * state->x;
    state->p = state->A * state->A * state->p + state->q;  /* p(n|n-1)=A^2*p(n-1|n-1)+q */

    /* Measurement */
    state->gain = state->p * state->H / (state->p * state->H * state->H + state->r);
    state->x = state->x + state->gain * (z_measure - state->H * state->x);
    state->p = (1 - state->gain * state->H) * state->p;

    return state->x;
}

void kalman2_init(kalman2_state *state, double *init_x, double (*init_p)[2])
{
    state->x[0]    = init_x[0];
    state->x[1]    = init_x[1];
    state->p[0][0] = init_p[0][0];
    state->p[0][1] = init_p[0][1];
    state->p[1][0] = init_p[1][0];
    state->p[1][1] = init_p[1][1];
    //state->A       = {{1, 0.1}, {0, 1}};
    state->A[0][0] = 1;
    state->A[0][1] = 0.1;
    state->A[1][0] = 0;
    state->A[1][1] = 1;
    //state->H       = {1,0};
    state->H[0]    = 1;
    state->H[1]    = 0;
    //state->q       = {{10e-6,0}, {0,10e-6}};  /* measure noise convariance */
    state->q[0]    = 10e-7;
    state->q[1]    = 10e-7;
    state->r       = 10e-7;  /* estimated error convariance */
}

float kalman2_filter(kalman2_state *state, double z_measure)
{
    double temp0 = 0.0f;
    double temp1 = 0.0f;
    double temp = 0.0f;

    /* Step1: Predict */
    state->x[0] = state->A[0][0] * state->x[0] + state->A[0][1] * state->x[1];
    state->x[1] = state->A[1][0] * state->x[0] + state->A[1][1] * state->x[1];
    /* p(n|n-1)=A^2*p(n-1|n-1)+q */
    state->p[0][0] = state->A[0][0] * state->p[0][0] + state->A[0][1] * state->p[1][0] + state->q[0];
    state->p[0][1] = state->A[0][0] * state->p[0][1] + state->A[1][1] * state->p[1][1];
    state->p[1][0] = state->A[1][0] * state->p[0][0] + state->A[0][1] * state->p[1][0];
    state->p[1][1] = state->A[1][0] * state->p[0][1] + state->A[1][1] * state->p[1][1] + state->q[1];

    /* Step2: Measurement */
    /* gain = p * H^T * [r + H * p * H^T]^(-1), H^T means transpose. */
    temp0 = state->p[0][0] * state->H[0] + state->p[0][1] * state->H[1];
    temp1 = state->p[1][0] * state->H[0] + state->p[1][1] * state->H[1];
    temp  = state->r + state->H[0] * temp0 + state->H[1] * temp1;
    state->gain[0] = temp0 / temp;
    state->gain[1] = temp1 / temp;
    /* x(n|n) = x(n|n-1) + gain(n) * [z_measure - H(n)*x(n|n-1)]*/
    temp = state->H[0] * state->x[0] + state->H[1] * state->x[1];
    state->x[0] = state->x[0] + state->gain[0] * (z_measure - temp); 
    state->x[1] = state->x[1] + state->gain[1] * (z_measure - temp);

    /* Update @p: p(n|n) = [I - gain * H] * p(n|n-1) */
    state->p[0][0] = (1 - state->gain[0] * state->H[0]) * state->p[0][0];
    state->p[0][1] = (1 - state->gain[0] * state->H[1]) * state->p[0][1];
    state->p[1][0] = (1 - state->gain[1] * state->H[0]) * state->p[1][0];
    state->p[1][1] = (1 - state->gain[1] * state->H[1]) * state->p[1][1];

    return state->x[0];
}
///////////////////////////////////////////////////////////////////////////////

void adc_cal_load()
{
    FILE *adc_file;
    memset(&adc0_cal, 0, 64);
    memset(&adc1_cal, 0, 64);
    memset(&adc2_cal, 0, 64);
    memset(&adc3_cal, 0, 64);
    memset(&adc4_cal, 0, 64);    
    usleep(1000);   

    if(system("ls /run/media/mmcblk0p2/f0/config/adc0_cal.info"))
    {
       fopen(ADC_0_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(ADC_0_CAL_FILE_PATH, "r");
        fread(&adc0_cal, sizeof(adc0_cal), 1, adc_file);  
        usleep(1000);
    }
    if(system("ls /run/media/mmcblk0p2/f0/config/adc1_cal.info"))
    {
       fopen(ADC_1_CAL_FILE_PATH,"wb");                            
    } 
    else
    {
        adc_file = fopen(ADC_1_CAL_FILE_PATH, "r");
        fread(&adc1_cal, sizeof(adc1_cal), 1, adc_file);
        usleep(1000); 
    }
    if(system("ls /run/media/mmcblk0p2/f0/config/adc2_cal.info"))
    {
       fopen(ADC_2_CAL_FILE_PATH,"wb");                            
    }  
    else
    {
        adc_file = fopen(ADC_2_CAL_FILE_PATH, "r");
        fread(&adc2_cal, sizeof(adc2_cal), 1, adc_file); 
        usleep(1000); 
    }
    if(system("ls /run/media/mmcblk0p2/f0/config/adc3_cal.info"))
    {
       fopen(ADC_3_CAL_FILE_PATH,"wb");                            
    } 
    else
    {
        adc_file = fopen(ADC_3_CAL_FILE_PATH, "r");
        fread(&adc3_cal, sizeof(adc3_cal), 1, adc_file);
        usleep(1000); 
    } 
    if(system("ls /run/media/mmcblk0p2/f0/config/adc4_cal.info"))
    {
       fopen(ADC_4_CAL_FILE_PATH,"wb");                            
    }
    else
    {
        adc_file = fopen(ADC_4_CAL_FILE_PATH, "r");
        fread(&adc4_cal, sizeof(adc4_cal), 1, adc_file); 
        usleep(1000);   
    }                 

    LOGE("--------------------------------------------------ADC0--------------------------------------------------\r\n");
    LOGE("adc_0_cal_0v_OK = value = %f / step = %f\n", adc0_cal.adc_0v_value, adc0_cal.adc_0v_step);
    LOGE("adc_0_cal_p10v_OK = value = %f / step = %f\n", adc0_cal.adc_10v_value, adc0_cal.adc_10v_step);
    LOGE("adc_0_cal_n10v_OK = value = %f / step = %f\n", adc0_cal.adc_n10v_value, adc0_cal.adc_n10v_step);				
    LOGE("adc_0_p_ratio value = %f\r\n", adc0_cal.adc_p_ratio);
    LOGE("adc_0_p_offset value = %f\r\n", adc0_cal.adc_p_offset);
    LOGE("adc_0_n_ratio value = %f\r\n", adc0_cal.adc_n_ratio);
    LOGE("adc_0_n_offset value = %f\r\n", adc0_cal.adc_n_offset);
    LOGE("adc0_cal.user_p_offset value = %f\r\n", adc0_cal.user_p_offset);
    LOGE("adc0_cal.user_n_offset value = %f\r\n", adc0_cal.user_n_offset);										
    LOGE("--------------------------------------------------ADC1--------------------------------------------------\r\n");
    LOGE("adc_1_cal_0v_OK = value = %f / step = %f\n", adc1_cal.adc_0v_value, adc1_cal.adc_0v_step);
    LOGE("adc_1_cal_p10v_OK = value = %f / step = %f\n", adc1_cal.adc_10v_value, adc1_cal.adc_10v_step);
    LOGE("adc_1_cal_n10v_OK = value = %f / step = %f\n", adc1_cal.adc_n10v_value, adc1_cal.adc_n10v_step);				
    LOGE("adc_1_p_ratio value = %f\r\n", adc1_cal.adc_p_ratio);
    LOGE("adc_1_p_offset value = %f\r\n", adc1_cal.adc_p_offset);
    LOGE("adc_1_n_ratio value = %f\r\n", adc1_cal.adc_n_ratio);
    LOGE("adc_1_n_offset value = %f\r\n", adc1_cal.adc_n_offset);
    LOGE("adc1_cal.user_p_offset value = %f\r\n", adc1_cal.user_p_offset);
    LOGE("adc1_cal.user_n_offset value = %f\r\n", adc1_cal.user_n_offset);					
    LOGE("--------------------------------------------------ADC2--------------------------------------------------\r\n");
    LOGE("adc_2_cal_0v_OK = value = %f / step = %f\n", adc2_cal.adc_0v_value, adc2_cal.adc_0v_step);
    LOGE("adc_2_cal_p10v_OK = value = %f / step = %f\n", adc2_cal.adc_10v_value, adc2_cal.adc_10v_step);
    LOGE("adc_2_cal_n10v_OK = value = %f / step = %f\n", adc2_cal.adc_n10v_value, adc2_cal.adc_n10v_step);				
    LOGE("adc_2_p_ratio value = %f\r\n", adc2_cal.adc_p_ratio);
    LOGE("adc_2_p_offset value = %f\r\n", adc2_cal.adc_p_offset);
    LOGE("adc_2_n_ratio value = %f\r\n", adc2_cal.adc_n_ratio);
    LOGE("adc_2_n_offset value = %f\r\n", adc2_cal.adc_n_offset);	
    LOGE("adc2_cal.user_p_offset value = %f\r\n", adc2_cal.user_p_offset);
    LOGE("adc2_cal.user_n_offset value = %f\r\n", adc2_cal.user_n_offset);
    LOGE("--------------------------------------------------ADC3--------------------------------------------------\r\n");
    LOGE("adc_3_cal_0v_OK = value = %f / step = %f\n", adc3_cal.adc_0v_value, adc3_cal.adc_0v_step);
    LOGE("adc_3_cal_p10v_OK = value = %f / step = %f\n", adc3_cal.adc_10v_value, adc3_cal.adc_10v_step);
    LOGE("adc_3_cal_n10v_OK = value = %f / step = %f\n", adc3_cal.adc_n10v_value, adc3_cal.adc_n10v_step);				
    LOGE("adc_3_p_ratio value = %f\r\n", adc3_cal.adc_p_ratio);
    LOGE("adc_3_p_offset value = %f\r\n", adc3_cal.adc_p_offset);
    LOGE("adc_3_n_ratio value = %f\r\n", adc3_cal.adc_n_ratio);
    LOGE("adc_3_n_offset value = %f\r\n", adc3_cal.adc_n_offset);	
    LOGE("adc3_cal.user_p_offset value = %f\r\n", adc3_cal.user_p_offset);
    LOGE("adc3_cal.user_n_offset value = %f\r\n", adc3_cal.user_n_offset); 
    LOGE("--------------------------------------------------ADC4--------------------------------------------------\r\n");
    LOGE("adc_4_cal_0v_OK = value = %f / step = %f\n", adc4_cal.adc_0v_value, adc4_cal.adc_0v_step);
    LOGE("adc_4_cal_p10v_OK = value = %f / step = %f\n", adc4_cal.adc_10v_value, adc4_cal.adc_10v_step);
    LOGE("adc_4_cal_n10v_OK = value = %f / step = %f\n", adc4_cal.adc_n10v_value, adc4_cal.adc_n10v_step);				
    LOGE("adc_4_p_ratio value = %f\r\n", adc4_cal.adc_p_ratio);
    LOGE("adc_4_p_offset value = %f\r\n", adc4_cal.adc_p_offset);
    LOGE("adc_4_n_ratio value = %f\r\n", adc4_cal.adc_n_ratio);
    LOGE("adc_4_n_offset value = %f\r\n", adc4_cal.adc_n_offset);	
    LOGE("adc4_cal.user_p_offset value = %f\r\n", adc4_cal.user_p_offset);
    LOGE("adc4_cal.user_n_offset value = %f\r\n", adc4_cal.user_n_offset);         	                   
}

void trigger_count_clear()
{
    ad7656->reg_count_reset = 1; 			
    ad7656->reg_count_reset = 0;
    if(cprf) LOGE("TRIGGER COUNT Clear\r\n");      
}

void trigger_auto_run()
{
    ad7656->auto_onoff = 0;
    ad7656->auto_onoff = 1;
    ad7656->auto_onoff = 0;    
}

void fifo_reset()
{
	fifo_read_0->reg_reset = 0;
	fifo_read_1->reg_reset = 0;
	fifo_read_2->reg_reset = 0;	
	//fifo_read_3->reg_reset = 0;
	//fifo_read_4->reg_reset = 0;	    		
	usleep(1);
	fifo_read_0->reg_reset = 1;
	fifo_read_1->reg_reset = 1;	
	fifo_read_2->reg_reset = 1;	
	//fifo_read_3->reg_reset = 1;
	//fifo_read_4->reg_reset = 1;	    	
	usleep(1);
	fifo_read_0->reg_reset = 0;	
	fifo_read_1->reg_reset = 0;	
	fifo_read_2->reg_reset = 0;	
	//fifo_read_3->reg_reset = 0;
	//fifo_read_4->reg_reset = 0;	    		
	if(cprf) LOGE("FIFO_RESET OK\r\n");	    
}

void conversion_fifo_read_sum_value(int sel,ADC_CAL *adc_cal,u_int8_t cal_flag1,u_int8_t cal_flag2,u_int8_t cal_flag3)
{
    short adc_data=0;
    double convert_data = 0;
    double real_data = 0;     

    if(sel == 3)    adc_data   =   ad7656->z_sum & 0x0000ffff; 
    else        adc_data   =   ad7656->xy_sum & 0x0000ffff;     

    //if((adc_data >> 12) & 0x0001)  if(eprf) printf("overflow event\r\n");
    if((adc_data >> 15) & 0x0001)
    {
        //flag = 1;
        convert_data = -(((~adc_data) & 0x07fff) + 1);     
    } 
    else 
    {
        //flag = 0;
        convert_data = adc_data & 0x07fff;    
    }
    if(aprf)
    { 
        if(sel == 3)    printf("ADC%d = %d /  %d\r\n", sel,ad7656->z_sum, convert_data);
        else        printf("ADC%d = %d /  %d\r\n", sel,ad7656->xy_sum, convert_data);    
    }  

    if(cal_flag1) adc_cal->adc_0v_step = convert_data;
    else if(cal_flag2) adc_cal->adc_10v_step = convert_data; 
    else if(cal_flag3) adc_cal->adc_n10v_step = convert_data;        

    else
    {
        if(convert_data<0)
        {
            real_data = ((convert_data * adc_cal->adc_n_ratio)  + adc_cal->adc_n_offset)+adc_cal->user_n_offset; 
        }
        else
        {
            real_data = ((convert_data * adc_cal->adc_p_ratio)  + adc_cal->adc_p_offset)+adc_cal->user_p_offset;  
        }   

        switch(sel)
        {
            case 3:
                //z_xy_sum[adc_trigger_count_3%MAX_SENSING_DATA_CNT].z_sum = real_data;
                //if(cprf) printf("ADC%d_DATA = %lf\r\n",sel,z_xy_sum[adc_trigger_count_3%MAX_SENSING_DATA_CNT].z_sum);    
                adc_ext[adc_trigger_count_3%MAX_SENSING_DATA_CNT].z_sum = real_data;                   
                if(cprf) printf("ADC%d_DATA = %lf\r\n",sel,adc_ext[adc_trigger_count_3%MAX_SENSING_DATA_CNT].z_sum); 
                adc_trigger_count_3++;
                break; 

                case 4:
                //z_xy_sum[adc_trigger_count_4%MAX_SENSING_DATA_CNT].xy_sum = real_data; 
                //if(cprf) printf("ADC%d_DATA = %lf\r\n",sel,z_xy_sum[adc_trigger_count_4%MAX_SENSING_DATA_CNT].xy_sum);  
                adc_ext[adc_trigger_count_4%MAX_SENSING_DATA_CNT].xy_sum = real_data;                    
                if(cprf) printf("ADC%d_DATA = %lf\r\n",sel,adc_ext[adc_trigger_count_4%MAX_SENSING_DATA_CNT].xy_sum); 
                adc_trigger_count_4++;                        
                break;
        }
    }
}  

void real_time_sensing()
{
    int i = 0;
    if(real_time_sensing_flag)
    {     
        if(timer_end_flag)
        {
            adc_trigger_count_0 = 0;
            adc_trigger_count_1 = 0;
            adc_trigger_count_2 = 0;
            adc_trigger_count_3 = 0;
            adc_trigger_count_4 = 0;	            	
            real_count_0 = 0;
            real_count_1 = 0;	
            real_count_2 = 0;
            real_count_3 = 0;	
            real_count_4 = 0;	            													

            ad7656->auto_onoff = 0;
            ad7656->auto_onoff = 1;
            ad7656->auto_onoff = 0;  

            timer_end_flag = 0;
        }	
    }
}

void ld_current_value_save()
{
    FILE *adc_file;
    ld_1d_data.ld_id = 0x80;
    ld_2d_data.ld_id = 0x00;     
    if(!system("ls /run/media/mmcblk0p2/f0/bin/ld_1d_value_file.txt"))
    {
        adc_file = fopen(LD_1D_VALUE_FILE_PATH,"wb");  
		fwrite(&ld_1d_data.ld_value, sizeof(ld_1d_data.ld_value), 1, adc_file);
		fclose(adc_file);
		system("sync"); 
        printf("LD_1D_ID = %x / LD_D1_VALUE = %x\r\n", ld_1d_data.ld_id, ld_1d_data.ld_value);                          
    }
    else (printf("LD_1D_VALUE FILE NOT FIND\r\n"));

    if(!system("ls /run/media/mmcblk0p2/f0/bin/ld_2d_value_file.txt"))
    {
        adc_file = fopen(LD_2D_VALUE_FILE_PATH,"wb"); 
		fwrite(&ld_2d_data.ld_value, sizeof(ld_2d_data.ld_value), 1, adc_file);
		fclose(adc_file);
		system("sync"); 
        printf("LD_2D_ID = %x / LD_D2_VALUE = %x\r\n", ld_2d_data.ld_id, ld_2d_data.ld_value);                                        
    }
    else (printf("LD_2D_VALUE FILE NOT FIND\r\n"));   
  
}

void ld_current_ctrl_init()
{
    FILE *adc_file;
    //ld_1d_data.ld_id = 0x80;
    //ld_2d_data.ld_id = 0x00;

    ld_1d_data.ld_id = 0x80;
    ld_2d_data.ld_id = 0x00;    

    if(system("ls /run/media/mmcblk0p2/f0/bin/ld_1d_value_file.txt"))
    {
        adc_file = fopen(LD_1D_VALUE_FILE_PATH,"wb"); 
        ld_1d_data.ld_value= 22;   
		fwrite(&ld_1d_data.ld_value, sizeof(ld_1d_data.ld_value), 1, adc_file);
		fclose(adc_file);
		system("sync");                              
    }
    else
    {
        adc_file = fopen(LD_1D_VALUE_FILE_PATH, "r");
        fread(&ld_1d_data.ld_value, sizeof(ld_1d_data.ld_value), 1, adc_file);  
        usleep(1000);
    } 

    if(system("ls /run/media/mmcblk0p2/f0/bin/ld_2d_value_file.txt"))
    {
        adc_file = fopen(LD_2D_VALUE_FILE_PATH,"wb");  
        ld_2d_data.ld_value= 22;   
		fwrite(&ld_2d_data.ld_value, sizeof(ld_2d_data.ld_value), 1, adc_file);
		fclose(adc_file);
		system("sync");                                     
    }
    else
    {
        adc_file = fopen(LD_2D_VALUE_FILE_PATH, "r");
        fread(&ld_2d_data.ld_value, sizeof(ld_2d_data.ld_value), 1, adc_file);  
        usleep(1000);
    } 
    printf("LD_1D_ID = %x / LD_D1_VALUE = %x\r\n", ld_1d_data.ld_id, ld_1d_data.ld_value);
    printf("LD_2D_ID = %x / LD_D2_VALUE = %x\r\n", ld_2d_data.ld_id, ld_2d_data.ld_value);   
    uart_to_i2c_ctl(ld_1d_data.ld_id,ld_1d_data.ld_value);	         
    uart_to_i2c_ctl(ld_2d_data.ld_id,ld_2d_data.ld_value);    

}
void uart_gpio_init()
{
    char data[6] = {0};
    data[0] = 0x57;
    data[1] = 0x02;
    data[2] = 0xaa;
    data[3] = 0x03;
    data[4] = 0xaa;
    data[5] = 0x50;	

    sdcd_serial_write( 6,data);
    printf("UART_1__SETTING_TX_OK\r\n");
}

void uart_gpio_ctl(unsigned short gpio)
{
    char data[3] = {0};
    //gpio_reg = gpio;
    data[0] = 0x4f;
    data[1] = gpio;
    data[2] = 0x50;
    printf("DATA[0] = %x / DATA[1] = %x / DATA[2] = %x\r\n",  data[0], data[1], data[2]);
    sdcd_serial_write( 3,data);
}

void uart_to_i2c_ctl(unsigned char ld, unsigned value)
{
    char data[6] = {0};
    data[0] = 0x53;
    data[1] = 0x5e;
    data[2] = 0x02;
    data[3] = ld;
    data[4] = value;
    data[5] = 50;            
    printf("DATA[3] = %x / DATA[4] = %x\r\n",  data[3], data[4]);
    sdcd_serial_write(6,data);
}

void uart_gpio_all_on()
{
    char data[3] = {0};
    gpio_reg = 0xff;
    data[0] = 0x4f;
    data[1] = gpio_reg;
    data[2] = 0x50;
    sdcd_serial_write( 3,data);
}

void uart_gpio_all_off()
{
    char data[3] = {0};
    gpio_reg = 0x00;
    data[0] = 0x4f;
    data[1] = gpio_reg;
    data[2] = 0x50;
    sdcd_serial_write( 3,data);

}

void uart_gpio_0_on()
{
    char data[3] = {0};
    gpio_reg |= GPIO_0;
    data[0] = 0x4f;
    data[1] = gpio_reg;
    data[2] = 0x50;
    sdcd_serial_write( 3,data);
}

void uart_gpio_1_on()
{
    char data[3] = {0};
    gpio_reg |= GPIO_1;
    data[0] = 0x4f;
    data[1] = gpio_reg;
    data[2] = 0x50;
    sdcd_serial_write( 3,data);
}

void uart_gpio_2_on()
{
    char data[3] = {0};
    gpio_reg |= GPIO_2;
    data[0] = 0x4f;
    data[1] = gpio_reg;
    data[2] = 0x50;
    sdcd_serial_write( 3,data);
}

void uart_gpio_0_off()
{
    char data[3] = {0};
    gpio_reg &= (~GPIO_0);
    data[0] = 0x4f;
    data[1] = gpio_reg;
    data[2] = 0x50;
    sdcd_serial_write( 3,data);
}

void uart_gpio_1_off()
{
    char data[3] = {0};
    gpio_reg &= (~GPIO_1);    
    data[0] = 0x4f;
    data[1] = gpio_reg;
    data[2] = 0x50;
    sdcd_serial_write( 3,data);
}

void uart_gpio_2_off()
{
    char data[3] = {0};
    gpio_reg &= (~GPIO_2);   
    data[0] = 0x4f;
    data[1] = gpio_reg;
    data[2] = 0x50;
    sdcd_serial_write( 3,data);
}

void formula_calculation_init()
{
	int	file_size;
	FILE *adc_file;

    if(system("ls /run/media/mmcblk0p2/f0/config/z_file.txt"))
    {
       fopen(Z_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(Z_FILE_PATH, "r");
        fseek(adc_file, 0, SEEK_END);
        file_size = ftell(adc_file);
        z_data = (char *)malloc(sizeof(char)*	(file_size+1));
        z_data[file_size] = '\0';
        fseek(adc_file, 0, SEEK_SET);
        fread(z_data, file_size, 1, adc_file); 	
        set_math_expression_0(z_data);
        fclose(adc_file);
    }

    if(system("ls /run/media/mmcblk0p2/f0/config/x_file.txt"))
    {
       fopen(X_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(X_FILE_PATH, "r");
        fseek(adc_file, 0, SEEK_END);
        file_size = ftell(adc_file);
        x_data = (char *)malloc(sizeof(char)*	(file_size+1));
        x_data[file_size] = '\0';
        fseek(adc_file, 0, SEEK_SET);
        fread(x_data, file_size, 1, adc_file); 	
        set_math_expression_1(x_data);
        fclose(adc_file);
    }

    if(system("ls /run/media/mmcblk0p2/f0/config/y_file.txt"))
    {
       fopen(Y_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(Y_FILE_PATH, "r");
        fseek(adc_file, 0, SEEK_END);
        file_size = ftell(adc_file);
        y_data = (char *)malloc(sizeof(char)*	(file_size+1));
        y_data[file_size] = '\0';
        fseek(adc_file, 0, SEEK_SET);
        fread(y_data, file_size, 1, adc_file); 	
        set_math_expression_2(y_data);
        fclose(adc_file); 
    }

	add_symbol("lfz");
	add_symbol("lfx");
	add_symbol("lfy");
	add_symbol("lftx");
	add_symbol("lfty");
				
	set_register_symbol();
    
}


