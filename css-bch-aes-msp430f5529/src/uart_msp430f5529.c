/*
 * uart_msp430f5529.c
 *
 *  Created on: 18 jun. 2019
 *      Author: Rommel
 */
#include <msp430.h>
#include "uart_msp430f5529.h"

void uart_send_byte(unsigned char symbol) {
    while (!(UCA1IFG&UCTXIFG));
    UCA1TXBUF = symbol;
}

void uart_handshake() {
    while (uart_receive_byte() != '1');
    uart_send_byte('0');
}

unsigned char uart_receive_byte() {
    while (!(UCA1IFG&UCRXIFG));
    return UCA1RXBUF;
}

/* Función para configuración UART a 115200 a 25 MHz */
void uart_init(){
    P4SEL |= BIT5+BIT4;                       // P4.5,4.4 = USCI_A1 TXD/RXD
    UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCA1CTL1 |= UCSSEL_2;                     // SMCLK
    UCA1BR0 = 9;                              // 1MHz 115200 (see User's Guide)
    UCA1BR1 = 0;                              // 1MHz 115200
    UCA1MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
    UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
}
