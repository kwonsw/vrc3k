//----------------------------------------------------------------------------------------------------------------------
// (C) Copyright 2008  Macro Image Technology Co., LTd. , All rights reserved
// 
// This source code is the property of Macro Image Technology and is provided
// pursuant to a Software License Agreement. This code's reuse and distribution
// without Macro Image Technology's permission is strictly limited by the confidential
// information provisions of the Software License Agreement.
//-----------------------------------------------------------------------------------------------------------------------
//
// File Name   		:	HDMITX.C
// Description 		:
// Ref. Docment		: 
// Revision History 	:

// ----------------------------------------------------------------------
// Include files
// ----------------------------------------------------------------------
#include	<string.h>
#include	"mdin3xx.h"

// -----------------------------------------------------------------------------
// Struct/Union Types and define
// -----------------------------------------------------------------------------

// ----------------------------------------------------------------------
// Static Global Data section variables
// ----------------------------------------------------------------------
static BOOL SetPAGE, GetHDMI = 0;
static BYTE GetEDID, GetPLUG, GetMDDC;

#if __MDINHTX_DBGPRT__ == 1
static BYTE OldPROC = 0xff;
#endif

// ----------------------------------------------------------------------
// External Variable 
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Static Prototype Functions
// ----------------------------------------------------------------------
static MDIN_ERROR_t MDINHTX_InitModeDVI(PMDIN_VIDEO_INFO pINFO);

// ----------------------------------------------------------------------
// Static functions
// ----------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------------
// Drive Function for MDDC interface
//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINHTX_GetMDDCProcDone(void)
{
	WORD rVal = 0x10, count = 100;

	while (count&&(rVal==0x10)) {
		if (MDINHIF_RegRead(MDIN_HDMI_ID, 0x0f2, &rVal)) return MDIN_I2C_ERROR;
		rVal &= 0x10;	count--;	MDINDLY_10uSec(10);		// delay 100us
	}

#if __MDINHTX_DBGPRT__ == 1
	if (count==0) UARTprintf("DDC method is failure. DDC FIFO is busy.\r\n");
#endif

	return (count)? MDIN_NO_ERROR : MDIN_TIMEOUT_ERROR;
}

static MDIN_ERROR_t MDINHTX_SetMDDCCmd(PMDIN_HDMIMDDC_INFO pMDDC)
{
	if (MDINHIF_MultiWrite(MDIN_HDMI_ID, 0x0ec, (PBYTE)pMDDC, 6)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x0f2, 0x0900)) return MDIN_I2C_ERROR;	// clear FIFO

	if (pMDDC->cmd==0x06) return MDIN_NO_ERROR;		// sequential write commnad
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x0f2, MAKEWORD(pMDDC->cmd,0))) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_GetMDDCStatus(void)
{
	WORD rVal;

	if (MDINHIF_RegRead(MDIN_HDMI_ID, 0x0f2, &rVal)) return MDIN_I2C_ERROR;

	// check BUS_LOW, NO_ACK, IN_PROG, FIFO_FULL
	if ((rVal&0x78)==0) return MDIN_NO_ERROR;

	// can happen if Rx is clock stretching the SCL line. DDC bus unusable
	if ((rVal&0x20)!=0) return MDIN_DDC_ACK_ERROR;

	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x0f2, 0x0f00)) return MDIN_I2C_ERROR;	// ABORT
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x0f2, 0x0a00)) return MDIN_I2C_ERROR;	// CLOCK
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_GetMDDCBuff(PMDIN_HDMIMDDC_INFO pMDDC)
{
	WORD i, rVal;

	for (i=0; i<pMDDC->bytes; i++) {
		if (MDINHIF_RegRead(MDIN_HDMI_ID, 0x0f4, &rVal)) return MDIN_I2C_ERROR;
		pMDDC->pBuff[i] = LOBYTE(rVal);
	}
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_ReadMDDC(PMDIN_HDMIMDDC_INFO pMDDC)
{
	MDIN_ERROR_t err;

	err = MDINHTX_GetMDDCProcDone(); if (err==MDIN_I2C_ERROR) return err;

	// Abort Master DCC operation and Clear FIFO pointer
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x0f2, 0x0900)) return MDIN_I2C_ERROR;

	if (MDINHTX_SetMDDCCmd(pMDDC)) return MDIN_I2C_ERROR;
	err = MDINHTX_GetMDDCProcDone(); if (err==MDIN_I2C_ERROR) return err;

	if (MDINHTX_GetMDDCBuff(pMDDC)) return MDIN_I2C_ERROR;

	err = MDINHTX_GetMDDCStatus(); if (err==MDIN_I2C_ERROR) return err;
	if (err) GetMDDC = MDIN_DDC_ACK_ERROR;

#if __MDINHTX_DBGPRT__ == 1
	if (err) UARTprintf("DDC Read method is failure. No ACK.\r\n");
#endif

	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
// Drive Function for EDID Parsing
//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINHTX_ReadEDID(BYTE rAddr, PBYTE pBuff, WORD bytes)
{
	MDIN_HDMIMDDC_INFO stMDDC; WORD rVal;

	if (MDINHIF_RegRead(MDIN_HDMI_ID, 0x0ee, &rVal)) return MDIN_I2C_ERROR;
	rVal = LOBYTE(rVal);

	stMDDC.sAddr	= MAKEWORD(0xa0, 0);
	stMDDC.rAddr	= MAKEWORD(rAddr, rVal);
	stMDDC.bytes	= bytes;
	stMDDC.pBuff	= pBuff;
	stMDDC.cmd		= (rVal)? 4 : 2;	// enhanced read or sequential read
	return MDINHTX_ReadMDDC(&stMDDC);
}

