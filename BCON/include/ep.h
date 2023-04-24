/*
 * ep.h
 *
 *  Created on: 2019. 11. 06.
 *      Author: ghkim
 */
#ifndef EP_H_
#define EP_H_

#define FW_VERSION 33
#define FW_DATE "2022.06.07"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "main.h"
#include "command.h"
#include "application.h"
#include "timer.h"
#include "flash.h"

//#define _3D		0
//#define _6D		1
#define NONE			0
//#define DATA	1
//#define OSG		2 //OSG_ACT
//#define OSG_VT	3
//#define SSD_DATA 3
#define	DATA_NEW		7
#define	DATA_77			6
#define OSG_12CLK		3
#define OSG_8CLK 		4
#define OSG_6CLK 		5
#define DATA_OSG		1
#define DATA_SSD		2
#define FORWARD			0
#define REVERSE			1
//#define BOTTOM	0
//#define TOP		1
#define LED_OFF			0
#define LED_ON			1

/*#define OUTPUT_ENABLE_NONE		0x00000000
#define OUTPUT_ENABLE_DATA		0x2033FCFF
//#define OUTPUT_ENABLE_OSG		0x1FCDDC3F //OSG_ACT
#define OUTPUT_ENABLE_OSG		0x1FFFFDFF //OSG_ACT
#define OUTPUT_ENABLE_OSG_VT	0x3FFFFFFF //OSG_VT
//#define OUTPUT_ENABLE_OSG_VT	0x2001FFFF //OSG_VT
#define OUTPUT_ENABLE_TOP		0x00300300*/

#define OUTPUT_ENABLE_NONE			(long long)0x0000000000000000
//#define OUTPUT_ENABLE_DATA		0x00000303C1FE0F03
//#define OUTPUT_ENABLE_OSG			0x000003FFFFFFFFFF
#define OUTPUT_ENABLE_SSD_DATA 		(long long)0x000003FFFFFFFFFF
#define OUTPUT_ENABLE_OSG_8CLK		(long long)0x000003FFFFFFFFFF
#define OUTPUT_ENABLE_OSG_12CLK		(long long)0x000003FFFFFFFFFF
#define OUTPUT_ENABLE_OSG_6CLK		(long long)0x000003FFFCFCFCFF
//#define OUTPUT_ENABLE_DATA_SSD		(long long)0x000003FFFFFFFF83
#define OUTPUT_ENABLE_DATA_SSD		(long long)0x00000307FFFFFFFF
#define OUTPUT_ENABLE_DATA_OSG		(long long)0x0000030041FE0803
#define OUTPUT_ENABLE_DATA_SSD_CAL	(long long)0x000003FFFFFFFFFF
#define ALL_ON						(long long)0x000003FFFFFFFFFF
#define ALL_OFF						(long long)0x0000000000000000

#define	OUTPUT_ENABLE_DATA_77		(long long)0x000003F80000007F
#define	OUTPUT_ENABLE_DATA_NEW		(long long)0x000003FFFFFFFFFF
/*typedef struct{
	unsigned char type;
	unsigned char direction;
	unsigned char el_select;
}BCON_CONFIG;

typedef struct{
	BCON_CONFIG bcon_config[60];
	unsigned char data_select;
	unsigned char led_on_off;
}MODEL_CONFIG;
MODEL_CONFIG model_config;

typedef struct{
	unsigned int bist_use_channel;
	unsigned int bist_adc_type;
}BIST_CONFIG;
BIST_CONFIG bist_config[60];
unsigned char bist_adc_value[30];*/

#pragma pack(push, 1)
typedef struct{
	unsigned char type;
	unsigned char direction;
	unsigned int bcon_output_enable_mask_1[2];
	//unsigned int bcon_output_enable_mask_2;
	//unsigned int bcon_output_enable_mask_2;
}BCON_CONFIG;

typedef struct{
	BCON_CONFIG bcon_config[48];
	unsigned char reserved;
	unsigned char led_on_off;
}MODEL_CONFIG;
MODEL_CONFIG model_config;
#pragma pack(pop)

/*typedef struct{
	long long bist_use_channel;
	long long bist_adc_type;
}BIST_CONFIG;
BIST_CONFIG bist_config[48];*/

typedef struct{
	unsigned int bist_adc_type[2];
}BIST_CONFIG;
BIST_CONFIG bist_config[48];

unsigned char bist_adc_value[42];

unsigned short bist_adc_value_jig[42];

void printf_bin(long long data);

//uint32_t bcon_output_enable_data;
long long bcon_output_enable_data;
uint16_t bcon_id;
uint8_t bcon_mode_osg_data;
uint8_t bcon_mode_direction;
//uint8_t bcon_mode_3d_6d;
//uint8_t bcon_mode_el_swap;
uint8_t bcon_led_on_off;
//uint32_t bist_mode_use;
//uint32_t bist_mode_adc_dac;
long long bist_mode_use;
//long long bist_mode_adc_dac;
long long bcon_mode_output_enable_mask;
uint32_t bist_flag;
uint32_t bist_flag2;
uint32_t bist_cnt;
uint32_t bist_cnt_1;
uint32_t ssd_data_cal_flag;
unsigned int fw_down_count;
unsigned char fw_dw_operation;
int bist_offset;
int z_offset;
float keithley_read_value;

typedef struct{
	double bist_0v_value;
	double bist_15v_value;
	double bist_0v_step;
	double bist_15v_step;
	double bist_ratio;
	double bist_offset;
	double bist_user_offset;
}BIST_CAL;
BIST_CAL bist_cal;

typedef struct{
	unsigned short id_check;
	unsigned short cmd_check;
	unsigned short length_check;
} PACKET;
PACKET packet;

typedef struct{
	unsigned int packet_total_number;
	unsigned int packet_number;
	unsigned char fw_data[1030];
} PACKET_FW;
PACKET_FW packet_fw;

typedef struct{
	unsigned int fw_flag;
	unsigned int number;
} FW_FLAG;
FW_FLAG fw_flag;

/*
#define BCON_BASE_ID 		(bcon_id & 0x00ff)
#define BCON_ID			((bcon_id >> 8) & 0x00ff)
#define RECV_BCON_BASE_ID	(packet.id_check & 0x00ff)
#define RECV_BCON_ID		((packet.id_check >> 8) & 0x00ff)

//#define BCON_BASE_ID 		((bcon_id >> 8) & 0x00ff)
//#define BCON_ID				(bcon_id & 0x00ff)
//#define RECV_BCON_BASE_ID	((packet.id_check >> 8) & 0x00ff)
//#define RECV_BCON_ID		(packet.id_check & 0x00ff)
#define SELECT_MACRO		((BCON_BASE_ID*8) + BCON_ID)
*/


//#define BCON_BASE_ID 		(bcon_id & 0x00ff)
//#define BCON_ID				((bcon_id >> 8) & 0x00ff)
#define BCON_ID 			(bcon_id & 0x00ff)
#define BCON_BASE_ID		((bcon_id >> 8) & 0x00ff)
#define RECV_BCON_BASE_ID	(packet.id_check & 0x00ff)
#define RECV_BCON_ID		((packet.id_check >> 8) & 0x00ff)
#define SELECT_MACRO		((BCON_BASE_ID*8) + BCON_ID)

#endif /* EP_H_ */
