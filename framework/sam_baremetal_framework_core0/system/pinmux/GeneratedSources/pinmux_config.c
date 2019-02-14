/*
 **
 ** Source file generated on September 17, 2018 at 10:53:01.	
 **
 ** Copyright (C) 2011-2018 Analog Devices Inc., All Rights Reserved.
 **
 ** This file is generated automatically based upon the options selected in 
 ** the Pin Multiplexing configuration editor. Changes to the Pin Multiplexing
 ** configuration should be made by changing the appropriate options rather
 ** than editing this file.
 **
 ** Selected Peripherals
 ** --------------------
 ** SPI0 (CLK, MISO, MOSI)
 ** UART0 (RX, TX)
 ** UART1 (RX, TX)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** PB02, PB03, PC09, PC10, PC11, PC13, PC14
 */

#include <sys/platform.h>
#include <stdint.h>

#define SPI0_CLK_PORTC_MUX  ((uint32_t) ((uint32_t) 0<<18))
#define SPI0_MISO_PORTC_MUX  ((uint32_t) ((uint32_t) 0<<20))
#define SPI0_MOSI_PORTC_MUX  ((uint32_t) ((uint32_t) 0<<22))
#define UART0_RX_PORTC_MUX  ((uint32_t) ((uint32_t) 0<<28))
#define UART0_TX_PORTC_MUX  ((uint32_t) ((uint32_t) 0<<26))
#define UART1_RX_PORTB_MUX  ((uint16_t) ((uint16_t) 1<<6))
#define UART1_TX_PORTB_MUX  ((uint16_t) ((uint16_t) 1<<4))

#define SPI0_CLK_PORTC_FER  ((uint32_t) ((uint32_t) 1<<9))
#define SPI0_MISO_PORTC_FER  ((uint32_t) ((uint32_t) 1<<10))
#define SPI0_MOSI_PORTC_FER  ((uint32_t) ((uint32_t) 1<<11))
#define UART0_RX_PORTC_FER  ((uint32_t) ((uint32_t) 1<<14))
#define UART0_TX_PORTC_FER  ((uint32_t) ((uint32_t) 1<<13))
#define UART1_RX_PORTB_FER  ((uint16_t) ((uint16_t) 1<<3))
#define UART1_TX_PORTB_FER  ((uint16_t) ((uint16_t) 1<<2))

int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX and FER Registers
 */
int32_t adi_initpinmux(void) {
    /* PORTx_MUX registers */
    *pREG_PORTB_MUX = UART1_RX_PORTB_MUX | UART1_TX_PORTB_MUX;
    *pREG_PORTC_MUX = SPI0_CLK_PORTC_MUX | SPI0_MISO_PORTC_MUX
     | SPI0_MOSI_PORTC_MUX | UART0_RX_PORTC_MUX | UART0_TX_PORTC_MUX;

    /* PORTx_FER registers */
    *pREG_PORTB_FER = UART1_RX_PORTB_FER | UART1_TX_PORTB_FER;
    *pREG_PORTC_FER = SPI0_CLK_PORTC_FER | SPI0_MISO_PORTC_FER
     | SPI0_MOSI_PORTC_FER | UART0_RX_PORTC_FER | UART0_TX_PORTC_FER;
    return 0;
}

