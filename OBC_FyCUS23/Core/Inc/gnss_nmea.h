/**
  ******************************************************************************
  * @file           : gnss_nmea.h
  * @brief          : GNSS NMEA message decoder.
  *
  * @author         Rubén Torres Bermúdez <rubentorresbermudez@gmail.com>
  ******************************************************************************
  * @attention
  *
  *  Created on:     11.08.2023
  *
  *  Description:
  *		This library is used to read a NMEA message from a ring buffer.
  *
  *  Copyright (C) 2023 Rubén Torres Bermúdez
  ******************************************************************************
  */

#ifndef INC_GNSS_NMEA_H_
#define INC_GNSS_NMEA_H_

/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Private define ------------------------------------------------------------*/
#define GNSS_RING_BUFFER_SIZE	1024
#define MAX_LENGTH_NMEA			80

/* Private typedef -----------------------------------------------------------*/
/**
 * RMC sentence structure
 */
typedef struct  {
	float time;
	char status;
	float lat;
	char ns;
	float lon;
	char ew;
	float speed;
	float cog;
	int date;
	float mv;
	char mvEW;
	char posMode;
	char navStatus;
}gnss_nmea_rmc_sentence_t;

/**
 * GGA sentence structure
 */
typedef struct  {
	float time;
	float lat;
	char ns;
	float lon;
	char ew;
	int8_t quality;
	uint8_t numSV;
	float hdop;
	float alt;
	char altUnit;
	float sep;
	float sepUnit;
	float diffAge;
	float diffStation;
}gnss_nmea_gga_sentence_t;

/**
 * NMEA message structure
 */
typedef struct
{
	gnss_nmea_rmc_sentence_t *rmc;
	gnss_nmea_gga_sentence_t * gga;
}gnss_nmea_t;



void gnss_nmea_Time2bytes(float time, uint8_t *hours, uint8_t *min, uint8_t *sec);
void gnss_data2buffer(gnss_nmea_t *gnss, uint8_t *buffer);
HAL_StatusTypeDef gnss_nmea_GetData(uint8_t *ring, uint32_t ini_pos, uint32_t end_pos, gnss_nmea_t *gnss);

#endif /* INC_GNSS_NMEA_H_ */
