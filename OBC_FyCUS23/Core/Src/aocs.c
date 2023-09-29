/**
  ******************************************************************************
  * @file           : aocs.c
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


/* Private includes ----------------------------------------------------------*/
#include "aocs.h"


/* Private variables ---------------------------------------------------------*/
/* Variables de la IMU */
imu_t imu = {0};

/* Variables del magnetómetro */
magneto_t mag = {0};

/* Variables del altímetro */
altimeter_t altimeter = {0};

/* Variables de las sondads de temperatura */
temperatures_t temps = {0};

/* Variables del GNSS */
uint8_t gnss_ring[GNSS_RING_BUFFER_SIZE] = {0};
volatile uint32_t gnss_ring_pos = 0;
volatile uint32_t gnss_ring_last_end = 0;
volatile uint32_t gnss_ring_last_ini = 0;
uint8_t pubx_ring[200] = {0};
gnss_nmea_rmc_sentence_t rmc;
gnss_nmea_gga_sentence_t gga;
gnss_nmea_t gnss = {&rmc, &gga};

/* Private user code ---------------------------------------------------------*/

//uint16_t checksum_calc(uint8_t *data, uint8_t lenght)
//{
//	uint8_t ck_a=0, ck_b=0;
//	for(uint8_t i=0; i<lenght; i++)
//	{
//		ck_a += data[i];
//		ck_b += ck_a;
//	}
//	return ((ck_a<<8) | ck_b);
//}

/**
 * Configure NEO GNSS receiver
 * @return HAL status
 */
inline HAL_StatusTypeDef aocs_config_gnss()
{

	uint8_t dynModel[44] =
	{
			0xB5,0x62,0x06,0x24,0x24,0x00, // Header
			0x00,0x01,				// Mask
			0x06,					// dynModel
			0x03,					// fixMode
			0x00,0x00,0x00,0x00,	// fixedAlt
			0x00,0x00,0x00,0x01,	// fixedAltVar
			0x05,					// minElev
			0x00,					// drLimit
			0x00,0x19,				// pDop
			0x00,0x19,				// tDop
			0x00,0x64,				// pAcc
			0x01,0x5E,				// tAcc
			0x00,					// staticHoldThresh
			0x3C,					// dgnssTimeout
			0x00,					// cnoTreshNumSVs
			0x00,					// cnThresh
			0x00,0x00,				// reserved1
			0x00,0x00,				// staticHoldMaxDist
			0x00,					// utcStadart
			0x00,0x00,0x00,0x00,0x00,
			0x8F,0xD1				// Checksum
	};

//	uint8_t cno_data[11] = 		{0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0x01, 0x35, 0x01, 0x41, 0xAD};//0xAD};
	uint8_t no_cno_data[11] = 	{0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0x01, 0x35, 0x00, 0x40, 0xAC};

	HAL_StatusTypeDef err = HAL_UART_Transmit(GNSS_UART_HANDLE, dynModel, 44, 500);
	if(err != HAL_OK) 	return err;

//	err = HAL_UART_Transmit(GNSS_UART_HANDLE, cno_data, 11, 100);
//	if(err != HAL_OK) 	return err;

	HAL_UART_Transmit(GNSS_UART_HANDLE, no_cno_data, 11, 100);
	if(err != HAL_OK) 	return err;

	return HAL_OK;
}


/**
 * Configure LIS2MDL magnetometer
 * @return HAL status
 */
inline HAL_StatusTypeDef aocs_config_magnetometer()
{
	if(lis2mdl_whoIam(&hi2c1, LIS2MDL_ADDRESS, 10) == LIS2MDL_CHIP_ID)
	{
		HAL_StatusTypeDef err = lis2mdl_config(&hi2c1, LIS2MDL_ADDRESS, 10);
    	if(err != HAL_OK) 	return err;
    	else 				return HAL_OK;
	}
    else return HAL_ERROR;

}



/**
 * Configure LSM6DSO IMU
 * @return HAL status
 */
