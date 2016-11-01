/**
 *	@file   nvp6124_eq.c
 *	@brief	
 *	@author luisfynn <tani223@pinetron.com>
 *	@date   2015/08/27 16:31
 */

/* system include */
#include <string.h>
/* local include */
#include "nvp6124.h"
#include "nvp6124_utc.h"

#define _EQ_ADJ_COLOR_
volatile unsigned char stage_update;

static unsigned char ANALOG_EQ_1080P[8]  =   {0x11,0x01,0x51,0x71,0x71,0x71,0x71,0x71};  // LPF 30MHz
static unsigned char DIGITAL_EQ_1080P[8] = {0x00,0x00,0x00,0x00,0x8B,0x8F,0x8F,0x8F};
#ifdef _EQ_ADJ_COLOR_
static unsigned char BRI_EQ_1080P[8]    = {0xF4,0xF4,0xF4,0xF4,0xF8,0xF8,0xF8,0xF8};
static unsigned char CON_EQ_1080P[8]    = {0x90,0x90,0x90,0x90,0x90,0x90,0x80,0x80};
static unsigned char SAT_EQ_1080P[8]    = {0x80,0x80,0x80,0x78,0x78,0x78,0x78,0x78};
static unsigned char BRI_EQ_720P[9]    = {0xF4,0xF4,0xF4,0xF4,0xF8,0xF8,0xF8,0xF8,0xF8};
static unsigned char CON_EQ_720P[9]    = {0x90,0x90,0x90,0x90,0x88,0x88,0x84,0x90,0x90};
static unsigned char SAT_EQ_720P[9]    = {0x84,0x84,0x84,0x80,0x80,0x80,0x80,0x84,0x84};
#endif
static unsigned char SHARP_EQ_1080P[8]  = {0x90,0x90,0x99,0x99,0x99,0x99,0x99,0x90};
static unsigned char PEAK_EQ_1080P[8]   = {0x00,0x10,0x00,0x00,0x00,0x00,0x50,0x00};
static unsigned char CTI_EQ_1080P[8]    = {0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A};
static unsigned char C_LOCK_EQ_1080P[8] = {0x92,0x92,0x92,0x92,0x92,0xA2,0xA2,0xA2};
static unsigned char UGAIN_EQ_1080P[8]  = {0x00,0x00,0x00,0x00,0x10,0x10,0x20,0x00};
static unsigned char VGAIN_EQ_1080P[8]  = {0x00,0x00,0x00,0x00,0x10,0x10,0x20,0x00};

static unsigned char SHARP_EQ_720P[9]   =  {0x90,0x90,0x99,0x99,0x99,0x99,0x99,0x90,0x90};
static unsigned char PEAK_EQ_720P[9]    =  {0x00,0x20,0x10,0x10,0x00,0x00,0x40,0x20,0x20};
static unsigned char CTI_EQ_720P[9]     =  {0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A};
static unsigned char C_LOCK_EQ_720P[9]  =  {0x92,0x92,0x92,0x92,0x92,0x92,0xA2,0x92,0xA2};
static unsigned char UGAIN_EQ_720P[9]   =  {0x30,0x30,0x30,0x30,0x30,0x30,0x40,0x30,0x30};
static unsigned char VGAIN_EQ_720P[9]   =  {0x30,0x30,0x30,0x30,0x30,0x30,0x40,0x30,0x30};
static unsigned char ANALOG_EQ_720P[9]  =  {0x13,0x03,0x53,0x73,0x73,0x73,0x73,0x03,0x13};
static unsigned char DIGITAL_EQ_720P[9] =  {0x00,0x00,0x00,0x00,0x88,0x8F,0x8F,0x00,0x00}; 

static unsigned char eq_stage = 0;
#ifdef _EQ_ADJ_COLOR_

