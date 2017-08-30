#include "TM4C123.h"                    // Device header
/* This program converts the analog output from an LM34 and convert it to temperature in Fahrenheit.
 Notice from Table 7-5, AIN0 channel is on PE3 pin Temp is sent to UART to be viewed on TeraTerminal */


#include <stdio.h>

void UART0Tx(char c);
void UART0_init(void);
void UART0_puts(char* s);
void delayMs(int n);

int main(void)
{
    int temperature;
    char buffer[16];

    /* initialize UART0 for output */
    UART0_init();

    /* enable clocks */
    SYSCTL->RCGCGPIO |= 0x10;   /* enable clock to GPIOE */
    SYSCTL->RCGCADC |= 1;       /* enable clock to ADC0 */
    
    /* initialize PE3 for AIN0 input */
    GPIOE->AFSEL |= 8;          /* enable alternate function */
    GPIOE->DEN &= ~8;           /* disable digital function */
    GPIOE->AMSEL |= 8;          /* enable analog function */
   
    /* initialize ADC0 */
    ADC0->ACTSS &= ~8;        /* disable SS3 during configuration */
    ADC0->EMUX &= ~0xF000;    /* software trigger conversion */
    ADC0->SSMUX3 = 0;         /* get input from channel 0 */
    ADC0->SSCTL3 |= 6;        /* take one sample at a time, set flag at 1st sample */
    ADC0->ACTSS |= 8;         /* enable ADC0 sequencer 3 */

    while(1)
    {
        ADC0->PSSI |= 8;        /* start a conversion sequence 3 */
        while((ADC0->RIS & 8) == 0) ;   /* wait for conversion complete */
        temperature = ADC0->SSFIFO3 * 330 / 4096;
			 // temperature = ADC0->SSFIFO3;
        ADC0->ISC = 8;          /* clear completion flag  */
        sprintf(buffer, "\r\nTemp = %dF", temperature);
        UART0_puts(buffer);
        delayMs(2000);
    }
}

void UART0_init(void)
{
    SYSCTL->RCGCUART |= 1;  /* provide clock to UART0 */
    SYSCTL->RCGCGPIO |= 0x01;   /* enable clock to GPIOA */

    /* UART0 initialization */
    UART0->CTL = 0;         /* disable UART0 */
    UART0->IBRD = 104;      /* 16MHz/16=1MHz, 1MHz/104=9600 baud rate */
    UART0->FBRD = 11;       /* fraction part, see Example 4-4 */
    UART0->CC = 0;          /* use system clock */
    UART0->LCRH = 0x60;     /* 8-bit, no parity, 1-stop bit, no FIFO */
    UART0->CTL = 0x301;     /* enable UART0, TXE, RXE */
    
    /* UART0 TX0 and RX0 use PA0 and PA1. Set them up. */
    GPIOA->DEN = 0x03;      /* Make PA0 and PA1 as digital */
    GPIOA->AFSEL = 0x03;    /* Use PA0,PA1 alternate function */
    GPIOA->PCTL = 0x11;     /* configure PA0 and PA1 for UART */
}

void UART0Tx(char c)  
{
    while((UART0->FR & 0x20) != 0); /* wait until Tx buffer not full */
    UART0->DR = c;                  /* before giving it another byte */
}

void UART0_puts(char* s)
{
    while (*s != 0)         /* if not end of string */
        UART0Tx(*s++);      /* send the character through UART0 */
}

/* delay n milliseconds (16 MHz CPU clock) */
void delayMs(int n)
{
    int32_t i, j;
    for(i = 0 ; i < n; i++)
        for(j = 0; j < 3180; j++)
            {}  /* do nothing for 1 ms */
}
