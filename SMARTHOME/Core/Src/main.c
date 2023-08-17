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
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c-lcd.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
	char password[6];
	char passUser[3][6];
	enum User {
		USER_0,
		USER_1,
		USER_2,
		USER_NULL,
	} user;
	enum Mode {
		Mode_None = 0,
		Mode_Keypad = 1,
		Mode_NFC,
		Mode_Fingerprint,
		Mode_Sel,
		Mode_AddUser,
		Mode_ClearUser,
		Mode_Delete,
	} mode;
	enum Status {
		SMARTHOME_NG,
		SMARTHOME_OK,
	} status;
}SMARTHOME;




/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char buffer[20];
// su dung uart debug
int fputc(int ch, FILE * f) {
  HAL_UART_Transmit( & huart2, (uint8_t * ) & ch, 1, HAL_MAX_DELAY);
  return ch;
}

SMARTHOME smarthome;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


#define SLAVE_ADDRESS_EEPROM 0x50 // change this according to ur setup

void write_eeprom (int address ,int value)
{
	uint8_t data_t[4];

	data_t[0] = address;  
	data_t[1] = value;  
	HAL_I2C_Master_Transmit (&hi2c2, SLAVE_ADDRESS_EEPROM << 1,(uint8_t *) data_t, 2, HAL_MAX_DELAY);
	HAL_Delay(10); // tWR Write Cycle Time 5 ms
	printf("write_eeprom at 0x%X \r\n",address);
}

uint8_t read_eeprom(uint16_t register_pointer)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t return_value = 0;

	status = HAL_I2C_Mem_Read(&hi2c2, SLAVE_ADDRESS_EEPROM << 1, (uint16_t)register_pointer, I2C_MEMADD_SIZE_8BIT, &return_value, 1, 100);

	/* Check the communication status */
	if(status != HAL_OK)
	{
			
	}
	printf("Read EEPROM at [%d] : %d \r\n",register_pointer, return_value);
	return return_value;
}



int matrix_col, matrix_row;  //debug printf LCD
GPIO_TypeDef* row_port[4] = {ROW0_GPIO_Port, ROW1_GPIO_Port, ROW2_GPIO_Port, ROW3_GPIO_Port};
GPIO_TypeDef* col_port[4] = {COL0_GPIO_Port, COL1_GPIO_Port, COL2_GPIO_Port, COL3_GPIO_Port};

char row_pin[4] = {ROW0_Pin, ROW1_Pin, ROW2_Pin, ROW3_Pin};
char col_pin[4] = {COL0_Pin, COL1_Pin, COL2_Pin, COL3_Pin};
char key;
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
//					matrix_row = row;  //debug printf LCD
//					matrix_col = col;  //debug printf LCD
					return keys[row][col];
			}
		}
		HAL_GPIO_WritePin(row_port[row], row_pin[row], GPIO_PIN_SET);
	}
	return 0x20; // return ve khoang trang
}

