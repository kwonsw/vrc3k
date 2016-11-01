/**
 *	@file   adv7611.c
 *	@brief	
 *	@author luisfynn <tani223@pinetron.com>
 *	@date   2014/11/07 11:12
 */

/* system include */
#include <stdlib.h>
/* local include */
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "adv7611.h"
#include "video.h"
#include "i2c.h"
#include "command.h"
#include "mutex.h"

MUTEX_DECLARE(s_sem_adv7611);

static void adv7611_i2c_lock(void)      { MUTEX_LOCK(s_sem_adv7611);    }
static void adv7611_i2c_unlock(void)    { MUTEX_UNLOCK(s_sem_adv7611);  }

unsigned char ADV7611_Video_Set(AHD_INFO *pvInfo);

static unsigned char adv7611_edid[];
static unsigned short hdmi_resol_parameter[11]= {0, };

unsigned char adv7611_i2c_write(unsigned char dev_id, unsigned char addr, unsigned char data)
{
	unsigned char ret = 1;
	if(I2C1_ByteWrite(dev_id, addr, data) != 1) ret = 0;
	
	return ret;
}

unsigned char adv7611_i2c_read(unsigned char dev_id, unsigned char addr, unsigned char* data)
{
	unsigned char ret = 1;
	if(I2C1_ByteRead(dev_id, addr, data) != 1) ret = 0;
	
	return ret;
}

unsigned short adv7611_i2c_read16(unsigned char dev_id, unsigned char addr, unsigned short mask)
{
	unsigned char read_data = 0;
	unsigned short sum = 0;

	if(adv7611_i2c_read(dev_id, addr, &read_data) != 1) return 0;
	sum = read_data<<8; 	

	if(adv7611_i2c_read(dev_id, addr+1, &read_data) != 1) return 0;
	sum = ( sum | read_data ) & mask ;
	
	return sum;
}

typedef struct HDMI_RX_PARAMETERS
{
	unsigned short	total_line_width;
	unsigned short	line_width;
	unsigned short  hsync_front_porch;
	unsigned short  hsync_pulse_width;
	unsigned short  hsync_back_porch;
	unsigned short	dvi_sync_polarity;
	unsigned short	field_total_height;
	unsigned short	field_height;
	unsigned short	field_front_porch;
	unsigned short	field_pulse_width;
	unsigned short	field_back_porch;
}HDMI_RX_PARA;