static MDIN_ERROR_t MDINHTX_ParseHeader(PMDIN_HDMICTRL_INFO pCTL)
{
	BYTE i, rBuff[8];

	if (MDINHTX_ReadEDID(0x00, rBuff, 8)) return MDIN_I2C_ERROR;

	if ((rBuff[0]|rBuff[7])) pCTL->err = EDID_BAD_HEADER;

	for (i=1; i<7; i++) {
		if (rBuff[i]!=0xff) pCTL->err = EDID_BAD_HEADER;
	}
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_ParseVersion(PMDIN_HDMICTRL_INFO pCTL)
{
	BYTE rBuff[2];

	if (MDINHTX_ReadEDID(0x12, rBuff, 2)) return MDIN_I2C_ERROR;

	if ((rBuff[0]!=1)||(rBuff[1]<3)) pCTL->err = EDID_VER_NOT861B;
	else							 pCTL->err = MDIN_NO_ERROR;

	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_CheckCRC(BYTE rAddr)
{
	BYTE i, j, rBuff[8], crc = 0;

	for (i=0; i<16; i++) {
		if (MDINHTX_ReadEDID(rAddr+i*8, rBuff, 8)) return MDIN_I2C_ERROR;

		for (j=0; j<8; j++) crc += rBuff[j];
	}
	return (crc)? MDIN_DDC_CRC_ERROR : MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_Check1stCRC(PMDIN_HDMICTRL_INFO pCTL)
{
	MDIN_ERROR_t err;

	err = MDINHTX_CheckCRC(0x00);
	if (err!=MDIN_DDC_CRC_ERROR) return err;

	pCTL->err = EDID_1ST_CRC_ERR;
	if (MDINHTX_ParseVersion(pCTL)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_Check2ndCRC(PMDIN_HDMICTRL_INFO pCTL)
{
	MDIN_ERROR_t err;

	err = MDINHTX_CheckCRC(0x80);
	if (err!=MDIN_DDC_CRC_ERROR) return err;

	pCTL->err = EDID_2ND_CRC_ERR;
	if (MDINHTX_ParseVersion(pCTL)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_BasicParse(PMDIN_HDMICTRL_INFO pCTL)
{
	if (MDINHTX_ParseHeader(pCTL)) return MDIN_I2C_ERROR;
	if (pCTL->err) return MDIN_NO_ERROR;

	if (MDINHTX_Check1stCRC(pCTL)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_GetVendorAddr(PBYTE pBuff)
{
	BYTE rAddr, eAddr, rBuff[2];

	rAddr = (SetPAGE)? 0x02 : 0x82;
	if (MDINHTX_ReadEDID(rAddr, rBuff, 2)) return MDIN_I2C_ERROR;
	if (rBuff[0]<4) return MDIN_NO_ERROR;

	rAddr = ((SetPAGE)? 0x04 : 0x84);
	eAddr = ((SetPAGE)? 0x03 : 0x80) + rBuff[0];

	while (rAddr<eAddr) {
		if (MDINHTX_ReadEDID(rAddr, rBuff, 1)) return MDIN_I2C_ERROR;

		if ((rBuff[0]&0xe0)==0x60) {*pBuff = rAddr; break;}
		rAddr += ((rBuff[0]&0x1f) + 1);
	}
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_CheckHDMISignature(PMDIN_HDMICTRL_INFO pCTL)
{
	BYTE rAddr = 0, rBuff[3];

	pCTL->phy = 0xffff;	// clear physical address
	if (MDINHTX_GetVendorAddr(&rAddr)) return MDIN_I2C_ERROR;

	// HDMI Signature block not found
	if (rAddr==0) {pCTL->err = EDID_VER_NOTHDMI; return MDIN_NO_ERROR;}

	if (MDINHTX_ReadEDID(rAddr+1, rBuff, 3)) return MDIN_I2C_ERROR;

	if (rBuff[0]!=0x03||rBuff[1]!=0x0c||rBuff[2]!=0x00) {
		pCTL->err = EDID_VER_NOTHDMI; return MDIN_NO_ERROR;
	}

	if (MDINHTX_ReadEDID(rAddr+4, rBuff, 2)) return MDIN_I2C_ERROR;
	pCTL->phy = MAKEWORD(rBuff[0], rBuff[1]);

#if __MDINHTX_DBGPRT__ == 1
	UARTprintf("PhyAddr = 0x%04x\r\n", pCTL->phy);
#endif

	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_SetNativeModeBy861B(PMDIN_HDMICTRL_INFO pCTL)
{
	BYTE rVal;

	if (MDINHTX_ReadEDID(0x83, &rVal, 1)) return MDIN_I2C_ERROR;

	switch (rVal&0x30) {
		case 0x10:	pCTL->mode = HDMI_OUT_SEP422_8; break;
		case 0x20:
		case 0x30:	pCTL->mode = HDMI_OUT_YUV444_8; break;
		default:	pCTL->mode = HDMI_OUT_RGB444_8; break;
	}

#if __MDINHTX_DBGPRT__ == 1
	UARTprintf(" DTV monitor supports: \n");
	if (rVal&0x80) UARTprintf(" Underscan");
	if (rVal&0x40) UARTprintf(" Basic audio");
	if (rVal&0x20) UARTprintf(" YCbCr 4:4:4");
	if (rVal&0x10) UARTprintf(" YCbCr 4:2:2");
	UARTprintf("\r\n");
#endif

	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_SetNativeFrmtByDTD(PMDIN_HDMICTRL_INFO pCTL)
{
	BYTE i, rBuff[6]; WORD blkH, blkV, totH, totV, posH, posV;

	pCTL->frmt = 0xff;		// clear native format
	blkH = blkV = totH = totV = posH = posV = 0;

	for (i=0; i<4; i++) {
		if (MDINHTX_ReadEDID(i*18+0x36, rBuff, 2)) return MDIN_I2C_ERROR;
		if (rBuff[0]==0&&rBuff[1]==0) continue;

		if (MDINHTX_ReadEDID(i*18+0x38, rBuff, 6)) return MDIN_I2C_ERROR;
		blkH = MAKEWORD(LO4BIT(rBuff[2]),rBuff[1]);
		blkV = MAKEWORD(LO4BIT(rBuff[5]),rBuff[4]);
		totH = MAKEWORD(HI4BIT(rBuff[2]),rBuff[0]) + blkH;
		totV = MAKEWORD(HI4BIT(rBuff[5]),rBuff[3]) + blkV;

		if (MDINHTX_ReadEDID(i*18+0x3e, rBuff, 4)) return MDIN_I2C_ERROR;
		posH = blkH - MAKEWORD(HI4BIT(rBuff[3])>>2,rBuff[0]);
		posV = blkV - MAKEBYTE(LO4BIT(rBuff[3])>>2,HI4BIT(rBuff[2]));

		break;
	}

#if __MDINHTX_DBGPRT__ == 1
	UARTprintf("totH=%d, totV=%d, posH=%d, posV=%d\r\n", totH, totV, posH, posV);
#endif

	for (i=0; i<VIDOUT_FORMAT_END; i++) {
		if (totH!=defMDINHTXVideo[i].stFREQ.pixels) continue;
		if (totV!=defMDINHTXVideo[i].stFREQ.v_line) continue;
		if (posH!=defMDINHTXVideo[i].stWIND.x) continue;
		if (posV!=defMDINHTXVideo[i].stWIND.y) continue;

		pCTL->frmt = defMDINHTXVideo[i].stMODE.id_1; break;
	}

#if __MDINHTX_DBGPRT__ == 1
	UARTprintf("Native Format from DTD = %d\r\n", pCTL->frmt);
#endif

	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_SetNativeFrmtByVID(PMDIN_HDMICTRL_INFO pCTL)
{
	BYTE rAddr, tagID, rVal, bytes;

	rAddr = (SetPAGE)? 0x02 : 0x82;
	if (MDINHTX_ReadEDID(rAddr, &rVal, 1)) return MDIN_I2C_ERROR;
	if (rVal==4||rVal==0) return MDIN_NO_ERROR;
	
	rAddr = (SetPAGE)? 0x04 : 0x84; 	bytes = rAddr + rVal;

	while (rAddr<bytes) {
		if (MDINHTX_ReadEDID(rAddr, &tagID, 1)) return MDIN_I2C_ERROR;
		if ((tagID&0xe0)==0x40) break;
		rAddr += ((tagID&0x1f)+1);	// next tag address
	}

	rAddr++; bytes = rAddr + (tagID&0x1f);
	while (rAddr<bytes) {
		if (MDINHTX_ReadEDID(rAddr++, &rVal, 1)) return MDIN_I2C_ERROR;
		if (rVal&0x80) {pCTL->frmt = rVal&0x7f; break;}

	#if __MDINHTX_DBGPRT__ == 1
		UARTprintf("non-Native Format from VID = %d\r\n", rVal);
	#endif
	}

#if __MDINHTX_DBGPRT__ == 1
	UARTprintf("Native Format from VID = %d\r\n", pCTL->frmt);
#endif

	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_ParseEDID(PMDIN_HDMICTRL_INFO pCTL)
{
	BYTE rVal, rNum, rAddr = 0;

	SetPAGE = 0;	// must preserve proc & auth value
	memset((PBYTE)pCTL, 0, sizeof(MDIN_HDMICTRL_INFO)-2);

	if (MDINHTX_BasicParse(pCTL)) return MDIN_I2C_ERROR;
	if (pCTL->err) return MDIN_NO_ERROR;

	// set native format from detailed timing descriptor
	if (MDINHTX_SetNativeFrmtByDTD(pCTL)) return MDIN_I2C_ERROR;

	// Check 861B extension
	if (MDINHTX_ReadEDID(0x7e, &rNum, 1)) return MDIN_I2C_ERROR;
	if (rNum==0) {
		pCTL->err = EDID_EXT_NOT861B; return MDIN_NO_ERROR;
	}

	// Check CRC, sum of extension (128 BYTEs) must be Zero (0)
	if (MDINHTX_Check2ndCRC(pCTL)) return MDIN_I2C_ERROR;
	if (pCTL->err) return MDIN_NO_ERROR;

	// case of NExtensions = 1
	if (rNum==1) {
		if (MDINHTX_CheckHDMISignature(pCTL)) return MDIN_I2C_ERROR;
		if (pCTL->err) return MDIN_NO_ERROR;
		if (MDINHTX_SetNativeModeBy861B(pCTL)) return MDIN_I2C_ERROR;
		if (MDINHTX_SetNativeFrmtByVID(pCTL)) return MDIN_I2C_ERROR;
		pCTL->type = HTX_DISPLAY_HDMI;
	#if __MDINHTX_DBGPRT__ == 1
		UARTprintf("************************ HDMI Mode!\r\n");
	#endif
		return MDIN_NO_ERROR;
	}

	// case of NExtensions > 1
	if (MDINHTX_ReadEDID(0x80, &rVal, 1)) return MDIN_I2C_ERROR;
	if (rVal!=0xf0) pCTL->err = EDID_MAP_NOBLOCK;
	if (pCTL->err) return MDIN_NO_ERROR;

	while (rNum--) {	rAddr++;
		if (MDINHIF_RegField(MDIN_HDMI_ID, 0xee, 0, 8, rAddr)) return MDIN_I2C_ERROR;

		if (MDINHTX_ReadEDID(0x00, &rVal, 1)) return MDIN_I2C_ERROR;
		if ((rVal&0x02)==0) continue;

		if (MDINHTX_Check1stCRC(pCTL)) return MDIN_I2C_ERROR;
		if (pCTL->err) break;

		SetPAGE = 1;
		if (MDINHTX_CheckHDMISignature(pCTL)) return MDIN_I2C_ERROR;
		if (pCTL->err) break;

		pCTL->type = HTX_DISPLAY_HDMI;
//		if (MDINHTX_SetNativeModeBy861B(pCTL)) return MDIN_I2C_ERROR;
		if (MDINHTX_SetNativeFrmtByVID(pCTL)) return MDIN_I2C_ERROR;

		SetPAGE = 0;
		if (MDINHTX_Check2ndCRC(pCTL)) return MDIN_I2C_ERROR;
		if (pCTL->err) break;

		if (MDINHTX_SetNativeModeBy861B(pCTL)) return MDIN_I2C_ERROR;
		if (MDINHTX_SetNativeFrmtByVID(pCTL)) return MDIN_I2C_ERROR;
		if (pCTL->type==HTX_DISPLAY_HDMI) break;
	}

	if (MDINHIF_RegField(MDIN_HDMI_ID, 0xee, 0, 8, 0x00)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_GetParseEDID(PMDIN_VIDEO_INFO pINFO)
{
	PMDIN_HTXVIDEO_INFO pVID = (PMDIN_HTXVIDEO_INFO)&pINFO->stVID_h;
	PMDIN_HDMICTRL_INFO pCTL = (PMDIN_HDMICTRL_INFO)&pINFO->stCTL_h;

#if __MDINHTX_DBGPRT__ == 1
	UARTprintf("========== Parse EDID (1st) ============\r\n");
#endif

	GetMDDC = MDIN_NO_ERROR;	// clear MDDC error
	if (MDINHTX_ParseEDID(pCTL)) return MDIN_I2C_ERROR;

#if __MDINHTX_DBGPRT__ == 1
	if (pCTL->err) UARTprintf("EDID Error %02x\r\n", pCTL->err);
#endif

	if (GetMDDC==MDIN_DDC_ACK_ERROR) {
#if SYSTEM_USE_HTX_HDCP == 1
		if (MDINHTX_EnableEncryption(OFF)) return MDIN_I2C_ERROR;
		pCTL->auth = HDCP_REAUTH_REQ;	// force re-authentication
#endif
	}

	// increase EDID read count
	GetEDID += (pCTL->err==MDIN_NO_ERROR)? -GetEDID : 1;

	// EDID error, or not supported version
	if (pCTL->err>=8) pCTL->proc = HTX_CABLE_DVI_OUT;
	if (pCTL->err!=0) return MDIN_NO_ERROR;

	if (pCTL->type==HTX_DISPLAY_HDMI)	pCTL->proc = HTX_CABLE_HDMI_OUT;
	else								pCTL->proc = HTX_CABLE_DVI_OUT;

	if (pVID->fine&HDMI_USE_FORCE_DVI)	pCTL->proc = HTX_CABLE_DVI_OUT;
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
// Drive Function for InfoFrame (AVI, AUD, CP)
//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINHTX_GetControlPKTOff(void)
{
	WORD rVal = 0x0800, count = 100;

	while (count&&(rVal==0x0800)) {
		if (MDINHIF_RegRead(MDIN_HDMI_ID, 0x13e, &rVal)) return MDIN_I2C_ERROR;
//		rVal &= 0x0800; count--;	MDINDLY_10uSec(5);	// delay 50us
		rVal &= 0x0800; count--;	MDINDLY_mSec(1);	// delay 1ms
	}

#if __MDINHTX_DBGPRT__ == 1
	UARTprintf("(Disable CP) val=0x%02X, count=%d\r\n", rVal, count);
#endif

	return (count)? MDIN_NO_ERROR : MDIN_TIMEOUT_ERROR;
}

static MDIN_ERROR_t MDINHTX_EnableControlPKT(BOOL OnOff)
{
	MDIN_ERROR_t err;	WORD rVal;

	if (GetHDMI==FALSE) return MDIN_NO_ERROR;	// check sink is HDMI

	if (MDINHIF_RegRead(MDIN_HDMI_ID, 0x1de, &rVal)) return MDIN_I2C_ERROR;
	if ( OnOff&&(HIBYTE(rVal)==0x01)) return MDIN_NO_ERROR;	// already mute set
	if (!OnOff&&(HIBYTE(rVal)==0x10)) return MDIN_NO_ERROR;	// already mute clear

	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x13e, 10, 2, 0)) return MDIN_I2C_ERROR;

	err = MDINHTX_GetControlPKTOff(); if (err==MDIN_I2C_ERROR) return err;
	if (err==MDIN_TIMEOUT_ERROR) return MDIN_NO_ERROR;

	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x1de, 8, 8, (OnOff)? 0x01 : 0x10)) return MDIN_I2C_ERROR;

	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x13e, 10, 2, 3)) return MDIN_I2C_ERROR;

	if (MDINHIF_RegRead(MDIN_HDMI_ID, 0x13e, &rVal)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_GetInfoFrameOff(BYTE mask)
{
	WORD rVal = mask, count = 100;

	while (count&&(rVal==mask)) {
		if (MDINHIF_RegRead(MDIN_HDMI_ID, 0x13e, &rVal)) return MDIN_I2C_ERROR;
		rVal &= mask; count--;	MDINDLY_mSec(1);	// delay 1ms
	}

#if __MDINHTX_DBGPRT__ == 1
	UARTprintf("(Disable InfoFrame) val=0x%02X, count=%d\r\n", rVal, count);
	if (count==0) UARTprintf(" Disable InfoFrame Error!!!\r\n");
#endif

	return (count)? MDIN_NO_ERROR : MDIN_TIMEOUT_ERROR;
}

static MDIN_ERROR_t MDINHTX_SetInfoFrameAVI(PMDIN_VIDEO_INFO pINFO, BOOL OnOff)
{
	BYTE i, mode, len, crc;	WORD rBuff[7];

	if (OnOff==OFF) return MDIN_NO_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x13e, 0, 1, 0)) return MDIN_I2C_ERROR;
	if (MDINHTX_GetInfoFrameOff(0x02)) return MDIN_I2C_ERROR;

	memset(rBuff, 0, sizeof(rBuff));	// clear infoframe buff

	// colorimetry information
	mode = defMDINHTXVideo[pINFO->stOUT_m.frmt].stMODE.id_1;
	rBuff[0] |= (mode== 4||mode== 5||mode==16||mode==19||mode==20||mode==31||
				 mode==32||mode==33||mode==34)? (2<<14) : (1<<14);
	if (pINFO->stVID_h.mode==HDMI_OUT_RGB444_8) rBuff[0] &= ~(3<<14);

	// picture aspect ratio information
	mode = defMDINHTXVideo[pINFO->stOUT_m.frmt].stMODE.repl;
	rBuff[0] |= (HI4BIT(mode)==2)? (2<<12) : (1<<12);

	// active format aspect ratio information
	rBuff[0] |= (8<<8); 

	// color space information
	if		(pINFO->stVID_h.mode==HDMI_OUT_RGB444_8) rBuff[0] |= (0<<5);
	else if (pINFO->stVID_h.mode==HDMI_OUT_YUV444_8) rBuff[0] |= (2<<5);
	else											 rBuff[0] |= (1<<5);

	// over/under scan information
	mode = defMDINHTXVideo[pINFO->stOUT_m.frmt].stMODE.id_1;
	rBuff[0] |= ((mode>1)&&(mode<60))? 1 : 2;	// VGA start is 60

	// video identification information
//	rBuff[1] |= defMDINHTXVideo[pINFO->stOUT_m.frmt].stMODE.id_1<<8;
	if (pINFO->dacPATH==DAC_PATH_AUX_4CH||pINFO->dacPATH==DAC_PATH_AUX_2HD)	 //for HDMI VIC when 4D1/2HD	//by hungry 2012.08.23
		rBuff[1] |= defMDINHTXVideo[pINFO->stOUT_x.frmt].stMODE.id_1<<8;
	else
		rBuff[1] |= defMDINHTXVideo[pINFO->stOUT_m.frmt].stMODE.id_1<<8;

//	rBuff[1] |= defMDINHTXVideo[pINFO->stOUT_x.frmt].stMODE.id_1<<8;
	// pixel repetition, 28Dec2011
	mode = defMDINHTXVideo[pINFO->stOUT_m.frmt].stMODE.repl;
	rBuff[2] |= LO4BIT(mode);		// 28Dec2011

	len = sizeof(rBuff)-1;	crc = 0x82 + 0x02 + len;	// because 1-byte dummy
	for (i=0; i<sizeof(rBuff); i++) crc += ((PBYTE)rBuff)[i];

	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x140, MAKEWORD(0x02,0x82))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x142, MAKEWORD(0x100-crc,len))) return MDIN_I2C_ERROR;
	if (MDINHIF_MultiWrite(MDIN_HDMI_ID, 0x144, (PBYTE)rBuff, len+1)) return MDIN_I2C_ERROR;

	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x13e, 0, 2, 3)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_EnableInfoFrmAVI(PMDIN_VIDEO_INFO pINFO, BOOL OnOff)
{
	WORD rVal;

	if (MDINHTX_SetInfoFrameAVI(pINFO, OnOff)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x13e, 0, 2, (OnOff)? 3 : 0)) return MDIN_I2C_ERROR;

	if (MDINHIF_RegRead(MDIN_HDMI_ID, 0x13e, &rVal)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_SetInfoFrameAUD(PMDIN_VIDEO_INFO pINFO, BOOL OnOff)
{
	BYTE i, len, crc;	WORD rBuff[5];

	if (OnOff==OFF) return MDIN_NO_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x13e, 4, 1, 0)) return MDIN_I2C_ERROR;
	if (MDINHTX_GetInfoFrameOff(0x20)) return MDIN_I2C_ERROR;

	memset(rBuff, 0, sizeof(rBuff));	// clear infoframe buff

	// audio channel count information
	if (pINFO->stAUD_h.fine&AUDIO_MULTI_CHANNEL) rBuff[0] |= 6;

	len = sizeof(rBuff);	crc = 0x84 + 0x01 + len;
	for (i=0; i<sizeof(rBuff); i++) crc += ((PBYTE)rBuff)[i];

	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x180, MAKEWORD(0x01,0x84))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x182, MAKEWORD(0x100-crc,len))) return MDIN_I2C_ERROR;
	if (MDINHIF_MultiWrite(MDIN_HDMI_ID, 0x184, (PBYTE)rBuff, len+0)) return MDIN_I2C_ERROR;

	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x13e, 4, 2, 3)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_EnableInfoFrmAUD(PMDIN_VIDEO_INFO pINFO, BOOL OnOff)
{
	WORD rVal;

	if (MDINHTX_SetInfoFrameAUD(pINFO, OnOff)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x13e, 4, 2, (OnOff)? 3 : 0)) return MDIN_I2C_ERROR;

	if (MDINHIF_RegRead(MDIN_HDMI_ID, 0x13e, &rVal)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}


//--------------------------------------------------------------------------------------------------------------------------
// Drive Function for ETC (reset, video, audio, etc)
//--------------------------------------------------------------------------------------------------------------------------
static MDIN_ERROR_t MDINHTX_SetModeHDMI(BOOL OnOff)
{
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x12e, 8, 1, MBIT(OnOff,1))) return MDIN_I2C_ERROR;
	GetHDMI = MBIT(OnOff,1);
	return MDIN_NO_ERROR;
}

static BOOL			MDINHTX_IsModeHDMI(void)
{
	return (GetHDMI)? TRUE : FALSE;
}

static MDIN_ERROR_t MDINHTX_Set656Mode(BYTE mode)
{
	WORD rVal = defMDINHTXVideo[mode].stB656.i_adj;

	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x03e, 0, 8, LOBYTE(rVal))) return MDIN_I2C_ERROR;

	rVal = defMDINHTXVideo[mode].stB656.h_syn;
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x040, rVal)) return MDIN_I2C_ERROR;

	rVal = defMDINHTXVideo[mode].stB656.o_fid;
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x042, rVal)) return MDIN_I2C_ERROR;

	rVal = defMDINHTXVideo[mode].stB656.h_len;
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x044, rVal)) return MDIN_I2C_ERROR;

	rVal = defMDINHTXVideo[mode].stB656.v_syn;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x046, 0, 8, LOBYTE(rVal))) return MDIN_I2C_ERROR;

	rVal = defMDINHTXVideo[mode].stB656.v_len;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x046, 8, 8, LOBYTE(rVal))) return MDIN_I2C_ERROR;

	rVal = defMDINHTXVideo[mode].stWIND.x;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x032, 0, 12, rVal)) return MDIN_I2C_ERROR;

	rVal = defMDINHTXVideo[mode].stWIND.y;
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x034, rVal)) return MDIN_I2C_ERROR;

	rVal = defMDINHTXVideo[mode].stWIND.w;
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x036, rVal)) return MDIN_I2C_ERROR;

	rVal = defMDINHTXVideo[mode].stWIND.h;
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x038, rVal)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_EnableAllPWR(PMDIN_VIDEO_INFO pINFO, BOOL OnOff)
{
	PMDIN_HDMICTRL_INFO pCTL = (PMDIN_HDMICTRL_INFO)&pINFO->stCTL_h;

	// set power down total
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x13c, 8, 1, MBIT(OnOff,1))) return MDIN_I2C_ERROR;

	// disable Ri check
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x076, 12, 2, 0)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x076, 15, 1, 0)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x026,  8, 1, 0)) return MDIN_I2C_ERROR;

	// disable KSV ready
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x076, 7, 1, 0)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x026, 9, 1, 0)) return MDIN_I2C_ERROR;

	// disable encryption
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x00e, 8, 1, 0)) return MDIN_I2C_ERROR;

	if (OnOff==OFF) pCTL->proc = HTX_CABLE_PLUG_OUT;
	if (OnOff==OFF) pCTL->auth = HDCP_AUTHEN_BGN;

	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_EnablePhyPWR(PMDIN_VIDEO_INFO pINFO, BOOL OnOff)
{
	PMDIN_HDMICTRL_INFO pCTL = (PMDIN_HDMICTRL_INFO)&pINFO->stCTL_h;

	// disable Ri check
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x076, 12, 2, 0)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x076, 15, 1, 0)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x026,  8, 1, 0)) return MDIN_I2C_ERROR;

	// disable KSV ready
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x076, 7, 1, 0)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x026, 9, 1, 0)) return MDIN_I2C_ERROR;

	// disable encryption
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x00e, 8, 1, 0)) return MDIN_I2C_ERROR;

	// set power down mode
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x008, 0, 1, MBIT(OnOff,1))) return MDIN_I2C_ERROR;

	if (OnOff==OFF) pCTL->auth = HDCP_AUTHEN_BGN;

	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_SoftReset(PMDIN_VIDEO_INFO pINFO)
{
	if (MDINHTX_EnableControlPKT(ON)) return MDIN_I2C_ERROR;
	if (MDINHTX_EnablePhyPWR(pINFO, OFF)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x004, 8, 8, 3)) return MDIN_I2C_ERROR;
	MDINDLY_mSec(1);
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x004, 8, 8, 0)) return MDIN_I2C_ERROR;
	if (MDINHTX_EnablePhyPWR(pINFO, ON)) return MDIN_I2C_ERROR;
	if (MDINHTX_EnableControlPKT(OFF)) return MDIN_I2C_ERROR;
	MDINDLY_mSec(64);          // allow TCLK (sent to Rx across the HDMS link) to stabilize
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_SetVideoMode(PMDIN_VIDEO_INFO pINFO)
{
	WORD mode = 0, acen = 0;
	BYTE outpath = 0;

#if __MDINHTX_DBGPRT__ == 1
	UARTprintf("MDINHTX_SetVideo> pINFO->stOUT_m.frmt: %04x\n", pINFO->stOUT_m.frmt);
#endif

	//	PMDIN_HDMICTRL_INFO pCTL = (PMDIN_HDMICTRL_INFO)&pINFO->stCTL_h;
	PMDIN_OUTVIDEO_ATTB	pOUT_stATTB = (PMDIN_OUTVIDEO_ATTB)&defMDINOutVideo[pINFO->stOUT_m.frmt];	//for HDMI Sync polarity when 4D1/2HD //by hungry 2012.08.23

	if (pINFO->dacPATH==DAC_PATH_AUX_4CH||pINFO->dacPATH==DAC_PATH_AUX_2HD)	// added on 16Aug2012
		 outpath = pINFO->stOUT_x.mode;
	else outpath = pINFO->stOUT_m.mode;

//	outpath = pINFO->stOUT_x.mode;
	
	if		(outpath==MDIN_OUT_RGB444_8) {
		switch (pINFO->stVID_h.mode) {
			case HDMI_OUT_RGB444_8:	mode = 0x00; acen = 0x00; break;	// bypass / bypass
			case HDMI_OUT_YUV444_8: mode = 0x20; acen = 0x06; break;	// dither / RGB2YUV
			case HDMI_OUT_SEP422_8: mode = 0x20; acen = 0x07; break;	// dither / RGB2YUV,422
		}
	}
	else if (outpath==MDIN_OUT_YUV444_8) {
		switch (pINFO->stVID_h.mode) {
			case HDMI_OUT_RGB444_8:	mode = 0x38; acen = 0x00; break;	// dither,YUV2RGB / bypass
			case HDMI_OUT_YUV444_8:	mode = 0x00; acen = 0x00; break;	// bypass / bypass
			case HDMI_OUT_SEP422_8: mode = 0x20; acen = 0x01; break;	// dither / 422
		}
	}
	else if (outpath==MDIN_OUT_MUX656_8) {		// added on 28Dec2011
		switch (pINFO->stVID_h.mode) {
			case HDMI_OUT_RGB444_8:	mode = 0x3e; acen = 0x00; break;	// dither,YUV2RGB,444 / bypass
			case HDMI_OUT_YUV444_8:	mode = 0x06; acen = 0x00; break;	// 444 / bypass
			case HDMI_OUT_SEP422_8: mode = 0x22; acen = 0x00; break;	// dither / bypass
		}
	}
	else 											 {
		switch (pINFO->stVID_h.mode) {
			case HDMI_OUT_RGB444_8:	mode = 0x3c; acen = 0x00; break;	// dither,YUV2RGB,444 / bypass
			case HDMI_OUT_YUV444_8:	mode = 0x04; acen = 0x00; break;	// 444 / bypass
			case HDMI_OUT_SEP422_8: mode = 0x20; acen = 0x00; break;	// dither / bypass
		}
	}

	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x04a, 0, 6, mode)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x048, 8, 3, acen)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x048, 0, 8, 0x00)) return MDIN_I2C_ERROR;
