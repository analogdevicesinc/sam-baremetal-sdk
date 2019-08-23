/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * Bare-Metal ("BM") device driver for event logging.
 *
 * This provides a number of functions for logging events
 *
 * @file       event_logging_simple.c
 * @brief      basic event logging functionality
 */
#include <math.h>
#include <services/int/adi_int.h>

#include "drivers/bm_sysctrl_driver/bm_system_control.h"

#include "bm_event_logging.h"

/**
 * The code below is only compiled on the ARM processor not on the SHARC cores
 */
#if defined (CORE0)

#include <services/int/adi_gic.h>

// State structure that keeps track of everything related to the event monitoring
BM_EVENT_LOGGER_STATE event_logger_state;

// Function prototypes
static void event_logging_make_timestamp(BM_SYSTEM_EVENT *event,
                                         uint64_t millis_timestamp,
                                         uint64_t emuclk);
static bool event_logging_send_event_to_uart(BM_SYSTEM_EVENT *event);
static bool event_logging_add_local_event(BM_SYSTEM_EVENT_LEVEL event_level,
                                          char *message,
                                          BM_SYSTEM_EVENT_SOURCE event_source);
static void event_logging_service_uart(void);

/**
 * @brief Sets a callback when a ERROR or FATAL message is logged
 *
 * @param error_callback pointer to user callback function
 * @return True if successful, false if not
 */
bool event_logging_set_error_callback(void (*error_callback)(uint32_t, void *)) {

    // Ensure the callback is not a null pointer
    if (error_callback != NULL) {

        // Set pointer in our global state structure
        event_logger_state.error_handling_callback = error_callback;

        // TODO: get this working with software interrupt
        return true;
    }

    return false;
}

/**
 * @brief Initializes the event logging system on the ARM
 *
 * The event logging system uses a handful of variables in shared L2
 * memory that allows it to receive messages from the SHARC cores.
 * The various memory pointers that this function takes as arguments
 * are these shared variables.
 *
 * @param core_1_shared_message pointer to shared message text buffer (core 1)
 * @param core_2_shared_message pointer to shared message text buffer (core 2)
 * @param core_1_shared_emuclk pointer to shared emuclk value (core 1)
 * @param core_2_shared_emuclk pointer to shared emuclk value (core 2)
 * @param core_1_shared_emuclk2 pointer to shared emuclk2 value (core 1)
 * @param core_2_shared_emuclk2 pointer to shared emuclk2 value (core 2)
 * @param core_1_shared_level pointer to shared event level value (core 1)
 * @param core_2_shared_level pointer to shared event level value (core 2)
 * @param core_1_shared_semaphore pointer to shared new mssage ready semaphore (core 1)
 * @param core_2_shared_semaphore pointer to shared new mssage ready semaphore (core 2)
 * @param core_clock_freq_hz frequency of processor (to convert emuclk to millis)
 *
 */
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
                                  float core_clock_freq_hz) {

    // Set the various pointers in the state structure
    event_logger_state.sharc_core_1_shared_message = core_1_shared_message;
    event_logger_state.sharc_core_2_shared_message = core_2_shared_message;
    event_logger_state.sharc_core_1_shared_emuclk = core_1_shared_emuclk;
    event_logger_state.sharc_core_2_shared_emuclk = core_2_shared_emuclk;
    event_logger_state.sharc_core_1_shared_emuclk2 = core_1_shared_emuclk2;
    event_logger_state.sharc_core_2_shared_emuclk2 = core_2_shared_emuclk2;
    event_logger_state.sharc_core_1_shared_level = core_1_shared_level;
    event_logger_state.sharc_core_2_shared_level = core_2_shared_level;
    event_logger_state.sharc_core1_shared_semaphore = core_1_shared_semaphore;
    event_logger_state.sharc_core2_shared_semaphore = core_2_shared_semaphore;

    // Save core clock frequency so we can turn SHARC cycles to millis
    event_logger_state.core_clock_frequency_hz = core_clock_freq_hz;

    event_logger_state.event_log_write_indx = 0;
    event_logger_state.event_log_read_indx = 0;
    event_logger_state.messages_dropped = 0;
}

