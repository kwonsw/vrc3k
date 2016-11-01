
#ifndef __XUART_H__
#define __XUART_H__

#include "uart.h"
#include "xprintf.h"

#define xuart_init(baud) \
				uart_init(baud); \
				xdev_out(uart_putc); \
				xdev_in(uart_getc)

#endif /* __XUART_H__ */
