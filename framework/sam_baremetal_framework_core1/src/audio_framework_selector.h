/*
 * Copyright (c) 2017 Analog Devices, Inc.  All rights reserved.
 *
 * This file includes the header file from the audio framework that is selected in
 * common/audio_system_config.h
 *
 */

#ifndef _AUDIO_FRAMEWORK_CORE1_H
#define _AUDIO_FRAMEWORK_CORE1_H

// Define your audio system parameters in this file
#include "common/audio_system_config.h"

#if defined(AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN) && AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN
#include "audio_frameworks/audio_framework_8ch_sam_and_audioproj_fin_core1.h"

#elif defined(AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN) && AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN
#include "audio_frameworks/audio_framework_16ch_sam_and_automotive_fin_core1.h"

#elif defined(AUDIO_FRAMEWORK_A2B_BYPASS_SC589) && AUDIO_FRAMEWORK_A2B_BYPASS_SC589

// In bypass mode, the SHARCs aren't used.  We default to this framework simply so everything links properly
#include "audio_frameworks/audio_framework_8ch_sam_and_audioproj_fin_core1.h"

#endif // Audio frameworks

#endif    //_AUDIO_FRAMEWORK_CORE1_H
