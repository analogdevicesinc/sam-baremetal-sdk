/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * These are the hooks for the your audio processing functions.
 *
 */

// Define your audio system parameters in this file
#include "common/audio_system_config.h"

// Only enable these functions if we're using a dual-core framework
#if (USE_BOTH_CORES_TO_PROCESS_AUDIO)

// Support for simple multi-core data sharing
#include "common/multicore_shared_memory.h"

// Variables related to the audio framework that is currently selected (e.g. input and output buffers)
#include "audio_framework_selector.h"

// Includes all header files for effects and calls for effect selector
#include "audio_processing/audio_effects_selector.h"

// Prototypes for this file
#include "callback_audio_processing.h"

/*
 *
 * Available Processing Power
 * --------------------------
 *
 * The two SHARC cores provide a hefty amount of audio processing power.  However, it is
 * important to ensure that any audio processing code can run and complete within one frame of audio.
 *
 * The total number of cycles available in the audio callback can be calculated as follows:
 *
 * total cycles = ( processor-clock-speed * audio-block-size ) / audio-sample-rate//
 *
 * For example, if the processor is running at 450MHz, the audio sampling rate is 48KHz and the
 * audio block size is set to 32 words, the total number of processor cycles available in each
 * callback is 300,000 cycles or 300,000/32 or 9,375 per sample of audio.
 *
 * Available Audio Buffers
 * -----------------------
 *
 * There are several sets of audio input and output buffers that correspond to the
 * various peripherals (e.g. audio codec, USB, S/PDIF, A2B).
 *
 * SHARC Core 1 manages the audio flow to these various peripherals.  SHARC Core 2
 * only has access to a set of input and output buffers used to move audio from
 * core 1 to core 2 and from core 2 to core 1.  If Faust is being used, SHARC Core 2
 * also has access to a send of input and output buffers for Faust.
 *
 * Inputs Buffers
 * **************
 *
 *  Audio from SHARC Core 1
 *
 *     audiochannel_0_left_in[];
 *     audiochannel_0_right_in[];
 *     audiochannel_1_left_in[];
 *     audiochannel_1_right_in[];
 *     audiochannel_2_left_in[];
 *     audiochannel_2_right_in[];
 *     audiochannel_3_left_in[];
 *     audiochannel_3_right_in[];
 *
 *     If the automotive board is being used, we have an additional 8 channels from
 *     SHARC Core 1 (e.g. audiochannel_4_left_in[]... audiochannel_7_left_in[])
 *
 *  Audio from Faust (be sure to enable Faust in audio_system_config.h)
 *
 *     audioChannel_faust_0_left_in[]
 *     audioChannel_faust_0_right_in[]
 *     audioChannel_faust_1_left_in[]
 *     audioChannel_faust_1_right_in[]
 *     audioChannel_faust_2_left_in[]
 *     audioChannel_faust_2_right_in[]
 *     audioChannel_faust_3_left_in[]
 *     audioChannel_faust_3_right_in[]
 *
 *
 * Output Buffers
 * *************
 *
 *   Audio sent back to SHARC Core 1 and then transmitted to various peripherals
 *
 *     audiochannel_0_left_out[];
 *     audiochannel_0_right_out[];
 *     audiochannel_1_left_out[];
 *     audiochannel_1_right_out[];
 *     audiochannel_2_left_out[];
 *     audiochannel_2_right_out[];
 *     audiochannel_3_left_out[];
 *     audiochannel_3_right_out[];
 *
 *     If the automotive board is being used, we have an additional 8 channels to
 *     send back to SHARC Core 1 (e.g. audiochannel_4_left_out[]... audiochannel_7_left_out[])
 *
 *  Audio to the Faust (be sure to enable Faust in audio_system_config.h)
 *
 *     audioChannel_faust_0_left_out[]
 *     audioChannel_faust_0_right_out[]
 *     audioChannel_faust_1_left_out[]
 *     audioChannel_faust_1_right_out[]
 *     audioChannel_faust_2_left_out[]
 *     audioChannel_faust_2_right_out[]
 *     audioChannel_faust_3_left_out[]
 *     audioChannel_faust_3_right_out[]
 *
 *  Note: Faust processing occurs before the audio callback so any data
 *  copied into Faust's input buffers will be available the next time
 *  the callback is called.  Similarly, Faust's output buffers contain
 *  audio that was processed before the callback.
 */

/*
 * Place any initialization code here for your audio processing algorithms
 */
void processaudio_setup(void) {

	// Initialize the audio effects in the audio_processing/ folder
	audio_effects_setup_core2();

    // *******************************************************************************
    // Add any custom setup code here
    // *******************************************************************************
}

 /*
  * This callback is called every time we have a new audio buffer that is ready
  * for processing.  It's currently configured for in-place processing so if no
  * processing is done to the audio, it is passed through unaffected.
  *
  * See the header file for the framework you have selected in the Audio_Frameworks
  * directory for a list of the input and output buffers that are available based on
  * the framework and hardware.
  *
  * The two SHARC cores provide a hefty amount of audio processing power. However, it is important
  * to ensure that any audio processing code can run and complete within one frame of audio.
  *
  * The total number of cycles available in the audio callback can be calculated as follows:
  * total cycles = ( processor-clock-speed * audio-block-size ) / audio-sample-rate
  *
  * For example, if the processor is running at 450MHz, the audio sampling rate is 48KHz and the audio
  * block size is set to 32 words, the total number of processor cycles available in each callback
  * is 300,000 cycles or 300,000/32 or 9,375 per sample of audio
  */

