/**
 *	@file   nvp6124_utc.c
 *	@brief	
 *	@author luisfynn <tani223@pinetron.com>
 *	@date   2015/08/27 16:00
 */

/* system include */

/* local include */
#include "nvp6124.h"
#include "nvp6124_utc.h"
#include "video.h"

void nvp6124_acp_each_setting(AHD_INFO *pvInfo);
unsigned char nvp6124_pelco_command(AHD_INFO *pvInfo, unsigned char command );

static unsigned char nvp6124_pelco_coax_mode(AHD_INFO *pvInfo);
static void nvp6124_pelco_shot(AHD_INFO *pvInfo);
//static unsigned char nvp6124_read_acp_status(void);
//static unsigned char nvp6124_read_acp_pattern(void);
static void nvp6124_init_acp_status(void);
static void nvp6124_init_acp_reg_wr(void);
static void nvp6124_init_acp_reg_rd(void);
static void nvp6124_set_acp_reg_wr(unsigned char bank, unsigned char addr, unsigned char data);
static void nvp6124_get_acp_reg_rd(unsigned char bank, unsigned char addr);
static void nvp6124_acp_read(AHD_INFO *pvInfo);
static void nvp6124_acp_sw_ver_transer(void);
static void nvp6124_acp_isp_write(unsigned int reg_addr, unsigned char reg_data);


void nvp6124_acp_rx_clear(void)
{
	//init acp
	nvp6124_i2c_write(0xff, BANK1);
	nvp6124_i2c_write(0xbd, 0xD0);		//set coax_1 pin to output signals 
	nvp6124_i2c_write(0xbe, 0xDD);		//set coax_2,3 pin to output signals 
	nvp6124_i2c_write(0xbf, 0x0D);		//set coax_4 pin to output signals 

	nvp6124_i2c_write( 0xFF, BANK3 );
	nvp6124_i2c_write( ACP_CLR, 0x01);
	
	nvp6124_i2c_write( 0xFF, BANK3 );
	nvp6124_i2c_write( ACP_CLR, 0x00);
}

void nvp6124_acp_each_setting(AHD_INFO *pvInfo)
{
	nvp6124_i2c_write(0xff, BANK3);

	if(pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25)			//720p25, 30
	{
		nvp6124_i2c_write( AHD2_FHD_BAUD, 0x0E);
		nvp6124_i2c_write( AHD2_FHD_LINE, pvInfo->vformat? 0x0D:0x0E);
	}
	else if(pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P25)	//1080p25, 30
	{
		nvp6124_i2c_write( AHD2_FHD_BAUD, 0x0E);
		nvp6124_i2c_write( AHD2_FHD_LINE, 0x0E);
	}
	nvp6124_i2c_write( AHD2_PEL_SYNC, 0x14);
	nvp6124_i2c_write( AHD2_PEL_SYNC+1, 0x00);  
	nvp6124_i2c_write( AHD2_FHD_LINES, 0x07);
	nvp6124_i2c_write( AHD2_FHD_BYTE, 0x03);
	nvp6124_i2c_write( AHD2_FHD_MODE, 0x10);
	nvp6124_i2c_write( AHD2_PEL_EVEN, 0x00);

	//Decoder RX Setting
	nvp6124_i2c_write( 0xFF, BANK5);
	nvp6124_i2c_write( 0x30, 0x00);
	nvp6124_i2c_write( 0x31, 0x01);
	nvp6124_i2c_write( 0x32, 0x64);
	nvp6124_i2c_write( 0x7C, 0x11);

	if(pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25)			//720p25, 30
	{
		nvp6124_i2c_write( 0x7D, 0x80);
	}
	else if(pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P25)	//1080p25, 30
	{
		nvp6124_i2c_write( 0x7D, 0x80);
	}

	nvp6124_i2c_write( 0xFF, BANK3);

	if(pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25)			//720p25, 30
	{
		nvp6124_i2c_write( 0x62, pvInfo->vformat? 0x05:0x06);	// Camera TX DATA Check
		nvp6124_i2c_write( 0x68, 0x40);	// RX size
	}
	else if(pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P25)	//1080p25, 30
	{
		nvp6124_i2c_write( 0x62, 0x06);	// Camera TX DATA Check
		nvp6124_i2c_write( 0x68, 0x70);	// RX size
	}

	nvp6124_i2c_write( 0x60, 0x55);
	nvp6124_i2c_write( 0x63, 0x01);
	nvp6124_i2c_write( 0x66, 0x80);
	nvp6124_i2c_write( 0x67, 0x01);
}

