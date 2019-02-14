/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * Bare-Metal ("BM") device driver for ADAUxxxx devices. This is a generic driver that can be used
 * with any I2C device for which SigmaStudio is able to generate an initialization file.
 *
 * It has additional support for devices with non-I2C interfaces as well.
 *
 * @file       adau_simple.c
 * @brief      A generic driver for audio converters which can be initialized from SigmaStudio
 */

#include <math.h>
#include <stdio.h>

#include "bm_adau_device.h"

/**
 * @brief      Initialize the an I2C ADAU device using the auto-generated export file from
 *             SigmaStudio
 *
 * @param      adau_device A pointer to the instance for this driver
 * @param      device_num  The SHARC TWI peripheral to use (e.g. TWI0, TWI1,
 *                         TWI2)
 * @param      i2c_address I2C address of the ADAU1761
 * @param      adau_init_data  Pointer to initialization structure
 * @param      address_bytes   Number of bytes used for control register addresses
 * @return     Result indicating success or failure
 */
BM_ADAU_RESULT adau_initialize(BM_ADAU_DEVICE *adau_device,
                               BM_TWI_PERIPHERAL_NUMBER device_num,
                               uint8_t i2c_address,
                               BM_ADAU_DEVICE_INIT_DATA *adau_init_data,
                               uint8_t address_bytes) {

    // Set up simple register-driven TWI driver
    if (twi_initialize(&adau_device->twi, i2c_address, TWI_TYPICAL_SCLK0_FREQ, device_num) != TWI_SIMPLE_SUCCESS) {
        return ADAU_SIMPLE_ERROR;
    }
    twi_set_clock(&adau_device->twi, 100000);

    if (address_bytes > 2 || address_bytes < 1) return ADAU_SIMPLE_ERROR;

    adau_device->address_bytes = address_bytes;

    BM_ADAU_RESULT resAdau;
    if (adau_init_data) {

        /*
         *  In some instances, SigmaStudio will insert an extra address byte (MSB) when
         *  one shouldn't be included in the auto-generated initialization code.  When initializing
         *  a device where this is an issue (e.g. ADAU1977/1979), this field can be set to
         *  true which essentially skips this byte.
         */

        resAdau = adau_load_bulk_reg_file(adau_device,
                                          adau_init_data->data_tx_buffer,
                                          adau_init_data->data_num_bytes,
                                          adau_init_data->total_lines,
                                          adau_init_data->ignore_first_byte_of_init_file);
        if (resAdau != ADAU_SUCCESS) return resAdau;
    }

    return ADAU_SUCCESS;
}

/**
 * @brief      Reads a control register from the ADAU1761 using its 16-bit
 *             address.  See the device datasheet for more info on control
 *             registers.
 *
 * @param      adau1761  Pointer to the instance of this driver
 * @param[in]  address   The control register address to read
 *
 * @return     The value of the control register
 */
BM_ADAU_RESULT adau_read_ctrl_reg(BM_ADAU_DEVICE *adau_device,
                                  uint16_t address,
                                  uint8_t *value) {

    uint8_t addr[2], length;

    // Number of bytes this device uses for register addresses for r/w over I2C/TWI
    uint8_t address_bytes = adau_device->address_bytes;

    if (address_bytes == 2) {
        addr[0] = address >> 8;
        addr[1] = address & 0xff;
        length = 2;
    }
    else {
        addr[0] = address & 0xff;
        length = 1;
    }

    if (TWI_SIMPLE_TIMEOUT == twi_write_block_r(&adau_device->twi, addr, length, true)) {
        return ADAU_TWI_TIMEOUT_ERROR;
    }
    if (TWI_SIMPLE_TIMEOUT == twi_read(&adau_device->twi, value)) {
        return ADAU_TWI_TIMEOUT_ERROR;
    }

    return ADAU_SUCCESS;
}

