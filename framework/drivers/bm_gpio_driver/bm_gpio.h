/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * Bare-Metal ("BM") device driver header file for GPIO
 *
 */

#ifndef _BM_GPIO_H
#define _BM_GPIO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <services/gpio/adi_gpio.h>
#include <services/int/adi_int.h>
#include <sys/platform.h>

/**
 * A "BM_GPIO_PORTPIN" allows us to pass a GPIO pin configuration as a single variable and thus
 * store various GPIO pins in pre-processor variables.  The upper 16 bits contain
 * the port and the lower 16 bits contain the pin value.
 */
typedef unsigned int BM_GPIO_PORTPIN;

// Makes a portpin value from a 16-bit port and a 16-bit pin value
#define BM_GPIO_PORTPIN_MAKE(PORT, PIN)     ((BM_GPIO_PORTPIN) ((PORT & 0xFF) << 16) | (PIN & 0xFF))

typedef enum
{
    GPIO_INPUT,                 // Configure pin as an input
    GPIO_OUTPUT                 // Configure pin as an output
} BM_GPIO_MODE;

typedef enum
{
    GPIO_RISING,                // Interrupts on rising edge
    GPIO_FALLING,               // Interrupts on falling edge
    GPIO_LEVEL_LOW              // Interrupts on low (level sensitive)
} BM_GPIO_EDGE;

typedef enum
{
    GPIO_LOW,                   // Low pin state  (0V)
    GPIO_HIGH,                  // High pin state (VDD)
    GPIO_ERROR = -1             // Error setting or reading pin state
} BM_GPIO_VAL;

typedef enum
{
    GPIO_SUCCESS,               // Operation succeeded
    GPIO_FAILURE                // Operation failed
} BM_GPIO_RESULT;



#ifdef __cplusplus
extern "C" {
#endif

// Sets up a GPIO pin as either an input or output
BM_GPIO_RESULT gpio_setup(const BM_GPIO_PORTPIN portpin,
                          BM_GPIO_MODE mode);

// Sets a GPIO pin to a high or low value
BM_GPIO_RESULT gpio_write(const BM_GPIO_PORTPIN portpin,
                          BM_GPIO_VAL value);

// Reads the value of a GPIO pin
BM_GPIO_VAL gpio_read(const BM_GPIO_PORTPIN portpin);

// Toggles a GPIO pin
BM_GPIO_RESULT gpio_toggle(const BM_GPIO_PORTPIN portpin);

// Attaches an interrupt to a given GPIO pin
BM_GPIO_RESULT gpio_attach_interrupt(const BM_GPIO_PORTPIN portpin,
                                     void (*callback)(void *),
                                     BM_GPIO_EDGE mode,
                                     void *data_object);

// Masks the interrupt and detaches callback function
BM_GPIO_RESULT gpio_detach_interrupt(const BM_GPIO_PORTPIN portpin);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _BM_GPIO_H