// When debugging audio algorithms, helpful to comment out this pragma for more linear single stepping.
#pragma optimize_for_speed
void processaudio_callback(void) {

    int i;

	if (true) {

		// Copy incoming audio buffers to the effects input buffers
		copy_buffer(audiochannel_0_left_in,  audio_effects_left_in, AUDIO_BLOCK_SIZE);
		copy_buffer(audiochannel_0_right_in, audio_effects_right_in, AUDIO_BLOCK_SIZE);

		// Process audio effects
		audio_effects_process_audio_core2();

		// Copy processed audio back to input buffers
		copy_buffer(audio_effects_left_out, audiochannel_0_left_in, AUDIO_BLOCK_SIZE);
		copy_buffer(audio_effects_right_out, audiochannel_0_right_in, AUDIO_BLOCK_SIZE);

	}

    for (i = 0; i < AUDIO_BLOCK_SIZE; i++) {

        // *******************************************************************************
        // Replace the pass-through code below with your custom audio processing code here
        // *******************************************************************************

        audiochannel_0_left_out[i]  = audiochannel_0_left_in[i];
        audiochannel_0_right_out[i] = audiochannel_0_right_in[i];
        audiochannel_1_left_out[i]  = audiochannel_1_left_in[i];
        audiochannel_1_right_out[i] = audiochannel_1_right_in[i];
        audiochannel_2_left_out[i]  = audiochannel_2_left_in[i];
        audiochannel_2_right_out[i] = audiochannel_2_right_in[i];
        audiochannel_3_left_out[i]  = audiochannel_3_left_in[i];
        audiochannel_3_right_out[i] = audiochannel_3_right_in[i];

        // If the automotive board is connected, process an additional 8 (16 total) channels
        #if defined(FRAMEWORK_16CH_SINGLE_OR_DUAL_CORE_AUTOMOTIVE) && FRAMEWORK_16CH_SINGLE_OR_DUAL_CORE_AUTOMOTIVE

            audiochannel_4_left_out[i]  = audiochannel_4_left_in[i];
            audiochannel_4_right_out[i] = audiochannel_4_right_in[i];
            audiochannel_5_left_out[i]  = audiochannel_5_left_in[i];
            audiochannel_5_right_out[i] = audiochannel_5_right_in[i];
            audiochannel_6_left_out[i]  = audiochannel_6_left_in[i];
            audiochannel_6_right_out[i] = audiochannel_6_right_in[i];
            audiochannel_7_left_out[i]  = audiochannel_7_left_in[i];
            audiochannel_7_right_out[i] = audiochannel_7_right_in[i];

        #endif

        // If we're using Faust, route audio into the flow
        #if defined(USE_FAUST_ALGORITHM_CORE2) && USE_FAUST_ALGORITHM_CORE2

            // Mix in 8 channel audio from Faust
            audiochannel_0_left_out[i]  = audioChannel_faust_0_left_out[i];
            audiochannel_0_right_out[i] = audioChannel_faust_0_right_out[i];
            audiochannel_1_left_out[i]  = audioChannel_faust_1_left_out[i];
            audiochannel_1_right_out[i] = audioChannel_faust_1_right_out[i];
            audiochannel_2_left_out[i]  = audioChannel_faust_2_left_out[i];
            audiochannel_2_right_out[i] = audioChannel_faust_2_right_out[i];
            audiochannel_3_left_out[i]  = audioChannel_faust_3_left_out[i];
            audiochannel_3_right_out[i] = audioChannel_faust_3_right_out[i];

            // Route 8 channel audio to Faust for next block
            audioChannel_faust_0_left_in[i]  = audiochannel_0_left_in[i];
            audioChannel_faust_0_right_in[i] = audiochannel_0_right_in[i];
            audioChannel_faust_1_left_in[i]  = audiochannel_1_left_in[i];
            audioChannel_faust_1_right_in[i] = audiochannel_1_right_in[i];
            audioChannel_faust_2_left_in[i]  = audiochannel_2_left_in[i];
            audioChannel_faust_2_right_in[i] = audiochannel_2_right_in[i];
            audioChannel_faust_3_left_in[i]  = audiochannel_3_left_in[i];
            audioChannel_faust_3_right_in[i] = audiochannel_3_right_in[i];

        #endif
    }
}

/*
 * This loop function is like a thread with a low priority.  This is good place to process
 * large FFTs in the background without interrupting the audio processing callback.
 */
void processaudio_background_loop(void) {

    // *******************************************************************************
    // Add any custom background processing here
    // *******************************************************************************


}

/*
 * This function is called if the code in the audio processing callback takes too long
 * to complete (essentially exceeding the available computational resources of this core).
 */
void processaudio_mips_overflow(void) {
}

#endif
