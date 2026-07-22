/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include <stdio.h>
#include <string.h>
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx.h"

#include "main.h"
#include "sys_timer.h"
#include "msp.h"
#include "mspvtx.h"
#include "led.h"
#include "char_canvas.h"
#include "font.h"
#include "cli.h"
#include "log.h"
#include "rtc6705.h"
#include "vtx.h"
#include "temp.h"
#include "uart_dma.h"
#include "videosignal_gen.h"
#include "flash.h"
#include "setting.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TICKS_TO_HZ(ticks) ((170000000) / (ticks))
#define DEBUG_DAC2 0
#define DEBUG_VIDEO_STANDARD 0

#define SYNC_COUNT_5MS          148     // 5ms/32us = 156 count, 156count *0.95 = 148count
#define SYNC_LOST_CONSEC_MAX    10
#define SIGNAL_GEN_COUNT    1000/5  // signal lost detect time : 1sec

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

COMP_HandleTypeDef hcomp2;

DAC_HandleTypeDef hdac1;
DAC_HandleTypeDef hdac3;
DMA_HandleTypeDef hdma_dac3_ch1;

OPAMP_HandleTypeDef hopamp1;
OPAMP_HandleTypeDef hopamp2;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim16;
DMA_HandleTypeDef hdma_tim1_up;
DMA_HandleTypeDef hdma_tim8_up;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_DAC1_Init(void);
static void MX_DAC3_Init(void);
static void MX_OPAMP1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_COMP2_Init(void);
static void MX_SPI2_Init(void);
static void MX_ADC2_Init(void);
static void MX_TIM3_Init(void);
static void MX_OPAMP2_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM16_Init(void);
/* USER CODE BEGIN PFP */


#define VIDEO_FORMAT_STR    ((setting()->videoFormat == VIDEO_PAL) ? "PAL" : "NTSC")

typedef enum {
    FRAME_UNKNOWN=0,
    FRAME_EVEN,
    FRAME_ODD,
}FRAME_ODD_EVEN;


typedef enum {
    STATE_LOST = 0,
    STATE_TUNE_PULSE_LEVEL_LOW,
    STATE_TUNE_PULSE_LEVEL_HIGH,
    STATE_VSYNC_WAIT,
    STATE_SYNC,
    STATE_DISABLE,
}STATE;
STATE state = STATE_LOST;

char *state_str[] = {
    "STATE_LOST",
    "STATE_TUNE_PULSE_LEVEL_LOW",
    "STATE_TUNE_PULSE_LEVEL_HIGH",
    "STATE_VSYNC_WAIT",
    "STATE_SYNC",
    "STATE_DISABLE",
};

int32_t canvas_v_offset[]  ={CANVAS_V_OFFSET_NTSC, CANVAS_V_OFFSET_PAL};
int32_t canvas_v[]         ={CANVAS_V_NTSC, CANVAS_V_PAL};

static uint16_t pluse_level_low = 1000;
static uint16_t pluse_level_high = 0;
static uint16_t state_lost_count = 0;

static bool osd_ebable = true;