/**
 * @brief      Writes an 8-bit value to a control register on the ADAU1761 using
 *             its 16-bit control register address.  See the ADAU1761 datasheet
 *             for more info on control registers.
 *
 * @param      adau_device  Pointer to the instance of this driver
 * @param[in]  address      The control register address to write
 * @param[in]  value        The value to write to the control register
 */
BM_ADAU_RESULT adau_write_ctrl_reg(BM_ADAU_DEVICE *adau_device,
                                   uint16_t address,
                                   uint8_t value) {

    uint8_t seq[3], length;

    // Number of bytes this device uses for register addresses for r/w over I2C/TWI
    uint8_t address_bytes = adau_device->address_bytes;

    if (address_bytes == 2) {
        seq[0] = address >> 8;
        seq[1] = address & 0xff;
        seq[2] = value;
        length = 3;
    }
    else {
        seq[0] = address & 0xff;
        seq[1] = value;
        length = 2;
    }

    if (TWI_SIMPLE_TIMEOUT == twi_write_block(&adau_device->twi, seq, length)) {
        return ADAU_TWI_TIMEOUT_ERROR;
    }

    return ADAU_SUCCESS;
}

/**
 * @brief      Writes to the parameter RAM on the ADAU1761 so parameters can be
 *             modified in real time. Note: the DSP within the ADAU1761 needs to
 *             be running before the parameter RAM can be accessed.   This is
 *             usually accomplished by loading a bulk register / memory
 *             configuration file.  Attempting to access parameter accesses.
 *
 * @param      adau_device  Pointer to the instance of this driver
 * @param[in]  address      The parameter memory address to write
 * @param[in]  value        The value to write
 */
BM_ADAU_RESULT adau_write_parameter_ram(BM_ADAU_DEVICE *adau_device,
                                        uint16_t address,
                                        uint32_t value) {

    uint8_t seq[6];
    seq[0] = address >> 8;
    seq[1] = address & 0xff;

    // MSB first
    seq[2] = (value >> 24) & 0xff;
    seq[3] = (value >> 16) & 0xff;
    seq[4] = (value >> 8) & 0xff;
    seq[5] = (value) & 0xff;

    if (TWI_SIMPLE_TIMEOUT == twi_write_block(&adau_device->twi, seq, 6)) {
        return ADAU_TWI_TIMEOUT_ERROR;
    }

    return ADAU_SUCCESS;
}

/**
 * @brief      Reads from the parameter RAM on the ADAU device so parameters can be
 *             modifiedin real time. Note: the DSP within the ADAU device needs to
 *             be running before theparameter RAM can be accessed.   This is
 *             usually accomplished by loading abulk register / memory
 *             configuration file.  Attempting to access parametermemory before
 *             the device has been configured will result in
 *             unpredictable accesses.
 *
 * @param      adau_device  Pointer to the instance of this driver
 * @param[in]  address      The parameter memory address to read
 *
 * @return     The read value
 */
BM_ADAU_RESULT adau_read_parameter_ram(BM_ADAU_DEVICE *adau_device,
                                       uint16_t address,
                                       uint32_t *value) {
    uint8_t addr[2], rx_buffer[4];
    uint32_t result = 0;
    addr[0] = address >> 8;
    addr[1] = address & 0xff;

    if (TWI_SIMPLE_TIMEOUT == twi_write_block_r(&adau_device->twi, addr, 2, true)) {
        return ADAU_TWI_TIMEOUT_ERROR;
    }
    if (TWI_SIMPLE_TIMEOUT == twi_read_block(&adau_device->twi, rx_buffer, 4)) {
        return ADAU_TWI_TIMEOUT_ERROR;
    }

    result |= (rx_buffer[0] << 24);
    result |= (rx_buffer[1] << 16);
    result |= (rx_buffer[2] << 8);
    result |= (rx_buffer[3]);

    *value = result;

    return ADAU_SUCCESS;
}

