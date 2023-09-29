/**
  ******************************************************************************
  * @file           : gnss_nmea.c
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

#include "gnss_nmea.h"


#define PARSE_FLOAT(x)	strtof(x+1, &x)
#define PARSE_INT(x)	strtoul(x+1, &x, 10)
#define PARSE_CHAR(x)	(*(x+1)); ((*(x+1)) !=',') ? x+=2 : x++


/**
 * Convert hexadecimal to integer
 * @param c Char to check
 * @return Integer number
 */
static inline int8_t hex2int(char c)
{
    if (c >= '0' && c <= '9')		return c - '0';
    else if (c >= 'A' && c <= 'F')	return c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')	return c - 'a' + 10;
    else							return HAL_ERROR;
}


//static float gnss_nmea_Coord2Deg(float deg_coord, char nsew) {
//    int degree = (int)(deg_coord/100);
//    float minutes = deg_coord - degree*100;
//    float dec_deg = minutes / 60;
//    float decimal = degree + dec_deg;
//    if (nsew == 'S' || nsew == 'W') { // return negative
//        decimal *= -1;
//    }
//    return decimal;
//}


/**
 * Convert time (hhmmss.ss) to bytes
 * @param time Time in format hhmmss.ss
 * @param hours Pointer to hours
 * @param min Pointer to minutes
 * @param sec Pointer to seconds
 */
void gnss_nmea_Time2bytes(float time, uint8_t *hours, uint8_t *min, uint8_t *sec)
{
	*hours = time*0.0001;
	*min = (time-*hours*10000)*0.01;
	*sec = time-((*hours)*10000)-((*min)*100);
}


/**
 * Find, check and extract a NMEA sentence from a ring buffer
 * @param ring Buffer ring where there is a NMEA message
 * @param ini_pos Initial position of NMEA sentence
 * @param end_pos End position of NMEA sentence
 * @param gnss Pointer to GNSS NMEA message structure for saving sentence
 * @return HAL status
 */
HAL_StatusTypeDef gnss_nmea_GetData(uint8_t *ring, uint32_t ini_pos, uint32_t end_pos, gnss_nmea_t *gnss)
{
	static char safe_ring[MAX_LENGTH_NMEA];	// para que no se pierda la info
	if(ring[ini_pos]!='$' || ring[end_pos]!='\n')
		return HAL_ERROR;


	// Guardado en safe ring
	uint8_t payload_end = 0;

	if(ini_pos>end_pos)
	{
		if((GNSS_RING_BUFFER_SIZE-ini_pos) + (end_pos+1) > MAX_LENGTH_NMEA)
			return HAL_ERROR;
		memcpy(safe_ring, &ring[ini_pos], GNSS_RING_BUFFER_SIZE-ini_pos);
		if(end_pos==0)	safe_ring[GNSS_RING_BUFFER_SIZE-ini_pos]=ring[end_pos];
		memcpy(&safe_ring[GNSS_RING_BUFFER_SIZE-ini_pos], &ring[0], end_pos+1);
		payload_end = (GNSS_RING_BUFFER_SIZE - ini_pos) + (end_pos+1) - 5;
	}
	else
	{
		if((end_pos - ini_pos) > MAX_LENGTH_NMEA)
			return HAL_ERROR;
		memcpy(safe_ring, &ring[ini_pos], 1+end_pos-ini_pos);
		payload_end = (1+end_pos-ini_pos) - 5;
	}



	// Obtener el checksum en hexadecimal
	int8_t upper = hex2int(safe_ring[payload_end+1]);
    if (upper == -1)
    	return HAL_ERROR;
    int8_t lower = hex2int(safe_ring[payload_end+2]);
    if (lower == -1)
    	return HAL_ERROR;
    uint8_t expected = upper << 4 | lower;

    // Calculate checksum
    uint8_t checksum = 0x00;
    for(uint8_t i=1;  i<payload_end; i++)	checksum ^= safe_ring[i];

    if(expected!=checksum)
    	return HAL_ERROR;


    /***   ALL SAFE   ***/


	// Interpretar sentencia

    static char *p;
    p = &safe_ring[3];
    if(!strncmp(p, "RMC", 3))
    {
    	p+=3;
		gnss->rmc->time = PARSE_FLOAT(p);
		gnss->rmc->status = PARSE_CHAR(p);
		if(gnss->rmc->status != 'A')	return HAL_ERROR;
		gnss->rmc->lat = PARSE_FLOAT(p);
		gnss->rmc->ns = PARSE_CHAR(p);
		gnss->rmc->lon = PARSE_FLOAT(p);
		gnss->rmc->ew = PARSE_CHAR(p);
		gnss->rmc->speed = PARSE_FLOAT(p);
		gnss->rmc->cog = PARSE_FLOAT(p);
		gnss->rmc->date = PARSE_INT(p);
		gnss->rmc->mv = PARSE_FLOAT(p);
		gnss->rmc->mvEW = PARSE_CHAR(p);
		gnss->rmc->posMode = PARSE_CHAR(p);
		gnss->rmc->navStatus = PARSE_CHAR(p);
    }

    else if(!strncmp(p, "GGA", 3))
    {
    	p+=3;
		gnss->gga->time = PARSE_FLOAT(p);
		gnss->gga->lat = PARSE_FLOAT(p);
		gnss->gga->ns = PARSE_CHAR(p);
		gnss->gga->lon = PARSE_FLOAT(p);
		gnss->gga->ew = PARSE_CHAR(p);
		gnss->gga->quality = PARSE_INT(p);
		gnss->gga->numSV = PARSE_INT(p);
		gnss->gga->hdop = PARSE_FLOAT(p);
		gnss->gga->alt = PARSE_FLOAT(p);
		gnss->gga->altUnit = PARSE_CHAR(p);
		gnss->gga->sep = PARSE_FLOAT(p);
		gnss->gga->sepUnit= PARSE_CHAR(p);
		gnss->gga->diffAge = PARSE_FLOAT(p);
		gnss->gga->diffStation = PARSE_FLOAT(p);
    }

    else if (!strncmp(p, "GSA", 3))
    {
    	return HAL_OK;
    	p+=3;
//    	PARSE_CHAR(p);
    	PARSE_INT(p);
    	PARSE_INT(p);

    	PARSE_INT(p);
    	PARSE_INT(p);
    	PARSE_INT(p);
    	PARSE_INT(p);
    	PARSE_INT(p);
    	PARSE_INT(p);
    	PARSE_INT(p);
    	PARSE_INT(p);
    	PARSE_INT(p);
    	PARSE_INT(p);
    	PARSE_INT(p);
    	PARSE_INT(p);

    	PARSE_FLOAT(p);
    	PARSE_FLOAT(p);
    	PARSE_FLOAT(p);
//    	PARSE_CHAR(p);
    }

    else if (!strncmp(p, "GST", 3))
    {
    	return HAL_OK;
    	PARSE_FLOAT(p);
    	PARSE_FLOAT(p);
    	PARSE_FLOAT(p);
    	PARSE_FLOAT(p);
    	PARSE_FLOAT(p);
    	PARSE_FLOAT(p);
    	PARSE_FLOAT(p);
    	PARSE_FLOAT(p);
    }


    return HAL_OK;

}
