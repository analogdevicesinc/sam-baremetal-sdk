/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * Bare-Metal ("BM") device driver for GPIO.
 *
 * GPIO Simple is a simplified set of gpio drivers that can be used on either the SHARC or ARM
 * core.  These functions expose a more limited set of functionality than the more complete
 * ADI system services drivers.
 *
 * For example, to set a LED10 on the SHARC Audio Module board as a output
 *
 *      gpio_setup(GPIO_SHARC_SAM_LED10, GPIO_OUTPUT);
 *
 * To toggle that LED...
 *
 *      gpio_toggle(GPIO_SHARC_SAM_LED10);
 *
 * Or to create an edge-sensitive interrupt using one of the SHARC Audio Module push buttons
 *
 *      gpio_setup(GPIO_SHARC_SAM_PB1, GPIO_INPUT);
 *      gpio_attachInterrupt(GPIO_SHARC_SAM_PB1, my_callback, GPIO_FALLING, 0);
 *
 */
#include "bm_gpio.h"

/**
 * @brief      Sets up a GPIO pin as either an input or output
 *
 * @param[in]  pin   BM_GPIO_PORTPIN has port in upper 16-bits and pin in lower 16-bits
 * @param[in]  mode  GPIO_INPUT or GPIO_OUTPUT
 *
 * @return     Returns success or failure based on result of operation
 */
BM_GPIO_RESULT gpio_setup(const BM_GPIO_PORTPIN portpin,
                          BM_GPIO_MODE mode){

    ADI_GPIO_PORT port = (ADI_GPIO_PORT)((portpin >> 16) & 0xF);
    uint16_t pinNumber = (portpin & 0x001F);
    uint32_t pinMask = (uint32_t)(1 << pinNumber);

    volatile uint32_t *pREG_PORT_DIR_SET, *pREG_PORT_DIR_CLR;

    switch (port) {
        case ADI_GPIO_PORT_A:
            pREG_PORT_DIR_SET = pREG_PORTA_DIR_SET;
            pREG_PORT_DIR_CLR = pREG_PORTA_DIR_CLR;
            break;
        case ADI_GPIO_PORT_B:
            pREG_PORT_DIR_SET = pREG_PORTB_DIR_SET;
            pREG_PORT_DIR_CLR = pREG_PORTB_DIR_CLR;
            break;
        case ADI_GPIO_PORT_C:
            pREG_PORT_DIR_SET = pREG_PORTC_DIR_SET;
            pREG_PORT_DIR_CLR = pREG_PORTC_DIR_CLR;
            break;
        case ADI_GPIO_PORT_D:
            pREG_PORT_DIR_SET = pREG_PORTD_DIR_SET;
            pREG_PORT_DIR_CLR = pREG_PORTD_DIR_CLR;
            break;
        case ADI_GPIO_PORT_E:
            pREG_PORT_DIR_SET = pREG_PORTE_DIR_SET;
            pREG_PORT_DIR_CLR = pREG_PORTE_DIR_CLR;
			break;
        case ADI_GPIO_PORT_F:
            pREG_PORT_DIR_SET = pREG_PORTF_DIR_SET;
            pREG_PORT_DIR_CLR = pREG_PORTF_DIR_CLR;
            break;
        case ADI_GPIO_PORT_G:
            pREG_PORT_DIR_SET = pREG_PORTG_DIR_SET;
            pREG_PORT_DIR_CLR = pREG_PORTG_DIR_CLR;
            break;
        default:
            return GPIO_FAILURE;
    }

    if (mode == GPIO_OUTPUT) {
        *pREG_PORT_DIR_SET = pinMask;
    }
    else if (mode == GPIO_INPUT) {
        *pREG_PORT_DIR_CLR = pinMask;
    }
    else {
        return GPIO_FAILURE;
    }

    return GPIO_SUCCESS;
}

