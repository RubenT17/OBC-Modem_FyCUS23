/**
  ******************************************************************************
  * @file           : ms5607.c
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


#include "ms5607.h"


/**
 * Perform a single conversion with a specified CMD
 * @param cmd Command for conversion. See
 * @return HAL status
 */
static inline HAL_StatusTypeDef ms5607_conversion(uint8_t cmd)
{
	uint8_t data = cmd; //(CMD_ADC_CONV+CMD_ADC_D1+CMD_ADC_512)
	HAL_StatusTypeDef err = HAL_I2C_Master_Transmit(&hi2c1, MS5607_ADDRESS, &data, 1, 10);
	if (err != HAL_OK)	return HAL_ERROR;
	else 				return HAL_OK;
}


/**
 * Read a specified length of memory
 * @param length Memory length for read
 * @return HAL status
 */
static inline HAL_StatusTypeDef ms5607_read(uint8_t length)
{
	uint8_t data = MS5607_CMD_ADC_READ;
	HAL_StatusTypeDef err = HAL_I2C_Master_Transmit(&hi2c1, MS5607_ADDRESS, &data, 1, 10);
	if (err != HAL_OK)	return HAL_ERROR;
	else 				return HAL_OK;
}


/**
 * Get raw data from MS5607
 * @param length Length for read
 * @param data Pointer to read data
 * @return HAL status
 */
static inline HAL_StatusTypeDef ms5607_getRawData(uint8_t length, uint8_t *data)
{
	HAL_StatusTypeDef err = HAL_I2C_Master_Receive(&hi2c1, MS5607_ADDRESS, data, length, 100);
	if (err != HAL_OK)	return HAL_ERROR;
	else 				return HAL_OK;
}


/**
 * Read a coefficient number from PROM
 * @param coef_num Coefficient number for read
 * @return HAL status
 */
static inline HAL_StatusTypeDef ms5607_readPROM(uint8_t coef_num)
{
	uint8_t data = MS5607_CMD_PROM_RD+coef_num*2;
	HAL_StatusTypeDef err = HAL_I2C_Master_Transmit(&hi2c1, MS5607_ADDRESS, &data, 1, 10); // send PROM READ command
	if (err != HAL_OK)	return HAL_ERROR;
	else 				return HAL_OK;
}


/**
 * Perform a device reset
 * @return HAL status
 */
HAL_StatusTypeDef ms5607_reset()
{
	uint8_t data = MS5607_CMD_RESET;
	HAL_StatusTypeDef err = HAL_I2C_Master_Transmit(&hi2c1, MS5607_ADDRESS, &data, 1, 10);
	if (err != HAL_OK)	return HAL_ERROR;
	else 				return HAL_OK;
}

/**
 * Read calibration values from PROM
 * @param cal Pointer to calibration values
 * @return HAL status
 */
HAL_StatusTypeDef ms5607_readCalibration(uint16_t *cal)
{
	uint8_t raw_data[2] = {0};

	for(uint8_t i=0; i<6; i++)
	{
		if(ms5607_readPROM((i+1)) != HAL_OK)			return HAL_ERROR;
		if(ms5607_getRawData(2, raw_data) != HAL_OK)	return HAL_ERROR;
		cal[i] = raw_data[0]<<8 | raw_data[1];
	}

	return HAL_OK;
}

/**
 * Perform a conversion and read temperature
 * @param osr OSR set for ADC
 * @param cal Calibration values
 * @param temp Pointer to temperature
 * @return HAL status
 */
HAL_StatusTypeDef ms5607_getTemp(uint8_t osr, uint16_t *cal, float *temp)
{
	uint8_t raw_data[3] = {0};

	// Get raw temp
	if(ms5607_conversion(MS5607_CMD_ADC_CONV+MS5607_CMD_ADC_D2+osr) != HAL_OK)	return HAL_ERROR;
	HAL_Delay(10);
	if(ms5607_read(3) != HAL_OK)	return HAL_ERROR;
	if(ms5607_getRawData(3, raw_data) != HAL_OK)	return HAL_ERROR;
	uint32_t DT = (raw_data[0]<<16) | (raw_data[1]<<8) | raw_data[2];

    float dT = (float)DT - ((float)cal[4])*((int)1<<8);
    *temp = (2000.0 + dT * ((float)cal[5])/(float)((long)1<<23))/100 ;

    return HAL_OK;
}


/**
 * Perform two conversions and read temperature and pressure
 * @param osr OSR set for ADC
 * @param cal Calibration values
 * @param temp Pointer to temperature
 * @param pressure Pointer to pressure
 * @return HAL status
 */
HAL_StatusTypeDef ms5607_getTempPressure(uint8_t osr, uint16_t *cal, float *temp, float *pressure)
{
	uint8_t raw_data[3] = {0};

	// Get raw temp
	if(ms5607_conversion(MS5607_CMD_ADC_CONV+MS5607_CMD_ADC_D2+osr) != HAL_OK)	return HAL_ERROR;
	HAL_Delay(10);
	if(ms5607_read(3) != HAL_OK)	return HAL_ERROR;
	if(ms5607_getRawData(3, raw_data) != HAL_OK)	return HAL_ERROR;
	uint32_t DT = (raw_data[0]<<16) | (raw_data[1]<<8) | raw_data[2];

	// Get raw pressure
	if(ms5607_conversion(MS5607_CMD_ADC_CONV+MS5607_CMD_ADC_D1+osr) != HAL_OK)	return HAL_ERROR;
	HAL_Delay(10);
	if(ms5607_read(3) != HAL_OK)				return HAL_ERROR;
	if(ms5607_getRawData(3, raw_data) != HAL_OK)return HAL_ERROR;
	uint32_t DP = (raw_data[0]<<16) | (raw_data[1]<<8) | raw_data[2];


    float dT = (float)DT - ((float)cal[4])*((int)1<<8);
    *temp = (2000.0 + dT * ((float)cal[5])/(float)((long)1<<23))/100 ;

    float OFF = (((int64_t)cal[1])*((long)1<<17)) + dT * ((float)cal[3])/((int)1<<6);
    float SENS = ((float)cal[0])*((long)1<<16) + dT * ((float)cal[2])/((int)1<<7);
    float pa = (float)((float)DP/((long)1<<15));
    float pb = (float)(SENS/((float)((long)1<<21)));
    float pc = pa*pb;
    float pd = (float)(OFF/((float)((long)1<<15)));
    *pressure = (pc - pd)/100;


    return HAL_OK;
}

/**
 * Convert pressure to altitude
 * @param pressure Pressure to conversion to altitude
 * @return Altitude conversion
 */
float ms5607_getAltitude(float pressure)
{
	// Intersema Application Note 501
	if(pressure > 265)
		return ( 44397.53466872111 - 44388.28967642527*pow(pressure*0.00098726429, 0.1902587519) );
	else
		return ( (1.73 - log(pressure*0.00442086649)) * 6369.426751592357);
}