SMARTHOME check_pass(int * sll_pass)
{	
	smarthome.status = SMARTHOME_OK;
	smarthome.user = USER_NULL;
	
	
	// check user 0
	for(int j = 0; j < *sll_pass; j++)
	{	
		smarthome.status = SMARTHOME_OK;
		smarthome.passUser[0][j] = read_eeprom(j);
		printf("smarthome.password[%d] vs smarthome.passUser[0][%d] > %d : %d \r\n", j,j,smarthome.password[j],smarthome.passUser[0][j]);
		if (smarthome.password[j]!=smarthome.passUser[0][j])
		{
			printf("User 0: NG \r\n");
			smarthome.status = SMARTHOME_NG;
			break;
		}
	}

	if (smarthome.status == SMARTHOME_OK)
	{
		lcd_put_cur(1,0);
		lcd_send_string("User 0: OK       ");
		printf("User 0: OK        \r\n");
		smarthome.user = USER_0;
		return smarthome;
	}
	// end check user 0
	
	
	// check user 1
	for(int j = 0; j < *sll_pass; j++)
	{	
		smarthome.status = SMARTHOME_OK;
		smarthome.passUser[1][j] = read_eeprom(j+6);
		printf("smarthome.password[%d] vs smarthome.passUser[1][%d] > %d : %d \r\n", j,j,smarthome.password[j],smarthome.passUser[1][j]);
		if (smarthome.password[j]!=smarthome.passUser[1][j])
		{
			printf("Usser 1: NG \r\n");
			smarthome.status = SMARTHOME_NG;
			break;
		}
	}

	if (smarthome.status == SMARTHOME_OK)
	{
		lcd_put_cur(1,0);
		lcd_send_string("User 1: OK       ");
		printf("User 1: OK        \r\n");
		smarthome.user = USER_1;
		return smarthome;
	}					
	// end check user 1
	
	
	// check user 2
	for(int j = 0; j < *sll_pass; j++)
	{	
		smarthome.status = SMARTHOME_OK;
		smarthome.passUser[2][j] = read_eeprom(j+6+6);
		printf("smarthome.password[%d] vs smarthome.passUser[2][%d] > %d : %d \r\n", j,j,smarthome.password[j],smarthome.passUser[2][j]);
		if (smarthome.password[j]!=smarthome.passUser[2][j])
		{
			printf("Usser 2: NG \r\n");
			smarthome.status = SMARTHOME_NG;
			break;
		}
	}

	if (smarthome.status == SMARTHOME_OK)
	{
		lcd_put_cur(1,0);
		lcd_send_string("User 2: OK       ");
		printf("User 2: OK        \r\n");
		smarthome.user = USER_2;
		return smarthome;
	}
	// end check user 2
	return smarthome;
}

void write_eeprom_user(int user)
{
	
	int sll_pass = 0;
	printf("User %d duoc chon \r\n", user);
	while(smarthome.mode == Mode_AddUser)
	{
			lcd_put_cur(0,0);
			lcd_send_string("Add MK User");
			lcd_send_data(user+48);
			lcd_put_cur(2,0);
			lcd_send_string("Key #: Done");
			lcd_put_cur(3,0);
			lcd_send_string("Key *: Exit");
			lcd_send_cmd (0x0F); //Display on/off control --> D = 1, C and B = 1. (Cursor and blink, last two bits)
			lcd_put_cur(1, sll_pass);
			
			key = scan_key();
			
			if ((47 < key) && (key < 58)) //phim so 48 -> 0, 57 -> 9
			{
				if (sll_pass < 6 ) // mode 6 so
				{
					printf("KeyNumber: %s \r\n", &key);
					smarthome.password[sll_pass] = key;

					lcd_send_data(key);
					sll_pass++;
				}
			}
			else if (key == '*') //exit
			{
				printf("Exit setup mat khau User %d \r\n", user);
				smarthome.mode = Mode_None;
			}
			
			if ((sll_pass > 3) && (key == '#')) //setup done
			{
				printf("Mat khau co %d ky tu \r\n", sll_pass);
				for(int i = 0; i < sll_pass ; i++)
				{
					if (user == 0)
					{
						write_eeprom(i, smarthome.password[i]);
						printf("Mat khau: %d - %d \r\n", i, read_eeprom(i));
					}
					else	if (user == 1)
					{
						write_eeprom(i+6, smarthome.password[i]);
						printf("Mat khau: %d - %d \r\n", i, read_eeprom(i+6));
					}
					else	if (user == 2)
					{
						write_eeprom(i+6+6, smarthome.password[i]);
						printf("Mat khau: %d - %d \r\n", i, read_eeprom(i+6+6));
					}			
				}
				printf("Done setup mat khau User %d \r\n", user);
				smarthome.mode = Mode_None;
			}
			HAL_Delay(100);
	}		
}


