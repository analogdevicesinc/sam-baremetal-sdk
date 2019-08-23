/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .cpp file for documentation.
 *
 */

#ifndef _AUDIO_EFFECTS_TEST_FRAMEWORK_H
#define _AUDIO_EFFECTS_TEST_FRAMEWORK_H

// Audio elements
#include "audio_elements/compressor.h"
#include "audio_elements/compressor.h"
#include "audio_processing/audio_elements/audio_elements_common.h"
#include "audio_processing/audio_elements/audio_utilities.h"

#include "audio_processing/audio_elements/allpass_filter.h"
#include "audio_processing/audio_elements/amplitude_modulation.h"
#include "audio_processing/audio_elements/biquad_filter.h"
#include "audio_processing/audio_elements/clickless_volume_ctrl.h"
#include "audio_processing/audio_elements/compressor.h"
#include "audio_processing/audio_elements/integer_delay_lpf.h"
#include "audio_processing/audio_elements/integer_delay_multitap.h"
#include "audio_processing/audio_elements/oscillators.h"
#include "audio_processing/audio_elements/simple_synth.h"
#include "audio_processing/audio_elements/variable_delay.h"
#include "audio_processing/audio_elements/zero_crossing_detector.h"

// Audio effects
#include "audio_processing/audio_effects/effect_autowah.h"
#include "audio_processing/audio_effects/effect_stereo_reverb.h"
#include "audio_processing/audio_effects/effect_stereo_flanger.h"
#include "audio_processing/audio_effects/effect_tube_distortion.h"
#include "audio_processing/audio_effects/effect_guitar_synth.h"
#include "audio_processing/audio_effects/effect_multiband_compressor.h"
#include "audio_processing/audio_effects/effect_tremelo.h"
#include "audio_processing/audio_effects/effect_ring_modulator.h"

// Audio buffers to pass audio to and from the effects
extern float audio_effects_left_in[];
extern float audio_effects_right_in[];

extern float audio_effects_left_out[];
extern float audio_effects_right_out[];

#ifdef __cplusplus
extern "C" {
#endif

void audio_effects_setup_core1();
void audio_effects_setup_core2();

void audio_effects_process_audio_core1();
void audio_effects_process_audio_core2();

#ifdef __cplusplus
}
#endif

#endif