/**
 * @brief      The SigmaStudio tools can dump a set of configuration files for
 *             easy device set up via Action -> Export System Files.  Amongst
 *             these files will be a length and a data file that need to be
 *             imported (See the ADAU testing code for an example).  This
 *             typically starts the ADAU1761 DSP at the end of the loading
 *             process so device set up becomes incredibly simple.
 *
 * @param      adau_device Pointer to the instance of this driver
 * @param      values      The values
 * @param      lengths     The lengths
 * @param[in]  total_lines  The total lines
 *
 * @return     { description_of_the_return_value }
 */
BM_ADAU_RESULT adau_load_bulk_reg_file(BM_ADAU_DEVICE *adau_device,
                                       uint8_t *values,
                                       uint16_t *lengths,
                                       uint16_t total_lines,
                                       bool ignore_first_byte_of_init_file) {

    int i;
    uint16_t length;

    for (i = 0; i < total_lines; i++) {

        length = *lengths++;

        // Some init files generated by SigmaStudio have an extra address byte which is zero - we ignore these
        if (ignore_first_byte_of_init_file) {
            length -= 1;
            values++;
        }

        // Look for any corrupted values in the initialization
        if (length > 10000 || length == 0) {
            return ADAU_CORRUPT_INIT_FILE;
        }
        else {
            // perform a bulk write
            if (TWI_SIMPLE_TIMEOUT == twi_write_block(&adau_device->twi, values, length)) {
                return ADAU_TWI_TIMEOUT_ERROR;
            }
        }

        // If we're programming the ADAU1761
        if (*(values + 0) == 0x40 && *(values + 1) == 0x02 && length == 8) {
            uint8_t pllLock = 0;

            uint16_t timeout = 10000;
            uint8_t addr[2] = { ADAU1761_REG_PLL_CONTROL_0 >> 8, ADAU1761_REG_PLL_CONTROL_0 & 0xff};

            while (timeout-- && !(pllLock & 0x2)) {
                uint8_t pllvalues[6];

                /*
                 * The control register in the ADAU1761 which contains the PLL lock bit needs to
                 * be read as a group of 6 registers.  The PLL lock but is in the last register.
                 */
                if (TWI_SIMPLE_TIMEOUT == twi_write_block_r(&adau_device->twi, addr, 2, true)) {
                    return ADAU_TWI_TIMEOUT_ERROR;
                }
                if (TWI_SIMPLE_TIMEOUT == twi_read_block(&adau_device->twi, pllvalues, 6)) {
                    return ADAU_TWI_TIMEOUT_ERROR;
                }
                pllLock = pllvalues[5];
            }
            if (timeout == 0) return ADAU_PLL_LOCK_TIMEOUT_ERROR;
        }

        // Update pointer
        values += length;
    }

    return ADAU_SUCCESS;
}

/******************************************************************************
 ******************************************************************************
 *                                                                            *
 *                          DEVICE SPECIFIC SUPPORT                           *
 *                                                                            *
 ******************************************************************************
 *****************************************************************************/

/******************************************************************************
 *                                   ADAU1761                                 *
 *****************************************************************************/