HDMI_RX_PARA HDMI_RX_IN_SYNC[MAX_ARRAY_COUNT]=
{
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											//RSV	
	
	{0x35a, 0x2d0, 0x10, 0x3e, 0x3c, 0x0, 0x41a, 0x1e0, 0x12, 0xc, 0x3c},		// HDMI_RX_CEA_720x480p60
	{0x360, 0x2d0, 0x0c, 0x40, 0x44, 0x0, 0x4e2, 0x240, 0xa, 0xa, 0x4e},		// HDMI_RX_CEA_720x576p50
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_CEA_1280x720p24	
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_CEA_1280x720p25
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_CEA_1280x720p30
	{0x7bc, 0x500, 0x1b8, 0x28, 0xdc, 0x0, 0x5dc, 0x2d0, 0xa, 0xa, 0x28},		// HDMI_RX_CEA_1280x720p50
	{0x672, 0x500, 0x6e, 0x28, 0xdc, 0x0, 0x5dc, 0x2d0, 0xa, 0xa, 0x28},		// HDMI_RX_CEA_1280x720p60
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_CEA_1920x1080p24
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_CEA_1920x1080p25
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_CEA_1920x1080p30
	{0xa50, 0x780, 0x210, 0x2c, 0x94, 0x0, 0x464, 0x21c, 0x4, 0xa, 0x1e},		// HDMI_RX_CEA_1920x1080I50
	{0x898, 0x780, 0x58, 0x2c, 0x94, 0x0, 0x464, 0x21c, 0x4, 0xa, 0x1e},		// HDMI_RX_CEA_1920x1080I60
	{0xa50, 0x780, 0x210, 0x2c, 0x94, 0x0, 0x8ca, 0x438, 0x8, 0xa, 0x48},		// HDMI_RX_CEA_1920x1080p50
	{0x898, 0x780, 0x58, 0x2c, 0x94, 0x0, 0x8ca, 0x438, 0x8, 0xa, 0x48},		// HDMI_RX_CEA_1920x1080p60

	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_640X350P85,
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_640X400P85,
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_720X400P85
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_640X480P60
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_640X480P72
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_640X480P75
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_640X480P85
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_800X600P56
	{0x420, 0x320, 0x28, 0x80, 0x58, 0x1, 0x4e8, 0x258, 0x2, 0x8, 0x2e},		// HDMI_RX_DMT_800X600P60
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_800X600P72
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_800X600P75
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_800X600P85
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_848X480P60
	{0x540, 0x400, 0x18, 0x88, 0xa0, 0, 0x64c, 0x300, 0x6, 0xc, 0x3a},			// HDMI_RX_DMT_1024X768P60
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1024X768P70
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1024X768P75
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1024X768P85
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1152X864P75
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1280X768P60_RB
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1280X768P60
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1280X768P75
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1280X768P85
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1280X800P60_RB
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1280X800P60
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1280X800P75
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1280X800P85
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1280X960P60
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1280X960P85
	{0x698, 0x500, 0x30, 0x70, 0xf8, 0x1, 0x854, 0x400, 0x2, 0x6, 0x4c},		// HDMI_RX_DMT_1280X1024P60
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1280X1024P75
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1280X1024P85
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1360X768P60
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1400X1050P60_RB
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1400X1050P60
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1400X1050P75
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1400X1050P85
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1440X900P60_RB
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1440X900P60
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1600X1200P60
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1680X1050P60_RB
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1680X1050P60
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1792X1344P60
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1856X1392P60
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1920X1200P60_RB
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1366X768P60_RB
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1366X768P60
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},											// HDMI_RX_DMT_1920X1080P60
};

static unsigned char ADV7611_Map_Init(void)
{
	unsigned short i;

	MUTEX_INIT(s_sem_adv7611);

	adv7611_i2c_lock();
	adv7611_i2c_write(IO_MAP_ADDR, 0xff, 0x80); 		//sw reset
	Delay1MS(5);
    
   	adv7611_i2c_write(IO_MAP_ADDR, 0xf4, CEC_MAP_ADDR);				//cec map setting 
   	adv7611_i2c_write(IO_MAP_ADDR, 0xf5, INFORFRAME_MAP_ADDR);		//inforframe map setting 
   	adv7611_i2c_write(IO_MAP_ADDR, 0xf8, DPLL_MAP_ADDR);			//dpll map setting 
   	adv7611_i2c_write(IO_MAP_ADDR, 0xf9, KSV_MAP_ADDR);				//ksv map setting 
   	adv7611_i2c_write(IO_MAP_ADDR, 0xfa, EDID_MAP_ADDR);			//edid map setting 
   	adv7611_i2c_write(IO_MAP_ADDR, 0xfb, HDMI_MAP_ADDR);			//hdmi map setting 
   	adv7611_i2c_write(IO_MAP_ADDR, 0xfd, CP_MAP_ADDR);				//cp map setting 

	//Start EDID sequence
	adv7611_i2c_write(KSV_MAP_ADDR, 0x77, 0x00);			

	for(i = 0; i < 0x100; i++)
	{
   		adv7611_i2c_write(EDID_MAP_ADDR, i, adv7611_edid[i]);		//Write EDID	
	}

	adv7611_i2c_write(KSV_MAP_ADDR, 0x77, 0x00);		// Set the Most Significant Bit of the SPA location to 0 	
	adv7611_i2c_write(KSV_MAP_ADDR, 0x70, 0x9e);		// Set the Least Significant Byte of the SPA location    	
	adv7611_i2c_write(KSV_MAP_ADDR, 0x74, 0x03);		// Enable the Internal EDID for Ports                    	
	//End of EDID sequence

   	adv7611_i2c_write(IO_MAP_ADDR, 0x02, 0xf5);		//input color space depends on color space reported by HDMI block 
	adv7611_i2c_unlock();

	//--------------vrc3k indivisual setting for audio-----------//
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOB, GPIO_Pin_3);		
	GPIO_SetBits(GPIOB, GPIO_Pin_4);		
	//-----------------------------------------------------------//
}