/*
	if (pINFO->dacPATH!=DAC_PATH_AUX_4CH&&pINFO->dacPATH!=DAC_PATH_AUX_2HD) return MDIN_NO_ERROR;

	// vid_mode - syncext for 4-CH input mode, 2-HD input mode
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x04a, 0, 1, 1)) return MDIN_I2C_ERROR;	// fix syncext
*/
	// de_ctrl - de_gen & vs_pol# & hs_pol#
//		mode  = (pINFO->stOUT_m.stATTB.attb&MDIN_SCANTYPE_PROG)?  (0<<2) : (1<<2);
//		mode |= (pINFO->stOUT_m.stATTB.attb&MDIN_POSITIVE_VSYNC)? (0<<1) : (1<<1);
//		mode |= (pINFO->stOUT_m.stATTB.attb&MDIN_POSITIVE_HSYNC)? (0<<0) : (1<<0);
	if (pINFO->dacPATH==DAC_PATH_AUX_4CH||pINFO->dacPATH==DAC_PATH_AUX_2HD){ //for HDMI Sync polarity when 4D1/2HD	//by hungry 2012.08.23
		mode  = (pOUT_stATTB->attb&MDIN_SCANTYPE_PROG)?  (0<<2) : (1<<2);
		mode |= (pOUT_stATTB->attb&MDIN_POSITIVE_VSYNC)? (0<<1) : (1<<1);
		mode |= (pOUT_stATTB->attb&MDIN_POSITIVE_HSYNC)? (0<<0) : (1<<0);
	}
	else {
		mode  = (pINFO->stOUT_m.stATTB.attb&MDIN_SCANTYPE_PROG)?  (0<<2) : (1<<2);
		mode |= (pINFO->stOUT_m.stATTB.attb&MDIN_POSITIVE_VSYNC)? (0<<1) : (1<<1);
		mode |= (pINFO->stOUT_m.stATTB.attb&MDIN_POSITIVE_HSYNC)? (0<<0) : (1<<0);
	}
	//	mode  = (pOUT_stATTB->attb&MDIN_SCANTYPE_PROG)?  (0<<2) : (1<<2);
	//	mode |= (pOUT_stATTB->attb&MDIN_POSITIVE_VSYNC)? (0<<1) : (1<<1);
	//	mode |= (pOUT_stATTB->attb&MDIN_POSITIVE_HSYNC)? (0<<0) : (1<<0);
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x032, 12, 3, mode)) return MDIN_I2C_ERROR;

	// set embedded sync decoding & de generator registers
	if (pINFO->dacPATH==DAC_PATH_AUX_4CH||pINFO->dacPATH==DAC_PATH_AUX_2HD)
		 mode = pINFO->stOUT_x.frmt;
	else mode = pINFO->stOUT_m.frmt;

