#include "pg.h"
#include "dac.h"

unsigned short GET_PG_STATUS(void)
{
	unsigned short result;
	
	result = pg_handle->status;
	
	return result;
}

BOOL SET_PG_CONTROL(unsigned short *data)
{
	/*
	int i;
	
	i = 0;
	while(1)
	{
		if(!(pg_handle->control & (PG_DAC_LOAD_OPERATION | PG_SIGNAL_INVERSION_CHANGE| PG_TIMING_CHANGE)) ) break;
		delayms(1);
		i++;
		if(i > 30) return FALSE;
	}
	*/
	
	pg_handle->control = *data;

	return TRUE;
}

BOOL SET_PG_SIGNAL(void)
{
	int i,ii;
	unsigned char pattern_id ;
	unsigned int vtotal;
	unsigned int inversion_data;
	unsigned int set_data;	
	unsigned short	set_data_0=0;
	unsigned short	set_data_1=0;
	unsigned short	set_data_2=0;
	
	unsigned short	inversion_data_0 = 0;
	unsigned short	inversion_data_1 = 0;
	unsigned short	inversion_data_2 = 0;	

	//pattern_id = ensis_operation.pattern_id;
	pattern_id = 0;
	/*
	i = 0;
	while(1)
	{
		if(!(pg_handle->control & (PG_DAC_LOAD_OPERATION | PG_SIGNAL_INVERSION_CHANGE| PG_TIMING_CHANGE)) ) break;
		delayms(1);
		i++;
		if(i > 30) return FALSE;
	}
	*/
	
	vtotal = 0;
	inversion_data = 0;
	set_data = 0;
	ii = 0;

	for(i=0; i<NUMBER_OF_SIGNAL; i++) if(vtotal < ensis_model_gp.pat_conf[pattern_id].sig_conf[i].time_sector[4]) vtotal = ensis_model_gp.pat_conf[pattern_id].sig_conf[i].time_sector[4]; //END
		
	debugprintf("VTOTAL = %d\r\n", vtotal);
	
	pg_handle->set_l = (unsigned short)0;
	pg_handle->set_m = (unsigned short)0;			
	pg_handle->set_h = (unsigned short)0; 		
		
	//for(i=0; i<NUMBER_OF_SIGNAL; i++)
 	for(i=0; i<NUMBER_OF_SIGNAL; i++)
	{
		if(i >=2)
		{
			pg_handle->start_l = (unsigned short)0; //START
			pg_handle->start_h = (unsigned short)0;			
			
			pg_handle->step1_low_l = (unsigned short)((ensis_model_gp.pat_conf[pattern_id].sig_conf[i].time_sector[0] >> 0) & 0x0000FFFF); //STEP1
			pg_handle->step1_low_h = (unsigned short)((ensis_model_gp.pat_conf[pattern_id].sig_conf[i].time_sector[0] >> 16) & 0x0000FFFF);
		
			pg_handle->step2_high_l = (unsigned short)((ensis_model_gp.pat_conf[pattern_id].sig_conf[i].time_sector[1] >> 0) & 0x0000FFFF); //STEP2
			pg_handle->step2_high_h = (unsigned short)((ensis_model_gp.pat_conf[pattern_id].sig_conf[i].time_sector[1] >> 16) & 0x0000FFFF);
			
			pg_handle->step3_low_l = (unsigned short)((ensis_model_gp.pat_conf[pattern_id].sig_conf[i].time_sector[2] >> 0) & 0x0000FFFF); //STEP3
			pg_handle->step3_low_h = (unsigned short)((ensis_model_gp.pat_conf[pattern_id].sig_conf[i].time_sector[2] >> 16) & 0x0000FFFF);
		
			pg_handle->step4_high_l = (unsigned short)((ensis_model_gp.pat_conf[pattern_id].sig_conf[i].time_sector[3] >> 0) & 0x0000FFFF); //STEP4
			pg_handle->step4_high_h = (unsigned short)((ensis_model_gp.pat_conf[pattern_id].sig_conf[i].time_sector[3] >> 16) & 0x0000FFFF);
		
			pg_handle->period_l = (unsigned short)((ensis_model_gp.pat_conf[pattern_id].sig_conf[i].time_sector[4] >> 0) & 0x0000FFFF); //PERIOD
			pg_handle->period_h = (unsigned short)((ensis_model_gp.pat_conf[pattern_id].sig_conf[i].time_sector[4] >> 16) & 0x0000FFFF);	
		
			pg_handle->end_l = (unsigned short)((vtotal >> 0) & 0x0000FFFF);
			pg_handle->end_h = (unsigned short)((vtotal >> 16) & 0x0000FFFF);
			if(ii < 16)
			{
				if(ensis_model_gp.pat_conf[0].sig_conf[i].inversion) inversion_data_0 |= (1<<(ii));
				set_data_0 	=	0;
				set_data_1	=	0;
				set_data_2	=	0;
				delayms(1);
				
				set_data_0 	= (1<<(ii));
				set_data_1	=	0;
				set_data_2	=	0;	
			}
			else if(ii >= 16 && ii < 32)
			{
				if(ensis_model_gp.pat_conf[0].sig_conf[i].inversion) inversion_data_1 |= (1<<(ii-16));	
				set_data_0 	=	0;
				set_data_1	=	0;
				set_data_2	=	0;
				delayms(1);
				
				set_data_0	=	0;
				set_data_1	=	(1<<(ii-16));
				set_data_2	=	0;				
			}
			else	if(ii >=32)
			{
				if(ensis_model_gp.pat_conf[0].sig_conf[i].inversion) inversion_data_2 |= (1<<(ii-32));	
				set_data_0 	=	0;
				set_data_1	=	0;
				set_data_2	=	0;
				delayms(1);
				
				set_data_0	=	0;
				set_data_1	=	0;
				set_data_2	=	(1<<(ii-32));			
			}
			pg_handle->set_l = (unsigned short)set_data_0;
			pg_handle->set_m = (unsigned short)set_data_1;			
			pg_handle->set_h = (unsigned short)set_data_2; 			
			ii++;
		}
	}

 		pg_handle->start_l = (unsigned short)0; //START
		pg_handle->start_h = (unsigned short)0;			
		
		pg_handle->step1_low_l = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[0] >> 0) & 0x0000FFFF); //STEP1
		pg_handle->step1_low_h = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[0] >> 16) & 0x0000FFFF);
	
		pg_handle->step2_high_l = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[1] >> 0) & 0x0000FFFF); //STEP2
		pg_handle->step2_high_h = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[1] >> 16) & 0x0000FFFF);
		
		pg_handle->step3_low_l = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[2] >> 0) & 0x0000FFFF); //STEP3
		pg_handle->step3_low_h = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[2] >> 16) & 0x0000FFFF);
	
		pg_handle->step4_high_l = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[3] >> 0) & 0x0000FFFF); //STEP4
		pg_handle->step4_high_h = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[3] >> 16) & 0x0000FFFF);
	
		pg_handle->period_l = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[4] >> 0) & 0x0000FFFF); //PERIOD
		pg_handle->period_h = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[4] >> 16) & 0x0000FFFF);	
	
		pg_handle->end_l =(unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[4] >> 0) & 0x0000FFFF); 
		pg_handle->end_h =  (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[4] >> 16) & 0x0000FFFF);	
		
		pg_handle->set_l = (unsigned short)0x0001;
		pg_handle->set_m = (unsigned short)0;			
		pg_handle->set_h = (unsigned short)0; 
			
	pg_handle->set_l = (unsigned short)0x0000;
	pg_handle->set_m = (unsigned short)0x0000;	
	pg_handle->set_h = (unsigned short)0x0000;
	
	pg_handle->vtotal_l = (unsigned short)((vtotal >> 0) & 0x0000FFFF);
	pg_handle->vtotal_h = (unsigned short)((vtotal >> 16) & 0x0000FFFF);
	
	pg_handle->inversion_l = inversion_data_0;
	pg_handle->inversion_m = inversion_data_1;
	pg_handle->inversion_h = inversion_data_2;	

	return TRUE;
}




BOOL VARIABLE_RGB(void)
{
	unsigned char pattern_id ;
	int i;
	pattern_id = ensis_operation.pattern_id;
	
	ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high = rgb_handle.red;
	ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high = rgb_handle.green;
	ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high = rgb_handle.blue;
	
	if(SET_DAC_OUTPUT_VALUE() == FALSE) return FALSE;

	i = 0;
	while(1)
	{
		if(!(pg_handle->control & (PG_VR_RGB_ON|PG_TIMING_CHANGE|PG_SIGNAL_INVERSION_CHANGE|PG_DAC_LOAD_OPERATION))) break;
		delayms(1);
		i++;
		if(i > 30) return FALSE;
	}
	
	pg_handle->control = (PG_VR_RGB_ON|PG_SYNC_PIN_LEVEL_CONTROL|PG_INTERRUPT_PIN_LEVEL_CONTROL|PG_DAC_LOAD_OPERATION|PG_ANALOG_MUX_ENABLE);
	return TRUE;
}
