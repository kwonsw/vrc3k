//----------------------------------------------------------------------------------------------------------------------
// (C) Copyright 2008  Macro Image Technology Co., LTd. , All rights reserved
// 
// This source code is the property of Macro Image Technology and is provided
// pursuant to a Software License Agreement. This code's reuse and distribution
// without Macro Image Technology's permission is strictly limited by the confidential
// information provisions of the Software License Agreement.
//-----------------------------------------------------------------------------------------------------------------------
//
// File Name   		:	MDINGAC.C
// Description 		:
// Ref. Docment		: 
// Revision History 	:

//--------------------------------------------------------------------------------------------------------------------------
// Note for GAC-char/copy/fill operation
//--------------------------------------------------------------------------------------------------------------------------
// GET_MDIN_IRQ() is GPIO(IN)/IRQ pin of CPU. This is connected to INT_OUT of MDIN3xx.

// ----------------------------------------------------------------------
// Include files
// ----------------------------------------------------------------------
#include	"stm32f10x.h"
#include	"mdin3xx.h"

extern GACMAP_FONT_INFO stFONT;

// ----------------------------------------------------------------------
// Struct/Union Types and define
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Static Global Data section variables
// ----------------------------------------------------------------------
static WORD gridGAC, autoGAC;

// ----------------------------------------------------------------------
// External Variable 
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Static Prototype Functions
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Static functions
// ----------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINGAC_SetFontMode(PGACMAP_FONT_INFO pGAC, PSPRITE_CTL_INFO pSPT)
{
	WORD mode, bpp, unit = MDIN3xx_GetSizeOfBank()*2;

	pGAC->r = ADDR2ROW(pGAC->addr, unit);
	pGAC->c = ADDR2COL(pGAC->addr, unit)/8;				// for 8Byte-unit

	pSPT->stBMP.r = ADDR2ROW(pSPT->stBMP.addr, unit);
	pSPT->stBMP.c = ADDR2COL(pSPT->stBMP.addr, unit)/8;	// for 8Byte-unit

	switch (pGAC->attb&GACMAP_COLORMODE_MASK) {
		case GACMAP_1BITPALT_COLOR:		bpp = 1; mode = (5<<4)|3; break;
		case GACMAP_4BITPALT_COLOR:		bpp = 4; mode = (3<<4)|3; break;
		case GACMAP_8BITPALT_COLOR:		bpp = 8; mode = (2<<4)|2; break;
		default:						return MDIN_INVALID_PARAM;
	}

	// gac_pixel_precision - precision
	if (MDINHIF_RegField(MDIN_HOST_ID, 0x058, 0, 8, mode)) return MDIN_I2C_ERROR;

	// gac_src_start_rb
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x05a, pGAC->r)) return MDIN_I2C_ERROR;

	// gac_dst_start_rb
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x05c, pSPT->stBMP.r)) return MDIN_I2C_ERROR;

	// gac_start_col
	mode = MAKEWORD(pGAC->c, pSPT->stBMP.c);
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x05e, mode)) return MDIN_I2C_ERROR;

	// gac_char_size
	mode = MAKEWORD(pGAC->w, pGAC->h);
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x06a, mode)) return MDIN_I2C_ERROR;

	// gac_src_byte_pitch
	mode = bpp*((pGAC->w<=16)? 2 : 4);
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x06b, mode)) return MDIN_I2C_ERROR;

	// gac_dst_byte_pitch
	bpp = (bpp==1)? 4 : bpp;	mode = (pSPT->stBMP.w)*bpp/8;	mode = (mode+7)/8*8;
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x067, mode)) return MDIN_I2C_ERROR;

	// gac_process_ctrl
	gridGAC = (pGAC->attb&MDIN_GAC_32Hx16V_GRID);	// set ch-grid
	autoGAC = (pGAC->attb&MDIN_GAC_BOTH_AUTOINC);	// set xy-auto

	mode = ((autoGAC|gridGAC)<<6)|((pGAC->mode&MDIN_GAC_DRAW_XYMODE)? 0x32 : 0x22);
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x064, mode)) return MDIN_I2C_ERROR;

	// gac_dst_start_pos
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x068, 0x0000)) return MDIN_I2C_ERROR;	// fix 0
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x069, 0x0000)) return MDIN_I2C_ERROR;	// fix 0

	// set sprite information
	pSPT->ctrl &= ~SPRITE_COLORMODE_MASK;
	if	(bpp==4) pSPT->ctrl |= SPRITE_4BITPALT_COLOR;
	else		 pSPT->ctrl |= SPRITE_8BITPALT_COLOR;

	return MDINOSD_SetSpriteConfig(pSPT, (SPRITE_INDEX_t)pSPT->index);
}

