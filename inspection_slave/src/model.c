#include "model.h"
#include "cantus.h"
#include "ep.h"
#include "pg.h"

void MODEL_PRINT(void)
{
	int i,j;
	int model_count = 0;
	unsigned int vtotal = 0;
	ENSIS_MODEL_GROUP *model = &ensis_model_gp;
	
	model_count = (int)model->pat_count;
	debugprintf("Total Pattern Count = %d\r\n", model_count);
	//for(i=0; i<model_count; i++)
	for(i=0; i<1; i++)
	{
		vtotal = 0;
		for(j=0; j<NUMBER_OF_SIGNAL; j++)
		{
			if(vtotal < model->pat_conf[i].sig_conf[j].time_sector[4]) vtotal = model->pat_conf[i].sig_conf[j].time_sector[4]; //END
		}
		debugprintf("VTOTAL = %d\r\n", (unsigned int)vtotal);
		debugprintf("Number of Pattern = %d\r\n", i);
		debugprintf("scm.ptn[%d].name = %s\r\n", i, model->pat_conf[i].pat_name);
		for(j=0; j<NUMBER_OF_SIGNAL; j++)
		{
			//debugprintf("scm.ptn[%d].sig_conf[%d].inversion = %d\r\n", i, j, model->pat_conf[i].sig_conf[j].inversion);
			debugprintf("CH%d-------------------------------------------------------------------------------------------\r\n",j);			
			debugprintf("scm.ptn[%d].sig_conf[%d].voltage_high = %d\r\n", i, j, (int)model->pat_conf[i].sig_conf[j].volt_high);
			debugprintf("scm.ptn[%d].sig_conf[%d].voltage_low = %d\r\n", i, j, (int)model->pat_conf[i].sig_conf[j].volt_low);
			debugprintf("scm.ptn[%d].sig_conf[%d].inversion = %d\r\n", i, j, model->pat_conf[i].sig_conf[j].inversion);				
			debugprintf("scm.ptn[%d].sig_conf[%d].step1 = %d\r\n", i, j, model->pat_conf[i].sig_conf[j].time_sector[0]);
			debugprintf("scm.ptn[%d].sig_conf[%d].step2 = %d\r\n", i, j, model->pat_conf[i].sig_conf[j].time_sector[1]);
			debugprintf("scm.ptn[%d].sig_conf[%d].step3 = %d\r\n", i, j, model->pat_conf[i].sig_conf[j].time_sector[2]);
			debugprintf("scm.ptn[%d].sig_conf[%d].step4 = %d\r\n", i, j, model->pat_conf[i].sig_conf[j].time_sector[3]);
			debugprintf("scm.ptn[%d].sig_conf[%d].period = %d\r\n", i, j, model->pat_conf[i].sig_conf[j].time_sector[4]);
			debugprintf("CH%d-------------------------------------------------------------------------------------------\r\n",j);				
		}
	}
}
