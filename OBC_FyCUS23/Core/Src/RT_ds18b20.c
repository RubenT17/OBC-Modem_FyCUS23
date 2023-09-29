/**
  ******************************************************************************
  * @file           : RT_ds18b20.c
  * @brief          : DS18B20 digital thermometer library for STM32 HAL.
  *
  * @author         Rubén Torres Bermúdez <rubentorresbermudez@gmail.com>
  ******************************************************************************
  * @attention
  *
  *  Created on:     13.09.2021
  *
  *  Description:
  *		This library is used to control one or more DS18B20 digital thermometers
  *		with STM32 HAL.
  *
  *		Basic example:
  *		DS18B20_Reset_CMD(DS_GPIO_Port, DS_Pin);
  *		DS18B20_ReadROM_CMD(DS_GPIO_Port, DS_Pin, dirb);
  *		DS18B20_Reset_CMD(DS_GPIO_Port, DS_Pin);
  *		DS18B20_SetResolution(DS_GPIO_Port, DS_Pin, dirb, DS18B20_RESOLUTION_10);
  *		DS18B20_All_Convert(DS_GPIO_Port, DS_Pin);
  *		temp = DS18B20_Read_Temp(DS_GPIO_Port, DS_Pin, dirb);
  *
  *		Warning:
  *		Before sending each command (ending in _CMD()), use the DS18B20_Reset_CMD()
  *		function.
  *		The GPIO used must be OUTPUT OPEN DRAIN and HIGH output speed.
  *		Configure the delayMicros() function in your main.c or in RT_ds18b20.c
  *
  *  Copyright (C) 2021 Rubén Torres Bermúdez
  ******************************************************************************
  */


#include <RT_ds18b20.h>

__weak void delayMicro(uint32_t delay){}

void _write_Bit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState bit_tx);
_Bool _read_Bit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void _write_Byte(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t data_tx);
uint8_t _read_Byte(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);



/**
  * @brief  Write a bit with onewire communication.
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @param  bit_tx specifies the value to be written.
  *          This parameter can be one of the GPIO_PinState enum values:
  *            @arg GPIO_PIN_RESET: to write 0.
  *            @arg GPIO_PIN_SET: to write 1.
  * @retval None.
  */
void _write_Bit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState bit_tx)
{
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	delayMicro(3);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, bit_tx);
	delayMicro(57);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
}



/**
  * @brief  Read a bit with onewire communication.
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @retval The bit read.
  */
_Bool _read_Bit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	_Bool bit_rx;
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	delayMicro(2);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
	delayMicro(10);
	bit_rx = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
	delayMicro(48);
	return bit_rx;
}



/**
  * @brief  Write a byte with onewire communication.
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @param  data_tx specifies the byte to be written.
  * @retval None
  */
void _write_Byte(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t data_tx)
{
	uint8_t mask = 0b00000001;
	uint8_t bit_tx = data_tx & mask;
	for(uint8_t i=0; i<8; i++)
	{
		_write_Bit(GPIOx, GPIO_Pin, bit_tx);
		delayMicro(3);
        mask = mask << 1;
        bit_tx = data_tx & mask;
	}
}



/**
  * @brief  Read a byte with onewire communication.
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @retval The byte read.
  */
uint8_t _read_Byte(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	uint8_t byte_rx = 0;
	for(uint8_t i=0;  i < 8; i++)
	{
		byte_rx |= _read_Bit(GPIOx, GPIO_Pin) << i;
	}
	return byte_rx;
}

// ****************************************************************************************






/**
  * @brief  Send a command to initializes the onewire communication and the connected device(s).
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @retval This parameter can be one of the enum values:
  *            @arg HAL_OK: if there are device(s) on the onewire communication.
  *            @arg HAL_ERROR: if there aren't device(s) on the onewire communication.
  */
HAL_StatusTypeDef DS18B20_Reset_CMD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	uint8_t fault;
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	delayMicro(500);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
	delayMicro(100);
	fault = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin); // Reset if success
	delayMicro(500);
	return fault ? HAL_ERROR : HAL_OK; //true:false
}



void DS18B20_ReadROM_CMD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t *dirb)
{
	_write_Byte(GPIOx, GPIO_Pin, DS18B20_READ_ROM);

	dirb[0] = _read_Byte(GPIOx, GPIO_Pin);
	dirb[1] = _read_Byte(GPIOx, GPIO_Pin);
	dirb[2] = _read_Byte(GPIOx, GPIO_Pin);
	dirb[3] = _read_Byte(GPIOx, GPIO_Pin);
	dirb[4] = _read_Byte(GPIOx, GPIO_Pin);
	dirb[5] = _read_Byte(GPIOx, GPIO_Pin);
	dirb[6] = _read_Byte(GPIOx, GPIO_Pin);
	dirb[7] = _read_Byte(GPIOx, GPIO_Pin);
}



