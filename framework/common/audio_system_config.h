/*
 * Copyright (c) 2018 Analog Devices, Inc.
 * All rights reserved.
 *
 * This file is used to set up the configuration for the bare metal framework.
 *
 * This is shared between all three processor cores.
 *
 * Important tips when editing this file:
 *
 * 1) This is a single file that is referenced by all three cores and it shows
 *    up in all three projects in the "common" folder.  It's possible in Eclipse
 *    to have multiple versions of this file open, but Eclipse can't always tell
 *    that it's the same file.  So it's recommended to only have one instance of
 *    this file open in editor.
 *
 * 2) For similar reasons, the make system doesn't always catch all of the
 *    necessary build changes after this file has been modified.  Thus it is
 *    recommended to do a project clean then build after any of these
 *    parameters are changed!
 */

#ifndef _AUDIO_SYSTEM_CONFIG_H
#define _AUDIO_SYSTEM_CONFIG_H

#define TRUE    (1)
#define FALSE   (0)

/*******************************************************************************
 * 1. Select which (if any) daughter boards are connected to the SHARC Audio Module
 ******************************************************************************/

// Select any connected daughter boards

// The Automotive audio daughter board is an 8-in / 16-out audio / A2B
// extender board
#define SAM_AUTOMOTIVE_AUDIO_BOARD_PRESENT                 FALSE

#if (SAM_AUTOMOTIVE_AUDIO_BOARD_PRESENT)
/**
 * The automotive board includes an 1/8" input jack.  The audio from this
 * jack can be routed to channel 0 so you can use the framework as is.
 * Normally, audio from the 1/8" input jack shows up in stereo channel
 * 2 due to the structure of the TDM stream from the automotive board.
 */
    #define AUTOMOTIVE_MAP_JACKS_TO_CH_0_AND_1             TRUE
#endif

// The Audio Project Fin contains 1/4" instrument jacks, MIDI In/Out/Thru, pots,
// buttons and proto area
#define SAM_AUDIOPROJ_FIN_BOARD_PRESENT                    TRUE

#if (SAM_AUDIOPROJ_FIN_BOARD_PRESENT)
/*
 *  If you're using Audio Project Fin version 3.02, set this to true.
 *  You can find the Audio Project Fin version right next to the POT
 *  associated with HADC2 on the Audio Project Fin.
 */
    #define SAM_AUDIOPROJ_FIN_BOARD_V3_02                  FALSE

#endif

/*******************************************************************************
 * 2. Set audio processing parameters
 ******************************************************************************/

// This should be a base 2 number from 8 to 128
#define AUDIO_BLOCK_SIZE                                   (32)

// Set audio sample rate
#define AUDIO_SAMPLE_RATE                                  (48000)

// Set to true to use both cores, set to false to just use SHARC Core 1
#define USE_BOTH_CORES_TO_PROCESS_AUDIO                    TRUE

/*******************************************************************************
 * 3. Select an audio processing framework to use (only select one)
 ******************************************************************************/

// Standard audio processing framework (SHARC Audio Module
// and SHARC Audio Module + Audio Project Fin)
#define AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN          TRUE

// Audio processing framework for use with the automotive daughter board
#define AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN        FALSE

// Bypasses the ADSP-SC589 so I2S signals routed directly between ADAU1761 and
// A2B controller (GPIO4 = LED on this board)
#define AUDIO_FRAMEWORK_A2B_BYPASS_SC589                   FALSE

/*******************************************************************************
 * 4. Select whether or not to enable A2B in the framework
 ******************************************************************************/

#define ENABLE_A2B                                         FALSE

#if (ENABLE_A2B)
/**
 * If A2B is enabled, select the role that this SHARC Audio Module board
 * will play (TRUE = master node, FALSE = slave node)
 */
    #define A2B_ROLE_MASTER                                TRUE


/**
 * If this SHARC Audio Module board is a master, select an A2B topology
 * 
 * Note that these topologies are created in SigmaStudio and stored
 * within drivers/bm_a2b_driver/a2b_topologies. See documentation for a
 * full description of these configurations.
 * NOTE: SET ONLY ONE TO TRUE
 */
    #if (A2B_ROLE_MASTER)
        #define A2B_TOPOLOGY_TDM8_SAM_to_SAM_2up_2down                       FALSE
        #define A2B_TOPOLOGY_TDM8_SAM_to_SAM_to_SAM_4up_4down                FALSE
        #define A2B_TOPOLOGY_TDM8_SAM_to_CLASSD_4down                        TRUE

        // Add your own pre-processor variables for custom A2B topologies here




    #endif  // A2B_ROLE_MASTER

#endif  // ENABLE_A2B

/*******************************************************************************
 * 5. Use enhanced ADAU1761 driver
 ******************************************************************************/

#define USE_ENHANCED_ADAU1761_DRIVER                       FALSE

/**
 * The ADAU1761 is a stereo codec but connects to the ADSP-SC589 via an 8
 * channel TDM link. Because the ADAU1761 has a DSP core, we can use the
 * remaining channels to provide pre-processing and post-processing using the
 * unused remaining channels.
 *
 * Below is the allocation of TDM channels
 * Output
 *  - First stereo pair : clean to DAC
 *  - Second stereo pair : loudness
 *  - Third stereo pair : "phat" stereo
 *  - Fourth stereo pair : Chorus
 *
 *  The outputs of all four channels are mixed together before sending to the
 *  DAC.
 *
 * Input
 *  - First stereo pair : clean from ADC
 *  - Second stereo pair : -6dB limiter
 *  - Third stereo pair : unused (zeros)
 *  - Fourth stereo pair : 440Hz (L) and 880Hz (R) sine waves
 */

