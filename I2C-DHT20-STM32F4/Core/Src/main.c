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
#include <stdio.h>
#include "i2c-lcd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADDRESS_SLAVE_DHT20 0X38 
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
float RH = 0;
float Temp = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
unsigned char  CheckCrc8(unsigned char *pDat,unsigned char Lenth)
{
	unsigned char crc = 0xff, i, j;

	for (i = 0; i < Lenth ; i++)
	{
			crc = crc ^ *pDat;
			for (j = 0; j < 8; j++)
			{
					if (crc & 0x80) crc = (crc << 1) ^ 0x31;
					else crc <<= 1;
			}
			pDat++;
	}
	return crc;
}

uint8_t read_register(uint8_t register_pointer)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t return_value = 0;

	status = HAL_I2C_Mem_Read(&hi2c1, ADDRESS_SLAVE_DHT20 << 1, (uint16_t)register_pointer, I2C_MEMADD_SIZE_8BIT, &return_value, 1, 100);

	/* Check the communication status */
	if(status != HAL_OK)
	{
			
	}
	printf("return_value for %X : %X \n",register_pointer, return_value);
	return return_value;
}

typedef enum 
{
  DHT20_OK       			= 0x00U,
  DHT20_ERROR_INIT    = 0x01U,
  DHT20_ERROR_CRC     = 0x02U,
  DHT20_BUSY				  = 0x03U
} Status_Trigger_DHT20;

//Trigger measurement data
Status_Trigger_DHT20 trigger_measurement_data_dht20()
{
	Status_Trigger_DHT20 status = DHT20_OK;

	HAL_Delay(100);
	uint8_t status_init = read_register(0x71);
	if ((status_init & 0x18) == 0x18) // et a byte of status word by sending 0x71. If the status word and 0x18 are not equal to 0x18
	{
		HAL_Delay(10);
		uint8_t data_t[3];
		data_t[0] = 0xAC; //address
		data_t[1] = 0x33; //data1
		data_t[2] = 0x00; //data2
		HAL_I2C_Master_Transmit (&hi2c1, ADDRESS_SLAVE_DHT20 << 1,(uint8_t *) data_t, 3, 100);
		HAL_Delay(80);
		
		uint8_t buffer[7];
		uint32_t data_read = 0;

    // receive the 2 x 8bit data into the receive buffer
    HAL_I2C_Master_Receive(&hi2c1, ADDRESS_SLAVE_DHT20 << 1, buffer, 7, 100);   
		
		for (int i =0; i<7;i++)
		{
			printf("bufffer %d: %X \n",i, buffer[i]);
		}
		
		// Check Bit[7] at Byte0
		if ((buffer[0] & 0x80) == 0x00)
		{
			// Check CRC
			if(CheckCrc8(&buffer[0],6)==buffer[6])
			{
				printf("CRC OK \n");
				data_read=buffer[1]; //0x90
				data_read=data_read<<8;// 0x9000
				data_read+=buffer[2]; //0x9000 + 0x28 => 0x9028
				data_read=data_read<<8; //0x902800
				data_read+=buffer[3]; //0x902800 + 0x96 => 0x902896
				data_read=data_read>>4;				 // 0x902896 => 0x90289
				
				RH = (float)data_read*100/1048576;
				printf("RH %0.1f%% \n", RH);
				
				data_read=buffer[3]&0x0F;
				data_read=data_read<<8;
				data_read+=buffer[4];
				data_read=data_read<<8;
				data_read+=buffer[5];		
				
				Temp = (float)data_read*200/1048576-50;
				printf("Temp %0.1f \n", Temp);
			}
			else 
			{
				printf("CRC fail \n");
				status = DHT20_ERROR_CRC;
			}
		}
		else
		{
			printf("Check Bit[7] at Byte0: %x \n",buffer[0]&0x80);
			status = DHT20_BUSY;
		}

	}
	else //  initialize the 0x1B, 0x1C, 0x1E registers
	{
		status = DHT20_ERROR_INIT;
	}
	return status;
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
	// scan address i2c
	for(int i=1; i<128; i++)
	{
			int ret = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(i<<1), 3, 5);
			if (ret != HAL_OK) /* No ACK Received At That Address */
			{
					printf("- \n");
			}
			else if(ret == HAL_OK)
			{
				printf("HAL_I2C_IsDeviceReady: 0x%X \n",i);

			}
	}
	
	HAL_Delay(100);
	lcd_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		Status_Trigger_DHT20 check_sensor  = trigger_measurement_data_dht20();
		char buffer[20];
		if (check_sensor == DHT20_OK)
		{
			sprintf (buffer, "Humidity: %0.1f", RH);
			lcd_put_cur(0,0);
			lcd_send_string (buffer);
			sprintf (buffer, "Temperature: %0.1f", Temp);
			lcd_put_cur(1,0);
			lcd_send_string (buffer);
			HAL_Delay(700);
		}
		else if (check_sensor == DHT20_BUSY)
		{
			sprintf (buffer, "Humidity:--BUSY--");
			lcd_put_cur(0,0);
			lcd_send_string (buffer);
			sprintf (buffer, "Temperature:--BUSY--");
			lcd_put_cur(1,0);
			lcd_send_string (buffer);
			HAL_Delay(1000);
			lcd_clear();
		}
		else
		{
			sprintf (buffer, "Humidity:--NONE--");
			lcd_put_cur(0,0);
			lcd_send_string (buffer);
			sprintf (buffer, "Temperature:--NONE--");
			lcd_put_cur(1,0);
			lcd_send_string (buffer);
			HAL_Delay(1000);
			lcd_clear();
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
