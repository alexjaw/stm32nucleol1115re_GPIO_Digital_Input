/**
  ******************************************************************************
  * file    main.c
  * author  A.Jaworowski
  *
  * version V1.1
  * date	2015-01-18
  * brief	Measure the time that the button is pressed.
  * A timer is started when the button is pressed and stopped when released.
  *
  * version V1.0
  * date	2015-01-18
  * brief	The very first example of digital input - blue button digital in.
  * The detection of the button (on PC13) is performed by connecting an interrupt
  * to the button I/O. Interrupts for GPIO are configured with EXTI and NVIC.
  * The interrupts are handled by handlers, it this case EXTI15_10_IRQHandler(),
  * in stm32l1xx_it.c
  *
  * Issues:
  * - TIM3_IRQHandler does not work - the interrupt is never activated - why?
  *
  * - ref: the ST-code in Utilities\STM32L152_Nucleo: STM_EVAL_PBInit()
  *
  *
  * Debugging is done using ST-Link and SWV.
 *
  ******************************************************************************
  * @file    main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11-February-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h> //printf()

/** @addtogroup IO_Toggle
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO uint32_t TimingDelay;
uint8_t BlinkSpeed = 0;
uint8_t bitInfo;
uint8_t flagButtonPressed, timerSeconds;
/* Private function prototypes -----------------------------------------------*/
RCC_ClocksTypeDef RCC_Clocks;
/* Private functions ---------------------------------------------------------*/

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */

void TIM3_IRQHandler(void)
{
	//Timer overflowed before pulse ended (Update interrupt)
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		//Clear the pending interrupt bit
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

		//Set the TimerOverflow flag
		timerSeconds++;
	}
}

int main(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct, TIM6_TimeBaseInitStruct;

	/* SysTick end of count event each 1ms */
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

	/* Enable PC13, the blue button pin*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE); //Bus connection GPIO and MCU
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); //Why? Bus connection EXTI and MCU?
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL; //Why?
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Configure external interrupt. Blue button EXTI line */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13); //SYSCFG: System Configuration Controller
    EXTI_InitStructure.EXTI_Line = EXTI_Line13;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set interrupt controller NVIC for Button. lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Configure timer, Timing mode*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    TIM_TimeBaseInitStruct.TIM_Prescaler = 32000;		//1000 Hz, i.e. timer is updated every 1ms.
    TIM_TimeBaseInitStruct.TIM_Period = 999;			//Reload after 1s
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    //TIM_TimeBaseInitStruct.TIM_ClockDivision;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);

    //Enable TIM3 interrupts
    //DOES NOT WORK
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    // Configure 10s timer, TIM6, timing mode
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
    TIM6_TimeBaseInitStruct.TIM_Prescaler = 32000;		//1000 Hz, i.e. timer is updated every 1ms.
    //TIM6_TimeBaseInitStruct.TIM_Period;
    TIM6_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    //TIM6_TimeBaseInitStruct.TIM_ClockDivision;
    TIM_TimeBaseInit(TIM6, &TIM6_TimeBaseInitStruct);
  
	/* Infinite loop */
    uint16_t pulseTime = 0;
    uint16_t extraTime = 0;
    TIM_SetCounter(TIM3, 0);
	while (1)
	{
		//TEST of timer TIM3
//		TIM_SetCounter(TIM3, 0);
//		TIM_Cmd(TIM3, ENABLE);
//		Delay(1000);
//		//TIM_GenerateEvent(TIM3, TIM_EventSource_Update);
//		TIM_Cmd(TIM3, DISABLE);
//		pulseTime = TIM_GetCounter(TIM3);
//		printf("Time: %d\n", pulseTime);
//		if (flagTimerOverflow)
//			printf("WARNING: flagTimerOverflow\n");
		//END TEST

		//TEST of timer with button
//		flagButtonPressed = 0;
//		EXTI_GenerateSWInterrupt(EXTI_Line13);
//		flagButtonPressed;
//		if (bitInfo == 1)		//first 1
//		{
//			bitInfo = 0;
//		}
//		else
//		{
//			bitInfo = 1;
//			Delay(1234);
//		}
		//END of test

		if (flagTimerOverflow)
		{
			printf("WARNING: flagTimerOverflow\n");
			flagTimerOverflow = 0;
			extraTime += 5000;		//!!!!!!!!!!!Exactly when do we get overflow
		}

		//Test for button and eventual bounce (50 ms)
		//However, when timer = 0, step into the condition
		if (flagButtonPressed && ((TIM_GetCounter(TIM3) > 50) || (TIM_GetCounter(TIM3) == 0)))
		{
			flagButtonPressed = 0;
			if (bitInfo == 0)		//pushed
			{
				//Set timer to 0
				TIM_SetCounter(TIM3, 0);
				//Start timer
				TIM_Cmd(TIM3, ENABLE);
			} else				//released
			{
				//Stop timer
				TIM_Cmd(TIM3, DISABLE);
				pulseTime += TIM_GetCounter(TIM3);
				printf("Time: %d\n", pulseTime);
				TIM_SetCounter(TIM3, 0);
			}
			//printf("Interrupt received from button pin. Bitstatus: %d\n", bitInfo);
		}
	}
}

/**
* @brief  Inserts a delay time.
* @param  nTime: specifies the delay time length, in 1 ms.
* @retval None
*/
void Delay(__IO uint32_t nTime)
{
  TimingDelay = nTime;
  
  while(TimingDelay != 0);
}

/**
* @brief  Decrements the TimingDelay variable.
* @param  None
* @retval None
*/
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

#ifdef  USE_FULL_ASSERT

/**
* @brief  Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param  file: pointer to the source file name
* @param  line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
* @}
*/


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
