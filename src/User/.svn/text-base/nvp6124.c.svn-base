/**
 *	@file   nvp6124.c
 *	@brief	
 *	@author luisfynn<tani223@pinetron.com>
 *	@date   2015/04/27 10:01
 */

/* system include */
#include <stdlib.h>
#include <string.h>
/* local include */
#include "stm32f10x.h"
#include "video.h"
#include "i2c.h"
#include "command.h"
#include "mutex.h"
#include "nvp6124_reg.h"
#include "nvp6124.h"

MUTEX_DECLARE(s_sem_nvp6124);

static void nvp6124_i2c_lock(void)      { MUTEX_LOCK(s_sem_nvp6124);    }
static void nvp6124_i2c_unlock(void)    { MUTEX_UNLOCK(s_sem_nvp6124);  }

#define NVP6124_I2C_ADDR 	0x64
#define NVP6124_REG_TABLE_SECTION       __attribute__ ((section(".rodata.nvp6124")))

/*-------audio-----------------*/
#define	AUDIO_MASTER	1
#define	AUDIO_SLAVE		0

#define	AUDIO_16CH		3		
#define	AUDIO_8CH		2	
#define	AUDIO_4CH		1	
#define	AUDIO_2CH		0	

#define	AUDIO_16K_SAMPLE		1
#define	AUDIO_8K_SAMPLE			0

#define AUDIO_16Bit				0
#define AUDIO_8Bit				1

/*mode*/
static void nvp6124_vconf_set(AHD_INFO *pvInfo);
static void nvp6124_common_init(AHD_INFO *pvInfo);
static void nvp6124_set_mode(AHD_INFO *pvInfo);
static void nvp6124_pll_reset(void);

/*audio*/
static unsigned char nvp6124_set_audio(AHD_INFO *pvInfo);

/*i2c*/
void nvp6124_i2c_write(unsigned char reg_addr, unsigned char reg_data)
{
      I2C1_ByteWrite(NVP6124_I2C_ADDR, reg_addr, reg_data);
}

void nvp6124_i2c_write_buf(unsigned char reg_addr, unsigned char *buf, unsigned char len)
{  
	int i;
	for(i=0; i<len; i++){
		//xprintf("%x register value : %x\n", i, *buf);
		if(I2C_OK != I2C1_ByteWrite(NVP6124_I2C_ADDR, reg_addr+i, buf[i])) break;
	}
}

unsigned char nvp6124_i2c_read(unsigned char reg_addr, unsigned char* p_reg_data)
{  
	if(I2C_OK != I2C1_ByteRead(NVP6124_I2C_ADDR, reg_addr, p_reg_data)) return 1;
	return 0;
}

unsigned char nvp6124_test_pattern(unsigned char OnOff)
{
	nvp6124_i2c_lock();

	if(OnOff)
	{
		//CH1 test pattern
		nvp6124_i2c_write(0xFF, BANK5);
		nvp6124_i2c_write(0x2c, 0x08);		

		nvp6124_i2c_write(0xFF, BANK0);
		nvp6124_i2c_write(0x78, 0x8c);		//horizontal colorbar

		nvp6124_i2c_write(0xFF, BANK5);
		nvp6124_i2c_write(0x0d, 0x00);
	}
	else
	{
		//CH1 test pattern
		nvp6124_i2c_write(0xFF, BANK5);
		nvp6124_i2c_write(0x2c, 0x00);		

		nvp6124_i2c_write(0xFF, BANK0);
		nvp6124_i2c_write(0x78, 0x88);		//horizontal colorbar

		nvp6124_i2c_write(0xFF, BANK5);
		nvp6124_i2c_write(0x0d, 0x10);
	}
	
	nvp6124_i2c_unlock();
}


unsigned char nvp6124_vmode_set(AHD_INFO *pvInfo)
{
	// mode						high							low
	//---------------------------------------------------------------------------------
	//pvInfo->app_mode 		1(high):repeater mode 	 		0(low): converter mode
	//pvInfo->input_mode 	1(high):hdmi input mode   		0(low): analog video input mode
	//pvInfo->output_mode 	1(high):full hd output mode   	0(low): hd output mode
	//pvInfo->vformat 		1(high):25p or PAL  			0(low): 30p or NTSC 
	//---------------------------------------------------------------------------------

	pvInfo->rx.current_resolution = pvInfo->current_resolution;

	if(pvInfo->app_mode){
		if( pvInfo->rx.current_resolution == AHDRX_OUT_960X480 || pvInfo->rx.current_resolution == AHDRX_OUT_960X576)
		{
			pvInfo->rx.current_resolution = RSV;
		}
	}

	nvp6124_vconf_set(pvInfo);
	nvp6124_set_mode(pvInfo);	
	nvp6124_set_audio(pvInfo);
	nvp6124_pll_reset();

	/*nvp6124 coaxial protocol*/
	nvp6124_acp_rx_clear();	
	nvp6124_acp_each_setting(pvInfo);

	/*nvp6124 eq*/
//	nvp6124_init_eq_stage(pvInfo);
//	nvp6124_set_equalizer(pvInfo);
}

unsigned char nvp6124_init(AHD_INFO *pvInfo)
{
	unsigned char i, ret, nID = 0;

	MUTEX_INIT(s_sem_nvp6124);
	
	//NVP6124 ID check
	while(nID != 0x84){
		nvp6124_i2c_write(0xFF, BANK0);
		nvp6124_i2c_read(0xF4, &nID);
		xprintf("AHD RX ID: %s\n", (nID == 0x84)?  "NVP6124" : "detecting");
	}

	nvp6124_common_init(pvInfo);
	nvp6124_vmode_set(pvInfo);

	LedControl(RX_TYPE, REVERSE);
}

