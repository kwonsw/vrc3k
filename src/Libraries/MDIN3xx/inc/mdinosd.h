//----------------------------------------------------------------------------------------------------------------------
// (C) Copyright 2010  Macro Image Technology Co., LTd. , All rights reserved
// 
// This source code is the property of Macro Image Technology and is provided
// pursuant to a Software License Agreement. This code's reuse and distribution
// without Macro Image Technology's permission is strictly limited by the confidential
// information provisions of the Software License Agreement.
//-----------------------------------------------------------------------------------------------------------------------
//
// File Name   		:  MDINOSD.H
// Description 		:  This file contains typedefine for the driver files	
// Ref. Docment		: 
// Revision History 	:

#ifndef		__MDINOSD_H__
#define		__MDINOSD_H__

// -----------------------------------------------------------------------------
// Include files
// -----------------------------------------------------------------------------
#ifndef		__MDINTYPE_H__
#include	 "mdintype.h"
#endif

// -----------------------------------------------------------------------------
// Struct/Union Types and define
// -----------------------------------------------------------------------------

#define RGB(r,g,b)		((MDIN_COLORRGB)MAKEDWORD(r,MAKEWORD(g,b)))
#define GetR(rgb)		(LOBYTE(HIWORD(rgb)))
#define GetG(rgb)		(HIBYTE(LOWORD(rgb)))
#define GetB(rgb)		(LOBYTE(LOWORD(rgb)))

#define YBR(y,b,r)		((MDIN_COLORYBR)MAKEDWORD(y,MAKEWORD(b,r)))
#define GetY(ybr)		(LOBYTE(HIWORD(ybr)))
#define GetCb(ybr)		(HIBYTE(LOWORD(ybr)))
#define GetCr(ybr)		(LOBYTE(LOWORD(ybr)))

#define aRGB(a,r,g,b)	((MDIN_COLORRGB)MAKEDWORD(MAKEWORD(a,r),MAKEWORD(g,b)))
#define aYBR(a,y,b,r)	((MDIN_COLORYBR)MAKEDWORD(MAKEWORD(a,y),MAKEWORD(b,r)))
#define GetA(color)		(HIBYTE(HIWORD(color)))

#define aRGB0565(a,r,g,b) (WORD)(((WORD)(r&0xf8)<<8)|((WORD)(g&0xfc)<<3)|((WORD)(b&0xf8)>>3))
#define aRGB4444(a,r,g,b) (WORD)(((WORD)(a)<<12)|((WORD)(r&0xf0)<<4)|((WORD)(g&0xf0)<<0)|((WORD)(b&0xf0)>>4))
#define aRGB1555(a,r,g,b) (WORD)(((WORD)(a)<<15)|((WORD)(r&0xf8)<<7)|((WORD)(g&0xf8)<<2)|((WORD)(b&0xf8)>>3))
#define aYUV0655(a,y,b,r) (WORD)(((WORD)(y&0xfc)<<8)|((WORD)(b&0xf8)<<2)|((WORD)(r&0xf8)>>3))
#define aRGB5888(a,r,g,b) (DWORD)(((DWORD)(a)<<27)|((DWORD)r<<16)|((DWORD)g<<8)|((DWORD)b>>0))

typedef struct {
	WORD	lx;			// the x position of upper-left of rectangle
	WORD	ly;			// the y position of upper-left of rectangle
	WORD	rx;			// the x position of lower-right of rectangle
	WORD	ry;			// the y position of lower-right of rectangle

}	stPACKED MDIN_RECT_BOX, *PMDIN_RECT_BOX;

typedef struct {
	WORD	x;			// the x position of upper-left of rectangle
	WORD	y;			// the y position of upper-left of rectangle
	WORD	w;			// the horizontal size of rectangle
	WORD	h;			// the vertical size of rectangle

}	stPACKED MDIN_AREA_BOX, *PMDIN_AREA_BOX;

typedef enum {
	BGBOX_INDEX0 = 0, BGBOX_INDEX1, BGBOX_INDEX2, BGBOX_INDEX3,
	BGBOX_INDEX4, BGBOX_INDEX5, BGBOX_INDEX6, BGBOX_INDEX7
	
}	BGBOX_INDEX_t;

typedef struct {
	DWORD	addr;		// the start address of bitmap data in MDIN-sdram
	DWORD	size;		// the size of bitmap data, in bytes
	PBYTE	pBuff;		// the pointer to bitmap data

}	stPACKED MDIN_BITMAP_INFO, *PMDIN_BITMAP_INFO;

typedef struct {
	WORD	w;			// the width of sprite bitmap, in pixels
	WORD	h;			// the height of sprite bitmap, in pixels
	DWORD	addr;		// the start address of sprite bitmap in MDIN-sdram

	WORD	r;			// the row+bank of sprite bitmap, is used only internal operation
	WORD	c;			// the column of sprite bitmap, is used only internal operation

}	stPACKED SPRITE_BMP_INFO, *PSPRITE_BMP_INFO;

