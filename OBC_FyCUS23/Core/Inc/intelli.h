/**
  ******************************************************************************
  * @file           : intelli.h
  * @brief          : OBC Intelligence for FyCUS 2023.
  *
  * @author         Rubén Torres Bermúdez <rubentorresbermudez@gmail.com>
  ******************************************************************************
  * @attention
  *
  *  Created on:     10.09.2023
  *
  *  Description:
  *		This library is used to control the OBC intelligence in FyCUS 2023 project.
  *
  *  Copyright (C) 2023 Rubén Torres Bermúdez
  ******************************************************************************
  */

#ifndef INC_INTELLI_H_
#define INC_INTELLI_H_

/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "bus_packet.h"
#include "aocs.h"

/* Private define ------------------------------------------------------------*/
#define MAX_TM_TIME	60
#define MIN_TM_TIME	3
#define EPS_DATA_LENGTH		20


/* Private typedef -----------------------------------------------------------*/
/**
 * Spacecraft subsystem ID
 */
typedef enum
{
	SC_SYSTEM_OBC,  /**< SC_SYSTEM_OBC */
	SC_SYSTEM_EPS,  /**< SC_SYSTEM_EPS */
	SC_SYSTEM_MODEM,/**< SC_SYSTEM_MODEM */
} sc_system_t;


/* Error types OBC */
/**
 * Required data to OBC to transmit
 */
typedef enum
{
	OBC_ERROR_OTHER 		= 0b1,              	/**< OBC_ERROR_OTHER */
	OBC_ERROR_GNSS			= 0b10,              	/**< OBC_ERROR_GNSS */
	OBC_ERROR_IMU			= 0b100,              	/**< OBC_ERROR_IMU */
	OBC_ERROR_MAG			= 0b1000,             	/**< OBC_ERROR_MAG */
	OBC_ERROR_ALTIM			= 0b10000,          	/**< OBC_ERROR_ALTIM */
	OBC_ERROR_TEMPS			= 0b100000,         	/**< OBC_ERROR_TEMPS */
	OBC_ERROR_RX_EPS		= 0b1000000,        	/**< OBC_ERROR_RX_EPS */
	OBC_ERROR_TX_EPS		= 0b10000000,       	/**< OBC_ERROR_TX_EPS */
	OBC_ERROR_RX_MODEM		= 0b100000000,    		/**< OBC_ERROR_RX_MODEM */
	OBC_ERROR_TX_MODEM		= 0b1000000000,   		/**< OBC_ERROR_TX_MODEM */
	OBC_ERROR_RTC			= 0b10000000000,      	/**< OBC_ERROR_RTC */
	OBC_ERROR_PUBX			= 0b1000000000000000,	/**< OBC_ERROR_PUBX */
}obc_error_t;


/* Spacecraft Status Systems */
/**
 * Spacecraft subsystem status
 */
typedef struct
{
	uint16_t obc;
	uint16_t eps;
	uint16_t modem;
} sc_status_t;


/* APIDs */
/**
 * ID
 */
typedef enum
{
	APID_TM_PROGRAMMED_TELEMETRY	= 0x00,   		/**< APID_TM_PROGRAMMED_TELEMETRY */
	APID_TM_REPORT_OBC				= 0x01,         /**< APID_TM_REPORT_OBC */
	APID_TM_REPORT_EPS				= 0x02,         /**< APID_TM_REPORT_EPS */
	APID_TM_REPORT_MODEM			= 0x03,         /**< APID_TM_REPORT_MODEM */
	APID_TM_LAST_DATA_REQUIRED		= 0x04,    		/**< APID_TM_LAST_DATA_REQUIRED */
	APID_TM_REPORT_FAULT_TC			= 0x10,      	/**< APID_TM_REPORT_FAULT_TC */

	APID_TC_PROGRAMMED_TELECOMMAND	= 0x20, 		/**< APID_TC_PROGRAMMED_TELECOMMAND */
	APID_TC_REQUIRED_REPORT_OBC		= 0x21,   		/**< APID_TC_REQUIRED_REPORT_OBC */
	APID_TC_REQUIRED_REPORT_EPS		= 0x22,   		/**< APID_TC_REQUIRED_REPORT_EPS */
	APID_TC_REQUIRED_REPORT_MODEM	= 0x23,  		/**< APID_TC_REQUIRED_REPORT_MODEM */
	APID_TC_REQUIRED_DATA			= 0x24,        	/**< APID_TC_REQUIRED_DATA */
	APID_TC_SET_PROGRMMED_TELEMETRY	= 0x30,			/**< APID_TC_SET_PROGRMMED_TELEMETRY */
	APID_TC_ARE_YOU_ALIVE			= 0x31,        	/**< APID_TC_ARE_YOU_ALIVE */

	APID_BUS_PROGRAMMED_TELEMETRY	= 0x40,			/**< APID_BUS_PROGRAMMED_TELEMETRY */
	APID_BUS_REPORT_OBC				= 0x41,         /**< APID_BUS_REPORT_OBC */
	APID_BUS_REPORT_EPS				= 0x42,         /**< APID_BUS_REPORT_EPS */
	APID_BUS_REPORT_MODEM			= 0x43,        	/**< APID_BUS_REPORT_MODEM */
	APID_BUS_DATA_SAVING			= 0x46,         /**< APID_BUS_DATA_SAVING */
	APID_BUS_NACK					= 0x50,         /**< APID_BUS_FAULT_TM */
	APID_BUS_ACK					= 0x51,         /**< APID_BUS_FAULT_TC */

	APID_BUS_GET_CURRENT_TIME		= 0x64,     	/**< APID_BUS_GET_CURRENT_TIME */
	APID_BUS_SET_CURRENT_TIME		= 0x65,     	/**< APID_BUS_SET_CURRENT_TIME */

} apid_t;