//	 mode = pINFO->stOUT_x.frmt;
	if (MDINHTX_Set656Mode(mode)) return MDIN_I2C_ERROR;

	// vid_mode - syncext for hdmi-bug
//	mode = (pCTL->proc==HTX_CABLE_HDMI_OUT||pCTL->proc==HTX_CABLE_DVI_OUT)? 1 : 0;
//	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x04a, 0, 1, mode)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x04a, 0, 1, 1)) return MDIN_I2C_ERROR;	// fix syncext
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_SetClockEdge(PMDIN_VIDEO_INFO pINFO)
{
	BOOL OnOff = MBIT(pINFO->stVID_h.fine, HDMI_CLK_EDGE_RISE);

	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x008, 1, 1, MBIT(OnOff,1))) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_SetDeepColor(PMDIN_VIDEO_INFO pINFO)
{
	BOOL OnOff = MBIT(pINFO->stVID_h.fine, HDMI_DEEP_COLOR_ON);

	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x048, 14, 2, (OnOff)? 2 : 1)) return MDIN_I2C_ERROR;

	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x04a,  6, 2, (OnOff)? 2 : 0)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x04a,  5, 1, (OnOff)? 1 : 0)) return MDIN_I2C_ERROR;

	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x12e, 11, 3, (OnOff)? 6 : 4)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x12e, 14, 1, (OnOff)? 1 : 0)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_SetVideoPath(PMDIN_VIDEO_INFO pINFO)
{
	WORD rVal;

	if (MDINHTX_SetVideoMode(pINFO)) return MDIN_I2C_ERROR;
	if (MDINHTX_SetClockEdge(pINFO)) return MDIN_I2C_ERROR;
	if (MDINHTX_SetDeepColor(pINFO)) return MDIN_I2C_ERROR;

	// save packet buffer control registers
	if (MDINHIF_RegRead(MDIN_HDMI_ID, 0x13e, &rVal)) return MDIN_I2C_ERROR;

	// Reset internal state machines and allow TCLK to Rx to stabilize
	if (MDINHTX_SoftReset(pINFO)) return MDIN_I2C_ERROR;

	// Retrieve packet buffer control registers
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x13e, rVal)) return MDIN_I2C_ERROR;

	// set ICLK to not replicated
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x048, 0, 2, 0)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_SetSoftNVAL(PMDIN_HTXAUDIO_INFO pAUD)
{
	WORD rVal; DWORD nVal;

	if (pAUD->frmt==AUDIO_INPUT_SPDIF) {
		if (MDINHIF_RegRead(MDIN_HDMI_ID, 0x118, &rVal)) return MDIN_I2C_ERROR;
		if (MDINHIF_RegField(MDIN_HDMI_ID, 0x114, 0, 8, 3)) return MDIN_I2C_ERROR;
		pAUD->freq &= 0xf0; pAUD->freq |= LO4BIT(rVal);
	}

	switch (LO4BIT(pAUD->freq)) {
		case AUDIO_FREQ_32kHz:	nVal =  4096; break;
		case AUDIO_FREQ_44kHz:	nVal =  6272; break;
		case AUDIO_FREQ_48kHz:	nVal =  6144; break;
		case AUDIO_FREQ_88kHz:	nVal = 12544; break;
		case AUDIO_FREQ_96kHz:	nVal = 12288; break;
		case AUDIO_FREQ_176kHz:	nVal = 25088; break;
		case AUDIO_FREQ_192kHz:	nVal = 24576; break;
		default:				nVal =     0; break;
	}

	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x102, 8, 8, LOBYTE(LOWORD(nVal)))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x104, 0, 8, HIBYTE(LOWORD(nVal)))) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x104, 8, 8, LOBYTE(HIWORD(nVal)))) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_SetAudioInit(PMDIN_VIDEO_INFO pINFO)
{
	PMDIN_HTXAUDIO_INFO pAUD = (PMDIN_HTXAUDIO_INFO)&pINFO->stAUD_h;

	// enable SPDIF
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x114, 0, 8, 3)) return MDIN_I2C_ERROR;

	// set software N_VAL
	if (MDINHTX_SetSoftNVAL(pAUD)) return MDIN_I2C_ERROR;

	// enable N/CTS packet
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x100, 8, 8, 2)) return MDIN_I2C_ERROR;

	// set MCLK
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x102, 0, 8, HI4BIT(pAUD->freq))) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_SetAudioPath(PMDIN_VIDEO_INFO pINFO)
{
	WORD rVal, mode;
	PMDIN_HTXAUDIO_INFO pAUD = (PMDIN_HTXAUDIO_INFO)&pINFO->stAUD_h;

	// disable audio input stream
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x114, 0, 1, OFF)) return MDIN_I2C_ERROR;

	// disable output audio packets
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x00c, 9, 1, ON)) return MDIN_I2C_ERROR;

	// set software N_VAL
	if (MDINHTX_SetSoftNVAL(pAUD)) return MDIN_I2C_ERROR;

	// set software sampling frequency
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x120, 8, 8, LO4BIT(pAUD->freq))) return MDIN_I2C_ERROR;

	if (pAUD->frmt==AUDIO_INPUT_SPDIF) {
		// clear HDRA_ON, SCK_EDGE, CBIT_ORDER, VBIT
		if (MDINHIF_RegField(MDIN_HDMI_ID, 0x11c, 12, 4, 0)) return MDIN_I2C_ERROR;
		if (MDINHIF_RegField(MDIN_HDMI_ID, 0x11e,  0, 8, 0)) return MDIN_I2C_ERROR;

		// clear FS_OVERRIDE
		if (MDINHIF_RegField(MDIN_HDMI_ID, 0x114, 9, 1, 0)) return MDIN_I2C_ERROR;

		MDINDLY_mSec(6);	// allow FIFO to flush

		// enable audio
		if (MDINHIF_RegField(MDIN_HDMI_ID, 0x114, 0, 8, 3)) return MDIN_I2C_ERROR;

		// set LAYOUT_1
		if (MDINHIF_RegField(MDIN_HDMI_ID, 0x12e, 9, 2, 0)) return MDIN_I2C_ERROR;
	}
	else {
		// input I2S sample length
		if (MDINHIF_RegField(MDIN_HDMI_ID, 0x124, 0, 8, HIBYTE(pAUD->fine))) return MDIN_I2C_ERROR;
/*
		// set original sampling frequency
		if (MDINHIF_RegField(MDIN_HDMI_ID, 0x122, 4, 4, LO4BIT(pAUD->freq))) return MDIN_I2C_ERROR;

		// set audio sample word length
		if (MDINHIF_RegField(MDIN_HDMI_ID, 0x122, 0, 4, HIBYTE(pAUD->fine))) return MDIN_I2C_ERROR;
*/
		// set original sampling frequency & audio sample word length
		mode = (1<<12) | MAKEBYTE(LO4BIT(pAUD->freq), HIBYTE(pAUD->fine));
		if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x122, mode)) return MDIN_I2C_ERROR;

		// set I2S data in map register
		if (MDINHIF_RegField(MDIN_HDMI_ID, 0x11c, 0, 2, pAUD->frmt)) return MDIN_I2C_ERROR;

		// set I2S control register
		if (MDINHIF_RegField(MDIN_HDMI_ID, 0x11c, 8, 8, LOBYTE(pAUD->fine))) return MDIN_I2C_ERROR;

		// set FS_OVERRIDE
		if (MDINHIF_RegField(MDIN_HDMI_ID, 0x114, 9, 1, 1)) return MDIN_I2C_ERROR;

		MDINDLY_mSec(6);	// allow FIFO to flush

		// enable audio
		mode = (pAUD->fine&AUDIO_MULTI_CHANNEL)? 15 : (1<<pAUD->frmt);
		if (MDINHIF_RegField(MDIN_HDMI_ID, 0x114, 0, 8, MAKEBYTE(mode,1))) return MDIN_I2C_ERROR;

		// set LAYOUT_1
		if (MDINHIF_RegField(MDIN_HDMI_ID, 0x12e, 9, 2, (mode==15)? 1 : 0)) return MDIN_I2C_ERROR;
	}

	// enable output audio packets
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x00c, 9, 1, OFF)) return MDIN_I2C_ERROR;

	// save packet buffer control registers
	if (MDINHIF_RegRead(MDIN_HDMI_ID, 0x13e, &rVal)) return MDIN_I2C_ERROR;

	// Reset internal state machines and allow TCLK to Rx to stabilize
	if (MDINHTX_SoftReset(pINFO)) return MDIN_I2C_ERROR;

	// Retrieve packet buffer control registers
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x13e, rVal)) return MDIN_I2C_ERROR;

	// set MCLK
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x102, 0, 8, HI4BIT(pAUD->freq))) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
// AudioDelayMS Range : 0 ~ 500(ms) for one audio stream delay
MDIN_ERROR_t MDINHTX_SetAudioDelay(BOOL AudioDelayOn, WORD AudioDelayMS, MDIN_ADELAY_MODE_t AudioDelayMode)
{
	WORD AudioDelayAmount = 0;
	WORD StartRowAddr;

	if(!AudioDelayOn) { // audio delay bypass Enable
		if (MDINHIF_RegField(MDIN_HOST_ID, 0x03e, 15, 1, 1)) return MDIN_I2C_ERROR; // bypass audio delay logic
	}
	else {
 		// disable audio delay(audio pass thru) & disable audio output
		if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x1fb, 0, 2, 0)) return MDIN_I2C_ERROR;
		if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x1fc, 0, 1, 0)) return MDIN_I2C_ERROR;
        
       	// calculate audio delay amount : input is msec unit for each delay mode
		// SCK = 64*fs = 64*48kHz = 3,072kHz, MDIN380: 4kByte/row, MDIN340: 2kByte/row
		switch (AudioDelayMode) {
			// for one audio stream (SD_IN(0) selected)
			case AUDIO_DELAY_MODE_128_ONE:
				if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x1fb, 3, 2, 0)) return MDIN_I2C_ERROR;	// SD_IN(0)
				if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x1fb, 5, 2, 3)) return MDIN_I2C_ERROR;
				if (AudioDelayMS>500)	AudioDelayAmount = 500 * 24;			// 24(=3,072kHz/128)
				else					AudioDelayAmount = AudioDelayMS * 24;
				break;
			// for all audio stream (SD_IN(3:0))
			case AUDIO_DELAY_MODE_32_ALL:
				if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x1fb, 5, 2, 2)) return MDIN_I2C_ERROR;
				if (AudioDelayMS>125)	AudioDelayAmount = 125 * 96;			// 96(=3,072kHz/32)
				else					AudioDelayAmount = AudioDelayMS * 96;
				break;
			// for one audio stream (SD_IN(0) selected)
			case AUDIO_DELAY_MODE_64_ONE:
				if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x1fb, 3, 2, 0)) return MDIN_I2C_ERROR;	// SD_IN(0)
				if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x1fb, 5, 2, 1)) return MDIN_I2C_ERROR;
				if (AudioDelayMS>250)	AudioDelayAmount = 250 * 48;			// 48(=3,072kHz/64)
				else					AudioDelayAmount = AudioDelayMS * 48;
				break;
			// for all audio stream (SD_IN(3:0))
			case AUDIO_DELAY_MODE_16_ALL:
				if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x1fb, 5, 2, 0)) return MDIN_I2C_ERROR;
				if (AudioDelayMS>62)		AudioDelayAmount = 62 * 192;			// 192(=3,072kHz/16)
				else					AudioDelayAmount = AudioDelayMS * 192;
				break;
			default: break;
		}

		// Row Addr Start/End Configuration
   		if (MDINHIF_RegRead(MDIN_LOCAL_ID, 0x1f8, &StartRowAddr)) return MDIN_I2C_ERROR;
		if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x1f8, StartRowAddr)) return MDIN_I2C_ERROR;
		if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x1f9, StartRowAddr+48)) return MDIN_I2C_ERROR; // 500ms = 192kByte = 48row (4kByte/row)
		if (MDINHIF_RegWrite(MDIN_LOCAL_ID, 0x1fa, AudioDelayAmount)) return MDIN_I2C_ERROR; // 0x1fa[15:0] = (0~65535)

		// enable audio delay & enable audio output
		if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x1fb, 0, 2, 3)) return MDIN_I2C_ERROR;
		if (MDINHIF_RegField(MDIN_LOCAL_ID, 0x1fc, 0, 1, 1)) return MDIN_I2C_ERROR;
	 	// enable audio delay
		if (MDINHIF_RegField(MDIN_HOST_ID, 0x03e, 15, 1, 0)) return MDIN_I2C_ERROR;
	}
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_InitModeDVI(PMDIN_VIDEO_INFO pINFO)
{
#if __MDINHTX_DBGPRT__ == 1
	UARTprintf("InitDVITX()!!!\r\n");
#endif

	//	FPLL is 1.0*IDCK.
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x082, 0, 8, 0x20)) return MDIN_I2C_ERROR;
	if (MDINHTX_SetModeHDMI(OFF)) return MDIN_I2C_ERROR;

	// set wake-up
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x008, 0, 1, 1)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x078, 8, 8, 2)) return MDIN_I2C_ERROR;

	if (MDINHTX_SoftReset(pINFO)) return MDIN_I2C_ERROR;

