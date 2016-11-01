/**
 *	@file   delay.c
 *	@brief	
 *	@author luisfynn <tani223@pinetron.com>
 *	@date   2015/03/30 09:43
 */

/* system include */
/* local include */
//#include "FreeRTOS.h"
#include "stm32f10x.h"

void TimerInit(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);			//TIM2 clock enable

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;					/* Enable the TIM2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 14;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//Time base configuration
	//time of 1 clock count = (Period * Prescaler) / (System Core Clock)
	//(10000*72)/72000000 = 0.01(10mS)
	TIM_TimeBaseStructure.TIM_Period = 10000-1;		// Interrupt occured, every 10000 Clock count. 
	TIM_TimeBaseStructure.TIM_Prescaler = 72-1; 	// Timer clock setting. Up to 1s, Clock Count need to 1,000,000(72/System Clock(=72Mhz)) 	
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);		/* Enable TIM2 Update interrupt */
	//	TIM_Cmd(TIM2, ENABLE);						// TIM2 enable counter			
}

void TIM2_ISR(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

void Delay5US(unsigned int TimerValue)
{
	TIM_Cmd(TIM2, ENABLE);

	TIM2->CNT = 0;	

	while(TIM2->CNT < 4*TimerValue)
	{
	}

	TIM_Cmd(TIM2, DISABLE);
}

void Delay1MS(unsigned int TimerValue)
{
	TIM_Cmd(TIM2, ENABLE);

	unsigned int i,j;

	for(j=0; j< TimerValue; j++)
	{
		for(i=0; i< 200; i++)
		{
			Delay5US(1);
		}
	}
	TIM_Cmd(TIM2, DISABLE);
}