static void nvp6124_960h_addition(AHD_INFO *pvInfo )
{
	nvp6124_i2c_write(0xFF, BANK0);
	nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_B0_PAL_Buf : NVP6124_B0_NTSC_Buf, 254);

	nvp6124_i2c_write(0xFF, BANK5);
	nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_B5_PAL_Buf : NVP6124_B5_NTSC_Buf, 254);
}

static void nvp6124_common_init(AHD_INFO *pvInfo )
{
	nvp6124_i2c_write(0xFF, BANK0);
	nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_B0_25P_Buf : NVP6124_B0_30P_Buf, 254);

	nvp6124_i2c_write(0xFF, BANK1);
	nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_B1_25P_Buf : NVP6124_B1_30P_Buf, 254);

	nvp6124_i2c_write(0xFF, BANK2);
	nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_B2_25P_Buf : NVP6124_B2_30P_Buf, 254);

	nvp6124_i2c_write(0xFF, BANK3);
	nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_B3_25P_Buf : NVP6124_B3_30P_Buf, 254);

	nvp6124_i2c_write(0xFF, BANK4);
	nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_B4_25P_Buf : NVP6124_B4_30P_Buf, 254);

#if 1
	nvp6124_i2c_write(0xFF, BANK5);
	nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_B5678_25P_Buf : NVP6124_B5678_30P_Buf, 254);

	nvp6124_i2c_write(0xFF, BANK6);
	nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_B5678_25P_Buf : NVP6124_B5678_30P_Buf, 254);

	nvp6124_i2c_write(0xFF, BANK7);
	nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_B5678_25P_Buf : NVP6124_B5678_30P_Buf, 254);

	nvp6124_i2c_write(0xFF, BANK8);
	nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_B5678_25P_Buf : NVP6124_B5678_30P_Buf, 254);
#endif

	nvp6124_i2c_write(0xFF, BANK9);
	nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_B9_25P_Buf : NVP6124_B9_30P_Buf, 254);

	nvp6124_i2c_write(0xFF, BANKA);
	nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_BA_25P_Buf : NVP6124_BA_30P_Buf, 254);

	nvp6124_i2c_write(0xFF, BANKB);
	nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_BB_25P_Buf : NVP6124_BB_30P_Buf, 254);

	if(	pvInfo->rx.current_resolution == AHDRX_OUT_960X480 || pvInfo->rx.current_resolution == AHDRX_OUT_960X576 )
	{
		nvp6124_960h_addition(pvInfo);
	}

	nvp6124_pll_reset();
	Delay1MS(1000);
	pvInfo->current_resolution = nvp6124_video_detect();
}

/*--------------CLK SET-----------------------------*/
static void nvp6124_outmode_select(AHD_INFO *pvInfo)
{
	nvp6124_i2c_write(0xff, BANK1);

	//select video output
	nvp6124_i2c_write(0xC0, 0x00);
	nvp6124_i2c_write(0xC1, 0x00);	
	nvp6124_i2c_write(0xC8, 0x00);

	if(	pvInfo->rx.current_resolution == AHDRX_OUT_960X480 || pvInfo->rx.current_resolution == AHDRX_OUT_960X576 ) 					nvp6124_i2c_write(0xCC, 0x30);
	else if( pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25 ) 	nvp6124_i2c_write(0xCC, 0x50);
	else if( pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P25 ) 	nvp6124_i2c_write(0xCC, 0x60);
	else if( pvInfo->rx.current_resolution == RSV) 	nvp6124_i2c_write(0xCC, 0x60);

	nvp6124_i2c_write(0x97, 0x0f);	
}

static void nvp6124_960h_clkset(AHD_INFO *pvInfo)
{
	nvp6124_i2c_write(0xFF, BANK0);

	nvp6124_i2c_write(0x18, 0x08);
	nvp6124_i2c_write(0x81, 0x00);
	nvp6124_i2c_write(0x85, 0x11);
	nvp6124_i2c_write(0x89, 0x00);

	nvp6124_i2c_write(0xFF, BANK1 );
	nvp6124_i2c_write(0x88, 0x66);
	nvp6124_i2c_write(0x89, 0x66);
	nvp6124_i2c_write(0x8A, 0x66);
	nvp6124_i2c_write(0x8B, 0x66);
	nvp6124_i2c_write(0x8C, 0x26);
	nvp6124_i2c_write(0x8D, 0x26);
	nvp6124_i2c_write(0x8E, 0x26);
	nvp6124_i2c_write(0x8F, 0x26);
	nvp6124_i2c_write(0xd7, 0x00);	

	nvp6124_i2c_write(0xFF, BANK2);
	nvp6124_i2c_write(0x12, 0xFF);
	nvp6124_i2c_write(0x16, 0x00);
	nvp6124_i2c_write(0x17, 0x00);	
	
	nvp6124_i2c_write(0xFF, BANK9);
	nvp6124_i2c_write(0x44, 0x00);

	nvp6124_i2c_write(0xFF, BANK5);
	nvp6124_i2c_write(0x62, 0x00);	
	nvp6124_i2c_write(0x64, 0x00);
	nvp6124_i2c_write(0x6A, 0x00);

	nvp6124_i2c_write(0xFF, BANK0);
	nvp6124_i2c_write(0x30, pvInfo->vformat? 0x12 : 0x12);
	nvp6124_i2c_write(0x58, pvInfo->vformat? 0x50 : 0x80);
	nvp6124_i2c_write(0xa0, pvInfo->vformat? 0x10 : 0x00);
	nvp6124_i2c_write(0xa4, pvInfo->vformat? 0x00 : 0x00);
	nvp6124_i2c_write(0x8e, pvInfo->vformat? 0x03 : 0x03);
	nvp6124_i2c_write(0x8f, pvInfo->vformat? 0x03 : 0x03);
	nvp6124_i2c_write(0x90, pvInfo->vformat? 0x03 : 0x03);
	nvp6124_i2c_write(0x91, pvInfo->vformat? 0x03 : 0x03);
}

