/**
 *	@file   i2c.c
 *	@brief	
 *	@author luisfynn <tani223@pinetron.com>
 *	@date   2014/10/14 13:08
 */

/* system include */
#include <stdio.h>
/* local include */
#include "stm32f10x.h"
#include "i2c.h"
#include "mutex.h"

MUTEX_DECLARE(s_sem_i2c1);
MUTEX_DECLARE(s_sem_i2c2);

#ifndef I2C_USE_HW
static void I2C1_Lock(void)     { MUTEX_LOCK(s_sem_i2c1);   } 
static void I2C1_Unlock(void)   { MUTEX_UNLOCK(s_sem_i2c1); }
static void I2C2_Lock(void)     { MUTEX_LOCK(s_sem_i2c2);   } 
static void I2C2_Unlock(void)   { MUTEX_UNLOCK(s_sem_i2c2); }

#define I2C1_SCL	GPIO_Pin_6
#define I2C1_SDA	GPIO_Pin_7

#define I2C2_SCL	GPIO_Pin_10
#define I2C2_SDA	GPIO_Pin_11

#define I2C_ACK		0
#define I2C_NACK	1	

#ifndef HIBYTE
#define	HIBYTE(a)	((unsigned char)((a)>>8))
#endif

#ifndef LOBYTE
#define	LOBYTE(a)	((unsigned char)((a)&0xff))
#endif

#define SDA_HIGH	1
#define SDA_LOW		0

static void I2C1_Start(void);
static void I2C1_Stop(void);
static void I2C1_WriteByte(unsigned char data);
I2C_Status I2C1_SlaveAck(unsigned char flag);
static unsigned char I2C1_ReadByte(void);

static void I2C2_Start(void);
static void I2C2_Stop(void);
static void I2C2_WriteByte(unsigned char data);
I2C_Status I2C2_SlaveAck(unsigned char flag);
static unsigned short I2C2_ReadByte(void);

void I2CDLY_5uSec(unsigned short delay)
{
	Delay5US(delay);
}

void I2C_Initialize(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* GPIOB Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* Configure I2C1 pins: SCL and SDA ----------------------------------------*/
	GPIO_InitStructure.GPIO_Pin =  I2C1_SCL | I2C1_SDA;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, I2C1_SDA);
	GPIO_SetBits(GPIOB, I2C1_SCL);

	/* Configure I2C2 pins: SCL and SDA ----------------------------------------*/
	GPIO_InitStructure.GPIO_Pin = I2C2_SCL | I2C2_SDA;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, I2C2_SDA);
	GPIO_SetBits(GPIOB, I2C2_SCL);

	MUTEX_INIT(s_sem_i2c1);
	MUTEX_INIT(s_sem_i2c2);
}

unsigned char I2C1_ByteWrite(unsigned char DevID, unsigned char WriteReg, unsigned char pBuff)
{
	unsigned char ret = I2C_FAIL;
	unsigned char i;

	I2C1_Lock();

	do {
		I2C1_Start();
		I2C1_WriteByte(DevID);   //device address
		if(I2C1_SlaveAck(I2C_ACK) != I2C_OK) break;

		I2C1_WriteByte(WriteReg);   //register address
		if(I2C1_SlaveAck(I2C_ACK) != I2C_OK) break;

		I2C1_WriteByte(pBuff);

		if(I2C1_SlaveAck(I2C_ACK) != I2C_OK) break;

		I2C1_Stop();

		ret = I2C_OK;
	} while(0);

	I2C1_Unlock();

	return ret; 
}