/**
 * @brief Directs all messages to the UART
 * The event logger can utilize the UART peripheral (available on P8 on the
 * SHARC Audio Module) to send messages to a terminal.  This often a better
 * solution than using printf via the emulator as they're much faster,
 * can be used in real-time systems and don't require the emulator connection
 * to work.
 *
 * @param baud Baud value from bm_uart.h
 * @param config 8N1 config from bm_uart.h
 * @param device_num which uart to use.  UART0 (0) connects to the P8 connector.

 * @return True if successful, false if not
 */
bool event_logging_connect_uart(BM_UART_BAUD_RATE baud,
                                BM_UART_CONFIG config,
                                uint32_t device_num) {

    if (uart_initialize(&event_logger_state.uart_instance, baud, config, device_num) != UART_SUCCESS) {
        return false;
    }

    event_logger_state.send_events_to_uart = true;

    uart_write_byte(&event_logger_state.uart_instance, 0x0C);      // clear the screen

    return true;
}

/**
 * @brief Logs an event
 *
 * Logs an event based on the provided string and event level
 *
 * @param level See .h file for valid enumeration inputs
 * @param message Pointer to a string containing the message
 * @return True
 */
bool log_event(BM_SYSTEM_EVENT_LEVEL level,
               char *message) {

    event_logging_add_local_event(level, message, EVENT_SRC_ARM);

    return true;
}

/**
 * @brief polling routine to being over messages from the SHARC cores
 *
 * This function should be periodically called to bring messages from the SHARC cores
 * over to the ARM and to manage the flow of messages to the UART if the UART is being
 * used.  Placing this function in the ISR of the 1ms timer tick within the baremetal
 * framework provides low message latency.
 */
