#include "model.h"
#include "cantus.h"
#include "ep.h"
#include "pg.h"

BOOL MODEL_SAVE(void)
{
	FATFS fs;
	FIL fsrc;
	FIL *fp = &fsrc;
	DIR dir;
	FRESULT result;
	ENSIS_MODEL_GROUP *model = &ensis_model_gp;
	unsigned int write_byte;
	
	if(f_mount(DRIVE_NAND, &fs) != FR_OK)
	{
		debugprintf("Mount Error\r\n");
		return FALSE;
	}
	
	result = f_opendir(&dir, MODEL_FILE_FOLDER);
	if(result != FR_OK) f_mkdir(MODEL_FILE_FOLDER);
	
	result = f_open(fp, MODEL_FILE_NAME, FA_CREATE_ALWAYS|FA_WRITE);
	if(result != FR_OK)
	{
		debugprintf("f_open Error\r\n");
		f_mount(DRIVE_NAND,NULL);
		return FALSE;
	}
	
	result = f_write(fp, model, sizeof(ensis_model_gp), &write_byte);
	if(result != FR_OK)
	{
		debugprintf("f_write Error\r\n");
		f_close(fp);
		f_mount(DRIVE_NAND,NULL);
		return FALSE;
	}
	debugprintf("%s %dbyte write ok\r\n", MODEL_FILE_NAME, write_byte);
	
	result = f_close(fp);
	if(result != FR_OK)
	{
		f_close(fp);
		f_mount(DRIVE_NAND,NULL);
		return FALSE;
	}
	
	return TRUE;
}

BOOL MODEL_LOAD(void)
{
	FATFS fs;
	FIL fp;
	FRESULT result;
	unsigned int read_byte;
	
	if(f_mount(DRIVE_NAND, &fs) != FR_OK)
	{
		debugprintf("Mount Error\r\n");
		return FALSE;
	}
	
	result = f_open(&fp, MODEL_FILE_NAME, FA_READ|FA_OPEN_EXISTING);
	if(result != FR_OK)
	{
		debugprintf("f_open Error\r\n");
		f_mount(DRIVE_NAND,NULL);
		return FALSE;
	}
	
	result = f_read(&fp, &ensis_model_gp, sizeof(ensis_model_gp), &read_byte);
	if(result != FR_OK)
	{
		debugprintf("f_read Error\r\n");
		f_close(&fp);
		f_mount(DRIVE_NAND,NULL);
		return FALSE;
	}
	debugprintf("%s %dbyte read ok\r\n", MODEL_FILE_NAME, read_byte);
	
	result = f_close(&fp);
	if(result != FR_OK)
	{
		f_close(&fp);
		f_mount(DRIVE_NAND,NULL);
		return FALSE;
	}
	return TRUE;
}

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
			debugprintf("scm.ptn[%d].sig_conf[%d].inversion = %d\r\n", i, j, model->pat_conf[i].sig_conf[j].inversion);
			debugprintf("scm.ptn[%d].sig_conf[%d].voltage_high = %d\r\n", i, j, (int)model->pat_conf[i].sig_conf[j].volt_high);
			debugprintf("scm.ptn[%d].sig_conf[%d].voltage_low = %d\r\n", i, j, (int)model->pat_conf[i].sig_conf[j].volt_low);
			debugprintf("scm.ptn[%d].sig_conf[%d].start = %d\r\n", i, j, model->pat_conf[i].sig_conf[j].time_sector[0]);
			debugprintf("scm.ptn[%d].sig_conf[%d].period = %d\r\n", i, j, model->pat_conf[i].sig_conf[j].time_sector[1]);
			debugprintf("scm.ptn[%d].sig_conf[%d].delay = %d\r\n", i, j, model->pat_conf[i].sig_conf[j].time_sector[2]);
			debugprintf("scm.ptn[%d].sig_conf[%d].width = %d\r\n", i, j, model->pat_conf[i].sig_conf[j].time_sector[3]);
			debugprintf("scm.ptn[%d].sig_conf[%d].end = %d\r\n", i, j, model->pat_conf[i].sig_conf[j].time_sector[4]);
		}
	}
}
