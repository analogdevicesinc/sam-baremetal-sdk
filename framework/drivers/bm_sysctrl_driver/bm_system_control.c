/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * Bare-Metal ("BM") device driver  for system control support
 *
 * This set of functions provides a few different capabilities:
 *  1) Initializing the system clocks
 *  2) Providing a system "tick" for measuring duration and delays
 *  3) Support for the HADC
 */
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <services/int/adi_int.h>
#include <sys/platform.h>

#include <drivers/hadc/adi_hadc.h>
#include <services/gpio/adi_gpio.h>
#include <services/pwr/adi_pwr.h>
#include <services/tmr/adi_tmr.h>

#include <cdefSC589.h>
#include <defSC589.h>
#include <sruSC589.h>

#include "bm_system_control.h"

//****************************************************************************
// System tick resources (HADC sampling and delay function support)
//****************************************************************************
volatile uint64_t system_milliticks = 0;
ADI_TMR_HANDLE timer_handle;
uint8_t timer_instance_memory[ADI_TMR_MEMORY];

// Global flag to indicate if this core is responsible for managing the HADCs
bool this_core_reads_hadc = false;

/**
 * HADC system resources
 * Note: for newer SHARC Audio Module boards, the HADC_MIN value will change once these inputs
 * are buffered
 */
#define HADC_MAX                (4096.0)
#define HADC_CHANNELS           (7)
#define HADC_CHANNEL_MASK       (0xFF80)
#define HADC_CHANNEL_MASK_INV   (0x7F)

ADI_HADC_HANDLE hadc_handle;
uint8_t hadc_instance_memory[ADI_HADC_MEM_SIZE];

// Channels for converted data
uint16_t hadc_channeldata_raw[HADC_CHANNELS] = {0};
uint16_t hadc_channeldata_raw_last[HADC_CHANNELS] = {8192};

static float hadc_channeldata_float[HADC_CHANNELS]={0.0};
static float hadc_channeldata_float_pre[HADC_CHANNELS]={0.0};


void (*one_ms_tick_callback)(void) = NULL;

/**
 * @brief      This is a simple "tick" interrupt handler that is triggered once
 *             every millisecond.  It supports the delay() function and also
 *             triggers the next round of HADC conversions.
 *
 * @param      pCBParam  The cb parameter
 * @param[in]  Event     The event
 * @param      pArg      The argument
 */
static void systemtimer_handler(void *pCBParam,
                                uint32_t Event,
                                void *pArg) {

    int i;

    switch (Event)
    {
        case ADI_TMR_EVENT_DATA_INT:

            // Increment our ticks using a big, unsigned 64-bit int.  This is
            // used to support the delay() and millis() functions
            system_milliticks++;

            // If this core is responsible for managing the HADCs, do that!
            if (this_core_reads_hadc) {

                // Normalize our last set of conversions (from 0.0 - 1.0)
                for (i = 0; i < HADC_CHANNELS; i++) {

                    // Add some hysteresis to remove noise when they POTs aren't changing
					float cur_val = ((float)hadc_channeldata_raw[i]) * (1.0 / HADC_MAX);
					hadc_channeldata_float_pre[i] += 0.01*(cur_val - hadc_channeldata_float_pre[i]);
					hadc_channeldata_float[i] = floor(hadc_channeldata_float_pre[i]*1000.0)*.001;

                }

                // Wait until the conversion sequence to is complete (although it
                // should be done by now since we're in a 1KHz loop
                ADI_HADC_RESULT result_HADC;

                // Get converted data
                result_HADC = adi_hadc_GetConvertedData(hadc_handle, HADC_CHANNEL_MASK_INV, hadc_channeldata_raw);
                if (ADI_HADC_SUCCESS != result_HADC) {
                    // handle HADC errors here
                }

                // Kick off the next conversion for next time through the timer
                // loop (so we're not wasting time waiting)
                result_HADC = adi_hadc_StartConversion(hadc_handle, true);
                if (ADI_HADC_SUCCESS != result_HADC) {
                    // handle HADC errors here
                }
            }

            // If a user callback has been set for the 1ms tick event, call it
            if (one_ms_tick_callback != NULL) {
                one_ms_tick_callback();
            }
            break;

        default:
            break;
    }
}

