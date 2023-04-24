/*
 * application.h
 *
 *  Created on: 2019. 6. 24.
 *      Author: ghkim
 */

#ifndef INC_APPLICATION_H_
#define INC_APPLICATION_H_

#include "ep.h"

extern ADC_HandleTypeDef hadc;

void ep496_init(void);
uint16_t bcon_id_read(void);
//void bcon_output_enable(uint32_t data_30bit);
void bcon_output_enable(long long data_64bit);
//void bcon_mode_select(uint8_t osg_data, uint8_t direction, uint8_t _3d_6d, uint8_t el_swap, uint8_t led_on_off);
void bcon_mode_select(uint8_t osg_data, uint8_t direction, uint8_t led_on_off, long long on_off_mask);
//void bist_output_enable(uint32_t data_30bit);
void bist_output_enable(long long data_64bit);
HAL_StatusTypeDef io_check(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
#endif /* INC_APPLICATION_H_ */
