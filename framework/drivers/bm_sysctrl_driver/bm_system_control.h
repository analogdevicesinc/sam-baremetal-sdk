/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * Bare-Metal ("BM") device driver header file for system control support
 */

#ifndef _BM_SYSCTRL_H
#define _BM_SYSCTRL_H

#include <stdbool.h>
#include <stdint.h>

typedef enum _BM_SYSCTRL_RESULT
{
    SYSCTRL_SUCCESS,                // The API call is success
    SYSCTRL_INVALID_CLOCK_SETTING,  // An invalid clock value was provided
    SYSCTRL_CLOCK_PWR_INIT_ERROR,   // An error occurred while initialing power / clock
    SYSCTRL_HADC_INIT_ERROR,        // An error occurred while initializing the HADC
    SYSCTRL_TIMER_INIT_ERROR        // An error occurred while initialing the TIMER
} BM_SYSCTRL_RESULT;

#ifdef __cplusplus
extern "C" {
#endif

// Simple SysCtrl init function
BM_SYSCTRL_RESULT simple_sysctrl_init(uint32_t ext_oscillator_freq,
                                      uint32_t core_clock_freq,
                                      uint32_t sys_clock_freq,
                                      uint32_t sclk_clock_freq,
                                      bool initialize_sys_clks,
                                      bool control_hadc,
                                      bool enable_timer_tick,
                                      uint8_t timer_id);

// Sets a callback for the 1ms tick event
void simple_sysctrl_set_1ms_callback(void (*tick_callback)(void));

// Reads HADC captured values
uint16_t hadc_read(uint8_t);
float hadc_read_float(uint8_t pin);

// Delay and timing functions
uint64_t millis(void);
void delay(unsigned long);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _BM_SYSCTRL_H