/**
 * @brief      Configures clocks, power, HADC and system tick
 *
 * * simple_sysctrl_init() sets up the various back-end functionality required.  It
 * should be called before any other functions in this library.  This library can
 * be used on all 3 cores.  However, one core needs to be the master.
 * This core will maintain the system ticks used by the delay function,
 * set up system clocks, etc. When calling the init function on the core
 * that will handle this housekeeping (likely the ARM), set masterCore = true.
 * Otherwise, leave it as false if WS is being used on additional cores.
 *
 * @param[in]  initializeSysClks  This core will initialize the system clocks
 * @param[in]  controlHADC         This core will initialize and manage the HADCs
 * @param[in]  enableTimerTick    This will have a timer tick event every 1 ms
 * @param[in]  timerId            ID of timer resources to use (0-6).  Ensure each core must use a different timer
 */
BM_SYSCTRL_RESULT simple_sysctrl_init(uint32_t ext_oscillator_freq,
                                      uint32_t core_clock_freq,
                                      uint32_t sys_clock_freq,
                                      uint32_t sclk_clock_freq,
                                      bool initialize_sys_clks,
                                      bool control_hadc,
                                      bool enable_timer_tick,
                                      uint8_t timer_id) {

    // If this core should initialize the system clocks...
    if (initialize_sys_clks) {

        // Set up clocks for SHARC Audio Module board (based on 25MHz external oscillator)
        if (adi_pwr_Init(0, ext_oscillator_freq)
            != ADI_PWR_SUCCESS) {
            return SYSCTRL_INVALID_CLOCK_SETTING;
        }

        if (adi_pwr_SetPowerMode(0, ADI_PWR_MODE_FULL_ON)
            != ADI_PWR_SUCCESS) {
            return SYSCTRL_INVALID_CLOCK_SETTING;
        }

        // Set our output clock (OCLK_0) at ~150MHz for SPDIF (450MHz / 3)
        if (adi_pwr_SetClkDivideRegister(0, ADI_PWR_CLK_DIV_OSEL, 3)
            != ADI_PWR_SUCCESS) {
            return SYSCTRL_INVALID_CLOCK_SETTING;
        }

        // Set up clocks for main system
        if (adi_pwr_SetFreq(0, core_clock_freq, sys_clock_freq)
            != ADI_PWR_SUCCESS) {
            return SYSCTRL_INVALID_CLOCK_SETTING;
        }

        // Ensure SPDIF connected to right clock in CDU
        if (adi_pwr_ConfigCduInputClock(ADI_PWR_CDU_CLKIN_0, ADI_PWR_CDU_CLKOUT_5)
            != ADI_PWR_SUCCESS) {
            return SYSCTRL_INVALID_CLOCK_SETTING;
        }

        // Read values back to ensure clocks are set the way we think they are
        uint32_t fsysclk = 0, fsclk0 = 0, fsclk1 = 0;
        if ((adi_pwr_GetSystemFreq(0, &fsysclk, &fsclk0, &fsclk1))
            != ADI_PWR_SUCCESS) {
            return SYSCTRL_INVALID_CLOCK_SETTING;
        }

        if (fsysclk != sys_clock_freq) {
            return SYSCTRL_CLOCK_PWR_INIT_ERROR;
        }
        else if (fsclk0 != sclk_clock_freq) {
            return SYSCTRL_CLOCK_PWR_INIT_ERROR;
        }
        else if (fsclk1 != sclk_clock_freq) {
            return SYSCTRL_CLOCK_PWR_INIT_ERROR;
        }
    }

    // If this core is responsible for initializing and polling the HADCs
    if (control_hadc) {
        this_core_reads_hadc = true;

        // Open the HADC driver
        if (adi_hadc_Open(0, hadc_instance_memory, &hadc_handle) != ADI_HADC_SUCCESS) {
            return SYSCTRL_HADC_INIT_ERROR;
        }

        // Set the channel mask for the channels to be converted
        if (adi_hadc_SetChannelMask(hadc_handle, HADC_CHANNEL_MASK) != ADI_HADC_SUCCESS) {
            return SYSCTRL_HADC_INIT_ERROR;
        }

        if (adi_hadc_SetSampleFreqDivFactor(hadc_handle, 1) != ADI_HADC_SUCCESS) {
            return SYSCTRL_HADC_INIT_ERROR;
        }

        if (adi_hadc_SetNumConversions(hadc_handle, 1) != ADI_HADC_SUCCESS) {
            return SYSCTRL_HADC_INIT_ERROR;
        }

        // Kick off first conversion
        if (adi_hadc_StartConversion(hadc_handle, true) != ADI_HADC_SUCCESS) {
            return SYSCTRL_HADC_INIT_ERROR;
        }
    }

    if (enable_timer_tick) {

        // Initialize 1ms timer
        if (adi_tmr_Open(timer_id,
                         timer_instance_memory,
                         ADI_TMR_MEMORY,
                         systemtimer_handler,
                         NULL,
                         &timer_handle) != ADI_TMR_SUCCESS) {
            return SYSCTRL_TIMER_INIT_ERROR;
        }

        // Set the mode to PWM OUT
        if (adi_tmr_SetMode(timer_handle, ADI_TMR_MODE_CONTINUOUS_PWMOUT)
            != ADI_TMR_SUCCESS) {
            return SYSCTRL_TIMER_INIT_ERROR;
        }

        // Set the IRQ mode to get interrupt after timer counts to Delay + Width
        if (adi_tmr_SetIRQMode(timer_handle, ADI_TMR_IRQMODE_WIDTH_DELAY)
            != ADI_TMR_SUCCESS) {
            return SYSCTRL_TIMER_INIT_ERROR;
        }

        // Set the Period - once per ms
        if (adi_tmr_SetPeriod(timer_handle, sclk_clock_freq / 1000)
            != ADI_TMR_SUCCESS) {
            return SYSCTRL_TIMER_INIT_ERROR;
        }

        // Set the timer width
        if (adi_tmr_SetWidth(timer_handle, sclk_clock_freq / 1000 / 2 - 1)
            != ADI_TMR_SUCCESS) {
            return SYSCTRL_TIMER_INIT_ERROR;
        }

        // Set the timer delay
        if (adi_tmr_SetDelay(timer_handle, sclk_clock_freq / 1000 / 2)
            != ADI_TMR_SUCCESS) {
            return SYSCTRL_TIMER_INIT_ERROR;
        }

        // Enable the timer
        if ((adi_tmr_Enable(timer_handle, true))
            != ADI_TMR_SUCCESS) {
            return SYSCTRL_TIMER_INIT_ERROR;
        }
    }

    return SYSCTRL_SUCCESS;
}

