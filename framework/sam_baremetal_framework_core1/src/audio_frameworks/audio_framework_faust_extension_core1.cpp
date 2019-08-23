/*
 * Copyright (c) 2017 Analog Devices, Inc.  All rights reserved.
 *
 * This is an extension of the 8 channel audio processing framework which adds support for Faust
 *
 */

// Define your audio system parameters in this file
#include "common/audio_system_config.h"
#include "../callback_audio_processing.h"

#if USE_FAUST_ALGORITHM_CORE1

// Structure containing shared variables between the three cores
#include "common/multicore_shared_memory.h"

// UART functionality for MIDI driver on Audio Project Fin
#include "drivers/bm_uart_driver/bm_uart.h"

#include "audio_framework_faust_extension_core1.h"

#include "../Faust/samFaustDSP.h"

// Faust object
samFaustDSP *aSamFaustDSP;

// Instance of UART driver for MIDI
static BM_UART midi_uart;

// Input and output buffers for Faust
float audioChannel_faust_0_left_in[AUDIO_BLOCK_SIZE];
float audioChannel_faust_0_right_in[AUDIO_BLOCK_SIZE];
float audioChannel_faust_1_left_in[AUDIO_BLOCK_SIZE];
float audioChannel_faust_1_right_in[AUDIO_BLOCK_SIZE];
float audioChannel_faust_2_left_in[AUDIO_BLOCK_SIZE];
float audioChannel_faust_2_right_in[AUDIO_BLOCK_SIZE];
float audioChannel_faust_3_left_in[AUDIO_BLOCK_SIZE];
float audioChannel_faust_3_right_in[AUDIO_BLOCK_SIZE];

float audioChannel_faust_0_left_out[AUDIO_BLOCK_SIZE];
float audioChannel_faust_0_right_out[AUDIO_BLOCK_SIZE];
float audioChannel_faust_1_left_out[AUDIO_BLOCK_SIZE];
float audioChannel_faust_1_right_out[AUDIO_BLOCK_SIZE];
float audioChannel_faust_2_left_out[AUDIO_BLOCK_SIZE];
float audioChannel_faust_2_right_out[AUDIO_BLOCK_SIZE];
float audioChannel_faust_3_left_out[AUDIO_BLOCK_SIZE];
float audioChannel_faust_3_right_out[AUDIO_BLOCK_SIZE];

// Prototype for MIDI callback
static void faust_midi_rx_callback(void);
static void faust_handle_pot(int MIDI_Value, int MIDI_Controller);
static void faust_handle_pushbutton(bool enable, int MIDI_Controller);

/**
 * @brief      Faust engine init for Core 1
 *
 * This function initializes the Faust engine and sets up the MIDI interface
 */
void faust_initialize(void){

    #if (defined(USE_FAUST_ALGORITHM_CORE1) && USE_FAUST_ALGORITHM_CORE1)

    //*************************************************************************
    // Initialize Faust
    //*************************************************************************
    // allocate the samFaustObject
    aSamFaustDSP = new samFaustDSP(AUDIO_SAMPLE_RATE, AUDIO_BLOCK_SIZE, FAUST_AUDIO_CHANNELS, FAUST_AUDIO_CHANNELS);

    // pass the channel buffer pointers to the samFausteObject.
    aSamFaustDSP->setDSP_ChannelBuffers(audioChannel_faust_0_left_out,
                                        audioChannel_faust_0_right_out,
                                        audioChannel_faust_1_left_out,
                                        audioChannel_faust_1_right_out,
                                        audioChannel_faust_2_left_out,
                                        audioChannel_faust_2_right_out,
                                        audioChannel_faust_3_left_out,
                                        audioChannel_faust_3_right_out,

                                        audioChannel_faust_0_left_in,
                                        audioChannel_faust_0_right_in,
                                        audioChannel_faust_1_left_in,
                                        audioChannel_faust_1_right_in,
                                        audioChannel_faust_2_left_in,
                                        audioChannel_faust_2_right_in,
                                        audioChannel_faust_3_left_in,
                                        audioChannel_faust_3_right_in);

    #endif

    // Initialize FIFO pointers for moving MIDI events from SHARC core 1 to SHARC core 2
    #if (USE_FAUST_ALGORITHM_CORE2)
		multicore_data->sh1_sh2_fifo_read_ptr = 0;
		multicore_data->sh1_sh2_fifo_write_ptr = 0;
    #endif

    // Initialize the MIDI / UART interface
    if (uart_initialize(&midi_uart,
                        UART_BAUD_RATE_MIDI,
                        UART_SERIAL_8N1,
                        UART_AUDIOPROJ_DEVICE_MIDI)
        != UART_SUCCESS) {

        // handle initialization errors here
    }

    // Set a call back for received MIDI bytes
    uart_set_rx_callback(&midi_uart, faust_midi_rx_callback);
}

