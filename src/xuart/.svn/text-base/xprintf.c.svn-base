/*------------------------------------------------------------------------/
  /  Universal string handler for user console interface
  /-------------------------------------------------------------------------/
  /
  /  Copyright (C) 2011, ChaN, all right reserved.
  /
  / * This software is a free software and there is NO WARRANTY.
  / * No restriction on use. You can use, modify and redistribute it for
  /   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
  / * Redistributions of source code must retain the above copyright notice.
  /
  /-------------------------------------------------------------------------*/

#include "FreeRTOS.h"
#include "xprintf.h"
#include "mdin3xx.h"
#include "i2c.h"

#if _USE_XFUNC_OUT
#include <stdarg.h>
#include <string.h>

void (*xfunc_out)(unsigned char);	/* Pointer to the output stream */
static char *outptr;
unsigned char *p_reg;

unsigned char xValue_Change(unsigned char *p_addr);

/*----------------------------------------------*/
/* Put a character                              */
/*----------------------------------------------*/

void xputc (char c)
{
	if (_CR_CRLF && c == '\n') xputc('\r');		/* CR -> CRLF */

	if (outptr) {
		*outptr++ = (unsigned char)c;
		return;
	}

	if (xfunc_out) xfunc_out((unsigned char)c);
}



/*----------------------------------------------*/
/* Put a null-terminated string                 */
/*----------------------------------------------*/

void xputs (					/* Put a string to the default device */
		const char* str				/* Pointer to the string */
		)
{
	while (*str)
		xputc(*str++);
}


void xfputs (					/* Put a string to the specified device */
		void(*func)(unsigned char),	/* Pointer to the output function */
		const char*	str				/* Pointer to the string */
		)
{
	void (*pf)(unsigned char);


	pf = xfunc_out;		/* Save current output device */
	xfunc_out = func;	/* Switch output to specified device */
	while (*str)		/* Put the string */
		xputc(*str++);
	xfunc_out = pf;		/* Restore output device */
}



/*----------------------------------------------*/
/* Formatted string output                      */
/*----------------------------------------------*/
/*  xprintf("%d", 1234);			"1234"
	xprintf("%6d,%3d%%", -200, 5);	"  -200,  5%"
	xprintf("%-6u", 100);			"100   "
	xprintf("%ld", 12345678L);		"12345678"
	xprintf("%04x", 0xA3);			"00a3"
	xprintf("%08LX", 0x123ABC);		"00123ABC"
	xprintf("%016b", 0x550F);		"0101010100001111"
	xprintf("%s", "String");		"String"
	xprintf("%-4s", "abc");			"abc "
	xprintf("%4s", "abc");			" abc"
	xprintf("%c", 'a');				"a"
	xprintf("%f", 10.0);            <xprintf lacks floating point support>
	*/

static
void xvprintf (
		const char*	fmt,	/* Pointer to the format string */
		va_list arp			/* Pointer to arguments */
		)
{
	unsigned int r, i, j, w, f;
	unsigned long v;
	char s[16], c, d, *p;


	for (;;) {
		c = *fmt++;					/* Get a char */
		if (!c) break;				/* End of format? */
		if (c != '%') {				/* Pass through it if not a % sequense */
			xputc(c); continue;
		}
		f = 0;
		c = *fmt++;					/* Get first char of the sequense */
		if (c == '0') {				/* Flag: '0' padded */
			f = 1; c = *fmt++;
		} else {
			if (c == '-') {			/* Flag: left justified */
				f = 2; c = *fmt++;
			}
		}
		for (w = 0; c >= '0' && c <= '9'; c = *fmt++)	/* Minimum width */
			w = w * 10 + c - '0';
		if (c == 'l' || c == 'L') {	/* Prefix: Size is long int */
			f |= 4; c = *fmt++;
		}
		if (!c) break;				/* End of format? */
		d = c;
		if (d >= 'a') d -= 0x20;
		switch (d) {				/* Type is... */
			case 'S' :					/* String */
				p = va_arg(arp, char*);
				for (j = 0; p[j]; j++) ;
				while (!(f & 2) && j++ < w) xputc(' ');
				xputs(p);
				while (j++ < w) xputc(' ');
				continue;
			case 'C' :					/* Character */
				xputc((char)va_arg(arp, int)); continue;
			case 'B' :					/* Binary */
				r = 2; break;
			case 'O' :					/* Octal */
				r = 8; break;
			case 'D' :					/* Signed decimal */
			case 'U' :					/* Unsigned decimal */
				r = 10; break;
			case 'X' :					/* Hexdecimal */
				r = 16; break;
			default:					/* Unknown type (passthrough) */
				xputc(c); continue;
		}

		/* Get an argument and put it in numeral */
		v = (f & 4) ? va_arg(arp, long) : ((d == 'D') ? (long)va_arg(arp, int) : (long)va_arg(arp, unsigned int));
		if (d == 'D' && (v & 0x80000000)) {
			v = 0 - v;
			f |= 8;
		}
		i = 0;
		do {
			d = (char)(v % r); v /= r;
			if (d > 9) d += (c == 'x') ? 0x27 : 0x07;
			s[i++] = d + '0';
		} while (v && i < sizeof(s));
		if (f & 8) s[i++] = '-';
		j = i; d = (f & 1) ? '0' : ' ';
		while (!(f & 2) && j++ < w) xputc(d);
		do xputc(s[--i]); while(i);
		while (j++ < w) xputc(' ');
	}
}