bool adau1761_set_samplerate(BM_ADAU_DEVICE *adau1761,
                             uint32_t sample_rate) {

    int i;

    uint16_t denominator_final = 0;
    uint16_t numerator_final = 0;
    uint8_t r, convsr = 0, mode_fract_int = 0, dspsr = 1, spsr = 0;

    if (sample_rate == 48000) {
        r = 4;
    }
    else if (sample_rate == 96000) {
        r = 4;
        convsr = 6;
        dspsr = 0;
        spsr = 6;
    }
    else if (sample_rate > 48000) {
        return false;
    }
    else if (sample_rate == 44100) {
        denominator_final = 22563;
        numerator_final = 15230;
        r = 3;
        mode_fract_int = 1;
    }
    else {
        float pll_output = 1024.0 * sample_rate;
        float divider = pll_output / (float)SHARC_SAM_MCLK;

        float component_int     = floor(divider);
        float component_fract   = divider - component_int;

        float numerator = 0, denominator = 0;
        float numerator_best = 0, denominator_best = 0;
        float delta_best = 1, x;

        for (i = 0; i < 65535; i++) {
            denominator += 1.0;
            numerator = denominator * component_fract;

            float rounded_numerator = (float)(int)numerator;

            if ((x = numerator - rounded_numerator) < delta_best) {
                delta_best = x;
                denominator_best = denominator;

                // Core agnostic implementation
                numerator_best = rounded_numerator;
            }
        }

        // No suitable fractions found
        if (denominator_best == 0) return false;

        numerator_final = (uint16_t)numerator_best;
        denominator_final = (uint16_t)denominator_best;
        r = (uint16_t)component_int;

        // Out of range
        if (r > 8 || r < 2) return false;

        // Fractional mode
        mode_fract_int = 1;
    }

// Change to #if 1 to see the intermediate PLL values in the ADAU1761
#if 0

    printf("ADAU1761 PLL Denominator: %d\n", denominator_final);
    printf("ADAU1761 PLL Numerator: %d\n", numerator_final);
    printf("ADAU1761 PLL R value: %d\n", r);
    printf("ADAU1761 PLL Mode (Int=0 / Fract=1): %d\n", mode_fract_int);

#endif

    // All PLL values must be written in a single continuous write to the control port
    // (see pg 27 of datasheet)
    uint8_t pll_regs[8];

    pll_regs[0] = ADAU1761_REG_PLL_CONTROL_0 >> 8;
    pll_regs[1] = ADAU1761_REG_PLL_CONTROL_0 & 0xff;
    pll_regs[2] = (uint8_t)(denominator_final >> 8);    // Byte 0 = Denominator MSB
    pll_regs[3] = (uint8_t)(denominator_final & 0xFF);  // Byte 1 = Denominator LSB
    pll_regs[4] = (uint8_t)(numerator_final >> 8);      // Byte 2 = Numerator MSB
    pll_regs[5] = (uint8_t)(numerator_final & 0xFF);    // Byte 3 = Numerator LSB
    pll_regs[6] = (uint8_t)((r << 3) | mode_fract_int);     // Byte 4 = R value, X=1, int / fract mode

    // 1. Disable the core clock
    adau_write_ctrl_reg(adau1761, ADAU1761_REG_CLOCK_CONTROL, 0x0);

    // 2. Power down PLL
    pll_regs[7] = (uint8_t)0x0;                         // Byte 5 = Disable PLL
    twi_write_block(&adau1761->twi, pll_regs, 8);

    pll_regs[7] = (uint8_t)0x1;                         // Byte 5 = Enable PLL
    twi_write_block(&adau1761->twi, pll_regs, 8);

    // 3. wait for PLL to lock
    uint8_t addr[2], result[6], value = 0;
    addr[0] = ADAU1761_REG_PLL_CONTROL_0 >> 8;
    addr[1] = ADAU1761_REG_PLL_CONTROL_0 & 0xff;

    while ((value & 0x2) == 0) {
        twi_write_block_r(&adau1761->twi, addr, 2, true);
        twi_read_block(&adau1761->twi, result, 6);
        value = result[5];
    }

    // 4. Enable core clock and set to PLL mode
    adau_write_ctrl_reg(adau1761, ADAU1761_REG_CLOCK_CONTROL, 0x9);

    if (!mode_fract_int) {

        // 5. Apply ADAU1761 scaling mode
        uint8_t convCtrl;
        adau_read_ctrl_reg(adau1761, ADAU1761_REG_CONVERTER_0, &convCtrl);
        convCtrl &= 0xF8;
        convCtrl |= convsr;

        adau_write_ctrl_reg(adau1761, ADAU1761_REG_CONVERTER_0, convCtrl);

        // Set ADAU1761 DSP sample rate
        adau_write_ctrl_reg(adau1761, ADAU1761_REG_DSP_SAMPLING_RATE_SETTING, dspsr);

        // Set ADAU1761 serial port sampling rate
        adau_write_ctrl_reg(adau1761, ADAU1761_REG_SERIAL_PORT_SAMPLING_RATE, spsr);
    }

    return true;
}