static void nvp6124_brightness_eq(AHD_INFO *pvInfo, unsigned int stage)
{
	nvp6124_i2c_write(0xFF, BANK0);
	if(pvInfo->rx.current_resolution == 0x4 || pvInfo->rx.current_resolution == 0x8 )			//720p25, 30	
		nvp6124_i2c_write(0x0c, BRI_EQ_720P[stage]);
	else if(pvInfo->rx.current_resolution == 0x40 || pvInfo->rx.current_resolution == 0x80)	//1080p25, 30
		nvp6124_i2c_write(0x0c, BRI_EQ_1080P[stage]);
}

static void nvp6124_contrast_eq(AHD_INFO *pvInfo, unsigned int stage)
{
	nvp6124_i2c_write(0xFF, BANK0);
	if(pvInfo->rx.current_resolution == 0x4 || pvInfo->rx.current_resolution == 0x8 )			//720p25, 30	
		nvp6124_i2c_write(0x10, CON_EQ_720P[stage]);
	else if(pvInfo->rx.current_resolution == 0x40 || pvInfo->rx.current_resolution == 0x80)	//1080p25, 30
		nvp6124_i2c_write(0x10, CON_EQ_1080P[stage]);
}

static void nvp6124_saturation_eq(AHD_INFO *pvInfo, unsigned int stage)
{
	nvp6124_i2c_write(0xFF, BANK0);
	if(pvInfo->rx.current_resolution == 0x4 || pvInfo->rx.current_resolution == 0x8 )			//720p25, 30	
		nvp6124_i2c_write(0x3C, SAT_EQ_720P[stage]);
	else if(pvInfo->rx.current_resolution == 0x40 || pvInfo->rx.current_resolution == 0x80)	//1080p25, 30
		nvp6124_i2c_write(0x3C, SAT_EQ_1080P[stage]);
}
#endif
static void nvp6124_c_filter_eq(AHD_INFO *pvInfo, unsigned int stage)
{
	nvp6124_i2c_write(0xFF, BANK0);
	if(pvInfo->rx.current_resolution == 0x4 || pvInfo->rx.current_resolution == 0x8 )			//720p25, 30	
		nvp6124_i2c_write(0x21, C_LOCK_EQ_720P[stage]);
	else if(pvInfo->rx.current_resolution == 0x40 || pvInfo->rx.current_resolution == 0x80)	//1080p25, 30
		nvp6124_i2c_write(0x21, C_LOCK_EQ_1080P[stage]);
}

static void nvp6124_sharpness_eq(AHD_INFO *pvInfo, unsigned int stage)
{
	nvp6124_i2c_write(0xFF, BANK0);
	if(pvInfo->rx.current_resolution == 0x4 || pvInfo->rx.current_resolution == 0x8 )			//720p25, 30	
		nvp6124_i2c_write(0x14, SHARP_EQ_720P[stage]);
	else if(pvInfo->rx.current_resolution == 0x40 || pvInfo->rx.current_resolution == 0x80)	//1080p25, 30
		nvp6124_i2c_write(0x14, SHARP_EQ_1080P[stage]);
}

static void nvp6124_peaking_eq(AHD_INFO *pvInfo, unsigned int stage)
{
	nvp6124_i2c_write(0xFF, BANK0);
	if(pvInfo->rx.current_resolution == 0x4 || pvInfo->rx.current_resolution == 0x8 )			//720p25, 30	
		nvp6124_i2c_write(0x18, PEAK_EQ_720P[stage]);
	else if(pvInfo->rx.current_resolution == 0x40 || pvInfo->rx.current_resolution == 0x80)	//1080p25, 30
		nvp6124_i2c_write(0x18, PEAK_EQ_1080P[stage]);
}

static void nvp6124_ctigain_eq(AHD_INFO *pvInfo, unsigned int stage)
{
	nvp6124_i2c_write(0xFF, BANK0);
	if(pvInfo->rx.current_resolution == 0x4 || pvInfo->rx.current_resolution == 0x8 )			//720p25, 30	
		nvp6124_i2c_write(0x38, CTI_EQ_720P[stage]);
	else if(pvInfo->rx.current_resolution == 0x40 || pvInfo->rx.current_resolution == 0x80)	//1080p25, 30
		nvp6124_i2c_write(0x38, CTI_EQ_1080P[stage]);
}