unsigned char ADV7611_Auto_Detect(AHD_INFO *pvInfo)
{
	unsigned char  de_regen_filter_locked, de_regen_lck_raw = 0;
	unsigned short *column;	
	unsigned char current_column, isEqual, current_row =0;
	
	adv7611_i2c_lock();
	ADV7611_Video_Set(pvInfo);			//first init

	HDMI_RX_PARA *pPara = NULL;
	pPara = (HDMI_RX_PARA*)&HDMI_RX_IN_SYNC;
	column = &pPara->total_line_width;	

	do{
		adv7611_i2c_read(HDMI_MAP_ADDR, 0x07, &de_regen_filter_locked);
		adv7611_i2c_read(HDMI_MAP_ADDR, 0x6a, &de_regen_lck_raw);
	
		de_regen_filter_locked = (de_regen_filter_locked >> 5) & 0x5;
		de_regen_lck_raw = de_regen_lck_raw & 0x3;	

	//	xprintf("de_regen_filter_locked: %02x de_regen_lck_raw: %02x\n", de_regen_filter_locked, de_regen_lck_raw);
	}while(de_regen_lck_raw != 0x3 && de_regen_filter_locked != 0x5);	

	//read Video input parameter
	hdmi_resol_parameter[0] = adv7611_i2c_read16(HDMI_MAP_ADDR, 0x1E, 0x3fff );		//total_line_width
	hdmi_resol_parameter[1] = adv7611_i2c_read16(HDMI_MAP_ADDR, 0x07, 0x1fff );		//line_width
	hdmi_resol_parameter[2] = adv7611_i2c_read16(HDMI_MAP_ADDR, 0x20, 0x1fff );		//hsync_front_porch
	hdmi_resol_parameter[3] = adv7611_i2c_read16(HDMI_MAP_ADDR, 0x22, 0x1fff );		//hsync_pulse_width
	hdmi_resol_parameter[4] = adv7611_i2c_read16(HDMI_MAP_ADDR, 0x24, 0x1fff );		//hsync_back_porch
	hdmi_resol_parameter[5] = adv7611_i2c_read16(HDMI_MAP_ADDR, 0x05, 0x30 );		//dvi_hsync_polarity & dvi_hsync_polarity
	hdmi_resol_parameter[5] = (hdmi_resol_parameter[5]>>5) & 0x3;	
	hdmi_resol_parameter[6] = adv7611_i2c_read16(HDMI_MAP_ADDR, 0x26, 0x3fff );		//field_total_height
	hdmi_resol_parameter[7] = adv7611_i2c_read16(HDMI_MAP_ADDR, 0x09, 0x1fff );		//field_height
	hdmi_resol_parameter[8] = adv7611_i2c_read16(HDMI_MAP_ADDR, 0x2a, 0x3fff );		//field_front_porch
	hdmi_resol_parameter[9] = adv7611_i2c_read16(HDMI_MAP_ADDR, 0x2e, 0x3fff );		//field_pulse_width
	hdmi_resol_parameter[10] = adv7611_i2c_read16(HDMI_MAP_ADDR, 0x32, 0x3fff );	//filed_back_porch

#if 0
	unsigned char j =0;
	xprintf("HDMI Video input parameter: ");
	for(j=0; j<0xb; j++)	xprintf("%04x%s", hdmi_resol_parameter[j], j%0xb == 0xa ? "\n" : " ");
#endif

	while(1){

		isEqual=0;
		current_column = 0;

		for(column = &pPara->total_line_width; column< &pPara->field_back_porch+1; column += 1 ){
#if	0 
			xprintf("column addr %04x value %04x \n", column, *column);	
			xprintf(" pPara->total_line_width addr %04x\n", &pPara->total_line_width);	
			xprintf(" pPara->total_line_width addr %04x\n", &pPara->line_width);	
			xprintf("*column : %x, current column : %x,  hdmi_resol_parameter: %x\n", *column, current_column,  hdmi_resol_parameter[current_column]); 
#endif
			if(*column == hdmi_resol_parameter[current_column])
			{
				isEqual += 1;
			}
			current_column += 1;
		}

		if(isEqual == 0xb)
		{
			break;
		}
		else if(current_row > MAX_ARRAY_COUNT-1)
		{
			break;
		}
		else
		{
			pPara += 1;
			current_row += 1;
		}
	}

	pvInfo->current_resolution = current_row;
//	xprintf("current resolution %x\n", pvInfo->current_resolution);
	adv7611_i2c_unlock();
}