void xprintf (			/* Put a formatted string to the default device */
		const char*	fmt,	/* Pointer to the format string */
		...					/* Optional arguments */
		)
{
	va_list arp;

	va_start(arp, fmt);
	xvprintf(fmt, arp);
	va_end(arp);
}


void xsprintf (			/* Put a formatted string to the memory */
		char* buff,			/* Pointer to the output buffer */
		const char*	fmt,	/* Pointer to the format string */
		...					/* Optional arguments */
		)
{
	va_list arp;


	outptr = buff;		/* Switch destination for memory */

	va_start(arp, fmt);
	xvprintf(fmt, arp);
	va_end(arp);

	*outptr = 0;		/* Terminate output string with a \0 */
	outptr = 0;			/* Switch destination for device */
}


void xfprintf (					/* Put a formatted string to the specified device */
		void(*func)(unsigned char),	/* Pointer to the output function */
		const char*	fmt,			/* Pointer to the format string */
		...							/* Optional arguments */
		)
{
	va_list arp;
	void (*pf)(unsigned char);


	pf = xfunc_out;		/* Save current output device */
	xfunc_out = func;	/* Switch output to specified device */

	va_start(arp, fmt);
	xvprintf(fmt, arp);
	va_end(arp);

	xfunc_out = pf;		/* Restore output device */
}



/*----------------------------------------------*/
/* Dump a line of binary dump                   */
/*----------------------------------------------*/

void put_dump (
		const void* buff,		/* Pointer to the array to be dumped */
		unsigned long addr,		/* Heading address value */
		int len,				/* Number of items to be dumped */
		int width				/* Size of the items (DF_CHAR, DF_SHORT, DF_LONG) */
		)
{
	int i;
	const unsigned char *bp;
	const unsigned short *sp;
	const unsigned long *lp;


	xprintf("%08lX ", addr);		/* address */

	switch (width) {
		case DW_CHAR:
			bp = buff;
			for (i = 0; i < len; i++)		/* Hexdecimal dump */
				xprintf(" %02X", bp[i]);
			xputc(' ');
			for (i = 0; i < len; i++)		/* ASCII dump */
				xputc((bp[i] >= ' ' && bp[i] <= '~') ? bp[i] : '.');
			break;
		case DW_SHORT:
			sp = buff;
			do								/* Hexdecimal dump */
				xprintf(" %04X", *sp++);
			while (--len);
			break;
		case DW_LONG:
			lp = buff;
			do								/* Hexdecimal dump */
				xprintf(" %08LX", *lp++);
			while (--len);
			break;
	}

	xputc('\n');
}

#endif /* _USE_XFUNC_OUT */



#if _USE_XFUNC_IN
unsigned char (*xfunc_in)(void);	/* Pointer to the input stream */

/*----------------------------------------------*/
/* Get a line from the input                    */
/*----------------------------------------------*/