typedef struct {
	WORD	ctrl;		// the display control of sprite bitmap
	WORD	x;			// the left corner of sprite display position
	WORD	y;			// the top corner of sprite display position
	BYTE	alpha;		// the alpha-blending of sprite layer
	BYTE	index;		// the number of index of sprite layer

	SPRITE_BMP_INFO stBMP;

}	stPACKED SPRITE_CTL_INFO, *PSPRITE_CTL_INFO;

typedef enum {
	SPRITE_INDEX0 = 0, SPRITE_INDEX1, SPRITE_INDEX2, SPRITE_INDEX3,
	SPRITE_INDEX4, SPRITE_INDEX5, SPRITE_INDEX6, SPRITE_INDEX7,
	SPRITE_INDEX8, SPRITE_INDEX9, SPRITE_INDEXA, SPRITE_INDEXB
	
}	SPRITE_INDEX_t;

#define		SPRITE_DISPLAY_ON			0x1000
#define		SPRITE_DISPLAY_OFF			0x0000

#define		SPRITE_2BITPALT_COLOR		0x0000
#define		SPRITE_4BITPALT_COLOR		0x0100
#define		SPRITE_8BITPALT_COLOR		0x0200
#define		SPRITE_aRGB0565_16BIT		0x0300
#define		SPRITE_aRGB4444_16BIT		0x0400
#define		SPRITE_aRGB1555_16BIT		0x0500
#define		SPRITE_aYBYR088_YC422		0x0600
#define		SPRITE_aYBYR277_YC422		0x0700
#define		SPRITE_aYBYR187_YC422		0x0800
#define		SPRITE_aYUV0655_YC444		0x0900
#define		SPRITE_aRGB0888_24BIT		0x0a00
#define		SPRITE_aRGB5888_32BIT		0x0b00
#define		SPRITE_COLORMODE_MASK		0x0f00

#define		SPRITE_PALT_ADDR_MASK		0x00ff

#define		SPRITE_OPAQUE				31
#define		SPRITE_OPAQUE0				28	
#define		SPRITE_TRANSPARENT			0

typedef struct {
	WORD	addr;		// the start address of palette entry
	WORD	size;		// the size of palette data, in bytes
	PBYTE	pBuff;		// the pointer to palette entry

}	stPACKED LAYER_PAL_INFO, *PLAYER_PAL_INFO;

typedef struct {
	WORD	ctrl;		// the display control of OSD layer
	BYTE	index;		// the number of index of OSD layer
	BYTE	dummy;		// dummy for 32-bit alignment

	LAYER_PAL_INFO stPAL;

}	stPACKED LAYER_CTL_INFO, *PLAYER_CTL_INFO;

typedef enum {
	LAYER_INDEX0 = 0, LAYER_INDEX1, LAYER_INDEX2
	
}	LAYER_INDEX_t;

#define		LAYER_CSCBYPASS_ON			0x0002
#define		LAYER_CSCBYPASS_OFF			0x0000

#define		LAYER_REPEAT_H_ON			0x0004
#define		LAYER_REPEAT_H_OFF			0x0000
#define		LAYER_REPEAT_V_ON			0x0008
#define		LAYER_REPEAT_V_OFF			0x0000

#define		LAYER_TPKEY_RGB_ON			0x0010
#define		LAYER_TPKEY_RGB_OFF			0x0000

#define		LAYER_TPKEY_YUV_ON			0x0020
#define		LAYER_TPKEY_YUV_OFF			0x0000

#define		LAYER_AUXPAL_16BIT			0x0040
#define		LAYER_AUXPAL_32BIT			0x0000

typedef struct {
	WORD	ctrl;		// the global blink control of S-BOX
	
}	stPACKED SBOX_BLK_INFO, *PSBOX_BLK_INFO;

#define		SBOX_BORDER_BLINK_ON		0x0010
#define		SBOX_BORDER_BLINK_OFF		0x0000

#define		SBOX_PLANE_BLINK_ON			0x0020
#define		SBOX_PLANE_BLINK_OFF		0x0000

#define		SBOX_BLINK_INVERSE			0x0040
#define		SBOX_BLINK_BLACK			0x0000

#define		SBOX_BLINK_CYCLE_MASK		0x000f

typedef struct {
	WORD	ctrl;		// the display control of sbox layer
	BYTE	alpha;		// the alpha-blending of sbox layer
	BYTE	index;		// the number of index of sbox layer

	BYTE	h_thk;		// the h-thickness of border of sbox
	BYTE	v_thk;		// the v-thickness of border of sbox

	MDIN_RECT_BOX	stRECT;		// the coordinates of sbox
	MDIN_COLORRGB	b_color;	// the color of border of sbox
	MDIN_COLORRGB	p_color;	// the color of plain of sbox
	
}	stPACKED SBOX_CTL_INFO, *PSBOX_CTL_INFO;

typedef enum {
	SBOX_INDEX0 = 0, SBOX_INDEX1, SBOX_INDEX2, SBOX_INDEX3,
	SBOX_INDEX4, SBOX_INDEX5, SBOX_INDEX6, SBOX_INDEX7
	
}	SBOX_INDEX_t;