/**
  * @brief  Send a command to select a onewire device.
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @param	dirb specifies the 64-bit lasered ROM code of the onewire device LSBF.
  * @retval None.
  */
void DS18B20_MatchROM_CMD (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t* dirb)
{
	_write_Byte(GPIOx, GPIO_Pin, DS18B20_MATCH_ROM);
	_write_Byte(GPIOx, GPIO_Pin, dirb[0]);
	_write_Byte(GPIOx, GPIO_Pin, dirb[1]);
	_write_Byte(GPIOx, GPIO_Pin, dirb[2]);
	_write_Byte(GPIOx, GPIO_Pin, dirb[3]);
	_write_Byte(GPIOx, GPIO_Pin, dirb[4]);
	_write_Byte(GPIOx, GPIO_Pin, dirb[5]);
	_write_Byte(GPIOx, GPIO_Pin, dirb[6]);
	_write_Byte(GPIOx, GPIO_Pin, dirb[7]);
}


/**
  * @brief  Send a command to convert A/D the temperature of the selected onewire device.
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @retval This parameter can be one of the enum values:
  *            @arg HAL_OK: if ADC could be done.
  *            @arg HAL_ERROR: if ADC could not be performed.
  */
HAL_StatusTypeDef DS18B20_ConvertT_CMD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	_write_Byte(GPIOx, GPIO_Pin, DS18B20_CONVERT_T);
	for(uint32_t i=0; i<37500; i++)		// Max time conversion = 20us * 37500 = 750ms
	{
		if (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_SET)
		{
			return HAL_OK;
		}
		delayMicro(20);
	}
	return HAL_ERROR;
}


/**
  * @brief  Send a command to read bytes of the scratchpad.
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @param  data buffer where the read bytes will be stored.
  * @param  n_bytes specifies the numbers of bit to be read.
  *
  * @retval This parameter can be one of the enum values:
  *            @arg HAL_OK: if success.
  *            @arg HAL_ERROR: if not success.
  */
HAL_StatusTypeDef DS18B20_ReadScratchpad_CMD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin,
		uint8_t *data, uint8_t n_bytes)
{
	if (n_bytes<1 || n_bytes>9) return HAL_ERROR;

	_write_Byte(GPIOx, GPIO_Pin, DS18B20_READ_SCRATCHPAD);
	for(uint8_t i=0; i<n_bytes; i++)
	{
		data[i] = _read_Byte(GPIOx, GPIO_Pin);
	}
	return HAL_OK;
}


/**
  * @brief  Send a command to write in scratchpad of the selected onewire device.
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @param  th_reg specifies the value to be written in the TH register.
  * @param  tl_reg specifies the value to be written in the TL register.
  * @param  resol specifies the resolution of the ADC onewire device.
  * @retval None.
  */
void DS18B20_WriteScratchpad_CMD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin,
		uint8_t th_reg, uint8_t tl_reg, ds18b20_resolution_t resol)
{
	_write_Byte(GPIOx, GPIO_Pin, DS18B20_WRITE_SCRATCHPAD);
	_write_Byte(GPIOx, GPIO_Pin, th_reg);
	_write_Byte(GPIOx, GPIO_Pin, tl_reg);
	_write_Byte(GPIOx, GPIO_Pin, resol);
}


/**
  * @brief  Send a command to search all ROM codes of onewire bus.
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @param	dirb Pointer to a direction of DS18B20
  * @retval None.
  */
void DS18B20_SearchROM_CMD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t *dirb)
{

	//TBC
}


/**
  * @brief  Send a command to select all onewire device.
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @retval None.
  */
void DS18B20_SkipROM_CMD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	_write_Byte(GPIOx, GPIO_Pin, DS18B20_SKIP_ROM);
}


/**
  * @brief  Send a command to copy TH, TL and configuration register to EEPROM.
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @retval None.
  */
void DS18B20_CopyScratchpad_CMD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	_write_Byte(GPIOx, GPIO_Pin, DS18B20_COPY_SCRATCHPAD);
}


/**
  * @brief  Send a command to restore TH, TL and configuration register from EEPROM.
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @retval None.
  */
void DS18B20_RecallE2_CMD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	_write_Byte(GPIOx, GPIO_Pin, DS18B20_RECALL_E2);
}








