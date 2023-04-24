/*
 * flash.c
 *
 *  Created on: 2019. 11. 14.
 *      Author: ghkim
 */

#include "flash.h"

void FLASH_If_Init(void)
{
  /* Unlock the Program memory */
  HAL_FLASH_Unlock();

  /* Clear all FLASH flags */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
  /* Unlock the Program memory */
  HAL_FLASH_Lock();
}

uint32_t FLASH_If_Erase(uint32_t start, uint32_t size)
{
  uint32_t NbrOfPages = 0;
  uint32_t PageError = 0;
  FLASH_EraseInitTypeDef pEraseInit;
  HAL_StatusTypeDef status = HAL_OK;

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  /* Get the sector where start the user flash area */
  NbrOfPages = (size/FLASH_PAGE_SIZE) + 1;

  pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
  pEraseInit.PageAddress = start;
  //pEraseInit.Banks = FLASH_BANK_1;
  pEraseInit.NbPages = NbrOfPages;
  status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  if (status != HAL_OK)
  {
    /* Error occurred while page erase */
    return HAL_ERROR;
  }

  return HAL_OK;
}


uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length)
{
  uint32_t i = 0;

  /* Unlock the Flash to enable the flash control register access *************/

  if(HAL_FLASH_Unlock() == HAL_OK)
  {
	  for (i = 0; (i < length) && (destination <= (USER_FLASH_END_ADDRESS-4)); i++)
	  {
		/* Device voltage range supposed to be [2.7V to 3.6V], the operation will
		   be done by word */
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, destination, *(uint32_t*)(p_source+i)) == HAL_OK)
		{
		 /* Check the written value */
		  if (*(uint32_t*)destination != *(uint32_t*)(p_source+i))
		  {
			/* Flash content doesn't match SRAM content */
			  printf("FLASH_Verify_Fail\r\n");
			  return(HAL_ERROR);
		  }
		  /* Increment FLASH destination address */
		  destination += 4;
		}
		else
		{
		  /* Error occurred while writing data in Flash memory */
			printf("FLASH_Write_Fail\r\n");
			return (HAL_ERROR);
		}
	  }

	  /* Lock the Flash to disable the flash control register access (recommended
		 to protect the FLASH memory against possible unwanted operation) *********/
	  if(HAL_FLASH_Lock() == HAL_OK) return (HAL_OK);
	  else
	  {
		  printf("lock Fail\r\n");
		  return (HAL_ERROR);
	  }
  }
  else
  {
	  printf("Unlock Fail\r\n");
	  return (HAL_ERROR);
  }

}

