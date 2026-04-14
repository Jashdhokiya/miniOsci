/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "config.h"
#include "osc_adc.h"
#include "osc_signal.h"
#include "osc_display.h"
#include "osc_ui.h"
#include "ili9341.h"
#include "func_gen.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint8_t holdDrawn = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_ADC1_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */

  /* Initialize display */
  Display_Init();

  /* Splash screen — landscape centered */
  ILI9341_FillScreen(COLOR_PANEL_BG);
  ILI9341_DrawString(64, 60, "MINI OSCILLOSCOPE", COLOR_GREEN, COLOR_PANEL_BG, 2);
  ILI9341_DrawString(96, 100, "Dual Channel + FuncGen", COLOR_CYAN, COLOR_PANEL_BG, 1);
  ILI9341_DrawString(88, 130, "STM32F411 BlackPill", COLOR_ORANGE, COLOR_PANEL_BG, 1);
  ILI9341_DrawString(100, 160, "v1.0  |  MiniOsci", COLOR_AXIS_LABEL, COLOR_PANEL_BG, 1);
  HAL_Delay(1500);
  Display_ClearScreen();

  /* Start function generator (1 kHz sine on PB8) */
  FuncGen_Init();
  FuncGen_Start();

  /* Start ADC + DMA + Timer sampling */
  OSC_ADC_Start();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /* === Oscilloscope Main Loop === */
    if (bufferReady && UI_GetState() == STATE_RUN)
    {
        bufferReady = 0;

        uint8_t ch        = UI_GetChannel();
        uint16_t* buf     = OSC_ADC_GetBuffer(ch);
        float voltDiv     = UI_GetVoltDiv();
        uint8_t timeDivIdx  = UI_GetTimeDivIndex();
        uint8_t voltDivIdx  = UI_GetVoltDivIndex();

        uint32_t sampleRate = 100000000UL / (TIME_DIV_ARR[timeDivIdx] + 1);
        float freq = Signal_GetFrequency(buf, SAMPLE_BUFFER_SIZE, sampleRate);
        float vpp  = Signal_GetVpp(buf, SAMPLE_BUFFER_SIZE);

        Display_ClearWaveform();
        Display_DrawGrid();
        Display_DrawAxisLabels(timeDivIdx, voltDivIdx);
        Display_DrawWaveform(buf, SAMPLE_BUFFER_SIZE, ch, voltDiv);
        Display_DrawMeasurements(freq, vpp, ch, timeDivIdx, voltDivIdx, 0);

        /* Stats overlay (toggle with PB4) */
        if (UI_GetStatsVisible()) {
            Display_DrawStats(buf, SAMPLE_BUFFER_SIZE, ch);
        }
    }
    else if (UI_GetState() == STATE_HOLD)
    {
        /* Redraw HOLD status once */
        if (!holdDrawn) {
            uint8_t ch = UI_GetChannel();
            uint16_t* buf = OSC_ADC_GetBuffer(ch);
            uint8_t tdi = UI_GetTimeDivIndex();
            uint8_t vdi = UI_GetVoltDivIndex();
            uint32_t sr = 100000000UL / (TIME_DIV_ARR[tdi] + 1);
            float f = Signal_GetFrequency(buf, SAMPLE_BUFFER_SIZE, sr);
            float v = Signal_GetVpp(buf, SAMPLE_BUFFER_SIZE);
            Display_DrawMeasurements(f, v, ch, tdi, vdi, 1);
            holdDrawn = 1;
        }
    }
    else
    {
        holdDrawn = 0;
    }

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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 200;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
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
#ifdef USE_FULL_ASSERT
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
