/* USER CODE BEGIN Header */
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
#include "adc.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
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
int timer_val =0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define SENSOR_ADC_MAX 		4095.0f
#define SENSOR_ADC_REFV  		3.3f
#define SENSOR_AVG_SLOPE 		2.5f
#define SENSOR_V25  			        0.76f
float sensorValue, temperature;
int in_led;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htim);
	HAL_ADC_Start(&hadc1);
	sensorValue = (int32_t)HAL_ADC_GetValue(&hadc1);
	//HAL_ADC_Stop(&hadc1);
	float sensor_vol = sensorValue * SENSOR_ADC_REFV / SENSOR_ADC_MAX;
	temperature = ((sensor_vol - SENSOR_V25) *1000 / SENSOR_AVG_SLOPE) + 25.0f;
	printf("Gia Tri ADC %f \n",temperature);
	in_led = (int)temperature;
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TIM_PeriodElapsedCallback could be implemented in the user file
   */
}

void shiftout(int data)
{
	for(int i=0;i <8;i++) // gui tin hieu 1 SO CAN HIEN THI LÊN 1 LED 
	{
		if(data&0x80)
		{
			HAL_GPIO_WritePin(DIN_GPIO_Port,DIN_Pin,GPIO_PIN_SET);
		}
		else
		{
			HAL_GPIO_WritePin(DIN_GPIO_Port,DIN_Pin,GPIO_PIN_RESET);
		}
		data<<=1;
		HAL_GPIO_WritePin(SCLK_GPIO_Port,SCLK_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(SCLK_GPIO_Port,SCLK_Pin,GPIO_PIN_RESET);
				
	}
}
char led_index[9]={0};
void display_4led(int so_int)
{
	unsigned char MA7DOAN[] = 
	{
		0xC0,// 0
		0xF9,// 1
		0xA4,// 2
		0xB0,// 3
		0x99,// 4
		0x92,// 5
		0x82,// 6
		0xF8,// 7
		0x80,// 8
		0x90,// 9
		0x88,// A 10
		0x83,// B 11
		0xC6,// C 12
		0xA1,// D 13
		0x86,// E 14
		0x8E,// F 15
		0xff // NONE 16  
	};
	led_index[8] = so_int/1000;
	led_index[4] = so_int%1000/100;			
	led_index[2] = so_int%100/10;			
	led_index[1] = so_int%10;	
	if(led_index[8] == 0)
	{
		led_index[8] = 16; // MA7DOAN[16] -> tat led 7 doan
		if(led_index[4] == 0)
		{
			led_index[4] = 16; // MA7DOAN[16] -> tat led 7 doan
					if(led_index[2] == 0)
					{
						led_index[2] = 16; // MA7DOAN[16] -> tat led 7 doan
					}
		}
	}		

	for(int i = 1; i < 9; i = i*2)
	{
		HAL_GPIO_WritePin(LATCH_GPIO_Port,LATCH_Pin,GPIO_PIN_RESET);
		shiftout(MA7DOAN[led_index[i]]);
		shiftout(i);
		HAL_GPIO_WritePin(LATCH_GPIO_Port,LATCH_Pin,GPIO_PIN_SET);
	}
}

//void display_nghin(int data)
//{
//	// 0x08 => 0b0000_1000 -> VI TRI LED THU 4
//	shiftout(0x08, MA7DOAN[data]);
//	HAL_GPIO_WritePin(LATCH_GPIO_Port,LATCH_Pin,GPIO_PIN_SET);
//	HAL_GPIO_WritePin(LATCH_GPIO_Port,LATCH_Pin,GPIO_PIN_RESET);
//}
//void display_tram(int data)
//{
//	// 0x04 => 0b0000_0100 -> VI TRI LED THU 3
//	shiftout(0x04,  MA7DOAN[data]);
//	HAL_GPIO_WritePin(LATCH_GPIO_Port,LATCH_Pin,GPIO_PIN_SET);
//	HAL_GPIO_WritePin(LATCH_GPIO_Port,LATCH_Pin,GPIO_PIN_RESET);
//}
//void display_chuc(int data)
//{
//	// 0x02 => 0b0000_0010 -> VI TRI LED THU 2
//	shiftout(0x02,  MA7DOAN[data]);
//	HAL_GPIO_WritePin(LATCH_GPIO_Port,LATCH_Pin,GPIO_PIN_SET);
//	HAL_GPIO_WritePin(LATCH_GPIO_Port,LATCH_Pin,GPIO_PIN_RESET);
//}
//void display_donvi(int data)
//{
//	// 0x01 => 0b0000_0001 -> VI TRI LED THU 1
//	shiftout(0x01,  MA7DOAN[data]);
//	HAL_GPIO_WritePin(LATCH_GPIO_Port,LATCH_Pin,GPIO_PIN_SET);
//	HAL_GPIO_WritePin(LATCH_GPIO_Port,LATCH_Pin,GPIO_PIN_RESET);
//}
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
  MX_ADC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
	HAL_ADC_Start(&hadc1);
	HAL_TIM_Base_Start_IT(&htim2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		display_4led(in_led);
		//timer_val = __HAL_TIM_GET_COUNTER(&htim2);
		//printf("Thoi gian thoi qua %d \n",timer_val);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
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