int xgets (		/* 0:End of stream, 1:A line arrived */
		char* buff,	/* Pointer to the buffer */
		int len		/* Buffer length */
		)
{
	int c, i;
	unsigned char temp_addr, temp_bank, write_value, read_value = 0;
	unsigned short addr, page, value = 0;


	if (!xfunc_in)	return 0;		/* No input function specified */

	i = 0;
	
	for (;;) {
		c = xfunc_in();				/* Get a char from the incoming stream */
		if (!c)	return 0;			/* End of stream? */
		if (c == '\r')	break;		/* End of line? */
		if (c == '\b' && i) {		/* Back space? */
			i--;
			if (_LINE_ECHO) xputc(c);
			continue;
		}
		if (c >= ' ' && i < len - 1) {	/* Visible chars */
			buff[i++] = c;
			if (_LINE_ECHO)
			{
				xputc(c);
			}
		}
	}

	buff[i] = '\0';	/* Terminate with a \0 */

#if 0
	p_reg = strstr(buff,"reg");

	if(p_reg == NULL){}
	else{
		vTaskSuspendAll();

		p_reg += 4;

		if(!strncmp(p_reg, "dump", 4)){
			p_reg += 5;
			if(!strncmp(p_reg,"nvp6124", 7)) 		{ nvp6124_i2c_dump(); 	} 
			else if(!strncmp(p_reg,"nvp6021", 7)) 	{ nvp6021_i2c_dump();	}
			else if(!strncmp(p_reg,"adv7611", 7)) 	{ 						}
			else if(!strncmp(p_reg,"mdin380", 7)) 	{ mdin3xx_i2c_dump(); 	}
			else									{						}	
		}else if(!strncmp(p_reg, "nvp6124", 7))		{
			p_reg += 8;
				
			temp_bank = xValue_Change(p_reg);	// htoul(p_reg, 16, NULL);
			temp_addr = xValue_Change(p_reg+1)<<4 | xValue_Change(p_reg+2); 
			
			if(*(p_reg+3) == 0x20 ){
				write_value = xValue_Change(p_reg+4)<<4 | xValue_Change(p_reg+5); 
				
				nvp6124_i2c_write(0xff, temp_bank);
				nvp6124_i2c_write(temp_addr, write_value);
			
				nvp6124_i2c_read(temp_addr, &read_value);
				xprintf("\nWrite: bank: %02x read addr: %02x value: %02x \n", temp_bank, temp_addr, read_value);
			}else if(*(p_reg+3) == '\0'){
				nvp6124_i2c_write(0xff, temp_bank);
				nvp6124_i2c_read(temp_addr, &read_value);
				xprintf("\nRead: bank: %02x read addr: %02x value: %02x \n", temp_bank, temp_addr, read_value);
			}
		}else if(!strncmp(p_reg, "nvp6021", 7))		{
			p_reg += 8;
				
			temp_bank = xValue_Change(p_reg);	
			temp_addr = xValue_Change(p_reg+1)<<4 | xValue_Change(p_reg+2); 
			
			if(*(p_reg+3) ==  0x20 ){
				write_value = xValue_Change(p_reg+4)<<4 | xValue_Change(p_reg+5); 
				
				nvp6021_i2c_write(0xff, temp_bank);
				nvp6021_i2c_write(temp_addr, write_value);
				
				nvp6021_i2c_read(temp_addr, &read_value);
				xprintf("\nWrite bank: %02x read addr: %02x value: %02x \n", temp_bank, temp_addr, read_value);
			}else if(*(p_reg+3) == '\0'){
				nvp6021_i2c_write(0xff, temp_bank);
				nvp6021_i2c_read(temp_addr, &read_value);
				xprintf("\nRead bank: %02x read addr: %02x value: %02x \n", temp_bank, temp_addr, read_value);
			}
		}else if(!strncmp(p_reg, "mdin380", 7))	{
				p_reg += 8;

				page = xValue_Change(p_reg);	
				addr = xValue_Change(p_reg+1)<<12 | xValue_Change(p_reg+2)<<8 | xValue_Change(p_reg+3)<<4 | xValue_Change(p_reg+4); 

				if(*(p_reg+5) == 0x20){
					value = xValue_Change(p_reg+6)<<12 | xValue_Change(p_reg+7)<<8 | xValue_Change(p_reg+8)<<4 | xValue_Change(p_reg+9); 
			
					if(page == 0x0){		
						MDINI2C_RegWrite(0xc0, addr, value );
						MDINI2C_RegRead(0xc0, addr, &value);
					}else if(page == 0x1){
						MDINI2C_RegWrite(0xc2, addr, value );
						MDINI2C_RegRead(0xc2, addr, &value);
					}
					else if(page == 0x2){
						MDINI2C_RegWrite(0xc4, addr, value );
						MDINI2C_RegRead(0xc4, addr, &value);
					}else {
					}
		
					xprintf("\nWrite page: %02x addr: %04x value: %04x\n", page, addr, value);	
				}else if(*(p_reg+5) == '\0'){
			
					if(page == 0x0)			MDINI2C_RegRead(0xc0, addr, &value);	
					else if	(page == 0x1)	MDINI2C_RegRead(0xc2, addr, &value);
					else if	(page == 0x2)	MDINI2C_RegRead(0xc4, addr, &value);
					else {
					}
					
					xprintf("\nRead page: %02x addr: %04x value: %04x\n", page, addr, value);	
				}
		}else if(!strncmp(p_reg, "adv7611", 7))		{
		}else	{
			xprintf("Wrong Command \n");
		}
		p_reg = NULL;
		
		xTaskResumeAll ();
	}

	if (_LINE_ECHO)
	{
		xputc('\n');
		xprintf("PDR>");
	}
#else
	if (_LINE_ECHO) xputc('\n');
#endif

	return 1;
}