/**
 * @brief      Sets a GPIO pin to a high or low value
 *
 * @param[in]  pin    BM_GPIO_PORTPIN has port in upper 16-bits and pin in lower 16-bits
 * @param[in]  value  GPIO_LOW or GPIO_HIGH
 *
 * @return     Returns success or failure based on result of operation
 */
BM_GPIO_RESULT gpio_write(const BM_GPIO_PORTPIN portpin,
                          BM_GPIO_VAL value) {

    ADI_GPIO_PORT port = (ADI_GPIO_PORT)((portpin >> 16) & 0xF);
    uint16_t pinNumber = (portpin & 0x001F);
    uint32_t pinMask = (uint32_t)(1 << pinNumber);

    volatile uint32_t *pREG_PORT_DATA_SET, *pREG_PORT_DATA_CLR;

    switch (port) {
        case ADI_GPIO_PORT_A:
            pREG_PORT_DATA_SET = pREG_PORTA_DATA_SET;
            pREG_PORT_DATA_CLR = pREG_PORTA_DATA_CLR;
            break;
        case ADI_GPIO_PORT_B:
            pREG_PORT_DATA_SET = pREG_PORTB_DATA_SET;
            pREG_PORT_DATA_CLR = pREG_PORTB_DATA_CLR;
            break;
        case ADI_GPIO_PORT_C:
            pREG_PORT_DATA_SET = pREG_PORTC_DATA_SET;
            pREG_PORT_DATA_CLR = pREG_PORTC_DATA_CLR;
            break;
        case ADI_GPIO_PORT_D:
            pREG_PORT_DATA_SET = pREG_PORTD_DATA_SET;
            pREG_PORT_DATA_CLR = pREG_PORTD_DATA_CLR;
            break;
        case ADI_GPIO_PORT_E:
            pREG_PORT_DATA_SET = pREG_PORTE_DATA_SET;
            pREG_PORT_DATA_CLR = pREG_PORTE_DATA_CLR;
            break;
        case ADI_GPIO_PORT_F:
            pREG_PORT_DATA_SET = pREG_PORTF_DATA_SET;
            pREG_PORT_DATA_CLR = pREG_PORTF_DATA_CLR;
            break;
        case ADI_GPIO_PORT_G:
            pREG_PORT_DATA_SET = pREG_PORTG_DATA_SET;
            pREG_PORT_DATA_CLR = pREG_PORTG_DATA_CLR;
            break;
        default:
            return GPIO_FAILURE;
    }

    if (value == GPIO_HIGH) {
        *pREG_PORT_DATA_SET = pinMask;
    }
    else if (value == GPIO_LOW) {
        *pREG_PORT_DATA_CLR = pinMask;
    }
    else {
        return GPIO_FAILURE;
    }

    return GPIO_SUCCESS;
}

/**
 * @brief      Toggles a GPIO pin
 *
 * @param[in]  BM_GPIO_PORTPIN  BM_GPIO_PORTPIN has port in upper 16-bits and pin in lower 16-bits
 *
 * @return     Returns success or failure based on result of operation
 */
BM_GPIO_RESULT gpio_toggle(const BM_GPIO_PORTPIN portpin) {

    ADI_GPIO_PORT port = (ADI_GPIO_PORT)((portpin >> 16) & 0xF);
    uint16_t pinNumber = (portpin & 0x001F);
    uint32_t pinMask = (uint32_t)(1 << pinNumber);

    volatile uint32_t *pREG_PORT_DATA_TGL;

    switch (port) {
        case ADI_GPIO_PORT_A:   pREG_PORT_DATA_TGL = pREG_PORTA_DATA_TGL; break;
        case ADI_GPIO_PORT_B:   pREG_PORT_DATA_TGL = pREG_PORTB_DATA_TGL; break;
        case ADI_GPIO_PORT_C:   pREG_PORT_DATA_TGL = pREG_PORTC_DATA_TGL; break;
        case ADI_GPIO_PORT_D:   pREG_PORT_DATA_TGL = pREG_PORTD_DATA_TGL; break;
        case ADI_GPIO_PORT_E:   pREG_PORT_DATA_TGL = pREG_PORTE_DATA_TGL; break;
        case ADI_GPIO_PORT_F:   pREG_PORT_DATA_TGL = pREG_PORTF_DATA_TGL; break;
        case ADI_GPIO_PORT_G:   pREG_PORT_DATA_TGL = pREG_PORTG_DATA_TGL; break;
        default:
            return GPIO_FAILURE;
    }

    *pREG_PORT_DATA_TGL = pinMask;

    return GPIO_SUCCESS;
}