unsigned char I2C1_ByteRead(unsigned char DevID, unsigned char Register, unsigned char* pBuff)
{
	unsigned char ret = I2C_FAIL;

	I2C1_Lock();

	do {
		I2C1_Start();
		I2C1_WriteByte(DevID);
		if(I2C1_SlaveAck(I2C_ACK) != I2C_OK) break;

		I2C1_WriteByte(Register);
		if(I2C1_SlaveAck(I2C_ACK) != I2C_OK) break;

		I2C1_Start();
		I2C1_WriteByte(DevID | 0x01);
		if(I2C1_SlaveAck(I2C_ACK) != I2C_OK) break;

		pBuff[0] = I2C1_ReadByte();
		if(I2C1_SlaveAck(I2C_NACK) != I2C_OK) break;

		I2C1_Stop();

		ret = I2C_OK;
	} while(0);

	I2C1_Unlock();

	return ret;
}

unsigned char I2C2_WordRead(unsigned short DevID, unsigned short Register, unsigned char* pBuff ,unsigned short bytes)
{
	unsigned char ret = I2C_FAIL;
	unsigned char i;

	I2C2_Lock();

	do {
		I2C2_Start();
		I2C2_WriteByte(DevID);
		if(I2C2_SlaveAck(I2C_ACK) != I2C_OK) break;

		I2C2_WriteByte(HIBYTE(Register));
		if(I2C2_SlaveAck(I2C_ACK) != I2C_OK) break;
		I2C2_WriteByte(LOBYTE(Register));
		if(I2C2_SlaveAck(I2C_ACK) != I2C_OK) break;

		I2C2_Start();
		I2C2_WriteByte(DevID | 0x01);
		if(I2C2_SlaveAck(I2C_ACK) != I2C_OK) break;

		for (i=0; i<bytes-1; i++)
		{
			pBuff[i^1] = I2C2_ReadByte();

			/*ACK From Master(MCU) to MDIN-3xx */
			GPIO_ResetBits(GPIOB, I2C2_SDA);
			GPIO_SetBits(GPIOB, I2C2_SCL);
			I2CDLY_5uSec(1);
			GPIO_ResetBits(GPIOB, I2C2_SCL);
			/*End of ACK*/
		}
		pBuff[i^1] = I2C2_ReadByte(); 
		if(I2C2_SlaveAck(I2C_NACK) != I2C_OK) break;
		I2C2_Stop();

		ret = I2C_OK;
	} while(0);

	I2C2_Unlock();

	return ret;
}

unsigned char I2C2_WordWrite(unsigned short DevID, unsigned short Register, unsigned char* pBuff, unsigned short bytes)
{
	unsigned char ret = I2C_FAIL;
	unsigned short i;

	I2C2_Lock();

	do {
		I2C2_Start();

		I2C2_WriteByte(DevID);               //slave addres
		if(I2C2_SlaveAck(I2C_ACK) != I2C_OK) break;

		I2C2_WriteByte(HIBYTE(Register));     //start address
		if(I2C2_SlaveAck(I2C_ACK) != I2C_OK) break;
		I2C2_WriteByte(LOBYTE(Register));     //start address
		if(I2C2_SlaveAck(I2C_ACK) != I2C_OK) break;

		for (i=0; i<bytes-1; i++) {
			I2C2_WriteByte(pBuff[i^1]);
			if(I2C2_SlaveAck(I2C_ACK) != I2C_OK) break;
		}
		if(i != bytes-1) break; /* if above for loop is broken */

		I2C2_WriteByte(pBuff[i^1]);
		if(I2C2_SlaveAck(I2C_ACK) != I2C_OK) break;

		I2C2_Stop();

		ret = I2C_OK;
	} while(0);

	I2C2_Unlock();

	return ret;
}

//------------------GPIO I2C-----------------
static void I2C2_Start(void)
{
	GPIO_SetBits(GPIOB, I2C2_SDA);
	GPIO_SetBits(GPIOB, I2C2_SCL);
	I2CDLY_5uSec(1);
	GPIO_ResetBits(GPIOB, I2C2_SDA);
	I2CDLY_5uSec(1);
	GPIO_ResetBits(GPIOB, I2C2_SCL);
}
static void I2C1_Start(void)
{
	GPIO_SetBits(GPIOB, I2C1_SDA);
	GPIO_SetBits(GPIOB, I2C1_SCL);
	I2CDLY_5uSec(1);
	GPIO_ResetBits(GPIOB, I2C1_SDA);
	I2CDLY_5uSec(1);
	GPIO_ResetBits(GPIOB, I2C1_SCL);
}

