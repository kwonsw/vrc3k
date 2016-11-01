/**
 *	@file   video380.c
 *	@brief	
 *	@author luisfynn <tani223@pinetron.com>
 *	@date   2014/10/14 14:45
 */

/* system include */
#include <stdlib.h>
#include <string.h>
/* local include */
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "mdin3xx.h"
#include "video.h"
#include "adv7611.h"
#include "i2c.h"
#include "command.h"
#include "mutex.h"

MUTEX_DECLARE(s_sem_mdin380);

static MDIN_VIDEO_INFO		stVideo;
static MDIN_INTER_WINDOW	stInterWND;

static void mdin380_i2c_lock(void)      { MUTEX_LOCK(s_sem_mdin380);    }
static void mdin380_i2c_unlock(void)    { MUTEX_UNLOCK(s_sem_mdin380);  }

BYTE AdjInterWND,  InputSelect, InputSelOld,  SrcSyncInfo;
BYTE SrcMainFrmt, PrevSrcMainFrmt, SrcMainMode, PrevSrcMainMode;
BYTE OutMainFrmt, PrevOutMainFrmt, OutMainMode, PrevOutMainMode;
BYTE SrcAuxFrmt, PrevSrcAuxFrmt, SrcAuxMode, PrevSrcAuxMode;
BYTE OutAuxFrmt, PrevOutAuxFrmt, OutAuxMode, PrevOutAuxMode;
BYTE AdcVideoFrmt, PrevAdcFrmt, EncVideoFrmt, PrevEncFrmt;
BOOL fSyncParsed;

BYTE MDINI2C_Read(BYTE nID, WORD rAddr, PBYTE pBuff, WORD bytes);
BYTE MDINI2C_Write(BYTE nID, WORD rAddr, PBYTE pBuff, WORD bytes);

// ----------------------------------------------------------------------
// External Variable 
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Static Prototype Functions
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Static functions
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Exported functions
// ----------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------------

MDIN_ERROR_t MDINDLY_5uSec(WORD delay)
{
	Delay5US(delay);
	return MDIN_NO_ERROR;
}