void event_logging_poll_sharc_cores_for_new_message(void) {

    bool increment_pointer;
    bool call_error_callback = false;

    // Check if we've dropped a message and if so, display a status message
    if (event_logger_state.send_events_to_uart && event_logger_state.messages_dropped == true) {
        char msg[128] = "\r\n<LOGGING ERROR - TRANSMIT FIFO FULL, MESSAGE(S) DROPPED>";
        if (uart_available_for_write(&event_logger_state.uart_instance) > strlen(msg) + 1) {
            uart_write_block(&event_logger_state.uart_instance, (uint8_t *)msg, strlen(msg));
            event_logger_state.messages_dropped = false;
        }
        event_logging_service_uart();
    }

    // Check SHARC Core 1 for messages
    if (*event_logger_state.sharc_core1_shared_semaphore) {

        event_logger_state.sharc_core_1_emuclk = ((uint64_t)(*event_logger_state.sharc_core_1_shared_emuclk)) +
                                                 (((uint64_t)(*event_logger_state.sharc_core_1_shared_emuclk2)) << 32);

        if (!event_logger_state.sharc_core_1_emuclk_calib) {
            event_logger_state.sharc_core_1_emuclk_calib = event_logger_state.sharc_core_1_emuclk;
            uint64_t emuclk_ticks = (uint64_t)(event_logger_state.core_clock_frequency_hz / 1000.0);
            event_logger_state.sharc_core_1_emuclk_calib -= millis() * emuclk_ticks;
        }
        event_logger_state.sharc_core_1_emuclk -= event_logger_state.sharc_core_1_emuclk_calib;

        // If there's a new message, copy it from the shared memory structure
        strcpy(event_logger_state.event_log[event_logger_state.event_log_write_indx].message,
               event_logger_state.sharc_core_1_shared_message);
        event_logger_state.event_log[event_logger_state.event_log_write_indx].event_level    = (BM_SYSTEM_EVENT_LEVEL)(*event_logger_state.sharc_core_1_shared_level);
        event_logger_state.event_log[event_logger_state.event_log_write_indx].event_source   = EVENT_SRC_SHARC_CORE1;

        // Create a timestamp for this event based on the emuclk for that core
        event_logging_make_timestamp(&event_logger_state.event_log[event_logger_state.event_log_write_indx],
                                     0,
                                     event_logger_state.sharc_core_1_emuclk);

        // Clear the semaphore so the SHARC can post its next message
        (*event_logger_state.sharc_core1_shared_semaphore) = false;

        // If an error is encountered and an error callback provided, call the user callback
        if (event_logger_state.event_log[event_logger_state.event_log_write_indx].event_level == EVENT_FATAL ||
            event_logger_state.event_log[event_logger_state.event_log_write_indx].event_level == EVENT_ERROR) {
            call_error_callback = true;
        }

        // If we're using the UART, only increment the write pointer if we're not going beyond the size of our FIFO
        increment_pointer = true;
        if (event_logger_state.send_events_to_uart) {
            if ((event_logger_state.event_log_write_indx + 1) % EVENT_LOG_QUEUE_LENGTH == event_logger_state.event_log_read_indx) {
                increment_pointer = false;
            }
        }
        else {
            // Handle FIFO overflow
            increment_pointer = false;
            event_logger_state.messages_dropped = true;
        }

        // Manage our queue pointer
        if (increment_pointer) {
            event_logger_state.event_log_write_indx++;
            if (event_logger_state.event_log_write_indx >= EVENT_LOG_QUEUE_LENGTH) {
                event_logger_state.event_log_write_indx = 0;
            }
        }
    }

    // Check SHARC Core 2 for messages
    if (*event_logger_state.sharc_core2_shared_semaphore) {

        event_logger_state.sharc_core_2_emuclk = ((uint64_t)(*event_logger_state.sharc_core_2_shared_emuclk)) +
                                                 (((uint64_t)(*event_logger_state.sharc_core_2_shared_emuclk2)) << 32);

        if (!event_logger_state.sharc_core_2_emuclk_calib) {
            event_logger_state.sharc_core_2_emuclk_calib = event_logger_state.sharc_core_2_emuclk;
            uint64_t emuclk_ticks = (uint64_t)(event_logger_state.core_clock_frequency_hz / 1000.0);
            event_logger_state.sharc_core_2_emuclk_calib -= millis() * emuclk_ticks;
        }
        event_logger_state.sharc_core_2_emuclk -= event_logger_state.sharc_core_2_emuclk_calib;

        // If there's a new message, copy it from the shared memory structure
        strcpy(event_logger_state.event_log[event_logger_state.event_log_write_indx].message,
               event_logger_state.sharc_core_2_shared_message);
        event_logger_state.event_log[event_logger_state.event_log_write_indx].event_level    = (BM_SYSTEM_EVENT_LEVEL)(*event_logger_state.sharc_core_2_shared_level);
        event_logger_state.event_log[event_logger_state.event_log_write_indx].event_source   = EVENT_SRC_SHARC_CORE2;

        // Create a timestamp for this event based on the emuclk for that core
        event_logging_make_timestamp(&event_logger_state.event_log[event_logger_state.event_log_write_indx], 0, event_logger_state.sharc_core_2_emuclk);

        // Clear the semaphore so the SHARC can post its next message
        *event_logger_state.sharc_core2_shared_semaphore = false;

        // If an error is encountered and an error callback provided, call the user callback
        if (event_logger_state.event_log[event_logger_state.event_log_write_indx].event_level == EVENT_FATAL ||
            event_logger_state.event_log[event_logger_state.event_log_write_indx].event_level == EVENT_ERROR) {
            call_error_callback = true;
        }

        // If we're using the UART, only increment the write pointer if we're not go beyond the size of our FIFO
        increment_pointer = true;
        if (event_logger_state.send_events_to_uart) {
            if ((event_logger_state.event_log_write_indx + 1) % EVENT_LOG_QUEUE_LENGTH == event_logger_state.event_log_read_indx) {
                increment_pointer = false;
            }
        }
        else {
            // Handle FIFO overflow
            increment_pointer = false;
            event_logger_state.messages_dropped = true;
        }

        // Manage our queue pointer
        if (increment_pointer) {
            event_logger_state.event_log_write_indx++;
            if (event_logger_state.event_log_write_indx >= EVENT_LOG_QUEUE_LENGTH) {
                event_logger_state.event_log_write_indx = 0;
            }
        }
    }

    // Send a few messages to the UART depending on how much room we have in the transmit fifo
    event_logging_service_uart();

    // Do this at the very end so we can send out the error message to the UART (above) first.
    if (call_error_callback && (event_logger_state.error_handling_callback != NULL)) {

        (*event_logger_state.error_handling_callback)(0, 0);

        // TODO - get this working with interrupts
    }
}