static void nvp6124_720p_clkset(AHD_INFO *pvInfo)
{
	nvp6124_i2c_write(0xFF, BANK1 );
	nvp6124_i2c_write(0x88, 0x55);
	nvp6124_i2c_write(0x89, 0x55);
	nvp6124_i2c_write(0x8A, 0x55);
	nvp6124_i2c_write(0x8B, 0x55);
	nvp6124_i2c_write(0x8C, 0x50);
	nvp6124_i2c_write(0x8D, 0x50);
	nvp6124_i2c_write(0x8E, 0x50);
	nvp6124_i2c_write(0x8F, 0x50);	
	nvp6124_i2c_write(0xd7, 0x0f);		
	nvp6124_i2c_write(0x97, 0x0f);			
	
	nvp6124_i2c_write(0xFF, BANK5);
	nvp6124_i2c_write(0x53, 0x00);			

	nvp6124_i2c_write(0xFF, BANK0);
	
	nvp6124_i2c_write(0x58, pvInfo->vformat?  0x90 : 0xa0);			// Horizontal start position of output image 
	nvp6124_i2c_write(0x59, pvInfo->vformat?  0x90 : 0xa0);			 
	nvp6124_i2c_write(0x5a, pvInfo->vformat?  0x90 : 0xa0);			 
	nvp6124_i2c_write(0x5b, pvInfo->vformat?  0x90 : 0xa0);			 

	nvp6124_i2c_write(0x30, pvInfo->vformat?  0x10 : 0x10);			// Y delay control
	nvp6124_i2c_write(0x31, pvInfo->vformat?  0x10 : 0x10);			
	nvp6124_i2c_write(0x32, pvInfo->vformat?  0x10 : 0x10);			
	nvp6124_i2c_write(0x33, pvInfo->vformat?  0x10 : 0x10);			

	nvp6124_i2c_write(0x18, pvInfo->vformat?  0x00 : 0x00);			// Y low pass filter control
	nvp6124_i2c_write(0x19, pvInfo->vformat?  0x00 : 0x00);			
	nvp6124_i2c_write(0x1a, pvInfo->vformat?  0x00 : 0x00);			
	nvp6124_i2c_write(0x1b, pvInfo->vformat?  0x00 : 0x00);			

	nvp6124_i2c_write(0x81, pvInfo->vformat?  0x07 : 0x06);			// AHD Mode selection 
	nvp6124_i2c_write(0x82, pvInfo->vformat?  0x07 : 0x06);			 
	nvp6124_i2c_write(0x83, pvInfo->vformat?  0x07 : 0x06);			 
	nvp6124_i2c_write(0x84, pvInfo->vformat?  0x07 : 0x06);			 

	nvp6124_i2c_write(0x85, 0x00);			// Control Active Region & select 960H	
	nvp6124_i2c_write(0x86, 0x00);				
	nvp6124_i2c_write(0x87, 0x00);				
	nvp6124_i2c_write(0x88, 0x00);				

	nvp6124_i2c_write(0x89, 0x10);			// SH720H setting 
	nvp6124_i2c_write(0x8a, 0x10);			 
	nvp6124_i2c_write(0x8b, 0x10);			 
	nvp6124_i2c_write(0x8c, 0x10);			 

	nvp6124_i2c_write(0x8e, 0x0b);			// MASK SEL	
	nvp6124_i2c_write(0x8f, 0x0b);				
	nvp6124_i2c_write(0x90, 0x0b);				
	nvp6124_i2c_write(0x91, 0x0b);				

	nvp6124_i2c_write(0xFF, BANK2);
	nvp6124_i2c_write(0x12, 0x00);				
	nvp6124_i2c_write(0x16, 0x55);				
	nvp6124_i2c_write(0x17, 0x55);				

	nvp6124_i2c_write(0xFF, BANK5);
	nvp6124_i2c_write(0x6a, 0x00);

	nvp6124_i2c_write(0xFF, BANK9);
	nvp6124_i2c_write(0x50, pvInfo->vformat? 0x46 : 0xee);
	nvp6124_i2c_write(0x51, pvInfo->vformat? 0x08 : 0x00);
	nvp6124_i2c_write(0x52, pvInfo->vformat? 0x10 : 0xe5);
	nvp6124_i2c_write(0x53, pvInfo->vformat? 0x4f : 0x4e);
	nvp6124_i2c_write(0x54, pvInfo->vformat? 0x46 : 0xee);
	nvp6124_i2c_write(0x55, pvInfo->vformat? 0x08 : 0x00);
	nvp6124_i2c_write(0x56, pvInfo->vformat? 0x10 : 0xe5);
	nvp6124_i2c_write(0x57, pvInfo->vformat? 0x4f : 0x4e);
	nvp6124_i2c_write(0x58, pvInfo->vformat? 0x46 : 0xee);
	nvp6124_i2c_write(0x59, pvInfo->vformat? 0x08 : 0x00);
	nvp6124_i2c_write(0x5a, pvInfo->vformat? 0x10 : 0xe5);
	nvp6124_i2c_write(0x5b, pvInfo->vformat? 0x4f : 0x4e);
	nvp6124_i2c_write(0x5c, pvInfo->vformat? 0x46 : 0xee);
	nvp6124_i2c_write(0x5d, pvInfo->vformat? 0x08 : 0x00);
	nvp6124_i2c_write(0x5e, pvInfo->vformat? 0x10 : 0xe5);
	nvp6124_i2c_write(0x5f, pvInfo->vformat? 0x4f : 0x4e);

	//nvp6124_i2c_write(0xFF, BANK1);
	//nvp6124_i2c_write(0x97, 0x0f);
}

