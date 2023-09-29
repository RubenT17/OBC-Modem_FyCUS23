/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  *
  * @author         Rubén Torres Bermúdez <rubentorresbermudez@gmail.com>
  ******************************************************************************
  * @attention
  *
  *  Description:
  *		This source file is used to control OBC+AOCS subsystem in FyCUS 2023
  *		project.
  *
  *  Copyright (C) 2023 Rubén Torres Bermúdez
  ******************************************************************************
  */

/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <string.h>
#include "intelli.h"
#include "aocs.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum {
	ISR_FLAG_NONE 			= 0,
	ISR_FLAG_GNSS			= 0b1,
	ISR_FLAG_IMU_ACELL		= 0b10,
	ISR_FLAG_IMU_GYRO		= 0b100,
	ISR_FLAG_MAG			= 0b1000,
	ISR_FLAG_MODEM_BUS		= 0b10000,
	ISR_FLAG_MODEM_GPIO		= 0b100000,
	ISR_FLAG_EPS_BUS		= 0b1000000,
	ISR_FLAG_EPS_GPIO		= 0b10000000,
	ISR_FLAG_PRIMARY_TIM	= 0b100000000,
	ISR_FLAG_SECONDARY_TIM	= 0b1000000000,
	ISR_FLAG_PUBX_MSG		= 0b10000000000,
}isr_flag_t;



/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DEBUG_LED				1
#define LOW_SENSOR_READ_FAST	1
#define GNSS_READ_PUBX			0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

I2C_HandleTypeDef hi2c1;

IWDG_HandleTypeDef hiwdg;

UART_HandleTypeDef hlpuart1;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_lpuart_tx;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart3_rx;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim16;

/* USER CODE BEGIN PV */

/* Estado de los sistemas */
volatile isr_flag_t isr_flag = ISR_FLAG_NONE;



/* Variables del buffer de la EPS */
uint8_t eps_buffer_rx[BUS_PACKET_BUS_SIZE] = {0};
bus_packet_t eps_packet = {0};