unsigned char nvp6124_read_acp_status(void)
{
	unsigned char val;

	nvp6124_i2c_write( 0xFF, 0x03);
	nvp6124_i2c_read( 0x78, &val);

	return val;
}

unsigned char nvp6124_read_acp_pattern(void)
{
	unsigned char val;

	nvp6124_i2c_write( 0xFF, 0x03);
	nvp6124_i2c_read( 0x78+1, &val);
	val = (val >> 2) & 0x03;

	return val;
}

static void nvp6124_init_acp_status(void)
{
	nvp6124_i2c_write( 0xFF, BANK3 );
	nvp6124_i2c_write( AHD2_FHD_LINES, 0x07);		
	nvp6124_i2c_write( ACP_MODE_ID, ACP_CAM_STAT);
	nvp6124_i2c_write( AHD2_FHD_D0, ACP_RX_D0);
	nvp6124_i2c_write( AHD2_FHD_D0+1,ACP_RX_D0+1);
	nvp6124_i2c_write( AHD2_FHD_D0+2,ACP_RX_D0+2);
	nvp6124_i2c_write( AHD2_FHD_D0+3,ACP_RX_D0+3);
	nvp6124_i2c_write( AHD2_FHD_OUT, 0x08);

	Delay1MS(100);
	xprintf("ACP_Camera_status_mode_set \n");
}

static void nvp6124_init_acp_reg_wr(void)
{
	nvp6124_i2c_write( 0xFF, BANK3 );
	nvp6124_i2c_write( AHD2_FHD_LINES, 0x03);	
	nvp6124_i2c_write( ACP_MODE_ID, ACP_REG_WR);
	xprintf("ACP_register_write_mode_set complete \n");
}

static void nvp6124_init_acp_reg_rd(void)
{
	nvp6124_i2c_write( 0xFF, BANK3 );
	nvp6124_i2c_write( AHD2_FHD_LINES, 0x03);
	nvp6124_i2c_write( ACP_MODE_ID, ACP_REG_RD);
	xprintf("ACP_register_read_mode_set \n");
}

static void nvp6124_acp_sw_ver_transer(void)
{
	acp_isp_write( 0x8485, DRIVER_SW_VER_MAJOR);
	Delay1MS(100);
	acp_isp_write( 0x8486, DRIVER_SW_VER_MINOR);
	Delay1MS(100);
	
	nvp6124_i2c_write( 0xFF, BANK3 );
	nvp6124_i2c_write( 0x09, 0x00 );
}

static unsigned char nvp6124_acp_isp_read(unsigned int reg_addr)
{
	unsigned char data;
	unsigned char bank;
	unsigned char addr;

	bank = (reg_addr>>8)&0xFF;
	addr = reg_addr&0xFF;

	nvp6124_init_acp_reg_rd();
	get_acp_reg_rd(bank, addr);

	nvp6124_i2c_write( 0xFF, BANK3 );
	nvp6124_i2c_read( ACP_RX_D0+3, &data);
	nvp6124_i2c_write( AHD2_FHD_OUT, 0x00 );
	acp_reg_rx_clear();

	return data;
}

static void nvp6124_acp_isp_write(unsigned int reg_addr, unsigned char reg_data)
{
	nvp6124_i2c_write( 0xFF, BANK3 );
	nvp6124_i2c_write( AHD2_FHD_LINES, 0x03);
	nvp6124_i2c_write( ACP_MODE_ID, ACP_REG_WR);
	nvp6124_i2c_write( 0x10, ACP_REG_WR);
	nvp6124_i2c_write( 0x11, (reg_addr>>8)&0xFF);
	nvp6124_i2c_write( 0x12, reg_addr&0xFF);
	nvp6124_i2c_write( 0x13, reg_data);
	nvp6124_i2c_write( 0x09, 0x08);
}

static void nvp6124_set_acp_reg_wr(unsigned char bank, unsigned char addr, unsigned char data)
{
	nvp6124_i2c_write( 0xFF, BANK3 );
	nvp6124_i2c_write( AHD2_FHD_D0, ACP_REG_WR);
	nvp6124_i2c_write( AHD2_FHD_D0+1, bank);
	nvp6124_i2c_write( AHD2_FHD_D0+2, addr);
	nvp6124_i2c_write( AHD2_FHD_D0+3, data);	
	nvp6124_i2c_write( AHD2_FHD_OUT, 0x08);
		
	Delay1MS( 100 );
	xprintf("set_ACP_register_write\n");
}

