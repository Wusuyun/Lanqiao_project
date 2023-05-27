/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "lcd.h"
#include "string.h"
#include "i2c_hal.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct 
{
	uint16_t X_sum;//库存总量
	uint16_t Y_sum;
	uint16_t X_buy;//购买数量
	uint16_t Y_buy;
	float X_price;//单价
	float Y_price;
}XYtypedef;

XYtypedef good;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define B1 !HAL_GPIO_ReadPin(GPIOB,B1_Pin)
#define B2 !HAL_GPIO_ReadPin(GPIOB,B2_Pin)
#define B3 !HAL_GPIO_ReadPin(GPIOB,B3_Pin)
#define B4 !HAL_GPIO_ReadPin(GPIOA,B4_Pin)
#define Freq 500
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t str[50];
uint8_t RX[20];
uint8_t Lcd_mode = 0;//三个模式，0、1、2
uint16_t T_PWM = 500;//按下B4输出30%PWM5秒 
uint8_t T_Led2 = 10;//以0.1秒为间隔闪烁
uint8_t flag_B4 = 0;
uint8_t flag_Led2 = 0;
float price = 2.0;
float Z = 0;//购买的商品总价
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//重定向函数
int fputc(int ch, FILE *stream)
{
    while(!(USART1->ISR & (1<<7)));
    USART1->TDR = (uint8_t) ch;
    return ch;
}