int display_keypad(void)
{		
		int sll_pass = 0;

		while(smarthome.mode == Mode_Keypad)
		{
				lcd_put_cur(0,0);
				lcd_send_string("Nhap mat khau -> #");
				
				lcd_send_cmd (0x0F); //Display on/off control --> D = 1, C and B = 1. (Cursor and blink, last two bits)
				lcd_put_cur(1, sll_pass);
				
				key = scan_key();
				
				if ((47 < key) && (key < 58)) //phim so 48 -> 0, 57 -> 9
				{
					if (sll_pass < 6 ) // mode 6 so
					{
						printf("KeyNumber: %s \r\n", &key);
						smarthome.password[sll_pass] = key;
						lcd_send_data(key);
						sll_pass++;
					}
				}
				if ((sll_pass > 3) && (key == '#'))
				{
					printf("Kiem tra mat khau \r\n");
					
					if (check_pass(&sll_pass).status)
					{
						smarthome.status = SMARTHOME_OK;
						smarthome.mode = Mode_None;
						HAL_Delay(1000);
						lcd_clear();
						HAL_Delay(500);
						return smarthome.status;
					}
					else
					{
						printf("PASS User: NG        \r\n");
						sll_pass = 0;
						lcd_put_cur(1,0);
						lcd_send_string("PASS User: NG       ");
						HAL_Delay(1000);
						lcd_put_cur(1,0);
						lcd_send_string("                 ");
						HAL_Delay(500);
						smarthome.status = SMARTHOME_NG;
					}
				}
				HAL_Delay(100);
		}
		return smarthome.status;
}

void reset_eeprom()
{
	for(int i = 0 ; i < 20; i++)
	{
		write_eeprom(i,0+48);
		read_eeprom(i);
	}
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
  MX_USART2_UART_Init();
  MX_SPI2_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
	lcd_init();
	//reset_eeprom();

	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		key = scan_key();

		if ((47 < key) && (key < 58)) //phim so 48 -> 0, 57 -> 9
		{
			printf("KeyNumber: %s \r\n", &key);

		}
		else if (key != 0x20)// cac phim chuc nang (loai tru 0x20 vi 0x20 la return khi k bam phim)
		{
			printf("KeyFunction: %s \r\n", &key);

			if (key == 'A')
			{	
				lcd_clear();
				smarthome.mode = Mode_Keypad;
				printf("smarthome.mode: Mode_Keypad \r\n");
			}
			else if (key == 'B')
			{
				lcd_clear();
				smarthome.mode = Mode_NFC;
				printf("smarthome.mode: Mode_NFC \r\n");
			}
			else if (key == 'C')
			{
				lcd_clear();
				smarthome.mode = Mode_Fingerprint;
				printf("smarthome.mode: Mode_Fingerprint \r\n");
			}
			else if (key == 'D')
			{
				lcd_clear();
				smarthome.mode = Mode_Delete;
				printf("smarthome.mode: Mode_Delete \r\n");
			}
			
		}

		if (smarthome.mode == Mode_None)
		{
			lcd_put_cur(0,0);
			lcd_send_string("A: Mode_Keypad");
			lcd_put_cur(1,0);
			lcd_send_string("B: Mode_NFC");
			lcd_put_cur(2,0);
			lcd_send_string("C: Mode_Fingerprint");	
			lcd_put_cur(3,0);
			lcd_send_string("D: Mode_Delete");	
			
			// lock door
			HAL_GPIO_WritePin(Door_GPIO_Port, Door_Pin, GPIO_PIN_SET); // lock
		}
		else if (smarthome.mode == Mode_Keypad)
		{
			printf("Status mo cua: %d \r\n",display_keypad());		
			if (display_keypad() == SMARTHOME_OK)
			{
				HAL_GPIO_WritePin(Door_GPIO_Port, Door_Pin, GPIO_PIN_RESET); // open
				lcd_put_cur(0,0);
				lcd_send_string("Mode_Keypad:");
				lcd_put_cur(1,0);
				lcd_send_string("Open Door 5 Sec");
				HAL_Delay(5000);
				lcd_clear();
				smarthome.mode = Mode_AddUser;
				
				write_eeprom_user(smarthome.user);
			}
		}
		else if (smarthome.mode == Mode_NFC)
		{
			
		}
		else if (smarthome.mode == Mode_Fingerprint)
		{
			
		}
		else if (smarthome.mode == Mode_Delete)
		{
			lcd_init();
			smarthome.mode = Mode_None;
			HAL_Delay(500);
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
