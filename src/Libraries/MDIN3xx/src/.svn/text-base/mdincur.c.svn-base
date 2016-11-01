//----------------------------------------------------------------------------------------------------------------------
// (C) Copyright 2010  Macro Image Technology Co., LTd. , All rights reserved
// 
// This source code is the property of Macro Image Technology and is provided
// pursuant to a Software License Agreement. This code's reuse and distribution
// without Macro Image Technology's permission is strictly limited by the confidential
// information provisions of the Software License Agreement.
//-----------------------------------------------------------------------------------------------------------------------
//
// File Name   		:	MDINCUR.C
// Description 		:
// Ref. Docment		: 
// Revision History 	:

// ----------------------------------------------------------------------
// Include files
// ----------------------------------------------------------------------
#include	"mdin3xx.h"

// ----------------------------------------------------------------------
// Struct/Union Types and define
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Static Global Data section variables
// ----------------------------------------------------------------------

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
static MDIN_ERROR_t MDINCUR_CursorColorMode(PCURSOR_CTL_INFO pINFO)
{
	WORD mode, attb;

	switch (pINFO->ctrl&CURSOR_COLORMODE_MASK) {
		case CURSOR_32BIT_4COLOR: mode = 0; attb = 1; break;
		case CURSOR_32BIT_8COLOR: mode = 0; attb = 2; break;
		default:				  mode = 1; attb = 2; break;
	}
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x301, 6, 1, mode)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x33c, 4, 3, attb)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINCUR_CursorDisplay(PCURSOR_CTL_INFO pINFO)
{
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x300, 11, 1, MBIT(pINFO->ctrl,CURSOR_DISPLAY_ON))) return MDIN_I2C_ERROR;		// display on/off
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINCUR_CursorPosition(PCURSOR_CTL_INFO pINFO)
{
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x33d, pINFO->x)) return MDIN_I2C_ERROR;				// x position
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x33e, pINFO->y)) return MDIN_I2C_ERROR;				// y position
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINCUR_CursorRepeat(PCURSOR_CTL_INFO pINFO)
{
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x301,  5, 1, MBIT(pINFO->ctrl,CURSOR_REPEAT_H_ON))) return MDIN_I2C_ERROR;	// H-repeat on/off
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x301,  4, 1, MBIT(pINFO->ctrl,CURSOR_REPEAT_V_ON))) return MDIN_I2C_ERROR;	// V-repeat on/off
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINCUR_CursorAlpha(PCURSOR_CTL_INFO pINFO)
{
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x33c,  7, 5, pINFO->alpha)) return MDIN_I2C_ERROR;		// alpha value
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINCUR_CursorBlink(PCURSOR_CTL_INFO pINFO)
{
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x33c, 12, 1, MBIT(pINFO->ctrl,CURSOR_BLINK_ON))) return MDIN_I2C_ERROR;		// blink on/off
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINCUR_CursorBlinkInverse(PCURSOR_CTL_INFO pINFO)
{
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x33c, 13, 1, MBIT(pINFO->ctrl,CURSOR_BLINK_INVERSE))) return MDIN_I2C_ERROR;	// blink inverse
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINCUR_CursorBlinkCycle(PCURSOR_CTL_INFO pINFO)
{
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x342,  9, 7, pINFO->cycle)) return MDIN_I2C_ERROR;		// blink cycle
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINCUR_CursorColorKey(PCURSOR_CTL_INFO pINFO)
{
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x357, 10, 1, MBIT(pINFO->ctrl,CURSOR_COLORKEY_ON))) return MDIN_I2C_ERROR;	// colorkey on/off
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINCUR_CursorBMPInfo(PCURSOR_CTL_INFO pINFO)
{
	WORD unit = MDIN3xx_GetSizeOfBank()*2;

	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x33f, pINFO->stBMP.w)) return MDIN_I2C_ERROR;			// width
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x340, pINFO->stBMP.h)) return MDIN_I2C_ERROR;			// height

	pINFO->stBMP.r = ADDR2ROW(pINFO->stBMP.addr, unit);
	pINFO->stBMP.c = ADDR2COL(pINFO->stBMP.addr, unit)/8;		// for 8Byte-unit
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x341, pINFO->stBMP.r)) return MDIN_I2C_ERROR;			// row+bank
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x342, 0, 8, pINFO->stBMP.c)) return MDIN_I2C_ERROR;	// column
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINCUR_CursorPalette(PCURSOR_CTL_INFO pINFO)
{
	if (pINFO->stPAL.pBuff==NULL||pINFO->stPAL.size==0) return MDIN_INVALID_PARAM;
	if (MDINHIF_MultiWrite(MDIN_LOCAL_ID, 0x343, pINFO->stPAL.pBuff, pINFO->stPAL.size)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINCUR_SetCursorConfig(PCURSOR_CTL_INFO pINFO)
{
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x33c, 14, 1, 1)) return MDIN_I2C_ERROR;	// fix blink in vsync

	if (MDINCUR_CursorColorMode(pINFO)) return MDIN_I2C_ERROR;		// set color mode
	if (MDINCUR_CursorDisplay(pINFO)) return MDIN_I2C_ERROR;		// display on/off
	if (MDINCUR_CursorPosition(pINFO)) return MDIN_I2C_ERROR;		// x,y position
	if (MDINCUR_CursorRepeat(pINFO)) return MDIN_I2C_ERROR;			// h,v repeat on/off
	if (MDINCUR_CursorAlpha(pINFO)) return MDIN_I2C_ERROR;			// alpha value
	if (MDINCUR_CursorBlink(pINFO)) return MDIN_I2C_ERROR;			// blink on/off
	if (MDINCUR_CursorBlinkInverse(pINFO)) return MDIN_I2C_ERROR;	// blink inverse on/off
	if (MDINCUR_CursorBlinkCycle(pINFO)) return MDIN_I2C_ERROR;		// set blink cycle
	if (MDINCUR_CursorColorKey(pINFO)) return MDIN_I2C_ERROR;		// colorkey on/off
	if (MDINCUR_CursorBMPInfo(pINFO)) return MDIN_I2C_ERROR;		// set BMP info
	if (MDINCUR_CursorPalette(pINFO)) return MDIN_I2C_ERROR;		// set cursor palette
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINCUR_EnableCursor(PCURSOR_CTL_INFO pINFO, BOOL OnOff)
{
	if (OnOff)	pINFO->ctrl |=  CURSOR_DISPLAY_ON;
	else		pINFO->ctrl &= ~CURSOR_DISPLAY_ON;
	return MDINCUR_CursorDisplay(pINFO);		// display on/off
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINCUR_SetCursorPosition(PCURSOR_CTL_INFO pINFO, WORD x, WORD y)
{
	pINFO->x = x; pINFO->y = y;
	return MDINCUR_CursorPosition(pINFO);		// x,y position
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINCUR_EnableCursorRepeat(PCURSOR_CTL_INFO pINFO, BOOL h_rpt, BOOL v_rpt)
{
	if (h_rpt)	pINFO->ctrl |=  CURSOR_REPEAT_H_ON;
	else		pINFO->ctrl &= ~CURSOR_REPEAT_H_ON;

	if (v_rpt)	pINFO->ctrl |=  CURSOR_REPEAT_V_ON;
	else		pINFO->ctrl &= ~CURSOR_REPEAT_V_ON;

	return MDINCUR_CursorRepeat(pINFO);			// h,v repeat on/off
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINCUR_SetCursorAlpha(PCURSOR_CTL_INFO pINFO, BYTE alpha)
{
	pINFO->alpha = alpha;
	return MDINCUR_CursorAlpha(pINFO);			// alpha value
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINCUR_EnableCursorBlink(PCURSOR_CTL_INFO pINFO, BOOL OnOff)
{
	if (OnOff)	pINFO->ctrl |=  CURSOR_BLINK_ON;
	else		pINFO->ctrl &= ~CURSOR_BLINK_ON;
	return MDINCUR_CursorBlink(pINFO);			// blink on/off
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINCUR_SetCursorBlinkInverse(PCURSOR_CTL_INFO pINFO, BOOL OnOff)
{
	if (OnOff)	pINFO->ctrl |=  CURSOR_BLINK_INVERSE;
	else		pINFO->ctrl &= ~CURSOR_BLINK_INVERSE;
	return MDINCUR_CursorBlinkInverse(pINFO);	// blink inverse on/off
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINCUR_SetCursorBlinkCycle(PCURSOR_CTL_INFO pINFO, BYTE cycle)
{
	pINFO->cycle = cycle;
	return MDINCUR_CursorBlinkCycle(pINFO);		// set blink cycle
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINCUR_EnableCursorColorKey(PCURSOR_CTL_INFO pINFO, BOOL OnOff)
{
	if (OnOff)	pINFO->ctrl |=  CURSOR_COLORKEY_ON;
	else		pINFO->ctrl &= ~CURSOR_COLORKEY_ON;
	return MDINCUR_CursorColorKey(pINFO);		// colorkey on/off
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINCUR_SetCursorBMPInfo(PCURSOR_CTL_INFO pINFO, BYTE w, BYTE h, DWORD addr)
{
	pINFO->stBMP.w		= w;
	pINFO->stBMP.h		= h;
	pINFO->stBMP.addr	= addr;
	return MDINCUR_CursorBMPInfo(pINFO);		// set BMP info
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINCUR_SetCursorPalette(PCURSOR_CTL_INFO pINFO, BYTE size, PBYTE pBuff)
{
	if (pBuff==NULL||size==0) return MDIN_INVALID_PARAM;

	pINFO->stPAL.size	= size;
	pINFO->stPAL.pBuff	= pBuff;
	return MDINCUR_CursorPalette(pINFO);		// set cursor palette
}

/*  FILE_END_HERE */
