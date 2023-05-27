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
#include "lcd.h"
#include "stdio.h"
#include "i2c_hal.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
	uint8_t hour;
	uint8_t minite;
	uint8_t second;
	uint8_t T_second;
	uint16_t Long;
	uint32_t TimeTotal;
	uint8_t TimeLED;
	
}Timtypedef;

	Timtypedef T = {0,1,55,100,0,115,25};
	
typedef struct
{
	uint8_t Key;
	uint8_t Key_Long;
	uint8_t Key_Short;
	uint8_t EEPROM;
	uint8_t KeyB2;
	
}Flagtypedef;

	Flagtypedef Flag = {0,0,0,1,1};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define B1 !HAL_GPIO_ReadPin(GPIOB,B1_Pin)
#define B2 !HAL_GPIO_ReadPin(GPIOB,B2_Pin)
#define B3 !HAL_GPIO_ReadPin(GPIOB,B3_Pin)
#define B4 !HAL_GPIO_ReadPin(GPIOA,B4_Pin)
#define Running 1
#define Standby 2
#define Setting 3
#define Pause 4
		
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t str[50];
uint8_t State = 1;
uint8_t Time = 0;//定时器专用
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//重定向函数
int fputc(int ch,FILE *stream)
{
	while(!(USART1->ISR & (1<<7)));
	USART1->TDR = (uint8_t)ch;
	return ch;
}

//时间计算
void Cal_Time(void)
{
	T.TimeTotal = (T.hour*3600) + (T.minite*60) + T.second;
}

//EEPROM写函数
void EEPROM_W(void)
{
	if(Flag.Key_Long == 2)
	{
		Flag.Key_Long = 0;
		switch (Flag.EEPROM)
		{
			case 1:
				EEPROM_Write(0x00,T.hour);
				HAL_Delay(10);
				EEPROM_Write(0x01,T.minite);
				HAL_Delay(10);
				EEPROM_Write(0x02,T.second);
				HAL_Delay(10);
				break;
			case 2:
				EEPROM_Write(0x03,T.hour);
				HAL_Delay(10);
				EEPROM_Write(0x04,T.minite);
				HAL_Delay(10);
				EEPROM_Write(0x05,T.second);
				HAL_Delay(10);
				break;
			case 3:
				EEPROM_Write(0x06,T.hour);
				HAL_Delay(10);
				EEPROM_Write(0x07,T.minite);
				HAL_Delay(10);
				EEPROM_Write(0x08,T.second);
				HAL_Delay(10);
				break;
			case 4:
				EEPROM_Write(0x10,T.hour);
				HAL_Delay(10);
				EEPROM_Write(0x11,T.minite);
				HAL_Delay(10);
				EEPROM_Write(0x12,T.second);
				HAL_Delay(10);
				break;
			case 5:
				EEPROM_Write(0x13,T.hour);
				HAL_Delay(10);
				EEPROM_Write(0x14,T.minite);
				HAL_Delay(10);
				EEPROM_Write(0x15,T.second);
				HAL_Delay(10);
				break;
		}
		printf("EEPROM_Weite OK!\r\n");
	}
}

//EEPROM读函数
void EEPROM_R(void)
{
	switch (Flag.EEPROM)
	{
		case 1:
			T.hour =  EEPROM_Read(0x00);
			T.minite = EEPROM_Read(0x01);
			T.second = EEPROM_Read(0x02);
			break;
		case 2:
			T.hour =  EEPROM_Read(0x03);
			T.minite = EEPROM_Read(0x04);
			T.second = EEPROM_Read(0x05);
			break;
		case 3:
			T.hour =  EEPROM_Read(0x06);
			T.minite = EEPROM_Read(0x07);
			T.second = EEPROM_Read(0x08);
			break;
		case 4:
			T.hour =  EEPROM_Read(0x10);
			T.minite = EEPROM_Read(0x11);
			T.second = EEPROM_Read(0x12);
			break;
		case 5:
			T.hour =  EEPROM_Read(0x13);
			T.minite = EEPROM_Read(0x14);
			T.second = EEPROM_Read(0x15);
			break;
	}
	Cal_Time();
}

