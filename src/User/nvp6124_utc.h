/**
 *	@file   nvp6124_utc.h
 *	@brief	
 *	@author luisfynn<tani223@pinetron.com>
 *	@date   2015/08/27 16:00
 */

#ifndef _NVP6124_UTC_HEADER_
#define _NVP6124_UTC_HEADER_

/* system include */

/* local include */
#include "video.h"

#define PELCO_CMD_RESET			0
#define PELCO_CMD_SET			1
#define PELCO_CMD_UP			2
#define PELCO_CMD_DOWN			3
#define PELCO_CMD_LEFT			4
#define PELCO_CMD_RIGHT			5
#define PELCO_CMD_OSD			6
#define PELCO_CMD_IRIS_OPEN		7
#define PELCO_CMD_IRIS_CLOSE	8
#define PELCO_CMD_FOCUS_NEAR	9
#define PELCO_CMD_FOCUS_FAR		10
#define PELCO_CMD_ZOOM_WIDE		11
#define PELCO_CMD_ZOOM_TELE		12
#define PELCO_CMD_SCAN_SR		13
#define PELCO_CMD_SCAN_ST		14
#define PELCO_CMD_PRESET1		15
#define PELCO_CMD_PRESET2		16
#define PELCO_CMD_PRESET3		17
#define PELCO_CMD_PTN1_SR		18
#define PELCO_CMD_PTN1_ST		19
#define PELCO_CMD_PTN2_SR		20
#define PELCO_CMD_PTN2_ST		21
#define PELCO_CMD_PTN3_SR		22
#define PELCO_CMD_PTN3_ST		23
#define PELCO_CMD_RUN			24

#define SET_ALL_CH	0xff

#define AHD2_PEL_D0		0x20
#define AHD2_FHD_D0		0x10
#define AHD2_PEL_OUT	0x0C
#define AHD2_PEL_BAUD	0x02
#define AHD2_PEL_LINE	0x07
#define PACKET_MODE		0x0B
#define AHD2_PEL_SYNC	0x0D
#define AHD2_PEL_SHOT	0X0F
#define AHD2_PEL_EVEN	0x2F
#define AHD2_FHD_BAUD	0x00
#define AHD2_FHD_LINE	0x03
#define AHD2_FHD_LINES	0x05
#define AHD2_FHD_BYTE	0x0A
#define AHD2_FHD_MODE	0x0B
#define AHD2_FHD_OUT	0x09
#define ACP_CLR			0x3A

#define ACP_CAM_STAT	0x55
#define ACP_REG_WR		0x60
#define ACP_REG_RD		0x61
#define ACP_MODE_ID		0x60

#define ACP_RX_D0		0x78

#define DRIVER_SW_VER_MAJOR     0x02
#define DRIVER_SW_VER_MINOR     0x00

void nvp6124_acp_rx_clear(void);
void nvp6124_acp_each_setting(AHD_INFO *pvInfo);
unsigned char nvp6124_pelco_command(AHD_INFO *pvInfo, unsigned char command );
unsigned char nvp6124_read_acp_status(void);
unsigned char nvp6124_read_acp_pattern(void);
/* external variable & function */

#endif /* _NVP6124_UTC_HEADER_*/

