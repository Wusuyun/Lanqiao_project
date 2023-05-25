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
#include "string.h"
#include "lcd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
	char brand;
	char id[5];
	uint32_t hour;
	uint8_t empty;
}stu;

stu car[8];

typedef struct
{
	char brand;
	char id[5];
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t mm;
	uint8_t ss;
	uint32_t totalhour;
	
}cartypedef;

cartypedef buf;

typedef struct 
{
	float CNBR;
	float VNBR;
	float total;
	
}Feetypedef;

Feetypedef Fee = {3.5,2.0,0};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define B1 !HAL_GPIO_ReadPin(GPIOB,B1_Pin)
#define B2 !HAL_GPIO_ReadPin(GPIOB,B2_Pin)
#define B3 !HAL_GPIO_ReadPin(GPIOB,B3_Pin)
#define B4 !HAL_GPIO_ReadPin(GPIOA,B4_Pin)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char RX[22] = {0};//���յ��ַ�
uint8_t str[50];
uint8_t emp = 0;//һ���м������г�λ
uint8_t cnbr = 0;
uint8_t vnbr = 0;
uint8_t LCD_mode = 0;
uint8_t Flag_B4 = 0;
uint32_t totalbuf = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//�ض�����
int fputc(int ch, FILE *stream)
{
	while(!(USART1->ISR & (1<<7)));
	USART1->TDR = (uint8_t)ch;
	return ch;
}

//LEDָʾ����
void LED_Show(void)
{
	if(emp != 0)
	{
		HAL_GPIO_WritePin(GPIOC,LED1_Pin,GPIO_PIN_RESET);
	}else HAL_GPIO_WritePin(GPIOC,LED1_Pin,GPIO_PIN_SET);
	
	if(Flag_B4)
	{
		HAL_GPIO_WritePin(GPIOC,LED2_Pin,GPIO_PIN_RESET);
	}else HAL_GPIO_WritePin(GPIOC,LED2_Pin,GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(GPIOC,LED3_Pin|LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin|LED8_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}
//�ṹ���ʼ������
void carinit(void)
{
	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 4 ; j++)
		{
			car[i].brand = 0;
			car[i].id[j] = 0;
			car[i].hour = 0;
			car[i].empty = 0;
		}
	}
}

//Ѱ��һ�����г�λ
uint8_t findlocat(void)
{
	for(int i = 0; i < 8 ; i++)
	{
		if(car[i].empty == 0)
		{
			return i;
		}
	}return 0xFF;
}

//�ж��������Ƿ��Ѿ�����
uint8_t isExist(char* str)
{
	for(int i = 0; i < 8; i++)
	{
		if(strncmp((const char*)str,(const char*)car[i].id,4) == 0)
		{
			return i;//������ڷ��س�λ
		}
	}return 0xFF;
}


//�����м������г�λ
void Empty(void)
{
	for(int i = 0 ; i<8 ; i++)
	{
		if(car[i].empty  == 0)
		{
			emp++;
		}else if(car[i].empty == 1)
		{
			emp--;
		}
	}
}

//�ַ������ּ�麯��
uint8_t check(char* str)
{
	if((str[0] != 'C' && str[0] != 'V') || str[1] != 'N' || str[2] != 'B' || str[3] != 'R')
	{
		for(int i = 10; i <= 22; i++)
		{
			if(str[i] > '9' || str[i] < '0')
			{
				return 0;
			}
		}
	}else return 1;		
}