/**
 * @brief      Faust audio callback
 *
 * Performs all of the Faust audio processing for the current block of audio.
 * Also manages POTs and PB reads.  This function only gets called from the
 * Audio framework when USE_FAUST_ALGORITHM_CORE1 is defined as TRUE in audio_system_config.h.
 *
 */
void Faust_audio_processing(void){

    static float lastPotValue0 = -1.0;
    static float lastPotValue1 = -1.0;
    static float lastPotValue2 = -1.0;
    static bool enablePB1 = false;
    static bool enablePB2 = false;
    static bool enablePB3 = false;
    static bool enablePB4 = false;

    // If we're using FAUST, handle the pots and pbs

    // pots are always on CC-2,3,4

    float epsilon = 1.0 / 50.0;

    // lock the value of the pot.
    float currentPotValue0 = multicore_data->audioproj_fin_pot_hadc0;
    // if it changed then send MIDI to the samFaustDSP object
    if ((currentPotValue0 >= (lastPotValue0 + epsilon)) || (currentPotValue0 <= (lastPotValue0 - epsilon))) {
        lastPotValue0 = currentPotValue0;
        int MIDIValue = 127.0 * currentPotValue0;
        faust_handle_pot(MIDIValue, 0x02);
    }

    // lock the value of the pot.
    float currentPotValue1 = multicore_data->audioproj_fin_pot_hadc1;
    // if it changed then send MIDI to the samFaustDSP object
    if ((currentPotValue1 >= (lastPotValue1 + epsilon)) || (currentPotValue1 <= (lastPotValue1 - epsilon))) {
        lastPotValue1 = currentPotValue1;
        int MIDIValue = 127.0 * currentPotValue1;
        faust_handle_pot(MIDIValue, 0x03);
    }

    // lock the value of the pot.
    float currentPotValue2 = multicore_data->audioproj_fin_pot_hadc2;
    // if it changed then send MIDI to the samFaustDSP object
    if ((currentPotValue2 >= (lastPotValue2 + epsilon)) || (currentPotValue2 <= (lastPotValue2 - epsilon))) {
        lastPotValue2 = currentPotValue2;
        int MIDIValue = 127.0 * currentPotValue2;
        faust_handle_pot(MIDIValue, 0x04);
    }

    // push buttons are always CC-102(66H), 103(67H), 104(68H), 105(69H)
    if (multicore_data->audioproj_fin_sw_1_core1_pressed) {
        multicore_data->audioproj_fin_sw_1_core1_pressed = false;
        enablePB1 = !enablePB1;
        faust_handle_pushbutton(enablePB1, 0x66);
    }

    if (multicore_data->audioproj_fin_sw_2_core1_pressed) {
        multicore_data->audioproj_fin_sw_2_core1_pressed = false;
        enablePB2 = !enablePB2;
        faust_handle_pushbutton(enablePB2, 0x67);
    }

    if (multicore_data->audioproj_fin_sw_3_core1_pressed) {
        multicore_data->audioproj_fin_sw_3_core1_pressed = false;
        enablePB3 = !enablePB3;
        faust_handle_pushbutton(enablePB3, 0x68);
    }

    if (multicore_data->audioproj_fin_sw_4_core1_pressed) {
        multicore_data->audioproj_fin_sw_4_core1_pressed = false;
        enablePB4 = !enablePB4;
        faust_handle_pushbutton(enablePB4, 0x69);
    }

    // run the FAUST call back
    aSamFaustDSP->processAudioCallback();
}

/*
 *     @brief      MIDI callback for each received serial byte
 */