/**
 * @brief      Reads the value of a GPIO pin
 *
 * @param[in]  pin   BM_GPIO_PORTPIN has port in upper 16-bits and pin in lower 16-bits
 *
 * @return     Returns success or failure based on result of operation
 */
BM_GPIO_VAL gpio_read(const BM_GPIO_PORTPIN pin) {

    ADI_GPIO_PORT port = (ADI_GPIO_PORT)((pin >> 16) & 0xF);
    uint16_t pinNumber = (pin & 0x001F);
    uint32_t pinMask = (uint32_t)(1 << pinNumber);

    volatile uint32_t *pREG_PORT_DATA;

    switch (port) {
        case ADI_GPIO_PORT_A:   pREG_PORT_DATA = pREG_PORTA_DATA; break;
        case ADI_GPIO_PORT_B:   pREG_PORT_DATA = pREG_PORTB_DATA; break;
        case ADI_GPIO_PORT_C:   pREG_PORT_DATA = pREG_PORTC_DATA; break;
        case ADI_GPIO_PORT_D:   pREG_PORT_DATA = pREG_PORTD_DATA; break;
        case ADI_GPIO_PORT_E:   pREG_PORT_DATA = pREG_PORTE_DATA; break;
        case ADI_GPIO_PORT_F:   pREG_PORT_DATA = pREG_PORTF_DATA; break;
        case ADI_GPIO_PORT_G:   pREG_PORT_DATA = pREG_PORTG_DATA; break;
        default:
            return GPIO_ERROR;
    }

    if (*pREG_PORT_DATA & pinMask) return GPIO_HIGH;
    else return GPIO_LOW;
}

/*
 * These arrays hold the user call back functions for each of the 16 GPIO pins
 * within each of the 6 blocks (A-F)
 */
void (*port_a_callbacks[16])(void *) = {NULL};
void (*port_b_callbacks[16])(void *) = {NULL};
void (*port_c_callbacks[16])(void *) = {NULL};
void (*port_d_callbacks[16])(void *) = {NULL};
void (*port_e_callbacks[16])(void *) = {NULL};
void (*port_f_callbacks[16])(void *) = {NULL};

/**
 * These are the default interrupt handlers for the GPIO pins.  These functions
 * determine which pin originated the interrupt and call the callback file
 * associated with that pin.
 */