/*******************************************************************************
 * 6. Optional software components
 ******************************************************************************/

#define FAUST_INSTALLED                                    FALSE

#if (FAUST_INSTALLED)
/*
 * Select whether or not to use the Faust algorithm to generate synth sounds
 * and do audio processing.  Presently, a Audio Project Fin is required as the Faust
 * algorithm is configured to generate a synth sound based on MIDI notes.
 * Note: the Faust source code takes a few minutes to compile so you may
 * get a compiler warning that it's taking a long time to compile.  This is
 * normal.
 */
    #define USE_FAUST_ALGORITHM_CORE1            TRUE
    #define USE_FAUST_ALGORITHM_CORE2            FALSE
    #define FAUST_AUDIO_CHANNELS                 (2)

#endif

#if (!FAUST_INSTALLED && SAM_AUDIOPROJ_FIN_BOARD_PRESENT)

/*
 * If we're not using Faust (which owns MIDI), select which core MIDI events
 * should be handled by.  Only set one to TRUE.  This will enable the
 * MIDI callback function in Callback_MIDI_Message.cpp on that core.
 */

    #define   MIDI_UART_MANAGED_BY_ARM_CORE      FALSE
    #define   MIDI_UART_MANAGED_BY_SHARC1_CORE   TRUE

#endif

/*******************************************************************************
 * 7. CPU clock speed
 ******************************************************************************/

// Do not modify these if working on the SHARC Audio Module board
#define CORE_CLOCK_FREQ_HZ                       (450000000)
#define EXT_OSCILLATOR_FREQ_HZ                   (25000000)

/*******************************************************************************
 * Fixed Configurations (read only)
 ******************************************************************************/

// This version of the framework is designed to run on the SHARC Audio Module
// hardware with rev of 1.3 or greater
#define   SHARC_SAM_REV_1_3_OR_LATER             TRUE

// Additional pre-processor variable settings to support Faust
#if (FAUST_INSTALLED)
    #define   MIDI_UART_MANAGED_BY_ARM_CORE      FALSE
    #define   MIDI_UART_MANAGED_BY_SHARC1_CORE   FALSE

    #if (USE_FAUST_ALGORITHM_CORE1 && USE_FAUST_ALGORITHM_CORE2)
        #define SH1_SH2_BYTE_FIFO_SIZE           (32)
    #endif
#endif

// Settings for events
#define MAX_EVENT_MESSAGE_LENGTH                 (128)

// Addition clock calculation
#define SYSTEM_CLOCK_FREQ_HZ                     (CORE_CLOCK_FREQ_HZ / 2)
#define SCK0_CLOCK_FREQ_HZ                       (SYSTEM_CLOCK_FREQ_HZ / 2)
#define SCK1_CLOCK_FREQ_HZ                       (SYSTEM_CLOCK_FREQ_HZ / 2)

/*******************************************************************************
 * Check for any illegal configurations
 ******************************************************************************/

// Check for illegal clocks
#if (CORE_CLOCK_FREQ_HZ != 450000000)
    #error Framework currently only supports a 450MHz clock speed
#endif

// If using bypass mode, ensure A2B is enabled
#if (AUDIO_FRAMEWORK_A2B_BYPASS_SC589) && !ENABLE_A2B
    #error Enable A2B when using the bypass framework
#endif

// Check for illegal audio block size
#if (AUDIO_BLOCK_SIZE != (4) && \
    AUDIO_BLOCK_SIZE != (8) && \
    AUDIO_BLOCK_SIZE != (16) && \
    AUDIO_BLOCK_SIZE != (32) && \
    AUDIO_BLOCK_SIZE != (64) && \
    AUDIO_BLOCK_SIZE != (128))
    #error Illegal audio configuration: Illegal audio block size set.  Must be from 4 to 128 and a base-2 number.
#endif

// Check if two cores are trying to run UART / MIDI
#if (MIDI_UART_MANAGED_BY_ARM_CORE) && (MIDI_UART_MANAGED_BY_SHARC1_CORE)
    #error Illegal audio configuration: both ARM and SHARC CORE 1 cannot be set to process UART / MIDI.  Select only one
#endif

// Check if two daughter boards are connected at the same time
#if (SAM_AUTOMOTIVE_AUDIO_BOARD_PRESENT) && (SAM_AUDIOPROJ_FIN_BOARD_PRESENT)
    #error Illegal audio configuration: Cannot have two SHARC Audio Module daughter boards connected at the same time
#endif

// Ensure we're not selecting the automotive board when this board isn't connected
#if AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN && (!SAM_AUTOMOTIVE_AUDIO_BOARD_PRESENT)
    #error Illegal audio configuration: selected automotive framework but automotive board is not connected
#endif

// Ensure our framework supports A2B
#if (ENABLE_A2B && AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN)
    #error The framework for the automotive board does not support A2B
#endif

#if ENABLE_A2B && (AUDIO_SAMPLE_RATE != 48000) &&  !A2B_ROLE_MASTER
    #warning Note that the audio sample rate will be determined by the A2B bus when not a master; this will override the local sample rate settings
#endif

// If we're using USB, make sure we're running at 48KHz or 44.1 KHz
#if ENABLE_USB_AUDIO &&  \
    (AUDIO_SAMPLE_RATE != 441000) && \
    (AUDIO_SAMPLE_RATE != 48000)
    #error USB Audio Driver only supports 44.1 and 48KHz
#endif

// If we're using the automotive fin, ensure we're using the automotive framework
#if SAM_AUTOMOTIVE_AUDIO_BOARD_PRESENT && !AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN
	#error Automotive fin attached but automotive framework is not selected
#endif

#endif // _AUDIO_SYSTEM_CONFIG_H
