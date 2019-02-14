/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * Bare-Metal ("BM") device driver header file for serial EEPROMs.
 *
 */
#ifndef _BM_SERIAL_EEPROM_H
#define _BM_SERIAL_EEPROM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "../bm_gpio_driver/bm_gpio.h"
#include "../bm_twi_driver/bm_twi.h"

#define SERIAL_EEPROM_ADDR000   (0x50)
#define SERIAL_EEPROM_ADDR001   (0x51)
#define SERIAL_EEPROM_ADDR010   (0x52)
#define SERIAL_EEPROM_ADDR011   (0x53)
#define SERIAL_EEPROM_ADDR100   (0x54)
#define SERIAL_EEPROM_ADDR101   (0x55)
#define SERIAL_EEPROM_ADDR110   (0x56)
#define SERIAL_EEPROM_ADDR111   (0x57)

typedef enum
{
    SERIAL_EEPROM_SIMPLE_SUCCESS,           // The API call is success
    SERIAL_EEPROM_SIMPLE_TWI_TIMEOUT,       // TWI access timed out
    SERIAL_EEPROM_SIMPLE_TWI_INITIALIZATION,// TWI initialization error (likely invalid parameters)
    SERIAL_EEPROM_SIMPLE_ERROR              // General failure
} BM_SERIAL_EEPROM_RESULT;

typedef struct
{
    BM_TWI twi;                         // Simple TWI driver
    uint8_t twi_address;
} BM_SERIAL_EEPROM;

#ifdef __cplusplus
extern "C" {
#endif

// Initializes the serial eeprom driver
BM_SERIAL_EEPROM_RESULT serial_eeprom_initialize(BM_SERIAL_EEPROM *serial_eeprom,
                                                 uint8_t twi_address,
                                                 BM_TWI_PERIPHERAL_NUMBER device_num,
                                                 uint32_t sclk_freq);

// Writes a byte to an address in the serial eeprom
BM_SERIAL_EEPROM_RESULT serial_eeprom_byte_write(BM_SERIAL_EEPROM *serial_eeprom,
                                                 uint16_t address,
                                                 uint8_t value);
// Writes a full 64-byte page in the serial eeprom
BM_SERIAL_EEPROM_RESULT serial_eeprom_page_write(BM_SERIAL_EEPROM *serial_eeprom,
                                                 uint16_t address,
                                                 uint8_t *values);

//Reads a byte of a serial eeprom at a specific address
BM_SERIAL_EEPROM_RESULT serial_eeprom_byte_read(BM_SERIAL_EEPROM *serial_eeprom,
                                                uint16_t address,
                                                uint8_t *value);

// Reads a block of data from serial eeprom
BM_SERIAL_EEPROM_RESULT serial_eeprom_block_read(BM_SERIAL_EEPROM *serial_eeprom,
                                                 uint16_t address,
                                                 uint8_t *values,
                                                 uint16_t count);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _BM_SERIAL_EEPROM_H
