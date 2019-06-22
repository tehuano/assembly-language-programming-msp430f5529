//***************************************************************************************
// Maestrxa en Sistemas Inteligentes Multimedia
// Asignatura: Lenguaje ensamblador
//
// Alumno:
// Fecha:
//
// Descripcixn del Cxdigo:
// El cxdigo siguiente espera por UART la letra 'R' para enviar
// por via UART a 115200 200 datos de un LFSR de 13 bits.
// Ademxs, si el MSP recibe un '0' apaga el LED 1,
// si recibe el '1' entonces enciende el LED 1
//
//    Hasta ahora el cxdigo permite trabajar con
//    los pines mostrados en la figura.
//
//    ACLK = REFO = 32kHz,     MCLK = SMCLK = 25MHz
//    UART a 115200 baudios
//
//          /|\   MSP430F5529
//           | ----------------------
//           | |                    |
//           --|RST                 |
//             |          (Led1)P1.0|-->LED 1 :
//      /|\    |          (Led2)P4.7|-->LED 2 :Led que siempre parpadea
//       ---o--|P1.1(Boton1)        |
//      /|\    |                    |
//       ---o--|P2.1(Boton2)        |
//             |                    |
//             |                    |
//       Vin-->|P6.0/A0             |
//             |          P2.0/TA1.1|--> CCR1 - Salida PWM
//             |                    |
//  RX(UART)-->|P4.5            P4.4|--> TX(UART)
//
//
//***************************************************************************************
//***************************************************************************************
// Librerias
#include <bch_31_16_3.h>
#include <msp430.h>
#include <stdio.h>
#include <string.h>

/* project libraries */
#include "TI_aes_128.h"
#include "memory_local.h"
#include "cipher.h"

//***************************************************************************************
//***************************************************************************************
// Definiciones
#define Led1On      P1OUT |=  BIT0
#define Led1Off     P1OUT &=  ~BIT0;

#define NO_OPERATION -1
//***************************************************************************************
//***************************************************************************************
// Variables

unsigned int Aleatorio;
unsigned short bandera = 0;
int fputc(int _c, register FILE *_fp);
int fputs(const char *_ptr, register FILE *_fp);
//***************************************************************************************
//***************************************************************************************
// Funciones
void configuracion();                         //Funcixn general de configuracixn
void SetVcoreUp(unsigned int level);          //Funcixn para asegurar los 25 MHz
void Mydelay(unsigned int multiplo, unsigned int carga);   //Funcion de retardos
void configTA0();                      //Funcixn de configuracixn para el Timer0
void configadc12();                         //Funcixn de configuracixn del ADC12
void configPWM();                                  //Funcixn para configurar PWM
void configUART();                    //Funcixn para configuracixn UART pos USCI
//***************************************************************************************
//***************************************************************************************
unsigned char rx_operation = 0x00;
unsigned char tx_operation = 0x00;
unsigned char flash_key_operation = 0x00;

unsigned char index_dmabuff = 0;
unsigned char index_keybuff = 0;

unsigned char write_to_flash = 0;
unsigned char write_key_to_flash = 0;
//***************************************************************************************
//***************************************************************************************
// Principal
int main(void)
{
    volatile unsigned int i;
    configuracion();
    configUART();
    configTA0();
    configadc12();
    configPWM();
    __bis_SR_register(GIE);
    while (1)
    {
        while (NO_OPERATION == rx_operation && NO_OPERATION == write_to_flash
                && NO_OPERATION == write_key_to_flash)
            ;
        if (OPERATION == rx_operation)
        {
            dma_read_from_flash();
            LFSRs(0x0001, 0x0001);
            bch_buffer_decoder();
            aes_enc_dec_buffer(1);
            for (i = 0; i < BUFFER_TMP_SIZE; i++)
            {
                printf(" %d\r\n", buffer_tmp[i]);
            }
            rx_operation = NO_OPERATION;
        }
        else if (OPERATION == write_to_flash)
        {
            aes_enc_dec_buffer(0);
            LFSRs(0x0001, 0x0001);
            bch_buffer_encoder();
            dma_write_to_flash();
            write_to_flash = NO_OPERATION;
        }
        else if (OPERATION == write_key_to_flash)
        {
            dma_write_key_to_flash();
            write_key_to_flash = NO_OPERATION;
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
                Led1On;
                break;
            case '0':
                Led1Off
                ;
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
                write_to_flash = 1;
            }
        }
        else if (OPERATION == flash_key_operation)
        {
            key_1[index_keybuff++] = UCA1RXBUF;
            if (index_keybuff >= KEY_BUFFER_SIZE)
            {
                index_keybuff = 0;
                flash_key_operation = NO_OPERATION;
                write_key_to_flash = 1;
            }
        }
        break;
    case 4:
        break;                             // Vector 4 - TXIFG
    default:
        break;
    }
}

