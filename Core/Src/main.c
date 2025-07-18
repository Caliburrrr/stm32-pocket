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
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "oscilloscope.h"
#include "generate.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */


#define WAVE_LENGTH 256                // 采样点数
#define  adcvaluelength 3000



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

extern DMA_HandleTypeDef hdma_tim1_up;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


uint16_t adcvalue[adcvaluelength];

uint8_t exitflag=0;//产生按键中断标志
uint8_t enflag=1;//编码器标志
uint8_t oledwaveflag=1;//波形标志
uint8_t numflag=0;//参数刷新标志


float generate_voltage=3.3;//初始幅度
uint8_t freqpos=1;
uint16_t freq[11]={390,195,130,97,78,65,55,48,43,39,39062};//频率psc数组


uint8_t keynum;   //按键号
uint8_t key1_state;//按键按下次数
uint8_t key2_state=1;
uint8_t key5_state;


float nowfreq;

 uint16_t sin_wavedata[WAVE_LENGTH];   // 存储数据
 uint16_t sq_wavedata[WAVE_LENGTH];
 uint16_t tang_wavedata[WAVE_LENGTH];
 uint16_t dc_wavedata[WAVE_LENGTH];
 uint16_t *pwave=sin_wavedata;       //波形指针


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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */


  OLED_Init();


 Generate_init(generate_voltage);//峰峰值初始化


 HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcvalue, 3000);
 HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);//TIM2pwm触发adc

 HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);//TIM3编码器模式


 HAL_DMA_Start(&hdma_tim1_up, (uint32_t*)pwave, (uint32_t)&GPIOA->ODR, 256);
  __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE); // TIM1触发DMA
  HAL_TIM_Base_Start(&htim1);

  uint8_t x=1;
  uint8_t y=20;
  uint8_t tgv=26;



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	uint8_t count =__HAL_TIM_GET_COUNTER(&htim3);


 if(exitflag==1)
 {	switch(keynum)
	{
	case 1:
		switch(key1_state)//采样频率
		{
			case 1:
				__HAL_TIM_SET_PRESCALER(&htim2,2999);
				HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
				HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
				break;
			case 2:
				__HAL_TIM_SET_PRESCALER(&htim2,19);
				HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
				HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
				break;
			case 3:
				__HAL_TIM_SET_PRESCALER(&htim2,49);
				HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
				HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
				break;
		}

		break;
	case 2:
		switch(key2_state)//水平,垂直,触发
		{
			case 1:enflag=1;
				   break;
			case 2:enflag=2;
				   break;
			case 3:enflag=3;
				   break;
		}
	     break;
	 case 3://幅度
	 {
		 HAL_TIM_Base_Stop(&htim1);
		 HAL_DMA_Abort(&hdma_tim1_up);
		 generate_voltage-=0.3;
		 if(generate_voltage<0.9)
		 {
			 generate_voltage=3.3;
		 }
		 Generate_init(generate_voltage);
		 HAL_DMA_Start(&hdma_tim1_up, (uint32_t*)pwave, (uint32_t)&GPIOA->ODR, 256);
		 HAL_TIM_Base_Start(&htim1);
	 }
	     break;
	 case 4://频率
	 {

		  HAL_TIM_Base_Stop(&htim1);
		  htim1.Instance->PSC=freq[freqpos];
		  HAL_TIM_Base_Start(&htim1);
		  freqpos++;
		  if(freqpos>10)
		  {
			  freqpos=0;
		  }

	 }
	     break;
	 case 5://波形指针
		 switch(key5_state)
			 {
      	 	 case 1://方波

					HAL_TIM_Base_Stop(&htim1);
					HAL_DMA_Abort(&hdma_tim1_up);
					pwave=sq_wavedata;
					HAL_DMA_Start(&hdma_tim1_up, (uint32_t*)pwave, (uint32_t)&GPIOA->ODR, 256);
					HAL_TIM_Base_Start(&htim1);
					oledwaveflag=2;
					break;
			 case 2://三角
					HAL_TIM_Base_Stop(&htim1);
					HAL_DMA_Abort(&hdma_tim1_up);
				    pwave=tang_wavedata;
					HAL_DMA_Start(&hdma_tim1_up, (uint32_t*)pwave, (uint32_t)&GPIOA->ODR, 256);
					HAL_TIM_Base_Start(&htim1);
					oledwaveflag=3;
					   break;
			 case 3://DC
					HAL_TIM_Base_Stop(&htim1);
					HAL_DMA_Abort(&hdma_tim1_up);
					pwave=dc_wavedata;
					HAL_DMA_Start(&hdma_tim1_up, (uint32_t*)pwave, (uint32_t)&GPIOA->ODR, 256);
					HAL_TIM_Base_Start(&htim1);
					oledwaveflag=4;
					break;
			 case 4://正弦
					 HAL_TIM_Base_Stop(&htim1);
					 HAL_DMA_Abort(&hdma_tim1_up);
					 pwave=sin_wavedata;
					 HAL_DMA_Start(&hdma_tim1_up, (uint32_t*)pwave, (uint32_t)&GPIOA->ODR, 256);
					 HAL_TIM_Base_Start(&htim1);
					 oledwaveflag=1;
					 break;
			 }
	     break;
	}
		 keynum=0;
		 exitflag=0;
 }


	switch(enflag)//获取水平,垂直,触发
	{
	case 1: x=count>20?20:count;
            OLED_ShowString(100, 7, "X", 12);
			numflag=3;
	         break;
	case 2: y=count;
	        OLED_ShowString(100, 7, "Y", 12);
			numflag=3;
            break;
	case 3: tgv=count;
	        OLED_ShowString(100, 7, "TGV", 12);
	        float realtgv=(tgv-26)*1.27/y+1.65;
	        if(realtgv>=3.3||realtgv<=0)
	        {
	        	realtgv=0;
	        	OLED_ShowNum(100, 6, realtgv, 1, 12);
	        }
	        else
	        {
		        OLED_ShowFloat(100, 6, realtgv, 1, 12);//触发电压
	        }
	        OLED_DrawLine(88, 52-tgv, 91, 52-tgv, 1);//触发小短线
	        numflag=3;
	        break;
	}


    switch(oledwaveflag)//显示当前波形
    {
    case 1: OLED_ShowString(100, 1, "sin", 12);break;
    case 2:	OLED_ShowString(100, 1, "sq", 12);break;
    case 3: OLED_ShowString(100, 1, "tang", 12);break;
    case 4: OLED_ShowString(100, 1, "dc", 12);break;
    }

    uint16_t sample=htim2.Instance->PSC;//显示采样率模式
	switch(sample)
    {
	   case 19:OLED_ShowString(100, 2, "High", 12);break;
	   case 49:OLED_ShowString(100, 2, "Mid", 12);break;
	   case 2999:OLED_ShowString(100, 2 , "Low", 12);break;
	}


    if(oledwaveflag==4)//更新测量幅度
    {
    	uint16_t nowadcvalue=HAL_ADC_GetValue(&hadc1);
    	float getdcvoltage= nowadcvalue/4094.9*3.3;
    	OLED_ShowFloat(0, 7, getdcvoltage, 1, 12);
    }
    else
    {
    	float getvoltage=get_amplitude();
    	OLED_ShowFloat(0, 7, getvoltage, 1, 12);
	}
    OLED_ShowChar(23, 7, 'V', 12);


    if(oledwaveflag==4)//更新频率
    {
        OLED_ShowNum(37, 7, 0, 4, 12);
    }
    else
    {
       OLED_ShowNum(37, 7,nowfreq+1, 4, 12);
    }
    OLED_ShowString(61, 7, "Hz", 12);



	OLED_ShowString(100, 0, "Y-T", 12);
    OLED_grid();



	adcvalue_change_waveform(x,y);// 将 ADC 数据转换为波形数据
	uint8_t tgpos = get_triggervoltage_pos(tgv);// 获取触发位置
	oledshow_waveform(tgpos);//画波形



	OLED_Update_Screen();

	 //参数局部刷新
	 switch(numflag)
	{
		case 1:OLED_Clear_num_1();
			   numflag=0;
			   break;
		case 2:OLED_Clear_num_2();
			   numflag=0;
			   break;
		case 3:OLED_Clear_num_3();
			   OLED_Clear_num_4();
			   numflag=0;
			   break;
	}
    //波形刷新
	OLED_Clear_wave();



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
  RCC_OscInitStruct.PLL.PLLN = 100;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

	exitflag=1;
	HAL_Delay(100);
if(keynum==0)
{
    switch (GPIO_Pin)
    {
        case GPIO_PIN_8:
            keynum=1;
            key1_state++;
            if(key1_state>3)
			{
				key1_state=1;
			}
			numflag=2;
            break;
        case GPIO_PIN_9:
        	keynum=2;
        	key2_state++;
			if(key2_state>3)
			{
				key2_state=1;
			}
            break;
        case GPIO_PIN_12:
        	keynum=3;
             break;
        case GPIO_PIN_13:
        	keynum=4;
             break;
        case GPIO_PIN_14:
        	keynum=5;
        	key5_state++;
			if(key5_state>4)
			{
				key5_state=1;
			}
			numflag=1;
             break;
    }
 }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)//ADC传输完成计算
{
    if (hadc->Instance == ADC1)
    {
        fftdata();
        getfrequency(&nowfreq);
        HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcvalue, 3000);
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