unsigned char xValue_Change(unsigned char *p_addr)
{

	if(*p_addr > 0x2f && *p_addr < 0x3a)	*p_addr -= '0';
	else									*p_addr -= 'W';

	return *p_addr;
}



int xfgets (	/* 0:End of stream, 1:A line arrived */
		unsigned char (*func)(void),	/* Pointer to the input stream function */
		char* buff,	/* Pointer to the buffer */
		int len		/* Buffer length */
		)
{
	unsigned char (*pf)(void);
	int n;


	pf = xfunc_in;			/* Save current input device */
	xfunc_in = func;		/* Switch input to specified device */
	n = xgets(buff, len);	/* Get a line */
	xfunc_in = pf;			/* Restore input device */

	return n;
}


/*----------------------------------------------*/
/* Get a value of the string                    */
/*----------------------------------------------*/
/*	"123 -5   0x3ff 0b1111 0377  w "
	^                           1st call returns 123 and next ptr
	^                        2nd call returns -5 and next ptr
	^                3rd call returns 1023 and next ptr
	^         4th call returns 15 and next ptr
	^    5th call returns 255 and next ptr
	^ 6th call fails and returns 0
	*/

int xatoi (			/* 0:Failed, 1:Successful */
		char **str,		/* Pointer to pointer to the string */
		long *res		/* Pointer to the valiable to store the value */
		)
{
	unsigned long val;
	unsigned char c, r, s = 0;


	*res = 0;

	while ((c = **str) == ' ') (*str)++;	/* Skip leading spaces */

	if (c == '-') {		/* negative? */
		s = 1;
		c = *(++(*str));
	}

	if (c == '0') {
		c = *(++(*str));
		switch (c) {
			case 'x':		/* hexdecimal */
				r = 16; c = *(++(*str));
				break;
			case 'b':		/* binary */
				r = 2; c = *(++(*str));
				break;
			default:
				if (c <= ' ') return 1;	/* single zero */
				if (c < '0' || c > '9') return 0;	/* invalid char */
				r = 8;		/* octal */
		}
	} else {
		if (c < '0' || c > '9') return 0;	/* EOL or invalid char */
		r = 10;			/* decimal */
	}

	val = 0;
	while (c > ' ') {
		if (c >= 'a') c -= 0x20;
		c -= '0';
		if (c >= 17) {
			c -= 7;
			if (c <= 9) return 0;	/* invalid char */
		}
		if (c >= r) return 0;		/* invalid char for current radix */
		val = val * r + c;
		c = *(++(*str));
	}
	if (s) val = 0 - val;			/* apply sign if needed */

	*res = val;
	return 1;
}

#endif /* _USE_XFUNC_IN */
