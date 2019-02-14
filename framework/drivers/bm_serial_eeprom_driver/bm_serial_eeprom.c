/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * Bare-Metal ("BM") device driver for serial EEPROMs.
 *
 * This driver provides support for the Microchip 24AA256 Serial EEPROMs used on A2B boards
 *
 */
#include "bm_serial_eeprom.h"

/**
 * @brief      Initializes the serial eeprom driver
 *
 * @param      serial_eeprom  pointer to driver instance
 * @param[in]  twi_address    twi address of serial eeprom
 * @param[in]  device_num     twi port to use
 *
 * @return     success or error code
 */
BM_SERIAL_EEPROM_RESULT serial_eeprom_initialize(BM_SERIAL_EEPROM *serial_eeprom,
                                                 uint8_t twi_address,
                                                 BM_TWI_PERIPHERAL_NUMBER device_num,
                                                 uint32_t sclk_freq) {

    serial_eeprom->twi_address = twi_address;

    // Set up the simple TWI driver to communicate with
    if (twi_initialize(&serial_eeprom->twi,
                       twi_address,
                       sclk_freq,
                       device_num) != TWI_SIMPLE_SUCCESS) {

        return SERIAL_EEPROM_SIMPLE_TWI_INITIALIZATION;
    }

    return SERIAL_EEPROM_SIMPLE_SUCCESS;
}

/**
 * @brief      Writes a byte to an address in the serial eeprom
 *
 * @param      serial_eeprom  pointer to driver instance
 * @param[in]  address        address in the eeprom
 * @param[in]  value          The value to write
 *
 * @return     success or error code
 */
BM_SERIAL_EEPROM_RESULT serial_eeprom_byte_write(BM_SERIAL_EEPROM *serial_eeprom,
                                                 uint16_t address,
                                                 uint8_t value) {

    uint8_t block[3] = { (uint8_t)(address >> 8),       // high address byte
                         (uint8_t)(address & 0xFF),     // low address byte
                         value };                       // value

    // write block to I2C memory
    if (TWI_SIMPLE_TIMEOUT == twi_write_block(&serial_eeprom->twi, block, 3)) {
        return SERIAL_EEPROM_SIMPLE_TWI_TIMEOUT;
    }

    return SERIAL_EEPROM_SIMPLE_SUCCESS;
}

/**
 * @brief      Reads a byte of a serial eeprom at a specific address
 *
 * @param      serial_eeprom  pointer to driver instance
 * @param[in]  address        address in the serial eeprom
 * @param      value          pointer to return value
 *
 * @return     success or error code
 */
BM_SERIAL_EEPROM_RESULT serial_eeprom_byte_read(BM_SERIAL_EEPROM *serial_eeprom,
                                                uint16_t address,
                                                uint8_t *value) {

    uint8_t block[2] = { (uint8_t)(address >> 8),       // high address byte
                         (uint8_t)(address & 0xFF) };   // low address byte

    // write block to I2/TWI memory but don't send stop bit
    if (TWI_SIMPLE_TIMEOUT == twi_write_block_r(&serial_eeprom->twi, block, 2, true)) {
        return SERIAL_EEPROM_SIMPLE_TWI_TIMEOUT;
    }

    // read a single byte
    if (TWI_SIMPLE_TIMEOUT == twi_read(&serial_eeprom->twi, value)) {
        return SERIAL_EEPROM_SIMPLE_TWI_TIMEOUT;
    }

    return SERIAL_EEPROM_SIMPLE_SUCCESS;
}

/**
 * @brief      Writes a full 64-byte page in the serial eeprom
 *
 * @param      serial_eeprom  pointer to driver instance
 * @param[in]  address        address in the serial eeprom
 * @param      values         pointer to array of values
 *
 * @return     success or error code
 */
BM_SERIAL_EEPROM_RESULT serial_eeprom_page_write(BM_SERIAL_EEPROM *serial_eeprom,
                                                 uint16_t address,
                                                 uint8_t *values) {

    int i;
    uint8_t block[66];

    block[0] = (uint8_t)(address >> 8);     // high address byte
    block[1] = (uint8_t)(address & 0xFF);   // low address byte

    for (i = 0; i < 64; i++) {
        block[i + 2] = values[i];
    }

    // write block to I2C memory
    if (TWI_SIMPLE_TIMEOUT == twi_write_block(&serial_eeprom->twi, block, 66)) {
        return SERIAL_EEPROM_SIMPLE_TWI_TIMEOUT;
    }

    return SERIAL_EEPROM_SIMPLE_SUCCESS;
}

/**
 * @brief      Reads a block of data from serial eeprom
 *
 * @param      serial_eeprom  pointer to driver instance
 * @param[in]  address        address in serial eeprom
 * @param      values         pointer to buffer to store read bytes into
 * @param[in]  count          number of bytes to read
 *
 * @return     success or error code
 */
BM_SERIAL_EEPROM_RESULT serial_eeprom_block_read(BM_SERIAL_EEPROM *serial_eeprom,
                                                 uint16_t address,
                                                 uint8_t *values,
                                                 uint16_t count) {

    uint8_t block[2] = { (uint8_t)(address >> 8),       // high address byte
                         (uint8_t)(address & 0xFF) };   // low address byte

    // write block to I2C/TWI memory but don't send stop bit
    if (TWI_SIMPLE_TIMEOUT == twi_write_block_r(&serial_eeprom->twi, block, 2, true)) {
        return SERIAL_EEPROM_SIMPLE_TWI_TIMEOUT;
    }

    // read a single byte
    if (TWI_SIMPLE_TIMEOUT == twi_read_block(&serial_eeprom->twi, values, count)) {
        return SERIAL_EEPROM_SIMPLE_TWI_TIMEOUT;
    }

    return SERIAL_EEPROM_SIMPLE_SUCCESS;
}
