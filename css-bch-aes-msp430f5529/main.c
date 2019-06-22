//***************************************************************************************
// Maestra en Sistemas Inteligentes Multimedia
// Asignatura: Lenguaje ensamblador
//
// Alumno:
// Fecha:
//

/* algorithms implemented */
#include "bch_31_16_3.h"
#include "bch_31_21_2.h"
#include "cipher.h"
#include "memory_local.h"
#include "TI_aes_128.h"
#include "aes_256.h"
/* standard libraries */
#include <msp430.h>
#include <stdio.h>
#include <string.h>

/***************************************************************************************/
unsigned char rx_operation = 0x00;
unsigned char tx_operation = 0x00;
unsigned char flash_key_operation = 0x00;

unsigned char index_dmabuff = 0;
unsigned char index_keybuff = 0;

unsigned char op_write_to_flash = 0;
unsigned char op_write_key_to_flash = 0;
/***************************************************************************************/
// Principal
int main(void) {
    unsigned int i;
    __bis_SR_register(GIE);
    uart_init();
    while (1)
    {
        while (NO_OPERATION == rx_operation && NO_OPERATION == op_write_to_flash
                && NO_OPERATION == op_write_key_to_flash);
        if (OPERATION == rx_operation)
        {
            read_from_flash();
            LFSRs(0x0001, 0x0001);
            bch_buffer_decoder();
            aes_enc_dec_buffer(1);
            for (i = 0; i < BUFFER_TMP_SIZE; i++)
            {
                printf(" %d\r\n", buffer_tmp[i]);
            }
            rx_operation = NO_OPERATION;
        }
        else if (OPERATION == op_write_to_flash)
        {
            aes_enc_dec_buffer(0);
            LFSRs(0x0001, 0x0001);
            bch_buffer_encoder();
            write_to_flash();
            op_write_to_flash = NO_OPERATION;
        }
        else if (OPERATION == op_write_key_to_flash)
        {
            write_key_to_flash();
            op_write_key_to_flash = NO_OPERATION;
        }
    }
}
//***************************************************************************************
//***************************************************************************************
// Interrupcion por el mxdulo USCI
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    switch (__even_in_range(UCA1IV, 4))
    {
    case 0:
        break;                             // Vector 0 - no interrupt
    case 2:                               // Vector 2 - RXIFG
        if (NO_OPERATION == tx_operation && NO_OPERATION == rx_operation
                && NO_OPERATION == flash_key_operation)
        {
            switch (UCA1RXBUF)
            {
            case '1':

                break;
            case '0':

                break;
            case 'R':
                rx_operation = 0x01;
                break;
            case 'T':
                tx_operation = 0x01;
                break;
            case 'K':
                flash_key_operation = 0x01;
                break;
            default:
                break;
            }
        }
        else if (OPERATION == tx_operation)
        {
            buffer_tmp[index_dmabuff++] = UCA1RXBUF;
            if (index_dmabuff >= BUFFER_TMP_SIZE)
            {
                index_dmabuff = 0;
                tx_operation = NO_OPERATION;
                op_write_to_flash = 1;
            }
        }
        else if (OPERATION == flash_key_operation)
        {
            key_1[index_keybuff++] = UCA1RXBUF;
            if (index_keybuff >= KEY_BUFFER_SIZE)
            {
                index_keybuff = 0;
                flash_key_operation = NO_OPERATION;
                op_write_key_to_flash = 1;
            }
        }
        break;
    case 4:
        break;                             // Vector 4 - TXIFG
    default:
        break;
    }
}
