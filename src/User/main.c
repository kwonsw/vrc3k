/**
 *	@file   main.c
 *	@brief  	
 *	@author luisfynn(tani223@pinetron.co.kr)
 *	@date   2015/03/16 09:34
 */

/* system include */
#include "stdlib.h"
#include "string.h"
/* local include */
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "xuart.h"
#include "video.h" 
#include "adv7611.h" 
#include "nvp6124.h" 
#include "mdin3xx.h" 

AHD_INFO *pvInfo = NULL;
static xTaskHandle xHandle1, xHandle2, xHandle3;	

/*Initial*/
static void device_init(AHD_INFO *pvInfo);
static void mcu_initialize( void );
static void power_control( unsigned char status );
static void device_reset( AHD_INFO *pvInfo );
static void video_mode_configuration(AHD_INFO *pvInfo);

/*FRTOS TASK*/
static xTaskHandle xHandle1, xHandle2, xHandle3;	

static void prvShellTask( void *pvParameters );
static void prvVideoProcessTask(void *pvParameters);
static void prvUartTask( void *pvParameters );
static void prvMdin380Task( void *pvParameters );
static void prvAHDCoaxialTask( void *pvParameters );

/*Video auto detection*/
static void check_video_config(AHD_INFO *pvInfo);

int main(void)
{
	pvInfo = (AHD_INFO*)malloc(sizeof(AHD_INFO));			/*Video info memory allocation*/

	if(pvInfo != NULL)		memset(pvInfo, 0, sizeof(AHD_INFO));
	else					NVIC_GenerateSystemReset();	

	mcu_initialize();											/*initialize hardware*/
	video_mode_configuration(pvInfo);
	device_init(pvInfo);									/*NVP6114 or ADV7611 INIT & AUTO detection */

	/*Task Create*/
	xTaskCreate( prvShellTask, "SHELL", configMINIMAL_STACK_SIZE + 256, NULL, UART_TASK_PRIORITY, &xHandle1 );
	xTaskCreate( prvVideoProcessTask, "Video resolution check", configMINIMAL_STACK_SIZE + 256, NULL, VIDEO_PROCESS_TASK_PRIORITY, &xHandle2 );
	xTaskCreate( prvAHDCoaxialTask, "UTC protocol", configMINIMAL_STACK_SIZE + 64, NULL, AHD_COAXIAL_TASK_PRIORITY, &xHandle3 );
	
	/* Starts the real time kernel tick processing  */
	vTaskStartScheduler();  				

	/*If we do reach here then it is likely that there was insufficient heap memory available for a resource to be created*/
	return FALSE;
}

//====================================================================================
/* TASK list*/
static void prvShellTask( void *pvParameters )
{
	for(;;)
	{
		main_loop();
	}
}

static void prvAHDCoaxialTask( void *pvParameters )
{
	for(;;)
	{
		if(pvInfo->app_mode)	nvp6021_coax_receive(pvInfo);
	}
}

static void prvVideoProcessTask(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 1000;

	for(;;)
	{
		xLastWakeTime = xTaskGetTickCount();
		vTaskDelayUntil( &xLastWakeTime, xFrequency );

		check_video_config(pvInfo);
	}
}
//===================================================================================
static unsigned char repeater_video_config(AHD_INFO *pvInfo)
{
	pvInfo->current_resolution = nvp6124_video_detect();
	//	xprintf("pvInfo->current_resolution %x\n", pvInfo->current_resolution);

	if(pvInfo->rx.current_resolution != pvInfo->current_resolution)
	{
		vTaskSuspend(xHandle1);			
		vTaskSuspend(xHandle3);				//suspend AHD coaxial task
		Delay1MS(10);

		pvInfo->flag = ON;					//video resolution changed
		MDIN3xx_EnableMainDisplay(OFF);

		nvp6124_vmode_set(pvInfo);

		GPIO_ResetBits(GPIOC,GPIO_Pin_9);	//NVP6021 reset
		Delay1MS(20);
		GPIO_SetBits(GPIOC,GPIO_Pin_9);		//NVP6021 release reset

		nvp6021_init(pvInfo);
		VideoProcessHandler(pvInfo);

		vTaskResume(xHandle1);		
		vTaskResume(xHandle3);				//Resume AHD coaxial task
	}

	if(pvInfo->vformat != isPAL)
	{
		vTaskSuspend(xHandle1);		
		vTaskSuspend(xHandle3);				//suspend AHD coaxial task
		Delay1MS(10);

		pvInfo->flag = ON;					//video resolution changed
		pvInfo->vformat = isPAL;	

		MDIN3xx_EnableMainDisplay(OFF);
		nvp6124_vmode_set(pvInfo);

		GPIO_ResetBits(GPIOC,GPIO_Pin_9);	//NVP6021 reset
		Delay1MS(20);
		GPIO_SetBits(GPIOC,GPIO_Pin_9);		//NVP6021 release reset

		nvp6021_init(pvInfo);
		VideoProcessHandler(pvInfo);

		vTaskResume(xHandle1);		
		vTaskResume(xHandle3);				//Resume AHD coaxial task
	}
}