/**
 * @brief If we're using the UART, send any messages to the UART FIFO
 * This function should be called periodically to move any new messages that have arrived into
 * the UART FIFO
 *
 */
static void event_logging_service_uart(void) {

    // If we're sending events to the UART, see if we have room in the FIFO to send some
    if (event_logger_state.send_events_to_uart) {
        if (event_logger_state.event_log_read_indx != event_logger_state.event_log_write_indx) {
            bool result = false;
            do {

                result = event_logging_send_event_to_uart(&event_logger_state.event_log[event_logger_state.event_log_read_indx]);

                // if there was room in the FIFO to send that message, update pointers
                if (result) {

                    event_logger_state.event_log_read_indx++;
                    if (event_logger_state.event_log_read_indx >= EVENT_LOG_QUEUE_LENGTH) {
                        event_logger_state.event_log_read_indx = 0;
                    }

                    // If that was the last message in the queue, stop
                    if (event_logger_state.event_log_read_indx == event_logger_state.event_log_write_indx) {
                        result = false;
                    }
                }
            } while (result == true);
        }
    }
}

/**
 * @brief sends a message to the UART
 *
 * This function generates a line of text that is sent to the UART which includes
 * the time stamp, the event level, the source of the event (which core) and the
 * text from the event.
 *
 * @param event A pointer to the event object (struct)
 * @return False if no room in the UART FIFO, True if transmitted
 */
static bool event_logging_send_event_to_uart(BM_SYSTEM_EVENT *event) {

    char stamp[64];

    char *event_level;
    char *event_source;

    uint16_t bytes_available_for_write;

    #if (EVENT_LOG_PRINT_DAYS)
    sprintf(stamp, "\r\n%04d : %02d:%02d:%02d.%04d",
            (int)event->time_days,
            (int)event->time_hours,
            (int)event->time_minutes,
            (int)event->time_seconds,
            (int)event->time_milliseconds);
    #else
    sprintf(stamp, "\r\n%02d:%02d:%02d.%03d",
            (int)event->time_hours,
            (int)event->time_minutes,
            (int)event->time_seconds,
            (int)event->time_milliseconds);
    #endif

    char event_level_string[6][24] = {    " ",
                                          " \033[0;32m[DEBUG - ",
                                          " \033[0;36m[INFO - ",
                                          " \033[0;33m[WARN - ",
                                          " \033[;31m[ERROR - ",
                                          " \033[1;31m[FATAL - "};

    event_level = event_level_string[event->event_level];

    if (event->event_source == EVENT_SRC_ARM) {
        event_source = "ARM]\033[0m  ";
    }
    else if (event->event_source == EVENT_SRC_SHARC_CORE1) {
        event_source = "SHARC CORE 1]\033[0m  ";
    }
    else if (event->event_source == EVENT_SRC_SHARC_CORE2) {
        event_source = "SHARC CORE 2]\033[0m  ";
    }
    else {
    	event_source = "UNKNOWN]\033[0m  ";
    }

    bytes_available_for_write = uart_available_for_write(&event_logger_state.uart_instance);

    if (bytes_available_for_write > 1024) {
        bytes_available_for_write  = 0;
    }

    // Concatenate strings into a single string for UART
    char uart_message[EVENT_LOG_MESSAGE_LEN + 64] = "";
    strcat(uart_message, stamp);
    strcat(uart_message, event_level);
    strcat(uart_message, event_source);
    strcat(uart_message, event->message);

    uint16_t string_length = strlen(uart_message);

    // If we have room in UART FIFO, send along
    if (bytes_available_for_write >= string_length) {
        uart_write_block(&event_logger_state.uart_instance, (uint8_t *)uart_message, string_length);
        return true;
    }

    // Otherwise, return false and we can try again next time
    return false;
}

/**
 * @brief Creates a message object (instance of struct)
 *
 * @param event_level level of event
 * @param message text string containing message
 * @param event_source source (ARM, SHARC1, SHARC2)
 * @return True if successful
 */
