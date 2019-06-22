/*
 * uart_msp430f5529.h
 *
 *  Created on: 18 jun. 2019
 *      Author: Rommel
 */

#ifndef INCLUDE_UART_MSP430F5529_H_
#define INCLUDE_UART_MSP430F5529_H_

void uart_init();                            //Función para configur
void uart_send_byte(unsigned char symbol);
unsigned char uart_receive_byte();
void uart_handshake();

#endif /* INCLUDE_UART_MSP430F5529_H_ */