#if	defined(SYSTEM_USE_MDIN380)&&defined(SYSTEM_USE_PCI_HIF)
//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINGAC_MultiWrite(BYTE nID, DWORD rAddr, PBYTE pBuff, DWORD bytes)
{
	WORD i;

	// enable ga_gac_finish irq
	if (MDIN3xx_EnableIRQ(MDIN_IRQ_GA_FINISH, ON)) return MDIN_I2C_ERROR;
	if (MDIN3xx_GetParseIRQ()) return MDIN_I2C_ERROR;		// clear irq

	for (i=0; i<bytes/2; i++) {
		if (MDIN3xx_GetParseIRQ()) return MDIN_I2C_ERROR;	// clear irq
		if (MDINHIF_RegWrite(nID, rAddr+i, ((PWORD)pBuff)[i])) return MDIN_I2C_ERROR;
		while (GET_MDIN_IRQ()==HIGH);		// wait for /IRQ of gac-finish
	}

	// disable ga_gac_finish irq
	if (MDIN3xx_GetParseIRQ()) return MDIN_I2C_ERROR;		// clear irq
	if (MDIN3xx_EnableIRQ(MDIN_IRQ_GA_FINISH, OFF)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}
#endif	/* defined(SYSTEM_USE_PCI_HIF) */

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINGAC_SetDrawCHMode(BYTE y, BYTE x, PBYTE pBuff, BYTE len, BYTE attb)
{
	WORD i, addr, pChar[32];

	if 		(gridGAC==MDIN_GAC_16Vx32H_GRID) addr = ((WORD)y<<5)|x;
	else if (gridGAC==MDIN_GAC_32Vx16H_GRID) addr = ((WORD)y<<4)|x;
	else if (gridGAC==MDIN_GAC_16Hx32V_GRID) addr = ((WORD)x<<4)|y;
	else									 addr = ((WORD)x<<5)|y;
	
	//printf("\r\nMDINGAC_SetDrawCHMode funciont\r\n");
	//printf("addr --> %x\r\n",addr);
	for (i=0; i<len; i++){
		pChar[i] = MAKEWORD(attb, pBuff[i]);
		//printf("pchar[%x] ====> %x\r\n", i, pChar[i]);
	}

#if	defined(SYSTEM_USE_MDIN380)&&defined(SYSTEM_USE_PCI_HIF)
	if (MDINGAC_MultiWrite(MDIN_HOST_ID, 0x200|addr, (PBYTE)pChar, len*2)) return MDIN_I2C_ERROR;
#else
	if (MDINI2C_MultiWrite(MDIN_HOST_ID, 0x200|addr, (PBYTE)pChar, len*2)) return MDIN_I2C_ERROR;
#endif
	return MDIN_NO_ERROR;
}


//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINGAC_SetDrawXYMode(WORD y, WORD x, PBYTE pBuff, BYTE len, BYTE attb)
{
	WORD i, pChar[100];
	WORD posix, posiy;

	if (len == 0) return MDIN_NO_ERROR;
	
	posix = x * stFONT.w;
	posiy = y * stFONT.h;

	if (MDINI2C_RegWrite(MDIN_HOST_ID, 0x068, posix)) return MDIN_I2C_ERROR;
	if (MDINI2C_RegWrite(MDIN_HOST_ID, 0x069, posiy)) return MDIN_I2C_ERROR;

	for (i=0; i<len; i++) pChar[i] = MAKEWORD(attb, pBuff[i]);

#if	defined(SYSTEM_USE_MDIN380)&&defined(SYSTEM_USE_PCI_HIF)
	if (MDINGAC_MultiWrite(MDIN_HOST_ID, 0x200, (PBYTE)pChar, len*2)) return MDIN_I2C_ERROR;
#else
	if (MDINI2C_MultiWrite(MDIN_HOST_ID, 0x200, (PBYTE)pChar, len*2)) return MDIN_I2C_ERROR;
#endif

	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINGAC_SetCopyMode(PGACCOPY_BMP_INFO pGAC, PSPRITE_CTL_INFO pSPT)
{
	WORD mode, bpp, unit = MDIN3xx_GetSizeOfBank()*2;

	pGAC->r = ADDR2ROW(pGAC->addr, unit);
	pGAC->c = ADDR2COL(pGAC->addr, unit)/8;				// for 8Byte-unit

	pSPT->stBMP.r = ADDR2ROW(pSPT->stBMP.addr, unit);
	pSPT->stBMP.c = ADDR2COL(pSPT->stBMP.addr, unit)/8;	// for 8Byte-unit

	switch (pGAC->attb&GACMAP_COLORMODE_MASK) {
		case GACMAP_2BITPALT_COLOR:		bpp =  2; mode = (4<<4)|4; break;
		case GACMAP_4BITPALT_COLOR:		bpp =  4; mode = (3<<4)|3; break;
		case GACMAP_8BITPALT_COLOR:		bpp =  8; mode = (2<<4)|2; break;
		case GACMAP_16BITFULL_COLOR:	bpp = 16; mode = (1<<4)|1; break;
		case GACMAP_24BITFULL_COLOR:	bpp = 24; mode = (6<<4)|6; break;
		case GACMAP_32BITFULL_COLOR:	bpp = 32; mode = (0<<4)|0; break;
		default:						return MDIN_INVALID_PARAM;
	}

	// gac_pixel_precision - precision
	if (MDINHIF_RegField(MDIN_HOST_ID, 0x058, 0, 8, mode)) return MDIN_I2C_ERROR;

	// gac_src_start_rb
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x05a, pGAC->r)) return MDIN_I2C_ERROR;

	// gac_dst_start_rb
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x05c, pSPT->stBMP.r)) return MDIN_I2C_ERROR;

	// gac_start_col
	mode = MAKEWORD(pGAC->c, pSPT->stBMP.c);
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x05e, mode)) return MDIN_I2C_ERROR;

	// gac_src_byte_pitch
	mode = (pGAC->w)*bpp/8;
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x06b, mode)) return MDIN_I2C_ERROR;

	// gac_dst_byte_pitch
	mode = (pSPT->stBMP.w)*bpp/8;
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x067, mode)) return MDIN_I2C_ERROR;

	// gac_process_ctrl
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x064, 0x0010)) return MDIN_I2C_ERROR;

	return MDINOSD_SetSpriteConfig(pSPT, (SPRITE_INDEX_t)pSPT->index);
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINGAC_CopyRectangle(PMDIN_AREA_BOX pSRC, PMDIN_AREA_BOX pDST)
{
#if	defined(SYSTEM_USE_MDIN380)&&defined(SYSTEM_USE_PCI_HIF)
	if (MDINHIF_MultiWrite(MDIN_HOST_ID, 0x06c, (PBYTE)pSRC, 8)) return MDIN_I2C_ERROR;
	if (MDINHIF_MultiWrite(MDIN_HOST_ID, 0x068, (PBYTE)pDST, 4)) return MDIN_I2C_ERROR;
#else
	if (MDINI2C_MultiWrite(MDIN_HOST_ID, 0x06c, (PBYTE)pSRC, 8)) return MDIN_I2C_ERROR;
	if (MDINI2C_MultiWrite(MDIN_HOST_ID, 0x068, (PBYTE)pDST, 4)) return MDIN_I2C_ERROR;
#endif

	// enable ga_gac_finish irq
	if (MDIN3xx_EnableIRQ(MDIN_IRQ_GA_FINISH, ON)) return MDIN_I2C_ERROR;
	if (MDIN3xx_GetParseIRQ()) return MDIN_I2C_ERROR;		// clear irq

	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x064, 0x0011)) return MDIN_I2C_ERROR;
	while (GET_MDIN_IRQ()==HIGH);			// wait for /IRQ of gac-finish

	// disable ga_gac_finish irq
	if (MDIN3xx_GetParseIRQ()) return MDIN_I2C_ERROR;		// clear irq
	if (MDIN3xx_EnableIRQ(MDIN_IRQ_GA_FINISH, OFF)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINGAC_SetFillMode(PGACFILL_BOX_INFO pGAC, PSPRITE_CTL_INFO pSPT)
{
	WORD mode, bpp, unit = MDIN3xx_GetSizeOfBank()*2;

	pSPT->stBMP.r = ADDR2ROW(pSPT->stBMP.addr, unit);
	pSPT->stBMP.c = ADDR2COL(pSPT->stBMP.addr, unit)/8;	// for 8Byte-unit

	switch (pGAC->attb&GACMAP_COLORMODE_MASK) {
		case GACMAP_2BITPALT_COLOR:		bpp =  2; mode = (4<<4)|4; break;
		case GACMAP_4BITPALT_COLOR:		bpp =  4; mode = (3<<4)|3; break;
		case GACMAP_8BITPALT_COLOR:		bpp =  8; mode = (2<<4)|2; break;
		case GACMAP_16BITFULL_COLOR:	bpp = 16; mode = (1<<4)|1; break;
		case GACMAP_32BITFULL_COLOR:	bpp = 32; mode = (0<<4)|0; break;
		default:						return MDIN_INVALID_PARAM;
	}

	pGAC->mode = pSPT->ctrl&SPRITE_COLORMODE_MASK;	// replace attribute

	// gac_pixel_precision - precision
	if (MDINHIF_RegField(MDIN_HOST_ID, 0x058, 0, 8, mode)) return MDIN_I2C_ERROR;

	// gac_src_start_rb, gac_dst_start_rb
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x05a, pSPT->stBMP.r)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x05c, pSPT->stBMP.r)) return MDIN_I2C_ERROR;

	// gac_start_col
	mode = MAKEWORD(pSPT->stBMP.c, pSPT->stBMP.c);
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x05e, mode)) return MDIN_I2C_ERROR;

	// gac_src_byte_pitch, gac_dst_byte_pitch
	mode = (pSPT->stBMP.w)*bpp/8;
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x06b, mode)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x067, mode)) return MDIN_I2C_ERROR;

	// gac_process_ctrl
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x064, 0x0018)) return MDIN_I2C_ERROR;

	return MDINOSD_SetSpriteConfig(pSPT, (SPRITE_INDEX_t)pSPT->index);
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINGAC_FillRectangle(PGACFILL_BOX_INFO pGAC, DWORD color)
{
	switch (pGAC->mode) {
		case SPRITE_aRGB0565_16BIT:
			color = aRGB0565(GetA(color),GetR(color),GetG(color),GetB(color));
			color = MAKEDWORD(LOWORD(color), LOWORD(color)); break;

		case SPRITE_aRGB4444_16BIT:
			color = aRGB4444(GetA(color),GetR(color),GetG(color),GetB(color));
			color = MAKEDWORD(LOWORD(color), LOWORD(color)); break;

		case SPRITE_aRGB1555_16BIT:
			color = aRGB1555(GetA(color),GetR(color),GetG(color),GetB(color));
			color = MAKEDWORD(LOWORD(color), LOWORD(color)); break;

		case SPRITE_aYUV0655_YC444:
			color = aYUV0655(GetA(color),GetR(color),GetG(color),GetB(color));
			color = MAKEDWORD(LOWORD(color), LOWORD(color)); break;

		case SPRITE_aRGB5888_32BIT:
			color = aRGB5888(GetA(color),GetR(color),GetG(color),GetB(color));
			color = MAKEDWORD(HIWORD(color), LOWORD(color)); break;

		case SPRITE_2BITPALT_COLOR:		color &= 0x03;
			color = (color<<14)|((color<<12)+(color<<10))|(color<<8)
				  | (color<< 6)|((color<< 4)+(color<< 2))|(color<<0);
			color = MAKEDWORD(LOWORD(color), LOWORD(color)); break;

		case SPRITE_4BITPALT_COLOR:		color &= 0x0f;
			color = (color<<12)|((color<< 8)+(color<< 4))|(color<<0);
			color = MAKEDWORD(LOWORD(color), LOWORD(color)); break;

		case SPRITE_8BITPALT_COLOR:		color &= 0xff;
			color = (color<<24)|((color<<16)+(color<< 8))|(color<<0);
			color = MAKEDWORD(HIWORD(color), LOWORD(color)); break;

		default:						return MDIN_INVALID_PARAM;
	}

#if	defined(SYSTEM_USE_MDIN380)&&defined(SYSTEM_USE_PCI_HIF)
	// gac_src_window, gac_dst_window
	if (MDINHIF_MultiWrite(MDIN_HOST_ID, 0x06c, (PBYTE)pGAC, 8)) return MDIN_I2C_ERROR;
	if (MDINHIF_MultiWrite(MDIN_HOST_ID, 0x068, (PBYTE)pGAC, 4)) return MDIN_I2C_ERROR;

	// ga_bmp_data_msb, ga_bmp_data_lsb
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x04a, HIWORD(color))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x04b, LOWORD(color))) return MDIN_I2C_ERROR;
#else
	// gac_src_window, gac_dst_window
	if (MDINI2C_MultiWrite(MDIN_HOST_ID, 0x06c, (PBYTE)pGAC, 8)) return MDIN_I2C_ERROR;
	if (MDINI2C_MultiWrite(MDIN_HOST_ID, 0x068, (PBYTE)pGAC, 4)) return MDIN_I2C_ERROR;

	// ga_bmp_data_msb, ga_bmp_data_lsb
	if (MDINI2C_RegWrite(MDIN_HOST_ID, 0x04a, HIWORD(color))) return MDIN_I2C_ERROR;
	if (MDINI2C_RegWrite(MDIN_HOST_ID, 0x04b, LOWORD(color))) return MDIN_I2C_ERROR;
#endif

	// enable ga_gac_finish irq
	if (MDIN3xx_EnableIRQ(MDIN_IRQ_GA_FINISH, ON)) return MDIN_I2C_ERROR;
	if (MDIN3xx_GetParseIRQ()) return MDIN_I2C_ERROR;		// clear irq

	if (MDINHIF_RegWrite(MDIN_HOST_ID, 0x064, 0x0019)) return MDIN_I2C_ERROR;
	while (GET_MDIN_IRQ()==HIGH);			// wait for /IRQ of gac-finish

	// disable ga_gac_finish irq
	if (MDIN3xx_GetParseIRQ()) return MDIN_I2C_ERROR;		// clear irq
	if (MDIN3xx_EnableIRQ(MDIN_IRQ_GA_FINISH, OFF)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

/*  FILE_END_HERE */
