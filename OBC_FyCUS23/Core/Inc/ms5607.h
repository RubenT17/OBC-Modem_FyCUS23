/**
  ******************************************************************************
  * @file           : ms5607.h
  * @brief          : MS5607 altimeter library.
  *
  * @author         Rubén Torres Bermúdez <rubentorresbermudez@gmail.com>
  ******************************************************************************
  * @attention
  *
  *  Created on:     10.09.2023
  *
  *  Description:
  *		This library is used to control a MS5607 altimeter.
  *
  *  Copyright (C) 2021 Rubén Torres Bermúdez
  ******************************************************************************
  */

#ifndef INC_MS5607_H_
#define INC_MS5607_H_

#include "main.h"
#include "math.h"


#define MS5607_ADDRESS	0b11101100

/**
 * CMD types
 */
typedef enum
{
	MS5607_CMD_RESET 	= 0x1E,  // ADC reset command
	MS5607_CMD_ADC_READ = 0x00,  // ADC read command
	MS5607_CMD_ADC_CONV = 0x40,  // ADC conversion command
	MS5607_CMD_ADC_D1 	= 0x00,  // ADC D1 conversion
	MS5607_CMD_ADC_D2 	= 0x10,  // ADC D2 conversion
	MS5607_CMD_PROM_RD 	= 0xA0,  // Prom read command
}ms5607_cmd_t;

/**
 * ADC OSR types
 */
typedef enum
{
	MS5607_ADC_256 		= 0x00,  // ADC OSR=256
	MS5607_ADC_512 		= 0x02,  // ADC OSR=512
	MS5607_ADC_1024 	= 0x04,  // ADC OSR=1024
	MS5607_ADC_2048 	= 0x06,  // ADC OSR=2048
	MS5607_ADC_4096 	= 0x08,  // ADC OSR=4096
}ms5607_adc_t;

HAL_StatusTypeDef ms5607_reset();
HAL_StatusTypeDef ms5607_readCalibration(uint16_t *cal);
HAL_StatusTypeDef ms5607_getTemp(uint8_t osr, uint16_t *cal, float *temp);
HAL_StatusTypeDef ms5607_getTempPressure(ms5607_adc_t osr, uint16_t *cal, float *temp, float *pressure);
float ms5607_getAltitude(float pressure);


#endif /* INC_MS5607_H_ */
