/**
  ******************************************************************************
  * @file           : intelli.c
  * @brief          : OBC Intelligence for FyCUS 2023.
  *
  * @author         Rubén Torres Bermúdez <rubentorresbermudez@gmail.com>
  ******************************************************************************
  * @attention
  *
  *  Created on:     10.09.2023
  *
  *  Description:
  *		This library is used to control the OBC intelligence in FyCUS 2023 project
  *
  *  Copyright (C) 2023 Rubén Torres Bermúdez
  ******************************************************************************
  */


/* Private includes ----------------------------------------------------------*/
#include "intelli.h"

/* Private variables ---------------------------------------------------------*/
uint8_t modem_buffer_tx[BUS_PACKET_BUS_SIZE+BUS_PACKET_FRAME_SYNC_SIZE] = {0};
uint8_t *const modem_data_tx = &modem_buffer_tx[BUS_PACKET_FRAME_SYNC_SIZE];

uint8_t eps_buffer_tx[BUS_PACKET_BUS_SIZE+BUS_PACKET_FRAME_SYNC_SIZE] = {0};
uint8_t *const eps_data_tx = &eps_buffer_tx[BUS_PACKET_FRAME_SYNC_SIZE];

uint8_t buffer_data [BUS_PACKET_DATA_SIZE];


/* Control de telecomandos y telemetrías */
uint32_t last_tc_apid = 0;
uint32_t last_tc_number = 0;
uint32_t last_tm_apid = 0;
uint32_t last_tm_number = 0;
tc_required_data_t programmed_tm =
		REQUIRED_DATA_GNSS | REQUIRED_DATA_ACELL | REQUIRED_DATA_GYRO | REQUIRED_DATA_TEMPERATURE_OUTDOOR | REQUIRED_DATA_BATTERY_VOLTAGE;


sc_status_t sc_status;


eps_data_t eps;


RTC_TimeTypeDef rtc;

/* Private function prototypes -----------------------------------------------*/
static inline void float2bytes(float value, uint8_t *buffer);
static inline float bytes2float(const uint8_t *buffer);
static inline void u16t2bytes(uint16_t value, uint8_t *buffer);


/* Private user code ---------------------------------------------------------*/
/**
 * Float to byte array conversion
 * @param value Float value
 * @param buffer Pointer to byte array
 */
static inline void float2bytes(float value, uint8_t *buffer)
{
    uint8_t *fpointer = (uint8_t *)&value;
    buffer[0] = fpointer[0];
    buffer[1] = fpointer[1];
    buffer[2] = fpointer[2];
    buffer[3] = fpointer[3];
}

/**
 * Byte array to float conversion
 * @param buffer Pointer to byte array
 * @return Float value
 */
static inline float bytes2float(const uint8_t *buffer)
{
    float value;
    uint8_t *fpointer = (uint8_t *)&value;
    fpointer[0] = buffer[0];
    fpointer[1] = buffer[1];
    fpointer[2] = buffer[2];
    fpointer[3] = buffer[3];
    return value;
}

/**
 * 16-bit unsigned integer to byte array conversion
 * @param value 16-bit unsigned integer
 * @param buffer Pointer to byte array
 */
static inline void u16t2bytes(uint16_t value, uint8_t *buffer)
{
    buffer[1] = (value>>8) & 0xFF;
    buffer[0] = value & 0xFF;
}

/**
 *
 * @param hours
 * @param min
 * @param sec
 * @return
 */
HAL_StatusTypeDef set_time(uint8_t hours,uint8_t min, uint8_t sec)
{
	rtc.Hours = hours;
	rtc.Minutes = min;
	rtc.Seconds = sec;
	return HAL_RTC_SetTime(RTC_HANDLE, &rtc, RTC_FORMAT_BIN);
}

/**
 *
 * @param hours
 * @param min
 * @param sec
 * @return
 */