//	if (MDINHTX_Set656Mode(pINFO->stOUT_m.frmt)) return MDIN_I2C_ERROR;

	pINFO->stVID_h.mode = HDMI_OUT_RGB444_8;	// Output must be RGB
	if (MDINHTX_SetVideoPath(pINFO)) return MDIN_I2C_ERROR;
	if (MDINHTX_SetAudioPath(pINFO)) return MDIN_I2C_ERROR;

	// Must be done AFTER setting up audio and video paths and BEFORE starting to send InfoFrames
	if (MDINHTX_SoftReset(pINFO)) return MDIN_I2C_ERROR;

	// disable encryption
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x00c, 9, 2, 0)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x00e, 8, 1, 0)) return MDIN_I2C_ERROR;

	// AVI Mute clear, in DVI this must be used to clear mute
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x1de, 8, 8, 0x10)) return MDIN_I2C_ERROR;

	// Clear packet enable/repeat controls as they will not sent in DVI mode
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x13e, 0x0000)) return MDIN_I2C_ERROR;

	// set DVI interrupt mask
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x070, 0x4000)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x074, 0x4000)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_InitModeHDMI(PMDIN_VIDEO_INFO pINFO)
{
	PMDIN_HDMICTRL_INFO pCTL = (PMDIN_HDMICTRL_INFO)&pINFO->stCTL_h;

#if __MDINHTX_DBGPRT__ == 1
	UARTprintf("InitHDMITX()!!!\r\n");
#endif

//	if (MDINHTX_Set656Mode(pINFO->stOUT_m.frmt)) return MDIN_I2C_ERROR;

	//	FPLL is 1.0*IDCK.
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x082, 0, 8, 0x20)) return MDIN_I2C_ERROR;
	if (MDINHTX_SetModeHDMI(ON)) return MDIN_I2C_ERROR;

	// set wake-up
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x008, 0, 1, 1)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x078, 8, 8, 2)) return MDIN_I2C_ERROR;

	if (MDINHTX_SetAudioInit(pINFO)) return MDIN_I2C_ERROR;
	if (MDINHTX_SetVideoPath(pINFO)) return MDIN_I2C_ERROR;
	if (MDINHTX_SetAudioPath(pINFO)) return MDIN_I2C_ERROR;

	// Must be done AFTER setting up audio and video paths and BEFORE starting to send InfoFrames
	if (MDINHTX_SoftReset(pINFO)) return MDIN_I2C_ERROR;

	// enable AVI info-frame
	if (MDINHTX_EnableInfoFrmAVI(pINFO, ON)) return MDIN_I2C_ERROR;

	// disable encryption
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x00e, 8, 1, OFF)) return MDIN_I2C_ERROR;

	// enable AUD info-frame
	if (MDINHTX_EnableInfoFrmAUD(pINFO, ON)) return MDIN_I2C_ERROR;

	// Enable Interrupts: VSync, Ri check, HotPlug
	// CLR_MASK is BIT_INT_HOT_PLUG|BIT_BIPHASE_ERROR|BIT_DROP_SAMPLE
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x070, 0x5800)) return MDIN_I2C_ERROR;
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x074, 0x5800)) return MDIN_I2C_ERROR;
	
	pCTL->proc = HTX_CABLE_PLUG_OUT;
	pCTL->auth = HDCP_AUTHEN_BGN;