/* Variables del bus del modem */
uint8_t modem_buffer_rx[BUS_PACKET_BUS_SIZE] = {0};
bus_packet_t modem_packet = {0};


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_TIM6_Init(void);
static void MX_IWDG_Init(void);
static void MX_CRC_Init(void);
static void MX_TIM7_Init(void);
static void MX_TIM16_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
void delayMicro (uint32_t us_delay);
static inline void uart_gnss_handle(UART_HandleTypeDef *huart);
static inline void uart_eps_handle(UART_HandleTypeDef *huart);
static inline void uart_modem_handle(UART_HandleTypeDef *huart);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void delayMicro (uint32_t us_delay)
{
	__HAL_TIM_SET_COUNTER(MICROS_TIM_HANDLE, 0);
	while (__HAL_TIM_GET_COUNTER(MICROS_TIM_HANDLE) < us_delay);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_LPUART1_UART_Init();
  MX_TIM6_Init();
  MX_IWDG_Init();
  MX_CRC_Init();
  MX_TIM7_Init();
  MX_TIM16_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */


  /* INICIO DE LA CONFIGURACIÓN */
#if DEBUG_LED
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
#endif

  /* Seguridad del microcontrolador */
  HAL_StatusTypeDef err = HAL_ERROR;
  HAL_IWDG_Init(&hiwdg);
  HAL_Delay(30);


  /* Configuración de las comunicaciones */
  bus_packet_CRC16CCSDSConfig();
  HAL_UART_Receive_DMA(GNSS_UART_HANDLE, gnss_ring, 1);
  HAL_UART_Receive_IT(EPS_UART_HANDLE, eps_buffer_rx, 1);
  HAL_UART_Receive_IT(MODEM_UART_HANDLE, modem_buffer_rx, 1);


  /* Tareas temporizadas y timers necesarios */
  HAL_TIM_Base_Start_IT(PRIMARY_TIM_HANDLE);
  HAL_TIM_Base_Start_IT(SECONDARY_TIM_HANDLE);
  HAL_TIM_Base_Start(MICROS_TIM_HANDLE);




  /* ********* AOCS ********* */

  /* Configuracion de magnetómetro */
  err = aocs_config_magnetometer();
  if (err != HAL_OK) sc_status.obc |= OBC_ERROR_MAG;
  else sc_status.obc &= ~OBC_ERROR_MAG;

  /* Configuración de la IMU */
  err = aocs_config_imu();
  if (err != HAL_OK) sc_status.obc |= OBC_ERROR_IMU;
  else sc_status.obc &= ~OBC_ERROR_IMU;

  /* Configuración del altímetro */
  err = aocs_config_altimeter();
  if (err != HAL_OK) sc_status.obc |= OBC_ERROR_ALTIM;
  else sc_status.obc &= ~OBC_ERROR_ALTIM;

  /* Configuracion de sondas de temperatura */
  err = aocs_config_temp();
  if (err != HAL_OK) sc_status.obc |= OBC_ERROR_TEMPS;
  else sc_status.obc &= ~OBC_ERROR_TEMPS;

  /* Configuración del GNSS */
  err = aocs_config_gnss();
  if (err != HAL_OK) sc_status.obc |= OBC_ERROR_GNSS;
  else sc_status.obc &= ~OBC_ERROR_GNSS;

//  /* Configuracion del RTC */
//  err = set_time(0, 0, 0);
//  if (err != HAL_OK) sc_status.obc |= OBC_ERROR_RTC;
//  else sc_status.obc &= ~OBC_ERROR_RTC;

  HAL_IWDG_Refresh(&hiwdg);
#if DEBUG_LED
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
#endif
  /* FIN DE LA CONFIGURACIÓN */




  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  while(isr_flag != ISR_FLAG_NONE)
	  {
		  HAL_ResumeTick();
		  HAL_IWDG_Refresh(&hiwdg);

#if(!LOW_SENSOR_READ_FAST)
		  if(isr_flag&ISR_FLAG_IMU_ACELL)
		  {
			  if(get_acell() != HAL_OK) sc_status.obc |= OBC_ERROR_IMU;
			  else sc_status.obc &= ~OBC_ERROR_IMU;
			  isr_flag ^= ISR_FLAG_IMU_ACELL;
		  }

		  if(isr_flag&ISR_FLAG_IMU_GYRO)
		  {
			  if(get_gyro() != HAL_OK) sc_status.obc |= OBC_ERROR_IMU;
			  else sc_status.obc &= ~OBC_ERROR_IMU;
			  isr_flag ^= ISR_FLAG_IMU_GYRO;
		  }

		  if(isr_flag&ISR_FLAG_MAG)
		  {
			  if(get_mag() != HAL_OK) sc_status.obc |= OBC_ERROR_MAG;
			  else sc_status.obc &= ~OBC_ERROR_MAG;
			  isr_flag ^= ISR_FLAG_MAG;
		  }
#endif

		  if(isr_flag&ISR_FLAG_PRIMARY_TIM)
		  {
			  send_programmed_TM();
			  __HAL_TIM_SET_COUNTER(PRIMARY_TIM_HANDLE, 0);
			  HAL_TIM_Base_Start_IT(PRIMARY_TIM_HANDLE);
			  isr_flag ^= ISR_FLAG_PRIMARY_TIM;

#if DEBUG_LED
			  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
			  HAL_Delay(2);
			  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
#endif
		  }


		  if(isr_flag&ISR_FLAG_GNSS)
		  {
			  if(aocs_get_gnss() != HAL_OK) sc_status.obc |= OBC_ERROR_GNSS;
			  else sc_status.obc &= ~OBC_ERROR_GNSS;
			  isr_flag ^= ISR_FLAG_GNSS;
		  }

		  else if(isr_flag&ISR_FLAG_MODEM_BUS)
		  {
			  if(bus_packet_Decode(modem_buffer_rx, &modem_packet) != HAL_OK)	sc_status.obc |= OBC_ERROR_RX_MODEM;
			  else sc_status.obc &= ~OBC_ERROR_RX_MODEM;

			  if(modem_packet.packet_type == BUS_PACKET_TYPE_TM)	  		intelliOBC_TM(modem_packet.apid, modem_packet.data);
			  else /*if(modem_packet.packet_type == BUS_PACKET_TYPE_TC)*/	intelliOBC_TC(modem_packet.apid, modem_packet.data);

			  isr_flag ^= ISR_FLAG_MODEM_BUS;

#if DEBUG_LED
			  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
			  HAL_Delay(2);
			  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
#endif
		  }


		  else if(isr_flag&ISR_FLAG_EPS_BUS)
		  {
			  if(bus_packet_Decode(eps_buffer_rx, &eps_packet) != HAL_OK)
			  {
				  sc_status.obc |= OBC_ERROR_RX_EPS;
				  send_nack(EPS_UART_HANDLE);
			  }
			  else
			  {
				  sc_status.obc &= ~OBC_ERROR_RX_EPS;
				  send_ack(EPS_UART_HANDLE);
			  }

			  if(eps_packet.packet_type == BUS_PACKET_TYPE_TM)	  		intelliOBC_TM(eps_packet.apid, eps_packet.data);
			  else /*if(eps_packet.packet_type == BUS_PACKET_TYPE_TC)*/	intelliOBC_TC(eps_packet.apid, eps_packet.data);

			  isr_flag ^= ISR_FLAG_EPS_BUS;
		  }


		  else if(isr_flag&ISR_FLAG_SECONDARY_TIM)
		  {
			  check_status();
#if(LOW_SENSOR_READ_FAST)
			  if(aocs_get_imu() != HAL_OK) sc_status.obc |= OBC_ERROR_IMU;
			  else sc_status.obc &= ~OBC_ERROR_IMU;

			  if(aocs_get_mag() != HAL_OK) sc_status.obc |= OBC_ERROR_MAG;
			  else sc_status.obc &= ~OBC_ERROR_MAG;
#endif
			  if(aocs_get_altimeter() != HAL_OK) sc_status.obc |= OBC_ERROR_ALTIM;
			  else sc_status.obc &= ~OBC_ERROR_ALTIM;

			  if(aocs_get_temp() != HAL_OK) sc_status.obc |= OBC_ERROR_TEMPS;
			  else sc_status.obc &= ~OBC_ERROR_TEMPS;

			  if(!(isr_flag&ISR_FLAG_EPS_GPIO))		sc_status.eps |= 0x80;
			  else
			  {
				  sc_status.eps &= ~(0x80);
				  isr_flag ^= ISR_FLAG_EPS_GPIO;
			  }

			  if(!(isr_flag&ISR_FLAG_MODEM_GPIO))	sc_status.modem |= 0x80;
			  else
			  {
				  sc_status.modem &= ~(0x80);
				  isr_flag ^= ISR_FLAG_MODEM_GPIO;
			  }


			  send_data_saving();

			  __HAL_TIM_SET_COUNTER(SECONDARY_TIM_HANDLE, 0);
			  HAL_TIM_Base_Start_IT(SECONDARY_TIM_HANDLE);
			  isr_flag ^= ISR_FLAG_SECONDARY_TIM;
		  }


#if(GNSS_READ_PUBX)
		  else if(isr_flag&ISR_FLAG_PUBX_MSG)
		  {
			  if(get_pubx() != HAL_OK) sc_status.obc |= OBC_ERROR_PUBX;
			  else sc_status.obc &= ~OBC_ERROR_PUBX;

			  isr_flag ^= ISR_FLAG_PUBX_MSG;
		  }
#endif

//		  else if(isr_flag&ISR_FLAG_MODEM_GPIO)
//		  {
//#warning "Poner funcionamiento GPIO"
//			  isr_flag ^= ISR_FLAG_MODEM_GPIO;
//		  }
//
//		  else if(isr_flag&ISR_FLAG_EPS_GPIO)
//		  {
//#warning "Poner funcionamiento GPIO"
//			  isr_flag ^= ISR_FLAG_EPS_GPIO;
//		  }

		  HAL_IWDG_Refresh(&hiwdg);
	  }


#if DEBUG_LED
	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
#endif
	HAL_IWDG_Refresh(&hiwdg);
	HAL_SuspendTick();
	HAL_PWR_EnableSleepOnExit();
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
#if DEBUG_LED
	  HAL_ResumeTick();
	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
#endif

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;
  hcrc.Init.InitValue = 0;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10909CEC;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_8;
  hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 3999;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief LPUART1 Initialization Function
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = MODEM_UART_BAUDRATE;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = EPS_UART_BAUDRATE;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = MICROS_TIM_PRESCALER;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = MICROS_TIM_ARR;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = PRIMARY_TIM_PRESCALER;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = PRIMARY_TIM_ARR;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief TIM16 Initialization Function
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = SECONDARY_TIM_PRESCALER;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = SECONDARY_TIM_ARR;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA2_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, ONEWIRE_Pin|RADFET_R1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, MKR_RST_Pin|GPS_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED3_Pin|LED2_Pin|LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RADFET_R2_GPIO_Port, RADFET_R2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : ONEWIRE_Pin */
  GPIO_InitStruct.Pin = ONEWIRE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(ONEWIRE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : MKR_INT1_Pin IMU_INT2_Pin */
  GPIO_InitStruct.Pin = MKR_INT1_Pin|IMU_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : MKR_RST_Pin */
  GPIO_InitStruct.Pin = MKR_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MKR_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED3_Pin LED2_Pin LED1_Pin */
  GPIO_InitStruct.Pin = LED3_Pin|LED2_Pin|LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : IMU_INT1_Pin MAG_INT1_Pin EPS_INT1_Pin */
  GPIO_InitStruct.Pin = IMU_INT1_Pin|MAG_INT1_Pin|EPS_INT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : RADFET_R2_Pin */
  GPIO_InitStruct.Pin = RADFET_R2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RADFET_R2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RADFET_R1_Pin */
  GPIO_InitStruct.Pin = RADFET_R1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RADFET_R1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : GPS_INT1_Pin */
  GPIO_InitStruct.Pin = GPS_INT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPS_INT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : GPS_RST_Pin */
  GPIO_InitStruct.Pin = GPS_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPS_RST_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if 		(htim->Instance == PRIMARY_TIM)		isr_flag |= ISR_FLAG_PRIMARY_TIM;
	else if (htim->Instance == SECONDARY_TIM)	isr_flag |= ISR_FLAG_SECONDARY_TIM;
	else 										return;

	HAL_PWR_DisableSleepOnExit();
}

void HAL_TIM_PeriodElapsedHalfCpltCallback(TIM_HandleTypeDef *htim)
{
	HAL_IWDG_Refresh(&hiwdg);
}



void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin)
	{
#if(!LOW_SENSOR_READ_FAST)
	case MAG_INT1_Pin:	isr_flag |= ISR_FLAG_MAG; 			break;
	case IMU_INT1_Pin:	isr_flag |= ISR_FLAG_IMU_ACELL;		break;
	case IMU_INT2_Pin:	isr_flag |= ISR_FLAG_IMU_GYRO;		break;
#endif
	case EPS_INT1_Pin:	isr_flag |= ISR_FLAG_EPS_GPIO; 		break;
	case MKR_INT1_Pin:	isr_flag |= ISR_FLAG_MODEM_GPIO;	break;
	case GPS_INT1_Pin:	isr_flag |= ISR_FLAG_GNSS; 			break;

	default: 												return;
	}
	HAL_PWR_DisableSleepOnExit();

}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_ResumeTick();

	if(huart->Instance == GNSS_UART)		// GPS
		uart_gnss_handle(huart);

	else if(huart->Instance == EPS_UART)	// EPS
		uart_eps_handle(huart);

	else if(huart->Instance == MODEM_UART)	// MODEM
		uart_modem_handle(huart);
}