HAL_StatusTypeDef get_time(uint8_t *hours, uint8_t *min, uint8_t *sec)
{
	if(HAL_RTC_GetTime(RTC_HANDLE, &rtc, RTC_FORMAT_BIN) != HAL_OK)	return HAL_ERROR;
	*hours = rtc.Hours;
	*min = rtc.Minutes;
	*sec = rtc.Seconds;
	return HAL_OK;
}

/**
 * Set a byte array with data required for transmission
 * @param requirements Data required. See tc_required_data_t
 * @param pdata Pointer to byte array
 * @return Length of byte array
 */
uint8_t dataRequired2bytes(tc_required_data_t requirements, uint8_t *pdata)
{
	uint8_t *ini = pdata;
	u16t2bytes(requirements, pdata); pdata+=2; // 2 Bytes
	if(requirements&REQUIRED_DATA_GNSS)	// 18 Bytes
	{
		float2bytes(gnss.rmc->lat, pdata); pdata+=4;
		*pdata = gnss.rmc->ns; pdata++;
		float2bytes(gnss.rmc->lon, pdata); pdata+=4;
		*pdata = gnss.rmc->ew; pdata++;
		float2bytes(gnss.gga->alt, pdata); pdata+=4;
		float2bytes(gnss.gga->sep, pdata); pdata+=4;
	}
	if(requirements&REQUIRED_DATA_ACELL) // 6 Bytes
	{
		u16t2bytes(imu.acell_x, pdata); pdata+=2;
		u16t2bytes(imu.acell_y, pdata); pdata+=2;
		u16t2bytes(imu.acell_z, pdata); pdata+=2;
	}
	if(requirements&REQUIRED_DATA_GYRO) // 6 Bytes
	{
		u16t2bytes(imu.gyro_x, pdata); pdata+=2;
		u16t2bytes(imu.gyro_y, pdata); pdata+=2;
		u16t2bytes(imu.gyro_z, pdata); pdata+=2;
	}
	if(requirements&REQUIRED_DATA_MAGNETOMETER) // 6 Bytes
	{
		u16t2bytes(mag.x, pdata); pdata+=2;
		u16t2bytes(mag.y, pdata); pdata+=2;
		u16t2bytes(mag.z, pdata); pdata+=2;
	}
	if(requirements&REQUIRED_DATA_PRESSURE) // 4 Bytes
	{
		float2bytes(altimeter.pressure, pdata); pdata+=4;
	}
	if(requirements&REQUIRED_DATA_TEMPERATURE_OUTDOOR) // 4 Bytes
	{
		float2bytes(altimeter.temp, pdata); pdata+=4;
	}
	if(requirements&REQUIRED_DATA_TEMPERATURE_UP) // 4 Bytes 1
	{
		float2bytes(temps.temp_up, pdata); pdata+=4;
	}
	if(requirements&REQUIRED_DATA_TEMPERATURE_BATTERY) // 4 Bytes 2
	{
		float2bytes(temps.temp_battery, pdata); pdata+=4;
	}
	if(requirements&REQUIRED_DATA_TEMPERATURE_DOWN) // 4 Bytes 3
	{
		float2bytes(temps.temp_down, pdata); pdata+=4;
	}
	// 58 bytes hasta aquí

	if(requirements&REQUIRED_DATA_PV_VOLTAGE) // 2 Bytes
	{
		u16t2bytes(eps.v_in, pdata); pdata+=2;
	}
	if(requirements&REQUIRED_DATA_PV_CURRENT) // 2 Bytes
	{
		u16t2bytes(eps.i_in, pdata); pdata+=2;
	}
	if(requirements&REQUIRED_DATA_BATTERY_VOLTAGE) // 2 Bytes
	{
		u16t2bytes(eps.v_bat, pdata); pdata+=2;
	}
	if(requirements&REQUIRED_DATA_BATTERY_CURRENT) // 2 Bytes
	{
		u16t2bytes(eps.i_bat, pdata); pdata+=2;
	}
	if(requirements&REQUIRED_DATA_BATTERY_CHARGING) // 2 Bytes
	{
		u16t2bytes(eps.charge_state, pdata); pdata+=2;
	}
	if(requirements&REQUIRED_DATA_BATTERY_DIETEMP) // 2 Bytes
	{
		u16t2bytes(eps.die_temp, pdata); pdata+=2;
	}
	// 58+12 bytes hasta aquí

	if(requirements&REQUIRED_DATA_CURRENT_TIME) // 4 Bytes
	{
		float2bytes(gnss.rmc->time, pdata); pdata+=4;
	}
	// 58+12+4 bytes hasta aquí

	return (pdata-ini);
}


