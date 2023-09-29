/**
  ******************************************************************************
  * @file           : aocs.h
  * @brief          : AOCS for FyCUS 2023.
  *
  * @author         Rubén Torres Bermúdez <rubentorresbermudez@gmail.com>
  ******************************************************************************
  * @attention
  *
  *  Created on:     05.09.2023
  *
  *  Description:
  *		This library is used to control the AOCS subsystem in FyCUS 2023 project
  *
  *  Copyright (C) 2023 Rubén Torres Bermúdez
  ******************************************************************************
  */

#ifndef INC_AOCS_H_
#define INC_AOCS_H_

/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "gnss_nmea.h"
#include "lsm6dso.h"
#include "lis2mdltr.h"
#include "ms5607.h"
#include "RT_ds18b20.h"

/* Private define ------------------------------------------------------------*/


/* Private typedef -----------------------------------------------------------*/
/**
 * IMU data structure
 */
typedef struct
{
	int16_t acell_x, acell_y, acell_z;
	int16_t gyro_x, gyro_y, gyro_z;
	int16_t temp;
} imu_t;

/**
 * Magnetometer data structure
 */
typedef struct
{
	int16_t x, y, z;
	float temp;
} magneto_t;

/**
 * Altimeter data structure
 */
typedef struct
{
	uint16_t calibration[5];
	float  temp;
	float pressure;
	float altitude;
} altimeter_t;

/**
 * Temperature sensors data structure
 */
typedef struct
{
	uint8_t dir1[8];
	uint8_t dir2[8];
	uint8_t dir3[8];
	float  temp_up, temp_battery, temp_down;
} temperatures_t;



/* Private variables ---------------------------------------------------------*/
/* Variables de la IMU */
extern imu_t imu;

/* Variables del magnetómetro */
extern magneto_t mag;

/* Variables del altímetro */
extern altimeter_t altimeter;

/* Variables de las sondads de temperatura */
extern temperatures_t temps;

/* Variables del GNSS */
extern uint8_t gnss_ring[GNSS_RING_BUFFER_SIZE];
extern volatile uint32_t gnss_ring_pos;
extern volatile uint32_t gnss_ring_last_end;
extern volatile uint32_t gnss_ring_last_ini;
extern uint8_t pubx_ring[200];
extern gnss_nmea_rmc_sentence_t rmc;
extern gnss_nmea_gga_sentence_t gga;
extern gnss_nmea_t gnss;


/* Private function prototypes -----------------------------------------------*/
HAL_StatusTypeDef aocs_config_gnss();
HAL_StatusTypeDef aocs_config_imu();
HAL_StatusTypeDef aocs_config_magnetometer();
HAL_StatusTypeDef aocs_config_altimeter();
HAL_StatusTypeDef aocs_config_temp();

HAL_StatusTypeDef aocs_get_gnss();
HAL_StatusTypeDef aocs_get_imu();
HAL_StatusTypeDef aocs_get_accel();
HAL_StatusTypeDef aocs_get_gyro();
HAL_StatusTypeDef aocs_get_mag();
HAL_StatusTypeDef aocs_get_altimeter();
HAL_StatusTypeDef aocs_get_temp();
HAL_StatusTypeDef aocs_get_pubx();


#endif /* INC_AOCS_H_ */