static unsigned char converter_typeA_vidoe_config(AHD_INFO *pvInfo)
{
	pvInfo->current_resolution = nvp6124_video_detect();

	if(pvInfo->rx.current_resolution != pvInfo->current_resolution)
	{
		vTaskSuspend(xHandle1);		
		Delay1MS(10);

		//xprintf("pvInfo->rx.current_resolution %02x\n", pvInfo->rx.current_resolution);

		pvInfo->flag = ON;			//video resolution changed
		MDIN3xx_EnableMainDisplay(OFF);

		nvp6124_vmode_set(pvInfo);

		VideoProcessHandler(pvInfo);
		vTaskResume(xHandle1);		
	}

	if(pvInfo->vformat != isPAL)
	{
		vTaskSuspend(xHandle1);		
		Delay1MS(10);

		pvInfo->flag = ON;					//video resolution changed
		pvInfo->vformat = isPAL;	

		VideoProcessHandler(pvInfo);
		vTaskResume(xHandle1);		
	}
	VideoHTXCtrlHandler();
}

static unsigned char converter_typeB_vidoe_config(AHD_INFO *pvInfo)
{
	unsigned char cable_plugged = 0;

	if(adv7611_i2c_read(IO_MAP_ADDR, 0x21, &cable_plugged) < 1) return 0;
	pvInfo->rx.hpa_status_port_a = ( cable_plugged >> 3 ) & 0x1;

	//xprintf("pvInfo->rx.hpa_status_port_a %02x\n", pvInfo->rx.hpa_status_port_a);

	//test function
	if(pvInfo->rx.hpa_status_port_a)
	{
		nvp6021_test_pattern(OFF);
		ADV7611_Auto_Detect(pvInfo);

		if(pvInfo->rx.current_resolution != pvInfo->current_resolution)
		{
			//xprintf("pvInfo->current_resolution %02X\n", pvInfo->current_resolution);
			pvInfo->flag = ON;
			pvInfo->rx.no_video_flag = OFF;	

			ADV7611_Video_Set(pvInfo);	

			GPIO_ResetBits(GPIOC,GPIO_Pin_9);			//NVP6021 reset
			Delay1MS(20);
			GPIO_SetBits(GPIOC,GPIO_Pin_9);				//NVP6021 release reset

			VideoProcessHandler(pvInfo);
			nvp6021_init(pvInfo);
		}

		if(pvInfo->tx.current_resolution != isFAHD_OUT)
		{
			//xprintf("isFAHD_OUT %02X  pvInfo->vformat %02x \n", isFAHD_OUT, pvInfo->vformat);
			pvInfo->tx.current_resolution = isFAHD_OUT;
			pvInfo->flag = ON;

			GPIO_ResetBits(GPIOC,GPIO_Pin_9);			//NVP6021 reset
			Delay1MS(20);
			GPIO_SetBits(GPIOC,GPIO_Pin_9);				//NVP6021 release reset

			VideoProcessHandler(pvInfo);
			nvp6021_init(pvInfo);
		}

		if(pvInfo->vformat != isPAL)
		{
			//xprintf("isPAL %02X  pvInfo->vformat %02x \n", isPAL, pvInfo->vformat);
			pvInfo->vformat = isPAL;
			pvInfo->flag = ON;

			GPIO_ResetBits(GPIOC,GPIO_Pin_9);			//NVP6021 reset
			Delay1MS(20);
			GPIO_SetBits(GPIOC,GPIO_Pin_9);				//NVP6021 release reset

			VideoProcessHandler(pvInfo);
			nvp6021_init(pvInfo);
		}
	}
	else
	{
		//nvp6021_test_pattern(ON);
	}
}