static void I2C2_Stop(void)
{
	GPIO_ResetBits(GPIOB, I2C2_SDA);
	I2CDLY_5uSec(1);
	GPIO_SetBits(GPIOB, I2C2_SCL);
	I2CDLY_5uSec(1);
	GPIO_SetBits(GPIOB, I2C2_SDA);
}
static void I2C1_Stop(void)
{
	GPIO_ResetBits(GPIOB, I2C1_SDA);
	I2CDLY_5uSec(1);
	GPIO_SetBits(GPIOB, I2C1_SCL);
	I2CDLY_5uSec(1);
	GPIO_SetBits(GPIOB, I2C1_SDA);
}

static void I2C2_WriteByte(unsigned char data)
{
	unsigned short i, tmpbyte = data;

	for(i = 0; i < 8; i++)
	{       
		if(0x80 & tmpbyte)
			GPIO_SetBits(GPIOB, I2C2_SDA);
		else
			GPIO_ResetBits(GPIOB, I2C2_SDA);

		I2CDLY_5uSec(1);
		GPIO_SetBits(GPIOB, I2C2_SCL);
		I2CDLY_5uSec(1);
		GPIO_ResetBits(GPIOB, I2C2_SCL);
		tmpbyte = tmpbyte << 1;
	}
}

static void I2C1_WriteByte(unsigned char data)
{
	unsigned short i, tmpbyte = data;

	for(i = 0; i < 8; i++)
	{       
		if(0x80 & tmpbyte)
			GPIO_SetBits(GPIOB, I2C1_SDA);
		else
			GPIO_ResetBits(GPIOB, I2C1_SDA);

		I2CDLY_5uSec(1);
		GPIO_SetBits(GPIOB, I2C1_SCL);
		I2CDLY_5uSec(1);
		GPIO_ResetBits(GPIOB, I2C1_SCL);
		tmpbyte = tmpbyte << 1;
	}
}

I2C_Status I2C2_SlaveAck(unsigned char flag)
{
	unsigned char ret = I2C_Error;

	GPIO_InitTypeDef GPIO_InitStruct;

	if(flag == I2C_ACK){
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStruct.GPIO_Pin  = I2C2_SDA;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStruct);

		GPIO_SetBits(GPIOB, I2C2_SCL);
		I2CDLY_5uSec(1);

		while(GPIO_ReadInputDataBit(GPIOB, I2C2_SDA) != I2C_ACK)
		{
		}

		I2CDLY_5uSec(1);
		GPIO_ResetBits(GPIOB, I2C2_SCL);
		ret = I2C_Success;
	}else if(flag == I2C_NACK){
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStruct.GPIO_Pin  = I2C2_SDA;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStruct);

		GPIO_SetBits(GPIOB, I2C2_SCL);
		I2CDLY_5uSec(1);

		while(GPIO_ReadInputDataBit(GPIOB, I2C2_SDA) != I2C_NACK)
		{
		}

		I2CDLY_5uSec(1);
		GPIO_ResetBits(GPIOB, I2C2_SCL);
		ret = I2C_Success;
	}else{
	}

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Pin  = I2C2_SDA;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	return ret;
}