__attribute__((section (".sram2")))
uint32_t font2selTable[256][4] = {
    {BLK, BLK, BLK, BLK}, {BLK, BLK, BLK, TRS}, {BLK, BLK, BLK, WHI}, {BLK, BLK, BLK, TRS}, {BLK, BLK, TRS, BLK}, {BLK, BLK, TRS, TRS}, {BLK, BLK, TRS, WHI}, {BLK, BLK, TRS, TRS}, // 0x00
    {BLK, BLK, WHI, BLK}, {BLK, BLK, WHI, TRS}, {BLK, BLK, WHI, WHI}, {BLK, BLK, WHI, TRS}, {BLK, BLK, TRS, BLK}, {BLK, BLK, TRS, TRS}, {BLK, BLK, TRS, WHI}, {BLK, BLK, TRS, TRS}, // 0x08
    {BLK, TRS, BLK, BLK}, {BLK, TRS, BLK, TRS}, {BLK, TRS, BLK, WHI}, {BLK, TRS, BLK, TRS}, {BLK, TRS, TRS, BLK}, {BLK, TRS, TRS, TRS}, {BLK, TRS, TRS, WHI}, {BLK, TRS, TRS, TRS}, // 0x10
    {BLK, TRS, WHI, BLK}, {BLK, TRS, WHI, TRS}, {BLK, TRS, WHI, WHI}, {BLK, TRS, WHI, TRS}, {BLK, TRS, TRS, BLK}, {BLK, TRS, TRS, TRS}, {BLK, TRS, TRS, WHI}, {BLK, TRS, TRS, TRS}, // 0x18
    {BLK, WHI, BLK, BLK}, {BLK, WHI, BLK, TRS}, {BLK, WHI, BLK, WHI}, {BLK, WHI, BLK, TRS}, {BLK, WHI, TRS, BLK}, {BLK, WHI, TRS, TRS}, {BLK, WHI, TRS, WHI}, {BLK, WHI, TRS, TRS}, // 0x20
    {BLK, WHI, WHI, BLK}, {BLK, WHI, WHI, TRS}, {BLK, WHI, WHI, WHI}, {BLK, WHI, WHI, TRS}, {BLK, WHI, TRS, BLK}, {BLK, WHI, TRS, TRS}, {BLK, WHI, TRS, WHI}, {BLK, WHI, TRS, TRS}, // 0x28
    {BLK, TRS, BLK, BLK}, {BLK, TRS, BLK, TRS}, {BLK, TRS, BLK, WHI}, {BLK, TRS, BLK, TRS}, {BLK, TRS, TRS, BLK}, {BLK, TRS, TRS, TRS}, {BLK, TRS, TRS, WHI}, {BLK, TRS, TRS, TRS}, // 0x30
    {BLK, TRS, WHI, BLK}, {BLK, TRS, WHI, TRS}, {BLK, TRS, WHI, WHI}, {BLK, TRS, WHI, TRS}, {BLK, TRS, TRS, BLK}, {BLK, TRS, TRS, TRS}, {BLK, TRS, TRS, WHI}, {BLK, TRS, TRS, TRS}, // 0x38
    {TRS, BLK, BLK, BLK}, {TRS, BLK, BLK, TRS}, {TRS, BLK, BLK, WHI}, {TRS, BLK, BLK, TRS}, {TRS, BLK, TRS, BLK}, {TRS, BLK, TRS, TRS}, {TRS, BLK, TRS, WHI}, {TRS, BLK, TRS, TRS}, // 0x40
    {TRS, BLK, WHI, BLK}, {TRS, BLK, WHI, TRS}, {TRS, BLK, WHI, WHI}, {TRS, BLK, WHI, TRS}, {TRS, BLK, TRS, BLK}, {TRS, BLK, TRS, TRS}, {TRS, BLK, TRS, WHI}, {TRS, BLK, TRS, TRS}, // 0x48
    {TRS, TRS, BLK, BLK}, {TRS, TRS, BLK, TRS}, {TRS, TRS, BLK, WHI}, {TRS, TRS, BLK, TRS}, {TRS, TRS, TRS, BLK}, {TRS, TRS, TRS, TRS}, {TRS, TRS, TRS, WHI}, {TRS, TRS, TRS, TRS}, // 0x50
    {TRS, TRS, WHI, BLK}, {TRS, TRS, WHI, TRS}, {TRS, TRS, WHI, WHI}, {TRS, TRS, WHI, TRS}, {TRS, TRS, TRS, BLK}, {TRS, TRS, TRS, TRS}, {TRS, TRS, TRS, WHI}, {TRS, TRS, TRS, TRS}, // 0x58
    {TRS, WHI, BLK, BLK}, {TRS, WHI, BLK, TRS}, {TRS, WHI, BLK, WHI}, {TRS, WHI, BLK, TRS}, {TRS, WHI, TRS, BLK}, {TRS, WHI, TRS, TRS}, {TRS, WHI, TRS, WHI}, {TRS, WHI, TRS, TRS}, // 0x60
    {TRS, WHI, WHI, BLK}, {TRS, WHI, WHI, TRS}, {TRS, WHI, WHI, WHI}, {TRS, WHI, WHI, TRS}, {TRS, WHI, TRS, BLK}, {TRS, WHI, TRS, TRS}, {TRS, WHI, TRS, WHI}, {TRS, WHI, TRS, TRS}, // 0x68
    {TRS, TRS, BLK, BLK}, {TRS, TRS, BLK, TRS}, {TRS, TRS, BLK, WHI}, {TRS, TRS, BLK, TRS}, {TRS, TRS, TRS, BLK}, {TRS, TRS, TRS, TRS}, {TRS, TRS, TRS, WHI}, {TRS, TRS, TRS, TRS}, // 0x70
    {TRS, TRS, WHI, BLK}, {TRS, TRS, WHI, TRS}, {TRS, TRS, WHI, WHI}, {TRS, TRS, WHI, TRS}, {TRS, TRS, TRS, BLK}, {TRS, TRS, TRS, TRS}, {TRS, TRS, TRS, WHI}, {TRS, TRS, TRS, TRS}, // 0x78
    {WHI, BLK, BLK, BLK}, {WHI, BLK, BLK, TRS}, {WHI, BLK, BLK, WHI}, {WHI, BLK, BLK, TRS}, {WHI, BLK, TRS, BLK}, {WHI, BLK, TRS, TRS}, {WHI, BLK, TRS, WHI}, {WHI, BLK, TRS, TRS}, // 0x80
    {WHI, BLK, WHI, BLK}, {WHI, BLK, WHI, TRS}, {WHI, BLK, WHI, WHI}, {WHI, BLK, WHI, TRS}, {WHI, BLK, TRS, BLK}, {WHI, BLK, TRS, TRS}, {WHI, BLK, TRS, WHI}, {WHI, BLK, TRS, TRS}, // 0x88
    {WHI, TRS, BLK, BLK}, {WHI, TRS, BLK, TRS}, {WHI, TRS, BLK, WHI}, {WHI, TRS, BLK, TRS}, {WHI, TRS, TRS, BLK}, {WHI, TRS, TRS, TRS}, {WHI, TRS, TRS, WHI}, {WHI, TRS, TRS, TRS}, // 0x90
    {WHI, TRS, WHI, BLK}, {WHI, TRS, WHI, TRS}, {WHI, TRS, WHI, WHI}, {WHI, TRS, WHI, TRS}, {WHI, TRS, TRS, BLK}, {WHI, TRS, TRS, TRS}, {WHI, TRS, TRS, WHI}, {WHI, TRS, TRS, TRS}, // 0x98
    {WHI, WHI, BLK, BLK}, {WHI, WHI, BLK, TRS}, {WHI, WHI, BLK, WHI}, {WHI, WHI, BLK, TRS}, {WHI, WHI, TRS, BLK}, {WHI, WHI, TRS, TRS}, {WHI, WHI, TRS, WHI}, {WHI, WHI, TRS, TRS}, // 0xa0
    {WHI, WHI, WHI, BLK}, {WHI, WHI, WHI, TRS}, {WHI, WHI, WHI, WHI}, {WHI, WHI, WHI, TRS}, {WHI, WHI, TRS, BLK}, {WHI, WHI, TRS, TRS}, {WHI, WHI, TRS, WHI}, {WHI, WHI, TRS, TRS}, // 0xa8
    {WHI, TRS, BLK, BLK}, {WHI, TRS, BLK, TRS}, {WHI, TRS, BLK, WHI}, {WHI, TRS, BLK, TRS}, {WHI, TRS, TRS, BLK}, {WHI, TRS, TRS, TRS}, {WHI, TRS, TRS, WHI}, {WHI, TRS, TRS, TRS}, // 0xb0
    {WHI, TRS, WHI, BLK}, {WHI, TRS, WHI, TRS}, {WHI, TRS, WHI, WHI}, {WHI, TRS, WHI, TRS}, {WHI, TRS, TRS, BLK}, {WHI, TRS, TRS, TRS}, {WHI, TRS, TRS, WHI}, {WHI, TRS, TRS, TRS}, // 0xb8
    {TRS, BLK, BLK, BLK}, {TRS, BLK, BLK, TRS}, {TRS, BLK, BLK, WHI}, {TRS, BLK, BLK, TRS}, {TRS, BLK, TRS, BLK}, {TRS, BLK, TRS, TRS}, {TRS, BLK, TRS, WHI}, {TRS, BLK, TRS, TRS}, // 0xc0
    {TRS, BLK, WHI, BLK}, {TRS, BLK, WHI, TRS}, {TRS, BLK, WHI, WHI}, {TRS, BLK, WHI, TRS}, {TRS, BLK, TRS, BLK}, {TRS, BLK, TRS, TRS}, {TRS, BLK, TRS, WHI}, {TRS, BLK, TRS, TRS}, // 0xc8
    {TRS, TRS, BLK, BLK}, {TRS, TRS, BLK, TRS}, {TRS, TRS, BLK, WHI}, {TRS, TRS, BLK, TRS}, {TRS, TRS, TRS, BLK}, {TRS, TRS, TRS, TRS}, {TRS, TRS, TRS, WHI}, {TRS, TRS, TRS, TRS}, // 0xd0
    {TRS, TRS, WHI, BLK}, {TRS, TRS, WHI, TRS}, {TRS, TRS, WHI, WHI}, {TRS, TRS, WHI, TRS}, {TRS, TRS, TRS, BLK}, {TRS, TRS, TRS, TRS}, {TRS, TRS, TRS, WHI}, {TRS, TRS, TRS, TRS}, // 0xd8
    {TRS, WHI, BLK, BLK}, {TRS, WHI, BLK, TRS}, {TRS, WHI, BLK, WHI}, {TRS, WHI, BLK, TRS}, {TRS, WHI, TRS, BLK}, {TRS, WHI, TRS, TRS}, {TRS, WHI, TRS, WHI}, {TRS, WHI, TRS, TRS}, // 0xe0
    {TRS, WHI, WHI, BLK}, {TRS, WHI, WHI, TRS}, {TRS, WHI, WHI, WHI}, {TRS, WHI, WHI, TRS}, {TRS, WHI, TRS, BLK}, {TRS, WHI, TRS, TRS}, {TRS, WHI, TRS, WHI}, {TRS, WHI, TRS, TRS}, // 0xe8
    {TRS, TRS, BLK, BLK}, {TRS, TRS, BLK, TRS}, {TRS, TRS, BLK, WHI}, {TRS, TRS, BLK, TRS}, {TRS, TRS, TRS, BLK}, {TRS, TRS, TRS, TRS}, {TRS, TRS, TRS, WHI}, {TRS, TRS, TRS, TRS}, // 0xf0
    {TRS, TRS, WHI, BLK}, {TRS, TRS, WHI, TRS}, {TRS, TRS, WHI, WHI}, {TRS, TRS, WHI, TRS}, {TRS, TRS, TRS, BLK}, {TRS, TRS, TRS, TRS}, {TRS, TRS, TRS, WHI}, {TRS, TRS, TRS, TRS}  // 0xf8
};