///////////////////////////////////////////////////////////////////////////////

static void nvp6124_pll_reset( void )
{
	nvp6124_i2c_write(0xFF, BANK1); 
	nvp6124_i2c_write(0x82, 0x14); 
	nvp6124_i2c_write(0x83, 0x2C); 
	nvp6124_i2c_write(0x3e, 0x10); 
	nvp6124_i2c_write(0x80, 0x60); 
	nvp6124_i2c_write(0x80, 0x61); 
	Delay1MS(100); 
	nvp6124_i2c_write(0x80, 0x40); 
	nvp6124_i2c_write(0x81, 0x02); 
	nvp6124_i2c_write(0x97, 0x00); 
	Delay1MS(10); 
	nvp6124_i2c_write(0x80, 0x60); 
	nvp6124_i2c_write(0x81, 0x00); 
	Delay1MS(10); 
	nvp6124_i2c_write(0x97, 0x0F); 
	nvp6124_i2c_write(0x38, 0x18); 
	nvp6124_i2c_write(0x38, 0x08); 
	Delay1MS(10); 
	nvp6124_i2c_write( 0xCA, 0xFF);
}

static void nvp6124_set_mode(AHD_INFO *pvInfo)
{
	unsigned char i;
	unsigned char read_data = 0;

	unsigned char pn_value_sd_nt[4] = 	{0x4D,0x0E,0x88,0x6C};
	unsigned char pn_value_sd_pal[4] = 	{0x75,0x35,0xB4,0x6C};
	unsigned char pn_value_fhd_nt[4] = 	{0x2C,0xF0,0xCA,0x52}; // FSC = 24M
	unsigned char pn_value_fhd_pal[4] =	{0xC8,0x7D,0xC3,0x52}; // FSC = 24M
	unsigned char pn_value_720p_30[4] =	{0xEE,0x00,0xE5,0x4E};
	unsigned char pn_value_720p_25[4] =	{0x46,0x08,0x10,0x4F};

	nvp6124_i2c_write(0xFF, BANK0);
	nvp6124_i2c_write(0x80, 0x0f);
	nvp6124_i2c_write(0xFF, BANK1);
	nvp6124_i2c_write(0x93, 0x80);
	
	switch(pvInfo->rx.current_resolution) 
	{ 
		case AHDRX_OUT_960X480:
		case AHDRX_OUT_960X576: 
			//reference ahd_drv_external_150420_video.c
			nvp6124_i2c_write(0xFF, BANK0);
			nvp6124_i2c_write(0x08, pvInfo->vformat? 0xdd: 0xa0);
			nvp6124_i2c_write(0x0C, pvInfo->vformat? 0xf4: 0xf4);
			nvp6124_i2c_write(0x10, pvInfo->vformat? 0x90: 0x90);
			nvp6124_i2c_write(0x14, pvInfo->vformat? 0x80: 0x80);
			nvp6124_i2c_write(0x18, pvInfo->vformat? 0x18: 0x18);
			nvp6124_i2c_write(0x21, pvInfo->vformat? 0x02: 0x82);
			nvp6124_i2c_write(0x23, pvInfo->vformat? 0x43: 0x43);
			nvp6124_i2c_write(0x30, pvInfo->vformat? 0x12: 0x11);
			nvp6124_i2c_write(0x3c, pvInfo->vformat? 0x80: 0x78);
			nvp6124_i2c_write(0x40, pvInfo->vformat? 0x00: 0x01);
			nvp6124_i2c_write(0x44, pvInfo->vformat? 0x00: 0x00); 
			nvp6124_i2c_write(0x48, pvInfo->vformat? 0x00: 0x00);
			nvp6124_i2c_write(0x4c, pvInfo->vformat? 0x04: 0x00);
			nvp6124_i2c_write(0x50, pvInfo->vformat? 0x04: 0x00);
			nvp6124_i2c_write(0x58, pvInfo->vformat? 0x80: 0x90);
			nvp6124_i2c_write(0x5c, pvInfo->vformat? 0x1e: 0x1e);
			nvp6124_i2c_write(0x64, pvInfo->vformat? 0x0d: 0x08);
			nvp6124_i2c_write(0x81, pvInfo->vformat? 0x00: 0x00);
			nvp6124_i2c_write(0x85, pvInfo->vformat? 0x11: 0x11);
			nvp6124_i2c_write(0x89, pvInfo->vformat? 0x10: 0x00);
			nvp6124_i2c_write(0x8e, pvInfo->vformat? 0x08: 0x07);
			nvp6124_i2c_write(0x93, 0x00);
			nvp6124_i2c_write(0x98, pvInfo->vformat? 0x07: 0x04);
			nvp6124_i2c_write(0xa0, pvInfo->vformat? 0x00: 0x10);
			nvp6124_i2c_write(0xa4, pvInfo->vformat? 0x00: 0x01);
			nvp6124_i2c_write(0xFF, BANK1);
			nvp6124_i2c_write(0x88, pvInfo->vformat? 0x7e: 0x7e);
			nvp6124_i2c_write(0x8c, pvInfo->vformat? 0x26: 0x26);
			nvp6124_i2c_write(0xcc, pvInfo->vformat? 0x36: 0x36);
			nvp6124_i2c_read(0xd7, &read_data);
			nvp6124_i2c_write(0xd7, read_data &(~1));
			
			nvp6124_i2c_write(0xFF, BANK5);
			nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_B5_PAL_Buf : NVP6124_B5_NTSC_Buf, 254);
			nvp6124_i2c_write(0x06, 0x40); 
			nvp6124_i2c_write(0x25, pvInfo->vformat? 0xca: 0xda);

			nvp6124_i2c_write(0xFF, BANK9);
			nvp6124_i2c_write(0x40, 0x60);
			nvp6124_i2c_read(0x44, &read_data);
			nvp6124_i2c_write(0x44, read_data|(~1));

			nvp6124_i2c_write(0x50, pvInfo->vformat?	pn_value_sd_pal[0] : pn_value_sd_nt[0]);	//ch%41 960H	
			nvp6124_i2c_write(0x51, pvInfo->vformat?	pn_value_sd_pal[1] : pn_value_sd_nt[1]);		
			nvp6124_i2c_write(0x52, pvInfo->vformat?	pn_value_sd_pal[2] : pn_value_sd_nt[2]);		
			nvp6124_i2c_write(0x53, pvInfo->vformat?	pn_value_sd_pal[3] : pn_value_sd_nt[3]);
			break;
		case AHDRX_OUT_1280X720P30:
		case AHDRX_OUT_1280X720P25:
			nvp6124_i2c_write(0xFF, BANK0);
			nvp6124_i2c_write(0x08, pvInfo->vformat? 0x60: 0x60);
			nvp6124_i2c_write(0x0C, pvInfo->vformat? 0xf4: 0xf4);
			nvp6124_i2c_write(0x10, pvInfo->vformat? 0x90: 0x90);
			nvp6124_i2c_write(0x14, pvInfo->vformat? 0x90: 0x90);
			nvp6124_i2c_write(0x18, pvInfo->vformat? 0x30: 0x30);			
			nvp6124_i2c_write(0x21, 0x92);
			nvp6124_i2c_write(0x22, 0x0a);
			nvp6124_i2c_write(0x23, 0x43);
			nvp6124_i2c_write(0x30, pvInfo->vformat? 0x12: 0x12);
			nvp6124_i2c_write(0x3c, pvInfo->vformat? 0x84: 0x84);
			nvp6124_i2c_write(0x40, pvInfo->vformat? 0x00: 0xfd);
			nvp6124_i2c_write(0x44, pvInfo->vformat? 0x30: 0x30);
			nvp6124_i2c_write(0x48, pvInfo->vformat? 0x30: 0x30);
			nvp6124_i2c_write(0x4c, pvInfo->vformat? 0x04: 0x04);
			nvp6124_i2c_write(0x50, pvInfo->vformat? 0x04: 0x04);
			nvp6124_i2c_write(0x58, pvInfo->vformat? 0x80: 0x90);
			nvp6124_i2c_write(0x5c, pvInfo->vformat? 0x9e: 0x9e);
			nvp6124_i2c_write(0x64, pvInfo->vformat? 0xb1: 0xb2);
			nvp6124_i2c_write(0x81, pvInfo->vformat? 0x07: 0x06);
			nvp6124_i2c_write(0x85, pvInfo->vformat? 0x00: 0x00);
			nvp6124_i2c_write(0x89, pvInfo->vformat? 0x10: 0x10);
			nvp6124_i2c_write(0x8e, pvInfo->vformat? 0x0d: 0x0d);
			nvp6124_i2c_write(0x93, 0x00);
			nvp6124_i2c_write(0x98, pvInfo->vformat? 0x07: 0x04);
			nvp6124_i2c_write(0xa0, pvInfo->vformat? 0x00: 0x00);
			nvp6124_i2c_write(0xa4, pvInfo->vformat? 0x00: 0x01);

			nvp6124_i2c_write(0xFF, BANK1);
			nvp6124_i2c_write(0x88, pvInfo->vformat? 0x5c: 0x5c);
			nvp6124_i2c_write(0x8c, pvInfo->vformat? 0x40: 0x40);
			nvp6124_i2c_write(0xcc, pvInfo->vformat? 0x46: 0x46);

			nvp6124_i2c_read(0xd7, &read_data);
			nvp6124_i2c_write(0xd7, read_data|1);
			
			nvp6124_i2c_write(0xFF, BANK5);
			nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_B5678_25P_Buf : NVP6124_B5678_30P_Buf, 254);
			nvp6124_i2c_write(0x01, 0x0d);    //alex 10.20
			nvp6124_i2c_write(0x06, 0x40); 
			nvp6124_i2c_write(0x2B, 0x78); 
			nvp6124_i2c_write(0x59, 0x00); 
			nvp6124_i2c_write(0x58, 0x13); 
			nvp6124_i2c_write(0xc0, 0x16); 

			//eq_init for 720p
			nvp6124_i2c_write(0xD8, 0x0c);    
			nvp6124_i2c_write(0xD9, 0x0e); 
			nvp6124_i2c_write(0xDA, 0x12);    
			nvp6124_i2c_write(0xDB, 0x14); 
			nvp6124_i2c_write(0xDC, 0x1c);    
			nvp6124_i2c_write(0xDD, 0x2c);
			nvp6124_i2c_write(0xDE, 0x34);

			nvp6124_i2c_write(0xFF, BANK9);
			nvp6124_i2c_write(0x40, 0x00);
			nvp6124_i2c_read(0x44, &read_data);
			nvp6124_i2c_write(0x44, read_data|1);

			nvp6124_i2c_write(0x50, pvInfo->vformat?	pn_value_720p_25[0] : pn_value_720p_30[0]);	//ch%41 960H	
			nvp6124_i2c_write(0x51, pvInfo->vformat?	pn_value_720p_25[1] : pn_value_720p_30[1]);		
			nvp6124_i2c_write(0x52, pvInfo->vformat?	pn_value_720p_25[2] : pn_value_720p_30[2]);		
			nvp6124_i2c_write(0x53, pvInfo->vformat?	pn_value_720p_25[3] : pn_value_720p_30[3]);
			break;
		case AHDRX_OUT_1920X1080P30:
		case AHDRX_OUT_1920X1080P25:
		case RSV:			//if no video, show test pattern at repeater
			nvp6124_i2c_write(0xFF, BANK0);
			nvp6124_i2c_write(0x08, pvInfo->vformat? 0x60: 0x60);
			nvp6124_i2c_write(0x0C, pvInfo->vformat? 0xf4: 0xf4);
			nvp6124_i2c_write(0x10, pvInfo->vformat? 0x90: 0x90);
			nvp6124_i2c_write(0x14, pvInfo->vformat? 0x90: 0x90);
			nvp6124_i2c_write(0x18, pvInfo->vformat? 0x00: 0x00);
			nvp6124_i2c_write(0x21, 0x92);
			nvp6124_i2c_write(0x22, 0x0a);
			nvp6124_i2c_write(0x23, 0x43);
			nvp6124_i2c_write(0x30, pvInfo->vformat? 0x12: 0x12);
			nvp6124_i2c_write(0x3c, pvInfo->vformat? 0x80: 0x80);
			nvp6124_i2c_write(0x40, pvInfo->vformat? 0x00: 0x00);
			nvp6124_i2c_write(0x44, pvInfo->vformat? 0x00: 0x00);
			nvp6124_i2c_write(0x48, pvInfo->vformat? 0x00: 0x00);
			nvp6124_i2c_write(0x4c, pvInfo->vformat? 0x00: 0x00);
			nvp6124_i2c_write(0x50, pvInfo->vformat? 0x00: 0x00);
			nvp6124_i2c_write(0x58, pvInfo->vformat? 0x6a: 0x49);
			nvp6124_i2c_write(0x5c, pvInfo->vformat? 0x9e: 0x9e);
			nvp6124_i2c_write(0x64, pvInfo->vformat? 0xbf: 0x8d);
			nvp6124_i2c_write(0x81, pvInfo->vformat? 0x03: 0x02);
			nvp6124_i2c_write(0x85, pvInfo->vformat? 0x00: 0x00);
			nvp6124_i2c_write(0x89, pvInfo->vformat? 0x10: 0x10);
			nvp6124_i2c_write(0x8e, pvInfo->vformat? 0x0a: 0x09);
			nvp6124_i2c_write(0x93, 0x00);
			nvp6124_i2c_write(0x98, pvInfo->vformat? 0x07: 0x04);
			nvp6124_i2c_write(0xa0, pvInfo->vformat? 0x00: 0x00);
			nvp6124_i2c_write(0xa4, pvInfo->vformat? 0x00: 0x01);

			nvp6124_i2c_write(0xFF, BANK1);
			nvp6124_i2c_write(0x88, pvInfo->vformat? 0x4c: 0x4c);
			nvp6124_i2c_write(0x8c, pvInfo->vformat? 0x84: 0x84);
			nvp6124_i2c_write(0xcc, pvInfo->vformat? 0x66: 0x66);
			
			nvp6124_i2c_read(0xd7, &read_data);
			nvp6124_i2c_write(0xd7, read_data|1);

			nvp6124_i2c_write(0xFF, BANK5);
			nvp6124_i2c_write_buf(0x00, pvInfo->vformat? NVP6124_B5678_25P_Buf : NVP6124_B5678_30P_Buf, 254);
			nvp6124_i2c_write(0x01, 0x0c);    //alex 10.20
			nvp6124_i2c_write(0x06, 0x40); 
			nvp6124_i2c_write(0x2a, 0x72); 
			nvp6124_i2c_write(0x2B, 0xA8); 
			nvp6124_i2c_write(0x58, 0x13); 
			nvp6124_i2c_write(0x59, 0x01); 

			//eq_init for 1080
			nvp6124_i2c_write(0xD8, 0x10);    
			nvp6124_i2c_write(0xD9, 0x1F); 
			nvp6124_i2c_write(0xDA, 0x2B);    
			nvp6124_i2c_write(0xDB, 0x7F); 
			nvp6124_i2c_write(0xDC, 0xFF);    
			nvp6124_i2c_write(0xDD, 0xFF);
			nvp6124_i2c_write(0xDE, 0xFF);

			nvp6124_i2c_write(0x24, pvInfo->vformat? 0x2a:0x1a);
			//nvp6124_i2c_write(0x47, pvInfo->vformat? 0x04:0xee);
			nvp6124_i2c_write(0x50, pvInfo->vformat? 0x84:0x86);
			nvp6124_i2c_write(0xbb, pvInfo->vformat? 0x00:0xe4);
			
			nvp6124_i2c_write(0xFF, BANK9);
			nvp6124_i2c_write(0x40, 0x00);
			nvp6124_i2c_read(0x44, &read_data);
			nvp6124_i2c_write(0x44, read_data|1);

			nvp6124_i2c_write(0x50, pvInfo->vformat?	pn_value_fhd_pal[0] : pn_value_fhd_nt[0]);	//ch%41 960H	
			nvp6124_i2c_write(0x51, pvInfo->vformat?	pn_value_fhd_pal[1] : pn_value_fhd_nt[1]);		
			nvp6124_i2c_write(0x52, pvInfo->vformat?	pn_value_fhd_pal[2] : pn_value_fhd_nt[2]);		
			nvp6124_i2c_write(0x53, pvInfo->vformat?	pn_value_fhd_pal[3] : pn_value_fhd_nt[3]);
			break;
		defalut:
			break;
	}