static void check_video_config(AHD_INFO *pvInfo)
{
	// mode						high							low
	//---------------------------------------------------------------------------------
	//pvInfo->app_mode 		1(high):repeater mode 	 		0(low): converter mode
	//pvInfo->input_mode 	1(high):hdmi input mode   		0(low): analog video input mode
	//pvInfo->output_mode 	1(high):full hd output mode   	0(low): hd output mode
	//pvInfo->vformat 		1(high):25p or PAL  			0(low): 30p or NTSC 
	//---------------------------------------------------------------------------------

	//First, check sw configuration changed
	//Second, check video input resoulution changed

	if(pvInfo->app_mode != isRepeater || pvInfo->input_mode != isHDMI_IN)
	{
		xprintf("application mode changed!!\n");
		video_mode_configuration(pvInfo);
		device_init(pvInfo);
	}
	else
	{
		if(pvInfo->app_mode)							 
		{
			repeater_video_config(pvInfo);
		}
		else if(!pvInfo->input_mode)
		{
			converter_typeA_vidoe_config(pvInfo);
		}
		else							
		{
			converter_typeB_vidoe_config(pvInfo);
		}
	}
}
//===================================================================================

/*Init Device & turn on LED for each case */
void LedControl(unsigned char device_initStatus, unsigned char Direction)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	switch(device_initStatus)
	{
		case 0:				//check MCU init success or fail 
			Direction?		GPIO_SetBits(GPIOA, GPIO_Pin_2) :	GPIO_ResetBits(GPIOA, GPIO_Pin_2);
			break;
		case 1:				//Reater or Converter
			Direction?		GPIO_SetBits(GPIOA, GPIO_Pin_0) :	GPIO_SetBits(GPIOC, GPIO_Pin_5);
			break;
		case 2:				//Input device  check 
			Direction?		GPIO_SetBits(GPIOA, GPIO_Pin_1) :	GPIO_SetBits(GPIOC, GPIO_Pin_6);
			break;
		case 3:				//FAHD out or AHD out (only NVP6021 option)
			Direction?		GPIO_SetBits(GPIOA, GPIO_Pin_3) :	GPIO_SetBits(GPIOC, GPIO_Pin_8);
			break;
		default:
			break;
	}
}

static void device_init(AHD_INFO *pvInfo)
{
	/* Enable Power supply & Device Reset */
	power_control(ON);										
	device_reset(pvInfo);

	// mode						high							low
	//---------------------------------------------------------------------------------
	//pvInfo->app_mode 		1(high):repeater mode 	 		0(low): converter mode
	//pvInfo->input_mode 	1(high):hdmi input mode   		0(low): analog video input mode
	//pvInfo->output_mode 	1(high):full hd output mode   	0(low): hd output mode
	//pvInfo->vformat 		1(high):25p or PAL  			0(low): 30p or NTSC 
	//---------------------------------------------------------------------------------

	if(pvInfo->app_mode)							 
	{	
		xprintf("Repeater mode \n");
		LedControl(MODE_TYPE, FORWARD);				

		nvp6124_init(pvInfo);
		MDIN3xx_Init(pvInfo);
		nvp6021_init(pvInfo);
		VideoProcessHandler(pvInfo);
	}
	else if(!pvInfo->input_mode)	
	{
		xprintf("AHD to HDMI converter mode \n");
		LedControl(MODE_TYPE, REVERSE);		

		nvp6124_init(pvInfo);
		MDIN3xx_Init(pvInfo);
		VideoProcessHandler(pvInfo);
		VideoHTXCtrlHandler();
	}
	else									
	{	
		xprintf("HDMI to AHD converter mode \n");
		LedControl(MODE_TYPE, REVERSE);			
	
		ADV7611_Init(pvInfo);	
		MDIN3xx_Init(pvInfo);
		nvp6021_init(pvInfo);
		VideoProcessHandler(pvInfo);
	}
}

