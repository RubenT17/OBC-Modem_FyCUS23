/**
  ******************************************************************************
  * @file           : lsm6dso.c
  * @brief          : LSM6DSO IMU library.
  *
  * @author         Rubén Torres Bermúdez <rubentorresbermudez@gmail.com>
  ******************************************************************************
  * @attention
  *
  *  Created on:     25.06.2023
  *
  *  Description:
  *		This library is used to control a LSM6DSO IMU.
  *
  *  Copyright (C) 2023 Rubén Torres Bermúdez
  ******************************************************************************
  */

#ifndef INC_LSM6DSO_C_
#define INC_LSM6DSO_C_

#include "lsm6dso.h"


/**
 * Common configuration for LSM6DSO
 * @param hi2c Pointer to a I2C_HandleTypeDef structure
 * @param DevAddress Device address
 * @param Timeout Timeout duration
 * @return HAL status
 */
HAL_StatusTypeDef lsm6dso_config(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout)
{
	HAL_StatusTypeDef err = 0;


//	// Desactiva I3C
//	err = HAL_I2C_Mem_Write(hi2c, DevAddress, LSM6DSO_CTRL9_XL, 1, (uint8_t *)0b11100010, 1, Timeout);
//	if(err != HAL_OK) return err;
//
//	// Activa pull-up en SDO
//	err = HAL_I2C_Mem_Write(hi2c, DevAddress, LSM6DSO_PIN_CTRL, 1, (uint8_t *)0b01111111, 1, Timeout);
//	if(err != HAL_OK) return err;
//
//	// Configuración de Acelerómetro
//	uint8_t accel_config = LSM6DSO_XL_ODR_104Hz<<4 | LSM6DSO_8g<<2 | LPF2_XL_DIS<<1 | 0;
//	err = HAL_I2C_Mem_Write(hi2c, DevAddress, LSM6DSO_CTRL1_XL, 1, (uint8_t *)&accel_config, 1, Timeout);
//	if(err != HAL_OK) return err; // Se puede poner en high performance o en low power
//
//	// Configuración de Gyro
//	uint8_t gyro_config = LSM6DSO_GY_ODR_104Hz<<4 | LSM6DSO_2000dps<<2 | 0<<1 | 0;
//	err = HAL_I2C_Mem_Write(hi2c, DevAddress, LSM6DSO_CTRL2_G, 1, (uint8_t *)&gyro_config, 1, Timeout);
//	if(err != HAL_OK) return err; // Se puede poner en high performance o en low power
//
//	// Configuración de interrupciones y Data Update
//	err = HAL_I2C_Mem_Write(hi2c, DevAddress, LSM6DSO_CTRL3_C, 1, (uint8_t *)0b00000100, 1, Timeout);
//	if(err != HAL_OK) return err;

	uint8_t data = -1;

	// INT1 -> accel data ready
	err = HAL_I2C_Mem_Write(hi2c, DevAddress, LSM6DSO_INT1_CTRL, 1, (uint8_t *)0b00000001, 1, Timeout);
	if(err != HAL_OK) return err;

	// INT2 -> gyro data ready
	err = HAL_I2C_Mem_Write(hi2c, DevAddress, LSM6DSO_INT2_CTRL, 1, (uint8_t *)0b00000010, 1, Timeout);
	if(err != HAL_OK) return err;

	// Configuración de Acelerómetro
	if(err==0)
	{	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LSM6DSO_CTRL1_XL, 1, &data, 1, Timeout);
		if(err==0)
		{
			data = LSM6DSO_XL_ODR_12Hz5<<4 | LSM6DSO_4g<<2 | LPF2_XL_DIS<<1 | 0;
			err = HAL_I2C_Mem_Write(hi2c, DevAddress, LSM6DSO_CTRL1_XL, 1, &data, 1, Timeout);
		}
	}

	// Configuración de Gyro
	if(err==0)
	{	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LSM6DSO_CTRL2_G, 1, &data, 1, Timeout);
		if(err==0)
		{
			data = LSM6DSO_GY_ODR_12Hz5<<4 | LSM6DSO_250dps<<1  | 0;
			err = HAL_I2C_Mem_Write(hi2c, DevAddress, LSM6DSO_CTRL2_G, 1, &data, 1, Timeout);
		}
	}

	if(err==0)
	{	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LSM6DSO_CTRL9_XL, 1, &data, 1, Timeout);
		if(err==0)
		{
			data |= 0b00000010;
			err = HAL_I2C_Mem_Write(hi2c, DevAddress, LSM6DSO_CTRL9_XL, 1, &data, 1, Timeout);
		}
	}
	if(err==0)
	{
		err = HAL_I2C_Mem_Read(hi2c, DevAddress, LSM6DSO_I3C_BUS_AVB, 1, &data, 1, Timeout);
		if(err==0)
		{
			data &= 0b00000000;
			err = HAL_I2C_Mem_Write(hi2c, DevAddress, LSM6DSO_I3C_BUS_AVB, 1, &data, 1, Timeout);
		}
	}
	if(err==0)
	{
		err = HAL_I2C_Mem_Read(hi2c, DevAddress, LSM6DSO_CTRL3_C, 1, &data, 1, Timeout);
		if(err==0)
		{
			data |= 0b01000100;
			err = HAL_I2C_Mem_Write(hi2c, DevAddress, LSM6DSO_CTRL3_C, 1, &data, 1, Timeout);
		}
	}
	if(err==0)
	{
		err = HAL_I2C_Mem_Read(hi2c, DevAddress, LSM6DSO_FIFO_CTRL4, 1, &data, 1, Timeout);
		if(err==0)
		{
			data &= 0b11110000;
			err = HAL_I2C_Mem_Write(hi2c, DevAddress, LSM6DSO_FIFO_CTRL4, 1, &data, 1, Timeout);
		}
	}



	return HAL_OK;
}