#if 0 
	/*--------vrc3k indivisual setting--------------*/
	nvp6124_i2c_write(0xFF, BANK5);
	nvp6124_i2c_write(0x01, 0x00);
	//nvp6124_i2c_write(0x50, 0x04);
	
	nvp6124_i2c_write(0xFF, BANK1);
	nvp6124_i2c_write(0xd0, 0x01);

	if(pvInfo->rx.current_resolution == 0x4 || pvInfo->rx.current_resolution == 0x8){
		//control low pass filter	
		nvp6124_i2c_write(0xFF, BANK0);
		nvp6124_i2c_write(0x18, pvInfo->vformat? 0x00: 0x00);			
	}
#endif
}

static unsigned char nvp6124_set_audio(AHD_INFO *pvInfo)
{
	unsigned char read_data;	

	//--------------VRC3K MCU setting---------------------//
	//audio multi path disable	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_Pin_3);		
	GPIO_ResetBits(GPIOB, GPIO_Pin_4);		
	//-----------------------------------------------//
	pvInfo->rx.audio_rec_master = AUDIO_MASTER;	
	pvInfo->rx.audio_pb_master = AUDIO_SLAVE;		
	pvInfo->rx.audio_sample_rate = AUDIO_16K_SAMPLE;					
	pvInfo->rx.audio_ch_num = AUDIO_8CH;					
	pvInfo->rx.audio_bit = AUDIO_16Bit;					

	//rec master	
	nvp6124_i2c_write(0xFF, BANK1);
	nvp6124_i2c_write(0x06, 0x1a);

	if(pvInfo->rx.audio_rec_master){
		nvp6124_i2c_write(0x07, 0x80 | (pvInfo->rx.audio_sample_rate<<3) | (pvInfo->rx.audio_bit<<2));	//Rec : master
		nvp6124_i2c_write(0x39, 0x01);
	}else{
		nvp6124_i2c_write(0x07, 0x00 | (pvInfo->rx.audio_sample_rate<<3) | (pvInfo->rx.audio_bit<<2));	//Rec : slave
		nvp6124_i2c_write(0x39, 0x81);
	}
	
	if(pvInfo->rx.audio_pb_master){
		nvp6124_i2c_write(0x13, 0x80 | (pvInfo->rx.audio_sample_rate<<3) | (pvInfo->rx.audio_bit<<2));	//pb i2c 8k 16bit : mater
		nvp6124_i2c_write(0xd5, 0x00);
	}else{
		nvp6124_i2c_write(0x13, 0x00 | (pvInfo->rx.audio_sample_rate<<3) | (pvInfo->rx.audio_bit<<2));	//pb i2c 8k 16bit : slave
		nvp6124_i2c_write(0xd5, 0x01);
	}


	if(pvInfo->rx.audio_ch_num == 8){
	
		nvp6124_i2c_write(0x06, 0x1b);
		nvp6124_i2c_write(0x08, 0x02);
		nvp6124_i2c_write(0x0f, 0x54);		//set I2S right sequence
		nvp6124_i2c_write(0x10, 0x76);
	
	}else if(pvInfo->rx.audio_ch_num == 4){
		
		nvp6124_i2c_write(0x06, 0x1b);
		nvp6124_i2c_write(0x08, 0x01);
		nvp6124_i2c_write(0x0f, 0x32);		//set I2S right sequence
	}

	nvp6124_i2c_write(0x23, 0x00);		//bypass
	
	nvp6124_i2c_write(0x01, 0x0f);		//ch1 audio input gain init
	nvp6124_i2c_write(0x02, 0x0f);		//ch2 gain	
	nvp6124_i2c_write(0x03, 0x0f);		//ch3 gain	
	nvp6124_i2c_write(0x04, 0x0f);		//ch4 gain	
	nvp6124_i2c_write(0x05, 0x0f);		//mic gain		
	nvp6124_i2c_write(0x40, 0x0f);		//ch5 gain		
	nvp6124_i2c_write(0x41, 0x0f);		//ch6 gain	
	nvp6124_i2c_write(0x42, 0x0f);		//ch7 gain	
	nvp6124_i2c_write(0x43, 0x0f);		//ch8 gain			
	nvp6124_i2c_write(0x22, 0x06);		//aogain 2Vpp				

	nvp6124_i2c_write(0x24, 0x14);		//set mic_1's data to i2s_sp left channel
	nvp6124_i2c_write(0x25, 0x15);		//set mic_2's data to i2s_sp right channel
}
/*------------------------------mode ------------------------------*/
#if 0
void nvp6124_H960_36M(AHD_INFO *pvInfo)
{
	unsigned char read_data = 0;

	//nvp6124_outmode_select(pvInfo);
	nvp6124_960h_clkset(pvInfo);
	xprintf("nvp6124_H960_%s OK\n", pvInfo->vformat? "PAL" : "NTSC");			
}