static inline void uart_gnss_handle(UART_HandleTypeDef *huart)
{
	static uint32_t gnss_ring_pos = 0;

	static uint8_t gnss_flag_nmea_ini = 0;
	static uint32_t gnss_current_ini = 0;

	if (gnss_ring[gnss_ring_pos] == '$')	// Ha encontrado un inicio de cadena?
	{
		gnss_flag_nmea_ini = 1;
		gnss_current_ini = gnss_ring_pos;
#if(GNSS_READ_PUBX)
		gnss_flag_pubx_ini = 0;
#endif
	}

	else if ((gnss_ring[gnss_ring_pos] == '\n') && gnss_flag_nmea_ini) // Ha encontrado un fin de cadena?
	{
		gnss_flag_nmea_ini = 0;
		gnss_ring_last_ini = gnss_current_ini;
		gnss_ring_last_end = gnss_ring_pos;
		isr_flag |= ISR_FLAG_GNSS;					// Interpreta la cadena
		HAL_PWR_DisableSleepOnExit();
	}

#if(GNSS_READ_PUBX)
	static uint8_t gnss_flag_pubx_ini = 0;
	static uint16_t gnss_flag_pubx_count = 0;
	static uint8_t gnss_flag_pubx_length = 0;

	else if(gnss_ring[gnss_ring_pos] == 0xB5)
	{
		gnss_flag_pubx_count=0;
		pubx_ring[gnss_flag_pubx_count] = gnss_ring[gnss_ring_pos];
		gnss_flag_pubx_ini = 1;
		gnss_flag_nmea_ini = 0;
		gnss_flag_pubx_count++;
	}

	else if(gnss_flag_pubx_ini)
	{
		if(gnss_flag_pubx_ini==1)
		{
			pubx_ring[gnss_flag_pubx_count] = gnss_ring[gnss_ring_pos];
			gnss_flag_pubx_count++;
			if(gnss_flag_pubx_count == 6)
			{
				gnss_flag_pubx_ini=2;
				gnss_flag_pubx_length = (pubx_ring[gnss_flag_pubx_count-1] << 8 | pubx_ring[gnss_flag_pubx_count-2]) + gnss_flag_pubx_count-1+2;
			}
		}

		if(gnss_flag_pubx_ini==2)
		{
			pubx_ring[gnss_flag_pubx_count] = gnss_ring[gnss_ring_pos];
			gnss_flag_pubx_count++;
			if(gnss_flag_pubx_length<gnss_flag_pubx_count)
			{
				gnss_flag_pubx_ini=0;
				isr_flag |= ISR_FLAG_PUBX_MSG;
			}
		}
	}
#endif

	gnss_ring_pos++;
	if(gnss_ring_pos >= GNSS_RING_BUFFER_SIZE) 	gnss_ring_pos = 0;	// Si ha llegado al final, dale la vuelta

	HAL_UART_Receive_DMA(huart, &gnss_ring[gnss_ring_pos], 1);
}