//关闭LED
void LED_Close(void)
{
	HAL_GPIO_WritePin(GPIOC,LED1_Pin|LED2_Pin|LED3_Pin|LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin|LED8_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

void LED1(void)
{
	HAL_GPIO_WritePin(GPIOC,LED1_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC,LED2_Pin|LED3_Pin|LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin|LED8_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

void LED2(void)
{
	switch(flag_Led2)
	{
		case 0:
			HAL_GPIO_WritePin(GPIOC,LED2_Pin,GPIO_PIN_SET);
			break;
		case 1:
			HAL_GPIO_WritePin(GPIOC,LED2_Pin,GPIO_PIN_RESET);
			break;
	}
	if(flag_B4)
	{
		HAL_GPIO_WritePin(GPIOC,LED3_Pin|LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin|LED8_Pin,GPIO_PIN_SET);
	}else HAL_GPIO_WritePin(GPIOC,LED1_Pin|LED3_Pin|LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin|LED8_Pin,GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

void good_Init(void)
{
	good.X_buy = 0;
	good.Y_buy = 0;
	good.X_sum = 10;
	good.Y_sum = 10;
	good.X_price = 1.0;
	good.Y_price = 1.0;
}

//LCD显示函数

void Lcd1(void)
{
	LCD_DisplayStringLine(Line1,(uint8_t*)"        SHOP         ");
	
	sprintf((char*)str,"     X:%d       ",good.X_buy);
	LCD_DisplayStringLine(Line3,str);
	
	sprintf((char*)str,"     Y:%d       ",good.Y_buy);
	LCD_DisplayStringLine(Line4,str);
}	

void Lcd2(void)
{
	LCD_DisplayStringLine(Line1,(uint8_t*)"        PRICE         ");
	
	sprintf((char*)str,"     X:%0.1f       ",good.X_price);
	LCD_DisplayStringLine(Line3,str);
	
	sprintf((char*)str,"     Y:%0.1f       ",good.Y_price);
	LCD_DisplayStringLine(Line4,str);
}	

void Lcd3(void)
{
	LCD_DisplayStringLine(Line1,(uint8_t*)"        REP         ");
	
	sprintf((char*)str,"     X:%d       ",good.X_sum);
	LCD_DisplayStringLine(Line3,str);
	
	sprintf((char*)str,"     Y:%d       ",good.Y_sum);
	LCD_DisplayStringLine(Line4,str);
}	


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(flag_B4)
	{
		__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,Freq*0.3);
		LED1();
		if(--T_PWM == 0)
		{
			flag_B4 = 0;
			T_PWM = 500;
			LED_Close();
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,Freq*0.05);
		}
	}
	
	if((good.X_sum == 0) && (good.Y_sum == 0))
	{
		if(--T_Led2 == 0)
		{
			flag_Led2 = !flag_Led2; 
		  LED2();
			T_Led2 = 10;
		}
	}else 
	{
		flag_Led2 = 0;
		LED2();
	}
}

//串口回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(RX[0] == '?')
	{
		printf("X:%0.1f,Y:%0.1f\r\n",good.X_price,good.Y_price);
	}

	HAL_UART_Receive_IT(&huart1,RX,1);
}

//EEPROM读函数

void EEPROM_R(void)
{
	good.X_sum = EEPROM_Read(0x00);
	good.Y_sum = EEPROM_Read(0x01);
	good.X_price = (float)EEPROM_Read(0x02)/10;
	good.Y_price = (float)EEPROM_Read(0x03)/10;	
}

//按键扫描函数
void Keyscan(void)
{
	if(B1)
	{
		printf("B1_Down!\r\n");
		if(++Lcd_mode == 3)
		{
			Lcd_mode = 0;
		}
		while(B1);
	}
	if(B2)
	{
		printf("B2_Down!\r\n");
		switch(Lcd_mode)
		{
			case 0:
			if(good.X_buy == good.X_sum)
			{
				good.X_buy = 0;
			}else good.X_buy++;
				break;
			case 1:
				if(good.X_price >= 2.0f)
				{
					good.X_price = 1.0;
					EEPROM_Write(0x02,(uint8_t)(good.X_price*10));
				}else	
				{
					good.X_price += 0.1f;
					EEPROM_Write(0x02,(uint8_t)(good.X_price*10));
				}
				break;
			case 2:
				good.X_sum++;
				EEPROM_Write(0x00,good.X_sum);
				break;
		}
		while(B2);
	}
	if(B3)
	{
		printf("B3_Down!\r\n");
		switch(Lcd_mode)
		{
			case 0:
			 if(good.Y_buy == good.Y_sum)
			 {
					good.Y_buy = 0;
			 }else good.Y_buy++;
				break;
			case 1:
				if(good.Y_price >= 2.0f)
				{
					good.Y_price = 1.0;
					EEPROM_Write(0x03,(uint8_t)(good.Y_price*10));
				}else	
				{
					good.Y_price += 0.1f;
					EEPROM_Write(0x03,(uint8_t)(good.Y_price*10));
				}
				break;
			case 2:
				good.Y_sum++;
				EEPROM_Write(0x01,good.Y_sum);
				break;
		}
		while(B3);
	}
	if(B4)
	{
		printf("B4_Down!\r\n");
		if(Lcd_mode == 0)
		{
			flag_B4 = 1;
			good.X_sum -= good.X_buy;
			good.Y_sum -= good.Y_buy;
			EEPROM_Write(0x00,good.X_sum);
			Z = (good.X_buy*good.X_price) + (good.Y_buy*good.Y_price);
			printf("X:%d,Y:%d,Z:%0.1f\r\n",good.X_buy,good.Y_buy,Z);
			good.X_buy = 0;
			good.Y_buy = 0;
			EEPROM_Write(0x01,good.Y_sum);
		}
		while(B4);
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
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

	I2CInit();
	LCD_Init();
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
	good_Init();
	printf("USART OK!\r\n");
	
	LED_Close();
	EEPROM_R();
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
	__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,Freq*0.05);
	
	HAL_TIM_Base_Start_IT(&htim1);
	
	HAL_UART_Receive_IT(&huart1,RX,1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
		Keyscan();
		switch(Lcd_mode)
		{
			case 0:
				Lcd1();
				break;
			case 1:
				Lcd2();
				break;
			case 2:
				Lcd3();
				break;
		}
    /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV3;
  RCC_OscInitStruct.PLL.PLLN = 20;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