/* Required data from TC */
typedef enum
{
	REQUIRED_DATA_GNSS					= 0b1,
	REQUIRED_DATA_ACELL					= 0b10,
	REQUIRED_DATA_GYRO					= 0b100,
	REQUIRED_DATA_MAGNETOMETER			= 0b1000,
	REQUIRED_DATA_PRESSURE				= 0b10000,
	REQUIRED_DATA_TEMPERATURE_OUTDOOR	= 0b100000,
	REQUIRED_DATA_TEMPERATURE_UP		= 0b1000000,
	REQUIRED_DATA_TEMPERATURE_BATTERY	= 0b10000000,
	REQUIRED_DATA_TEMPERATURE_DOWN		= 0b100000000,
	REQUIRED_DATA_PV_VOLTAGE			= 0b1000000000,
	REQUIRED_DATA_PV_CURRENT			= 0b10000000000,
	REQUIRED_DATA_BATTERY_VOLTAGE		= 0b100000000000,
	REQUIRED_DATA_BATTERY_CURRENT		= 0b1000000000000,
	REQUIRED_DATA_BATTERY_CHARGING		= 0b10000000000000,
	REQUIRED_DATA_BATTERY_DIETEMP		= 0b100000000000000,
	REQUIRED_DATA_CURRENT_TIME			= 0b1000000000000000,
	REQUIRED_DATA_ALL					= 0b1111111111111111
} tc_required_data_t;


typedef struct
{
	uint16_t v_in;
	uint16_t v_out;
	uint16_t v_bat;
	uint16_t i_bat;
	uint16_t i_in;
	uint16_t uc_temp;
	uint16_t die_temp;
	uint16_t thermistor_voltage;
	uint16_t bsr;
	uint16_t charge_state;
}eps_data_t;



/* Private variables ---------------------------------------------------------*/
extern uint8_t modem_buffer_tx[BUS_PACKET_BUS_SIZE+BUS_PACKET_FRAME_SYNC_SIZE];
extern uint8_t *const modem_data_tx;

extern uint8_t eps_buffer_tx[BUS_PACKET_BUS_SIZE+BUS_PACKET_FRAME_SYNC_SIZE];
extern uint8_t *const eps_data_tx;

extern uint8_t buffer_data [BUS_PACKET_DATA_SIZE];


/* Control de telecomandos y telemetrías */
extern uint32_t last_tc_apid;
extern uint32_t last_tc_number;
extern uint32_t last_tm_apid;
extern uint32_t last_tm_number;
extern tc_required_data_t programmed_tm;


extern sc_status_t sc_status;

extern eps_data_t eps;

extern RTC_TimeTypeDef rtc;


/* Private function prototypes -----------------------------------------------*/
uint8_t dataRequired2bytes(tc_required_data_t requirements, uint8_t *pdata);

HAL_StatusTypeDef set_time(uint8_t hours,uint8_t min, uint8_t sec);
HAL_StatusTypeDef get_time(uint8_t *hours, uint8_t *min, uint8_t *sec);

HAL_StatusTypeDef send_TM();
HAL_StatusTypeDef send_programmed_TM();
HAL_StatusTypeDef send_data_saving();
HAL_StatusTypeDef send_nack(UART_HandleTypeDef *huart);
HAL_StatusTypeDef send_ack(UART_HandleTypeDef *huart);

void check_status();

void intelliOBC_TC(apid_t apid, uint8_t *data);
void intelliOBC_TM(uint8_t apid, uint8_t *data);


#endif /* INC_INTELLI_H_ */