unsigned char ADV7611_Init(AHD_INFO *pvInfo)
{
	// mode						high							low
	//---------------------------------------------------------------------------------
	//pvInfo->app_mode 		1(high):repeater mode 	 		0(low): converter mode
	//pvInfo->input_mode 	1(high):hdmi input mode   		0(low): analog video input mode
	//pvInfo->output_mode 	1(high):full hd output mode   	0(low): hd output mode
	//pvInfo->vformat 		1(high):25p or PAL  			0(low): 30p or NTSC 
	//---------------------------------------------------------------------------------

	unsigned short nID = 0;

	while(nID != 0x2051){	
		nID = adv7611_i2c_read16(IO_MAP_ADDR, 0xea, 0xffff);	
		xprintf("HDMI RX ID:  %s\n", (nID == 0x2051)? "ADV7611" : "Detection error");
	}

	ADV7611_Map_Init();
	
	if(pvInfo->input_mode)
	{
		adv7611_i2c_read(IO_MAP_ADDR, 0x21, &pvInfo->rx.hpa_status_port_a);
		pvInfo->rx.hpa_status_port_a = ( pvInfo->rx.hpa_status_port_a >>3 ) & 0x1;
	
		if(pvInfo->rx.hpa_status_port_a)
		{
			Delay1MS(100);
			ADV7611_Auto_Detect(pvInfo);
		}
		ADV7611_Video_Set(pvInfo);	
	}

	LedControl(RX_TYPE, FORWARD);
	xprintf("adv7611 init success\n");
}

static unsigned char adv7611_vfreq_set(AHD_INFO *pvInfo)
{
	//prim mode & vid_std set
	adv7611_i2c_lock();

	switch(pvInfo->rx.current_resolution){

		case HDMI_RX_CEA_1280X720P24: case HDMI_RX_CEA_1920X1080P24:
			adv7611_i2c_write(IO_MAP_ADDR, 0x01, 0x45);
			adv7611_i2c_write(IO_MAP_ADDR, 0x00, 0x1e);
			break;
		case HDMI_RX_CEA_1280X720P25: case HDMI_RX_CEA_1920X1080P25:
			adv7611_i2c_write(IO_MAP_ADDR, 0x01, 0x35);
			adv7611_i2c_write(IO_MAP_ADDR, 0x00, 0x1e);
			break;
		case HDMI_RX_CEA_1280X720P30: case HDMI_RX_CEA_1920X1080P30:
			adv7611_i2c_write(IO_MAP_ADDR, 0x01, 0x25);
			adv7611_i2c_write(IO_MAP_ADDR, 0x00, 0x1e);
			break;
		case HDMI_RX_CEA_720X576P50: case HDMI_RX_CEA_1280X720P50:
		case HDMI_RX_CEA_1920X1080P50:
			adv7611_i2c_write(IO_MAP_ADDR, 0x01, 0x15);
			adv7611_i2c_write(IO_MAP_ADDR, 0x00, 0x1e);
			break;
		case HDMI_RX_CEA_720X480P60: case HDMI_RX_CEA_1280X720P60:
		case HDMI_RX_CEA_1920X1080P60:
			adv7611_i2c_write(IO_MAP_ADDR, 0x01, 0x05);
			adv7611_i2c_write(IO_MAP_ADDR, 0x00, 0x1e);
			break;
		case HDMI_RX_DMT_800X600P60:
			adv7611_i2c_write(IO_MAP_ADDR, 0x01, 0x06);
			adv7611_i2c_write(IO_MAP_ADDR, 0x00, 0x01);
			break;
		case HDMI_RX_DMT_1024X768P60:
			adv7611_i2c_write(IO_MAP_ADDR, 0x01, 0x06);
			adv7611_i2c_write(IO_MAP_ADDR, 0x00, 0x0c);
			break;
		case HDMI_RX_DMT_1280X1024P60:
			adv7611_i2c_write(IO_MAP_ADDR, 0x01, 0x06);
			adv7611_i2c_write(IO_MAP_ADDR, 0x00, 0x05);
			break;
		default:
			adv7611_i2c_write(IO_MAP_ADDR, 0x01, 0x05);
			adv7611_i2c_write(IO_MAP_ADDR, 0x00, 0x1e);
			break;
	}

	adv7611_i2c_unlock();
}

