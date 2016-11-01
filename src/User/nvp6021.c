/**
 *	@file   nvp6021.c
 *	@brief	
 *	@author luisfynn <tani223@pinetron.com>
 *	@date   2015/05/08 15:59
 */

/* system include */

/* local include */
#include "i2c.h"
#include "video.h"
#include "stm32f10x_conf.h"
#include "nvp6124_utc.h"
#include "command.h"
#include "mutex.h"

MUTEX_DECLARE(s_sem_nvp6021);

static void nvp6021_i2c_lock(void)      { MUTEX_LOCK(s_sem_nvp6021);    }
static void nvp6021_i2c_unlock(void)    { MUTEX_UNLOCK(s_sem_nvp6021);  }

#define NVP6021_I2C_ADDR 			0x60
#define NVP6021_UTC_RECEIVE_UP		0x08
#define NVP6021_UTC_RECEIVE_DOWN	0x10
#define NVP6021_UTC_RECEIVE_LEFT	0x04
#define NVP6021_UTC_RECEIVE_RIGHT	0x02
#define NVP6021_UTC_RECEIVE_SET		0x200
#define NVP6021_UTC_RECEIVE_OSD		0x400

static void nvp6021_i2c_write(unsigned char reg_addr, unsigned char reg_data)
{  
	I2C1_ByteWrite(NVP6021_I2C_ADDR, reg_addr, reg_data);
}

static unsigned char nvp6021_i2c_read(unsigned char reg_addr, unsigned char* p_reg_data)
{  
	if(I2C_OK != I2C1_ByteRead(NVP6021_I2C_ADDR, reg_addr, p_reg_data)) return I2C_FAIL;
	return I2C_OK;
}

void nvp6021_tx_coax_init(void)
{
	nvp6021_i2c_lock();

	nvp6021_i2c_write(0xFF,BANK2);
	nvp6021_i2c_write(0x63,0xFF);	// TX out-put Threshold
	nvp6021_i2c_write(0xFF,0x03);

	nvp6021_i2c_write(0x20,0x2F);	// A-CP TX BOUD
	nvp6021_i2c_write(0x23,0x08);	// Line position1
	nvp6021_i2c_write(0x24,0x00);	// Line position2
	nvp6021_i2c_write(0x25,0x07);	// Lines count
	nvp6021_i2c_write(0x2B,0x10);	// A-CP mode choose
	nvp6021_i2c_write(0x2D,0x0D);	// Start point1
	nvp6021_i2c_write(0x2E,0x01);	// Start point2
	nvp6021_i2c_write(0xA9,0x00);	// i2c master mode off

	nvp6021_i2c_write(0x30,0x55);	// HEADER
	nvp6021_i2c_write(0x31,0x24);	// EQ Pattern [7:4]=device NVP6021 set, [3:2]=EQ Pattern [1]=Color status
	nvp6021_i2c_write(0x32,0x00);	
	nvp6021_i2c_write(0x33,0x00);	
	nvp6021_i2c_write(0x34,0x00);	
	nvp6021_i2c_write(0x35,0x00);
	nvp6021_i2c_write(0x36,0x00);	
	nvp6021_i2c_write(0x37,0x00);	
	nvp6021_i2c_write(0x29,0x08);	// Status out
	
	nvp6021_i2c_unlock();
}

void nvp6021_rx_coax_init(void)
{
	nvp6021_i2c_lock();

	nvp6021_i2c_write(0xFF, BANK0);
	nvp6021_i2c_write(0x17, 0x00); 	//RX Threshold on-interrupt
	nvp6021_i2c_write(0xFF, 0x03);

	nvp6021_i2c_write(0x09, 0x00);
	nvp6021_i2c_write(0x80, 0x55);	//RX ID
	nvp6021_i2c_write(0x82, 0x10);
	nvp6021_i2c_write(0x83, 0x01);
	nvp6021_i2c_write(0x86, 0x80);
	nvp6021_i2c_write(0x87, 0x01);
	nvp6021_i2c_write(0x88, 0x20);

	nvp6021_i2c_write(0x30, 0x55); //TX ID

	nvp6021_i2c_unlock();
}

