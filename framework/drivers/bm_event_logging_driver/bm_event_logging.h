/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * Bare-Metal ("BM") device driver header file event logging
 *
 */
#ifndef _BM_EVENT_LOGGING_H_
#define _BM_EVENT_LOGGING_H_

#include <stdio.h>
#include <string.h>

#include "drivers/bm_gpio_driver/bm_gpio.h"
#include "drivers/bm_uart_driver/bm_uart.h"

// Global event messaging parameters
#define EVENT_LOG_MESSAGE_LEN        (128)
#define EVENT_LOG_QUEUE_LENGTH       (128)
#define EVENT_LOG_QUEUE_LENGTH_SHARC (16)
#define EVENT_LOG_PRINT_DAYS         (false)

// State and data structs
typedef enum {
    EVENT_NONE = 0,
    EVENT_DEBUG,
    EVENT_INFO,
    EVENT_WARN,
    EVENT_ERROR,
    EVENT_FATAL
} BM_SYSTEM_EVENT_LEVEL;

typedef enum {
    EVENT_SRC_ARM,
    EVENT_SRC_SHARC_CORE1,
    EVENT_SRC_SHARC_CORE2
} BM_SYSTEM_EVENT_SOURCE;

typedef struct
{
    char message[EVENT_LOG_MESSAGE_LEN];
    BM_SYSTEM_EVENT_LEVEL event_level;
    BM_SYSTEM_EVENT_SOURCE event_source;
    uint32_t time_milliseconds;
    uint8_t time_seconds;
    uint8_t time_minutes;
    uint8_t time_hours;
    uint32_t time_days;
} BM_SYSTEM_EVENT;

typedef struct
{
    uint64_t event_emuclk;
    uint32_t event_level;
    char message[EVENT_LOG_MESSAGE_LEN];
} BM_SYSTEM_EVENT_SHARC;

typedef struct
{

    // Pointers into shared L2 memory for retrieving messages from SHARCs
    char *sharc_core_1_shared_message;
    char *sharc_core_2_shared_message;
    uint32_t*sharc_core_1_shared_emuclk;
    uint32_t*sharc_core_1_shared_emuclk2;
    uint32_t *sharc_core_2_shared_emuclk;
    uint32_t *sharc_core_2_shared_emuclk2;
    uint32_t *sharc_core_1_shared_level;
    uint32_t *sharc_core_2_shared_level;

    // SHARC EMUCLK (cycle count) values
    uint64_t sharc_core_1_emuclk;
    uint64_t sharc_core_2_emuclk;
    uint64_t sharc_core_1_emuclk_calib;
    uint64_t sharc_core_2_emuclk_calib;

    // Shared semaphore to know when new messages ready
    uint32_t *sharc_core1_shared_semaphore;
    uint32_t *sharc_core2_shared_semaphore;

    // Call back for ERROR and FATAL events
    void (*error_handling_callback)(uint32_t, void *);

    // UART support
    bool send_events_to_uart;
    BM_UART uart_instance;

    // System clock frequency for calculating time stamps
    float core_clock_frequency_hz;

    // Indication that we got backed up and lost a message
    bool messages_dropped;

    // Event log
    BM_SYSTEM_EVENT event_log[EVENT_LOG_QUEUE_LENGTH];
    uint16_t event_log_write_indx;
    uint16_t event_log_read_indx;
} BM_EVENT_LOGGER_STATE;

extern BM_EVENT_LOGGER_STATE event_logger_state;

#ifdef __cplusplus
extern "C" {
#endif

// SHARC and ARM - log an event as string or constant
bool log_event(BM_SYSTEM_EVENT_LEVEL level,
               char *message);

// ARM only - Inializes event messaging on the ARM core
void event_logging_initialize_arm(char *core_1_shared_message,
                                  char *core_2_shared_message,
                                  uint32_t *core_1_shared_emuclk,
                                  uint32_t *core_2_shared_emuclk,
                                  uint32_t *core_1_shared_emuclk2,
                                  uint32_t *core_2_shared_emuclk2,
                                  uint32_t *core_1_shared_level,
                                  uint32_t *core_2_shared_level,
                                  uint32_t *core_1_shared_semaphore,
                                  uint32_t *core_2_shared_semaphore,
                                  float core_clock_freq_hz);

// ARM only - connects messaging system to UART
bool event_logging_connect_uart(BM_UART_BAUD_RATE baud,
                                BM_UART_CONFIG config,
                                uint32_t device_num);

// ARM only - sets a callback for error and fatal messages
bool event_logging_set_error_callback(void (*error_callback)(uint32_t, void *));

// ARM only - polling function to move messages from SHARCs, and to UART
void event_logging_poll_sharc_cores_for_new_message(void);

// SHARC only - Initializes event messaging on a SHARC core
bool event_logging_initialize_sharc_core(char *shared_event,
                                         uint32_t *shared_emuclk,
                                         uint32_t *shared_emuclk2,
                                         uint32_t *shared_level,
                                         uint32_t *shared_semaphore);

// SHARC only - polling function to move messages to ARM
bool event_logging_process_queue_sharc_core(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif    // _BM_EVENT_LOGGING_H_
