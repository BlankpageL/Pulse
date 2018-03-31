/*
 * uart.h
 *
 *  Created on: 2017-3-5
 *      Author: redchenjs
 */

#ifndef UART_H_
#define UART_H_

extern void uart_init(void);

extern unsigned char uart_transmit_frame(unsigned char *p_buff, unsigned int num);

#endif /* UART_H_ */
