/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_conf.h 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Library configuration file.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F10x_CONF_H
#define __STM32F10x_CONF_H

/* Includes ------------------------------------------------------------------*/
/* Uncomment/Comment the line below to enable/disable peripheral header file inclusion */
#include "stm32f10x_adc.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_can.h"
#include "stm32f10x_cec.h"
#include "stm32f10x_crc.h"
#include "stm32f10x_dac.h"
#include "stm32f10x_dbgmcu.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_fsmc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_iwdg.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_sdio.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_wwdg.h"
#include "stm32f10x_systick.h"
#include "misc.h" /* High level functions for NVIC and SysTick (add-on to CMSIS functions) */

#include "video.h"

/*========================================================================================*/
/* AHD VIDEO converter Define-------------------------------------------------------------*/
// task priority 
#define UART_TASK_PRIORITY     		( tskIDLE_PRIORITY + 2 )		
#define VIDEO_PROCESS_TASK_PRIORITY   	( tskIDLE_PRIORITY + 2 )		
#define AHD_COAXIAL_TASK_PRIORITY   	( tskIDLE_PRIORITY + 2 )		

// delay in task
#define TIM2_ISR		TIM2_IRQHandler

#define		TEST_MODE2(x)	(x)? GPIO_SetBits(GPIOB, GPIO_Pin_15): GPIO_ResetBits(GPIOB, GPIO_Pin_15)	
#define		TEST_MODE1(x)	(x)? GPIO_SetBits(GPIOB, GPIO_Pin_14): GPIO_ResetBits(GPIOB, GPIO_Pin_14)
#define		GET_MDIN_IRQ()	GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0)

#define	ANALOG	0	
#define HDMI	1	

#define	Composite	0
#define	AHD			1

#define TX_AHD		0
#define TX_FAHD		1	

#ifndef FALSE
#define FALSE 	0
#endif

#ifndef TRUE
#define	TRUE	1
#endif

#ifndef	ON 
#define ON 		1	
#endif

#ifndef	OFF 
#define	OFF		0	
#endif

#define	MCUINIT					0	
#define	MODE_TYPE				1	
#define	RX_TYPE					2	
#define	TX_TYPE					3	

#define	REVERSE				0 
#define FORWARD				1		

#define	REPEATER			1	
#define	CONVERTER			0	

#define	isRepeater	(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4))? 	REPEATER : CONVERTER 
#define	isHDMI_IN	(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5))? 	HDMI : ANALOG
#define	isFAHD_OUT	(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6))? 	TX_FAHD : TX_AHD 
#define	isPAL		(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7))? 	PAL : NTSC 

//#define 	AHD_DEBUG				//Use if Need Debug 

/*AHD VIDEO converter function------------------------------------------------------------*/

/*AHD VIDEO converter variable ===========================================================*/
/*main*/
static ErrorStatus HSEStartUpStatus;
void LedControl(unsigned char DeviceInitStatus, unsigned char Direction);
/*========================================================================================*/



/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT

/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function which reports 
  *         the name of the source file and the source line number of the call 
  *         that failed. If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */

#endif /* __STM32F10x_CONF_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