static inline void uart_eps_handle(UART_HandleTypeDef *huart)
{
	static uint8_t eps_packet_length = 0;
	static bus_sync_flag_t eps_sync_flag = BUS_PACKET_SYNC_FIND;
	static uint8_t eps_buffer_pos = 0;

	if (eps_sync_flag != BUS_PACKET_SYNC_COMPLETED)
	{
		eps_sync_flag = bus_packet_SyncFrameDetect(eps_sync_flag, eps_buffer_rx[eps_buffer_pos]);

		if(eps_sync_flag == BUS_PACKET_SYNC_COMPLETED)
		{
			eps_buffer_pos=0;
			HAL_UART_Receive_IT(huart, &eps_buffer_rx[eps_buffer_pos], 1);
		}
		else
		{
			eps_buffer_pos++;
			if(eps_buffer_pos >= BUS_PACKET_BUS_SIZE)
				eps_buffer_pos=0;

			HAL_UART_Receive_IT(huart, &eps_buffer_rx[eps_buffer_pos], 1);
		}
	}

	else
	{
		if(eps_buffer_pos > 0)	// Se sabe ya la longitud real?
		{
			eps_packet_length = bus_packet_GetLength(eps_buffer_rx);	// Cual era?

			if (eps_buffer_pos >= (eps_packet_length-1))		// Ha terminado la transmisión?
			{
				eps_buffer_pos = 0;
				isr_flag |= ISR_FLAG_EPS_BUS;
				eps_sync_flag = BUS_PACKET_SYNC_FIND;
				HAL_PWR_DisableSleepOnExit();
			}
			else eps_buffer_pos++;
		}
		else	eps_buffer_pos++;

		if(eps_buffer_pos < BUS_PACKET_BUS_SIZE)
			HAL_UART_Receive_IT(huart, &eps_buffer_rx[eps_buffer_pos], 1);
		else Error_Handler();
	}
}