MDIN_ERROR_t MDINDLY_10uSec(WORD delay)
{
	Delay5US(2*delay);
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINDLY_mSec(WORD delay)
{
	Delay1MS(delay);
	return MDIN_NO_ERROR;
}

static void MDIN3xx_SetHCLKMode(MDIN_HOST_CLK_MODE_t mode)
{
	switch (mode) {
		case MDIN_HCLK_CRYSTAL:	TEST_MODE2( LOW); TEST_MODE1( LOW); break;
		case MDIN_HCLK_MEM_DIV: TEST_MODE2(HIGH); TEST_MODE1(HIGH); break;

#if	defined(SYSTEM_USE_MDIN380)	
		case MDIN_HCLK_HCLK_IN: TEST_MODE2( LOW); TEST_MODE1(HIGH); break;
#endif	
	}
}

void mdin3xx_i2c_dump(void)
{
	WORD i, write_data, read_data;

	xprintf("\n=====MDIN Host Register Dump start=====\n");	
	xprintf("       00   01   02   03   04   05   06   07   08   09   0A   0B   0C   0D   0E   0F\n");	
	xprintf("====================================================================================\n");	

	for(i=0; i<0x100; i++)
	{
		MDINI2C_RegRead(0xc0, i, &read_data);
		
		if(i%16==0) xprintf("%04x: ", i);
		xprintf("%04x%s", read_data, (i%16==15)? "\n":" ");
	}

	xprintf("\n=====MDIN Local Register Dump start=====\n");	
	xprintf("       00   01   02   03   04   05   06   07   08   09   0A   0B   0C   0D   0E   0F\n");	
	xprintf("====================================================================================\n");	

	for(i=0; i<0x400; i++)
	{
		MDINI2C_RegRead(0xc2, i, &read_data);
		
		if(i%16==0) xprintf("%04x: ", i);
		xprintf("%04x%s", read_data, (i%16==15)? "\n":" ");
	}
	
	xprintf("\n=====MDIN Hdmi Register Dump start=====\n");	
	xprintf("       00   01   02   03   04   05   06   07   08   09   0A   0B   0C   0D   0E   0F\n");	
	xprintf("====================================================================================\n");	

	for(i=0; i<0x200; i++)
	{
		MDINI2C_RegRead(0xc4, i, &read_data);
		
		if(i%16==0) xprintf("%04x: ", i);
		xprintf("%04x%s", read_data, (i%16==15)? "\n":" ");
	}
}

void MDIN_I2C_TEST(void)
{
	/*MDIN I2C Read/Write Test*/
	WORD write_data, read_data;

	xprintf("======test routine start ==============\r\n");	
	while(1){
		/*read HOST addr 0x0 value 0x85*/
		write_data = 0x0;
		MDINI2C_Write(I2C_MDIN3xx_ADDR, 0x400, (PBYTE)&write_data, 2);

		read_data = 0;
		MDINI2C_Read(I2C_MDIN3xx_ADDR, 0x000, (PBYTE)&read_data, 2);
		xprintf("test_1 %0s\n", read_data == 0x85? "Read Success" : "Read Fail");

		/*read HOST addr 0x4 value 0x92(mdin-380)*/
		write_data = 0x0;
		MDINI2C_Write(I2C_MDIN3xx_ADDR, 0x400, (PBYTE)&write_data, 2);

		read_data = 0;
		MDINI2C_Read(I2C_MDIN3xx_ADDR, 0x004, (PBYTE)&read_data, 2);
		xprintf("test_2 %0s\n", read_data == 0x92? "Read Success" : "Read Fail");

		/*read HDMI addr 0x2 value 0x9736*/
		write_data = 0x0202;
		MDINI2C_Write(I2C_MDIN3xx_ADDR, 0x400, (PBYTE)&write_data, 2);

		read_data = 0;
		MDINI2C_Read(I2C_MDIN3xx_ADDR, 0x002/2, (PBYTE)&read_data, 2);
		xprintf("test_3 %0s\n", read_data == 0x9736? "Read Success" : "Read Fail");
	}
	/*End of MDIN I2C Read/Write Test*/
}

void MDIN3xx_Init(AHD_INFO *pvInfo)
{
	WORD nID = 0;
	BYTE vformat, resolution;

	MUTEX_INIT(s_sem_mdin380);
	GPIO_InitTypeDef GPIO_InitStructure;

	//MDIN_I2C_TEST();		//MDIN I2C READ/WRITE TEST

	// mode						high							low
	//---------------------------------------------------------------------------------
	//pvInfo->app_mode 		1(high):repeater mode 	 		0(low): converter mode
	//pvInfo->input_mode 	1(high):hdmi input mode   		0(low): analog video input mode
	//pvInfo->output_mode 	1(high):full hd output mode   	0(low): hd output mode
	//pvInfo->vformat 		1(high):25p or PAL  			0(low): 30p or NTSC 
	//---------------------------------------------------------------------------------
	if( pvInfo->vformat == NTSC)	vformat = VID_VENC_NTSC_M;
	else							vformat = VID_VENC_PAL_B; 

	if(pvInfo->app_mode)		 
	{
		if(pvInfo->rx.no_video_flag)
		{
			if(vformat == VID_VENC_PAL_B)			resolution = VIDSRC_1920x1080p25;
			else if(vformat == VID_VENC_NTSC_M)		resolution = VIDSRC_1920x1080p30;
		}
		else
		{
			if		( pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30)	resolution = VIDSRC_1280x720p30;
			else if	( pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25)	resolution = VIDSRC_1280x720p25;
			else if	( pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P30)	resolution = VIDSRC_1920x1080p30;
			else if	( pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P25)	resolution = VIDSRC_1920x1080p25;
		}
	}
	else if(!pvInfo->input_mode)		
	{
		if(pvInfo->rx.no_video_flag)
		{
			if(vformat == VID_VENC_PAL_B)			resolution = VIDSRC_1920x1080p25;
			else if(vformat == VID_VENC_NTSC_M)		resolution = VIDSRC_1920x1080p30;
		}
		else{
			if		( pvInfo->rx.current_resolution == AHDRX_OUT_960X480)		resolution = VIDSRC_960x480i60;
			else if	( pvInfo->rx.current_resolution == AHDRX_OUT_960X576)		resolution = VIDSRC_960x576i50;
			else if	( pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30)	resolution = VIDSRC_1280x720p30;
			else if	( pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25)	resolution = VIDSRC_1280x720p25;
			else if	( pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P30)	resolution = VIDSRC_1920x1080p30;
			else if	( pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P25)	resolution = VIDSRC_1920x1080p25;
		}	
	}
	else				
	{
		if(!pvInfo->rx.hpa_status_port_a)			//unpluged HDMI cable		
		{
			if(vformat == VID_VENC_PAL_B)			resolution = VIDSRC_1920x1080p50;
			else if(vformat == VID_VENC_NTSC_M)		resolution = VIDSRC_1920x1080p60;
			
			pvInfo->tx.current_resolution = pvInfo->output_mode;
		}
		else
		{
			if		( pvInfo->rx.current_resolution == HDMI_RX_CEA_1280X720P60 )	resolution = VIDSRC_1280x720p60;
			else if	( pvInfo->rx.current_resolution == HDMI_RX_CEA_1280X720P50 )	resolution = VIDSRC_1280x720p50;
			else if	( pvInfo->rx.current_resolution == HDMI_RX_CEA_1920X1080P60 )	resolution = VIDSRC_1920x1080p60;
			else if	( pvInfo->rx.current_resolution == HDMI_RX_CEA_1920X1080P50 )	resolution = VIDSRC_1920x1080p50;
			else if	( pvInfo->rx.current_resolution == HDMI_RX_CEA_1920X1080I50 )	resolution = VIDSRC_1920x1080i50;
			else if	( pvInfo->rx.current_resolution == HDMI_RX_CEA_1920X1080I60 )	resolution = VIDSRC_1920x1080i60;
			else if	( pvInfo->rx.current_resolution == HDMI_RX_CEA_720X480P60 )		resolution = VIDSRC_720x480p60;
			else if	( pvInfo->rx.current_resolution == HDMI_RX_CEA_720X576P50 )		resolution = VIDSRC_720x576p50;
			else if	( pvInfo->rx.current_resolution == HDMI_RX_DMT_800X600P60 )		resolution = VIDSRC_800x600p60;
			else if	( pvInfo->rx.current_resolution == HDMI_RX_DMT_1024X768P60 )	resolution = VIDSRC_1024x768p60;
			else if	( pvInfo->rx.current_resolution == HDMI_RX_DMT_1280X1024P60 )	resolution = VIDSRC_1280x1024p60;

			pvInfo->tx.current_resolution = pvInfo->output_mode;
		}
	}
	/*=======================================================================================*/

#if	!defined(SYSTEM_USE_PCI_HIF)&&defined(SYSTEM_USE_MCLK202)
	MDIN3xx_SetHCLKMode(MDIN_HCLK_CRYSTAL);	// set HCLK to XTAL
	MDINDLY_mSec(50);						// delay 50ms
#endif
	while (nID!=0x85){
		MDIN3xx_GetChipID(&nID);	// get chip-id
		xprintf("MDIN IC : %s(%04x)\r\n",nID == 0x85? "MDIN380" : "detecting...", nID);
	}
	MDIN3xx_EnableMainDisplay(OFF);		// set main display off
	MDIN3xx_SetMemoryConfig();			// initialize DDR memory

#if	!defined(SYSTEM_USE_PCI_HIF)&&defined(SYSTEM_USE_MCLK202)
	MDIN3xx_SetHCLKMode(MDIN_HCLK_MEM_DIV);	// set HCLK to MCLK/2
	MDINDLY_mSec(10);	// delay 10ms
#endif
	MDIN3xx_SetVCLKPLLSource(MDIN_PLL_SOURCE_XTAL);	// set PLL source
	MDIN3xx_EnableClockDrive(MDIN_CLK_DRV_ALL, ON);

	MDIN3xx_SetInDataMapMode(MDIN_IN_DATA24_MAP3);	// set 24pin mode in-data map0 

	if(pvInfo->app_mode)
	{
		MDIN3xx_SetDIGOutMapMode(MDIN_DIG_OUT_M_MAP12);	// main -> digital out enable
	}
	else if(!pvInfo->input_mode)
	{
		MDIN3xx_SetDIGOutMapMode(MDIN_DIG_OUT_X_MAP12);	// main -> hdmi out enable
	}
	else
	{
		MDIN3xx_SetDIGOutMapMode(MDIN_DIG_OUT_M_MAP12); // main -> digital out enable
	}

	MDINOSD_SetBGLayerColor(RGB(128,128,128));		// set BG-Layer color
	MDINOSD_SetBGBoxColor(RGB(0,64,128));			// set BG-BOX color
	MDINOSD_SetAuxBGColor();  						// aux background color;
	
	MDIN3xx_SetFrontNRFilterCoef(NULL);		// set default frontNR filter coef
	MDINAUX_SetFrontNRFilterCoef(NULL);		// set default frontNR filter coef
	MDIN3xx_SetColorEnFilterCoef(NULL);		// set default color enhancer coef
	MDIN3xx_SetBlockNRFilterCoef(NULL);		// set default blockNR filter coef
	MDIN3xx_SetMosquitFilterCoef(NULL);		// set default mosquit filter coef
	MDIN3xx_SetColorTonFilterCoef(NULL);	// set default colorton filter coef
	MDIN3xx_SetPeakingFilterCoef(NULL);		// set default peaking filter coef

	MDIN3xx_EnableLTI(OFF);					// set LTI off
	MDIN3xx_EnableCTI(OFF);					// set CTI off

	if(pvInfo->app_mode)				
	{
	}
	else if(!pvInfo->input_mode)	
	{
	}
	else					
	{
		MDIN3xx_SetPeakingFilterLevel(0xff);
	}

	MDIN3xx_EnablePeakingFilter(ON);		// set peaking on
	MDIN3xx_EnableFrontNRFilter(OFF);		// set frontNR off
	MDIN3xx_EnableBWExtension(OFF);			// set B/W extension off
	
	MDIN3xx_SetIPCBlock();		// initialize IPC block (3DNR gain is 37)

	memset(&stVideo, 0, sizeof(MDIN_VIDEO_INFO));
	MDIN3xx_SetMFCHYFilterCoef(&stVideo, NULL);	// set default MFC filters
	MDIN3xx_SetMFCHCFilterCoef(&stVideo, NULL); 
	MDIN3xx_SetMFCVYFilterCoef(&stVideo, NULL);
	MDIN3xx_SetMFCVCFilterCoef(&stVideo, NULL);

	// set aux display ON
	stVideo.dspFLAG = MDIN_AUX_DISPLAY_ON | MDIN_AUX_FREEZE_OFF;

	// set video path
	stVideo.srcPATH = PATH_MAIN_A_AUX_A;	// set main is A, aux is A 
	stVideo.dacPATH = DAC_PATH_MAIN_OUT;	// set main -> HDMI output
	stVideo.encPATH = VENC_PATH_PORT_B;		// set venc is port B 

	// define video format of PORTA-INPUT
	stVideo.stSRC_a.frmt = resolution; 

	if(pvInfo->app_mode)			
	{	
		stVideo.stSRC_a.mode = MDIN_SRC_MUX656_8; 
	}
	else if(!pvInfo->input_mode)
	{
		stVideo.stSRC_a.mode = MDIN_SRC_MUX656_8; 
	}
	else					
	{
		stVideo.stSRC_a.mode = MDIN_SRC_EMB422_8;
	}
	
	stVideo.stSRC_a.fine = MDIN_SYNC_FREERUN;

	// define video format of MAIN-OUTPUT
	if(pvInfo->app_mode)
	{		 
		if(pvInfo->rx.no_video_flag)
		{
			if(vformat == VID_VENC_PAL_B)			stVideo.stOUT_m.frmt = VIDSRC_1920x1080p25;
			else if(vformat == VID_VENC_NTSC_M)		stVideo.stOUT_m.frmt = VIDSRC_1920x1080p30;
		}
		else
		{
			if		( resolution == VIDSRC_1280x720p30)		stVideo.stOUT_m.frmt = VIDOUT_1280x720p30;
			else if	( resolution == VIDSRC_1280x720p25)		stVideo.stOUT_m.frmt = VIDOUT_1280x720p25;
			else if	( resolution == VIDSRC_1920x1080p30)	stVideo.stOUT_m.frmt = VIDOUT_1920x1080p30;
			else if	( resolution == VIDSRC_1920x1080p25)	stVideo.stOUT_m.frmt = VIDOUT_1920x1080p25;
		}	
	}
	else if(!pvInfo->input_mode)
	{
		//AHD input HDMI output case
		//whether no video input or exist video input, always fix main frame 1080p
		if(vformat == VID_VENC_NTSC_M)		stVideo.stOUT_m.frmt = VIDOUT_1920x1080p60;
		else 								stVideo.stOUT_m.frmt = VIDOUT_1920x1080p50;
	}
	else
	{	
		if(!pvInfo->rx.hpa_status_port_a)
		{
			if(vformat == VID_VENC_PAL_B)			stVideo.stOUT_m.frmt = VIDSRC_1920x1080p25;
			else if(vformat == VID_VENC_NTSC_M)		stVideo.stOUT_m.frmt = VIDSRC_1920x1080p30;
		}
		else
		{
			if(pvInfo->tx.current_resolution)
			{
				if(vformat == VID_VENC_NTSC_M)		stVideo.stOUT_m.frmt = VIDOUT_1920x1080p30;
				else 								stVideo.stOUT_m.frmt = VIDOUT_1920x1080p25;
			}
			else
			{
				if(vformat == VID_VENC_NTSC_M)		stVideo.stOUT_m.frmt = VIDOUT_1280x720p30;
				else 								stVideo.stOUT_m.frmt = VIDOUT_1280x720p25;
			}
		}
	}

	stVideo.stOUT_m.mode = MDIN_OUT_EMB422_8;
	stVideo.stOUT_m.fine = MDIN_SYNC_FREERUN;	// set main outsync free-run

	if(vformat == VID_VENC_NTSC_M)
	{
		stVideo.stOUT_m.brightness = 0x80;			// set main picture factor
		stVideo.stOUT_m.contrast = 0x80;
		stVideo.stOUT_m.saturation = 0x80;
		stVideo.stOUT_m.hue = 0x80;
	
		stVideo.stOUT_x.brightness = 0x80;			// set aux picture factor
		stVideo.stOUT_x.contrast = 0x80;
		stVideo.stOUT_x.saturation = 0x80;
		stVideo.stOUT_x.hue = 0x80;
	}
	else
	{
		stVideo.stOUT_m.brightness = 0x80;			// set main picture factor
		stVideo.stOUT_m.contrast = 0x80;
		stVideo.stOUT_m.saturation = 0x80;
		stVideo.stOUT_m.hue = 0x80;
	
		stVideo.stOUT_x.brightness = 0x80;			// set aux picture factor
		stVideo.stOUT_x.contrast = 0x80;
		stVideo.stOUT_x.saturation = 0x80;
		stVideo.stOUT_x.hue = 0x80;
	}

#if RGB_GAIN_OFFSET_TUNE == 1
	stVideo.stOUT_m.r_gain = 128;				// set main gain/offset
	stVideo.stOUT_m.g_gain = 128;
	stVideo.stOUT_m.b_gain = 128;
	stVideo.stOUT_m.r_offset = 128;
	stVideo.stOUT_m.g_offset = 128;
	stVideo.stOUT_m.b_offset = 128;
#endif

	// define video format of IPC-block
	stVideo.stIPC_m.mode = MDIN_DEINT_ADAPTIVE;
	stVideo.stIPC_m.film = MDIN_DEINT_FILM_OFF;
	stVideo.stIPC_m.gain = 40;
	stVideo.stIPC_m.fine = MDIN_DEINT_3DNR_OFF | MDIN_DEINT_CCS_OFF;  //cross color reduction on : MDIN_DEINT_CCS_OFF

	// define map of frame buffer
	stVideo.stMAP_m.frmt = MDIN_MAP_AUX_ON_NR_OFF;	// when MDIN_DEINT_3DNR_ON

	// define video format of PORTB-INPUT
	stVideo.stSRC_b.frmt = resolution; 		
	stVideo.stSRC_b.mode = MDIN_SRC_MUX656_8;
	stVideo.stSRC_b.fine = MDIN_SYNC_FREERUN;

	// define video format of AUX
	if(vformat == VID_VENC_PAL_B)		
	{
		stVideo.stOUT_x.frmt = VIDOUT_720x576p50;
	}
	else if(vformat == VID_VENC_NTSC_M)
	{
		stVideo.stOUT_x.frmt = VIDOUT_720x480p60;
	}
	stVideo.stOUT_x.mode = MDIN_OUT_EMB422_8;
	stVideo.stOUT_x.fine = MDIN_SYNC_FREERUN;	// set aux outsync free-run

#if RGB_GAIN_OFFSET_TUNE == 1
	stVideo.stOUT_x.r_gain = 128;				// set aux gain/offset
	stVideo.stOUT_x.g_gain = 128;
	stVideo.stOUT_x.b_gain = 128;
	stVideo.stOUT_x.r_offset = 128;
	stVideo.stOUT_x.g_offset = 128;
	stVideo.stOUT_x.b_offset = 128;
#endif

	// define video format of video encoder
	//stVideo.encFRMT = vformat;

	// define video format of HDMI-OUTPUT
	stVideo.stVID_h.mode  = HDMI_OUT_RGB444_8;
	stVideo.stAUD_h.frmt  = AUDIO_INPUT_I2S_0;						// audio input format
	stVideo.stAUD_h.freq  = AUDIO_MCLK_256Fs | AUDIO_FREQ_48kHz;	// sampling frequency
	stVideo.stAUD_h.fine  = AUDIO_MAX24B_MINUS0 | AUDIO_WS_POLAR_LOW | AUDIO_SD_JUST_LEFT |
							AUDIO_SCK_EDGE_RISE | AUDIO_SD_MSB_FIRST | AUDIO_SD_1ST_SHIFT;

	MDINHTX_SetHDMIBlock(&stVideo);		// initialize HDMI block
	
	stVideo.exeFLAG = MDIN_UPDATE_MAINFMT;	// execution of video process
	MDIN3xx_VideoProcess(&stVideo);			// mdin3xx main video process

	// define window for inter-area
	stInterWND.lx = 315;	stInterWND.rx = 405;
	stInterWND.ly = 90;		stInterWND.ry = 150;

	MDIN3xx_SetDeintInterWND(&stInterWND, MDIN_INTER_BLOCK0);
	MDIN3xx_EnableDeintInterWND(MDIN_INTER_BLOCK0, OFF);

	stVideo.exeFLAG = MDIN_UPDATE_MAINFMT;	// execution of video process

	xprintf("MDIN3xx INIT complete\n");
}

void VideoProcessHandler(AHD_INFO *pvInfo)
{
	BYTE vformat, resolution;
	
	mdin380_i2c_lock();

	if(pvInfo->flag) 	stVideo.exeFLAG = MDIN_UPDATE_MAINFMT;

	if(stVideo.exeFLAG == 0){
		MDIN3xx_EnableMainDisplay(ON);
		mdin380_i2c_unlock();
		return;		// not change video formats
	}
	
	if(stVideo.exeFLAG)
	{
		if( pvInfo->vformat == NTSC)	vformat = VID_VENC_NTSC_M;
		else							vformat = VID_VENC_PAL_B; 
	
		if(pvInfo->app_mode)					
		{
			if(pvInfo->rx.no_video_flag)
			{
				if(vformat = VID_VENC_NTSC_M)
				{
					stVideo.stSRC_a.frmt = VIDSRC_1920x1080p30;
					stVideo.stOUT_m.frmt = VIDOUT_1920x1080p30;
				}
				else
				{
					stVideo.stSRC_a.frmt = VIDSRC_1920x1080p25;
					stVideo.stOUT_m.frmt = VIDOUT_1920x1080p25;
				}
			}
			else
			{
				if(pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30)
				{
					stVideo.stSRC_a.frmt = VIDSRC_1280x720p30;
					stVideo.stOUT_m.frmt = VIDOUT_1280x720p30;
				}
				else if(pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25)
				{
					stVideo.stSRC_a.frmt = VIDSRC_1280x720p25;
					stVideo.stOUT_m.frmt = VIDOUT_1280x720p25;

				} 
				else if(pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P30)
				{
					stVideo.stSRC_a.frmt = VIDSRC_1920x1080p30;
					stVideo.stOUT_m.frmt = VIDOUT_1920x1080p30;
				}
				else if(pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P25)	
				{
					stVideo.stSRC_a.frmt = VIDSRC_1920x1080p25;
					stVideo.stOUT_m.frmt = VIDOUT_1920x1080p25;
				}
			}
		}
		else if(!pvInfo->input_mode)		
		{
			if(pvInfo->rx.no_video_flag)
			{
				if(vformat == VID_VENC_NTSC_M)
				{
					stVideo.stSRC_a.frmt = VIDSRC_1920x1080p30;
					stVideo.stOUT_m.frmt = VIDOUT_1920x1080p60;
				}
				else
				{
					stVideo.stSRC_a.frmt = VIDSRC_1920x1080p25;
					stVideo.stOUT_m.frmt = VIDOUT_1920x1080p50;
				}
				MDIN3xx_SetOutTestPattern(MDIN_OUT_TEST_BLUE);
			}
			else
			{
				if		( pvInfo->rx.current_resolution == AHDRX_OUT_960X480)		resolution = VIDSRC_960x480i60;
				else if	( pvInfo->rx.current_resolution == AHDRX_OUT_960X576)		resolution = VIDSRC_960x576i50;
				else if	( pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30)	resolution = VIDSRC_1280x720p30;
				else if	( pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25)	resolution = VIDSRC_1280x720p25;
				else if	( pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P30)	resolution = VIDSRC_1920x1080p30;
				else if	( pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P25)	resolution = VIDSRC_1920x1080p25;

				stVideo.stSRC_a.frmt = resolution;

				if(vformat == VID_VENC_NTSC_M)		stVideo.stOUT_m.frmt = VIDOUT_1920x1080p60;
				else 								stVideo.stOUT_m.frmt = VIDOUT_1920x1080p50;
			
				MDIN3xx_SetOutTestPattern(MDIN_OUT_TEST_DISABLE);
			}
		}
		else
		{
			if(!pvInfo->rx.hpa_status_port_a)
			{
				if(vformat = VID_VENC_NTSC_M)
				{
					stVideo.stSRC_a.frmt = VIDSRC_1920x1080p30;
					stVideo.stOUT_m.frmt = VIDOUT_1920x1080p30;
				}
				else
				{
					stVideo.stSRC_a.frmt = VIDSRC_1920x1080p25;
					stVideo.stOUT_m.frmt = VIDOUT_1920x1080p25;
				}
			}
			else
			{
				if		( pvInfo->rx.current_resolution == HDMI_RX_CEA_1280X720P60 )	resolution = VIDSRC_1280x720p60;
				else if	( pvInfo->rx.current_resolution == HDMI_RX_CEA_1280X720P50 )	resolution = VIDSRC_1280x720p50;
				else if	( pvInfo->rx.current_resolution == HDMI_RX_CEA_1920X1080P60 )	resolution = VIDSRC_1920x1080p60;
				else if	( pvInfo->rx.current_resolution == HDMI_RX_CEA_1920X1080P50 )	resolution = VIDSRC_1920x1080p50;
				else if	( pvInfo->rx.current_resolution == HDMI_RX_CEA_1920X1080I50 )	resolution = VIDSRC_1920x1080i50;
				else if	( pvInfo->rx.current_resolution == HDMI_RX_CEA_1920X1080I60 )	resolution = VIDSRC_1920x1080i60;
				else if	( pvInfo->rx.current_resolution == HDMI_RX_CEA_720X480P60 )		resolution = VIDSRC_720x480p60;
				else if	( pvInfo->rx.current_resolution == HDMI_RX_CEA_720X576P50 )		resolution = VIDSRC_720x576p50;
				else if	( pvInfo->rx.current_resolution == HDMI_RX_DMT_800X600P60 )		resolution = VIDSRC_800x600p60;
				else if	( pvInfo->rx.current_resolution == HDMI_RX_DMT_1024X768P60 )	resolution = VIDSRC_1024x768p60;
				else if	( pvInfo->rx.current_resolution == HDMI_RX_DMT_1280X1024P60 )	resolution = VIDSRC_1280x1024p60;

				stVideo.stSRC_a.frmt = resolution;

				if(pvInfo->tx.current_resolution){
					if(vformat == VID_VENC_NTSC_M)		stVideo.stOUT_m.frmt = VIDOUT_1920x1080p30;
					else 								stVideo.stOUT_m.frmt = VIDOUT_1920x1080p25;
				}else{
					if(vformat == VID_VENC_NTSC_M)		stVideo.stOUT_m.frmt = VIDOUT_1280x720p30;
					else 								stVideo.stOUT_m.frmt = VIDOUT_1280x720p25;
				}
			//	xprintf("stVideo.stSRC_a.frmt %02x stVideo.stOUT_m.frmt %02x\n", stVideo.stSRC_a.frmt, stVideo.stOUT_m.frmt);
			}
		}
	}

	stVideo.stOUT_m.mode = MDIN_OUT_EMB422_8;
	stVideo.stOUT_m.fine = MDIN_SYNC_FREERUN;	// set main outsync free-run

	MDIN3xx_VideoProcess(&stVideo);		// mdin3xx main video processa
	MDIN3xx_EnableMainDisplay(ON);
	pvInfo->flag = 0;

	mdin380_i2c_unlock();
}

void VideoHTXCtrlHandler(void)
{
	mdin380_i2c_lock();
	MDINHTX_CtrlHandler(&stVideo);
	mdin380_i2c_unlock();
}

//--------------------------------------------------------------------------------------------------------------------------
// Drive Function for I2C read & I2C write
// You must make functions which is defined below.
//--------------------------------------------------------------------------------------------------------------------------
BYTE MDINI2C_Read(BYTE nID, WORD rAddr, PBYTE pBuff, WORD bytes)
{
#ifndef I2C_USE_HW
	if(I2C_OK != I2C2_WordRead(I2C_MDIN3xx_ADDR,rAddr,pBuff,bytes))
	{
		xprintf(" nID: %04x, rAddr: %04x, pBuff: %04x \n", nID, rAddr, *pBuff);
	}
#else
	if(I2C_Success != I2C_Write(I2C2, I2C_MDIN3xx_ADDR, rAddr, 2, NULL, 0)) return 1;
	if(I2C_Success != I2C_Read(I2C2, I2C_MDIN3xx_ADDR, pBuff, bytes)) return 1;

	/* endian swap */
	if(bytes > 0 && (bytes % 2 == 0)) {
		int i;
		for(i=0; i<bytes; i+=2) {
			BYTE temp = pBuff[i+0];
			pBuff[i+0] = pBuff[i+1];
			pBuff[i+1] = temp;
		}
	}

#endif
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
BYTE MDINI2C_Write(BYTE nID, WORD rAddr, PBYTE pBuff, WORD bytes)
{
//	xprintf(" nID : %04x, rAddr: %04x, pBuff: %04x byte: %04x \n", nID, rAddr, *pBuff, bytes);
#ifndef I2C_USE_HW
	if(I2C_OK != I2C2_WordWrite(I2C_MDIN3xx_ADDR, rAddr, pBuff, bytes))
	{
	//	xprintf(" I2C_FAIL  nID : %04x, rAddr: %04x, pBuff: %04x \n", nID, rAddr, *pBuff);
	}
#else
	int i;

	if(bytes > 0 && bytes <= 32 && (bytes % 2 == 0)) {
		BYTE swap_buf[32]; /* use local buffer not to be dirty source buffer */

		/* endian swap */
		for(i=0; i<bytes; i+=2) {
			swap_buf[i+0] = pBuff[i+1];
			swap_buf[i+1] = pBuff[i+0];
		}

		if(I2C_Success != I2C_Write(I2C2, I2C_MDIN3xx_ADDR, rAddr, 2, swap_buf, bytes)) return 1;
	} else {
		/* endian swap */
		if(bytes > 0 && (bytes % 2 == 0)) {
			for(i=0; i<bytes; i+=2) {
				BYTE temp = pBuff[i+0];
				pBuff[i+0] = pBuff[i+1];
				pBuff[i+1] = temp;
			}
		}

		if(I2C_Success != I2C_Write(I2C2, I2C_MDIN3xx_ADDR, rAddr, 2, pBuff, bytes)) return 1;
	}

#endif
	return 0;
}

int do_mdin380(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	if(argc < 2) return -1;

	if(!strcmp(argv[1], "dump") && argc == 2) {
		mdin380_i2c_lock();
		mdin3xx_i2c_dump();
		mdin380_i2c_unlock();

	} else if(!strcmp(argv[1], "reg") && argc >= 3) {
		u32 addr = simple_strtoul(argv[2], NULL, 16);
		u8 page = (addr >> 16) & 0xf;
		u16 reg = addr & 0xffff;

		if(page > 2) {
			xprintf("invalid page number (%d)\n", page);
			return -1;
		}

		if(argc == 3) {
			u16 data;

			mdin380_i2c_lock();
			switch(page) {
				case 0: MDINI2C_RegRead(0xc0, reg, &data); break;
				case 1: MDINI2C_RegRead(0xc2, reg, &data); break;
				case 2: MDINI2C_RegRead(0xc4, reg, &data); break;
			}
			mdin380_i2c_unlock();

			xprintf("RD) reg(%05x), data(%02x)\n", addr, data);
		} else if(argc == 4) {
			u16 data = simple_strtoul(argv[3], NULL, 16);

			mdin380_i2c_lock();
			switch(page) {
				case 0: MDINI2C_RegWrite(0xc0, reg, data); break;
				case 1: MDINI2C_RegWrite(0xc2, reg, data); break;
				case 2: MDINI2C_RegWrite(0xc4, reg, data); break;
			}
			mdin380_i2c_unlock();

			xprintf("WR) reg(%05x), data(%02x)\n", addr, data);
		} else {
			return -1;
		}
	}else if(!strcmp(argv[1], "test") && argc == 4){
		if(!strcmp(argv[2], "auxout")){
			if(!strcmp(argv[3], "on")){ 
				mdin380_i2c_lock();
				MDINI2C_RegWrite(0xc2, 0x14c, 0x2000);		//wide horizontal ramp test pattern
				mdin380_i2c_unlock();
			}else if(!strcmp(argv[3],"off")){
				mdin380_i2c_lock();
				MDINI2C_RegWrite(0xc2, 0x14c, 0x0);	
				mdin380_i2c_unlock();
			}
		}else if(!strcmp(argv[2], "mainout")){
			if(!strcmp(argv[3], "on")){ 
				mdin380_i2c_lock();
				MDINI2C_RegWrite(0xc2, 0x43, 0x20);			//wide horizontal ramp test pattern
				mdin380_i2c_unlock();
			}else if(!strcmp(argv[3],"off")){
				mdin380_i2c_lock();
				MDINI2C_RegWrite(0xc2, 0x43, 0x0);	
				mdin380_i2c_unlock();
			}
		}else if(!strcmp(argv[2], "input")){
				if(!strcmp(argv[3], "on")){ 
				mdin380_i2c_lock();
				MDINI2C_RegWrite(0xc2, 0x42, 0xdc);			//vertical color bar pattern
				mdin380_i2c_unlock();
			}else if(!strcmp(argv[3],"off")){
				mdin380_i2c_lock();
				MDINI2C_RegWrite(0xc2, 0x42, 0x0);	
				mdin380_i2c_unlock();
			}
		}
	}
	return 0;
}

SHELL_CMD(
	mdin380,   CONFIG_SYS_MAXARGS, 1,  do_mdin380,
	"mdin380 device control",
	"{option}\n"
	"  reg {addr.5}             				- read\n"
	"  reg {addr.5} {data.4}   					- write\n"
	"  dump                    					- dump\n"
	"  test {auxout/mainout/input} {on/off} 	- test pattern\n" 
);

/*  FILE_END_HERE */

