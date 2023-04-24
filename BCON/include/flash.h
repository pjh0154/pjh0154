/*
 * flash.h
 *
 *  Created on: 2019. 11. 14.
 *      Author: ghkim
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "ep.h"

//#define OFFSET_ADDRESS_1		0X08020000
#define OFFSET_ADDRESS			0x08020000
#define OFFSET_ADDRESS_2 		0X08020800
#define OFFSET_ADDRESS_3 		0X08021600
#define USER_FLASH_END_ADDRESS	0x08040000
#define FW_UPDATA_FLAG			0x08024000
#define FW_UPDATA_ADDRESS		0x08028000

void FLASH_If_Init(void);
uint32_t FLASH_If_Erase(uint32_t start, uint32_t size);
uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length);

#endif /* INC_FLASH_H_ */