static inline void uart_modem_handle(UART_HandleTypeDef *huart)
{
	static uint8_t modem_packet_length = 0;
	static bus_sync_flag_t modem_sync_flag = BUS_PACKET_SYNC_FIND;
	static uint8_t modem_buffer_pos = 0;

	if (modem_sync_flag != BUS_PACKET_SYNC_COMPLETED)
	{
		modem_sync_flag = bus_packet_SyncFrameDetect(modem_sync_flag, modem_buffer_rx[modem_buffer_pos]);

		if(modem_sync_flag == BUS_PACKET_SYNC_COMPLETED)
			modem_buffer_pos=0;

		else
		{
			modem_buffer_pos++;
			if(modem_buffer_pos >= BUS_PACKET_BUS_SIZE)
				modem_buffer_pos=0;
		}
	}

	else
	{
		if(modem_buffer_pos > 0)	// Se sabe ya la longitud real?
		{
			modem_packet_length = bus_packet_GetLength(modem_buffer_rx);	// Cual era?

			if (modem_buffer_pos >= (modem_packet_length-1))		// Ha terminado la transmisión?
			{
				modem_buffer_pos = 0;
				isr_flag |= ISR_FLAG_MODEM_BUS;
				modem_sync_flag = BUS_PACKET_SYNC_FIND;
				HAL_PWR_DisableSleepOnExit();
			}
			else modem_buffer_pos++;
		}
		else	modem_buffer_pos++;

		if(modem_buffer_pos >= BUS_PACKET_BUS_SIZE)
		{
			modem_buffer_pos = 0;
			modem_sync_flag = BUS_PACKET_SYNC_FIND;
		}
	}
	HAL_UART_Receive_IT(huart, &modem_buffer_rx[modem_buffer_pos], 1);
}





/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
	  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
	  for(uint32_t i=0; i<3000000; i++)
	  {
		  asm("NOP");
	  }
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
