/**
  ******************************************************************************
  * @file           : lis2mdltr.h
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

#ifndef INC_LIS2MDLTR_H_
#define INC_LIS2MDLTR_H_


#include "main.h"
#include <stdlib.h>


#define LIS2MDL_ADDRESS 					0b00111100 //!< Default address for write
#define LIS2MDL_CHIP_ID 					0x40       //!< Chip ID from WHO_AM_I register
#define LIS2MDL_LSB2MG 						1.5 	   //!< Sensitivity Megnetometer
#define LIS2MDL_LSB2TEMP 					0.125 	   //!< Sensitivity Temperature
#define LIS2MDL_MILLIGAUSS_TO_MICROTESLA  	0.1 //!< Conversion rate of Milligauss to Microtesla


/**
 * LIS2MDL I2C register address bits
 */
typedef enum {
  LIS2MDL_OFFSET_X_REG_L = 0x45,
  LIS2MDL_OFFSET_X_REG_H = 0x46,
  LIS2MDL_OFFSET_Y_REG_L = 0x47,
  LIS2MDL_OFFSET_Y_REG_H = 0x48,
  LIS2MDL_OFFSET_Z_REG_L = 0x49,
  LIS2MDL_OFFSET_Z_REG_H = 0x4A,
  LIS2MDL_WHO_AM_I = 0x4F,
  LIS2MDL_CFG_REG_A = 0x60,
  LIS2MDL_CFG_REG_B = 0x61,
  LIS2MDL_CFG_REG_C = 0x62,
  LIS2MDL_INT_CRTL_REG = 0x63,
  LIS2MDL_INT_SOURCE_REG = 0x64,
  LIS2MDL_INT_THS_L_REG = 0x65,
  LIS2MDL_STATUS_REG = 0x67,
  LIS2MDL_OUTX_L_REG = 0x68,
  LIS2MDL_OUTX_H_REG = 0x69,
  LIS2MDL_OUTY_L_REG = 0x6A,
  LIS2MDL_OUTY_H_REG = 0x6B,
  LIS2MDL_OUTZ_L_REG = 0x6C,
  LIS2MDL_OUTZ_H_REG = 0x6D,
  LIS2MDL_TEMP_OUT_L_REG = 0x6E,
  LIS2MDL_TEMP_OUT_H_REG = 0x6F,
} lis2mdl_register_t;


/**
 * Magnetometer update rate settings
 */
typedef enum {
  LIS2MDL_RATE_10_HZ,  //!< 10 Hz
  LIS2MDL_RATE_20_HZ,  //!< 20 Hz
  LIS2MDL_RATE_50_HZ,  //!< 50 Hz
  LIS2MDL_RATE_100_HZ, //!< 100 Hz
} lis2mdl_rate_t;



HAL_StatusTypeDef lis2mdl_config(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout);
HAL_StatusTypeDef lis2mdl_readMag(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout, int16_t *x, int16_t *y, int16_t *z);
HAL_StatusTypeDef lis2mdl_readTemp(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout, float *temp);
HAL_StatusTypeDef lis2mdl_single_conversion(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout);
uint8_t lis2mdl_whoIam(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout);
uint8_t lis2mdl_status(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout);



#endif /* INC_LIS2MDLTR_H_ */