static void nvp6124_ugain_eq(AHD_INFO *pvInfo, unsigned int stage)
{
	nvp6124_i2c_write(0xFF, BANK0);
	if(pvInfo->rx.current_resolution == 0x4 || pvInfo->rx.current_resolution == 0x8 )			//720p25, 30	
		nvp6124_i2c_write(0x44, UGAIN_EQ_720P[stage]);
	else if(pvInfo->rx.current_resolution == 0x40 || pvInfo->rx.current_resolution == 0x80)	//1080p25, 30
		nvp6124_i2c_write(0x44, UGAIN_EQ_1080P[stage]);
}

static void nvp6124_vgain_eq(AHD_INFO *pvInfo, unsigned int stage)
{
	nvp6124_i2c_write(0xFF, BANK0);
	if(pvInfo->rx.current_resolution == 0x4 || pvInfo->rx.current_resolution == 0x8 )			//720p25, 30	
		nvp6124_i2c_write(0x48, VGAIN_EQ_720P[stage]);
	else if(pvInfo->rx.current_resolution == 0x40 || pvInfo->rx.current_resolution == 0x80)	//1080p25, 30
		nvp6124_i2c_write(0x48, VGAIN_EQ_1080P[stage]);
}

static unsigned int nvp6124_get_ceq_stage(AHD_INFO *pvInfo, unsigned int acc_gain)
{
	unsigned char c_eq = 0;

	if(pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P25)		//1080p25, 30
	{
		if	   (acc_gain >= 0x000 && acc_gain < 0x052 )   c_eq = 1;
		else if(acc_gain >= 0x052 && acc_gain < 0x089 )   c_eq = 2;
		else if(acc_gain >= 0x089 && acc_gain < 0x113 )   c_eq = 3;
		else if(acc_gain >= 0x113 && acc_gain < 0x25F )   c_eq = 4;
		else if(acc_gain >= 0x25F && acc_gain < 0x700 )   c_eq = 5;
		else if(acc_gain >= 0x700 && acc_gain < 0x7FF )   c_eq = 6;
		else											  c_eq = 7;  
	}
	else if(pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25 )			//720p25, 30	
	{
		if	   (acc_gain >= 0x000 && acc_gain < 0x055 )  c_eq = 1;
		else if(acc_gain >= 0x055 && acc_gain < 0x082 )  c_eq = 2;
		else if(acc_gain >= 0x082 && acc_gain < 0x0D8 )  c_eq = 3;
		else if(acc_gain >= 0x0D8 && acc_gain < 0x18F )  c_eq = 4;
		else if(acc_gain >= 0x18F && acc_gain < 0x700 )  c_eq = 5;
		else if(acc_gain >= 0x700 && acc_gain < 0x7FF )  c_eq = 6;
		else											 c_eq = 7;  
	}

	return c_eq;
}

static unsigned int nvp6124_get_yeq_stage(AHD_INFO *pvInfo, unsigned int y_minus_slp)
{
	unsigned char y_eq = 0;

	if(pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1920X1080P25)		//1080p25, 30
	{
		if     (y_minus_slp == 0x000)						    y_eq = 0;
		else if(y_minus_slp >  0x000 && y_minus_slp < 0x0E7)    y_eq = 1;
		else if(y_minus_slp >= 0x0E7 && y_minus_slp < 0x11A)    y_eq = 2;
		else if(y_minus_slp >= 0x11A && y_minus_slp < 0x151)    y_eq = 3;
		else if(y_minus_slp >= 0x151 && y_minus_slp < 0x181)    y_eq = 4;
		else if(y_minus_slp >= 0x181 && y_minus_slp < 0x200)    y_eq = 5;
		else													y_eq = 6;
	}
	else if(pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25 )			//720p25, 30	
	{
		if     (y_minus_slp == 0x000)                           y_eq = 0;
		else if(y_minus_slp >  0x000 && y_minus_slp < 0x104)    y_eq = 1;
		else if(y_minus_slp >= 0x104 && y_minus_slp < 0x125)    y_eq = 2;
		else if(y_minus_slp >= 0x125 && y_minus_slp < 0x14C)    y_eq = 3;
		else if(y_minus_slp >= 0x14C && y_minus_slp < 0x16F)    y_eq = 4;
		else if(y_minus_slp >= 0x16F && y_minus_slp < 0x185)    y_eq = 5;
		else													y_eq = 6;
	}
	return y_eq;
}