//uint32_t dataBuffer[2][512] __attribute__((aligned(4))) = {0};
uint32_t dataBuffer[2][CANVAS_H+16] __attribute__((aligned(4))) = {0};

void osd_dma(DETECT_SYNC detect_sync);
void sync_detect(DETECT_SYNC detect_sync);
/* USER CODE END PFP */


/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void rebootDfu(void)
{
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    LL_PWR_EnableBkUpAccess();
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_RTCAPB);
    LL_RCC_EnableRTC();
    TAMP->BKP0R = BOOTLOADER_DFU_MAGIC;
    __disable_irq(); 
    NVIC_SystemReset();
}

int __io_putchar(uint8_t ch) {
    return ITM_SendChar(ch);
}

void osd_enable(uint8_t en)
{
    if (en){
        state = STATE_LOST;
        state_lost_count = 0;
    }else{
        state = STATE_DISABLE;
        videosignal_gen_stop();
    }
}

__attribute__((section (".ccmram_code"), optimize("O2")))
void SetLine(register volatile uint32_t *data, register volatile uint16_t *buf, int line)
{
    register volatile uint32_t *fp;
    int linex = line*3;
    for(int ch=0; ch<COLUMN_SIZE; ch++){
        register volatile  uint8_t *fontp = (uint8_t*)&font[*buf++][linex];
        fp = font2selTable[ *fontp ];
        *data++ = *fp++;
        *data++ = *fp++;
        *data++ = *fp++;
        *data++ = *fp++;
        fp = font2selTable[ *(fontp+1) ];
        *data++ = *fp++;
        *data++ = *fp++;
        *data++ = *fp++;
        *data++ = *fp++;
        fp = font2selTable[ *(fontp+2) ];
        *data++ = *fp++;
        *data++ = *fp++;
        *data++ = *fp++;
        *data++ = *fp++;
    }
    *data++ = TRS;      // end
    return;
}

