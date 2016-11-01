//----------------------------------------------------------------------------------------------------------------------
// (C) Copyright 2010  Macro Image Technology Co., LTd. , All rights reserved
// 
// This source code is the property of Macro Image Technology and is provided
// pursuant to a Software License Agreement. This code's reuse and distribution
// without Macro Image Technology's permission is strictly limited by the confidential
// information provisions of the Software License Agreement.
//-----------------------------------------------------------------------------------------------------------------------
//
// File Name   		:	MDINOSD.C
// Description 		:
// Ref. Docment		: 
// Revision History 	:

// ----------------------------------------------------------------------
// Include files
// ----------------------------------------------------------------------
#include	"mdin3xx.h"
#include    "mdintype.h"
#include    "video.h"

extern ROMDATA WORD mdinaux_font_yuv_pal[8][2];
extern ROMDATA WORD mdin3xx_font_rgb_pal[16][2];
extern ROMDATA BYTE mdin3xx_font_cnv_bmp[112][32];

SPRITE_CTL_INFO stOSD[12];
LAYER_CTL_INFO stLayer[3];
GACMAP_FONT_INFO stFONT;		
BYTE CH01[]	= "CH1";
BYTE CH02[]	= "CH2";
BYTE CH03[]	= "CH3";
BYTE CH04[]	= "CH4";

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetBGLayerColor(MDIN_COLORRGB rgb)
{
	WORD r = GetR(rgb), g = GetG(rgb), b = GetB(rgb);

	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x09a, g)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x09b, MAKEWORD(b,r))) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetBGBoxColor(MDIN_COLORRGB rgb)
{
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x302, GetR(rgb))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x303, LOWORD(rgb))) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

