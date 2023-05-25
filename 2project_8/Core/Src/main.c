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
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "lcd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
	uint8_t B_want;
	uint8_t B_now;
	uint8_t B[6];
	uint8_t B_buf[6];
	uint8_t B_going;
	uint8_t B_Start;

}BX_TypeDef;

BX_TypeDef BX;

typedef struct
{
	uint16_t Key;
	uint16_t Led;
	uint16_t Door;
	uint16_t UD;
	uint16_t Wait;

}Time_TypeDef;

Time_TypeDef T;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define B1 !HAL_GPIO_ReadPin(GPIOB,B1_Pin)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t ShowTime[16];
uint8_t ShowDate[16];
uint8_t str[30];
uint16_t Time = 0;
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

//结构体初始化
void BX_Init(void)
{
	BX.B[0] = 0;
	BX.B[1] = 0;
	BX.B[2] = 0;
	BX.B[3] = 0;
	
	BX.B_buf[0] = 0;
	BX.B_buf[1] = 0;
	BX.B_buf[2] = 0;
	BX.B_buf[3] = 0;
	
	BX.B_now = 1;
	BX.B_want = 0;
	BX.B_going = 0;//电梯是否正在运行
	BX.B_Start = 0;//各状态标志位
}

void T_init(void)
{
	T.Key = 100;
	T.Led = 25;
	T.Door = 400;
	T.UD = 600;
	T.Wait = 200;
}

//RTC显示函数
void RTC_Show(uint8_t* showtime)
{
	RTC_TimeTypeDef T;
	RTC_DateTypeDef D;
	
	HAL_RTC_GetTime(&hrtc,&T,RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc,&D,RTC_FORMAT_BIN);
	
	sprintf((char*)showtime,"     %d: %d: %d      ",T.Hours,T.Minutes,T.Seconds);
	LCD_DisplayStringLine(Line7,(uint8_t*)ShowTime);
}

//LCD刷新所在楼层
void LCD_Show(void)
{
	if(BX.B_now)
	{
		sprintf((char*)str,"         %d       ",BX.B_now);
		LCD_DisplayStringLine(Line4,str);
	}
	else
	{
		sprintf((char*)str,"                     ");
		LCD_DisplayStringLine(Line4,str);
	}
}

//绝对值函数
int rabs(int data) 
{
	if(data > 0)
		return data;
	else
		return -data;
}

//开关门函数
void Door(uint8_t data)//开门为1，关门为0
{
	if(data)
	{
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);
		__HAL_TIM_SetCompare(&htim17,TIM_CHANNEL_1,600);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
		__HAL_TIM_SetCompare(&htim17,TIM_CHANNEL_1,500);
	}
}

//上下行函数
void UpDown(uint8_t data)//上行为1，下行为0
{
	if(data)
	{
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);
		__HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_1,800);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);
		__HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_1,600);
	}
}

