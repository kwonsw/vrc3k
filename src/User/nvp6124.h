/**
 *	@file   nvp6124.h
 *	@brief	
 *	@author luisfynn <tani223@pinetron.com>
 *	@date   2015/05/08 15:53
 */

#ifndef _NVP6124_HEADER_
#define _NVP6124_HEADER_

/* system include */

/* local include */
#include "video.h"

/* external variable & function */

#define AHDRX_OUT_960X480		0x1
#define AHDRX_OUT_960X576		0x2
#define AHDRX_OUT_1280X720P30	0x4
#define AHDRX_OUT_1280X720P25	0x8	
#define AHDRX_OUT_1920X1080P30	0x40
#define AHDRX_OUT_1920X1080P25	0x80

unsigned char nvp6124_video_detect(void);
unsigned char nvp6124_init(AHD_INFO *pvInfo);
unsigned char nvp6124_vmode_set(AHD_INFO *pvInfo);
//void nvp6124_common_init(AHD_INFO *pvInfo );

void nvp6124_i2c_write(unsigned char reg_addr, unsigned char reg_data);
void nvp6124_i2c_write_buf(unsigned char reg_addr, unsigned char *buf, unsigned char len);
unsigned char nvp6124_i2c_read(unsigned char reg_addr, unsigned char* p_reg_data);

#endif /* _NVP6124_HEADER_*/

