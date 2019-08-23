/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * This is an extension of the 8 channel audio processing framework which adds support for Faust
 *
 */

// Define your audio system parameters in this file
#include "common/audio_system_config.h"
#include "../callback_audio_processing.h"

#if (USE_FAUST_ALGORITHM_CORE2)

// Structure containing shared variables between the three cores
#include "common/multicore_shared_memory.h"

// UART functionality for MIDI driver on Audio Project Fin
#include "drivers/bm_uart_driver/bm_uart.h"

#include "audio_framework_faust_extension_core2.h"

#include "../Faust/samFaustDSP.h"

// Faust object
samFaustDSP *aSamFaustDSP;

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

// Function prototypes
static void faust_handle_pot(int MIDI_Value, int MIDI_Controller);
static void faust_handle_pushbutton(bool enable, int MIDI_Controller);
static void faust_core2_process_midi(uint8_t val);
static void faust_midi_rx_callback(void);

#if !USE_FAUST_ALGORITHM_CORE1
// Instance of UART driver for MIDI
static BM_UART midi_uart;
#endif

/**
 * @brief      Faust engine init for Core 2
 *
 * This function initializes the Faust engine and sets up the MIDI interface
 */
void faust_initialize(){

    // allocate the samFaustObject
    aSamFaustDSP = new samFaustDSP(AUDIO_SAMPLE_RATE, AUDIO_BLOCK_SIZE, FAUST_AUDIO_CHANNELS, FAUST_AUDIO_CHANNELS);

    // pass the channel buffer pointers to the samFaustObject.
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

	#if !USE_FAUST_ALGORITHM_CORE1

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
	#endif
}

/**
 * @brief      Faust audio callback
 *
 * Performs all of the Faust audio processing for the current block of audio.
 * Also manages POTs and PB reads.
 *
 */
void Faust_audio_processing(){

	/**
	 * If core 1 is also being used for Faust, core 1 will pass along MIDI bytes
	 * via the fifo in our shared memory structure.  If core 1 is not being used for
	 * Faust, core 2 will connect to the UART directly.  In this case, new MIDI
	 * bytes will come in via the faust_midi_rx_callback() routine.
	 */
	#if USE_FAUST_ALGORITHM_CORE1

		// check the FIFO for any new bytes.  When read pointer = write pointer, FIFO is empty.
		if (multicore_data->sh1_sh2_fifo_write_ptr != multicore_data->sh1_sh2_fifo_read_ptr) {

			uint8_t midi_byte = multicore_data->sh1_sh2_byte_fifo[multicore_data->sh1_sh2_fifo_read_ptr++];

			// Wrap the read pointer if necessary
			if (multicore_data->sh1_sh2_fifo_read_ptr >= SH1_SH2_BYTE_FIFO_SIZE) {
				multicore_data->sh1_sh2_fifo_read_ptr = 0;
			}

			faust_core2_process_midi(midi_byte);
		}

	#endif

    // Process this block of audio
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
    if (multicore_data->audioproj_fin_sw_1_core2_pressed) {
        multicore_data->audioproj_fin_sw_1_core2_pressed = false;
        enablePB1 = !enablePB1;
        faust_handle_pushbutton(enablePB1, 0x66);
    }

    if (multicore_data->audioproj_fin_sw_2_core2_pressed) {
        multicore_data->audioproj_fin_sw_2_core2_pressed = false;
        enablePB2 = !enablePB2;
        faust_handle_pushbutton(enablePB2, 0x67);
    }

    if (multicore_data->audioproj_fin_sw_3_core2_pressed) {
        multicore_data->audioproj_fin_sw_3_core2_pressed = false;
        enablePB3 = !enablePB3;
        faust_handle_pushbutton(enablePB3, 0x68);
    }

    if (multicore_data->audioproj_fin_sw_4_core2_pressed) {
        multicore_data->audioproj_fin_sw_4_core2_pressed = false;
        enablePB4 = !enablePB4;
        faust_handle_pushbutton(enablePB4, 0x69);
    }

    // run the FAUST call back
    aSamFaustDSP->processAudioCallback();
}

static void faust_handle_pot(int MIDI_Value, int MIDI_Controller){
    int type    = 0xB0;                  // MIDI continuous controller
    int channel = 0;                     // always on channel 1
    int data1   = MIDI_Controller;      // MIDI Controller Assigned to POT0
    int data2   = (MIDI_Value & 0x7F);      // The value of the slider is formed into a 7-bit MIDI data byte
    aSamFaustDSP->propagateMidi(3, 0.0, type, channel, data1, data2);
}

static void faust_handle_pushbutton(bool enable, int MIDI_Controller){
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

static void faust_core2_process_midi(uint8_t val) {

    //
    static int count;
    static double time;
    static int type;
    static int channel;
    static int data1;
    static int data2;
    static int state = 0;

    // state machine to implement simple MIDI parsing.
    switch (state) {
        case 0:   // Get a byte

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
        case 1:   // first of 2 data bytes
            data1 = val;
            state = 2;
            break;

        case 2:  // second of 2 data bytes
            data2 = val;
            //printf("2 byte message type = <%x>, channel = <%x>, data1 = <%x>, data2 = <%x>\n", type, channel, data1, data2);
            aSamFaustDSP->propagateMidi(3, 0.0, type, channel, data1, data2);
            state = 0;
            break;

        case 3:   // single data byte
            data1 = val;
            data2 = val;
            //printf("3 byte message type = <%x>, channel = <%x>, data1 = <%x>, data2 = <%x>\n", type, channel, data1, data2);
            aSamFaustDSP->propagateMidi(2, 0.0, type, channel, data1, data2);
            state = 0;
            break;

        default:
            state = 0;
    } // switch
}

#if !USE_FAUST_ALGORITHM_CORE1
/**
 * @brief Callback for UART interrupt when only core 2 is running Faust
 */
static void faust_midi_rx_callback(void) {

    uint8_t val;

    while (uart_available(&midi_uart)) {

    	// Get byte from UART RX FIFO and send to Faust
        uart_read_byte(&midi_uart, &val);
        faust_core2_process_midi(val);

    }
}
#endif // !USE_FAUST_ALGORITHM_CORE1

#endif // USE_FAUST_ALGORITHM_CORE2
