/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 */
#ifndef _CALLBACK_MIDI_MESSAGE_H
#define _CALLBACK_MIDI_MESSAGE_H

// Define your audio system parameters in this file
#include "common/audio_system_config.h"

// UART functionality for MIDI driver on Audio Project Fin
#include "drivers/bm_uart_driver/bm_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

extern BM_UART MIDI_UART_ARM;

// MIDI set up routine
bool midi_setup_arm(void);

// MIDI receive byte callback
void midi_rx_callback_arm(void);

#ifdef __cplusplus
}
#endif

#endif     // _CALLBACK_MIDI_MESSAGE_H