//====================================================================================
/* Hardware Setup*/
static void RCC_Configuration(void)
{
	RCC_DeInit();					/* RCC system reset(for debug purpose) */	
	RCC_HSEConfig(RCC_HSE_ON);		/* Enable HSE(High SPeed External Clock) */
	HSEStartUpStatus = RCC_WaitForHSEStartUp(); 	 /* Wait till HSE is ready */

	if (HSEStartUpStatus == SUCCESS)
	{
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable); 	/* Enable Prefetch Buffer(FLASH access control enable) */
		FLASH_SetLatency(FLASH_Latency_2); 						/* Flash 2 wait state */
		RCC_HCLKConfig(RCC_SYSCLK_Div1);						/* HCLK = SYSCLK */
		RCC_PCLK2Config(RCC_HCLK_Div1);							/* PCLK2 = HCLK */
		RCC_PCLK1Config(RCC_HCLK_Div2);							/* PCLK1 = HCLK/2 */
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);	/* PLLCLK = 8MHz * 9 = 72 MHz */
		RCC_PLLCmd(ENABLE);										/* Enable PLL */

		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)		/* Wait till PLL is ready */
		{
		}

		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);				/* Select PLL as system clock source */

		while (RCC_GetSYSCLKSource() != 0x08)					/* Wait till PLL is used as system clock source */
		{
		}
	}

	if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST) != RESET)			/* Check if the system has resumed from WWDG reset */
	{ 															/* WWDGRST flag set */
		RCC_ClearFlag();										/* Clear reset flags */
	}
	else
	{ 															/* WWDGRST flag is not set */
	}

    NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x0 );				/* Set the Vector Table base address at 0x08000000 */
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_1 );

    RCC_APB2PeriphClockCmd(    RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB				/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE  and AFIO clocks */
           						| RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE );
}

static void mcu_initialize( void )
{
	RCC_Configuration();
	TimerInit();												/* Enable TIM2 */
	xuart_init(115200);											/* Enable UART1 w/ 115200 baud */
	I2C_Initialize();											/* I2C(GPIO) init */
	LedControl(MCUINIT, FORWARD);
	
	xprintf("\r====================================\n");
	xprintf("======AHD REPEATER/CONVERTER========\n");
	xprintf("======VERSION 1.0           ========\n");
	xprintf("====================================\n");
}

static void power_control(unsigned char status)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	status?	GPIO_SetBits(GPIOD, GPIO_Pin_2) : GPIO_ResetBits(GPIOD, GPIO_Pin_2); 
}

static void video_mode_configuration(AHD_INFO *pvInfo)
{
	// mode						high							low
	//---------------------------------------------------------------------------------
	//pvInfo->app_mode 		1(high):repeater mode 	 		0(low): converter mode
	//pvInfo->input_mode 	1(high):hdmi input mode   		0(low): analog video input mode
	//pvInfo->output_mode 	1(high):full hd output mode   	0(low): hd output mode
	//pvInfo->vformat 		1(high):25p or PAL  			0(low): 30p or NTSC 
	//---------------------------------------------------------------------------------

	//check configuration
	pvInfo->app_mode =	(isRepeater); 
	pvInfo->input_mode = (isHDMI_IN);
	pvInfo->output_mode = (isFAHD_OUT);
	pvInfo->vformat = 	(isPAL);
}

static void device_reset(AHD_INFO *pvInfo)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOC, GPIO_Pin_2);			//NVP6124 reset 
	GPIO_ResetBits(GPIOC, GPIO_Pin_3);			//ADV7611 reset
	GPIO_ResetBits(GPIOC, GPIO_Pin_4);			//MDIN380 reset
	GPIO_ResetBits(GPIOC, GPIO_Pin_9);			//NVP6021 reset
	Delay1MS(100);

	if(pvInfo->app_mode)					
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_2);			//NVP6124 release reset
		GPIO_SetBits(GPIOC, GPIO_Pin_4);			//MDIN380 release reset
		GPIO_SetBits(GPIOC, GPIO_Pin_9);			//NVP6021 release reset

	}
	else if(!pvInfo->input_mode)		
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_2);			//NVP6124 release reset
		GPIO_SetBits(GPIOC, GPIO_Pin_4);			//MDIN380 release reset
	}
	else								
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_3);			//ADV7611 release reset
		GPIO_SetBits(GPIOC, GPIO_Pin_4);			//MDIN380 release reset
		GPIO_SetBits(GPIOC, GPIO_Pin_9);			//NVP6021 release reset
	}
}