/**
 * Read linear acceleration sensor
 * @param hi2c Pointer to a I2C_HandleTypeDef structure
 * @param DevAddress Device address
 * @param Timeout Timeout duration
 * @param x Pointer to x-axis value
 * @param y Pointer to y-axis value
 * @param z Pointer to z-axis value
 * @return HAL status
 */
HAL_StatusTypeDef lsm6dso_readAccel(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout, int16_t *x, int16_t *y, int16_t *z)
{
	uint8_t data[6] = {0};
	HAL_StatusTypeDef err = 0;

	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LSM6DSO_OUTX_L_A, 1, data, 6, Timeout);
	if(err != HAL_OK) return err;

//	float dato = ((float)((float)(((int16_t)(data[1]<<8) | data[0])) * LSM6DSO_ACC_SENSITIVITY_FS_8G));
	*x = ((data[1]<<8) | data[0]);
	*y = ((data[3]<<8) | data[2]);
	*z = ((data[5]<<8) | data[4]);

	return HAL_OK;
}


/**
 * Read angular acceleration sensor
 * @param hi2c Pointer to a I2C_HandleTypeDef structure
 * @param DevAddress Device address
 * @param Timeout Timeout duration
 * @param x Pointer to x-axis value
 * @param y Pointer to y-axis value
 * @param z Pointer to z-axis value
 * @return HAL status
 */
HAL_StatusTypeDef lsm6dso_readGyro(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout, int16_t *x, int16_t *y, int16_t *z)
{
	uint8_t data[6] = {0};
	HAL_StatusTypeDef err = 0;

	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LSM6DSO_OUTX_L_G, 1, data, 6, Timeout);
	if(err != HAL_OK) return err;

	*x = ((data[1]<<8) | data[0]);
	*y = ((data[3]<<8) | data[2]);
	*z = ((data[5]<<8) | data[4]);

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
HAL_StatusTypeDef lsm6dso_readTemp(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout, int16_t *temp)
{
	uint8_t data[2] = {0};
	HAL_StatusTypeDef err = 0;

	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LSM6DSO_OUT_TEMP_L, 1, (uint8_t *)&data, 2, Timeout);
	if(err != HAL_OK) return err;

	*temp = (data[1]<<8) | data[0];

	return HAL_OK;
}

/**
 * Read status register
 * @param hi2c Pointer to a I2C_HandleTypeDef structure
 * @param DevAddress Device address
 * @param Timeout Timeout duration
 * @return HAL status
 */
uint8_t lsm6dso_readStatus(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout)
{
	HAL_StatusTypeDef err = HAL_OK;
	uint8_t data = 0;
	err = HAL_I2C_Mem_Read(hi2c, DevAddress, LSM6DSO_STATUS_REG, 1, (uint8_t *)&data, 1, Timeout);
	if(err != HAL_OK) 	return 0;
	else				return data;
}

/**
 * Read WHO I AM register
 * @param hi2c Pointer to a I2C_HandleTypeDef structure
 * @param DevAddress Device address
 * @param Timeout Timeout duration
 * @return Device response
 */
uint8_t lsm6dso_whoIam(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout)
{
	uint8_t id=0;
	HAL_I2C_Mem_Read(&hi2c1, DevAddress, LSM6DSO_WHO_AM_I, 1, &id, 1, Timeout);
	return id;
}

/**
 * Software reset
 * @param hi2c Pointer to a I2C_HandleTypeDef structure
 * @param DevAddress Device address
 * @param Timeout Timeout duration
 * @return HAL status
 */
HAL_StatusTypeDef lsm6dso_resetSoftware(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout)
{
	uint8_t data = 0;
	HAL_StatusTypeDef err = HAL_I2C_Mem_Read(hi2c, DevAddress, LSM6DSO_CTRL3_C, 1, &data, 1, Timeout);
	if(err==HAL_OK)
	{
		data |= 0b00000001;
		err = HAL_I2C_Mem_Write(hi2c, DevAddress, LSM6DSO_CTRL3_C, 1, &data, 1, Timeout);
	}

	if(err != HAL_OK) 	return err;
	else 				return HAL_OK;
}

/**
 * Reset memory contents
 * @param hi2c Pointer to a I2C_HandleTypeDef structure
 * @param DevAddress Device address
 * @param Timeout Timeout duration
 * @return HAL status
 */
HAL_StatusTypeDef lsm6dso_resetMemory(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout)
{
	HAL_StatusTypeDef err = HAL_I2C_Mem_Write(hi2c, DevAddress, LSM6DSO_CTRL3_C, 1, (uint8_t *)0b10000000, 1, Timeout);
	if(err != HAL_OK) 	return err;
	else 				return HAL_OK;
}

#endif /* INC_LSM6DSO_C_ */