MDIN_ERROR_t MDINOSD_SetAuxBGColor(void)
{
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x14C, 0x10)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x14D, 0x8080)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetBGBoxArea(BGBOX_INDEX_t nID, WORD x, WORD y, WORD w, WORD h)
{
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x366, x)) return MDIN_I2C_ERROR;	// x
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x367, nID*4+0x80)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x366, y)) return MDIN_I2C_ERROR;	// y
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x367, nID*4+0x81)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x366, w)) return MDIN_I2C_ERROR;	// w
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x367, nID*4+0x82)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x366, h)) return MDIN_I2C_ERROR;	// h
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x367, nID*4+0x83)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_EnableBGBox(BGBOX_INDEX_t nID, BOOL OnOff)
{
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x367, 0x00|0x20)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x366, nID, 1, MBIT(OnOff,1))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x367, 0x80|0x20)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINAUX_SpriteColorMode(PSPRITE_CTL_INFO pINFO, BYTE nID)
{
	WORD mode;

	switch (pINFO->ctrl&SPRITE_COLORMODE_MASK) {
		case SPRITE_2BITPALT_COLOR: mode = 1; break;
		case SPRITE_4BITPALT_COLOR: mode = 2; break;
		default:					mode = 0; break;
	}
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x2a1+(nID*7), 4, 3, mode)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINOSD_SpriteColorMode(PSPRITE_CTL_INFO pINFO, BYTE nID)
{
	WORD mode, attb, p = ((nID%4)/2)*8+((nID%4)%2)*3;


	if (nID>SPRITE_INDEX7) return MDINAUX_SpriteColorMode(pINFO, nID%SPRITE_INDEX8);
	switch (pINFO->ctrl&SPRITE_COLORMODE_MASK) 
	{
		case SPRITE_2BITPALT_COLOR: mode = 1; attb = 0; break;
		case SPRITE_4BITPALT_COLOR: mode = 2; attb = 0; break;
		case SPRITE_8BITPALT_COLOR: mode = 3; attb = 0; break;

		case SPRITE_aRGB0565_16BIT: mode = 4; attb = 0; break;
		case SPRITE_aRGB4444_16BIT: mode = 4; attb = 1; break;
		case SPRITE_aRGB1555_16BIT: mode = 4; attb = 2; break;
		case SPRITE_aYBYR088_YC422: mode = 4; attb = 4; break;
		case SPRITE_aYBYR277_YC422: mode = 4; attb = 5; break;
		case SPRITE_aYBYR187_YC422: mode = 4; attb = 6; break;
		case SPRITE_aYUV0655_YC444: mode = 4; attb = 7; break;

		case SPRITE_aRGB0888_24BIT: mode = 6; attb = 0; break;
		default:					mode = 5; attb = 0; break;
	}
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x304+(nID*7), 4, 3, mode)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x355+(nID/4), p, 3, attb)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINAUX_SpriteDisplay(PSPRITE_CTL_INFO pINFO, BYTE nID)
{
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x2a0, 0+nID, 1, MBIT(pINFO->ctrl,SPRITE_DISPLAY_ON))) return MDIN_I2C_ERROR;	// display on/off
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINOSD_SpriteDisplay(PSPRITE_CTL_INFO pINFO, BYTE nID)
{
	if (nID>SPRITE_INDEX7) return MDINAUX_SpriteDisplay(pINFO, nID%SPRITE_INDEX8);

	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x300, 3+nID, 1, MBIT(pINFO->ctrl,SPRITE_DISPLAY_ON))) return MDIN_I2C_ERROR;	// display on/off
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINAUX_SpritePosition(PSPRITE_CTL_INFO pINFO, BYTE nID)
{
	WORD rVal, x, y;

	if (MDINHIF_RegRead(MDIN_LOCAL_ID, 0x145, &rVal)) return MDIN_I2C_ERROR;
	x = (rVal&0x0080)? pINFO->x*2 : pINFO->x;	// if YC-muxed then x = x*2

	if (MDINHIF_RegRead(MDIN_LOCAL_ID, 0x17f, &rVal)) return MDIN_I2C_ERROR;
	y = (rVal&0x8000)? pINFO->y/2 : pINFO->y;	// if interlace then y = y/2

	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x2a2+nID*7, x)) return MDIN_I2C_ERROR;			// x position
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x2a3+nID*7, y)) return MDIN_I2C_ERROR;			// y position
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINOSD_SpritePosition(PSPRITE_CTL_INFO pINFO, BYTE nID)
{
	if (nID>SPRITE_INDEX7) return MDINAUX_SpritePosition(pINFO, nID%SPRITE_INDEX8);

	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x305+nID*7, pINFO->x)) return MDIN_I2C_ERROR;		// x position
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x306+nID*7, pINFO->y)) return MDIN_I2C_ERROR;		// y position
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINAUX_SpriteAlpha(PSPRITE_CTL_INFO pINFO, BYTE nID)
{
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x2a1+nID*7, 7, 5, pINFO->alpha)) return MDIN_I2C_ERROR;	// alpha value
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINOSD_SpriteAlpha(PSPRITE_CTL_INFO pINFO, BYTE nID)
{
	if (nID>SPRITE_INDEX7) return MDINAUX_SpriteAlpha(pINFO, nID%SPRITE_INDEX8);

	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x304+nID*7, 7, 5, pINFO->alpha)) return MDIN_I2C_ERROR;	// alpha value
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINOSD_SpritePaletteAddr(PSPRITE_CTL_INFO pINFO, BYTE nID)
{
	WORD idx = (pINFO->ctrl&SPRITE_PALT_ADDR_MASK)>>4;

	if (nID>SPRITE_INDEX7) return MDIN_NO_ERROR;

	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x304+nID*7, 0, 4, idx)) return MDIN_I2C_ERROR;				// palette addr
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINAUX_SpriteBMPInfo(PSPRITE_CTL_INFO pINFO, BYTE nID)
{
	WORD unit = MDIN3xx_GetSizeOfBank()*2;

	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x2a4+nID*7, pINFO->stBMP.w)) return MDIN_I2C_ERROR;		// width
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x2a5+nID*7, pINFO->stBMP.h)) return MDIN_I2C_ERROR;		// height

	pINFO->stBMP.r = ADDR2ROW(pINFO->stBMP.addr, unit);
	pINFO->stBMP.c = ADDR2COL(pINFO->stBMP.addr, unit)/8;			// for 8Byte-unit
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x2a6+nID*7, pINFO->stBMP.r)) return MDIN_I2C_ERROR;		// row+bank
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x2a7+nID*7, 0, 8, pINFO->stBMP.c)) return MDIN_I2C_ERROR;	// column
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINOSD_SpriteBMPInfo(PSPRITE_CTL_INFO pINFO, BYTE nID)
{
	WORD unit = MDIN3xx_GetSizeOfBank()*2;

	if (nID>SPRITE_INDEX7) return MDINAUX_SpriteBMPInfo(pINFO, nID%SPRITE_INDEX8);

	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x307+nID*7, pINFO->stBMP.w)) return MDIN_I2C_ERROR;		// width
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x308+nID*7, pINFO->stBMP.h)) return MDIN_I2C_ERROR;		// height

	pINFO->stBMP.r = ADDR2ROW(pINFO->stBMP.addr, unit);
	pINFO->stBMP.c = ADDR2COL(pINFO->stBMP.addr, unit)/8;			// for 8Byte-unit
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x309+nID*7, pINFO->stBMP.r)) return MDIN_I2C_ERROR;		// row+bank
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x30a+nID*7, 0, 8, pINFO->stBMP.c)) return MDIN_I2C_ERROR;	// column
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetSpriteConfig(PSPRITE_CTL_INFO pINFO, SPRITE_INDEX_t nID)
{
	pINFO->index = nID;		// set sprite index

	if (MDINOSD_SpriteColorMode(pINFO, pINFO->index)) return MDIN_I2C_ERROR;		// set color mode
	if (MDINOSD_SpriteDisplay(pINFO, pINFO->index)) return MDIN_I2C_ERROR;			// display on/off
	if (MDINOSD_SpritePosition(pINFO, pINFO->index)) return MDIN_I2C_ERROR;			// x,y position
	if (MDINOSD_SpriteAlpha(pINFO, pINFO->index)) return MDIN_I2C_ERROR;			// alpha value
	if (MDINOSD_SpritePaletteAddr(pINFO, pINFO->index)) return MDIN_I2C_ERROR;		// palette addr
	if (MDINOSD_SpriteBMPInfo(pINFO, pINFO->index)) return MDIN_I2C_ERROR;			// set BMP info


	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_EnableSprite(PSPRITE_CTL_INFO pINFO, BOOL OnOff)
{
	if (OnOff)	pINFO->ctrl |=  SPRITE_DISPLAY_ON;
	else		pINFO->ctrl &= ~SPRITE_DISPLAY_ON;
	return MDINOSD_SpriteDisplay(pINFO, pINFO->index);			// display on/off
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetSpritePosition(PSPRITE_CTL_INFO pINFO, WORD x, WORD y)
{
	pINFO->x = x; pINFO->y = y;
	return MDINOSD_SpritePosition(pINFO, pINFO->index);		// x,y position
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetSpriteAlpha(PSPRITE_CTL_INFO pINFO, BYTE alpha)
{
	pINFO->alpha = alpha;
	return MDINOSD_SpriteAlpha(pINFO, pINFO->index);		// alpha value
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetSpritePaletteAddr(PSPRITE_CTL_INFO pINFO, BYTE addr)
{
	pINFO->ctrl &= ~SPRITE_PALT_ADDR_MASK;
	pINFO->ctrl |= (addr&SPRITE_PALT_ADDR_MASK);
	return MDINOSD_SpritePaletteAddr(pINFO, pINFO->index);	// palette addr
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetSpriteBMPInfo(PSPRITE_CTL_INFO pINFO, WORD w, WORD h, DWORD addr)
{
	pINFO->stBMP.w		= w;
	pINFO->stBMP.h		= h;
	pINFO->stBMP.addr	= addr;
	return MDINOSD_SpriteBMPInfo(pINFO, pINFO->index);		// set BMP info
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINAUX_LayerRepeat(PLAYER_CTL_INFO pINFO, WORD nID)
{
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x2a0, 8, 1, MBIT(pINFO->ctrl,LAYER_REPEAT_H_ON))) return MDIN_I2C_ERROR;	// H-repeat on/off
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x2a0, 9, 1, MBIT(pINFO->ctrl,LAYER_REPEAT_V_ON))) return MDIN_I2C_ERROR;	// V-repeat on/off
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINOSD_LayerRepeat(PLAYER_CTL_INFO pINFO, WORD nID)
{
	if (nID>LAYER_INDEX1) return MDINAUX_LayerRepeat(pINFO, nID%LAYER_INDEX2);

	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x301, 0+nID*2, 1, MBIT(pINFO->ctrl,LAYER_REPEAT_H_ON))) return MDIN_I2C_ERROR;	// H-repeat on/off
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x301, 1+nID*2, 1, MBIT(pINFO->ctrl,LAYER_REPEAT_V_ON))) return MDIN_I2C_ERROR;	// V-repeat on/off
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINOSD_LayerCSCBypass(PLAYER_CTL_INFO pINFO, WORD nID)
{
	if (nID>LAYER_INDEX1) return MDIN_NO_ERROR;

	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x353+nID, 98*2+MBIT(pINFO->ctrl,LAYER_CSCBYPASS_ON))) return MDIN_I2C_ERROR;	// csc-bypass on/off
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINAUX_LayerRGBKey(PLAYER_CTL_INFO pINFO, WORD nID)
{
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x2d0, 1, 1, MBIT(pINFO->ctrl,LAYER_TPKEY_RGB_ON))) return MDIN_I2C_ERROR;	// rgb-key on/off
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINOSD_LayerRGBKey(PLAYER_CTL_INFO pINFO, WORD nID)
{
	if (nID>LAYER_INDEX1) return MDINAUX_LayerRGBKey(pINFO, nID%LAYER_INDEX2);

	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x357, 8+nID*1, 1, MBIT(pINFO->ctrl,LAYER_TPKEY_RGB_ON))) return MDIN_I2C_ERROR;	// rgb-key on/off
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINOSD_LayerYUVKey(PLAYER_CTL_INFO pINFO, WORD nID)
{
	if (nID>LAYER_INDEX1) return MDIN_NO_ERROR;

	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x359, 8+nID*1, 1, MBIT(pINFO->ctrl,LAYER_TPKEY_YUV_ON))) return MDIN_I2C_ERROR;	// yuv-key on/off
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINAUX_LayerPalette(PLAYER_CTL_INFO pINFO, WORD nID)
{
	if (MBIT(pINFO->ctrl,LAYER_AUXPAL_16BIT)==1&&pINFO->stPAL.size>64) return MDIN_INVALID_PARAM;
	if (MBIT(pINFO->ctrl,LAYER_AUXPAL_16BIT)==0&&pINFO->stPAL.size>32) return MDIN_INVALID_PARAM;

	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x2bd, 0, 1, MBIT(pINFO->ctrl,LAYER_AUXPAL_16BIT))) return MDIN_I2C_ERROR;
	if (MDINHIF_MultiWrite(MDIN_LOCAL_ID, 0x2c0, pINFO->stPAL.pBuff, pINFO->stPAL.size)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINOSD_LayerPalette(PLAYER_CTL_INFO pINFO, WORD nID)
{
	WORD i;

	if (pINFO->stPAL.pBuff==0||pINFO->stPAL.size==0) return MDIN_INVALID_PARAM;
	if (nID>LAYER_INDEX1) return MDINAUX_LayerPalette(pINFO, nID%LAYER_INDEX2);

	for (i=0; i<pINFO->stPAL.size/4; i++) {
		if (MDINHIF_MultiWrite(MDIN_LOCAL_ID, 0x360+nID*3, pINFO->stPAL.pBuff+i*4, 4)) return MDIN_I2C_ERROR;
		if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x362+nID*3, 0x0000+i+pINFO->stPAL.addr)) return MDIN_I2C_ERROR;
		if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x362+nID*3, 0x0100+i+pINFO->stPAL.addr)) return MDIN_I2C_ERROR;
	}
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetLayerConfig(PLAYER_CTL_INFO pINFO, LAYER_INDEX_t nID)
{
	pINFO->index = nID;		// set layer index

	if (MDINOSD_LayerRepeat(pINFO, pINFO->index)) return MDIN_I2C_ERROR;		// H,V repeat on/off
	if (MDINOSD_LayerCSCBypass(pINFO, pINFO->index)) return MDIN_I2C_ERROR;		// csc-bypass on/off
	if (MDINOSD_LayerRGBKey(pINFO, pINFO->index)) return MDIN_I2C_ERROR;		// rgb-key on/off
	if (MDINOSD_LayerYUVKey(pINFO, pINFO->index)) return MDIN_I2C_ERROR;		// yuv-key on/off
	if (MDINOSD_LayerPalette(pINFO, pINFO->index)) return MDIN_I2C_ERROR;		// set palette entry
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_EnableLayerRepeat(PLAYER_CTL_INFO pINFO, BOOL h_rpt, BOOL v_rpt)
{
	if (h_rpt)	pINFO->ctrl |=  LAYER_REPEAT_H_ON;		// H-repeat on/off
	else		pINFO->ctrl &= ~LAYER_REPEAT_H_ON;

	if (v_rpt)	pINFO->ctrl |=  LAYER_REPEAT_V_ON;		// V-repeat on/off
	else		pINFO->ctrl &= ~LAYER_REPEAT_V_ON;

	return MDINOSD_LayerRepeat(pINFO, pINFO->index);	// H,V repeat on/off
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_EnableLayerCSCBypass(PLAYER_CTL_INFO pINFO, BOOL OnOff)
{
	if (OnOff)	pINFO->ctrl |=  LAYER_CSCBYPASS_ON;
	else		pINFO->ctrl &= ~LAYER_CSCBYPASS_ON;
	return MDINOSD_LayerCSCBypass(pINFO, pINFO->index);	// csc-bypass on/off
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_EnableLayerTPKeyRGB(PLAYER_CTL_INFO pINFO, BOOL OnOff)
{
	if (OnOff)	pINFO->ctrl |=  LAYER_TPKEY_RGB_ON;
	else		pINFO->ctrl &= ~LAYER_TPKEY_RGB_ON;
	return MDINOSD_LayerRGBKey(pINFO, pINFO->index);	// rgb-key on/off
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_EnableLayerTPKeyYUV(PLAYER_CTL_INFO pINFO, BOOL OnOff)
{
	if (OnOff)	pINFO->ctrl |=  LAYER_TPKEY_YUV_ON;
	else		pINFO->ctrl &= ~LAYER_TPKEY_YUV_ON;
	return MDINOSD_LayerYUVKey(pINFO, pINFO->index);	// yuv-key on/off
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetLayerPalette(PLAYER_CTL_INFO pINFO, WORD addr, WORD size, PBYTE pBuff)
{
	if ((pBuff==0)||(size==0)) return MDIN_INVALID_PARAM;

	pINFO->stPAL.addr	= addr;
	pINFO->stPAL.size	= size;
	pINFO->stPAL.pBuff	= pBuff;
	return MDINOSD_LayerPalette(pINFO, pINFO->index);	// set palette entry
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetBitmapData(DWORD addr, DWORD size, PBYTE pBuff)
{
	if (pBuff==0||size==0) return MDIN_INVALID_PARAM;

	if (MDIN3xx_SetPriorityHIF(MDIN_PRIORITY_HIGH)) return MDIN_I2C_ERROR;
	if (MDINHIF_DMAWrite(MDIN_SDRAM_ID, addr, pBuff, size)) return MDIN_I2C_ERROR;
	if (MDIN3xx_SetPriorityHIF(MDIN_PRIORITY_NORM)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetGlobalColorKeyRGB(MDIN_COLORRGB rgb)
{
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x357, 0, 8, GetR(rgb))) return MDIN_I2C_ERROR;		// R component of RGB color-key
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x358, 8, 8, GetG(rgb))) return MDIN_I2C_ERROR;		// G component of RGB color-key
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x358, 0, 8, GetB(rgb))) return MDIN_I2C_ERROR;		// B component of RGB color-key
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetGlobalColorKeyYUV(MDIN_COLORYBR ybr)
{
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x359, 0, 8, GetY(ybr))) return MDIN_I2C_ERROR;		// Y  component of YUV color-key
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x35a, 8, 8, GetCb(ybr))) return MDIN_I2C_ERROR;	// Cb component of YUV color-key
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x35a, 0, 8, GetCr(ybr))) return MDIN_I2C_ERROR;	// Cr component of YUV color-key
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINAUX_SetGlobalColorKeyRGB(MDIN_COLORRGB rgb)
{
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x2d1, 0, 8, GetR(rgb))) return MDIN_I2C_ERROR;		// R component of RGB color-key
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x2d2, 8, 8, GetG(rgb))) return MDIN_I2C_ERROR;		// G component of RGB color-key
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x2d2, 0, 8, GetB(rgb))) return MDIN_I2C_ERROR;		// B component of RGB color-key
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINOSD_SBoxCtrl(PSBOX_CTL_INFO pINFO, WORD nID)
{
	WORD mode = (pINFO->alpha<<4)|(pINFO->h_thk<<2)|(pINFO->v_thk<<0);

	mode |= (pINFO->ctrl&SBOX_BORDER_ON)?	(1<<11) : 0;
	mode |= (pINFO->ctrl&SBOX_PLANE_ON)?	(1<<10) : 0;
	mode |= (pINFO->ctrl&SBOX_BLINK_ON)?	(1<< 9) : 0;

	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x368, mode)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x369, 0x0080|(0+nID*8))) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINOSD_SBoxRect(PSBOX_CTL_INFO pINFO, WORD nID)
{
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x368, pINFO->stRECT.lx)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x369, 0x0080|(1+nID*8))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x368, pINFO->stRECT.rx)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x369, 0x0080|(2+nID*8))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x368, pINFO->stRECT.ly)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x369, 0x0080|(3+nID*8))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x368, pINFO->stRECT.ry)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x369, 0x0080|(4+nID*8))) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINOSD_SBoxColor(PSBOX_CTL_INFO pINFO, WORD nID)
{
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x368, MAKEWORD(GetR(pINFO->b_color),GetG(pINFO->b_color)))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x369, 0x0080|(5+nID*8))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x368, MAKEWORD(GetB(pINFO->b_color),GetR(pINFO->p_color)))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x369, 0x0080|(6+nID*8))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x368, MAKEWORD(GetG(pINFO->p_color),GetB(pINFO->p_color)))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x369, 0x0080|(7+nID*8))) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetSBoxConfig(PSBOX_CTL_INFO pINFO, SBOX_INDEX_t nID)
{
	pINFO->index = nID;		// set sprite index

	if (MDINOSD_SBoxCtrl(pINFO, pINFO->index)) return MDIN_I2C_ERROR;		// sbox control
	if (MDINOSD_SBoxRect(pINFO, pINFO->index)) return MDIN_I2C_ERROR;		// sbox rectangle
	if (MDINOSD_SBoxColor(pINFO, pINFO->index)) return MDIN_I2C_ERROR;		// sbox color
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_EnableSBoxBorder(PSBOX_CTL_INFO pINFO, BOOL OnOff)
{
	if (OnOff)	pINFO->ctrl |=  SBOX_BORDER_ON;
	else		pINFO->ctrl &= ~SBOX_BORDER_ON;
	return MDINOSD_SBoxCtrl(pINFO, pINFO->index);		// border on/off
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_EnableSBoxPlane(PSBOX_CTL_INFO pINFO, BOOL OnOff)
{
	if (OnOff)	pINFO->ctrl |=  SBOX_PLANE_ON;
	else		pINFO->ctrl &= ~SBOX_PLANE_ON;
	return MDINOSD_SBoxCtrl(pINFO, pINFO->index);		// plane on/off
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_EnableSBoxBlink(PSBOX_CTL_INFO pINFO, BOOL OnOff)
{
	if (OnOff)	pINFO->ctrl |=  SBOX_BLINK_ON;
	else		pINFO->ctrl &= ~SBOX_BLINK_ON;
	return MDINOSD_SBoxCtrl(pINFO, pINFO->index);		// blink on/off
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetSBoxAlpha(PSBOX_CTL_INFO pINFO, BYTE alpha)
{
	pINFO->alpha = alpha;
	return MDINOSD_SBoxCtrl(pINFO, pINFO->index);		// alpha value
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetSBoxBorderThickH(PSBOX_CTL_INFO pINFO, BYTE h_thk)
{
	pINFO->h_thk = h_thk;
	return MDINOSD_SBoxCtrl(pINFO, pINFO->index);		// H-thick
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetSBoxBorderThickV(PSBOX_CTL_INFO pINFO, BYTE v_thk)
{
	pINFO->v_thk = v_thk;
	return MDINOSD_SBoxCtrl(pINFO, pINFO->index);		// V-thick
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetSBoxArea(PSBOX_CTL_INFO pINFO, WORD lx, WORD ly, WORD rx, WORD ry)
{
	pINFO->stRECT.lx	= lx;
	pINFO->stRECT.ly	= ly;
	pINFO->stRECT.rx	= rx;
	pINFO->stRECT.ry	= ry;
	return MDINOSD_SBoxRect(pINFO, pINFO->index);		// rectangle
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetSBoxBorderColor(PSBOX_CTL_INFO pINFO, MDIN_COLORRGB rgb)
{
	pINFO->b_color = rgb;
	return MDINOSD_SBoxColor(pINFO, pINFO->index);		// boundary color
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetSBoxPlaneColor(PSBOX_CTL_INFO pINFO, MDIN_COLORRGB rgb)
{
	pINFO->p_color = rgb;
	return MDINOSD_SBoxColor(pINFO, pINFO->index);		// plane color
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINOSD_SetGlobalSBoxBlink(SBOX_BLK_INFO stBLK)
{
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x369, 0x0000|0)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x368, 12, 4, LO4BIT(stBLK.ctrl))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x369, 0x0080|0)) return MDIN_I2C_ERROR;

	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x369, 0x0000|8)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x368, 12, 3, HI4BIT(stBLK.ctrl))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x369, 0x0080|8)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}


/*  FILE_END_HERE */
