/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include <stdint.h>
#include <stdbool.h>
#include "stm32g4xx_hal.h"
#include "stm32g4xx_ll_opamp.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_dac.h"
#include "stm32g4xx_ll_usart.h"
#include "stm32g4xx_ll_dac.h"
#include "stm32g4xx_ll_pwr.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_bus.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

#ifdef RESOLUTION_HD

#define ROW_SIZE_NTSC 26
#define ROW_SIZE_PAL 32
#define COLUMN_SIZE 45
#define CANVAS_H_R  540
#define CANVAS_H_OFFSET   76       // 4pix(1byte) aligne
#define CANVAS_H_OFFSET_GEN   64       // 4pix(1byte) aligne
#define PIX_PERIOD 14               // period =14+1 =15
#define VIDEO_GEN_LINE_NTSC  720    // 170MHz/15734Hz/15=720 ... mod4=0
#define VIDEO_GEN_LINE_PAL   724    // 170MHz/15625Hz/15=724 ... mod4=0
#define VIDEO_TIM_NS(t)    (t/88)

#else

#define ROW_SIZE_NTSC 13
#define ROW_SIZE_PAL 16
#define COLUMN_SIZE 30
#define CANVAS_H_R  360
#define CANVAS_H_OFFSET   36       // 4pix(1byte) aligne
#define CANVAS_H_OFFSET_GEN   28       // 4pix(1byte) aligne
#define PIX_PERIOD 23               // period=23+1=24
#define VIDEO_GEN_LINE_NTSC  448    // 170MHz/15734Hz/24=450.2= --mod4--> 448
#define VIDEO_GEN_LINE_PAL   452    // 170MHz/15625Hz/24=453.3 --mod4--> 452

#define VIDEO_TIM_NS(t)    (t/141)

#endif

#define CANVAS_H        (CANVAS_H_R + CANVAS_H_OFFSET)
#define CANVAS_V_OFFSET_NTSC    32
#define CANVAS_V_OFFSET_PAL     36
#define CANVAS_V_OFFSET_NTSC_GEN    16
#define CANVAS_V_OFFSET_PAL_GEN     16
#define CANVAS_V_NTSC           (234*2)     /* 18pix * 13char */
#define CANVAS_V_PAL            (288*2)     /* 18pix * 16char */
#define ROW_SIZE_MAX 32
#define COLUMN_SIZE_MAX 45

typedef enum {
    VIDEO_NTSC = 0,
    VIDEO_PAL
}VIDEO_FORMAT;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern COMP_HandleTypeDef hcomp2;
extern DAC_HandleTypeDef hdac1;
extern DAC_HandleTypeDef hdac3;
extern DMA_HandleTypeDef hdma_dac3_ch1;
extern OPAMP_HandleTypeDef hopamp1;
extern OPAMP_HandleTypeDef hopamp2;
extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim8;
extern DMA_HandleTypeDef hdma_tim1_up;
extern DMA_HandleTypeDef hdma_tim8_up;
extern UART_HandleTypeDef huart1;

typedef enum {
    DETECT_UNKNOWN =0,
    DETECT_HSYNC,
    DETECT_VSYNC,
    DETECT_EQUIVALENT,
}DETECT_SYNC;


/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern int32_t canvas_v_offset[];
extern VIDEO_FORMAT video_format;
extern int32_t canvas_v_offset[];
extern VIDEO_FORMAT video_format;
extern int32_t canvas_v[];
#define FONT_SIZE   64
#define FONT_CHARS  512
extern uint8_t font[FONT_CHARS][FONT_SIZE];
void SetLine(register volatile uint32_t *data, register volatile uint16_t *buf, int line);
void rebootDfu(void);

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
bool writeFlashFont(uint16_t addr, uint8_t *data);
void enableOSD(bool en);
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void osd_enable(uint8_t en);
void intHsyncFallEdge(void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPI_CS_Pin GPIO_PIN_0
#define SPI_CS_GPIO_Port GPIOF
#define NRST_Pin GPIO_PIN_10
#define NRST_GPIO_Port GPIOG
#define SWITCH_Pin GPIO_PIN_0
#define SWITCH_GPIO_Port GPIOB
#define T_SWDIO_Pin GPIO_PIN_13
#define T_SWDIO_GPIO_Port GPIOA
#define T_SWCLK_Pin GPIO_PIN_14
#define T_SWCLK_GPIO_Port GPIOA
#define T_SWO_Pin GPIO_PIN_3
#define T_SWO_GPIO_Port GPIOB
#define DEBUG_Pin GPIO_PIN_6
#define DEBUG_GPIO_Port GPIOB
#define BOOT_Pin GPIO_PIN_8
#define BOOT_GPIO_Port GPIOB

#define BLK 0x138800ed  // VINP3
#define WHI 0x138800e1  // VINP0
#define TRS 0x138800e9  // VINP2

#define BOOTLOADER_DFU_MAGIC    0x8FA3D62C

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