static unsigned int nvp6124_is_bypass_mode(unsigned int agc_val, unsigned int y_ref2_sts)
{
	if(((agc_val < 0x20) && (y_ref2_sts >= 0x176)) || ((agc_val >= 0x20) && (y_ref2_sts >= 0x1B0)))          return 1;

	return 0;
}

#define ACP_CLR_CNT			1
#define ACP_SET_CNT			(ACP_CLR_CNT+1)
#define ACP_READ_START_CNT	(ACP_SET_CNT+3)
#define RETRY_CNT			5
#define COMPARE_NUM			3
#define COUNT_NUM			4
#define LOOP_BUF_SIZE		20
static unsigned char check_c_stage[LOOP_BUF_SIZE];
static unsigned char check_y_stage[LOOP_BUF_SIZE];
static unsigned char acp_val[LOOP_BUF_SIZE];
static unsigned char acp_ptn[LOOP_BUF_SIZE];
static unsigned char vidmode_back;
static unsigned char video_on;
static unsigned char eq_loop_cnt;
static unsigned char loop_cnt = 0;
static unsigned char bypass_flag_retry;
static unsigned char ystage_flag_retry;
static unsigned char cstage_flag_retry;
static unsigned char one_setting=0;
static unsigned char bypass_flag=0;
static unsigned char acp_isp_wr_en=0;

static void nvp6124_sw_reset(void)
{
	unsigned char tmp;

	nvp6124_i2c_write(0xFF, BANK1);
	
	nvp6124_i2c_read(0x97, &tmp);

	tmp &= 0x0f;
	tmp &= ~1;	
	
	nvp6124_i2c_write(0x97, tmp);
	nvp6124_i2c_write(0x97, 0x0f);
	
	xprintf("nvp6124 software reset complete \n");
}

static void nvp6124_lossfakechannel(unsigned char bypassflag)
{
	unsigned char tmp;

	nvp6124_i2c_write(0xFF, BANK0);
	nvp6124_i2c_read(0x7A, &tmp);
	
	tmp = tmp & 0xF0;

	if(bypassflag)
		tmp = tmp | 0x0F ;
	else
		tmp = tmp | 0x01 ;

	nvp6124_i2c_write(0x7A, tmp);
}

static void nvp6124_bubble_sort(unsigned char *buf ,unsigned char len)
{
	int i,j,tmp;

	for(i = 0; i < len; i++)
	{
		for(j = 0; j < len - 1 ; j++)
		{
			if(buf[j] > buf[j+1])
			{
				tmp = buf[j];
				buf[j] = buf[j+1];
				buf[j+1] = tmp;
			}
		}
	}
}

static unsigned char nvp6124_calc_stage_gap(unsigned char *buf)
{
	unsigned char cnt,zero_cnt=0;
	unsigned char tmpbuf[LOOP_BUF_SIZE];

	memcpy(tmpbuf,buf,LOOP_BUF_SIZE);

	nvp6124_bubble_sort(tmpbuf, LOOP_BUF_SIZE);

	for(cnt = 0; cnt < LOOP_BUF_SIZE; cnt++)
	{
		if(tmpbuf[cnt] == 0) zero_cnt++;
	}

	return(abs(tmpbuf[LOOP_BUF_SIZE-1] - tmpbuf[zero_cnt]));
}