//LED闪烁函数
void LED_BL(void)
{
	if(State == Running)
	{
		HAL_GPIO_WritePin(GPIOC,LED2_Pin|LED3_Pin|LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin|LED8_Pin,GPIO_PIN_SET);
		HAL_GPIO_TogglePin(GPIOC,LED1_Pin);
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
	}
}
//LCD显示函数
void LCD_Show(void)
{
	switch (Flag.KeyB2)
	{
		case 1:
			sprintf((char*)str,"    ->%d: %d: %d    ",T.hour,T.minite,T.second);
			LCD_DisplayStringLine(Line4,str);
			break;
		case 2:
			sprintf((char*)str,"     %d: ->%d: %d   ",T.hour,T.minite,T.second);
			LCD_DisplayStringLine(Line4,str);
			break;
		case 3:
			sprintf((char*)str,"     %d: %d: ->%d   ",T.hour,T.minite,T.second);
			LCD_DisplayStringLine(Line4,str);
			break;
	}
	
	switch (State)
	{
		case Running:
			LCD_DisplayStringLine(Line7,(uint8_t*)"       Runing     ");
			break;
		case Standby:
			LCD_DisplayStringLine(Line7,(uint8_t*)"       Standby     ");
			break;
		case Setting:
			LCD_DisplayStringLine(Line7,(uint8_t*)"       Setting     ");
			break;
		case Pause:
			LCD_DisplayStringLine(Line7,(uint8_t*)"       Pause     ");
			break;
	}	
}
//时间计算规则函数
void CALTime(void)
{
	if(State == Setting)
	{
		if(Flag.Key_Short == 3)
		{
			Flag.Key_Short = 0;
			switch (Flag.KeyB2)
			{
				case 1:
					if(++T.hour == 24)
						T.hour = 0;	
					break;
				case 2:
					if(++T.minite == 60)
						T.minite = 0;
					break;
				case 3:
					if(++T.second == 60)
						T.second = 0;
					break;
			}
		}
		else if(Flag.Key_Short == 4)
		{
			Flag.Key_Short = 0;
			switch (Flag.KeyB2)
			{
				case 1:
					if(--T.hour == 255)
						T.hour = 23;	
					break;
				case 2:
					if(--T.minite == 255)
						T.minite = 59;
					break;
				case 3:
					if(--T.second == 255)
						T.second = 59;
					break;
			}
		}
	}
}
//定时器回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == htim1.Instance)
	{
		if(State == Running)
		{
			HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
			__HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_1,800);
			if(--T.T_second == 0)
			{
				T.TimeTotal--;
				T.T_second = 100;
				T.second--;
				if(T.second == 0)
				{
					if(T.minite == 0)
					{
						if(T.hour == 0)
						{
							State = Standby;
						}else 
						{
							T.hour--;
							T.minite = 59;
						}
					}else 
					{
						T.minite--;
						T.second = 59;
					}
				}
				
				if(T.TimeTotal == 0)
				{
					State = Standby;
				}
			}
		}else
		{
			HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_1);

		}
	
		
		if(Flag.Key)
		{
			if(T.Long)
			{
				T.Long--;
			}else
			{
				if(B2)//检测如果B2还处于按下状态，则为长按
				{
					Flag.Key_Long = 2;
					printf("B2_Long_Down!\r\n");
					Flag.Key = 0;
					T.Long = 0;
				}else if(B3)//一个边沿检测无法检测到松手，不适合需要检测松手的长按
				{
					Flag.Key_Long = 3;
					printf("B3_Long_Down!\r\n");
					Flag.Key = 0;
					T.Long = 0;
					
					printf("B3_Up\r\n");
					Flag.Key_Long = 0;
				}else if(B4)
				{
					Flag.Key_Long = 4;
					printf("B4_Long_Down!\r\n");
					Flag.Key = 0;
					T.Long = 0;
					State = Standby;
				}
				//否则清除标志位，如果在清除之前又有一次按下触发中断，则为双击
				else
				{
					Flag.Key = 0;
					T.Long = 0;
				}
			}
		}		
		
		if(--T.TimeLED == 0)
		{
			LED_BL();
		}
	}
}

uint32_t Flag_Key1 = 0;
uint32_t Flag_Key2 = 0;

//按键中断回调函数
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin)
	{
		case B2_Pin:
			printf("B2_Down!\r\n");
			Flag.Key = 1;
			T.Long = 80;
			State = Setting;
			if(++Flag.KeyB2 == 4)
			{
				Flag.KeyB2 = 1;
			}
			break;
		case B3_Pin:
			printf("B3_Down!\r\n");
			Flag.Key_Short = 3;
			Flag.Key = 1;
			T.Long = 80;
			CALTime();
			break;
		case B4_Pin:
			printf("B4_Down!\r\n");
			Flag.Key_Short = 4;
			Flag.Key = 1;
			T.Long = 80;
			CALTime();
			if(State == Running)
			{
				State = Pause;
			}
			else if(State == Pause | State == Setting | State == Standby)
			{
				State = Running;
			}
			break;				
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
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	I2CInit();
	
	LCD_Init();
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
	sprintf((char*)str,"  NO %d       ",Flag.EEPROM);
	LCD_DisplayStringLine(Line1,str);

	T.hour =  EEPROM_Read(0x00);
	T.minite = EEPROM_Read(0x01);
	T.second = EEPROM_Read(0x02);

	printf("USART OK!\r\n");
	HAL_TIM_Base_Start_IT(&htim1);
	
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		if(B1)
		{
			HAL_Delay(10);
			while(B1);
			printf("B1_Down!\r\n");
			if(++Flag.EEPROM == 6)
				Flag.EEPROM = 1;
			
			EEPROM_R();
			sprintf((char*)str,"  NO %d       ",Flag.EEPROM);
			LCD_DisplayStringLine(Line1,str);
		}
	
		EEPROM_W();
		LCD_Show();
    /* USER CODE END WHILE */

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