//���ڻص�����
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(emp != 0)
	{
		//����ȡ��Ϣ
		if((check(RX)) == 1)
		{
			char* rx = RX;
			buf.brand = RX[0];
			printf("buf.brand:%d\r\n",buf.brand);
			rx = strchr(RX,':');
			rx++;
			strncpy(buf.id,rx,4);
			printf("buf.id: %s\r\n",buf.id);
			rx = strchr(rx,':');
			rx++;
			buf.year = ((RX[10]-48)*10) + (RX[11]-48); 
			buf.month = ((RX[12]-48)*10) + (RX[13]-48);
			buf.day = ((RX[14]-48)*10) + (RX[15]-48);
			buf.hour = ((RX[16]-48)*10) + (RX[17]-48);
			buf.mm = ((RX[18]-48)*10) + (RX[19]-48);
			buf.ss = ((RX[20]-48)*10)+ (RX[21]-48);
			buf.totalhour = (buf.year*365*24) + (buf.month*30*24) + (buf.day*24) + (buf.hour) + (buf.mm || buf.ss);
			printf("buf.totalhour:%d\r\n",buf.totalhour);
			
			//�ٴμ��ʱ���ʽ�Ƿ���ȷ
			if((buf.year>99) || (buf.month>12) || (buf.day>31) || (buf.hour>60) || (buf.mm>60) || (buf.ss>60))
			{
				printf("errow_Time!\r\n");
			}
			
			else
			{
				if(isExist(buf.id) != 0xFF)//����Ѿ����������
				{
					if(buf.brand == 'C')
					{
						totalbuf = (buf.totalhour - car[isExist(buf.id)].hour);
						printf("buf.totalhour:%d\r\n",buf.totalhour);
						printf("car[%d].hour:%d\r\n",isExist(buf.id),car[isExist(buf.id)].hour);
						printf("totalbuf: %d\r\n",totalbuf);
						Fee.total = totalbuf * Fee.CNBR;
						printf("CNBR:%s:%d:%0.2f\r\n",buf.id,totalbuf,Fee.total);
						cnbr--;
					}
					else if(buf.brand == 'V')
					{
						totalbuf = (buf.totalhour - car[isExist(buf.id)].hour);
						Fee.total = totalbuf * Fee.VNBR;
						printf("VNBR:%s:%d:%0.2f\r\n",buf.id,totalbuf,Fee.total);
						vnbr--;
					}
					car[isExist(buf.id)].empty = 0;
					car[isExist(buf.id)].brand = 0;
					car[isExist(buf.id)].hour = 0;
					for(int j = 0; j<4; j++)
					{
						car[isExist(buf.id)].id[j] = 0;
					}
					emp++;
					printf("Car_OUT\r\n");
				}
				
				else if(findlocat() != 0xFF)//�����������Ѱ�ҳ�λ
				{
					
					int i = findlocat();
					printf("i:%d\r\n",i);

					car[i].brand = buf.brand;
					strncpy(car[i].id,buf.id,4);
					car[i].empty = 1;
					car[i].hour = buf.totalhour;
					printf("car[%d].hour:%d\r\n",i,car[i].hour);
					if(buf.brand == 'C')
					{
						cnbr++;
					}
					else if(buf.brand == 'V')
					{
						vnbr++;
					}
					emp--;
					printf("Car_IN\r\n");
				}
				else printf("No locate!\r\n");
			}
					
		}else printf("errow_brand!\r\n");
	}
		HAL_UART_Receive_IT(&huart1,(uint8_t*)RX,22);
}

//LCD��ʾ����
void LCD1(void)
{
	LCD_DisplayStringLine(Line1,(uint8_t*)"       Data          ");
	
	sprintf((char*)str,"   CNBR:%d   ",cnbr);
	LCD_DisplayStringLine(Line3,str);
	
	sprintf((char*)str,"   VNBR:%d   ",vnbr);
	LCD_DisplayStringLine(Line5,str);
	
	sprintf((char*)str,"   IDLE:%d   ",emp);
	LCD_DisplayStringLine(Line7,str);
}

void LCD2(void)
{
	LCD_DisplayStringLine(Line1,(uint8_t*)"       Para          ");

	sprintf((char*)str,"   CNBR:%0.2f    ",Fee.CNBR);
	LCD_DisplayStringLine(Line3,str);

	sprintf((char*)str,"   VNBR:%0.2f     ",Fee.VNBR);
	LCD_DisplayStringLine(Line5,str);

	LCD_ClearLine(Line7);
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
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

	LCD_Init();
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
	Empty();
	carinit();
	
	printf("USART OK!\r\n");
	
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
	__HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_2,0);
		
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		
		HAL_UART_Receive_IT(&huart1,(uint8_t*)RX,22);

		if(B1)
		{
			printf("B1 Down\r\n");
			HAL_Delay(10);
			while(B1);
			LCD_mode = !LCD_mode;
		}
		
		if(B2)
		{
			printf("B2 Down\r\n");
			HAL_Delay(10);
			while(B2);
			if(LCD_mode)
			{
				Fee.CNBR += 0.5;
				Fee.VNBR += 0.5;
			}
		}
		
		if(B3)
		{
			printf("B3 Down\r\n");
			HAL_Delay(10);
			while(B3);
			if(LCD_mode)
			{
				if(Fee.VNBR == 0.5)
				{
					Fee.VNBR = 0.5;
				}
				else if(Fee.CNBR == 0.5)
				{
					Fee.CNBR = 0.5;
				}
				else
				{
					Fee.CNBR -= 0.5;
					Fee.VNBR -= 0.5;
				}
			}
		}
		
		if(B4)
		{
			printf("B4 Down\r\n");
			HAL_Delay(10);
			while(B4);
			
			Flag_B4 = !Flag_B4;
			switch (Flag_B4)
			{
				case 0:
					__HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_2,0);
					break;
				case 1:
					__HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_2,200);
					break;
			}
		}
		
		switch (LCD_mode)
		{
			case 0:
				LCD1();
				break;
			case 1:
				LCD2();
				break;
		}
		
		LED_Show();

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
