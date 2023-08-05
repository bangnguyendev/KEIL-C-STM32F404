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
#include "i2c.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "i2c-lcd.h"
#include "keypad44.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
GPIO_TypeDef* row_port[4] = {ROW1_GPIO_Port, ROW2_GPIO_Port, ROW3_GPIO_Port, ROW4_GPIO_Port};
GPIO_TypeDef* col_port[4] = {COL1_GPIO_Port, COL2_GPIO_Port, COL3_GPIO_Port, COL4_GPIO_Port};

char row_pin[4] = {ROW1_Pin, ROW2_Pin, ROW3_Pin, ROW4_Pin};
char col_pin[4] = {COL1_Pin, COL2_Pin, COL3_Pin, COL4_Pin};
char key;
int mode = 0;
int so[5] = {48};
int number;
int dem = 0;
int matrix_col, matrix_row;  //debug printf LCD
char scan_key()
{
	char keys[4][4] =
	{
		{'1', '2', '3', 'A'},
		{'4', '5', '6', 'B'},
		{'7', '8', '9', 'C'},
		{'*', '0', '#', 'D'},
	};
	for (int row = 0; row < 4; row ++) //quet hang
	{
		HAL_GPIO_WritePin(row_port[row], row_pin[row], GPIO_PIN_RESET);
		for (int col = 0; col < 4; col ++) //quet cot
		{
			if (!HAL_GPIO_ReadPin(col_port[col],col_pin[col]))
			{
					HAL_Delay(20);
					while(!HAL_GPIO_ReadPin(col_port[col],col_pin[col])){} // cho tha phim
					HAL_GPIO_WritePin(row_port[row], row_pin[row], GPIO_PIN_SET);
					matrix_row = row;  //debug printf LCD
					matrix_col = col;  //debug printf LCD
					return keys[row][col];
			}
		}
		HAL_GPIO_WritePin(row_port[row], row_pin[row], GPIO_PIN_SET);
	}
	return 0x20; // return ve khoang trang
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
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	lcd_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		key = scan_key();
		
		if (key == 'A')
		{
			lcd_put_cur(0,10);
			lcd_send_string("Nhap 5 so:");
			mode = 1;
		}
		else if (key == 'B')
		{
			mode = 2;
		}
		else if (key == 'D')
		{
			mode = 3;
		}
		
		if (mode == 1)
		{
			lcd_put_cur(0,0);
			lcd_send_string("Row : ");
			lcd_send_data(matrix_row + 48);

			lcd_put_cur(1,0);
			lcd_send_string("Col : ");
			lcd_send_data(matrix_col + 48);

			lcd_put_cur(2,0);
			lcd_send_string("Key :");
			lcd_send_data(key);

			lcd_put_cur(3,0);
			lcd_send_string("Key ASCII :");
			char buffer[20];
			sprintf(buffer,"0x%X - %d",key, key);
			lcd_send_string(buffer);
			
			lcd_put_cur(1,10);
			lcd_send_string("->:");
			lcd_put_cur(1,19 - dem);
			lcd_send_cmd (0x04); //Entry mode set --> I/D = 0 (increment cursor) & S = 0 (no shift)
			lcd_send_cmd (0x0F); //Display on/off control --> D = 1, C and B = 1. (Cursor and blink, last two bits)
			if ((key >= 48) && (key <=57))
			{
					if (dem < 5)
					{					
						so[dem] = key;
						lcd_send_data(key);
						dem ++;
						if (dem == 5)
						{
							mode = 3;
						}
					}
			}
			lcd_send_cmd (0x06); //Entry mode set --> I/D = 0 (increment cursor) & S = 0 (no shift)
		}
		else if (mode == 2)
		{
			lcd_put_cur(2,10);
			lcd_send_string("Xoa 5 so!!");
			HAL_Delay(600);
			for(int i = 0; i <5; i++)
			{
				so[i] = 48; // xoa ve so 0
			}
			mode = 0;
			dem = 0;
			number = 0;
			lcd_init();
		}
		else if (mode == 3)
		{
			lcd_put_cur(2,10);
			lcd_send_string("Done:");
			number = 	(so[4]-48)*10000 +
								(so[3]-48)*1000 +
								(so[2]-48)*100 +
								(so[1]-48)*10 +
								(so[0]-48);
			char buffer[20];
			sprintf(buffer,"%d",number);
			lcd_send_string(buffer);
			
			lcd_put_cur(1,14);
			lcd_send_cmd (0x0F); //Display on/off control --> D = 1, C and B = 1. (Cursor and blink, last two bits)
			
		}
		else
		{
			lcd_put_cur(0,0);
			lcd_send_string("Row : ");
			lcd_send_data(matrix_row + 48);

			lcd_put_cur(1,0);
			lcd_send_string("Col : ");
			lcd_send_data(matrix_col + 48);

			lcd_put_cur(2,0);
			lcd_send_string("Key :");
			lcd_send_data(key);

			lcd_put_cur(3,0);
			lcd_send_string("Key ASCII :");
			char buffer[20];
			sprintf(buffer,"0x%X - %d",key, key);
			lcd_send_string(buffer);
		}

		HAL_Delay(500);
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
  RCC_OscInitStruct.PLL.PLLM = 4;
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