static void pint0_handler(uint32_t iid,
                          void *handlerArg) {
    int i;
    for (i = 0; i < 16; i++) {
        // Check to see if we have an interrupt pending and it's one we have a
        // call back for
        if (*pREG_PINT0_REQ & (0x1 << i) && port_a_callbacks[i] != NULL) {
            *pREG_PINT0_REQ |= (0x1 << i);  // Clear interrupt
            port_a_callbacks[i](handlerArg);
        }
    }
}
static void pint1_handler(uint32_t iid,
                          void *handlerArg) {
    int i;
    for (i = 0; i < 16; i++) {
        // Check to see if we have an interrupt pending and it's one we have a
        // call back for
        if (*pREG_PINT1_REQ & (0x1 << i) && port_b_callbacks[i] != NULL) {
            *pREG_PINT1_REQ |= (0x1 << i);  // Clear interrupt
            port_b_callbacks[i](handlerArg);
        }
    }
}
static void pint2_handler(uint32_t iid,
                          void *handlerArg) {
    int i;
    for (i = 0; i < 16; i++) {
        // Check to see if we have an interrupt pending and it's one we have a
        // call back for
        if (*pREG_PINT2_REQ & (0x1 << i) && port_c_callbacks[i] != NULL) {
            *pREG_PINT2_REQ |= (0x1 << i);  // Clear interrupt
            port_c_callbacks[i](handlerArg);
        }
    }
}
static void pint3_handler(uint32_t iid,
                          void *handlerArg) {
    int i;
    for (i = 0; i < 16; i++) {
        // Check to see if we have an interrupt pending and it's one we have a
        // call back for
        if (*pREG_PINT3_REQ & (0x1 << i) && port_d_callbacks[i] != NULL) {
            *pREG_PINT3_REQ |= (0x1 << i);  // Clear interrupt
            port_d_callbacks[i](handlerArg);
        }
    }
}
static void pint4_handler(uint32_t iid,
                          void *handlerArg) {
    int i;
    for (i = 0; i < 16; i++) {
        // Check to see if we have an interrupt pending and it's one we have a
        // call back for
        if (*pREG_PINT4_REQ & (0x1 << i) && port_e_callbacks[i] != NULL) {
            *pREG_PINT4_REQ |= (0x1 << i);  // Clear interrupt
            port_e_callbacks[i](handlerArg);
        }
    }
}
static void pint5_handler(uint32_t iid,
                          void *handlerArg) {
    int i;
    for (i = 0; i < 16; i++) {
        // Check to see if we have an interrupt pending and it's one we have a
        // call back for
        if (*pREG_PINT5_REQ & (0x1 << i) && port_f_callbacks[i] != NULL) {
            *pREG_PINT5_REQ |= (0x1 << i);  // Clear interrupt
            port_f_callbacks[i](handlerArg);
        }
    }
}

/**
 * @brief      Attaches an interrupt to a given GPIO pin
 *
 * @param[in]  BM_GPIO_PORTPIN     BM_GPIO_PORTPIN has port in upper 16-bits and pin in lower 16-bits
 * @param[in]  callback    The callback function
 * @param[in]  mode        When to trigger the interrupt (e.g. rising, falling, level)
 * @param      dataObject  An optional data object that will be passed to the callback
 *
 * @return     Returns success or failure based on result of operation
 */