__attribute__((section (".ccmram_code"), optimize("O2")))
void intHsyncFallEdge(void)
{
//    HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_SET);

    if (state != STATE_DISABLE){
        volatile uint32_t uwIC1Value = TIM2->CCR1;      // down to down edge
        volatile uint32_t uwIC2Value = TIM2->CCR2;      // down to up edge;

        DETECT_SYNC detect_sync = DETECT_UNKNOWN;

        #define HSYNC_PERIOD_MIN (572*17)   // 63.5*0.9  ntsc:63.5
        #define HSYNC_PERIOD_MAX (704*17)   // 64.0*1.1  pal:64.0
        #define HSYNC_PULSE_MIN (44*17)
        #define HSYNC_PULSE_MAX (50*17)
        #define VSYNC_PULSE_MIN (244*17)
        #define VSYNC_PULSE_MAX (298*17)
        #define EQUIVALENT_PULSE_MIN    (20*17)
        #define EQUIVALENT_PULSE_MAX    (26*17)

        if (    uwIC1Value > HSYNC_PERIOD_MIN && uwIC1Value < HSYNC_PERIOD_MAX &&
                uwIC2Value > HSYNC_PULSE_MIN && uwIC2Value < HSYNC_PULSE_MAX ){
                    detect_sync = DETECT_HSYNC;
        } else if ( uwIC1Value > HSYNC_PERIOD_MIN/2 && uwIC1Value < HSYNC_PERIOD_MAX/2 &&
                    uwIC2Value > EQUIVALENT_PULSE_MIN && uwIC2Value < EQUIVALENT_PULSE_MAX ){
                        detect_sync = DETECT_EQUIVALENT;
        }else if (  uwIC1Value > HSYNC_PERIOD_MIN/2 && uwIC1Value < HSYNC_PERIOD_MAX/2 &&
                    uwIC2Value > VSYNC_PULSE_MIN && uwIC2Value < VSYNC_PULSE_MAX ){
                        detect_sync = DETECT_VSYNC;
        }

        osd_dma(detect_sync);
        sync_detect(detect_sync);
    }

//    HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_RESET);
}

uint32_t sync_count = 0;                // for detect sync lost
uint16_t sync_lost_consec = 0;
int32_t equivalent_pulse_count = 0;     // ntsc:odd=6,odd=5
int32_t video_line = 0;
int32_t video_line_last = 0;
FRAME_ODD_EVEN  frame_odd_even = FRAME_UNKNOWN;
__attribute__((section (".ccmram_code"), optimize("O2")))
void sync_detect(DETECT_SYNC detect_sync)
{
    switch(detect_sync){
        case DETECT_HSYNC:
            if (frame_odd_even == FRAME_UNKNOWN){
                if (equivalent_pulse_count & 0x1){
                    if (setting()->videoFormat == VIDEO_NTSC){
                        frame_odd_even = FRAME_ODD;
                        video_line = 1;
                    }else{
                        frame_odd_even = FRAME_EVEN;
                        video_line = 0;
                    }
                }else{
                    if (setting()->videoFormat == VIDEO_NTSC){
                        frame_odd_even = FRAME_EVEN;
                        video_line = 0;
                    }else{
                        frame_odd_even = FRAME_ODD;
                        video_line = 1;
                    }
                }
            }else{
                video_line += 2;
            }
            sync_count += 2;
            break;
        case DETECT_VSYNC:
            sync_count++;
            switch(state){
                case STATE_VSYNC_WAIT:
                    state = STATE_SYNC;
                    break;
                case STATE_SYNC:
                    if (video_line > 0){
                        video_line_last = video_line;
                    }
                    video_line = 0;
                    break;
                default:
                    break;
            }
            equivalent_pulse_count = 0;
            frame_odd_even = FRAME_UNKNOWN;
            break;
        case DETECT_EQUIVALENT:
            equivalent_pulse_count++;
            sync_count++;
            break;
        case DETECT_UNKNOWN:
            break;
    }
}

