/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * These are the hooks for the MIDI / Serial processing functions.
 *
 */
#include <stdint.h>

// Define your audio system parameters in this file
#include "common/audio_system_config.h"

/**
 * UART / MIDI messages can be processed either by the ARM core or by SHARC Core 1.
 * Select which option in the audio_system_config.h file.
 */
#if defined(MIDI_UART_MANAGED_BY_SHARC1_CORE) && (MIDI_UART_MANAGED_BY_SHARC1_CORE)

// Driver for UART / MIDI functionality for Audio Project Fin
#include "drivers/bm_uart_driver/bm_uart.h"

// Event logging / error handling / functionality
#include "drivers/bm_event_logging_driver/bm_event_logging.h"

#include "callback_midi_message.h"

// Create an instance of our MIDI UART driver
BM_UART midi_uart_sharc1;

/**
 * @brief Sets up MIDI on the SHARC Core 1
 *
 * @return true if successful
 */
bool midi_setup_sharc1(void) {

    if (uart_initialize(&midi_uart_sharc1, UART_BAUD_RATE_MIDI, UART_SERIAL_8N1, UART_AUDIOPROJ_DEVICE_MIDI)
        != UART_SUCCESS) {
        return false;
    }

    // Set our user call back for received MIDI bytes
    uart_set_rx_callback(&midi_uart_sharc1, midi_rx_callback_sharc1);

    return true;
}

/**
 * @brief Callback when new MIDI bytes arrive
 */
void midi_rx_callback_sharc1(void) {

    uint8_t val;

    // Keep reading bytes from MIDI FIFO until we have processed all of them
    while (uart_available(&midi_uart_sharc1)) {

        // Replace the uart_read_byte() / uart_write_byte() functions below with any custom code
        // This code just passes the received MIDI byte back to MIDI out

        // Read the new byte
        uart_read_byte(&midi_uart_sharc1, &val);

        // Write that byte back to MIDI TX
        uart_write_byte(&midi_uart_sharc1, val);
    }
}

#endif