I2C_Status I2C1_SlaveAck(unsigned char flag)
{
	unsigned char ret = I2C_Error;
#if 0
	if(!isHDMI_IN){

		GPIO_InitTypeDef GPIO_InitStruct;

		if(flag == I2C_ACK){
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStruct.GPIO_Pin  = I2C1_SDA;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIO_InitStruct);

			GPIO_SetBits(GPIOB, I2C1_SCL);
			I2CDLY_5uSec(1);

			while(GPIO_ReadInputDataBit(GPIOB, I2C1_SDA) != I2C_ACK)
			{
			}

			I2CDLY_5uSec(1);
			GPIO_ResetBits(GPIOB, I2C1_SCL);
			ret = I2C_Success;
		}else{
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStruct.GPIO_Pin  = I2C1_SDA;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIO_InitStruct);

			GPIO_SetBits(GPIOB, I2C1_SCL);
			I2CDLY_5uSec(1);

			while(GPIO_ReadInputDataBit(GPIOB, I2C1_SDA) != I2C_NACK)
			{
			}

			I2CDLY_5uSec(1);
			GPIO_ResetBits(GPIOB, I2C1_SCL);
			ret = I2C_Success;
		}

		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
		GPIO_InitStruct.GPIO_Pin  = I2C1_SDA;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStruct);
	}else{
		if(flag ==I2C_ACK)
			GPIO_ResetBits(GPIOB, I2C1_SDA);
		else
			GPIO_SetBits(GPIOB, I2C1_SDA);

		I2CDLY_5uSec(1);
		GPIO_SetBits(GPIOB, I2C1_SCL);
		I2CDLY_5uSec(1);
		GPIO_ResetBits(GPIOB, I2C1_SCL);
		ret = I2C_Success;
	}
#endif
	if(flag ==I2C_ACK)
		GPIO_ResetBits(GPIOB, I2C1_SDA);
	else
		GPIO_SetBits(GPIOB, I2C1_SDA);

	I2CDLY_5uSec(1);
	GPIO_SetBits(GPIOB, I2C1_SCL);
	I2CDLY_5uSec(1);
	GPIO_ResetBits(GPIOB, I2C1_SCL);
	ret = I2C_Success;

	return ret;
}

static unsigned short I2C2_ReadByte(void)
{
	unsigned char i, bit;
	unsigned char ReadValue = 0;

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin  = I2C2_SDA;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	for(i = 0; i < 8; i++)
	{
		if(SDA_HIGH == GPIO_ReadInputDataBit(GPIOB, I2C2_SDA))	bit = 0x01;
		else    												bit = 0x00;

		I2CDLY_5uSec(1);			
		GPIO_SetBits(GPIOB, I2C2_SCL);
		I2CDLY_5uSec(1);	
		GPIO_ResetBits(GPIOB, I2C2_SCL);		
		I2CDLY_5uSec(1);
		ReadValue = (ReadValue<<1)|bit;
	}

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Pin  = I2C2_SDA|I2C2_SCL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	return ReadValue;
}

static unsigned char I2C1_ReadByte(void)
{
	unsigned char i, bit, ReadValue = 0;
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin  = I2C1_SDA;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	for(i = 0; i < 8; i++)
	{
		if(SDA_HIGH == GPIO_ReadInputDataBit(GPIOB, I2C1_SDA))	bit = 0x01;
		else													bit = 0x00;

		I2CDLY_5uSec(1);
		GPIO_SetBits(GPIOB, I2C1_SCL);
		I2CDLY_5uSec(1);		
		GPIO_ResetBits(GPIOB, I2C1_SCL);		
		I2CDLY_5uSec(1);
		ReadValue = (ReadValue<<1)|bit;
	}

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Pin = I2C1_SDA | I2C1_SCL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	return ReadValue;
}

#else

static void I2C_Lock(I2C_TypeDef* I2Cx)
{
	if(I2Cx == I2C1) {
		MUTEX_LOCK(s_sem_i2c1);
	} else if(I2Cx == I2C2) {
		MUTEX_LOCK(s_sem_i2c2);
	}
}

static void I2C_Unlock(I2C_TypeDef* I2Cx)
{
	if(I2Cx == I2C1) {
		MUTEX_UNLOCK(s_sem_i2c1);
	} else if(I2Cx == I2C2) {
		MUTEX_UNLOCK(s_sem_i2c2);
	}
}

