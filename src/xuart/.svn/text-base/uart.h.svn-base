
#ifndef __UART_H__
#define __UART_H__

#define	UART_BUFF_SIZE	128	/* (power of 2) */

#define UART1			USART1
#define UART1_IRQn		USART1_IRQn
#define UART1_GPIO		GPIOA
#define UART1_RxPin		GPIO_Pin_10
#define UART1_TxPin		GPIO_Pin_9

#define UART1_ISR		USART1_IRQHandler

void uart_init(uint32_t baud);
void uart_putc(uint8_t c);
void uart_puts(uint8_t *s);
uint8_t uart_isrx(void);
uint8_t uart_getc(void);

#endif /* __UART_H__ */