static void nvp6124_get_acp_reg_rd(unsigned char bank, unsigned char addr)
{
	nvp6124_i2c_write( 0xFF, BANK3 );
	nvp6124_i2c_write( AHD2_FHD_D0, ACP_REG_RD);
	nvp6124_i2c_write( AHD2_FHD_D0+1, bank);
	nvp6124_i2c_write( AHD2_FHD_D0+2, addr);
	nvp6124_i2c_write( AHD2_FHD_D0+3, 0x00);//Dummy
	nvp6124_i2c_write( AHD2_FHD_OUT, 0x08);
	
	Delay1MS( 100 );
	xprintf("get_register_read\n");
}

static void nvp6124_acp_read(AHD_INFO *pvInfo)
{
	unsigned char buf[16];
	unsigned char val, i;
	unsigned char ch;

	nvp6124_i2c_write( 0xFF, BANK0);

	nvp6124_i2c_read( 0xc7, &val);
	xprintf("ACP_RX_DONE = %x\n", val);
	
	nvp6124_i2c_write( 0xFF, BANK3);
	nvp6124_i2c_read( ACP_RX_D0, &val);

	if(val == ACP_CAM_STAT)
	{
		for(i=0;i<8;i++)
		{
			nvp6124_i2c_read( ACP_RX_D0+i, &buf[i]);
			nvp6124_i2c_write( AHD2_FHD_D0+i, buf[i]); 
			pvInfo->rx.getacpdata[i] = buf[i];
		}
		nvp6124_i2c_write( AHD2_FHD_OUT, 0x08); 

		xprintf("ACP_DATA = [%02x %02x %02x %02x %02x %02x %02x %02x]\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
	}
	else if(val == ACP_REG_WR)
	{
		for(i=0;i<4;i++)
		{
			nvp6124_i2c_write( ACP_RX_D0+i, buf[i]); 
			pvInfo->rx.getacpdata[i] = buf[i];
		}
		nvp6124_i2c_write( AHD2_FHD_OUT, 0x00);

		xprintf("ACP_Write = 0x%02x 0x%02x 0x%02x 0x%02x\n",buf[0], buf[1], buf[2], buf[3]);
	}
	else if(val == ACP_REG_RD)
	{
		for(i=0;i<4;i++)
		{
			nvp6124_i2c_write( ACP_RX_D0+i, buf[i]); 
			pvInfo->rx.getacpdata[i] = buf[i];
		}
		nvp6124_i2c_write( AHD2_FHD_OUT, 0x00);

		xprintf("ACP_Read = 0x%02x 0x%02x 0x%02x 0x%02x\n",buf[0], buf[1], buf[2], buf[3]);
	}
	else
	{
		for(i=0;i<8;i++)
		{
			pvInfo->rx.getacpdata[i] = 0x00;
		}
		nvp6124_i2c_write( AHD2_FHD_OUT, 0x00);		
		xprintf("ACP_RX_Error!!!!\n");
	}
	acp_reg_rx_clear();
}

static unsigned char nvp6124_pelco_coax_mode(AHD_INFO *pvInfo)
{
	nvp6124_i2c_write(0xff, BANK5);

	if(pvInfo->rx.current_resolution != RSV)		nvp6124_i2c_write(0x7c, 0x11);
	else							xprintf("Failed set nvp6124 coax protocol\n"); 
		
	switch(pvInfo->rx.current_resolution) 
	{
		case AHDRX_OUT_960X480:		//for SD
		case AHDRX_OUT_960X576:
			nvp6124_i2c_write(0xff, BANK3);	
			nvp6124_i2c_write(AHD2_PEL_BAUD, 	0x1B); 								/* Pelco TX Baud Rate - 1us */	
			nvp6124_i2c_write(AHD2_PEL_LINE, 	0x0E);								/* Pelco Protocol TX Start Line in VBI - 18,19 line */	
			nvp6124_i2c_write(PACKET_MODE, 		0x06);								/* Coaxial Protocol Type - ? */	
			nvp6124_i2c_write(AHD2_PEL_SYNC+0, 	pvInfo->vformat? 0x28 : 0x40 );		/* Start Point in Coaxial Protocol Active Line LSB */
			nvp6124_i2c_write(AHD2_PEL_SYNC+1, 	pvInfo->vformat? 0x00 : 0x00 );		/* Start Point in Coaxial Protocol Active Line MSB */		
			nvp6124_i2c_write(AHD2_PEL_EVEN, 	0x01);								/* Assert on odd frame */	
			break;
		case AHDRX_OUT_1280X720P30:		//for AHD
		case AHDRX_OUT_1280X720P25:
			nvp6124_i2c_write(0xff, BANK3);	
			nvp6124_i2c_write(AHD2_PEL_BAUD, 	0x0d); 								/* Pelco TX Baud Rate - 0.5us */	
			nvp6124_i2c_write(AHD2_PEL_LINE, 	pvInfo->vformat? 0x0d :	0x0E);		/* Pelco Protocol TX Start Line in VBI - 17,18 line */	
			nvp6124_i2c_write(PACKET_MODE, 		0x06);								/* Coaxial Protocol Type - ? */	
			nvp6124_i2c_write(AHD2_PEL_SYNC+0, 	pvInfo->vformat? 0x38 : 0x40 );		/* Start Point in Coaxial Protocol Active Line LSB */
			nvp6124_i2c_write(AHD2_PEL_SYNC+1, 	pvInfo->vformat? 0x05 : 0x04 );		/* Start Point in Coaxial Protocol Active Line MSB */		
			nvp6124_i2c_write(AHD2_PEL_EVEN, 	0x00);
			break;
		case AHDRX_OUT_1920X1080P30:		//for FAHD 
		case AHDRX_OUT_1920X1080P25:		 
			nvp6124_i2c_write(0xff, BANK3);	
			nvp6124_i2c_write(AHD2_FHD_BAUD, 	0x0e); 								/* ACP TX Baud Rate - ? */	
			nvp6124_i2c_write(AHD2_FHD_LINE, 	pvInfo->vformat? 0x0e : 0x0e );		/* ACP TX Start Line in VBI - ? */	
			nvp6124_i2c_write(AHD2_PEL_SYNC+0, 	pvInfo->vformat? 0x28 : 0x24 );		/* Start Point in Coaxial Protocol Active Line LSB */	
			nvp6124_i2c_write(AHD2_FHD_LINES, 	0x03);								/* ACP Line number - ? */ 
			nvp6124_i2c_write(AHD2_FHD_BYTE, 	0x03);								/* ACP Transmission amount - ? */ 
			nvp6124_i2c_write(AHD2_FHD_MODE, 	0x10);								/* Coaxial Protocol Type - ? */ 
			nvp6124_i2c_write(AHD2_PEL_SYNC+1, 	pvInfo->vformat? 0x00 : 0x00 );								/* Start Point in Coaxial Protocol Active Line MSB */ 
			nvp6124_i2c_write(AHD2_PEL_EVEN, 	0x00);								/* Assert on all frame */	
			break;
		default:
			break;
	}
}

unsigned char nvp6124_pelco_command(AHD_INFO *pvInfo, unsigned char command )
{
	unsigned char i, ret = 0;
	unsigned char str[4];

	nvp6124_pelco_coax_mode(pvInfo);	
	Delay1MS(20);

	if(pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P25){
		switch(command)
		{
			case  PELCO_CMD_RESET :
				str[0] = 0x00;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_SET:
				str[0] = 0x02;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_UP:			
				str[0] = 0x00;str[1] = 0x08;str[2] = 0x00;str[3] = 0x32; ret = 1;
				break;
			case  PELCO_CMD_DOWN:			
				str[0] = 0x00;str[1] = 0x10;str[2] = 0x00;str[3] = 0x32; ret = 1;
				break;
			case  PELCO_CMD_LEFT:			
				str[0] = 0x00;str[1] = 0x04;str[2] = 0x32;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_RIGHT:			
				str[0] = 0x00;str[1] = 0x02;str[2] = 0x32;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_OSD:			
				str[0] = 0x00;str[1] = 0x03;str[2] = 0x00;str[3] = 0x3F; ret = 1;
				break;
			case  PELCO_CMD_IRIS_OPEN:
				str[0] = 0x02;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_IRIS_CLOSE:	
				str[0] = 0x04;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_FOCUS_NEAR:	
				str[0] = 0x01;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_FOCUS_FAR:		
				str[0] = 0x00;str[1] = 0x80;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_ZOOM_WIDE:		
				str[0] = 0x00;str[1] = 0x40;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_ZOOM_TELE:		
				str[0] = 0x00;str[1] = 0x20;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			default:
				break; //unexpected command
		}		
	}else if(pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25){ 
		switch(command)
		{
			case  PELCO_CMD_RESET :
				str[0] = 0x00;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_SET:
				str[0] = 0x40;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_UP:			
				str[0] = 0x00;str[1] = 0x10;str[2] = 0x10;str[3] = 0x4C; ret = 1;
				break;
			case  PELCO_CMD_DOWN:				
				str[0] = 0x00;str[1] = 0x08;str[2] = 0x08;str[3] = 0x4C; ret = 1;
				break;
			case  PELCO_CMD_LEFT:				
				str[0] = 0x00;str[1] = 0x20;str[2] = 0x20;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_RIGHT:				
				str[0] = 0x00;str[1] = 0x40;str[2] = 0x40;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_OSD:				
				str[0] = 0x00;str[1] = 0xC0;str[2] = 0xC0;str[3] = 0xFA; ret = 1;
				break;
			case  PELCO_CMD_IRIS_OPEN:
				str[0] = 0x40;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_IRIS_CLOSE:		
				str[0] = 0x20;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_FOCUS_NEAR:		
				str[0] = 0x80;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_FOCUS_FAR:				
				str[0] = 0x00;str[1] = 0x01;str[2] = 0x01;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_ZOOM_WIDE:				
				str[0] = 0x00;str[1] = 0x02;str[2] = 0x02;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_ZOOM_TELE:				
				str[0] = 0x00;str[1] = 0x04;str[2] = 0x04;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_SCAN_SR:			
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0xE0;str[3] = 0x46; ret = 1;
				break;
			case  PELCO_CMD_SCAN_ST:			
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0xE0;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_PRESET1:			
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0xE0;str[3] = 0x80; ret = 1;
				break;
			case  PELCO_CMD_PRESET2:		
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0xE0;str[3] = 0x40; ret = 1;
				break;
			case  PELCO_CMD_PRESET3:			
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0xE0;str[3] = 0xC0; ret = 1;
				break;
			case  PELCO_CMD_PTN1_SR:			
				str[0] = 0x00;str[1] = 0xF8;str[2] = 0xF8;str[3] = 0x01; ret = 1;
				break;
			case  PELCO_CMD_PTN1_ST:			
				str[0] = 0x00;str[1] = 0x84;str[2] = 0x84;str[3] = 0x01; ret = 1;
				break;
			case  PELCO_CMD_PTN2_SR:			
				str[0] = 0x00;str[1] = 0xF8;str[2] = 0xF8;str[3] = 0x02; ret = 1;
				break;
			case  PELCO_CMD_PTN2_ST:			
				str[0] = 0x00;str[1] = 0x84;str[2] = 0x84;str[3] = 0x02; ret = 1;
				break;
			case  PELCO_CMD_PTN3_SR:			
				str[0] = 0x00;str[1] = 0xF8;str[2] = 0xF8;str[3] = 0x03; ret = 1;
				break;
			case  PELCO_CMD_PTN3_ST:			
				str[0] = 0x00;str[1] = 0x84;str[2] = 0x84;str[3] = 0x03; ret = 1;
				break;
			case  PELCO_CMD_RUN:				
				str[0] = 0x00;str[1] = 0xC4;str[2] = 0xC4;str[3] = 0x00; ret = 1;
				break;
			default :
				break;
		}		
	}else if(pvInfo->rx.current_resolution == AHDRX_OUT_960X480 || pvInfo->rx.current_resolution == AHDRX_OUT_960X576){
		switch(command)
		{
			case  PELCO_CMD_RESET :
				str[0] = 0x00;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_SET:
				str[0] = 0x40;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00; ret = 1; 
				break;
			case  PELCO_CMD_UP:			
				str[0] = 0x00;str[1] = 0x10;str[2] = 0x00;str[3] = 0x4C; ret = 1;
				break;
			case  PELCO_CMD_DOWN:			
				str[0] = 0x00;str[1] = 0x08;str[2] = 0x00;str[3] = 0x4C; ret = 1;
				break;
			case  PELCO_CMD_LEFT:			
				str[0] = 0x00;str[1] = 0x20;str[2] = 0x4C;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_RIGHT:			
				str[0] = 0x00;str[1] = 0x40;str[2] = 0x4C;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_OSD:			
				str[0] = 0x00;str[1] = 0xC0;str[2] = 0x00;str[3] = 0xFA; ret = 1;
				break;
			case  PELCO_CMD_IRIS_OPEN:
				str[0] = 0x40;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_IRIS_CLOSE:	
				str[0] = 0x20;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_FOCUS_NEAR:	
				str[0] = 0x80;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_FOCUS_FAR:		
				str[0] = 0x00;str[1] = 0x01;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_ZOOM_WIDE:		
				str[0] = 0x00;str[1] = 0x02;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_ZOOM_TELE:		
				str[0] = 0x00;str[1] = 0x04;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_SCAN_SR:		
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0x00;str[3] = 0x46; ret = 1;
				break;
			case  PELCO_CMD_SCAN_ST:		
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			case  PELCO_CMD_PRESET1:		
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0x00;str[3] = 0x80; ret = 1;
				break;
			case  PELCO_CMD_PRESET2:		
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0x00;str[3] = 0x40; ret = 1;
				break;
			case  PELCO_CMD_PRESET3:		
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0x00;str[3] = 0xC0; ret = 1;
				break;
			case  PELCO_CMD_PTN1_SR:		
				str[0] = 0x00;str[1] = 0xF8;str[2] = 0x00;str[3] = 0x01; ret = 1;
				break;
			case  PELCO_CMD_PTN1_ST:		
				str[0] = 0x00;str[1] = 0x84;str[2] = 0x00;str[3] = 0x01; ret = 1;
				break;
			case  PELCO_CMD_PTN2_SR:		
				str[0] = 0x00;str[1] = 0xF8;str[2] = 0x00;str[3] = 0x02; ret = 1;
				break;
			case  PELCO_CMD_PTN2_ST:		
				str[0] = 0x00;str[1] = 0x84;str[2] = 0x00;str[3] = 0x02; ret = 1;
				break;
			case  PELCO_CMD_PTN3_SR:		
				str[0] = 0x00;str[1] = 0xF8;str[2] = 0x00;str[3] = 0x03; ret = 1;
				break;
			case  PELCO_CMD_PTN3_ST:		
				str[0] = 0x00;str[1] = 0x84;str[2] = 0x00;str[3] = 0x03; ret = 1;
				break;
			case  PELCO_CMD_RUN:			
				str[0] = 0x00;str[1] = 0xC4;str[2] = 0x00;str[3] = 0x00; ret = 1;
				break;
			default :
				break;
		}	
	}else{
	}

	nvp6124_i2c_write(0xff, BANK3); 
	
	if(pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P25)
	{
		for(i=0; i<4; i++)
		{
			nvp6124_i2c_write(AHD2_FHD_D0+i, str[i] );
		}
		nvp6124_pelco_shot(pvInfo);
	}
	else if(pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25)
	{
		for(i=0; i<4; i++)
		{
			nvp6124_i2c_write(AHD2_PEL_D0+i, str[i] );
		}
		nvp6124_pelco_shot(pvInfo);
	}
	else if(pvInfo->rx.current_resolution == AHDRX_OUT_960X480 || pvInfo->rx.current_resolution == AHDRX_OUT_960X576)
	{
		for(i=0; i<4; i++)
		{
			nvp6124_i2c_write(AHD2_PEL_D0+i, str[i] );
		}
		nvp6124_pelco_shot(pvInfo);
	}else
	{
	}
}

static void nvp6124_pelco_shot(AHD_INFO *pvInfo)
{
	nvp6124_i2c_write(0xff, BANK3); 

	if(pvInfo->rx.current_resolution == AHDRX_OUT_960X480 || pvInfo->rx.current_resolution == AHDRX_OUT_960X576){
		nvp6124_i2c_write(AHD2_PEL_SHOT, 0x01); 
		nvp6124_i2c_write(AHD2_PEL_SHOT, 0x00); 
		xprintf("960h Coax shot!!\n");
	}else if(pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25){
		nvp6124_i2c_write(AHD2_PEL_SHOT, 0x01); 
		nvp6124_i2c_write(AHD2_PEL_SHOT, 0x00); 
		xprintf("720p Coax shot!!\n");
	}else if(pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P25){
		nvp6124_i2c_write(AHD2_FHD_OUT, 0x08); 
		nvp6124_i2c_write(AHD2_FHD_OUT, 0x00); 
		xprintf("1080p Coax shot!!\n");
	}else{
	}
}
