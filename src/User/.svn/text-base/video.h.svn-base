//----------------------------------------------------------------------------------------------------------------------
// (C) Copyright 2008  Macro Image Technology Co., LTd. , All rights reserved
// 
// This source code is the property of Macro Image Technology and is provided
// pursuant to a Software License Agreement. This code's reuse and distribution
// without Macro Image Technology's permission is strictly limited by the confidential
// information provisions of the Software License Agreement.
//-----------------------------------------------------------------------------------------------------------------------
//
// File Name   		:  VIDEO.H
// Description 		:  This file contains typedefine for the driver files	
// Ref. Docment		: 
// Revision History 	:

#ifndef		__VIDEO_H__
#define		__VIDEO_H__

// -----------------------------------------------------------------------------
// Include files
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Struct/Union Types and define
// -----------------------------------------------------------------------------
// video.c

#include "mdin3xx.h"

typedef	struct AHDTX_INFO_TYPE
{
	unsigned char  	current_resolution;
	unsigned char  	prev_resolution;
}AHDTX_INFO;

typedef	struct AHDRX_INFO_TYPE
{
	//adv7611 variable
	unsigned char 	hpa_status_port_a;	
	unsigned char 	hpa_status_count;	
	//nvp6124 variable	
	unsigned char 	current_resolution;
	unsigned char 	prev_resolution;
	unsigned char 	no_video_flag;
	unsigned char 	getacpdata[8];
	
	//nvp6124 audio
	unsigned char 	audio_rec_master;
	unsigned char 	audio_pb_master;
	unsigned char 	audio_sample_rate;
	unsigned char 	audio_ch_num;
	unsigned char 	audio_bit;
}AHDRX_INFO;

typedef struct AHD_INFO_TYPE {
	//mode configuration
	unsigned char	app_mode;
	unsigned char	input_mode;
	unsigned char	output_mode;
	unsigned char	vformat;
	//video detect
	unsigned char 	current_resolution;
	unsigned char 	flag;

	AHDRX_INFO	rx;
	AHDTX_INFO	tx;
}AHD_INFO;

typedef enum {
	RSV = 0,	
	AHDRX_OUT_960X480, 
	AHDRX_OUT_960X576,	
	AHDRX_OUT_1280X720P30 = 0x4,
	AHDRX_OUT_1280X720P25 = 0x8,
	AHDRX_OUT_1280X720P60 = 0x10,
	AHDRX_OUT_1280X720P50 = 0x20,
	AHDRX_OUT_1920X1080P30 = 0x40,
	AHDRX_OUT_1920X1080P25 = 0x80,
} AHDRX_OUT_RESOL;

typedef enum {
	AHDTX_OUT_1280X720P = 0,
	AHDTX_OUT_1920X1080P
} AHDTX_OUT_RESOL;

typedef enum {                                                                                  
     BANK0 =0,                                                                                 
     BANK1,                                                                                   
     BANK2,                                                                                     
     BANK3,                                                                                
     BANK4,                                                                         
     BANK5,                                                                  
     BANK6,                                                              
     BANK7,                                                              
     BANK8,                                                              
     BANK9,                                                              
     BANKA,                                                              
     BANKB                                                              
}   AHD_BANK;                                                       
                                                                 
typedef enum {                                              
    NTSC = 0, 
	PAL,
	AUTO,
	NONE
}   VFORMAT;                                        
                                                  
// ----------------------------------------------------------------------
// Exported Variables
// ----------------------------------------------------------------------
#if 0
extern MDIN_VIDEO_INFO		stVideo;
extern MDIN_INTER_WINDOW	stInterWND;
extern MDIN_VIDEO_WINDOW	stZOOM, stCROP;

extern unsigned char AdjInterWND,  InputSelect, InputSelOld,  SrcSyncInfo;
extern unsigned char SrcMainFrmt, PrevSrcMainFrmt, SrcMainMode, PrevSrcMainMode;
extern unsigned char OutMainFrmt, PrevOutMainFrmt, OutMainMode, PrevOutMainMode;
extern unsigned char SrcAuxFrmt, PrevSrcAuxFrmt, SrcAuxMode, PrevSrcAuxMode;
extern unsigned char OutAuxFrmt, PrevOutAuxFrmt, OutAuxMode, PrevOutAuxMode;
extern unsigned char AdcVideoFrmt, PrevAdcFrmt, EncVideoFrmt, PrevEncFrmt;

// -----------------------------------------------------------------------------
// Exported function Prototype
// -----------------------------------------------------------------------------
// video.c
void HDRX_SetRegInitial(void);
void CreateVideoInstance(void);
void VideoProcessHandler(void);
void VideoHTXCtrlHandler(void);
void VideoSetEdgeEnhance(unsigned char mode);
void VideoSetAspectRatio(unsigned char mode);
void VideoSetOverScanning(unsigned char mode);
void VideoSetMFCHYFilter(unsigned char mode);
void VideoSetMFCHCFilter(unsigned char mode);
void VideoSetMFCVYFilter(unsigned char mode);
void VideoSetMFCVCFilter(unsigned char mode);
void VideoSetOutCSCCoef(unsigned char mode);
void VideoSetInCSCCoef(unsigned char mode);
void VideoSetIPCNoiseRobust1(BOOL OnOff);
void VideoSetIPCNoiseRobust2(BOOL OnOff);
void VideoSetIPCSlowMotion(BOOL OnOff);
void DecoderInitial(PNCDEC_INFO pINFO);
#endif
#endif	/* __VIDEO_H__ */