#if SYSTEM_USE_HTX_HDCP == 0
	if (MDINHTX_EnableControlPKT(OFF)) return MDIN_I2C_ERROR; // Mute OFF
#endif

	return MDIN_NO_ERROR;
}

static void MDINHTX_ShowStatusID(PMDIN_HDMICTRL_INFO pCTL)
{
#if __MDINHTX_DBGPRT__ == 1
	if (OldPROC==pCTL->proc) return;

	UARTprintf("{HDMI-Proc}: \n\r");
	switch (pCTL->proc) {
		case HTX_CABLE_PLUG_OUT:	UARTprintf("PLUG Out\n\r");	break;
		case HTX_CABLE_EDID_CHK:	UARTprintf("check EDID\r\n");	break;
		case HTX_CABLE_HDMI_OUT:	UARTprintf("HDMI Out\r\n");	break;
		case HTX_CABLE_DVI_OUT:		UARTprintf("DVI Out\r\n");	break;
		default:					UARTprintf("Unknown\r\n");	break;
	}
	OldPROC = pCTL->proc;
#endif
}

static MDIN_ERROR_t MDINHTX_IRQHandler(PMDIN_HDMICTRL_INFO pCTL)
{
	WORD rVal, rBuff[2];

	if (MDINHIF_RegRead(MDIN_HDMI_ID, 0x070, &rVal)) return MDIN_I2C_ERROR;
	if ((rVal&0x01)==0) return MDIN_NO_ERROR;

	if (MDINHIF_MultiRead(MDIN_HDMI_ID, 0x070, (PBYTE)rBuff, 4)) return MDIN_I2C_ERROR;
	if (MDINHIF_MultiWrite(MDIN_HDMI_ID, 0x070, (PBYTE)rBuff, 4)) return MDIN_I2C_ERROR;

	if (MDINHIF_RegRead(MDIN_HDMI_ID, 0x008, &rVal)) return MDIN_I2C_ERROR;

	// in order not to fail bouncing detection
	if ((rBuff[0]&0x4000)&&(rVal&0x0200)==0) pCTL->proc = HTX_CABLE_PLUG_OUT;

#if __MDINHTX_DBGPRT__ == 1
	if ((rBuff[0]&0x4000)&&(rVal&0x0200)==0)
		UARTprintf("HotPlug interrupt detection!!! = %04X\n\r", rVal&0x0200);
#endif

	// Now clear all other interrupts
	if (MDINHIF_MultiWrite(MDIN_HDMI_ID, 0x070, (PBYTE)rBuff, 4)) return MDIN_I2C_ERROR;

	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_HPDHandler(PMDIN_HDMICTRL_INFO pCTL)
{
	WORD rVal;

	if (MDINHIF_RegRead(MDIN_HDMI_ID, 0x008, &rVal)) return MDIN_I2C_ERROR;

	if (rVal&0x0200) {
		if (pCTL->proc!=HTX_CABLE_PLUG_OUT) return MDIN_NO_ERROR;
		if (++GetPLUG<10) return MDIN_NO_ERROR;
		pCTL->proc = HTX_CABLE_EDID_CHK;
	}
	else {
		GetPLUG = 0; pCTL->proc = HTX_CABLE_PLUG_OUT;
//		CECDisconnect();
	}

	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_PWRHandler(PMDIN_HDMICTRL_INFO pCTL)
{
	switch (pCTL->proc) {
		case HTX_CABLE_EDID_CHK: return MDINHIF_RegField(MDIN_HDMI_ID, 0x13c, 8, 1, 1);
		case HTX_CABLE_PLUG_OUT: return MDINHIF_RegField(MDIN_HDMI_ID, 0x13c, 8, 1, 0);
	}
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_SetAUTOFormat(PMDIN_VIDEO_INFO pINFO)
{
	PMDIN_HDMICTRL_INFO pCTL = (PMDIN_HDMICTRL_INFO)&pINFO->stCTL_h;
	BYTE frmt;

	if ((pINFO->stVID_h.fine&HDMI_USE_AUTO_FRMT)==0) return MDIN_NO_ERROR;
	if ((pCTL->frmt==0x00)||(pCTL->frmt==0xff)) return MDIN_NO_ERROR;

	for (frmt=0; frmt<VIDOUT_FORMAT_END; frmt++) {
		if (pCTL->frmt==defMDINHTXVideo[frmt].stMODE.id_1) break;
		if (pCTL->frmt==defMDINHTXVideo[frmt].stMODE.id_2) break;
	}
	if (frmt==VIDOUT_FORMAT_END) return MDIN_NO_ERROR;

	// check call video process
	if (pINFO->stOUT_m.frmt==frmt)
		 pINFO->exeFLAG &= ~MDIN_UPDATE_HDMIFMT;	// not update
	else pINFO->exeFLAG |=  MDIN_UPDATE_HDMIFMT;	// update

	pINFO->stVID_h.mode = pCTL->mode;			// get native mode
	pINFO->stOUT_m.frmt = frmt;					// get native format
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_SetOutputMode(PMDIN_VIDEO_INFO pINFO)
{
	PMDIN_HDMICTRL_INFO pCTL = (PMDIN_HDMICTRL_INFO)&pINFO->stCTL_h;

	if (pCTL->proc==HTX_CABLE_HDMI_OUT) {
		if (MDINHTX_IsModeHDMI()==TRUE) return MDIN_NO_ERROR;
		if (MDINHTX_InitModeHDMI(pINFO)) return MDIN_I2C_ERROR;
	}

	if (pCTL->proc==HTX_CABLE_DVI_OUT) {
		if (MDINHTX_IsModeHDMI()==FALSE) return MDIN_NO_ERROR;
		if (MDINHTX_InitModeDVI(pINFO)) return MDIN_I2C_ERROR;
	}
	return MDIN_NO_ERROR;
}

static MDIN_ERROR_t MDINHTX_TimeOutHandler(PMDIN_VIDEO_INFO pINFO)
{
	PMDIN_HDMICTRL_INFO pCTL = (PMDIN_HDMICTRL_INFO)&pINFO->stCTL_h;

	if (GetEDID<10) return MDIN_NO_ERROR;
	pCTL->proc = HTX_CABLE_DVI_OUT;

	if (MDINHTX_SetOutputMode(pINFO)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

BYTE cntcnt=0;
//--------------------------------------------------------------------------------------------------------------------------
// Drive Function for HDMI-TX Handler, Video Process
//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINHTX_CtrlHandler(PMDIN_VIDEO_INFO pINFO)
{
	PMDIN_HDMICTRL_INFO pCTL = (PMDIN_HDMICTRL_INFO)&pINFO->stCTL_h;

	if (MDINHTX_IRQHandler(pCTL)) return MDIN_I2C_ERROR;	// IRQ handler

	if (MDINHTX_HPDHandler(pCTL)) return MDIN_I2C_ERROR;	// HPD handler

	if (MDINHTX_PWRHandler(pCTL)) return MDIN_I2C_ERROR;	// PWR handler

	MDINHTX_ShowStatusID(pCTL);	// debug print

	if (pCTL->proc==HTX_CABLE_EDID_CHK) {
		if (MDINHTX_GetParseEDID(pINFO)) return MDIN_I2C_ERROR;
		if (MDINHTX_VideoProcess(pINFO)) return MDIN_I2C_ERROR;
//		if (pCTL->proc!=HTX_CABLE_DVI_OUT) CECConnect();
		if (MDINHTX_SetAUTOFormat(pINFO)) return MDIN_I2C_ERROR;
		if (MDINHTX_SetOutputMode(pINFO)) return MDIN_I2C_ERROR;
	}

	if (MDINHTX_TimeOutHandler(pINFO)) return MDIN_I2C_ERROR;

//	if ((pCTL->proc!=HTX_CABLE_DVI_OUT)&&(pCTL->proc!=HTX_CABLE_PLUG_OUT))
//		CECHandler();

	return MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINHTX_VideoProcess(PMDIN_VIDEO_INFO pINFO)
{
	if (pINFO->stCTL_h.proc!=HTX_CABLE_HDMI_OUT&&
		pINFO->stCTL_h.proc!=HTX_CABLE_DVI_OUT) return MDIN_NO_ERROR;

	if (MDINHTX_SetVideoPath(pINFO)) return MDIN_I2C_ERROR;
	if (pINFO->stCTL_h.proc!=HTX_CABLE_HDMI_OUT) return MDIN_NO_ERROR;

	if (MDINHTX_SoftReset(pINFO)) return MDIN_I2C_ERROR;
	if (MDINHTX_EnableInfoFrmAVI(pINFO, ON)) return MDIN_I2C_ERROR;
	if (MDINHTX_EnableInfoFrmAUD(pINFO, ON)) return MDIN_I2C_ERROR;
	return MDIN_NO_ERROR;
}

MDIN_ERROR_t MDINHTX_SetHDMIBlock(PMDIN_VIDEO_INFO pINFO)
{
	if (MDINHTX_EnablePhyPWR(pINFO, ON)) return MDIN_I2C_ERROR;	// PhyPowerOn
	if (MDINHTX_EnableAllPWR(pINFO, ON)) return MDIN_I2C_ERROR;	// PowerOn

	// initialize MDDC
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x0f2, 0x0f00)) return MDIN_I2C_ERROR;	// ABORT
	MDINDLY_mSec(1);
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x0f2, 0x0900)) return MDIN_I2C_ERROR;	// CLEAR FIFO
	MDINDLY_mSec(1);
	if (MDINHIF_RegWrite(MDIN_HDMI_ID, 0x0f2, 0x0a00)) return MDIN_I2C_ERROR;	// CLOCK
	MDINDLY_mSec(1);
	if (MDINHIF_RegField(MDIN_HDMI_ID, 0x026, 8, 8, 0)) return MDIN_I2C_ERROR;	// disable Ri

//	if (MDINHTX_InitModeDVI(pINFO)) return MDIN_I2C_ERROR;
	if (MDINHTX_InitModeHDMI(pINFO)) return MDIN_I2C_ERROR;

//	CECInitialize();
	return MDIN_NO_ERROR;
}