__attribute__((section (".ccmram_code"), optimize("O2")))
void osd_dma(DETECT_SYNC detect_sync)
{
// static uint32_t vcanvas_count = 0;

    if (detect_sync != DETECT_HSYNC){
        //vcanvas_count = 0;
        return;
    }
    if (!osd_ebable){
        return;
    }
    if (state != STATE_SYNC){
        return;
    }
    if ( video_line == 0 ){
        charCanvasNext();
    }

    int32_t canvas_line = video_line - canvas_v_offset[setting()->videoFormat];

    if ( canvas_line >= 0 && canvas_line < canvas_v[setting()->videoFormat] ){

        // 1. Stop TIM1
        TIM1->CR1 &= ~TIM_CR1_CEN;

        // 2. Disable DMA channel
        DMA1_Channel1->CCR &= ~DMA_CCR_EN;

        // 3. Clear TIM1 update flag
        TIM1->SR &= ~TIM_SR_UIF;

        // 4. Disable TIM1 DMA
        TIM1->DIER &= ~TIM_DIER_UDE;

        // 5. Reset TIM1 counter
        TIM1->CNT = 0;

        // 6. Configure DMA transfer: Mem-to-Periph, High priority, no increment on peripheral, increment memory, 32-bit
        DMA1_Channel1->CCR = DMA_CCR_DIR       // Memory to peripheral
                           | DMA_CCR_PL_1      // High priority
                           | DMA_CCR_MINC      // Memory increment
                           | DMA_CCR_MSIZE_1   // Memory size: word
                           | DMA_CCR_PSIZE_1;  // Peripheral size: word

        // 7. Set DMA addresses: source = dataBuffer, destination = OPAMP1->CSR
        DMA1_Channel1->CPAR = (uint32_t)&OPAMP1->CSR;
        DMA1_Channel1->CMAR = (uint32_t)&dataBuffer[(canvas_line>>1) & 0x1];
        // 8. Set number of words to transfer
        DMA1_Channel1->CNDTR = CANVAS_H + 1;

        // 9. Enable DMA channel
        DMA1_Channel1->CCR |= DMA_CCR_EN;

        // 10. Enable DMA request on TIM1 update event
        TIM1->DIER |= TIM_DIER_UDE;

        // 11. Start TIM1
        TIM1->CR1 |= TIM_CR1_CEN;

        // 12. Advance canvas line counter
        //vcanvas_count++;
    }

    int next_line = canvas_line + 2;

    if ( next_line >= 0 && next_line < canvas_v[setting()->videoFormat] ){
        #ifdef RESOLUTION_HD
            SetLine(&dataBuffer[(next_line>>1) & 0x1][CANVAS_H_OFFSET], charCanvasGet(next_line/18), next_line % 18);
        #else
            SetLine(&dataBuffer[(next_line>>1) & 0x1][CANVAS_H_OFFSET], charCanvasGet((next_line>>1)/18), (next_line>>1) % 18);
        #endif
    }

}



