/**
  ******************************************************************************
  * @file           : lsm6dso.h
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

#ifndef INC_LSM6DSO_H_
#define INC_LSM6DSO_H_

#include "main.h"
//#include "lsm6dso_reg.h"


#define LSM6DSO_ADDRESS 0b11010100   //!< Default address for write mem
#define LSM6DSO_CHIP_ID 0x6C   //!< Default address for write mem

#define LSM6DSO_ACC_SENSITIVITY_FS_4G   0.122f
#define LSM6DSO_ACC_SENSITIVITY_FS_8G   0.244f
#define LSM6DSO_ACC_SENSITIVITY_FS_16G  0.488f

#define LSM6DSO_GYRO_SENSITIVITY_FS_125DPS    4.375f
#define LSM6DSO_GYRO_SENSITIVITY_FS_250DPS    8.750f
#define LSM6DSO_GYRO_SENSITIVITY_FS_500DPS   17.500f
#define LSM6DSO_GYRO_SENSITIVITY_FS_1000DPS  35.000f
#define LSM6DSO_GYRO_SENSITIVITY_FS_2000DPS  70.000f



typedef enum {
  LSM6DSO_2g   = 0,                                        /**< LSM6DSO_2g */
  LSM6DSO_16g  = 1, /* if XL_FS_MODE = ‘1’ -> LSM6DSO_2g *//**< LSM6DSO_16g */
  LSM6DSO_4g   = 2,                                        /**< LSM6DSO_4g */
  LSM6DSO_8g   = 3,                                        /**< LSM6DSO_8g */
} lsm6dso_fs_xl_t;


typedef enum {
  LSM6DSO_XL_ODR_OFF    = 0,
  LSM6DSO_XL_ODR_12Hz5  = 1,
  LSM6DSO_XL_ODR_26Hz   = 2,
  LSM6DSO_XL_ODR_52Hz   = 3,
  LSM6DSO_XL_ODR_104Hz  = 4,
  LSM6DSO_XL_ODR_208Hz  = 5,
  LSM6DSO_XL_ODR_417Hz  = 6,
  LSM6DSO_XL_ODR_833Hz  = 7,
  LSM6DSO_XL_ODR_1667Hz = 8,
  LSM6DSO_XL_ODR_3333Hz = 9,
  LSM6DSO_XL_ODR_6667Hz = 10,
  LSM6DSO_XL_ODR_1Hz6   = 11, /* (low power only) */
} lsm6dso_odr_xl_t;


typedef enum {
  LSM6DSO_250dps   = 0,
  LSM6DSO_125dps   = 1,
  LSM6DSO_500dps   = 2,
  LSM6DSO_1000dps  = 4,
  LSM6DSO_2000dps  = 6,
} lsm6dso_fs_g_t;


typedef enum {
  LSM6DSO_GY_ODR_OFF    = 0,
  LSM6DSO_GY_ODR_12Hz5  = 1,
  LSM6DSO_GY_ODR_26Hz   = 2,
  LSM6DSO_GY_ODR_52Hz   = 3,
  LSM6DSO_GY_ODR_104Hz  = 4,
  LSM6DSO_GY_ODR_208Hz  = 5,
  LSM6DSO_GY_ODR_417Hz  = 6,
  LSM6DSO_GY_ODR_833Hz  = 7,
  LSM6DSO_GY_ODR_1667Hz = 8,
  LSM6DSO_GY_ODR_3333Hz = 9,
  LSM6DSO_GY_ODR_6667Hz = 10,
} lsm6dso_odr_g_t;


typedef enum {
	LPF2_XL_DIS = 0,
	LPF2_XL_EN = 1,
} lsm6dso_lpf2_xl_state_t;



typedef enum {
	LSM6DSO_FUNC_CFG_ACCESS = 0x01U,
	LSM6DSO_PIN_CTRL = 0x02U,
	LSM6DSO_FIFO_CTRL4 = 0x0AU,
	LSM6DSO_COUNTER_BDR_REG1 = 0x0BU,
	LSM6DSO_INT1_CTRL = 0x0DU,
	LSM6DSO_INT2_CTRL = 0x0EU,
	LSM6DSO_WHO_AM_I = 0x0FU,
	LSM6DSO_CTRL1_XL = 0x10U,
	LSM6DSO_CTRL2_G = 0x11U,
	LSM6DSO_CTRL3_C = 0x12U,
	LSM6DSO_CTRL8_XL = 0x17U,
	LSM6DSO_CTRL9_XL = 0x18U,
	LSM6DSO_STATUS_REG = 0x1EU,
	LSM6DSO_OUT_TEMP_L = 0x20U,
	LSM6DSO_OUT_TEMP_H = 0x21U,
	LSM6DSO_OUTX_L_G = 0x22U,
	LSM6DSO_OUTX_H_G = 0x23U,
	LSM6DSO_OUTY_L_G = 0x24U,
	LSM6DSO_OUTY_H_G = 0x25U,
	LSM6DSO_OUTZ_L_G = 0x26U,
	LSM6DSO_OUTZ_H_G = 0x27U,
	LSM6DSO_OUTX_L_A = 0x28U,
	LSM6DSO_OUTX_H_A = 0x29U,
	LSM6DSO_OUTY_L_A = 0x2AU,
	LSM6DSO_OUTY_H_A = 0x2BU,
	LSM6DSO_OUTZ_L_A = 0x2CU,
	LSM6DSO_OUTZ_H_A = 0x2DU,
	LSM6DSO_I3C_BUS_AVB = 0x62U,
} lsm6dso_register_t;




HAL_StatusTypeDef lsm6dso_config(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout);
HAL_StatusTypeDef lsm6dso_readAccel(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout, int16_t *x, int16_t *y, int16_t *z);
HAL_StatusTypeDef lsm6dso_readGyro(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout, int16_t *x, int16_t *y, int16_t *z);
HAL_StatusTypeDef lsm6dso_readTemp(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout, int16_t *temp);
uint8_t lsm6dso_readStatus(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout);
uint8_t lsm6dso_whoIam(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout);
HAL_StatusTypeDef lsm6dso_resetSoftware(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout);
HAL_StatusTypeDef lsm6dso_resetMemory(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Timeout);



#endif /* INC_LSM6DSO_H_ */
