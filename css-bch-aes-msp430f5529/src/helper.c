/*
 * helper.c
 *
 *  Created on: 3 may. 2019
 *      Author: Rommel García Hernández
 */

// Algoritmo
#include <msp430.h>
#include <stdio.h>
#include <string.h>
//***************************************************************************************
// Definiciones
#define Xorled2 P4OUT ^= BIT7
#define Led2  BIT7
#define Boton1 BIT1
#define Boton2 BIT1

#define UART_PRINTF

#ifdef UART_PRINTF
    int fputc(int _c, register FILE *_fp);
    int fputs(const char *_ptr, register FILE *_fp);
#endif

//***************************************************************************************
void SetVcoreUp (unsigned int level) {
    PMMCTL0_H = PMMPW_H;                                             // Habilita el registro PMM para escritura
    SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level; // Fija la parte alta de SVS/SVM al nuevo nivel
    SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;                    // Fija la parte baja de SVM al nuveo nivel
    while ((PMMIFG & SVSMLDLYIFG) == 0);                             // Espera hasta que SVM se establezca
    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);                               // Borra las bandera que se activaron
    PMMCTL0_L = PMMCOREV0 * level;                                   // Fija VCore a su nuvo nivel
    if ((PMMIFG & SVMLIFG))                                          // Espera hasta que se alcance el nuevo nivel
    while ((PMMIFG & SVMLVLRIFG) == 0);
    SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level; // Fija la parte baja de SVS a su nuveo nivel
    PMMCTL0_H = 0x00;                                                // Bloquea los registros PMM para no escribirlos
}
//***************************************************************************************
//Configuración
void configuracion ( ){
    WDTCTL = WDTPW | WDTHOLD;  // Deten el perro guardián
    P4DIR |= BIT7;             // P4.7 como salida
    P4OUT &= ~BIT7;            // Apagamos al inicio P4.7
    P1DIR |= BIT0;             // P1.0 como salida
    P1OUT &= ~BIT0;            // Apagamos al inicio P1.0
    P2DIR &= ~BIT1;            //P2.1 como entrada
    P2REN |= BIT1;             //P2.1 con resistencia habilitada
    P2OUT |= BIT1;             //P2.1 con resistencia pull-up
    P1DIR &= ~BIT1;            //P1.1 como entrada
    P1REN |= BIT1;             //P1.1 con resistencia habilitada
    P1OUT |= BIT1;             //P1.1 con resistencia pull-up
    SetVcoreUp(0x01);          //Se incrementa Vcore de uno en uno
    SetVcoreUp(0x02);          // Para que el sistema soporte los 25 MHz
    SetVcoreUp(0x03);          // ES decir, un cambio de voltaje a la vez
    UCSCTL3 = SELREF_2;         // Fija DCO FLL con referencia a REFO
    UCSCTL4 |= SELA_2;          // Fija ACLK = REFO
    __bis_SR_register(SCG0);    //Deshabilita el lazo de control de FLL
    UCSCTL0 = 0x0000;           //Fija al valor más bajo DCOx y MODx
    UCSCTL1 = DCORSEL_7;        //Selecciona DCO en rango de los 50 MHz
    UCSCTL2 = FLLD_0 + 762;     //Fija el multiplicador DCO para 25MHz
                                // (N + 1) * FLLRef = Fdco
                                // (762 + 1) * 32768 = 25MHz
                                //y fija FLL Div = fDCOCLK/2
    __bic_SR_register(SCG0);    //Habilita el lazo de control de FLL
    // Ya que es necesario esperar el ajuste de DCO cuando los bits son
    // modificados, es necesario esperar n x 32 x 32 x f_MCLK / f_FLL_de_referecnia
    // La fórmula queda de la siguiente manera
    // 32 x 32 x 25 MHz / 32,768 Hz ~ 780k MCLK ciclos para estabilizar DCO
    __delay_cycles(782000);
    do {      // Hacemos un bucle hasta que XT1, XT2 y DCO se estabilicen
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG); //Manten limpiando las banderas de las fuentes de osci
        SFRIFG1 &= ~OFIFG;                      // Borra las banderas de fallo
    } while (SFRIFG1&OFIFG);                   // Condicion de pregunta para fallo de osciladores
}

//***************************************************************************************
// Función para generar retardos:   carga = 0.0201 + 2.79 tiempo
//cargax1000   tiempo(ms)
void Mydelay(unsigned int multiplo, unsigned int carga) {
    volatile unsigned int contador1;    // Variable local
    volatile unsigned int contador2;    // Variable local
    contador1 = carga;
    contador2 = multiplo;
    while(contador2 > 0){
        do {
            contador1=contador1-1;
        } while(contador1 > 0);
        contador2=contador2-1;
        contador1=carga;
    }
}
//***************************************************************************************
// Función que se debe mejorar la descripción
void configTA0() {
    TA0CCTL0 = CCIE;                     //
    TA0CCR0 = 30000;
    TA0CTL = TASSEL_1 + MC_1 + TACLR;  //
}
//***************************************************************************************
// Función que se debe mejorar la descripción
void configadc12() {
    ADC12CTL0 = ADC12SHT02 + ADC12ON;         // Sampling time, ADC12 on
    ADC12CTL1 = ADC12SHP;                     // Use sampling timer
    //ADC12IE = 0x01;                           // Enable interrupt
    ADC12CTL0 |= ADC12ENC;
    P6SEL |= 0x01;                            // P6.0 ADC option select
    P1DIR |= 0x01;                            // P1.0 output
}
//***************************************************************************************
// Función que se debe mejorar la descripción
void configPWM() {
    P2DIR |= BIT0;                            // P2.0
    P2SEL |= BIT0;                            // P2.0 options select
    TA1CCR0 = 512-1;                          // PWM Period
    TA1CCTL1 = OUTMOD_7;                      // CCR1 reset/set
    TA1CCR1 = 128;                            // CCR1 PWM duty cycle
    TA1CTL = TASSEL_1 + MC_1 + TACLR;         // ACLK, up mode, clear TAR
}
//***************************************************************************************
// Función para configuración UART a 115200 a 25 MHz
void configUART() {
      P4SEL = BIT5+BIT4;                        // P4.4, P4.5 = USCI_A0 TXD/RXD
      UCA1CTL1 |= UCSWRST;                      // Reset FSM
      UCA1CTL1 |= UCSSEL_2;                     // Fuente SMCLK (25 MHz)
      UCA1BR0 = 13;                            // 25MHz a 115200 vea http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
      UCA1BR1 = 0;                             // 25MHz a 115200 vea http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
      UCA1MCTL = UCBRS_0 + UCBRF_9 + UCOS16;
      UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
      UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt
}
//***************************************************************************************
// Interrupciones
// Timer0 A0 interrupt service routine
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) TIMER0_A0_ISR (void) {
    Xorled2;                                                      //Parpadeo en led 2 (4.7)
}
//***************************************************************************************

#ifdef UART_PRINTF
int fputc(int _c, register FILE *_fp) {
    while(!(UCA1IFG&UCTXIFG));
    UCA1TXBUF = (unsigned char) _c;
    return((unsigned char)_c);
}

int fputs(const char *_ptr, register FILE *_fp) {
    unsigned int i, len;
    len = strlen(_ptr);
    for(i=0 ; i<len ; i++) {
        while(!(UCA1IFG&UCTXIFG));
        UCA1TXBUF = (unsigned char) _ptr[i];
    }
    return len;
}
#endif