void sync_proc(void)
{
 static uint32_t state_time = 0;

    uint32_t now = HAL_GetTick();
    if(now - state_time < 5){
        return;
    }
    state_time += 5;

    switch(state){
        case STATE_LOST:
            if (sync_count >= SYNC_COUNT_5MS){
                // sync detect
                videosignal_gen_stop();
                pluse_level_high = pluse_level_low +50;
                pluse_level_low -= 50;
                HAL_DAC_SetValue(&hdac3, DAC_CHANNEL_2, DAC_ALIGN_12B_R, (0xfff*pluse_level_low)/3300);
                state = STATE_TUNE_PULSE_LEVEL_LOW;
                DEBUG_PRINTF("state:STATE_TUNE_PULSE_LEVEL_LOW");
            }else{
                state_lost_count = (state_lost_count < SIGNAL_GEN_COUNT) ? state_lost_count+1 : state_lost_count;
                if (state_lost_count == SIGNAL_GEN_COUNT){
                    state_lost_count = 0xffff;
                    videosignal_gen_start();
                }
                pluse_level_low = (pluse_level_low + 250) % (2500);
                pluse_level_low = (pluse_level_low == 0) ? 250 : pluse_level_low;
                HAL_DAC_SetValue(&hdac3, DAC_CHANNEL_2, DAC_ALIGN_12B_R, (0xfff*pluse_level_low)/3300);
            }
            break;
        case STATE_TUNE_PULSE_LEVEL_LOW:
            if (sync_count < SYNC_COUNT_5MS || pluse_level_low == 0){    // min 0mv
                state = STATE_TUNE_PULSE_LEVEL_HIGH;
                DEBUG_PRINTF("state:STATE_TUNE_PULSE_LEVEL_HIGH");
                HAL_DAC_SetValue(&hdac3, DAC_CHANNEL_2, DAC_ALIGN_12B_R, (0xfff*pluse_level_high)/3300);
            }else{
                pluse_level_low -= 50;
                HAL_DAC_SetValue(&hdac3, DAC_CHANNEL_2, DAC_ALIGN_12B_R, (0xfff*pluse_level_low)/3300);
            }
            break;
        case STATE_TUNE_PULSE_LEVEL_HIGH:
            if (sync_count < SYNC_COUNT_5MS || pluse_level_high == 2500){    // max2500mv
                state = STATE_VSYNC_WAIT;
                DEBUG_PRINTF("state:STATE_VSYNC_WAIT)");
                HAL_DAC_SetValue(&hdac3, DAC_CHANNEL_2, DAC_ALIGN_12B_R, (0xfff * ((pluse_level_high + pluse_level_low)/2))/3300);
                HAL_DAC_SetValue(&hdac3, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (0xfff * ((pluse_level_high/4) + 100)/3300));    // black level -12dB=1/4 +50mv       todo: auto level tune
            }else{
                pluse_level_high += 50;
                HAL_DAC_SetValue(&hdac3, DAC_CHANNEL_2, DAC_ALIGN_12B_R, (0xfff*pluse_level_high)/3300);
            }
            break;
        case STATE_VSYNC_WAIT:
        case STATE_SYNC:
            if (sync_count < SYNC_COUNT_5MS){
                if ( ++sync_lost_consec > SYNC_LOST_CONSEC_MAX){
                    DEBUG_PRINTF("state:STATE_LOST)");
                    state = STATE_LOST;
                    pluse_level_low = 1000;
                    state_lost_count = 0;
                }
            }else{
                sync_lost_consec = 0;
            }
            if (video_line_last != 0){;
                if (video_line > 500 && video_line_last < 510) {        // NTSC (262-10)*2= 504
                    setting()->videoFormat = VIDEO_NTSC;     // ntsc
                } else if (video_line > 600 && video_line < 620) {
                    setting()->videoFormat = VIDEO_PAL;     // pal
                }
            }
            break;
        case STATE_DISABLE:
            HAL_DAC_SetValue(&hdac3, DAC_CHANNEL_2, DAC_ALIGN_12B_R, (0xfff*pluse_level_high)/3300);
            break;
    }
    sync_count = 0;
}



__attribute__((section (".ccmram_code"), optimize("O2")))
bool writeFlashFont(uint16_t addr, uint8_t *data)
{
    enableOSD(false);

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
    if ( (addr & 0x1f) == 0 ){
        // erase
        uint32_t erase_addr = (uint32_t)&font[addr][0];
        DEBUG_PRINTF("erase start 0x%x", erase_addr);
        static FLASH_EraseInitTypeDef EraseInitStruct;
        EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
        EraseInitStruct.Page        = GET_FLASH_PAGE(erase_addr);
        EraseInitStruct.NbPages     = 1;
        uint32_t PageError = 0;
        if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK){
            ;
        }
        DEBUG_PRINTF("erase end");
    }
    // write
    for(int x=0; x<FONT_SIZE; x+=8){
        uint64_t wdata = *data++;
        wdata += (uint64_t)(*data++)<<8;
        wdata += (uint64_t)(*data++)<<16;
        wdata += (uint64_t)(*data++)<<24;
        wdata += (uint64_t)(*data++)<<32;
        wdata += (uint64_t)(*data++)<<40;
        wdata += (uint64_t)(*data++)<<48;
        wdata += (uint64_t)(*data++)<<56;

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)&font[addr][x], wdata) == HAL_OK){
        }
    }
    HAL_FLASH_Lock();

    return true;
}
void enableOSD(bool en)
{
    osd_ebable = en;
}




/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */ 
    DEBUG_INIT();
    DEBUG_PRINTF("Start");
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
  MX_DAC1_Init();
  MX_DAC3_Init();
  MX_OPAMP1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_COMP2_Init();
  MX_SPI2_Init();
  MX_ADC2_Init();
  MX_TIM3_Init();
  MX_OPAMP2_Init();
  MX_ADC1_Init();
  MX_TIM4_Init();
  MX_TIM16_Init();
  /* USER CODE BEGIN 2 */

    HAL_TIM_Base_MspInit(&htim16);
    initSysTimer();

    for (int x=0; x<CANVAS_H_OFFSET; x++){
        dataBuffer[0][x] = TRS;
        dataBuffer[1][x] = TRS;
    }
    charCanvasInit();


    HAL_COMP_MspInit(&hcomp2);
    HAL_COMP_Start(&hcomp2);

    HAL_OPAMP_MspInit(&hopamp1);
    HAL_OPAMP_Start(&hopamp1);
    OPAMP1->CSR = TRS;

    HAL_OPAMP_MspInit(&hopamp2);
    HAL_OPAMP_Start(&hopamp2);

    HAL_DAC_Init(&hdac1);
    HAL_DAC_MspInit(&hdac1);
    HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (0xfff*1500)/3300);    // WHITE:1.5v (Foxeer OSD is 1.5V)

    HAL_DAC_Init(&hdac3);
    HAL_DAC_MspInit(&hdac3);
    HAL_DAC_Start(&hdac3, DAC_CHANNEL_1);
    HAL_DAC_SetValue(&hdac3, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (0xfff*286)/3300);    // 286mVv

    HAL_DAC_MspInit(&hdac3);
    HAL_DAC_Start(&hdac3, DAC_CHANNEL_2);
    HAL_TIM_Base_MspInit(&htim1);

    HAL_TIM_Base_MspInit(&htim2);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableIT_CC1(TIM2);
    LL_TIM_EnableCounter(TIM2);

    setting_init();
    uart_init();
    initLed();
    msp_init();
