/**
 *	@file   i2c.h
 *	@brief	
 *	@author luisfynn <tani223@pinetron.com>
 *	@date   2014/10/14 13:17
 */

#ifndef _I2C_HEADER_
#define _I2C_HEADER_

/* system include */
/* local include */

#define I2C_OK		1
#define I2C_FAIL	0

void I2C_Initialize(void);

unsigned char I2C1_ByteRead(unsigned char DevID, unsigned char Register, unsigned char* pBuff);
unsigned char I2C1_ByteWrite(unsigned char DevID, unsigned char WriteReg, unsigned char pBuff);

typedef enum {I2C_Error = 0, I2C_Success = !I2C_Error } I2C_Status;

#ifndef I2C_USE_HW
unsigned char I2C2_WordRead(unsigned short DevID, unsigned short Register, unsigned char* pBuff, unsigned short bytes);
unsigned char I2C2_WordWrite(unsigned short DevID, unsigned short Register, unsigned char*, unsigned short bytes);
#else
#include "stm32f10x_i2c.h"
I2C_Status I2C_Read(I2C_TypeDef* I2Cx, uint8_t dev_addr, uint8_t* data_buf, uint32_t nbuf);
I2C_Status I2C_Write(I2C_TypeDef* I2Cx, uint8_t dev_addr, uint32_t reg_addr, uint32_t reg_addr_len, const uint8_t* data_buf,  uint32_t data_len);
#endif

#endif /* _I2C_HEADER_*/

