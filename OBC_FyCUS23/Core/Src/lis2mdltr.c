/**
  ******************************************************************************
  * @file           : lis2mdltr.c
  * @brief          : LIS2MDLTR magnetometer library.
  *
  * @author         Rubén Torres Bermúdez <rubentorresbermudez@gmail.com>
  ******************************************************************************
  * @attention
  *
  *  Created on:     29.04.2023
  *
  *  Description:
  *		This library is used to control a LIS2MDLTR magnetometer.
  *
  *  Copyright (C) 2023 Rubén Torres Bermúdez
  ******************************************************************************
  */


#include "lis2mdltr.h"


/**
 * Common configuration for LIS2MDL
 * @param hi2c Pointer to a I2C_HandleTypeDef structure
 * @param DevAddress Device address
 * @param Timeout Timeout duration
 * @return HAL status
 */
HAL_StatusTypeDef lis2mdl_config(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout)
{
	HAL_StatusTypeDef err = HAL_OK;
	Timeout = Timeout/4;
	uint8_t data = 0;

	// Active Zyxda to DRDY pin
	data = 0b00010001;
	err = HAL_I2C_Mem_Write(hi2c, DevAddress, LIS2MDL_CFG_REG_C, 1, &data, 1, Timeout);
	if(err != HAL_OK) return err;
	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_CFG_REG_C, 1, &data, 1, Timeout);
	if(err != HAL_OK) return err;

	// Configure the output data rate and the measurement configuration
	data = 0b10000000;
	err = HAL_I2C_Mem_Write(hi2c, DevAddress, LIS2MDL_CFG_REG_A, 1, &data, 1, Timeout);
	if(err != HAL_OK) return err;
	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_CFG_REG_A, 1, &data, 1, Timeout);
	if(err != HAL_OK) return err;


//	err  = HAL_I2C_Mem_Write(hi2c, DevAddress, LIS2MDL_INT_CRTL_REG, 1, (uint8_t *)0b11100001, 1, Timeout);
//	if(err != HAL_OK) return err;

	return HAL_OK;
}

/**
 * Read status register
 * @param hi2c Pointer to a I2C_HandleTypeDef structure
 * @param DevAddress Device address
 * @param Timeout Timeout duration
 * @return HAL status
 */
uint8_t lis2mdl_status(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout)
{
	HAL_StatusTypeDef err = HAL_OK;
	uint8_t data = 0;
	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_STATUS_REG, 1, &data, 1, Timeout);
	if(err != HAL_OK) return err;

	return data;
}

/**
 * Perform a single conversion
 * @param hi2c Pointer to a I2C_HandleTypeDef structure
 * @param DevAddress Device address
 * @param Timeout Timeout duration
 * @return HAL status
 */
HAL_StatusTypeDef lis2mdl_single_conversion(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout)
{
	HAL_StatusTypeDef err = HAL_OK;

	uint8_t data = 0b1000001;
	err = HAL_I2C_Mem_Write(hi2c, DevAddress, LIS2MDL_CFG_REG_A, 1, &data, 1, Timeout);
	if(err != HAL_OK) return err;
	return HAL_ERROR;
}


/**
 * Read magnetometer sensor
 * @param hi2c Pointer to a I2C_HandleTypeDef structure
 * @param DevAddress Device address
 * @param Timeout Timeout duration
 * @param x Pointer to x-axis value
 * @param y Pointer to y-axis value
 * @param z Pointer to z-axis value
 * @return HAL status
 */
HAL_StatusTypeDef lis2mdl_readMag(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout, int16_t *x, int16_t *y, int16_t *z)
{
	HAL_StatusTypeDef err = HAL_OK;

	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_OUTX_L_REG, 1, (uint8_t *)x, 2, Timeout);
	if(err != HAL_OK) return err;

	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_OUTY_L_REG, 1, (uint8_t *)y, 2, Timeout);
	if(err != HAL_OK) return err;

	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_OUTZ_L_REG, 1, (uint8_t *)z, 2, Timeout);
	if(err != HAL_OK) return err;


	return HAL_OK;
}


/**
 * Read temperature sensor
 * @param hi2c Pointer to a I2C_HandleTypeDef structure
 * @param DevAddress Device address
 * @param Timeout Timeout duration
 * @param temp Pointer to temperature
 * @return HAL status
 */
HAL_StatusTypeDef lis2mdl_readTemp(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout, float *temp)
{
	uint8_t data[2];
	HAL_StatusTypeDef err = HAL_OK;

	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_TEMP_OUT_L_REG, 1, data, 2, Timeout);
	if(err != HAL_OK) return err;


	return HAL_OK;

}

/**
 * Read WHO I AM register
 * @param hi2c Pointer to a I2C_HandleTypeDef structure
 * @param DevAddress Device address
 * @param Timeout Timeout duration
 * @return Device response
 */
uint8_t lis2mdl_whoIam(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout)
{
	uint8_t id=0;
	HAL_I2C_Mem_Read(&hi2c1, DevAddress, LIS2MDL_WHO_AM_I, 1, &id, 1, Timeout);
	return id;
}


/**
 * Software reset
 * @param hi2c Pointer to a I2C_HandleTypeDef structure
 * @param DevAddress Device address
 * @param Timeout Timeout duration
 * @return HAL status
 */