#ifndef TARGET_NOVTX
    initVtx();
    mspVtx_init();
    initTemp();
#endif

#ifdef DEV_MODE
    startCli();
#endif
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    uint8_t rxdata;
#ifdef  DEV_MODE
    uart_poll();
    if (!uart_read_byte(&rxdata)) {
        dataCli(rxdata);
    }
    static uint32_t diag_time = 100;
    if (diag_time < HAL_GetTick()){
        char buf[35];
        diag_time += 100;
        charCanvasClear();

        sprintf(buf,"%s", "---VIDEO---");
        charCanvasWrite(1,0,  0, (uint8_t*)buf, strlen(buf));
        sprintf(buf,"LINE........%d (%s)", (int)video_line_last, VIDEO_FORMAT_STR);
        charCanvasWrite(2,1,  0, (uint8_t*)buf, strlen(buf));
        sprintf(buf,"SYNC-TH.....%04dMV (%d-%d)", (pluse_level_high+pluse_level_low)/2, pluse_level_low, pluse_level_high);
        charCanvasWrite(3,1,  0, (uint8_t*)buf, strlen(buf));
        sprintf(buf,"%s", "---VTX---");
        charCanvasWrite(4,0,  0, (uint8_t*)buf, strlen(buf));
        sprintf(buf,"FREQ........%04dMHZ", getVtxFreq());
        charCanvasWrite(5,1,  0, (uint8_t*)buf, strlen(buf));
        sprintf(buf,"VPD.........%04dMV", getVpd());
        charCanvasWrite(6,1,  0, (uint8_t*)buf, strlen(buf));
        sprintf(buf,"VPD-TARGET..%04dMV", getVpdTarget());
        charCanvasWrite(7,1,  0, (uint8_t*)buf, strlen(buf));
        sprintf(buf,"VREF........%04dMV", getVref());
        charCanvasWrite(8,1,  0, (uint8_t*)buf, strlen(buf));
        sprintf(buf,"TEMP.........%03ld\xe", getTemp());
        charCanvasWrite(9,1,  0, (uint8_t*)buf, strlen(buf));
        charCanvasDraw();
    }
#else
#ifndef TARGET_NOVTX
    mspUpdate();
#endif
    uart_poll();
    
    int cnt=32;
    while(cnt--){
        if (uart_read_byte(&rxdata)){
            break;
        }
        msp_parse_char(rxdata);
    }
#endif

#if 1
    static LED_STATE led_state = LED_OFF;
    static uint32_t previous_time = 0;

    procSysTimer();
    sync_proc();
#ifndef TARGET_NOVTX
    procTemp();
    procVtx();
#endif

    uint32_t now = HAL_GetTick();
    if (now - previous_time > 1000){
        previous_time = now;
        if (led_state == LED_OFF){
            led_state = LED_GREEN128;
        }else{
            led_state = LED_OFF;
        }
        setting_update();
        setLed(led_state);
        DEBUG_PRINTF("led_state=%d",led_state);

        DEBUG_PRINTF("%s : %s(%d)", state_str[state], VIDEO_FORMAT_STR, video_line_last);
        DEBUG_PRINTF("pluse_level %dmv(%d-%d)",(pluse_level_high+pluse_level_low)/2, pluse_level_high, pluse_level_low);
#ifndef TARGET_NOVTX
        debuglogVtx("main");
#endif
//        DEBUG_PRINTF("IC1=%d IC2=%d", uwIC1Value, uwIC2Value);

    }


