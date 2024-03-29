/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Button_Back_Pin GPIO_PIN_0
#define Button_Back_GPIO_Port GPIOC
#define Button_Back_EXTI_IRQn EXTI0_IRQn
#define Button_Cancel_Pin GPIO_PIN_1
#define Button_Cancel_GPIO_Port GPIOC
#define Button_Cancel_EXTI_IRQn EXTI1_IRQn
#define Rotary_CLK_Pin GPIO_PIN_0
#define Rotary_CLK_GPIO_Port GPIOA
#define Rotary_DT_Pin GPIO_PIN_1
#define Rotary_DT_GPIO_Port GPIOA
#define PR_CTRL_0_Pin GPIO_PIN_2
#define PR_CTRL_0_GPIO_Port GPIOA
#define PR_CTRL_1_Pin GPIO_PIN_3
#define PR_CTRL_1_GPIO_Port GPIOA
#define Rotary_SW_Pin GPIO_PIN_4
#define Rotary_SW_GPIO_Port GPIOA
#define Rotary_SW_EXTI_IRQn EXTI4_IRQn
#define ALRT_OC_3_Pin GPIO_PIN_5
#define ALRT_OC_3_GPIO_Port GPIOA
#define ALRT3_1_Pin GPIO_PIN_6
#define ALRT3_1_GPIO_Port GPIOA
#define ALRT2_1_Pin GPIO_PIN_7
#define ALRT2_1_GPIO_Port GPIOA
#define ALRT3_4_Pin GPIO_PIN_4
#define ALRT3_4_GPIO_Port GPIOC
#define ALRT1_2_Pin GPIO_PIN_5
#define ALRT1_2_GPIO_Port GPIOC
#define LED_BLUE_Pin GPIO_PIN_0
#define LED_BLUE_GPIO_Port GPIOB
#define ALRT4_3_Pin GPIO_PIN_1
#define ALRT4_3_GPIO_Port GPIOB
#define ALRT2_3_Pin GPIO_PIN_2
#define ALRT2_3_GPIO_Port GPIOB
#define ALRT3_2_Pin GPIO_PIN_12
#define ALRT3_2_GPIO_Port GPIOB
#define ALRT1_4_Pin GPIO_PIN_13
#define ALRT1_4_GPIO_Port GPIOB
#define ALRT_OC_4_Pin GPIO_PIN_14
#define ALRT_OC_4_GPIO_Port GPIOB
#define ALRT_SC_4_Pin GPIO_PIN_15
#define ALRT_SC_4_GPIO_Port GPIOB
#define ALRT_OC_2_Pin GPIO_PIN_6
#define ALRT_OC_2_GPIO_Port GPIOC
#define ALRT2_2_Pin GPIO_PIN_7
#define ALRT2_2_GPIO_Port GPIOC
#define ALRT_SC_2_Pin GPIO_PIN_8
#define ALRT_SC_2_GPIO_Port GPIOC
#define ALRT4_1_Pin GPIO_PIN_9
#define ALRT4_1_GPIO_Port GPIOC
#define ALRT3_3_Pin GPIO_PIN_9
#define ALRT3_3_GPIO_Port GPIOA
#define ALRT4_4_Pin GPIO_PIN_10
#define ALRT4_4_GPIO_Port GPIOA
#define ALRT1_3_Pin GPIO_PIN_11
#define ALRT1_3_GPIO_Port GPIOA
#define ALRT_SC_3_Pin GPIO_PIN_12
#define ALRT_SC_3_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define ALRT_SC_1_Pin GPIO_PIN_11
#define ALRT_SC_1_GPIO_Port GPIOC
#define ALRT_OC_1_Pin GPIO_PIN_12
#define ALRT_OC_1_GPIO_Port GPIOC
#define ALRT2_4_Pin GPIO_PIN_5
#define ALRT2_4_GPIO_Port GPIOB
#define ALRT4_2_Pin GPIO_PIN_6
#define ALRT4_2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