void nvp6021_coax_receive(AHD_INFO *pvInfo)
{
	unsigned char coax_val_high, coax_val_low;
	unsigned short coax_value;

	nvp6021_i2c_lock();
	
	nvp6021_i2c_write(0xff, BANK3);
	
	nvp6021_i2c_read(0x91, &coax_val_high);
	nvp6021_i2c_read(0x93, &coax_val_low);
	
	coax_value = coax_val_high <<8 | coax_val_low ;

	if		(coax_value == NVP6021_UTC_RECEIVE_UP)		nvp6124_pelco_command(pvInfo, PELCO_CMD_UP);		//up key
	else if	(coax_value == NVP6021_UTC_RECEIVE_DOWN)	nvp6124_pelco_command(pvInfo, PELCO_CMD_DOWN);		//down key
	else if	(coax_value == NVP6021_UTC_RECEIVE_LEFT)	nvp6124_pelco_command(pvInfo, PELCO_CMD_LEFT);		//left key
	else if	(coax_value == NVP6021_UTC_RECEIVE_RIGHT)	nvp6124_pelco_command(pvInfo, PELCO_CMD_RIGHT);		//right key
	else if	(coax_value == NVP6021_UTC_RECEIVE_SET)		nvp6124_pelco_command(pvInfo, PELCO_CMD_SET);		//set key
	else if	(coax_value == NVP6021_UTC_RECEIVE_OSD)		nvp6124_pelco_command(pvInfo, PELCO_CMD_OSD);		//osd key
	
	nvp6021_i2c_unlock();
}

static void nvp6021_1080p_pll(AHD_INFO *pvInfo)
{
	nvp6021_i2c_lock();
	
	nvp6021_i2c_write(0xFF, BANK0);
	nvp6021_i2c_write(0x01, pvInfo->vformat? 0xf1 : 0xf0);		
	nvp6021_i2c_write(0x04, pvInfo->vformat? 0x00 : 0x00);		
	nvp6021_i2c_write(0x00, 0x01); //must be set
	
	nvp6021_i2c_unlock();
}

static void nvp6021_720p_pll(AHD_INFO *pvInfo)
{
	nvp6021_i2c_lock();
	
	nvp6021_i2c_write(0xFF, BANK0);
	nvp6021_i2c_write(0x00, pvInfo->vformat? 0x07 : 0x07);		//720p pll & clock phase manual setting
	nvp6021_i2c_write(0x01, pvInfo->vformat? 0xe1 : 0xe0);		
	nvp6021_i2c_write(0x05, pvInfo->vformat? 0x00 : 0x00);		
	nvp6021_i2c_write(0x06, pvInfo->vformat? 0x20 : 0x20);		
	nvp6021_i2c_write(0x07, pvInfo->vformat? 0x18 : 0x18);		

	nvp6021_i2c_write(0x05, 0xff);		
	Delay5US(20);	
	nvp6021_i2c_write(0x05, 0x00);		

	nvp6021_i2c_write(0x11, 0x00);		
	nvp6021_i2c_write(0x12, 0x60);		
	
	nvp6021_i2c_unlock();
}