#endif
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = ENABLE;
  hadc1.Init.Oversampling.Ratio = ADC_OVERSAMPLING_RATIO_256;
  hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_4;
  hadc1.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
  hadc1.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR_ADC1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.GainCompensation = 0;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc2.Init.OversamplingMode = ENABLE;
  hadc2.Init.Oversampling.Ratio = ADC_OVERSAMPLING_RATIO_256;
  hadc2.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_4;
  hadc2.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
  hadc2.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;        /* 1/60MHz * 640.5 * 256(oversmple) = 2.7ms */
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief COMP2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_COMP2_Init(void)
{

  /* USER CODE BEGIN COMP2_Init 0 */

  /* USER CODE END COMP2_Init 0 */

  /* USER CODE BEGIN COMP2_Init 1 */

  /* USER CODE END COMP2_Init 1 */
  hcomp2.Instance = COMP2;
  hcomp2.Init.InputPlus = COMP_INPUT_PLUS_IO2;
  hcomp2.Init.InputMinus = COMP_INPUT_MINUS_DAC3_CH2;
  hcomp2.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
  hcomp2.Init.Hysteresis = COMP_HYSTERESIS_70MV;
  hcomp2.Init.BlankingSrce = COMP_BLANKINGSRC_NONE;
  hcomp2.Init.TriggerMode = COMP_TRIGGERMODE_EVENT_RISING;
  if (HAL_COMP_Init(&hcomp2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN COMP2_Init 2 */

  /* USER CODE END COMP2_Init 2 */

}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */

  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_AUTOMATIC;
  sConfig.DAC_DMADoubleDataMode = DISABLE;
  sConfig.DAC_SignedFormat = DISABLE;
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_Trigger2 = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_EXTERNAL;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT2 config
  */
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief DAC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC3_Init(void)
{

  /* USER CODE BEGIN DAC3_Init 0 */

  /* USER CODE END DAC3_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC3_Init 1 */

  /* USER CODE END DAC3_Init 1 */

  /** DAC Initialization
  */
  hdac3.Instance = DAC3;
  if (HAL_DAC_Init(&hdac3) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_AUTOMATIC;
  sConfig.DAC_DMADoubleDataMode = DISABLE;
  sConfig.DAC_SignedFormat = DISABLE;
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_Trigger2 = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_INTERNAL;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac3, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT2 config
  */
  if (HAL_DAC_ConfigChannel(&hdac3, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC3_Init 2 */

  /* USER CODE END DAC3_Init 2 */

}


/**
  * @brief OPAMP1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_OPAMP1_Init(void)
{

  /* USER CODE BEGIN OPAMP1_Init 0 */

  /* USER CODE END OPAMP1_Init 0 */

  /* USER CODE BEGIN OPAMP1_Init 1 */

  /* USER CODE END OPAMP1_Init 1 */
  hopamp1.Instance = OPAMP1;
  hopamp1.Init.PowerMode = OPAMP_POWERMODE_HIGHSPEED;
  hopamp1.Init.Mode = OPAMP_FOLLOWER_MODE;
  hopamp1.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_IO2;
  hopamp1.Init.InternalOutput = DISABLE;
  hopamp1.Init.TimerControlledMuxmode = OPAMP_TIMERCONTROLLEDMUXMODE_DISABLE;
  hopamp1.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
  if (HAL_OPAMP_Init(&hopamp1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN OPAMP1_Init 2 */

  /* USER CODE END OPAMP1_Init 2 */

}

/**
  * @brief OPAMP2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_OPAMP2_Init(void)
{

  /* USER CODE BEGIN OPAMP2_Init 0 */

  /* USER CODE END OPAMP2_Init 0 */

  /* USER CODE BEGIN OPAMP2_Init 1 */

  /* USER CODE END OPAMP2_Init 1 */
  hopamp2.Instance = OPAMP2;
  hopamp2.Init.PowerMode = OPAMP_POWERMODE_NORMALSPEED;
  hopamp2.Init.Mode = OPAMP_PGA_MODE;
  hopamp2.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_IO0;
  hopamp2.Init.InternalOutput = DISABLE;
  hopamp2.Init.TimerControlledMuxmode = OPAMP_TIMERCONTROLLEDMUXMODE_DISABLE;
  hopamp2.Init.PgaConnect = OPAMP_PGA_CONNECT_INVERTINGINPUT_NO;
  hopamp2.Init.PgaGain = OPAMP_PGA_GAIN_4_OR_MINUS_3;
  hopamp2.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
  if (HAL_OPAMP_Init(&hopamp2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN OPAMP2_Init 2 */

  /* USER CODE END OPAMP2_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_1LINE;
  hspi2.Init.DataSize = SPI_DATASIZE_5BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_LSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = PIX_PERIOD;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_GATED;
  sSlaveConfig.InputTrigger = TIM_TS_ETRF;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_NONINVERTED;
  sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
  sSlaveConfig.TriggerFilter = 0;
  if (HAL_TIM_SlaveConfigSynchro(&htim1, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIMEx_RemapConfig(&htim1, TIM_TIM1_ETR_COMP2);
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger = TIM_TS_ETRF;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_INVERTED;
  sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
  sSlaveConfig.TriggerFilter = 1;
  if (HAL_TIM_SlaveConfigSynchro(&htim2, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIMEx_RemapConfig(&htim2, TIM_TIM2_ETR_COMP2);
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIMEx_TISelection(&htim2, TIM_TIM2_TI1_COMP2, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIMEx_TISelection(&htim2, TIM_TIM2_TI2_COMP2, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = PIX_PERIOD;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC2REF;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 10;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}


/**
  * @brief TIM16 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 17000;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 65535;
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
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_EnableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA2_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel1_IRQn);
  /* DMA2_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel2_IRQn);
  /* DMA2_Channel3_IRQn interrupt configuration */
//  HAL_NVIC_SetPriority(DMA2_Channel3_IRQn, 0, 0);
//  HAL_NVIC_EnableIRQ(DMA2_Channel3_IRQn);
  /* DMA2_Channel4_IRQn interrupt configuration */
//  HAL_NVIC_SetPriority(DMA2_Channel4_IRQn, 0, 0);
//  HAL_NVIC_EnableIRQ(DMA2_Channel4_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : SPI_CS_Pin */
  GPIO_InitStruct.Pin = SPI_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SPI_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SWITCH_Pin BOOT_Pin */
  GPIO_InitStruct.Pin = SWITCH_Pin|BOOT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : DEBUG_Pin */
  GPIO_InitStruct.Pin = DEBUG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DEBUG_GPIO_Port, &GPIO_InitStruct);

  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