#define Timed(x) Timeout = 0xFFFF; while (x) { if (Timeout-- == 0) goto errReturn;}

void I2C_Initialize(void)
{
	GPIO_InitTypeDef gpio_def;

	/* GPIOB Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* Configure I2C1 pins: SCL and SDA ----------------------------------------*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	gpio_def.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
	gpio_def.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_def.GPIO_Mode  = GPIO_Mode_AF_OD;
	GPIO_Init(GPIOB, &gpio_def);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);

	/* Configure I2C2 pins: SCL and SDA ----------------------------------------*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	gpio_def.GPIO_Pin   = GPIO_Pin_10 | GPIO_Pin_11;
	gpio_def.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_def.GPIO_Mode  = GPIO_Mode_AF_OD;
	GPIO_Init(GPIOB, &gpio_def);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, DISABLE);

	I2C_InitTypeDef i2c_def;
	I2C_StructInit(&i2c_def);
	i2c_def.I2C_Mode = I2C_Mode_I2C;
	i2c_def.I2C_DutyCycle = I2C_DutyCycle_2;
	i2c_def.I2C_OwnAddress1 = 0x00;
	i2c_def.I2C_Ack = I2C_Ack_Enable;
	i2c_def.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	i2c_def.I2C_ClockSpeed = 100000;

	I2C_Init(I2C1, &i2c_def);
	I2C_Init(I2C2, &i2c_def);

	I2C_Cmd(I2C1, ENABLE);
	I2C_Cmd(I2C2, ENABLE);

	MUTEX_INIT(s_sem_i2c1);
	MUTEX_INIT(s_sem_i2c2);
}

I2C_Status I2C_Read(I2C_TypeDef* I2Cx, uint8_t dev_addr, uint8_t *data_buf, uint32_t data_len)
{
	__IO uint32_t Timeout = 0;

	//    I2Cx->CR2 |= I2C_IT_ERR;  interrupts for errors 

	if (!data_len)
		return I2C_Success;

	I2C_Lock(I2Cx);

	// Wait for idle I2C interface
	Timed(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

	// Enable Acknowledgement, clear POS flag
	I2C_AcknowledgeConfig(I2Cx, ENABLE);
	I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Current);

	// Intiate Start Sequence (wait for EV5
	I2C_GenerateSTART(I2Cx, ENABLE);
	Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

	// Send Address
	I2C_Send7bitAddress(I2Cx, dev_addr, I2C_Direction_Receiver);

	// EV6
	Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_ADDR));

	if (data_len == 1)
	{
		// Clear Ack bit      
		I2C_AcknowledgeConfig(I2Cx, DISABLE);       

		// EV6_1 -- must be atomic -- Clear ADDR, generate STOP
		__disable_irq();
		(void) I2Cx->SR2;                           
		I2C_GenerateSTOP(I2Cx,ENABLE);      
		__enable_irq();

		// Receive data   EV7
		Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE));
		*data_buf++ = I2C_ReceiveData(I2Cx);

	}
	else if (data_len == 2)
	{
		// Set POS flag
		I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Next);

		// EV6_1 -- must be atomic and in this order
		__disable_irq();
		(void) I2Cx->SR2;                           // Clear ADDR flag
		I2C_AcknowledgeConfig(I2Cx, DISABLE);       // Clear Ack bit
		__enable_irq();

		// EV7_3  -- Wait for BTF, program stop, read data twice
		Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF));

		__disable_irq();
		I2C_GenerateSTOP(I2Cx,ENABLE);
		*data_buf++ = I2Cx->DR;
		__enable_irq();

		*data_buf++ = I2Cx->DR;

	}
	else 
	{
		(void) I2Cx->SR2;                           // Clear ADDR flag
		while (data_len-- != 3)
		{
			// EV7 -- cannot guarantee 1 transfer completion time, wait for BTF 
			//        instead of RXNE
			Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF)); 
			*data_buf++ = I2C_ReceiveData(I2Cx);
		}

		Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF));  

		// EV7_2 -- Figure 1 has an error, doesn't read N-2 !
		I2C_AcknowledgeConfig(I2Cx, DISABLE);           // clear ack bit

		__disable_irq();
		*data_buf++ = I2C_ReceiveData(I2Cx);             // receive byte N-2
		I2C_GenerateSTOP(I2Cx,ENABLE);                  // program stop
		__enable_irq();

		*data_buf++ = I2C_ReceiveData(I2Cx);             // receive byte N-1

		// wait for byte N
		Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)); 
		*data_buf++ = I2C_ReceiveData(I2Cx);

		data_len = 0;

	}

	// Wait for stop
	Timed(I2C_GetFlagStatus(I2Cx, I2C_FLAG_STOPF));

	I2C_Unlock(I2Cx);
	return I2C_Success;

errReturn:
	// Any cleanup here
	I2C_ClearFlag(I2Cx, I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_AF | I2C_FLAG_TIMEOUT);
	I2C_GenerateSTOP(I2Cx, ENABLE);

	I2C_Unlock(I2Cx);
	return I2C_Error;
}

/*
 * Read buffer of bytes -- AN2824 Figure 3
 */
I2C_Status I2C_Write(I2C_TypeDef* I2Cx, uint8_t dev_addr, uint32_t reg_addr, uint32_t reg_addr_len, const uint8_t* data_buf, uint32_t data_len)
{
	__IO uint32_t Timeout = 0;

	I2C_Lock(I2Cx);

	/* Enable Error IT (used in all modes: DMA, Polling and Interrupts */
	//    I2Cx->CR2 |= I2C_IT_ERR;

	Timed(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

	// Intiate Start Sequence
	I2C_GenerateSTART(I2Cx, ENABLE);
	Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

	// Send Address  EV5
	I2C_Send7bitAddress(I2Cx, dev_addr, I2C_Direction_Transmitter);
	Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	// EV6
	if(reg_addr_len > 0) {
		if(reg_addr_len >= 4) { I2C_SendData(I2Cx, (reg_addr >> 24) & 0xff); Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF)); }
		if(reg_addr_len >= 3) { I2C_SendData(I2Cx, (reg_addr >> 16) & 0xff); Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF)); }
		if(reg_addr_len >= 2) { I2C_SendData(I2Cx, (reg_addr >>  8) & 0xff); Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF)); }
		if(reg_addr_len >= 1) { I2C_SendData(I2Cx, (reg_addr >>  0) & 0xff); Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF)); }
	}

	if(data_buf && data_len > 0) {
		// Write first byte EV8_1
		I2C_SendData(I2Cx, *data_buf++);

		while (--data_len) {
			// wait on BTF
			Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF));  
			I2C_SendData(I2Cx, *data_buf++);
		}
	}

	Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF));  
	I2C_GenerateSTOP(I2Cx, ENABLE);
	Timed(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF));

	I2C_Unlock(I2Cx);
	return I2C_Success;

errReturn:
	I2C_ClearFlag(I2Cx, I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_AF | I2C_FLAG_TIMEOUT);
	I2C_GenerateSTOP(I2Cx, ENABLE);

	I2C_Unlock(I2Cx);
	return I2C_Error;
}

unsigned char I2C1_ByteWrite(unsigned char dev_addr, unsigned char reg_addr, unsigned char reg_data)
{
	if(I2C_Success != I2C_Write(I2C1, dev_addr, reg_addr, 1, &reg_data, 1)) return I2C_FAIL;
	return I2C_OK;
}

unsigned char I2C1_ByteRead(unsigned char dev_addr, unsigned char reg_addr, unsigned char* p_reg_data)
{
	if(I2C_Success != I2C_Write(I2C1, dev_addr, reg_addr, 1, NULL, 0)) return I2C_FAIL;
	if(I2C_Success != I2C_Read(I2C1, dev_addr, p_reg_data, 1)) return I2C_FAIL;
	return I2C_OK;
}

#endif