static void faust_midi_rx_callback(void) {

    static int count;
    static double time;
    static int type;
    static int channel;
    static int data1;
    static int data2;
    static int state = 0;

    uint8_t val;

    while (uart_available(&midi_uart)) {

        uart_read_byte(&midi_uart, &val);
        // state machine to implement simple MIDI parsing.
        //printf("MIDI Byte = <%x>\n", val);

        #if (USE_FAUST_ALGORITHM_CORE2)
        /*
         * If we're using Core 2 for Faust, pipe these MIDI events over to the second SHARC
         * core.  We do this regardless of whether or not we're using Faust on SHARC core 1.
         */
        if (((multicore_data->sh1_sh2_fifo_write_ptr + 1) % SH1_SH2_BYTE_FIFO_SIZE) !=
            multicore_data->sh1_sh2_fifo_read_ptr) {

            // If there is room in the FIFO, copy this byte
            multicore_data->sh1_sh2_byte_fifo[multicore_data->sh1_sh2_fifo_write_ptr++] = val;

            // Wrap our write pointer if necessary
            if (multicore_data->sh1_sh2_fifo_write_ptr >= SH1_SH2_BYTE_FIFO_SIZE) {
                multicore_data->sh1_sh2_fifo_write_ptr = 0;
            }
        }
        else {
            // Add code here to handle a FIFO full error
        }

        #endif

        #if (USE_FAUST_ALGORITHM_CORE1)
        /*
         * If we're using Core 1 for Faust, handle the MIDI events coming in from the UART.
         */
        switch (state) {
            case 0:       // Get a byte

                switch (val & 0xF0) {
                    // two data bytes
                    case 0x80:
                    case 0x90:
                    case 0xA0:
                    case 0xB0:
                    case 0xE0:
                        count = 3;
                        type = (val & 0xF0);
                        channel = (val & 0x0F);
                        state = 1;
                        break;

                    // one data byte
                    case 0xC0:
                    case 0xD0:
                        count = 2;
                        type = (val & 0xF0);
                        channel = (val & 0x0F);
                        state = 3;
                        break;

                    default:
                        state = 0;
                }

                break;
            case 1:       // first of 2 data bytes
                data1 = val;
                state = 2;
                break;

            case 2:      // second of 2 data bytes
                data2 = val;
                //printf("2 byte message type = <%x>, channel = <%x>, data1 = <%x>, data2 = <%x>\n", type, channel, data1, data2);
                aSamFaustDSP->propagateMidi(3, 0.0, type, channel, data1, data2);
                state = 0;
                break;

            case 3:       // single data byte
                data1 = val;
                data2 = val;
                //printf("3 byte message type = <%x>, channel = <%x>, data1 = <%x>, data2 = <%x>\n", type, channel, data1, data2);
                aSamFaustDSP->propagateMidi(2, 0.0, type, channel, data1, data2);
                state = 0;
                break;

            default:
                state = 0;
        }     // switch
        #endif
    } // while
}

static void faust_handle_pot(int MIDI_Value,
                             int MIDI_Controller) {
    int type    = 0xB0;                  // MIDI continuous controller
    int channel = 0;                     // always on channel 1
    int data1   = MIDI_Controller;      // MIDI Controller Assigned to POT0
    int data2   = (MIDI_Value & 0x7F);      // The value of the slider is formed into a 7-bit MIDI data byte
    aSamFaustDSP->propagateMidi(3, 0.0, type, channel, data1, data2);
}

static void faust_handle_pushbutton(bool enable,
                                    int MIDI_Controller) {
    //  Data needed to convert PB to on/off midi messages

    int PB_ToggleValue;

    if (enable) {
        PB_ToggleValue = 127;
    }
    else {
        PB_ToggleValue = 0;
    }

    // form the MIDI message
    int type    = 0xB0;              // MIDI continuous controller
    int channel = 0;                 // always on channel 1
    int data1   = MIDI_Controller;  // MIDI Controller Assigned to POT0
    int data2   = (PB_ToggleValue & 0x7F);  // The value of the button is formed into a 7-bit MIDI data byte
    aSamFaustDSP->propagateMidi(3, 0.0, type, channel, data1, data2);
}

#endif  // USE_FAUST_ALGORITHM_CORE1