static unsigned char adv7611_vclk_dly(AHD_INFO *pvInfo)
{
	adv7611_i2c_lock();
	switch(pvInfo->rx.current_resolution){

		case HDMI_RX_CEA_1280X720P50:
		case HDMI_RX_CEA_1280X720P60:
		case HDMI_RX_CEA_1920X1080I50:
		case HDMI_RX_CEA_1920X1080I60:
		case HDMI_RX_DMT_1024X768P60:
			adv7611_i2c_write(IO_MAP_ADDR, 0x19, 0x80);
			break;
		default:
			adv7611_i2c_write(IO_MAP_ADDR, 0x19, 0x89);
			break;
	}
	adv7611_i2c_unlock();
}

unsigned char ADV7611_Video_Set(AHD_INFO *pvInfo)
{
	unsigned char i;

	adv7611_i2c_lock();

	//	xprintf("HDMI input resolution number: %x\n", pvInfo->rx.resolution);
	pvInfo->rx.current_resolution = pvInfo->current_resolution;
	adv7611_vfreq_set(pvInfo);			//mode & vclk-out delay set

   	adv7611_i2c_write(IO_MAP_ADDR, 0x02, 0xf5);		//input color space depends on color space reported by HDMI block 
	adv7611_i2c_write(IO_MAP_ADDR, 0x03, 0x80);
	adv7611_i2c_write(IO_MAP_ADDR, 0x04, 0x60);
	adv7611_i2c_write(IO_MAP_ADDR, 0x05, 0x2c);
	
	adv7611_i2c_write(IO_MAP_ADDR, 0x0b, 0x44);
	adv7611_i2c_write(IO_MAP_ADDR, 0x0c, 0x42);
	
	adv7611_i2c_write(IO_MAP_ADDR, 0x14, 0x7f);
	adv7611_i2c_write(IO_MAP_ADDR, 0x15, 0x80);
	
	adv7611_vclk_dly(pvInfo);	
	
	adv7611_i2c_write(IO_MAP_ADDR, 0x33, 0x40);
	adv7611_i2c_write(IO_MAP_ADDR, 0xba, 0x01);
	adv7611_i2c_write(IO_MAP_ADDR, 0x40, 0x81);
	
	adv7611_i2c_write(HDMI_MAP_ADDR, 0x9b, 0x03);
	
	for(i=0; i<12; i++)
	{
		adv7611_i2c_write(HDMI_MAP_ADDR, 0xc1+i, 0x01);			//recommended setting
	}

	adv7611_i2c_write(HDMI_MAP_ADDR, 0x00, 0x00);
	adv7611_i2c_write(HDMI_MAP_ADDR, 0x83, 0xfe);
	adv7611_i2c_write(HDMI_MAP_ADDR, 0x6f, 0x0c);
	adv7611_i2c_write(HDMI_MAP_ADDR, 0x85, 0x1f);
	adv7611_i2c_write(HDMI_MAP_ADDR, 0x87, 0x70);
	adv7611_i2c_write(HDMI_MAP_ADDR, 0x8d, 0x04);
	adv7611_i2c_write(HDMI_MAP_ADDR, 0x8e, 0x1e);
	adv7611_i2c_write(HDMI_MAP_ADDR, 0x1a, 0x8a);
	adv7611_i2c_write(HDMI_MAP_ADDR, 0x57, 0xda);
	adv7611_i2c_write(HDMI_MAP_ADDR, 0x58, 0x01);
	adv7611_i2c_write(HDMI_MAP_ADDR, 0x03, 0x98);	//I2S bit[4:0]  : 24bit,   MODE[6:5] : I2S mode
	adv7611_i2c_write(HDMI_MAP_ADDR, 0x75, 0x10);
	
	adv7611_i2c_unlock();
}

