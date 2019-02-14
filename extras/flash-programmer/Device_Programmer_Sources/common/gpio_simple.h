/*
 * Copyright (c) 2018 Analog Devices.  All rights reserved.
 *
 * @file:    adi_adau1761.h
 * @brief:   ADAU1977 ADC
 * @version: $Revision: 0 $
 * @date:    $Date: 2017-05-04 12:32:28 -0500 (May, 04 2017) $
 *
 * @details
 *            This is the primary header file for ADAU1977 ADC
 *            The file contains the API declarations, data structure and constant
 *            definitions used in the APIs.
 */


#ifndef __ADI_SIMPLE_GPIO_H__
#define __ADI_SIMPLE_GPIO_H__

/*=============  I N C L U D E S   =============*/
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/platform.h>
#include <services/gpio/adi_gpio.h>


/*==============  D E F I N E S  ===============*/

///< ADAU1977 return values
typedef enum BM_GPIO_MODE
{
    GPIO_INPUT,                     ///< Configure pin as an input
    GPIO_OUTPUT                    ///< Configure pin as an output
} BM_GPIO_MODE;

typedef enum BM_GPIO_VAL
{
    GPIO_LOW,                     ///< Low pin state  (0V)
    GPIO_HIGH,                    ///< High pin state (VDD)
    GPIO_ERROR                    ///< Error setting or reading pin state
} BM_GPIO_VAL;

typedef enum BM_GPIO_RESULT
{
    GPIO_SUCCESS,                 ///< Operation succeeded
    GPIO_FAILURE                ///< Operation failed
} BM_GPIO_RESULT;

typedef uint32_t BM_GPIO_PORTPIN;

/**
 * A simple structure that contains information for a single instance of this
 * driver
 */
#ifdef __cplusplus
extern "C"{
#endif

BM_GPIO_RESULT     gpio_setup(const BM_GPIO_PORTPIN pin, BM_GPIO_MODE mode );
BM_GPIO_RESULT     gpio_write(const BM_GPIO_PORTPIN pin, BM_GPIO_VAL value );
BM_GPIO_VAL         gpio_read(const BM_GPIO_PORTPIN pin );

#ifdef __cplusplus
} ///< extern "C"
#endif

///< __ADI_SIMPLE_GPIO_H__
#endif