BM_GPIO_RESULT gpio_attach_interrupt(const BM_GPIO_PORTPIN portpin,
                                     void (*callback)(void *),
                                     BM_GPIO_EDGE mode,
                                     void *data_object) {

    ADI_GPIO_PORT port = (ADI_GPIO_PORT)((portpin >> 16) & 0xF);
    uint16_t pinnumber = (portpin & 0x001F);
    uint32_t pinmask = (uint32_t)(1 << pinnumber);

    volatile uint32_t *pREG_PORT_INEN_SET;
    volatile uint32_t *pREG_PORT_DIR_CLR;
    volatile uint32_t *pREG_PINT_INV_CLR;
    volatile uint32_t *pREG_PINT_INV_SET;
    volatile uint32_t *pREG_PINT_EDGE_SET;
    volatile uint32_t *pREG_PINT_EDGE_CLR;
    volatile uint32_t *pREG_PINT_ASSIGN;
    volatile uint32_t *pREG_PINT_MSK_SET;
    uint32_t INTR_PINT_BLOCK;
    void (*pint_handler)(uint32_t, void *);

    switch (port) {
        case ADI_GPIO_PORT_A:
            pREG_PORT_INEN_SET = pREG_PORTA_INEN_SET;
            pREG_PORT_DIR_CLR =  pREG_PORTA_DIR_CLR;
            pREG_PINT_INV_SET  = pREG_PINT0_INV_SET;
            pREG_PINT_INV_CLR  = pREG_PINT0_INV_CLR;
            pREG_PINT_EDGE_SET = pREG_PINT0_EDGE_SET;
            pREG_PINT_EDGE_CLR = pREG_PINT0_EDGE_CLR;
            pREG_PINT_ASSIGN   = pREG_PINT0_ASSIGN;
            pREG_PINT_MSK_SET  = pREG_PINT0_MSK_SET;
            INTR_PINT_BLOCK    = INTR_PINT0_BLOCK;
            pint_handler        = pint0_handler;
            port_a_callbacks[pinnumber] = callback;
            break;

        case ADI_GPIO_PORT_B:
            pREG_PORT_INEN_SET = pREG_PORTB_INEN_SET;
            pREG_PORT_DIR_CLR =  pREG_PORTB_DIR_CLR;
            pREG_PINT_INV_SET  = pREG_PINT1_INV_SET;
            pREG_PINT_INV_CLR  = pREG_PINT1_INV_CLR;
            pREG_PINT_EDGE_SET = pREG_PINT1_EDGE_SET;
            pREG_PINT_EDGE_CLR = pREG_PINT1_EDGE_CLR;
            pREG_PINT_ASSIGN   = pREG_PINT1_ASSIGN;
            pREG_PINT_MSK_SET  = pREG_PINT1_MSK_SET;
            INTR_PINT_BLOCK    = INTR_PINT1_BLOCK;
            pint_handler        = pint1_handler;
            port_b_callbacks[pinnumber] = callback;
            break;

        case ADI_GPIO_PORT_C:
            pREG_PORT_INEN_SET = pREG_PORTC_INEN_SET;
            pREG_PORT_DIR_CLR =  pREG_PORTC_DIR_CLR;
            pREG_PINT_INV_SET  = pREG_PINT2_INV_SET;
            pREG_PINT_INV_CLR  = pREG_PINT2_INV_CLR;
            pREG_PINT_EDGE_SET = pREG_PINT2_EDGE_SET;
            pREG_PINT_EDGE_CLR = pREG_PINT2_EDGE_CLR;
            pREG_PINT_ASSIGN   = pREG_PINT2_ASSIGN;
            pREG_PINT_MSK_SET  = pREG_PINT2_MSK_SET;
            INTR_PINT_BLOCK    = INTR_PINT2_BLOCK;
            pint_handler        = pint2_handler;
            port_c_callbacks[pinnumber] = callback;
            break;

        case ADI_GPIO_PORT_D:
            pREG_PORT_INEN_SET = pREG_PORTD_INEN_SET;
            pREG_PORT_DIR_CLR =  pREG_PORTD_DIR_CLR;
            pREG_PINT_INV_SET  = pREG_PINT3_INV_SET;
            pREG_PINT_INV_CLR  = pREG_PINT3_INV_CLR;
            pREG_PINT_EDGE_SET = pREG_PINT3_EDGE_SET;
            pREG_PINT_EDGE_CLR = pREG_PINT3_EDGE_CLR;
            pREG_PINT_ASSIGN   = pREG_PINT3_ASSIGN;
            pREG_PINT_MSK_SET  = pREG_PINT3_MSK_SET;
            INTR_PINT_BLOCK    = INTR_PINT3_BLOCK;
            pint_handler        = pint3_handler;
            port_d_callbacks[pinnumber] = callback;
            break;

        case ADI_GPIO_PORT_E:
            pREG_PORT_INEN_SET = pREG_PORTE_INEN_SET;
            pREG_PORT_DIR_CLR =  pREG_PORTE_DIR_CLR;
            pREG_PINT_INV_SET  = pREG_PINT4_INV_SET;
            pREG_PINT_INV_CLR  = pREG_PINT4_INV_CLR;
            pREG_PINT_EDGE_SET = pREG_PINT4_EDGE_SET;
            pREG_PINT_EDGE_CLR = pREG_PINT4_EDGE_CLR;
            pREG_PINT_ASSIGN   = pREG_PINT4_ASSIGN;
            pREG_PINT_MSK_SET  = pREG_PINT4_MSK_SET;
            INTR_PINT_BLOCK    = INTR_PINT4_BLOCK;
            pint_handler        = pint4_handler;
            port_e_callbacks[pinnumber] = callback;
            break;

        case ADI_GPIO_PORT_F:
            pREG_PORT_INEN_SET = pREG_PORTF_INEN_SET;
            pREG_PORT_DIR_CLR =  pREG_PORTF_DIR_CLR;
            pREG_PINT_INV_SET  = pREG_PINT5_INV_SET;
            pREG_PINT_INV_CLR  = pREG_PINT5_INV_CLR;
            pREG_PINT_EDGE_SET = pREG_PINT5_EDGE_SET;
            pREG_PINT_EDGE_CLR = pREG_PINT5_EDGE_CLR;
            pREG_PINT_ASSIGN   = pREG_PINT5_ASSIGN;
            pREG_PINT_MSK_SET  = pREG_PINT5_MSK_SET;
            INTR_PINT_BLOCK    = INTR_PINT5_BLOCK;
            pint_handler        = pint5_handler;
            port_f_callbacks[pinnumber] = callback;

            break;
        default:
            return GPIO_FAILURE;
    }

    *pREG_PORT_INEN_SET = pinmask;
    *pREG_PORT_DIR_CLR = pinmask;

    // Set up edge triggered interrupts for this port
    if (mode == GPIO_RISING || mode == GPIO_FALLING) {
        *pREG_PINT_EDGE_SET = pinmask;
        if      (mode == GPIO_RISING) *pREG_PINT_INV_CLR = pinmask;
        else if (mode == GPIO_FALLING) *pREG_PINT_INV_SET = pinmask;
    }
    // Set up level triggered interrupts for this port
    else if (mode == GPIO_LEVEL_LOW) {
        *pREG_PINT_EDGE_CLR = pinmask;     // Clear edge interrupt
        *pREG_PINT_INV_SET = pinmask;      // Invert pin for active low
    }

    // Assign PINT to this port
    *pREG_PINT_ASSIGN = 0x0000;
    *pREG_PINT_MSK_SET = pinmask;

    adi_int_InstallHandler(INTR_PINT_BLOCK, pint_handler, data_object, true);

    return GPIO_SUCCESS;
}

