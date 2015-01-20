/**
  ******************************************************************************
  * file    main.c
  * author  A.Jaworowski
  *
  * version	v1.2
  * date	2015-01-20
  * brief	Can measure during 10s
  * This is made by making TIM3 as a 1000ms timer. Each time TIM3 reaches 1000ms,
  * its restarted and a counter is updated. Every 10000ms, the counter is set to 0.
  * Timing of the button is performed by reading TIM3 values.
  *
  * Issues: When the button is used, often I get time values from TIM3 > 1000, which
  * should not be possible since its auto reloaded every 1000 ms.
  *
  * Solutions: V1.1. The problem was that I never enable NVIC for TIM3, now its done.
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
uint8_t bitInfo, flagTimerRestarted = 0;
uint8_t flagButtonPressed, timerSeconds;
uint16_t myTimer = 0;
/* Private function prototypes -----------------------------------------------*/
RCC_ClocksTypeDef RCC_Clocks;
/* Private functions ---------------------------------------------------------*/

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */

/**
 * @brief	Increment myTimer every 1000ms, Restarts after 10s
 * @param	None
 * @note	-
 * @retval	None
 */
void TIM3_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	myTimer+=1000;
	if (myTimer >= 11000)
	{
		myTimer = 1000;
		flagTimerRestarted = 1;
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
    NVIC_EnableIRQ(TIM3_IRQn);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    // Configure 10s timer, TIM6, timing mode
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
//    TIM6_TimeBaseInitStruct.TIM_Prescaler = 32000;		//1000 Hz, i.e. timer is updated every 1ms.
//    //TIM6_TimeBaseInitStruct.TIM_Period;
//    TIM6_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
//    //TIM6_TimeBaseInitStruct.TIM_ClockDivision;
//    TIM_TimeBaseInit(TIM6, &TIM6_TimeBaseInitStruct);
  
	/* Infinite loop */
    uint16_t startTime = 0, endTime = 0, pulseTime = 0, totalTime = 0, counts = 0, tim3Time = 0;
    TIM_SetCounter(TIM3, 0);
    TIM_Cmd(TIM3, ENABLE);
	while (1)
	{
		//TEST of timer TIM3
		Delay(1500);
		tim3Time = TIM_GetCounter(TIM3);
		printf("TIM3 and Time: %d\t%d\n", tim3Time, myTimer);
		//END TEST

		//Test for button and eventual bounce (50 ms)
		//However, when timer = 0, step into the condition
		if (flagButtonPressed && ((TIM_GetCounter(TIM3) > 50) || (TIM_GetCounter(TIM3) == 0)))
		{
			flagButtonPressed = 0;
			if (bitInfo == 0)		//pushed
			{
				tim3Time = TIM_GetCounter(TIM3);
				startTime = myTimer + tim3Time;
			} else				//released
			{
				tim3Time = TIM_GetCounter(TIM3);
				endTime = myTimer + tim3Time;
				pulseTime = endTime - startTime;
				totalTime += pulseTime;
				counts++;
				printf("Pulse and total Time: %d\t%d\n", pulseTime, totalTime);
				endTime = 0;
				startTime = 0;
				pulseTime = 0;
			}
		}
		if (flagTimerRestarted)
		{
			flagTimerRestarted = 0;
			printf("\n---RESULT---\n");
			printf("Counts and Total Time: %d counts\t%d s\n", counts, totalTime/1000);
			totalTime = 0;
			counts = 0;
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