static unsigned char adv7611_edid[] = {
	//	      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
	/*0x00*/  0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x52, 0x74, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,      
	/*0x10*/  0x00, 0x00, 0x01, 0x03, 0x80, 0x38, 0x20, 0x78, 0x2A, 0xE5, 0xB5, 0xA3, 0x55, 0x49, 0x99, 0x27, 
	/*0x20*/  0x13, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0xC0, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	/*0x30*/  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C, 
	/*0x40*/  0x45, 0x00, 0x80, 0x68, 0x21, 0x00, 0x00, 0x1E, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 
	/*0x50*/  0x58, 0x2C, 0x45, 0x00, 0x80, 0x68, 0x21, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x54, 
	/*0x60*/  0x45, 0x53, 0x54, 0x20, 0x54, 0x56, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFD, 
	/*0x70*/  0x00, 0x38, 0x3E, 0x1E, 0x44, 0x0F, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0xDF, 
	/*0x80*/  0x02, 0x03, 0x1C, 0x70, 0x46, 0x10, 0x1F, 0x04, 0x13, 0x14, 0x05, 0x26, 0x09, 0x57, 0x03, 0x15, 
	/*0x90*/  0x07, 0x50, 0x83, 0x01, 0x00, 0x00, 0x65, 0x03, 0x0C, 0x00, 0x11, 0x00, 0x01, 0x1D, 0x80, 0x18, 
	/*0xa0*/  0x71, 0x1C, 0x16, 0x20, 0x58, 0x2C, 0x25, 0x00, 0x80, 0x68, 0x01, 0x00, 0x00, 0x9E, 0x01, 0x1D, 
	/*0xb0*/  0x00, 0x72, 0x51, 0xD0, 0x1E, 0x20, 0x6E, 0x28, 0x55, 0x00, 0x80, 0x68, 0x01, 0x00, 0x00, 0x1E, 
	/*0xc0*/  0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00, 0x80, 0x68, 0x01, 0x00, 
	/*0xd0*/  0x00, 0x18, 0x8C, 0x0A, 0xA0, 0x14, 0x51, 0xF0, 0x16, 0x00, 0x26, 0x7C, 0x43, 0x00, 0x80, 0x68, 
	/*0xe0*/  0x01, 0x00, 0x00, 0x98, 0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00, 
	/*0xf0*/  0x90, 0x2C, 0x01, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 
};

