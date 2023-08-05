
/** Put this in the src folder **/

#include "i2c-lcd.h"
extern I2C_HandleTypeDef hi2c1;  // change your handler here accordingly

#define SLAVE_ADDRESS_LCD 0x27 // change this according to ur setup

void lcd_send_cmd (char cmd)
{
  char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd&0xf0); /*higher nibble of data first*/
	data_l = ((cmd<<4)&0xf0); /*lower nibble of data */
	data_t[0] = data_u|0x0C;  //en=1, rs=0
	data_t[1] = data_u|0x08;  //en=0, rs=0
	data_t[2] = data_l|0x0C;  //en=1, rs=0
	data_t[3] = data_l|0x08;  //en=0, rs=0
	HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD << 1,(uint8_t *) data_t, 4, 100);
}

void lcd_send_data (char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;  //en=1, rs=1
	data_t[1] = data_u|0x09;  //en=0, rs=1
	data_t[2] = data_l|0x0D;  //en=1, rs=1
	data_t[3] = data_l|0x09;  //en=0, rs=1
	HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD << 1,(uint8_t *) data_t, 4, 100);
}

void lcd_clear (void)
{
	lcd_send_cmd (0x80);
	for (int i=0; i<80; i++)
	{
		lcd_send_data (' ');
	}
}

#define LCD_SETDDRAMADDR 0x80
void lcd_put_cur(int row, int col)
{
		int cmd_col = 0x80;
    switch (row)
    {
        case 0:
            cmd_col = LCD_SETDDRAMADDR | (col + 0x00);
            break;
        case 1:
            cmd_col = LCD_SETDDRAMADDR | (col + 0x40);
            break;
				case 2:
            cmd_col = LCD_SETDDRAMADDR | (col + 0x14);
            break;
        case 3:
            cmd_col = LCD_SETDDRAMADDR | (col + 0x54);
            break;
    }
    lcd_send_cmd (cmd_col);
}

void lcd_init (void)
{
	// 4 bit initialisation
	HAL_Delay(50);  // wait for >40ms
	lcd_send_cmd (0x30);
	HAL_Delay(5);  // wait for >4.1ms
	lcd_send_cmd (0x30);
	HAL_Delay(1);  // wait for >100us
	lcd_send_cmd (0x30);
	HAL_Delay(10);
	lcd_send_cmd (0x20);  //4bit mode
	HAL_Delay(10);

  // dislay initialisation
	lcd_send_cmd (0x28); //Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	HAL_Delay(1);
	lcd_send_cmd (0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off
	HAL_Delay(1);
	lcd_send_cmd (0x01);  //clear display
	HAL_Delay(1);
	HAL_Delay(1);
	lcd_send_cmd (0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	HAL_Delay(1);
	lcd_send_cmd (0x0C); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
	HAL_Delay(5);
	lcd_clear();
	HAL_Delay(100);
}

void lcd_send_string (char *str)
{
	while (*str) lcd_send_data (*str++);
}


void lcd_sent_number_xxxx(int number)
{
	int thousands = number / 1000;
	int hundreds = (number % 1000) / 100;
	int tens = (number % 100) / 10;
	int units = number % 10;
	lcd_send_data(thousands + 0x30); // + 0x30 tuong duong +48, tuong duong + '0' trong ASCII
	lcd_send_data(hundreds + 0x30);
	lcd_send_data(tens + 0x30);
	lcd_send_data(units + 0x30);
}

void lcd_sent_number_xxxx_no_zero(int number)
{
	int thousands = number / 1000;
	int hundreds = (number % 1000) / 100;
	int tens = (number % 100) / 10;
	int units = number % 10;
	char so_xxxx[4];

	so_xxxx[0] = thousands + 0x30;
	so_xxxx[1] = hundreds + 0x30;
	so_xxxx[2] = tens + 0x30;
	so_xxxx[3] = units + 0x30;
	if (thousands == 0)
	{
		so_xxxx[0] = 32;
		if (hundreds == 0)
		{
			so_xxxx[1] = 32;
			if (tens == 0)
			{
				so_xxxx[2] = 32;
			}	
		}
	}
	
	lcd_send_data(so_xxxx[0]);
	lcd_send_data(so_xxxx[1]);
	lcd_send_data(so_xxxx[2]);
	lcd_send_data(so_xxxx[3]);
}