void nvp6124_720P_74M(AHD_INFO *pvInfo)
{
	//nvp6124_outmode_select(pvInfo);
	nvp6124_720p_clkset(pvInfo);
	xprintf("nvp6124_720p_%s OK\n", pvInfo->vformat? "25p" : "30p");			
}

void nvp6124_1080P_148M(AHD_INFO *pvInfo)
{
	//nvp6124_outmode_select(pvInfo);
	xprintf("nvp6124_1080p_%s OK\n", pvInfo->vformat? "25p" : "30p");			
}
#endif

static void nvp6124_vconf_set(AHD_INFO *pvInfo)
{
	nvp6124_outmode_select(pvInfo);

	if(pvInfo->current_resolution < AHDRX_OUT_960X480 || pvInfo->current_resolution > AHDRX_OUT_1920X1080P25)
	{
		xprintf("loading test pattern\n");	
		pvInfo->rx.no_video_flag = ON;
		pvInfo->rx.current_resolution = RSV;
	}
	else
	{
		pvInfo->rx.no_video_flag = OFF;
	}

	if(pvInfo->rx.current_resolution == AHDRX_OUT_960X480 || pvInfo->rx.current_resolution == AHDRX_OUT_960X576)
	{
		nvp6124_960h_clkset(pvInfo);
	}
	else if(pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25)
	{
		nvp6124_720p_clkset(pvInfo);
	}
	else if(pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P25)
	{
	}
}