/**
 * Check status of different subsystems and components
 */
void check_status()
{
	if(sc_status.obc&OBC_ERROR_OTHER);
	if(sc_status.obc&OBC_ERROR_GNSS);
	if(sc_status.obc&OBC_ERROR_IMU)			aocs_config_imu();
	if(sc_status.obc&OBC_ERROR_MAG)			aocs_config_magnetometer();
	if(sc_status.obc&OBC_ERROR_ALTIM)		aocs_config_altimeter();
	if(sc_status.obc&OBC_ERROR_TEMPS)		aocs_config_temp();
	if(sc_status.obc&OBC_ERROR_RX_EPS);
	if(sc_status.obc&OBC_ERROR_TX_EPS);
	if(sc_status.obc&OBC_ERROR_RX_MODEM);
	if(sc_status.obc&OBC_ERROR_TX_MODEM);
	if(sc_status.obc&OBC_ERROR_PUBX);
}


/**
 * Send telemetry data to modem for transmission to ground
 * @return HAL status
 */
HAL_StatusTypeDef send_TM()
{
	memcpy(modem_buffer_tx, BUS_PACKET_FRAME_SYNC, 4);
	return HAL_UART_Transmit_DMA(MODEM_UART_HANDLE, modem_buffer_tx, (bus_packet_GetLength(modem_data_tx) + BUS_PACKET_FRAME_SYNC_SIZE));
}

/**
 * Send programmed telemetry data to modem for transmission to ground
 * @return HAL status
 */
HAL_StatusTypeDef send_programmed_TM()
{
	static uint8_t data_length;
	HAL_StatusTypeDef err;
	data_length = dataRequired2bytes(programmed_tm, buffer_data);

	err = bus_packet_EncodePacketize(BUS_PACKET_TYPE_TM, APID_TM_PROGRAMMED_TELEMETRY,
			BUS_PACKET_ECF_EXIST, buffer_data, data_length,
			modem_data_tx);
	if(err != HAL_OK)	goto ERROR_PROCCESING_TC;

	err = send_TM();
	if(err != HAL_OK)	goto ERROR_PROCCESING_TC;

	return HAL_OK;

	ERROR_PROCCESING_TC:
	bus_packet_EncodePacketize(BUS_PACKET_TYPE_TM, APID_TM_REPORT_FAULT_TC,
			BUS_PACKET_ECF_EXIST, buffer_data, 0, modem_data_tx);
	send_TM();
	return HAL_ERROR;
}


/**
 * Send telemetry data to modem for saving data
 * @return HAL status
 */
HAL_StatusTypeDef send_data_saving()
{
	static uint8_t data_length;
	HAL_StatusTypeDef err;
	data_length = dataRequired2bytes(REQUIRED_DATA_ALL, buffer_data);

	err = bus_packet_EncodePacketize(BUS_PACKET_TYPE_TC, APID_BUS_DATA_SAVING,
			BUS_PACKET_ECF_EXIST, buffer_data, data_length,
			modem_data_tx);
	if(err != HAL_OK)	goto ERROR_PROCCESING_TC;

	err = send_TM();
	if(err != HAL_OK)	goto ERROR_PROCCESING_TC;

	return HAL_OK;

	ERROR_PROCCESING_TC:
	bus_packet_EncodePacketize(BUS_PACKET_TYPE_TM, APID_TM_REPORT_FAULT_TC,
			BUS_PACKET_ECF_EXIST, buffer_data, 0, modem_data_tx);
	send_TM();
	return HAL_ERROR;
}

