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
#include "stm32f3xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "printing.h"
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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define I2C1_TE3_Pin GPIO_PIN_0
#define I2C1_TE3_GPIO_Port GPIOF
#define SPI1_CS5_Pin GPIO_PIN_1
#define SPI1_CS5_GPIO_Port GPIOF
#define SPI1_CS4_Pin GPIO_PIN_0
#define SPI1_CS4_GPIO_Port GPIOA
#define SPI1_CS0_Pin GPIO_PIN_1
#define SPI1_CS0_GPIO_Port GPIOA
#define I2C1_TE1_Pin GPIO_PIN_2
#define I2C1_TE1_GPIO_Port GPIOA
#define I2C1_TE4_Pin GPIO_PIN_3
#define I2C1_TE4_GPIO_Port GPIOA
#define SPI1_CS6_Pin GPIO_PIN_4
#define SPI1_CS6_GPIO_Port GPIOA
#define SPI1_CS3_Pin GPIO_PIN_0
#define SPI1_CS3_GPIO_Port GPIOB
#define I2C1_TE5_Pin GPIO_PIN_8
#define I2C1_TE5_GPIO_Port GPIOA
#define SPI1_CS7_Pin GPIO_PIN_15
#define SPI1_CS7_GPIO_Port GPIOA
#define SPI1_CS2_Pin GPIO_PIN_3
#define SPI1_CS2_GPIO_Port GPIOB
#define I2C1_TE2_Pin GPIO_PIN_4
#define I2C1_TE2_GPIO_Port GPIOB
#define SPI1_CS1_Pin GPIO_PIN_5
#define SPI1_CS1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