static void nvp6021_common_init(AHD_INFO *pvInfo)
{
	// mode						high							low
	//---------------------------------------------------------------------------------
	//pvInfo->app_mode 		1(high):repeater mode 	 		0(low): converter mode
	//pvInfo->input_mode 	1(high):hdmi input mode   		0(low): analog video input mode
	//pvInfo->output_mode 	1(high):full hd output mode   	0(low): hd output mode
	//pvInfo->vformat 		1(high):25p or PAL  			0(low): 30p or NTSC 
	//---------------------------------------------------------------------------------

	if(pvInfo->app_mode)						 
	{	
		switch(pvInfo->rx.current_resolution)
		{
			case AHDRX_OUT_1280X720P30:	
			case AHDRX_OUT_1280X720P25:	
				nvp6021_720p_pll(pvInfo);
				break;
			case AHDRX_OUT_1920X1080P30:	
			case AHDRX_OUT_1920X1080P25:	
			case RSV:						//for 1080p test pattern
				nvp6021_1080p_pll(pvInfo);
				break;
			default:	
				break;
		}
	}
	else if(!pvInfo->input_mode)		
	{
	}
	else										
	{
		if(pvInfo->tx.current_resolution)
		{
			nvp6021_1080p_pll(pvInfo);
		}
		else
		{
			nvp6021_720p_pll(pvInfo);
		}
	}

	nvp6021_i2c_write(0xFF, BANK1);
	nvp6021_i2c_write(0x0E, 0x00); //Bit swap off

	nvp6021_i2c_write(0xFF, BANK2);
	nvp6021_i2c_write(0x00, 0xFE);

	nvp6021_i2c_write(0x02, 0x00);

	nvp6021_i2c_write(0x04, 0x80); //2015.06.17
	nvp6021_i2c_write(0x05, 0x00); //swap cb-cr
	nvp6021_i2c_write(0x06, 0x10);

	nvp6021_i2c_write(0x0C, 0x04); //SYNC LEVEL
	nvp6021_i2c_write(0x0D, 0x3F); //BLACK LEVEL
	nvp6021_i2c_write(0x0E, 0x00);
	nvp6021_i2c_write(0x10, 0xEB);
	nvp6021_i2c_write(0x11, 0x10);
	nvp6021_i2c_write(0x12, 0xF0);
	nvp6021_i2c_write(0x13, 0x10);
	nvp6021_i2c_write(0x14, 0x01);
	nvp6021_i2c_write(0x15, 0x00);
	nvp6021_i2c_write(0x16, 0x00);
	nvp6021_i2c_write(0x17, 0x00);
	nvp6021_i2c_write(0x18, 0x00);
	nvp6021_i2c_write(0x19, 0x00);

	nvp6021_i2c_write(0x1C, 0x80); //y-scale
	nvp6021_i2c_write(0x1D, 0x80); //cb-scale
	nvp6021_i2c_write(0x1E, 0x80); //cr-scale

	nvp6021_i2c_write(0x20, 0x00);
	nvp6021_i2c_write(0x21, 0x00);
	nvp6021_i2c_write(0x22, 0x00);
	nvp6021_i2c_write(0x23, 0x00);
	nvp6021_i2c_write(0x24, 0x00);
	nvp6021_i2c_write(0x25, 0x00);
	nvp6021_i2c_write(0x26, 0x00);
	nvp6021_i2c_write(0x27, 0x00);
	nvp6021_i2c_write(0x28, 0x00);
	nvp6021_i2c_write(0x29, 0x00);
	nvp6021_i2c_write(0x2A, 0x00);
	nvp6021_i2c_write(0x2B, 0x00);
	nvp6021_i2c_write(0x2C, 0x00);
	nvp6021_i2c_write(0x2D, 0x00);
	nvp6021_i2c_write(0x2E, 0x00);
	nvp6021_i2c_write(0x2F, 0x00);
	nvp6021_i2c_write(0x30, 0x00);
	nvp6021_i2c_write(0x31, 0x00);
	nvp6021_i2c_write(0x32, 0x00);
	nvp6021_i2c_write(0x33, 0x00);
	nvp6021_i2c_write(0x34, 0x00);

	nvp6021_i2c_write(0x36, 0x01);
	nvp6021_i2c_write(0x37, 0x80);

	nvp6021_i2c_write(0x39, 0x00);

	nvp6021_i2c_write(0x3C, 0x00);
	nvp6021_i2c_write(0x3D, 0x00);
	nvp6021_i2c_write(0x3E, 0x00);

	nvp6021_i2c_write(0x40, 0x01);
	nvp6021_i2c_write(0x41, 0xFF);
	nvp6021_i2c_write(0x42, 0x80);

	nvp6021_i2c_write(0x48, 0x00);
	nvp6021_i2c_write(0x49, 0x00);
	nvp6021_i2c_write(0x4A, 0x00);
	nvp6021_i2c_write(0x4B, 0x00);
	nvp6021_i2c_write(0x4C, 0x00);
	nvp6021_i2c_write(0x4D, 0x00);
	nvp6021_i2c_write(0x4E, 0x00);
	nvp6021_i2c_write(0x4F, 0x00);
	nvp6021_i2c_write(0x50, 0x00);
	nvp6021_i2c_write(0x51, 0x00);
	nvp6021_i2c_write(0x52, 0x00);
	nvp6021_i2c_write(0x53, 0x00);
	nvp6021_i2c_write(0x54, 0x00);
	nvp6021_i2c_write(0x55, 0x00);
	nvp6021_i2c_write(0x56, 0x00);
	nvp6021_i2c_write(0x57, 0x00);
	nvp6021_i2c_write(0x59, 0x00);
	nvp6021_i2c_write(0x5A, 0x00);
	nvp6021_i2c_write(0x5C, 0x00);
	nvp6021_i2c_write(0x5D, 0x00);
	nvp6021_i2c_write(0x5E, 0x00);
	nvp6021_i2c_write(0x5F, 0x00);


	nvp6021_i2c_write(0x60, 0x80);
	nvp6021_i2c_write(0x61, 0x80);
	nvp6021_i2c_write(0x63, 0xE0);
	nvp6021_i2c_write(0x64, 0x19);
	nvp6021_i2c_write(0x65, 0x04);
	nvp6021_i2c_write(0x66, 0xEB);
	nvp6021_i2c_write(0x67, 0x60);
	nvp6021_i2c_write(0x68, 0x00);
	nvp6021_i2c_write(0x69, 0x00);
	nvp6021_i2c_write(0x6A, 0x00);
	nvp6021_i2c_write(0x6B, 0x00);

	//CASE1 : Repeater mode
	//CASE2-1 : Converter mode & Analog(composite or AHD) video input
	//CASE2-2 : Converter mode & HDMI video input

	if(pvInfo->app_mode)							//CASE1 
	{	
		switch(pvInfo->rx.current_resolution)
		{
			case AHDRX_OUT_1280X720P30:	
			case AHDRX_OUT_1280X720P25:	
				nvp6021_i2c_write(0xFF, BANK2);
				nvp6021_i2c_write(0x3A, pvInfo->vformat? 0x13 : 0x13);
				nvp6021_i2c_write(0x01, pvInfo->vformat? 0xEB : 0xea);
				break;
			case AHDRX_OUT_1920X1080P30:	
			case AHDRX_OUT_1920X1080P25:	
			case RSV:						//if no video, show test pattern	
				nvp6021_i2c_write(0xFF, BANK2);
				nvp6021_i2c_write(0x3A, pvInfo->vformat? 0x11 : 0x11);
				nvp6021_i2c_write(0x01, pvInfo->vformat? 0xF1 : 0xf0);

				nvp6021_i2c_write(0xFF, BANK2);
				nvp6021_i2c_write(0x3D, pvInfo->vformat? 0x80 : 0x80);   //pn enable

				nvp6021_i2c_write(0x5c, pvInfo->vformat? 0x52 : 0x52);   //pn value
				nvp6021_i2c_write(0x5d, pvInfo->vformat? 0xC3 : 0xca);   //pn value
				nvp6021_i2c_write(0x5e, pvInfo->vformat? 0x7D : 0xf0);   //pn value
				nvp6021_i2c_write(0x5f, pvInfo->vformat? 0xC8 : 0x2c);   //pn value

				nvp6021_i2c_write(0x1D, pvInfo->vformat? 0x80 : 0xa0);   //cb scale -b0
				nvp6021_i2c_write(0x1E, pvInfo->vformat? 0x80 : 0xa0);   //cr scale -b0

				nvp6021_i2c_write(0x37, pvInfo->vformat? 0x80 : 0xa0);   //burst scale

				nvp6021_i2c_write(0x5A, pvInfo->vformat? 0x07 : 0x07);   //EXPANDER_MODE
				break;
			default:
				break;
		}
	}
	else if(!pvInfo->input_mode)		//CASE2-1
	{
	}
	else		//CASE2-2
	{
		if(pvInfo->tx.current_resolution)
		{
			nvp6021_i2c_write(0xFF, BANK2);
			nvp6021_i2c_write(0x3A, pvInfo->vformat? 0x11 : 0x11);
			nvp6021_i2c_write(0x01, pvInfo->vformat? 0xF1 : 0xf0);

			nvp6021_i2c_write(0x3D, pvInfo->vformat? 0x80 : 0x80);   //pn enable

			nvp6021_i2c_write(0x5c, pvInfo->vformat? 0x52 : 0x52);   //pn value
			nvp6021_i2c_write(0x5d, pvInfo->vformat? 0xC3 : 0xca);   //pn value
			nvp6021_i2c_write(0x5e, pvInfo->vformat? 0x7D : 0xf0);   //pn value
			nvp6021_i2c_write(0x5f, pvInfo->vformat? 0xC8 : 0x2c);   //pn value

			nvp6021_i2c_write(0x1D, pvInfo->vformat? 0x80 : 0xa0);   //cb scale -b0
			nvp6021_i2c_write(0x1E, pvInfo->vformat? 0x80 : 0xa0);   //cr scale -b0

			nvp6021_i2c_write(0x37, pvInfo->vformat? 0x80 : 0xa0);   //burst scale

			nvp6021_i2c_write(0x5A, pvInfo->vformat? 0x07 : 0x07);   //EXPANDER_MODE
		}
		else
		{
			nvp6021_i2c_write(0xFF, BANK2);
			nvp6021_i2c_write(0x3A, pvInfo->vformat? 0x13 : 0x13);
			nvp6021_i2c_write(0x01, pvInfo->vformat? 0xEB : 0xea);
		}

	}

	nvp6021_tx_coax_init();
	nvp6021_rx_coax_init();
}