/**
 * Send ACK
 * @param huart UART handle to send packet.
 * @return HAL status
 */
HAL_StatusTypeDef send_ack(UART_HandleTypeDef *huart)
{
	static uint8_t data[BUS_PACKET_FRAME_SYNC_SIZE+BUS_PACKET_HEADER_SIZE+BUS_PACKET_ECF_SIZE];

	HAL_StatusTypeDef err = bus_packet_EncodePacketize(BUS_PACKET_TYPE_TM, APID_BUS_ACK,
			BUS_PACKET_ECF_EXIST, (uint8_t *)NULL, 0,
			data+BUS_PACKET_DATA_SIZE);
	if(err != HAL_OK) return err;
	memcpy(data, BUS_PACKET_FRAME_SYNC, 4);
	return HAL_UART_Transmit(huart, data, (bus_packet_GetLength(data+BUS_PACKET_FRAME_SYNC_SIZE) + BUS_PACKET_FRAME_SYNC_SIZE), 2);
}

/**
 * Send NACK
 * @param huart UART handle to send packet.
 * @return HAL status
 */
HAL_StatusTypeDef send_nack(UART_HandleTypeDef *huart)
{
	static uint8_t data[BUS_PACKET_FRAME_SYNC_SIZE+BUS_PACKET_HEADER_SIZE+BUS_PACKET_ECF_SIZE];

	 HAL_StatusTypeDef err = bus_packet_EncodePacketize(BUS_PACKET_TYPE_TM, APID_BUS_NACK,
			BUS_PACKET_ECF_EXIST, (uint8_t *)NULL, 0,
			data+BUS_PACKET_DATA_SIZE);
	if(err != HAL_OK) return err;
	memcpy(data, BUS_PACKET_FRAME_SYNC, 4);
	return HAL_UART_Transmit(huart, data, (bus_packet_GetLength(data+BUS_PACKET_FRAME_SYNC_SIZE) + BUS_PACKET_FRAME_SYNC_SIZE), 2);
}





/* ************** INTELLI OBC ************** */
/**
 * Intelligence for a received telecommand
 * @param apid Received APID
 * @param data Received data
 */
