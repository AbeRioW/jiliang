/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "ds18b20.h"
#include "max30100.h"
#include "stdio.h"
#include "stdbool.h"
#include "dht11.h"
#include "hx711.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
MAX30100_HandleTypeDef hmax;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint16_t ir_buffer[BUFFER_SIZE];
uint16_t red_buffer[BUFFER_SIZE];
uint32_t buffer_index = 0;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
HX711 hx711;

static void ui_setting(void);

int timer_set=60;
int hr_set=120;
float num_d;
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

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	bool come_with = false;

	float weight = 0;
	uint16_t ir_value, red_value;
    uint8_t heart_rate = 0;
    uint8_t spo2 = 0;  // In a real application, you would calculate this
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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
		OLED_Init();
	OLED_ColorTurn(0);
  OLED_DisplayTurn(0);
	OLED_Clear();
	//初始化HX711
	HX711_Init(&hx711, GPIOB, GPIO_PIN_4, GPIOB, GPIO_PIN_3);
	
	//HZ711去毛重
	HX711_Tare(&hx711, 10);
	
	HX711_SetScale(&hx711, 1000.0f);
	
	DS_Init();
	    if(MAX30100_Init(&hmax, &hi2c1, GPIOB, GPIO_PIN_5) != HAL_OK)
			{
					OLED_ShowString(0,0,(uint8_t*)"hello",8,1);
					OLED_Refresh();
			}
			
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if(HAL_GPIO_ReadPin(hmax.int_port, hmax.int_pin) == GPIO_PIN_RESET) {
        // Read FIFO until empty
        while(MAX30100_ReadFIFO(&hmax, &ir_value, &red_value) == HAL_OK) {
             // Apply moving average filter
             ir_buffer[buffer_index] = moving_average(&ir_value, FILTER_WINDOW, buffer_index);
//           red_buffer[buffer_index] = moving_average(red_value, FILTER_WINDOW, buffer_index);
                
             buffer_index++;
              if(buffer_index >= BUFFER_SIZE) {
                    // Calculate heart rate when buffer is full
                    heart_rate = CalculateHeartRate(ir_buffer, BUFFER_SIZE);
                    
                    //calculate SpO2 here
                    // spo2 = calculate_spo2(ir_buffer, red_buffer, BUFFER_SIZE);
                    
                    // Reset buffer
                    buffer_index = 0;
                    
                    // Print results via UART
                    char msg[10];
                    sprintf(msg, "HR: %03d bpm\r\n", heart_rate);
										OLED_ShowString(0,0,(uint8_t*)msg,8,1);
								    OLED_Refresh();
								
										if((heart_rate>hr_set)&&(heart_rate<200))
										{
												xinlv_yichang();
											  uart1_send_messageXIN();
										}
									
										num_d = Get_DS_Temperature();		
										sprintf(msg, "temp:%.2f", num_d); 
										OLED_ShowString(0,10,(uint8_t*)msg,8,1);
										OLED_Refresh();
								    
								    DHT11_READ_DATA();
										
										weight = HX711_GetWeight(&hx711, 10);
										if(weight>0)
										{
											sprintf(msg, "HEIGHT:%.2f", weight); 
											OLED_ShowString(0,40,(uint8_t*)msg,8,1);
											OLED_Refresh();
											if(weight<10)
											{
													anshi_chiyao();
											}
										}
										
										if(send_messgae)
										{
											send_messgae = false;
											uart1_send_messageCALL();
										}
										
			if(botton ==RIGHT)
			{
				 botton = UNPRESS;
					OLED_Clear();
					ui_setting();
				  
			}
			
		

                }
            }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static void ui_setting(void)
{
	  int position = 0;
	char timer_show[20],show_hr[20];

		while(1)
		{
				  sprintf(timer_show,"Timer:%02d",timer_set);
		  sprintf(show_hr,"HR:%03d",hr_set);
		OLED_ShowString(0,0,(uint8_t*)timer_show,8,position==0?0:1);
		OLED_ShowString(0,10,(uint8_t*)show_hr,8,position==1?0:1);
		OLED_ShowString(0,20,(uint8_t*)"Back",8,position==2?0:1);
		OLED_Refresh();
				if(botton==LEFT)
				{
						botton = UNPRESS;
										HAL_Delay(300);
					  switch(position)
						{
							case 0:
								timer_set--;
							  if(timer_set<0)
										timer_set=90;
								break;
							case 1:
								hr_set--;
							  if(hr_set<40)
									 hr_set=150;
							break;
						}
				}
				
				if(botton==MIDLE)
				{
						botton = UNPRESS;
										HAL_Delay(300);
					  switch(position)
						{
							case 0:
								timer_set++;
							  if(timer_set>90)
										timer_set=0;
								break;
							case 1:
								hr_set++;
							  if(hr_set>150)
									 hr_set=40;
							break;
							case 2:
									return;
							break;
						}
				}
				
				if(botton==RIGHT)
				{
											botton = UNPRESS;
										HAL_Delay(300);
						position++;
					  if(position>2)
							position=0;
				}
		}
}


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