#define		SBOX_BORDER_ON			0x0001
#define		SBOX_BORDER_OFF			0x0000

#define		SBOX_PLANE_ON			0x0002
#define		SBOX_PLANE_OFF			0x0000

#define		SBOX_BLINK_ON			0x0004
#define		SBOX_BLINK_OFF			0x0000

#define		SBOX_OPAQUE				31
#define		SBOX_TRANSPARENT		0

// -----------------------------------------------------------------------------
// Exported function Prototype
// -----------------------------------------------------------------------------

MDIN_ERROR_t MDINOSD_SetBGLayerColor(MDIN_COLORRGB rgb);

MDIN_ERROR_t MDINOSD_SetBGBoxColor(MDIN_COLORRGB rgb);
MDIN_ERROR_t MDINOSD_SetBGBoxArea(BGBOX_INDEX_t nID, WORD x, WORD y, WORD w, WORD h);
MDIN_ERROR_t MDINOSD_EnableBGBox(BGBOX_INDEX_t nID, BOOL OnOff);

MDIN_ERROR_t MDINOSD_SetSpriteConfig(PSPRITE_CTL_INFO pINFO, SPRITE_INDEX_t nID);
MDIN_ERROR_t MDINOSD_EnableSprite(PSPRITE_CTL_INFO pINFO, BOOL OnOff);
MDIN_ERROR_t MDINOSD_SetSpritePosition(PSPRITE_CTL_INFO pINFO, WORD x, WORD y);
MDIN_ERROR_t MDINOSD_SetSpriteAlpha(PSPRITE_CTL_INFO pINFO, BYTE alpha);
MDIN_ERROR_t MDINOSD_SetSpritePaletteAddr(PSPRITE_CTL_INFO pINFO, BYTE addr);
MDIN_ERROR_t MDINOSD_SetSpriteBMPInfo(PSPRITE_CTL_INFO pINFO, WORD w, WORD h, DWORD addr);

MDIN_ERROR_t MDINOSD_SetLayerConfig(PLAYER_CTL_INFO pINFO, LAYER_INDEX_t nID);
MDIN_ERROR_t MDINOSD_EnableLayerRepeat(PLAYER_CTL_INFO pINFO, BOOL h_rpt, BOOL v_rpt);
MDIN_ERROR_t MDINOSD_EnableLayerCSCBypass(PLAYER_CTL_INFO pINFO, BOOL OnOff);
MDIN_ERROR_t MDINOSD_EnableLayerTPKeyRGB(PLAYER_CTL_INFO pINFO, BOOL OnOff);
MDIN_ERROR_t MDINOSD_EnableLayerTPKeyYUV(PLAYER_CTL_INFO pINFO, BOOL OnOff);
MDIN_ERROR_t MDINOSD_SetLayerPalette(PLAYER_CTL_INFO pINFO, WORD addr, WORD size, PBYTE pBuff);

MDIN_ERROR_t MDINOSD_SetBitmapData(DWORD addr, DWORD size, PBYTE pBuff);
MDIN_ERROR_t MDINOSD_SetGlobalColorKeyRGB(MDIN_COLORRGB rgb);
MDIN_ERROR_t MDINOSD_SetGlobalColorKeyYUV(MDIN_COLORYBR ybr);
MDIN_ERROR_t MDINAUX_SetGlobalColorKeyRGB(MDIN_COLORRGB rgb);

MDIN_ERROR_t MDINOSD_SetSBoxConfig(PSBOX_CTL_INFO pINFO, SBOX_INDEX_t nID);
MDIN_ERROR_t MDINOSD_EnableSBoxBorder(PSBOX_CTL_INFO pINFO, BOOL OnOff);
MDIN_ERROR_t MDINOSD_EnableSBoxPlane(PSBOX_CTL_INFO pINFO, BOOL OnOff);
MDIN_ERROR_t MDINOSD_EnableSBoxBlink(PSBOX_CTL_INFO pINFO, BOOL OnOff);
MDIN_ERROR_t MDINOSD_SetSBoxAlpha(PSBOX_CTL_INFO pINFO, BYTE alpha);
MDIN_ERROR_t MDINOSD_SetSBoxBorderThickH(PSBOX_CTL_INFO pINFO, BYTE h_thk);
MDIN_ERROR_t MDINOSD_SetSBoxBorderThickV(PSBOX_CTL_INFO pINFO, BYTE v_thk);
MDIN_ERROR_t MDINOSD_SetSBoxArea(PSBOX_CTL_INFO pINFO, WORD lx, WORD ly, WORD rx, WORD ry);
MDIN_ERROR_t MDINOSD_SetSBoxBorderColor(PSBOX_CTL_INFO pINFO, MDIN_COLORRGB rgb);
MDIN_ERROR_t MDINOSD_SetSBoxPlaneColor(PSBOX_CTL_INFO pINFO, MDIN_COLORRGB rgb);
MDIN_ERROR_t MDINOSD_SetGlobalSBoxBlink(SBOX_BLK_INFO stBLK);
void DrawCamName(BYTE dmode, BYTE resol);


#endif	/* __MDINOSD_H__ */