inline HAL_StatusTypeDef aocs_config_imu()
{
    if(lsm6dso_whoIam(&hi2c1, LSM6DSO_ADDRESS, 30) == LSM6DSO_CHIP_ID)
    {
    	HAL_StatusTypeDef err = lsm6dso_config(&hi2c1, LSM6DSO_ADDRESS, 30);
    	if(err != HAL_OK) 	return err;
    	else 				return HAL_OK;
    }
    else return HAL_ERROR;
}


/**
 * Configure MS5607 altimeter
 * @return HAL status
 */
inline HAL_StatusTypeDef aocs_config_altimeter()
{
	HAL_StatusTypeDef err = ms5607_reset();
	if (err != HAL_OK)	return err;
	HAL_Delay(10);

	err = ms5607_readCalibration(altimeter.calibration);
	if (err != HAL_OK)	return err;

	return HAL_OK;
}


/**
 * Configure DS18B20 temperature sensors
 * @return HAL status
 */
inline HAL_StatusTypeDef aocs_config_temp()
{
	HAL_StatusTypeDef err = HAL_OK;
	uint8_t dir1[8] = {40,255,100,30,35,183,146,218};	// Número 4
	uint8_t dir2[8] = {40,255,100,30,35,183,6,170};		// Número 3
	uint8_t dir3[8] = {40,255,100,30,35,163,223,182};	// Número 6
	memcpy(temps.dir1, dir1, 8);
	memcpy(temps.dir2, dir2, 8);
	memcpy(temps.dir3, dir3, 8);
	err |= DS18B20_Reset_CMD(ONEWIRE_GPIO_Port, ONEWIRE_Pin);
	err |= DS18B20_SetResolution(ONEWIRE_GPIO_Port, ONEWIRE_Pin, temps.dir1, DS18B20_RESOLUTION_12);
	err |= DS18B20_Reset_CMD(ONEWIRE_GPIO_Port, ONEWIRE_Pin);
	err |= DS18B20_SetResolution(ONEWIRE_GPIO_Port, ONEWIRE_Pin, temps.dir2, DS18B20_RESOLUTION_12);
	err |= DS18B20_Reset_CMD(ONEWIRE_GPIO_Port, ONEWIRE_Pin);
	err |= DS18B20_SetResolution(ONEWIRE_GPIO_Port, ONEWIRE_Pin, temps.dir3, DS18B20_RESOLUTION_12);

	if(err != HAL_OK)	return err;
	else				return HAL_OK;
}





//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////



/**
 * Decode NMEA message into from ring buffer data
 * @return HAL status
 */
inline HAL_StatusTypeDef aocs_get_gnss()
{
	return gnss_nmea_GetData(gnss_ring, gnss_ring_last_ini,gnss_ring_last_end, &gnss);
}


/**
 * Get megnetometer values from LIS2MDL
 * @return HAL status
 */
inline HAL_StatusTypeDef aocs_get_mag()
{
	HAL_StatusTypeDef err = HAL_OK;

	uint8_t status = lis2mdl_status(&hi2c1, LIS2MDL_ADDRESS, 10);
	if(status&0b00001000)
	{
		err |= lis2mdl_readMag(&hi2c1, LIS2MDL_ADDRESS, 20, &mag.x, &mag.y, &mag.z);
		err |= lis2mdl_readTemp(&hi2c1, LIS2MDL_ADDRESS, 10, &mag.temp);
	}
	if (err != HAL_OK)	return err;
	else				return HAL_OK;
}


/**
 * Get angular and linear acceleration from LSM6DSO
 * @return HAL status
 */
inline HAL_StatusTypeDef aocs_get_imu()
{
	HAL_StatusTypeDef err = HAL_OK;

	uint8_t status = lsm6dso_readStatus(&hi2c1, LSM6DSO_ADDRESS, 10);
	if(status & 0b001)
	{
		err |= lsm6dso_readAccel(&hi2c1, LSM6DSO_ADDRESS, 100, &imu.acell_x, &imu.acell_y, &imu.acell_z);
	}
	if(status & 0b010)
	{
		err |= lsm6dso_readGyro(&hi2c1, LIS2MDL_ADDRESS, 100, &imu.gyro_x, &imu.gyro_y, &imu.gyro_z);
	}
	if(status & 0b100)
	{
		err |= lsm6dso_readTemp(&hi2c1, LIS2MDL_ADDRESS, 100, &imu.temp);
	}
	if (err != HAL_OK)	return err;
	else 				return HAL_OK;
}