void intelliOBC_TC(apid_t apid, uint8_t *data)
{
	HAL_StatusTypeDef err;
	switch (apid)
	{

	case APID_TC_PROGRAMMED_TELECOMMAND:
		asm("NOP");
		break;

	case APID_TC_REQUIRED_REPORT_OBC:
		err = bus_packet_EncodePacketize(BUS_PACKET_TYPE_TM, APID_TM_REPORT_OBC, BUS_PACKET_ECF_EXIST,
				(uint8_t *)&sc_status.obc, 2, modem_data_tx);
		if(err != HAL_OK)	goto ERROR_PROCCESING_TC;

		err = send_TM();
		if(err != HAL_OK)	goto ERROR_PROCCESING_TC;

		break;

	case APID_TC_REQUIRED_REPORT_EPS:
		err = bus_packet_EncodePacketize(BUS_PACKET_TYPE_TM, APID_TM_REPORT_EPS, BUS_PACKET_ECF_EXIST,
				(uint8_t *)&sc_status.eps, 2, modem_data_tx);
		if(err != HAL_OK)	goto ERROR_PROCCESING_TC;

		err = send_TM();
		if(err != HAL_OK)	goto ERROR_PROCCESING_TC;

		break;

	case APID_TC_REQUIRED_REPORT_MODEM:
		err = bus_packet_EncodePacketize(BUS_PACKET_TYPE_TM, APID_TM_REPORT_MODEM, BUS_PACKET_ECF_EXIST,
				(uint8_t *)&sc_status.modem, 2, modem_data_tx);
		if(err != HAL_OK)	goto ERROR_PROCCESING_TC;

		err = send_TM();
		if(err != HAL_OK)	goto ERROR_PROCCESING_TC;

		break;

	case APID_TC_REQUIRED_DATA:
		asm("NOP");
		uint16_t required = data[0]<<8 | data[1];
		uint8_t data_length = dataRequired2bytes(required, buffer_data);
		err = bus_packet_EncodePacketize(BUS_PACKET_TYPE_TM, APID_TM_LAST_DATA_REQUIRED,
				BUS_PACKET_ECF_EXIST, buffer_data, data_length, modem_data_tx);
		if(err != HAL_OK)	goto ERROR_PROCCESING_TC;

		err = send_TM();
		if(err != HAL_OK)	goto ERROR_PROCCESING_TC;

		break;

	case APID_TC_ARE_YOU_ALIVE:
		memcpy(buffer_data, (uint8_t *)"FyCUS 2023", 10);
		err = bus_packet_EncodePacketize(BUS_PACKET_TYPE_TM, APID_TM_LAST_DATA_REQUIRED,
				BUS_PACKET_ECF_EXIST, buffer_data, 10, modem_data_tx);
		if(err != HAL_OK)	goto ERROR_PROCCESING_TC;

		err = send_TM();
		if(err != HAL_OK)	goto ERROR_PROCCESING_TC;

		break;


	case APID_TC_SET_PROGRMMED_TELEMETRY:
		if((data[2]>MAX_TM_TIME) || (data[2]<MIN_TM_TIME))		goto ERROR_PROCCESING_TC;
		programmed_tm = data[0]<<8 | data[1] | REQUIRED_DATA_GNSS;
		  __HAL_TIM_SET_COUNTER(PRIMARY_TIM_HANDLE, 0);
		__HAL_TIM_SET_AUTORELOAD(PRIMARY_TIM_HANDLE, (uint16_t)((data[2]*10000/5)-1));
		break;

	case APID_BUS_GET_CURRENT_TIME:
		float2bytes(gnss.rmc->time, buffer_data);
		err = bus_packet_EncodePacketize(BUS_PACKET_TYPE_TC, APID_BUS_SET_CURRENT_TIME,
				BUS_PACKET_ECF_EXIST, buffer_data, 4, modem_data_tx);
		if(err != HAL_OK)	goto ERROR_PROCCESING_TC;

		err = send_TM();
		if(err != HAL_OK)	goto ERROR_PROCCESING_TC;


//	case APID_TM_PROGRAMMED_TELEMETRY:
//	case APID_TM_REPORT_OBC:
//	case APID_TM_REPORT_EPS:
//	case APID_TM_REPORT_MODEM:
//	case APID_BUS_OBC_REPORT:
//	case APID_BUS_EPS_REPORT:
//	case APID_BUS_MODEM_REPORT:
//	case APID_BUS_SET_CURRENT_TIME:



	default:	// Send Fault TC
		ERROR_PROCCESING_TC:
		bus_packet_EncodePacketize(BUS_PACKET_TYPE_TM, APID_TM_REPORT_FAULT_TC,
				BUS_PACKET_ECF_EXIST, buffer_data, 0, modem_data_tx);
		send_TM();
		break;
	}
}






/**
 * Intelligence for a received telemetry
 * @param apid Received APID
 * @param data Received data
 */
void intelliOBC_TM(uint8_t apid, uint8_t *data)
{
	switch (apid)
	{

	case APID_BUS_REPORT_EPS:
		sc_status.eps = data[0]<<8 | data[1];
		break;

	case APID_BUS_REPORT_MODEM:
		sc_status.modem = data[0]<<8 | data[1];
		break;

	case APID_BUS_PROGRAMMED_TELEMETRY:
		memcpy(&eps, data, EPS_DATA_LENGTH);
		break;

	case APID_BUS_REPORT_OBC:
	case APID_TM_PROGRAMMED_TELEMETRY:
	default:
		break;
	}
}