bool event_logging_add_local_event(BM_SYSTEM_EVENT_LEVEL event_level,
                                   char *message,
                                   BM_SYSTEM_EVENT_SOURCE event_source) {

    strcpy(event_logger_state.event_log[event_logger_state.event_log_write_indx].message, message);
    event_logger_state.event_log[event_logger_state.event_log_write_indx].event_level = event_level;
    event_logger_state.event_log[event_logger_state.event_log_write_indx].event_source = event_source;
    uint64_t stamp = millis();
    event_logging_make_timestamp(&event_logger_state.event_log[event_logger_state.event_log_write_indx], stamp, 0);

    // If an error is encountered and an error callback provided, call the user callback
    if ((event_level == EVENT_FATAL || event_level == EVENT_ERROR) && event_logger_state.error_handling_callback != NULL) {

        // TODO - get this working with interrupts
        (*event_logger_state.error_handling_callback)(0, 0);
    }

    // If we're using the UART, only increment the write pointer if we're not going beyond the size of our FIFO
    bool increment_pointer = true;
    if (event_logger_state.send_events_to_uart) {
        if ((event_logger_state.event_log_write_indx + 1) % EVENT_LOG_QUEUE_LENGTH == event_logger_state.event_log_read_indx) {
            increment_pointer = false;
            event_logger_state.messages_dropped = true;
        }
    }
    else {
        // Handle FIFO overflow
        increment_pointer = false;
        event_logger_state.messages_dropped = true;
    }

    // increment write pointer
    if (increment_pointer) {
        event_logger_state.event_log_write_indx++;
        if (event_logger_state.event_log_write_indx >= EVENT_LOG_QUEUE_LENGTH) {
            event_logger_state.event_log_write_indx = 0;
        }
    }

    return true;
}

/**
 * @brief Creates a timestamp using millis value
 *
 * This function creates a time stamp (days, hours, minutes, seconds, milliseconds)
 * based on millis value (milliseconds since processor started)
 *
 * @param event a pointer to the event object (struct)
 * @param millis_timestamp elapsed milliseconds since processors started
 * @param emuclk an optional argument, zero millis_timestamp to create based on an emuclk value
 * from the SHARCs.
 */
static void event_logging_make_timestamp(BM_SYSTEM_EVENT *event,
                                         uint64_t millis_timestamp,
                                         uint64_t emuclk) {

    // If emuclk is provided, generate a millis timestamp from this
    if (emuclk) {
        double emuclk2 = (double)emuclk;
        double divider = 0.001 / (1.0 / event_logger_state.core_clock_frequency_hz);
        millis_timestamp = (uint64_t)(emuclk2 / divider);
    }

    // Calculate days
    uint32_t days = (uint32_t)floor(((float)millis_timestamp) / (1000.0 * 60.0 * 60.0 * 24.0));
    millis_timestamp = millis_timestamp - (uint64_t)(days * 1000 * 60 * 60 * 24);

    // Calculate hours
    uint32_t hours = (uint32_t)floor(((float)millis_timestamp) / (1000.0 * 60.0 * 60.0));
    millis_timestamp = millis_timestamp - (uint64_t)(hours * 1000 * 60 * 60);

    // Calculate minutes
    uint32_t minutes = (uint32_t)floor(((float)millis_timestamp) / (1000.0 * 60.0));
    millis_timestamp = millis_timestamp - (uint64_t)(minutes * 1000 * 60);

    // Calculate seconds
    uint32_t seconds = (uint32_t)floor(((float)millis_timestamp) / (1000.0));

    // Calculate milliseconds
    uint32_t millis = millis_timestamp - (uint64_t)(seconds * 1000);

    // Store results in the event object (struct)
    event->time_days         = days;
    event->time_hours        = (uint8_t)hours;
    event->time_minutes      = (uint8_t)minutes;
    event->time_seconds      = (uint8_t)seconds;
    event->time_milliseconds = (uint8_t)millis;
}

#endif    // END ARM-only code

/**
 * The code below is only compiled on the SHARC processors, not on the ARM
 */

#if defined(CORE1) || defined(CORE2)

#include <sys/adi_core.h>
#include <sys/platform.h>
#include <services/dma/adi_dma.h>
#include <services/int/adi_sec.h>