static unsigned char nvp6124_calc_arr_mean(unsigned char *buf)
{
	unsigned char cnt,tmp=0,zero_cnt=0;
	unsigned char tmpbuf[LOOP_BUF_SIZE];

	memcpy(tmpbuf,buf,LOOP_BUF_SIZE);

	nvp6124_bubble_sort(tmpbuf, LOOP_BUF_SIZE);

	for(cnt = 0; cnt < LOOP_BUF_SIZE; cnt++)
	{
		if(tmpbuf[cnt] == 0) zero_cnt++;

		tmp = tmp + tmpbuf[cnt];
	}
	tmp = abs( tmp / (LOOP_BUF_SIZE-zero_cnt));

	for(cnt = 0; cnt <LOOP_BUF_SIZE; cnt++){
		xprintf("cnt[%d]  origin = %x   sort= %x\n",cnt, buf[cnt], tmpbuf[cnt]);
	}
	xprintf("mean value ========================== %d\n",tmp);

	return tmp;
}

static unsigned char nvp6124_compare_arr(unsigned char cur_ptr, unsigned char *buf)
{
	unsigned char start_ptr;

	if(cur_ptr == 2)		start_ptr = LOOP_BUF_SIZE - 1;
	else if(cur_ptr == 1)	start_ptr = LOOP_BUF_SIZE - 2;
	else if(cur_ptr == 0)	start_ptr = LOOP_BUF_SIZE - 3;
	else					start_ptr = cur_ptr - 3;

	if((buf[start_ptr] == buf[(start_ptr+1)%LOOP_BUF_SIZE])						&&
			(buf[(start_ptr+1)%LOOP_BUF_SIZE] == buf[(start_ptr+2)%LOOP_BUF_SIZE])	&&
			(buf[(start_ptr+2)%LOOP_BUF_SIZE] == buf[(start_ptr+3)%LOOP_BUF_SIZE]))
		return buf[start_ptr];
	else
		return 0xFF;
}

static unsigned char nvp6124_decide_noptn_stage(unsigned char agcval)
{
	unsigned char stage;

	if	   (agcval <= 0x18)	stage = 1;
	else if(agcval <= 0x1A)	stage = 2;
	else if(agcval <= 0x1C)	stage = 3;
	else if(agcval <= 0x1E)	stage = 4;
	else if(agcval <= 0x20)	stage = 5;
	else					stage = 6;

	return stage;
}

void nvp6124_init_eq_stage(AHD_INFO *pvInfo)
{
	unsigned char default_stage=0;

	if(pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25 )			//720p25, 30	
	{
		nvp6124_i2c_write(0xFF, BANK5);
		nvp6124_i2c_write(0x58, ANALOG_EQ_720P[default_stage]);
		nvp6124_i2c_write(0xFF, BANKA);         
		nvp6124_i2c_write(0x3B, DIGITAL_EQ_720P[default_stage]);
	}
	else
	{
		nvp6124_i2c_write(0xFF, BANK5);
		nvp6124_i2c_write(0x58, ANALOG_EQ_1080P[default_stage]);
		nvp6124_i2c_write(0xFF, BANKA);         
		nvp6124_i2c_write(0x3B, DIGITAL_EQ_1080P[default_stage]);
	}
	memset(check_y_stage, 0x00, sizeof(check_y_stage));
}