void nvp6021_test_pattern(unsigned char OnOff)
{
	nvp6021_i2c_lock();

	if(OnOff){
		nvp6021_i2c_write(0xFF, BANK1); 
		nvp6021_i2c_write(0x03, 0xE0);
		nvp6021_i2c_write(0xFF, BANK2);
		nvp6021_i2c_write(0x04, 0x81);
	}else{
		nvp6021_i2c_write(0xFF, BANK1); 
		nvp6021_i2c_write(0x03, 0x00);
		nvp6021_i2c_write(0xFF, BANK2);
		nvp6021_i2c_write(0x04, 0x80);
	}
	
	nvp6021_i2c_unlock();
}

unsigned char nvp6021_init(AHD_INFO *pvInfo)
{
	MUTEX_INIT(s_sem_nvp6021);

	nvp6021_common_init(pvInfo);
	
	if(pvInfo->app_mode)
	{
		if(pvInfo->rx.no_video_flag)  	nvp6021_test_pattern(ON);
		else							nvp6021_test_pattern(OFF);					  
	}
	else if(!pvInfo->input_mode)
	{
	}
	else
	{
	//	if(pvInfo->rx.hpa_status_port_a)	nvp6021_test_pattern(OFF);
	//	else								nvp6021_test_pattern(ON);					  
	}

	xprintf("nvp6021 INIT complete\n");
	pvInfo->tx.current_resolution? LedControl(TX_TYPE, REVERSE) : LedControl(TX_TYPE, FORWARD);
}