uint8_t lis2mdl_resetSoftware(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout)
{
	uint8_t data = 0b00100000;
	HAL_StatusTypeDef err = HAL_I2C_Mem_Write(hi2c, DevAddress, LIS2MDL_CFG_REG_A, 1, &data, 1, Timeout);
	if(err != HAL_OK) 	return err;
	else 				return HAL_OK;
}

/**
 * Reboot memory contents
 * @param hi2c Pointer to a I2C_HandleTypeDef structure
 * @param DevAddress Device address
 * @param Timeout Timeout duration
 * @return HAL status
 */
HAL_StatusTypeDef lis2mdl_rebootMemory(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout)
{
	uint8_t data = 0;
	HAL_StatusTypeDef err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_CFG_REG_A, 1, &data, 1, Timeout);
	if(err==HAL_OK)
	{
		data |= 0b01000000;
		err = HAL_I2C_Mem_Write(hi2c, DevAddress, LIS2MDL_CFG_REG_A, 1, &data, 1, Timeout);
	}

	if(err != HAL_OK) 	return err;
	else 				return HAL_OK;
}


/**
 * Self-test utility (NO TESTED FUNCTION)
 * @param hi2c Pointer to a I2C_HandleTypeDef structure
 * @param DevAddress Device address
 * @param Timeout Timeout duration
 * @return HAL status
 */
uint8_t lis2mdl_selfTest(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout)
{
	uint8_t data = 0;
	uint8_t status = 0;
	HAL_StatusTypeDef err = lis2mdl_resetSoftware(hi2c, DevAddress, Timeout);
	if(err != HAL_OK) 	return err;

	// Reset registers, check 2 times
	err = HAL_I2C_Mem_Write(hi2c, DevAddress, LIS2MDL_CFG_REG_A, 1, &data, 1, Timeout);
	if(err != HAL_OK) 			return err;
	else if(data & 0b00100000) 	return HAL_ERROR;

	// Initialize sensor
	err = HAL_I2C_Mem_Write(hi2c, DevAddress, LIS2MDL_CFG_REG_A, 1, (uint8_t *)0x8C, 1, Timeout);
	if(err != HAL_OK) 	return err;
	err = HAL_I2C_Mem_Write(hi2c, DevAddress, LIS2MDL_CFG_REG_B, 1, (uint8_t *)0x02, 1, Timeout);
	if(err != HAL_OK) 	return err;
	err = HAL_I2C_Mem_Write(hi2c, DevAddress, LIS2MDL_CFG_REG_C, 1, (uint8_t *)0x10, 1, Timeout);
	if(err != HAL_OK) 	return err;

	// Power up
	HAL_Delay(21);

	// Check Zyxda
	status = 0;
	while(!status)	status = lis2mdl_status(hi2c, DevAddress, Timeout);
	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_OUTX_H_REG, 1, &data, 1, Timeout);
	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_OUTY_H_REG, 1, &data, 1, Timeout);
	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_OUTZ_H_REG, 1, &data, 1, Timeout);

	// Save average data on each axis
	for(uint8_t i=0; i<50; i++)
		status = lis2mdl_status(hi2c, DevAddress, Timeout);

	status = 0;
	while(!(status&0b00001000))
		status = lis2mdl_status(hi2c, DevAddress, Timeout);
	uint8_t outx_NoST[2] = {0};
	uint8_t outy_NoST[2] = {0};
	uint8_t outz_NoST[2] = {0};
	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_OUTX_L_REG, 1, outx_NoST, 2, Timeout);
	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_OUTY_L_REG, 1, outy_NoST, 2, Timeout);
	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_OUTZ_L_REG, 1, outz_NoST, 2, Timeout);



	// Enable self-test
	err = HAL_I2C_Mem_Write(hi2c, DevAddress, LIS2MDL_CFG_REG_C, 1, (uint8_t *)0x12, 1, Timeout);
	if(err != HAL_OK) 	return err;
	HAL_Delay(61);

	// Check Zyxda
	status = 0;
	while(!status)	status = lis2mdl_status(hi2c, DevAddress, Timeout);
	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_OUTX_H_REG, 1, &data, 1, Timeout);
	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_OUTY_H_REG, 1, &data, 1, Timeout);
	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_OUTZ_H_REG, 1, &data, 1, Timeout);

	// Save average data on each axis
	for(uint8_t i=0; i<50; i++)
		status = lis2mdl_status(hi2c, DevAddress, Timeout);

	status = 0;
	while(!(status&0b00001000))
		status = lis2mdl_status(hi2c, DevAddress, Timeout);
	uint8_t outx_ST[2] = {0};
	uint8_t outy_ST[2] = {0};
	uint8_t outz_ST[2] = {0};
	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_OUTX_L_REG, 1, outx_ST, 2, Timeout);
	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_OUTY_L_REG, 1, outy_ST, 2, Timeout);
	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LIS2MDL_OUTZ_L_REG, 1, outz_ST, 2, Timeout);


	// Checking test
	uint16_t diffx = abs(outx_ST-outx_NoST);
	uint16_t diffy = abs(outy_ST-outy_NoST);
	uint16_t diffz = abs(outz_ST-outz_NoST);

	if((15 <= diffx) && (diffx <= 500) &&
			(15 <= diffy) && (diffy <= 500) &&
			(15 <= diffz) && (diffz <= 500) )
		return HAL_OK;

	else return HAL_ERROR;
}