void nvp6124_set_equalizer(AHD_INFO *pvInfo)
{
	unsigned char i,tmp_acp_val,tmp_acp_ptn,tmp_y_stage,tmp_c_stage;
	unsigned char ch, vidmode, agc_lock, stage_gap, vloss, temp;
	unsigned char  agc_val, acc_gain_sts, y_ref_status, y_ref2_status;

	nvp6124_i2c_write(0xFF, BANK9);
	nvp6124_i2c_write(0x61, 0x00);			//data : ch%4

	nvp6124_i2c_write(0xFF, BANK0);
	nvp6124_i2c_read(0xF7, &agc_val);

	nvp6124_i2c_write(0xFF, BANK0);
	nvp6124_i2c_read(0xD0, &vidmode);

	nvp6124_i2c_read(0xEC, &agc_lock);
	nvp6124_i2c_read(0xEC, &vloss);

	nvp6124_i2c_write(0xFF, BANK5);
	nvp6124_i2c_read(0xE2, &acc_gain_sts);
	acc_gain_sts = (acc_gain_sts & 0x07)<<8;
	nvp6124_i2c_read(0xE3, &temp);
	acc_gain_sts |= temp;

	nvp6124_i2c_read(0xEA, &y_ref_status) & 0x07;
	y_ref_status = (y_ref_status &0x07)<< 8;
	nvp6124_i2c_read(0xEB, &temp);
	y_ref_status |= temp;

	nvp6124_i2c_read(0xE8, &y_ref2_status) & 0x07;
	y_ref2_status = (y_ref2_status &0x07)<< 8;
	nvp6124_i2c_read(0xEB, &temp);
	y_ref2_status |= temp;

	nvp6124_i2c_read(0xE8, &y_ref2_status) & 0x07;
	
	if(vidmode >= AHDRX_OUT_1280X720P30)
	{
		if((((vloss)&0x01) == 0x00) && (((agc_lock)&0x01) == 0x01))
			video_on = 1;
		else
			video_on = 0;
	}
	else
	{
		video_on = 0;
	}

	if(pvInfo->rx.current_resolution >= AHDRX_OUT_1280X720P30)	
	{
		if(video_on)
		{
			if(loop_cnt != 0xFF)
			{
				loop_cnt++;
				if(loop_cnt > 200) loop_cnt = ACP_READ_START_CNT;

				if(loop_cnt == ACP_CLR_CNT)		nvp6124_acp_rx_clear(); 
				else							nvp6124_acp_each_setting(pvInfo); 

				eq_loop_cnt++;
				eq_loop_cnt %= LOOP_BUF_SIZE;

				check_c_stage[eq_loop_cnt] = nvp6124_get_ceq_stage(pvInfo, acc_gain_sts);
				check_y_stage[eq_loop_cnt] = nvp6124_get_yeq_stage(pvInfo, y_ref_status);
				acp_val[eq_loop_cnt] = nvp6124_read_acp_status();
				acp_ptn[eq_loop_cnt] = nvp6124_read_acp_pattern();
			}

			if(pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25 )			//720p25, 30 	
			{
				if((loop_cnt != 0xFF) && (loop_cnt >= ACP_READ_START_CNT))
				{
					if(one_setting == 0)
					{
						bypass_flag = 0xFF;
						tmp_acp_val = nvp6124_compare_arr(eq_loop_cnt, acp_val);
						tmp_acp_ptn = nvp6124_compare_arr(eq_loop_cnt, acp_ptn);

						xprintf("acp_val = %x   acp_ptn = %x\n", acp_val[eq_loop_cnt], acp_ptn[eq_loop_cnt]);
						xprintf("tmp_acp_val = %x   tmp_acp_ptn = %x\n", tmp_acp_val, tmp_acp_ptn);

						if((tmp_acp_val == 0x55) && (tmp_acp_ptn == 0x01))
						{
							bypass_flag = 0;
							one_setting = 1;
						}
						else
						{
							bypass_flag_retry++;
							bypass_flag_retry %= 200;
							if(bypass_flag_retry == RETRY_CNT)
							{
								bypass_flag = 1;
								one_setting = 1;
							}
						}
					}

					if(bypass_flag == 1)
					{
						stage_update= 1;
					}
					else if(bypass_flag == 0)
					{
						if(ch == 0)
						{
							xprintf("loop_cnt = %d\n",loop_cnt);
							xprintf("check_y_stage = %d \n",check_y_stage[eq_loop_cnt]);
							xprintf("check_c_stage = %d \n",check_c_stage[eq_loop_cnt]);
						}

						if( loop_cnt == (ACP_READ_START_CNT+bypass_flag_retry))
						{
							nvp6124_init_eq_stage(pvInfo); 
							xprintf("720p init_eq_stage\n");
						}
						else if(loop_cnt >= (ACP_READ_START_CNT+bypass_flag_retry+3))
						{
							tmp_y_stage = nvp6124_compare_arr(eq_loop_cnt,check_y_stage);
							tmp_c_stage = nvp6124_compare_arr(eq_loop_cnt,check_c_stage);
							xprintf("tmp_y_stage = %d\n",tmp_y_stage);
							xprintf("tmp_c_stage = %d\n",tmp_c_stage);

							if((tmp_c_stage != 0xFF) && (tmp_y_stage != 0xFF))
								stage_update = 1;
							else
							{
								if(tmp_y_stage == 0xFF)
								{
									ystage_flag_retry++;
									ystage_flag_retry %= 200;
								}
								if(tmp_c_stage == 0xFF)
								{
									cstage_flag_retry++;
									cstage_flag_retry %= 200;
								}

								if((ystage_flag_retry==RETRY_CNT)||(cstage_flag_retry==RETRY_CNT))	
								{
									stage_update = 1;
									check_y_stage[eq_loop_cnt] = nvp6124_calc_arr_mean(check_y_stage);
								}

								stage_gap = abs(nvp6124_decide_noptn_stage(agc_val) - check_y_stage[eq_loop_cnt]);
								xprintf("agc_val = %d y_stage = %d\n", nvp6124_decide_noptn_stage(agc_val),check_y_stage[eq_loop_cnt]);
								if(stage_gap >= 3)
									xprintf("GAP IS TOO BIG\n");

								if(( nvp6124_calc_stage_gap(check_y_stage) >= 2) || (stage_gap >= 3))
								{
									nvp6124_init_eq_stage(pvInfo); 
									nvp6124_sw_reset();
								}
							}
						}
					}
					else
					{
						stage_update = 0;
					}

					if(bypass_flag == 1)
					{
						check_y_stage[eq_loop_cnt] = 7;
						if(((check_c_stage[eq_loop_cnt] == 5)   && y_ref2_status >= 0x1A0) ||
								(acc_gain_sts == 0x7FF && y_ref2_status >= 0x1B0))
							check_y_stage[eq_loop_cnt] = 8;
					}
				}
			}
			else
			{
				xprintf("1080P acp_val=%x  acp_ptn=%x\n", acp_val[eq_loop_cnt],acp_ptn[eq_loop_cnt]);
				if((loop_cnt != 0xFF) && (loop_cnt >= ACP_READ_START_CNT))
				{
					if(one_setting == 0)
					{
						bypass_flag = 0xFF;
						tmp_acp_val = nvp6124_compare_arr(eq_loop_cnt, acp_val);
						tmp_acp_ptn = nvp6124_compare_arr(eq_loop_cnt, acp_ptn);

						if((tmp_acp_val == 0x55) && (tmp_acp_ptn == 0x01))
						{
							bypass_flag = 0;
							one_setting = 1;
						}
						else
						{
							bypass_flag_retry++;
							bypass_flag_retry %= 200;
							if(bypass_flag_retry == RETRY_CNT)
							{
								bypass_flag = 1;
								one_setting = 1;
								nvp6124_lossfakechannel(bypass_flag);
							}
						}
					}

					if(bypass_flag == 1)
					{
						stage_update = 1;
					}
					else if(bypass_flag == 0)
					{
						xprintf("1080p loop_cnt = %d\n",loop_cnt);
						xprintf("1080p check_y_stage = %d \n",check_y_stage[eq_loop_cnt]);
						xprintf("1080p check_c_stage = %d \n",check_c_stage[eq_loop_cnt]);

						if( loop_cnt == (ACP_READ_START_CNT+bypass_flag_retry))
						{
							nvp6124_init_eq_stage(pvInfo); 
							xprintf("1080p init_eq_stage\n");
						}
						else if(loop_cnt >= (ACP_READ_START_CNT+bypass_flag_retry+3))
						{
							tmp_y_stage = nvp6124_compare_arr(eq_loop_cnt, check_y_stage);
							if(tmp_y_stage != 0xFF)
							{
								stage_update = 1;
							}
							else
							{
								if(tmp_y_stage == 0xFF)
								{
									ystage_flag_retry++;
									ystage_flag_retry %= 200;
								}

								if(ystage_flag_retry == RETRY_CNT)
								{
									stage_update= 1;
									check_y_stage[eq_loop_cnt] = nvp6124_calc_arr_mean(check_y_stage);
								}

								stage_gap = abs( nvp6124_decide_noptn_stage(agc_val) - check_y_stage[eq_loop_cnt]);
								xprintf("1080p agc_val=%d y_stage=%d\n", nvp6124_decide_noptn_stage(agc_val),check_y_stage[eq_loop_cnt]);
								if(stage_gap >= 3)
									xprintf("GAP IS TOO BIG\n");

								if(( nvp6124_calc_stage_gap(check_y_stage) >= 2) || (stage_gap >= 3))
								{
									nvp6124_init_eq_stage(pvInfo); 
									nvp6124_sw_reset();
								}
							}
						}
					}
				}
			}
		}
		else
		{
			if(vidmode_back >= 0x04) 
			{
				stage_update = 1;
			}
			else
				stage_update = 0;

			for(i=0;i<LOOP_BUF_SIZE;i++)
			{
				check_c_stage[i]=0;
				check_y_stage[i]=0;
				acp_val[i] = 0;
				acp_ptn[i] = 0;
			}
			eq_loop_cnt=0;
			check_y_stage[eq_loop_cnt] = 1; 
			loop_cnt =0;
			bypass_flag_retry=0;
			ystage_flag_retry=0;
			cstage_flag_retry=0;
			one_setting = 0;
			acp_isp_wr_en = 0;
			nvp6124_lossfakechannel(0);
		}

		if(stage_update)
		{
			stage_update = 0;
			acp_isp_wr_en = 1;
			eq_stage = check_y_stage[eq_loop_cnt];

			if(video_on) loop_cnt = 0xFF;

			if(pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P30 || pvInfo->rx.current_resolution == AHDRX_OUT_1280X720P25)		//720p25, 30
			{
				nvp6124_i2c_write(0xFF, BANK5);
				nvp6124_i2c_write(0x58, ANALOG_EQ_720P[eq_stage]);

				nvp6124_i2c_write(0xFF, BANKA);
				nvp6124_i2c_write(0x3B, DIGITAL_EQ_720P[eq_stage]);
			}
			else
			{
				nvp6124_i2c_write(0xFF, BANK5);
				nvp6124_i2c_write(0x58, ANALOG_EQ_1080P[eq_stage]);
				nvp6124_i2c_write(0xFF, BANKA);
				nvp6124_i2c_write(0x3B, DIGITAL_EQ_1080P[eq_stage]);
			}
			xprintf("Stage update : eq_stage = %d\n", eq_stage);
			
			if(eq_stage != 7)
			{
#ifdef _EQ_ADJ_COLOR_
				nvp6124_brightness_eq(pvInfo, eq_stage);
				nvp6124_contrast_eq(pvInfo, eq_stage);
				nvp6124_saturation_eq(pvInfo, eq_stage);
#endif
				nvp6124_sharpness_eq(pvInfo, eq_stage);
				nvp6124_peaking_eq(pvInfo, eq_stage);
				nvp6124_ctigain_eq(pvInfo, eq_stage);
				nvp6124_c_filter_eq(pvInfo, eq_stage);
				nvp6124_ugain_eq(pvInfo, eq_stage);
				nvp6124_vgain_eq(pvInfo, eq_stage);
			}
		}
		vidmode_back = vidmode;
	}
}