unsigned char nvp6124_video_detect(void)
{
	unsigned char isNoVideo, DetectVideo = 0;

	nvp6124_i2c_lock();
	
	nvp6124_i2c_write(0xFF, BANK0);			//select bank0
	nvp6124_i2c_read(0xD0, &DetectVideo);	//read video ch1 mode 

	nvp6124_i2c_unlock();

	return DetectVideo;
}

#if 0
static void nvp6124_common_init(AHD_INFO *pvInfo)
{
	nvp6124_common_init(pvInfo);
	nvp6124_pll_reset();
	Delay1MS(1000);
	pvInfo->current_resolution = nvp6124_video_detect();
}
#endif

/*-----------------------------------------------------------------*/
int do_nvp6124(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	if(argc < 2) return -1;

	if(!strcmp(argv[1], "dump")) {
		int begin_bank = 0;
		int end_bank = 11;

		if(argc > 3) {
			return -1;
		} else if(argc == 3) {
			begin_bank = end_bank = simple_strtoul(argv[2], NULL, 10);
		}

		nvp6124_i2c_lock();
		{
			unsigned char bank;

			xprintf("============AHD RX  Register Dump start============\n");
			xprintf("    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
			for(bank=begin_bank; bank<=end_bank; bank++) {
				unsigned int i;

				xprintf("=======================BANK%d=======================\n", bank);

				nvp6124_i2c_write(0xff, bank);
				for(i=0; i< 0x100; i++) {
					unsigned char data;
					nvp6124_i2c_read(i, &data);
					if(i%16==0) xprintf("%02x: ", i);
					xprintf("%02x%s", data, (i%16==15)? "\n":" ");
				}
			}
		}
		nvp6124_i2c_unlock();

	} else if(!strcmp(argv[1], "reg") && argc >= 3) {
		u16 addr = simple_strtoul(argv[2], NULL, 16);
		u8 bank = (addr >> 8) & 0xf;
		u8 reg = addr & 0xff;

		if(argc == 3) {
			u8 data;
			nvp6124_i2c_lock();
			nvp6124_i2c_write(0xff, bank); 
			nvp6124_i2c_read(reg, &data); 
			nvp6124_i2c_unlock();

			xprintf("RD) reg(%03x), data(%02x)\n", addr, data);
		} else if(argc == 4) {
			u8 data = simple_strtoul(argv[3], NULL, 16);
			nvp6124_i2c_lock();
			nvp6124_i2c_write(0xff, bank); 
			nvp6124_i2c_write(reg, data); 
			nvp6124_i2c_unlock();

			xprintf("WR) reg(%03x), data(%02x)\n", addr, data);
		} else {
			return -1;
		}
	}else if(!strcmp(argv[1], "test") && argc ==3){
		if(!strcmp(argv[2], "on"))			nvp6124_test_pattern(ON); 
		else if(!strcmp(argv[2], "off"))	nvp6124_test_pattern(OFF); 
		
	}
	return 0;
}

SHELL_CMD(
		nvp6124,   CONFIG_SYS_MAXARGS, 1,  do_nvp6124,
		"nvp6124 device control",
		"{option}\n"
		"  reg {addr.3}             - read\n"
		"  reg {addr.3} {data.2}    - write\n"
		"  dump [{bank}]            - dump"
		"  test {auxout/mainout/input} {on/off} 	- test pattern\n" 
		);