/******************************************************************************
 *                                   ADAU1452                                 *
 *****************************************************************************/

/**
 * @brief      Initialize the ADAU1452 using the auto-generated export file from
 *             SigmaStudio.  The ADAU1452 on the automotive board is connected
 *             via the SPI port thus the standard I2C functions above won't work.
 *
 * @param      adau1452    A pointer to the instance for this driver
 * @param[in]  BM_GPIO_PORTPIN     The GPIO pin to use as the SPI slave select line
 * @param[in]  device_num  The SHARC SPI peripheral to use (e.g. SPI0, SPI1,
 *                         SPI2)
 *
 * @return     Result indicating success or failure
 */
BM_ADAU_RESULT adau1452_initialize(BM_ADAU1452_DEVICE *adau1452,
                                   BM_GPIO_PORTPIN spi_select_pin,
                                   BM_SPI_PERIPHERAL_NUMBER device_num,
                                   BM_ADAU_DEVICE_INIT_DATA *adau_init_data){

    int i, j;
    volatile int v;

    // This delay is necessary after a cold start to ensure the ADAU1452 is ready to be booted
    for (j = 0; j < 100000000; j++) { v++; }

    // Set up simple register-driven TWI driver
    if (spi_initialize(&adau1452->spi,
                       SPI_MODE_3,
                       SPI_SSEL_MANUAL,
                       SPI_WORDLEN_8BIT,
                       112500000,
                       device_num) != SPI_SIMPLE_SUCCESS) {
        return ADAU_SIMPLE_ERROR;
    }

    // We need to manually manage the slave select line since it needs to span multiple bytes per transaction
    adau1452->spi_select_pin = spi_select_pin;

    // Use SPI helper functions to set up that GPIO pin
    gpio_setup(spi_select_pin, GPIO_OUTPUT);

    // Max speed of ADAU1452 SPI clock is ~3MHz until after PLL locks
    // Set initially to 300K
    // TODO - flip this back to 3MHz once everything is working
    spi_setClock(&adau1452->spi, 300000);

    // We need to perform 3 dummy writes to the ADAU1452 to put into SPI mode
    // See page 35 of the datasheet more details.
    for (i = 0; i < 3; i++) {

        spi_select(adau1452->spi_select_pin);

        //< Add a short delay
        for (j = 0; j < 1000; j++) { v++; }

        spi_transfer(&adau1452->spi, 0x0);

        //< Add a short delay
        for (j = 0; j < 1000; j++) { v++; }

        spi_deselect(adau1452->spi_select_pin);

        //< Add a short delay
        for (j = 0; j < 1000; j++) { v++; }
    }

    // Add another delay
    for (j = 0; j < 100000; j++) { v++; }

    // If init file provided, initialize the ADAU1452
    BM_ADAU_RESULT res_adau1452;
    if (adau_init_data) {
        res_adau1452 = adau1452_load_bulk_reg_file(adau1452,
                                                   adau_init_data->data_tx_buffer,
                                                   adau_init_data->data_num_bytes,
                                                   adau_init_data->total_lines);

        if (res_adau1452 != ADAU_SUCCESS) {
            return res_adau1452;
        }
    }

    // Read the core status register to be sure the processor has been initialized
    uint16_t coreStatusReg = adau1452_read_word(adau1452, ADAU1452_REG_CORE_STATUS);

    if ((coreStatusReg & 0x1) != 0x1) {

        return ADAU_SIMPLE_ERROR;
    }

    return ADAU_SUCCESS;
}

/**
 * @brief      Writes a block of memory into the ADAU1452
 *
 * @param      adau1452    The adau 1452
 * @param[in]  subaddress  The subaddress
 * @param      value       The value
 * @param[in]  count       The count
 *
 * @return     BM_ADAU_RESULT enumeration
 */