// Event queue and indexes for each SHARC core
BM_SYSTEM_EVENT_SHARC event_queue[EVENT_LOG_QUEUE_LENGTH_SHARC];
uint32_t event_queue_read_ptr = 0;
uint32_t event_queue_write_ptr = 0;

// Pointers to our shared memory structure
char         *event_shared_message;
uint32_t    *event_shared_emuclk;
uint32_t    *event_shared_emuclk2;
uint32_t    *event_shared_level;
uint32_t    *event_shared_semaphore;

/**
 * @brief Logs an event
 *
 * Logs an event based on the provided string and event level
 *
 * @param level is the level (info, debug, etc.) of the event
 * @param message is the contents of the message
 *
 * @return true if successful, false if not
 */
bool log_event(BM_SYSTEM_EVENT_LEVEL level,
               char *message) {

    if (strlen(message) > EVENT_LOG_MESSAGE_LEN) {
        return false;
    }

    if ((event_queue_write_ptr + 1) % EVENT_LOG_QUEUE_LENGTH_SHARC == event_queue_read_ptr) {
        return false;
    }

    strcpy(event_queue[event_queue_write_ptr].message, message);
    event_queue[event_queue_write_ptr].event_level = level;
    event_queue[event_queue_write_ptr].event_emuclk = __builtin_emuclk();

    event_queue_write_ptr++;
    if (event_queue_write_ptr >= EVENT_LOG_QUEUE_LENGTH_SHARC) {
        event_queue_write_ptr = 0;
    }

    return true;
}

/**
 * @brief Initialize event messages on a SHARC core
 *
 * Similar to the init function on the ARM core, this function sets pointers
 * to variables in shared L2 memory that are used to pass messages from
 * the SHARC cores to the ARM
 *
 * @param shared_message pointer to shared string for message
 * @param shared_emuclk pointer to shared emuclk value
 * @param shared_emuclk2 pointer to shared emuclk value2
 * @param shared_level pointer to shared event level value
 * @param shared_semaphore pointer to shared semaphore value
 * @return True
 */
bool event_logging_initialize_sharc_core(char *shared_message,
                                         uint32_t *shared_emuclk,
                                         uint32_t *shared_emuclk2,
                                         uint32_t *shared_level,
                                         uint32_t *shared_semaphore) {

    // Set shared variables
    event_shared_message = shared_message;
    event_shared_emuclk  = shared_emuclk;
    event_shared_emuclk2 = shared_emuclk2;
    event_shared_level   = shared_level;
    event_shared_semaphore = shared_semaphore;

    return true;
}

/**
 * @brief Manages the flow of messages from SHARC to ARM
 *
 * This function should be called periodically to move messages logged on
 * a SHARC core to the ARM.  It works well in the 1ms tick event provided
 * by the sysctrl_simple driver
 *
 * @return True
 */
bool event_logging_process_queue_sharc_core(void) {

    // If no new messages, return
    if (event_queue_read_ptr == event_queue_write_ptr) {
        return true;
    }

    // Don't send anything until the last message has been picked up
    if (!(*event_shared_semaphore)) {

        // terminate string just in case
        event_queue[event_queue_read_ptr].message[EVENT_LOG_MESSAGE_LEN - 2] = 0;

        // Copy the event from our queue into shared memory
        memcpy(event_shared_message, event_queue[event_queue_read_ptr].message, EVENT_LOG_MESSAGE_LEN - 1);
        (*event_shared_level)  = (uint32_t)event_queue[event_queue_read_ptr].event_level;
        (*event_shared_emuclk)  = (uint32_t)(0xFFFFFFFF & event_queue[event_queue_read_ptr].event_emuclk);
        (*event_shared_emuclk2) = (uint32_t)(0xFFFFFFFF & (event_queue[event_queue_read_ptr].event_emuclk >> 32));

        // memcpy( (void *) event_queue_shared_event, (void *) &event_queue[event_queue_read_ptr], sizeof(BM_SYSTEM_EVENT_SHARC) );
        event_queue_read_ptr++;
        if (event_queue_read_ptr >= EVENT_LOG_QUEUE_LENGTH_SHARC) {
            event_queue_read_ptr = 0;
        }
        *event_shared_semaphore = true;
    }
    return true;
}

#endif