/**
 * @brief Sets a callback for the 1ms tick event
 *
 * @param tick_callback function pointer to callback
 */
void simple_sysctrl_set_1ms_callback(void (*tick_callback)(void)) {

    // If a callback has been provided to be called when a 1ms tick event occurs, set it here
    if (tick_callback != NULL) {
        one_ms_tick_callback = tick_callback;
    }
}

//******************************************************************************
//  Delay and tick support
//******************************************************************************

/**
 * @brief      Get elapsed number of milliseconds
 *
 * Note that this function only works if a simple_sysctrl_init() has been called with
 * enable_timer_tick set to true
 *
 * @return     Returns elapsed number of milliseconds
 */
uint64_t millis(void) {
    return system_milliticks;
}

/**
 * @brief      Delay a certain number of milliseconds
 *
 * Note that this function only works if a simple_sysctrl_init() has been called with
 * enable_timer_tick set to true
 *
 * @param[in]  delay  Milliseconds to delay
 */
void delay(unsigned long delay) {

    uint64_t milli_delay = delay + system_milliticks;

    // Check if we wrapped
    if (milli_delay < system_milliticks) {
        while (milli_delay < system_milliticks) {}
        while (milli_delay > system_milliticks) {}
    }
    else {
        while (milli_delay > system_milliticks) {}
    }
}

/**
 * @brief      Returns the last read HADC value as a float
 *
 * @param[in]  pin   The HADC pin
 *
 * @return     float value from 0 to 1.0 of HADC value for that pine
 */
float hadc_read_float(uint8_t pin) {

    if (pin >= HADC_CHANNELS) return 0.0;

    return hadc_channeldata_float[pin];
}

/**
 * @brief      Returns the last read HADC value as a 12-bit int.
 *
 * @param[in]  pin   The HADC pin
 *
 * @return     Int from 0 to 4095
 */
uint16_t hadc_read(uint8_t pin) {

    if (pin >= HADC_CHANNELS) return 0;

    return hadc_channeldata_raw[pin];
}