/**
 * Get linear acceleration from LSM6DSO
 * @return HAL status
 */
inline HAL_StatusTypeDef aocs_get_accel()
{
	HAL_StatusTypeDef err = HAL_OK;
	uint8_t status = lsm6dso_readStatus(&hi2c1, LSM6DSO_ADDRESS, 10);
	if(status & 0b001)
	{
		err = lsm6dso_readAccel(&hi2c1, LSM6DSO_ADDRESS, 100, &imu.acell_x, &imu.acell_y, &imu.acell_z);
		if (err != HAL_OK)	return err;
	}
	return HAL_OK;
}


/**
 * Get angular acceleration from LSM6DSO
 * @return HAL status
 */
inline HAL_StatusTypeDef aocs_get_gyro()
{
	HAL_StatusTypeDef err = HAL_OK;
	uint8_t status = lsm6dso_readStatus(&hi2c1, LSM6DSO_ADDRESS, 10);
	if(status & 0b010)
	{
		err = lsm6dso_readGyro(&hi2c1, LSM6DSO_ADDRESS, 100, &imu.gyro_x, &imu.gyro_y, &imu.gyro_z);
		if (err != HAL_OK)	return err;
	}
	return HAL_OK;
}

/**
 * Get altitude, pressure and temperature from MS5607
 * @return HAL status
 */
inline HAL_StatusTypeDef aocs_get_altimeter()
{
	if(ms5607_getTempPressure(MS5607_ADC_4096, altimeter.calibration, &altimeter.temp, &altimeter.pressure) != HAL_OK)	return HAL_ERROR;
	altimeter.altitude = ms5607_getAltitude(altimeter.pressure);
	return HAL_OK;
}


/**
 * Get temperature from DS18B20
 * @return HAL status
 */
inline HAL_StatusTypeDef aocs_get_temp()
{

	if(DS18B20_Reset_CMD(ONEWIRE_GPIO_Port, ONEWIRE_Pin) != HAL_OK)		return HAL_ERROR;
	if(DS18B20_All_Convert(ONEWIRE_GPIO_Port, ONEWIRE_Pin) != HAL_OK)	return HAL_ERROR;
	temps.temp_up = DS18B20_Read_Temp(ONEWIRE_GPIO_Port, ONEWIRE_Pin, temps.dir1);
	temps.temp_battery = DS18B20_Read_Temp(ONEWIRE_GPIO_Port, ONEWIRE_Pin, temps.dir2);
	temps.temp_down = DS18B20_Read_Temp(ONEWIRE_GPIO_Port, ONEWIRE_Pin, temps.dir3);

	if (temps.temp_up==DS18B20_CONVERSION_ERROR ||
			temps.temp_battery==DS18B20_CONVERSION_ERROR ||
			temps.temp_down==DS18B20_CONVERSION_ERROR )
	{
		return HAL_ERROR;
	}
	else return HAL_OK;
}


/**
 * Get PUBX message from NEO GNSS receiver
 * @return HAL status
 */
inline HAL_StatusTypeDef aocs_get_pubx()
{
	uint16_t gnss_flag_pubx_length = 6 + (pubx_ring[5] << 8 | pubx_ring[4]) + 2+1;
	uint8_t pubx_msg[300];
	memcpy(pubx_msg, pubx_ring, gnss_flag_pubx_length);

	if(pubx_msg[0]==0xB5 && pubx_msg[1]==0x62)
	{
		if(pubx_msg[2]==0x1 && pubx_msg[3]==0x35)
		{
			return HAL_OK;
		}
		else if(pubx_msg[2]==0x05 && pubx_msg[3]==0x01)
		{
			return HAL_OK;
		}
		else if(pubx_msg[2]==0x05 && pubx_msg[3]==0x00)
		{
			return HAL_ERROR;
		}
	}
	return HAL_ERROR;
}