//关闭所有LED
void LED_Close(void)
{
	HAL_GPIO_WritePin(GPIOC,LED1_Pin|LED2_Pin|LED3_Pin|LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin|LED8_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}
//要去哪个楼层，点亮哪个LED
void B_LED(void)
{
	LED_Close();
	for(int i = 0;i < 4;i++)
	{
		if(BX.B[i])
		{
			HAL_GPIO_WritePin(GPIOC,(1<<(8 + i)),GPIO_PIN_RESET);
		}
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
	}
}
//LED闪烁函数
uint8_t Flag_LED = 0;
void LED_BL(void)
{
	switch (BX.B_going)
	{
		//上行流水灯
		case 1:
			HAL_GPIO_WritePin(GPIOC,1<<(12 + Flag_LED),GPIO_PIN_SET);//关闭上一个灯
//			for(int i = 0;i < 4;i++)
//			{
//				HAL_GPIO_WritePin(GPIOC,(1<<(12 + i)),GPIO_PIN_SET);
//			}
			if(++Flag_LED == 4)//循环点亮
			{
				Flag_LED = 0;
			}
			HAL_GPIO_WritePin(GPIOC,(1<<(12 + Flag_LED)),GPIO_PIN_RESET);
			break;
		case 2:
//			HAL_GPIO_WritePin(GPIOC,1<<(12 + Flag_LED),GPIO_PIN_SET);
			for(int i = 0;i < 4;i++)
			{
				HAL_GPIO_WritePin(GPIOC,(1<<(12 + i)),GPIO_PIN_SET);
			}

			if(--Flag_LED == 255)//倒过来循环点亮
			{
				Flag_LED = 3;
			}
			HAL_GPIO_WritePin(GPIOC,(1<<(12 + Flag_LED)),GPIO_PIN_RESET);
			break;
	}
	
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

//计算想去的楼层
void Cal_want(void)
{
	BX.B_want = 0;

	for(int i = 0;i < 4;i++)
	{
		if(BX.B[i])
		{
			BX.B_buf[i] = (uint8_t)(BX.B[i] - BX.B_now);
//		printf("BX.B_buf[i]: %d\r\n",BX.B_buf[i]);
		}
	}
	
	uint16_t num = 300;
	uint8_t index = 0;
	
	for(int i = 0;i < 4;i++)
	{
		if((num > BX.B_buf[i]) && BX.B_buf[i] && (BX.B_buf[i] < 200))
		{
			index = BX.B[i];
			num = BX.B_buf[i];
		}
	}
	
	BX.B_want = index;
	num = 0;
	index = 0;
	
	if(!BX.B_want)
	{
		for(int i = 0;i< 4;i++)
		{
			if(num < BX.B_buf[i])
			{
				index = BX.B[i];
				num = BX.B_buf[i];
			}
		}
		
		BX.B_want = index;
		printf("Want goto: %d\r\n",BX.B_want);
	}
	
	BX.B_buf[BX.B_want - 1] = 0;
}

#define Up 1
#define Down 0
#define Open 1
#define Close 0
//定时器中断回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == htim1.Instance)
	{
		switch (BX.B_Start)
		{
			//第一步判断最后一次按键是否按下超过一秒
			case 1:
				if(--Time == 0)
				{
					Cal_want();
					printf("goto: %d\r\n",BX.B_want);
					BX.B_Start = 2;
					Time = T.Door;
					Door(Close);
					printf("Door Closing...\r\n");
				}
				break;
				//第二步关门
			case 2:
				if(--Time == 0)
				{
					BX.B_Start = 3;
					Time = 0;
				}
				//第三步计算上下行
				break;
			case 3:
				if(BX.B_want > BX.B_now)
				{
					printf("Uping...\r\n");
					UpDown(Up);
					BX.B_going = 1;
				}else
				{
					printf("Downing...\r\n");
					UpDown(Down);
					BX.B_going = 2;
				}
				BX.B_Start = 4;
				Time = T.UD * rabs((BX.B_now-BX.B_want));
				break;
				//第四步上下行，上下行过程中有流水灯
			case 4:
				if((--Time % T.UD) == 0)
				{
					if(BX.B_want > BX.B_now) BX.B_now++;
					if(BX.B_want < BX.B_now) BX.B_now--;
					if(BX.B_want == BX.B_now) 
					{
						BX.B_Start = 5;
						Time = T.Led;
						
						BX.B_now = BX.B_want;
						BX.B[BX.B_now-1] = 0;
						printf("arive: %d\r\n",BX.B_now);
						
						Door(Open);
						printf("Door Opnning...\r\n");
						
						BX.B_going = 0;
					}
				}
				break;
				//第五步，到达的楼层闪两下
			case 5:
				if(--Time == 0)
				{
					BX.B_Start = 6;
					Time = T.Led;
					BX.B_now = 0;//灭
				}
				break;
			case 6:
				if(--Time == 0)
				{
					BX.B_Start = 7;
					Time = T.Led;
					BX.B_now = BX.B_want;//亮
				}
				break;
			case 7:
				if(--Time == 0)
				{
					BX.B_Start = 8;
					Time = T.Led;
					BX.B_now = 0;//灭
				}
				break;
			case 8:
				if(--Time == 0)
				{
					BX.B_Start = 9;
					Time = T.Door - 100;
					BX.B_now = BX.B_want;//亮
				}
				break;
				//第九步，开门
			case 9:
				if(--Time == 0)
				{
					BX.B_Start = 10;
					Time = 0;	
				}
				break;
				//第十步，计算是否还有下一个目标层
			case 10:
				Cal_want();
				if(!BX.B_want)
				{
					printf("Over!\r\n");
					Time = 0;
					BX.B_Start = 0;
				}
				else
				{
					printf("goto: %d\r\n",BX.B_want);
					Time = T.Wait;
					BX.B_Start = 11;
				}
				break;
			//第十一步，等待两秒后关门起步
			case 11:
				if(--Time == 0)
				{
					BX.B_Start = 2;
					Time = T.Door;
					
					Door(Close);
					printf("Door Closing\r\n");
				}
				break;
		}

		B_LED();
		
		if(BX.B_going)
		{
			if(--T.Led == 0)
			{
				LED_BL();
				T.Led = 25;
			}
		}
		else
		{
			HAL_GPIO_WritePin(GPIOC,(1<<(12 + Flag_LED)),GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
		}
		
	}
	
	
	
}

//按键中断回调函数
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(BX.B_Start < 2)
		{	
			switch (GPIO_Pin)
			{
				case B2_Pin:
					printf("B2_Down!\r\n");
					switch (BX.B_now)
					{
						case 1:
							BX.B_Start = 1;
							BX.B[1] = 2;
							Time = T.Key;
							break;
						case 2:
							break;
						case 3:
							BX.B_Start = 1;
							BX.B[1] = 2;
							Time = T.Key;
							break;
						case 4:
							BX.B_Start = 1;
							BX.B[1] = 2;
							Time = T.Key;
							break;	
					}

					break;
				case B3_Pin:
					printf("B3_Down!\r\n");
					switch (BX.B_now)
					{
						case 1:
							BX.B_Start = 1;
							BX.B[2] = 3;
							Time = T.Key;
							break;
						case 2:
							BX.B_Start = 1;
							BX.B[2] = 3;
							Time = T.Key;
							break;
						case 3:
							break;
						case 4:
							BX.B_Start = 1;
							BX.B[2] = 3;
							Time = T.Key;
							break;	
					}

					break;
				case B4_Pin:
					printf("B4_Down!\r\n");
					switch (BX.B_now)
					{
						case 1:
							BX.B_Start = 1;
							BX.B[3] = 4;
							Time = T.Key;
							break;
						case 2:
							BX.B_Start = 1;
							BX.B[3] = 4;
							Time = T.Key;
							break;
						case 3:
							BX.B_Start = 1;
							BX.B[3] = 4;
							Time = T.Key;
							break;
						case 4:
							break;	
					}
					break;
			}	
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
  MX_RTC_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM17_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

	LCD_Init();
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
	LCD_DisplayStringLine(Line1,(uint8_t*)"        NOW        " );
	
	BX_Init();
	T_init();
	
	LED_Close();
	
	printf("USART OK!\r\n");
	
	HAL_TIM_Base_Start_IT(&htim1);
	
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
	__HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_1,800);
	HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);
	__HAL_TIM_SetCompare(&htim17,TIM_CHANNEL_1,600);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		if(BX.B_Start < 2)
		{
			if(B1)
			{
				printf("B1_Down!\r\n");
				while(B1);
				switch (BX.B_now)
				{
					case 1:
						break;
					case 2:
						BX.B_Start = 1;
						BX.B[0] = 1;
						Time = T.Key;
						break;
					case 3:
						BX.B_Start = 1;
						BX.B[0] = 1;
						Time = T.Key;
					break;
					case 4:
						BX.B_Start = 1;
						BX.B[0] = 1;
						Time = T.Key;
						break;	
				}
			}
		}
		LCD_Show();
		RTC_Show(ShowTime);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;

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
