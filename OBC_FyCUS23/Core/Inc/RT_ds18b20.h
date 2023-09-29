/**
  ******************************************************************************
  * @file           : RT_ds18b20.c
  * @brief          : DS18B20 digital thermometer library for STM32 HAL.
  *
  * @author         Rubén Torres Bermúdez <rubentorresbermudez@gmail.com>
  ******************************************************************************
  * @attention
  *
  *  Created on:     13.09.2021
  *
  *  Description:
  *		This library is used to control one or more DS18B20 digital thermometers
  *		with STM32 HAL.
  *
  *		Basic example:
  *		DS18B20_Reset_CMD(DS_GPIO_Port, DS_Pin);
  *		DS18B20_ReadROM_CMD(DS_GPIO_Port, DS_Pin, dirb);
  *		DS18B20_Reset_CMD(DS_GPIO_Port, DS_Pin);
  *		DS18B20_SetResolution(DS_GPIO_Port, DS_Pin, dirb, DS18B20_RESOLUTION_10);
  *		DS18B20_All_Convert(DS_GPIO_Port, DS_Pin);
  *		temp = DS18B20_Read_Temp(DS_GPIO_Port, DS_Pin, dirb);
  *
  *		Warning:
  *		Before sending each command (ending in _CMD()), use the DS18B20_Reset_CMD()
  *		function.
  *		The GPIO used must be OUTPUT OPEN DRAIN and HIGH output speed.
  *		Configure the delayMicros() function in your main.c or in RT_ds18b20.c
  *
  *  Copyright (C) 2021 Rubén Torres Bermúdez
  ******************************************************************************
  */


#ifndef RT_DS18B20_H
#define RT_DS18B20_H

#include "main.h"
#include <math.h>

#define DS18B20_CONVERSION_ERROR 	99.0
#define DS18B20_ROM_SIZE		 	8

typedef enum {
	DS18B20_SEARCH_ROM	=		0xF0,
	DS18B20_READ_ROM	=		0x33,
	DS18B20_MATCH_ROM	=		0x55,
	DS18B20_SKIP_ROM	=		0xCC,
	DS18B20_ALARM_SEARCH =		0xEC,
	DS18B20_CONVERT_T	 =      0x44,
	DS18B20_WRITE_SCRATCHPAD =	0x4E,
	DS18B20_READ_SCRATCHPAD	=	0xBE,
	DS18B20_COPY_SCRATCHPAD	=	0x48,
	DS18B20_RECALL_E2		=	0xB8,
	DS18B20_READ_POWERSUPPLY =	0xB4,
} ds18b20_cmd_t;


typedef enum {
	DS18B20_RESOLUTION_9 	= 	0b00011111,
	DS18B20_RESOLUTION_10 	= 	0b00111111,
	DS18B20_RESOLUTION_11 	= 	0b01011111,
	DS18B20_RESOLUTION_12 	= 	0b01111111,
} ds18b20_resolution_t;



HAL_StatusTypeDef DS18B20_Reset_CMD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
HAL_StatusTypeDef DS18B20_ConvertT_CMD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
HAL_StatusTypeDef DS18B20_ReadScratchpad_CMD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin,
		uint8_t *data, uint8_t n_bytes);
void DS18B20_ReadROM_CMD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t* dirb);
void DS18B20_MatchROM_CMD (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t *dirb);
void DS18B20_SkipROM_CMD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void DS18B20_CopyScratchpad_CMD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void DS18B20_RecallE2_CMD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

HAL_StatusTypeDef DS18B20_All_Convert(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
HAL_StatusTypeDef DS18B20_SetResolution(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t *dirb,
		ds18b20_resolution_t resol);
float DS18B20_Decode_Temperature (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t *data);
float DS18B20_Read_Temp(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t* dirb);
float DS18B20_ConvertRead_Temp(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t *dirb);


#endif /* RT_DS18B20_H */