// *************************************************************************
/**
  * @brief  Shortcut to read the temperature of a onewire device.
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @param	dirb Pointer to a direction of DS18B20
  * @param	resol Resolution to be set
  * @retval This parameter can be one of the enum values:
  *            @arg HAL_OK: if success.
  *            @arg HAL_ERROR: if not success.
  */
HAL_StatusTypeDef DS18B20_SetResolution(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t *dirb,
		ds18b20_resolution_t resol)
{
	uint8_t data[4] = {0};

	if(DS18B20_Reset_CMD(GPIOx, GPIO_Pin) == HAL_ERROR)	return HAL_ERROR;
	DS18B20_MatchROM_CMD(GPIOx, GPIO_Pin, dirb);
	DS18B20_ReadScratchpad_CMD(GPIOx, GPIO_Pin, data, 4);

	if(DS18B20_Reset_CMD(GPIOx, GPIO_Pin) == HAL_ERROR)	return HAL_ERROR;
	DS18B20_MatchROM_CMD(GPIOx, GPIO_Pin, dirb);
	DS18B20_WriteScratchpad_CMD(GPIOx, GPIO_Pin, data[2], data[3], resol);

	return HAL_OK;
}


/**
  * @brief  Read the scratchpad of the selected onewire device and decode the data temperature.
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15)
  * @param  data specifies two bytes with DS18B20 temperature.
  * @retval A float value that specifies the decoded temperature.
  */
float DS18B20_Decode_Temperature (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t *data)
{
	float temperature = 0.0;
	uint8_t temp_LSB = data[0];
	uint8_t temp_MSB = data[1];
	uint16_t u16_temp = ((temp_MSB << 8) | temp_LSB);
	uint16_t mask = 0b0000000000000001;

	for (int8_t k = -4; k < 7; k++)
	{
		if (u16_temp & mask)
		{
			temperature += powf(2,k);
		}
		mask = mask << 1;
	}
	return temperature;
}



/**
  * @brief  Shortcut to do the conversion and read of the temperature in a onewire device.
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @param	dirb specifies the 64-bit lasered ROM code of the onewire device LSBF.
  * @retval A float value that specifies the temperature of the onewire device
  * 		or -99 if there was a error.
  */
float DS18B20_ConvertRead_Temp(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t *dirb)
{
	float temp;
	uint8_t data[2]={0};

	if(DS18B20_Reset_CMD(GPIOx, GPIO_Pin) == HAL_ERROR)	return -99.0;
	DS18B20_MatchROM_CMD(GPIOx, GPIO_Pin, dirb);
	DS18B20_ConvertT_CMD(GPIOx, GPIO_Pin);

	if(DS18B20_Reset_CMD(GPIOx, GPIO_Pin) == HAL_ERROR)	return -99.0;
	DS18B20_MatchROM_CMD(GPIOx, GPIO_Pin, dirb);
	DS18B20_ReadScratchpad_CMD(GPIOx, GPIO_Pin, data, 2);
	temp = DS18B20_Decode_Temperature(GPIOx, GPIO_Pin, data);
	return temp;
}



/**
  * @brief  Shortcut to do the A/D conversion on all onewire device(s).
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @retval This parameter can be one of the enum values:
  *            @arg HAL_OK: if success.
  *            @arg HAL_ERROR: if not success.
  */
HAL_StatusTypeDef DS18B20_All_Convert(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	if(DS18B20_Reset_CMD(GPIOx, GPIO_Pin) == HAL_ERROR)	return HAL_ERROR;
	DS18B20_SkipROM_CMD(GPIOx, GPIO_Pin);
	DS18B20_ConvertT_CMD(GPIOx, GPIO_Pin);
	return HAL_OK;
}



/**
  * @brief  Shortcut to read the temperature of a onewire device.
  * @param  GPIOx where x can be (A..K) to select the GPIO peripheral.
  * @param  GPIO_Pin specifies the port bit to be written.
  *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
  * @param	dirb specifies the 64-bit lasered ROM code of the onewire device LSBF.
  * @retval A float value that specifies the temperature of the onewire device
  * 		or -99.0 if there was a error.
  */
float DS18B20_Read_Temp(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t* dirb)
{
	uint8_t data[2]={0};

	if(DS18B20_Reset_CMD(GPIOx, GPIO_Pin) == HAL_ERROR)	return -99.0;
	DS18B20_MatchROM_CMD(GPIOx, GPIO_Pin, dirb);
	DS18B20_ReadScratchpad_CMD(GPIOx, GPIO_Pin, data, 2);
	return DS18B20_Decode_Temperature(GPIOx, GPIO_Pin, data);
}