int do_adv7611(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	if(argc < 2) return -1;

	if(!strcmp(argv[1], "dump") && argc == 2) {
		adv7611_i2c_lock();
		{
			unsigned char page;
			unsigned int reg;

			page = IO_MAP_ADDR;

			xprintf("============AHD TX  Register Dump start============\n");
			xprintf("    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
			xprintf("=======================%s=======================\n", IO_MAP_ADDR );

			for(reg=0; reg< 0x100; reg++) {
				unsigned char data;
				adv7611_i2c_read(page, reg, &data);
				if(reg%16==0) xprintf("%02x: ", reg); xprintf("%02x%s", data, (reg%16==15)? "\n":" ");
			}

			page = CEC_MAP_ADDR;

			xprintf("============AHD TX  Register Dump start============\n");
			xprintf("    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
			xprintf("=======================%s=======================\n", CEC_MAP_ADDR );

			for(reg=0; reg< 0x100; reg++) {
				unsigned char data;
				adv7611_i2c_read(page, reg, &data);
				if(reg%16==0) xprintf("%02x: ", reg); xprintf("%02x%s", data, (reg%16==15)? "\n":" ");
			}

			page = INFORFRAME_MAP_ADDR;

			xprintf("============AHD TX  Register Dump start============\n");
			xprintf("    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
			xprintf("=======================%s=======================\n", INFORFRAME_MAP_ADDR );

			for(reg=0; reg< 0x100; reg++) {
				unsigned char data;
				adv7611_i2c_read(page, reg, &data);
				if(reg%16==0) xprintf("%02x: ", reg); xprintf("%02x%s", data, (reg%16==15)? "\n":" ");
			}

			page = DPLL_MAP_ADDR;

			xprintf("============AHD TX  Register Dump start============\n");
			xprintf("    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
			xprintf("=======================%s=======================\n", DPLL_MAP_ADDR );

			for(reg=0; reg< 0x100; reg++) {
				unsigned char data;
				adv7611_i2c_read(page, reg, &data);
				if(reg%16==0) xprintf("%02x: ", reg); xprintf("%02x%s", data, (reg%16==15)? "\n":" ");
			}

			page = KSV_MAP_ADDR;

			xprintf("============AHD TX  Register Dump start============\n");
			xprintf("    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
			xprintf("=======================%s=======================\n", KSV_MAP_ADDR );

			for(reg=0; reg< 0x100; reg++) {
				unsigned char data;
				adv7611_i2c_read(page, reg, &data);
				if(reg%16==0) xprintf("%02x: ", reg); xprintf("%02x%s", data, (reg%16==15)? "\n":" ");
			}

			page = EDID_MAP_ADDR;

			xprintf("============AHD TX  Register Dump start============\n");
			xprintf("    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
			xprintf("=======================%s=======================\n", EDID_MAP_ADDR );

			for(reg=0; reg< 0x100; reg++) {
				unsigned char data;
				adv7611_i2c_read(page, reg, &data);
				if(reg%16==0) xprintf("%02x: ", reg); xprintf("%02x%s", data, (reg%16==15)? "\n":" ");
			}

			page = HDMI_MAP_ADDR;

			xprintf("============AHD TX  Register Dump start============\n");
			xprintf("    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
			xprintf("=======================%s=======================\n", HDMI_MAP_ADDR );

			for(reg=0; reg< 0x100; reg++) {
				unsigned char data;
				adv7611_i2c_read(page, reg, &data);
				if(reg%16==0) xprintf("%02x: ", reg); xprintf("%02x%s", data, (reg%16==15)? "\n":" ");
			}

			page = CP_MAP_ADDR;

			xprintf("============AHD TX  Register Dump start============\n");
			xprintf("    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
			xprintf("=======================%s=======================\n", CP_MAP_ADDR );

			for(reg=0; reg< 0x100; reg++) {
				unsigned char data;
				adv7611_i2c_read(page, reg, &data);
				if(reg%16==0) xprintf("%02x: ", reg); xprintf("%02x%s", data, (reg%16==15)? "\n":" ");
			}
		}
		adv7611_i2c_unlock();

	} else if(!strcmp(argv[1], "reg") && argc >= 3) {
		u16 addr = simple_strtoul(argv[2], NULL, 16);
		u8 page = (addr >> 8) & 0xff;
		u8 reg = addr & 0xff;

		if(argc == 3) {
			u8 data;

			adv7611_i2c_lock();
			adv7611_i2c_read(page, reg, &data);
			adv7611_i2c_unlock();

			xprintf("RD)page(%02x) reg(%02x), data(%02x)\n", page, reg, data);
		} else if(argc == 4) {
			u8 data = simple_strtoul(argv[3], NULL, 16);

			adv7611_i2c_lock();
			adv7611_i2c_write(page, reg, data);
			adv7611_i2c_unlock();

			xprintf("WR)page(%02x) reg(%02x), data(%02x)\n", page, reg, data);
		} else {
			return -1;
		}
	} else if(!strcmp(argv[1], "parameter") && argc == 2){

		unsigned char j =0;
		xprintf("HDMI Video input parameter: ");
		for(j=0; j<0xb; j++)	xprintf("%04x%s", hdmi_resol_parameter[j], j%0xb == 0xa ? "\n" : " ");
	}
	return 0;
}

SHELL_CMD(
	adv7611,   CONFIG_SYS_MAXARGS, 1,  do_adv7611,
	"adv7611 device control",
	"{option}\n"
	"  reg {addr.4}             - read\n"
	"  reg {addr.4} {data.2}   - write\n"
	"  dump                    - dump"
	"  parameter               - show input resolution parameter"
);



//#endif