/**
 * @brief      Masks the interrupt and detaches callback function
 *
 * @param[in]  BM_GPIO_PORTPIN  BM_GPIO_PORTPIN has port in upper 16-bits and pin in lower 16-bits
 *
 * @return     { description_of_the_return_value }
 *
 * @return     Returns success or failure based on result of operation
 */
BM_GPIO_RESULT gpio_detach_interrupt(const BM_GPIO_PORTPIN portpin) {

    ADI_GPIO_PORT port = (ADI_GPIO_PORT)((portpin >> 16) & 0xF);
    uint16_t pinnumber = (portpin & 0x001F);
    uint32_t pinmask = (uint32_t)(1 << pinnumber);

    switch (port) {
        case ADI_GPIO_PORT_A:
            port_a_callbacks[pinnumber] = NULL;
            *pREG_PINT0_MSK_CLR = pinmask;
            break;
        case ADI_GPIO_PORT_B:
            port_b_callbacks[pinnumber] = NULL;
            *pREG_PINT1_MSK_CLR = pinmask;
            break;
        case ADI_GPIO_PORT_C:
            port_c_callbacks[pinnumber] = NULL;
            *pREG_PINT2_MSK_CLR = pinmask;
            break;
        case ADI_GPIO_PORT_D:
            port_d_callbacks[pinnumber] = NULL;
            *pREG_PINT3_MSK_CLR = pinmask;
            break;
        case ADI_GPIO_PORT_E:
            port_e_callbacks[pinnumber] = NULL;
            *pREG_PINT4_MSK_CLR = pinmask;
            break;
        case ADI_GPIO_PORT_F:
            port_f_callbacks[pinnumber] = NULL;
            *pREG_PINT5_MSK_CLR = pinmask;
            break;
        default:
            return GPIO_FAILURE;
    }
    return GPIO_SUCCESS;
}