BM_ADAU_RESULT adau1452_write_block(BM_ADAU1452_DEVICE  *adau1452,
                                    uint16_t address,
                                    uint8_t *value,
                                    uint16_t count) {

    int i, j;

    // Assert the SPI select line
    spi_select(adau1452->spi_select_pin);

    // 7-bit address (0x0) followed by write bit (0x0)
    spi_transfer(&adau1452->spi, 0x0);

    // Write MSB of subaddress
    spi_transfer(&adau1452->spi, (address >> 8) );

    // Write LSB of subaddress
    spi_transfer(&adau1452->spi, (address & 0xFF) );

    for (i = 0; i < count; i++) {

        // Write MSB of subaddress
        spi_transfer(&adau1452->spi, (uint32_t)*value);
        value++;
    }

    // De-assert the SPI select line
    spi_deselect(adau1452->spi_select_pin);

    // Add a short delay
    volatile int v = 0;
    for (j = 0; j < 1000; j++) { v++; }

    return ADAU_SUCCESS;
}

/**
 * @brief      Reads a word from the ADAU1452 memory
 *
 * @param      adau1452    The adau 1452
 * @param[in]  subaddress  The subaddress
 *
 * @return     The 16-bit word that was read
 */
uint16_t adau1452_read_word(BM_ADAU1452_DEVICE *adau1452,
                            uint16_t address) {

    int j;

    // Assert the SPI select line
    spi_select(adau1452->spi_select_pin);

    // 7-bit address (0x0) followed by read bit (0x1)
    spi_transfer(&adau1452->spi, 0x1);

    // Write MSB of subaddress
    spi_transfer(&adau1452->spi, (address >> 8) );

    // Write LSB of subaddress
    spi_transfer(&adau1452->spi, (address & 0xFF) );

    uint16_t value = 0;

    value |= (spi_transfer(&adau1452->spi, 0) << 8);
    value |= spi_transfer(&adau1452->spi, 0);

    // De-assert the SPI select line
    spi_deselect(adau1452->spi_select_pin);

    //< Add a short delay
    volatile int v = 0;
    for (j = 0; j < 1000; j++) { v++; }

    return value;
}

/**
 * @brief      Loads an init file generated by SigmaStudio into the ADAU1452
 *
 * @param      adau1452    The adau 1452
 * @param      values      The values
 * @param      lengths     The lengths
 * @param[in]  total_lines  The total lines
 *
 * @return     BM_ADAU_RESULT enumeration
 */
BM_ADAU_RESULT adau1452_load_bulk_reg_file(BM_ADAU1452_DEVICE *adau1452,
                                           uint8_t *values,
                                           uint16_t *lengths,
                                           uint16_t total_lines) {

    int i, j;
    uint16_t length;
    volatile int v;

    for (i = 0; i < total_lines; i++) {

        uint16_t subaddress;

        length = *lengths++;

        // Look for any corrupted values in the initialization
        if (length > 10000 || length == 0) {
            return ADAU_CORRUPT_INIT_FILE;
        }
        else {

            subaddress = (values[0] << 8 | values[1]);

            // perform a bulk write
            adau1452_write_block(adau1452, subaddress, &values[2], length - 2);

            if (length >= 4) {
                uint16_t firstValue = (values[2] << 8) + values[3];

                // Check to see if we are enabling the PLL because if so, we need to wait a moment
                if (subaddress == ADAU1452_REG_PLL_ENABLE && firstValue == 0x01) {

                    // Wait for PLL to lock
                    uint32_t timeOutCntr = 0;
                    while (adau1452_read_word(adau1452, ADAU1452_REG_PLL_LOCK) != 0x01) {
                        // short delay
                        for (j = 0; j < 10000; j++) { v++; }

                        if (timeOutCntr++ > 10000) {
                            printf(" adau1452_loadBulkRegFile(): Waiting for PLL lock has timed out\n");
                            return ADAU_SIMPLE_ERROR;
                        }
                    }
                }
            }
        }

        values += length;

        // Add a short delay between blocks
        for (j = 0; j < 10000; j++) { v++; }
    }

    return ADAU_SUCCESS;
}