int do_nvp6021(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	if(argc < 2) return -1;

	if(!strcmp(argv[1], "dump")) {
		int begin_bank = 0;
		int end_bank = 2;

		if(argc > 3) {
			return -1;
		} else if(argc == 3) {
			begin_bank = end_bank = simple_strtoul(argv[2], NULL, 10);
		}

		nvp6021_i2c_lock();
		{
			unsigned char bank;

			xprintf("============AHD TX  Register Dump start============\n");
			xprintf("    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
			for(bank=begin_bank; bank<=end_bank; bank++) {
				unsigned int i;

				xprintf("=======================BANK%d=======================\n", bank);

				nvp6021_i2c_write(0xff, bank);
				for(i=0; i< 0x100; i++) {
					unsigned char data;
					nvp6021_i2c_read(i, &data);
					if(i%16==0) xprintf("%02x: ", i);
					xprintf("%02x%s", data, (i%16==15)? "\n":" ");
				}
			}
		}
		nvp6021_i2c_unlock();

	} else if(!strcmp(argv[1], "reg") && argc >= 3) {
		u16 addr = simple_strtoul(argv[2], NULL, 16);
		u8 bank = (addr >> 8) & 0xf;
		u8 reg = addr & 0xff;

		if(argc == 3) {
			u8 data;
			nvp6021_i2c_lock();
			nvp6021_i2c_write(0xff, bank); 
			nvp6021_i2c_read(reg, &data); 
			nvp6021_i2c_unlock();

			xprintf("RD) reg(%03x), data(%02x)\n", addr, data);
		} else if(argc == 4) {
			u8 data = simple_strtoul(argv[3], NULL, 16);
			nvp6021_i2c_lock();
			nvp6021_i2c_write(0xff, bank); 
			nvp6021_i2c_write(reg, data); 
			nvp6021_i2c_unlock();

			xprintf("WR) reg(%03x), data(%02x)\n", addr, data);
		} else {
			return -1;
		}
	}else if(!strcmp(argv[1], "test") && argc ==3){
		if(!strcmp(argv[2], "on")){ 
			nvp6021_test_pattern(ON);	
		}else if(!strcmp(argv[2], "off")){
			nvp6021_test_pattern(OFF);	
		}
	}
	return 0;
}

SHELL_CMD(
		nvp6021,   CONFIG_SYS_MAXARGS, 1,  do_nvp6021,
		"nvp6021 device control",
		"{option}\n"
		"  reg {addr.3}             - read\n"
		"  reg {addr.3} {data.2}    - write\n"
		"  dump [{bank}]            - dump\n"
		"  test {on/off} 			- test pattern\n" 
		);
