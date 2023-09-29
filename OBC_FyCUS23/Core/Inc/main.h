/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

extern CRC_HandleTypeDef hcrc;

extern I2C_HandleTypeDef hi2c1;

extern IWDG_HandleTypeDef hiwdg;

extern UART_HandleTypeDef hlpuart1;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_lpuart_tx;
extern DMA_HandleTypeDef hdma_usart3_tx;

extern RTC_HandleTypeDef hrtc;

extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim16;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EPS_UART_HANDLE &huart1
#define PRIMARY_TIM_ARR ((7 *10000/5)-1)
#define SECONDARY_TIM_HANDLE &htim16
#define MODEM_UART_BAUDRATE 500000
#define GNSS_UART USART3
#define GNSS_UART_HANDLE &huart3
#define MICROS_TIM_ARR 65535
#define PRIMARY_TIM TIM7
#define MODEM_UART_HANDLE &hlpuart1
#define SECONDARY_TIM TIM16
#define EPS_UART USART1
#define PRIMARY_TIM_HANDLE &htim7
#define MICROS_TIM_HANDLE &htim6
#define MICROS_TIM_PRESCALER (80-1)
#define EPS_UART_BAUDRATE 460800
#define SECONDARY_TIM_ARR ((3 *10000/5) -1)
#define MODEM_UART LPUART1
#define PRIMARY_TIM_PRESCALER ((8000*5)-1)
#define MICROS_TIM TIM6
#define SECONDARY_TIM_PRESCALER (8000*5-1)
#define RTC_HANDLE &hrtc
#define ONEWIRE_Pin GPIO_PIN_3
#define ONEWIRE_GPIO_Port GPIOA
#define MKR_INT1_Pin GPIO_PIN_0
#define MKR_INT1_GPIO_Port GPIOB
#define MKR_INT1_EXTI_IRQn EXTI0_IRQn
#define MKR_RST_Pin GPIO_PIN_2
#define MKR_RST_GPIO_Port GPIOB
#define LED3_Pin GPIO_PIN_12
#define LED3_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_13
#define LED2_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_14
#define LED1_GPIO_Port GPIOB
#define IMU_INT2_Pin GPIO_PIN_15
#define IMU_INT2_GPIO_Port GPIOB
#define IMU_INT2_EXTI_IRQn EXTI15_10_IRQn
#define IMU_INT1_Pin GPIO_PIN_6
#define IMU_INT1_GPIO_Port GPIOC
#define IMU_INT1_EXTI_IRQn EXTI9_5_IRQn
#define MAG_INT1_Pin GPIO_PIN_7
#define MAG_INT1_GPIO_Port GPIOC
#define MAG_INT1_EXTI_IRQn EXTI9_5_IRQn
#define RADFET_R2_Pin GPIO_PIN_9
#define RADFET_R2_GPIO_Port GPIOC
#define RADFET_R1_Pin GPIO_PIN_8
#define RADFET_R1_GPIO_Port GPIOA
#define EPS_INT1_Pin GPIO_PIN_12
#define EPS_INT1_GPIO_Port GPIOC
#define EPS_INT1_EXTI_IRQn EXTI15_10_IRQn
#define GPS_INT1_Pin GPIO_PIN_8
#define GPS_INT1_GPIO_Port GPIOB
#define GPS_INT1_EXTI_IRQn EXTI9_5_IRQn
#define GPS_RST_Pin GPIO_PIN_9
#define GPS_RST_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
