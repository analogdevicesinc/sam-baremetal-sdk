/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * This file includes the header file from the audio framework that is selected in
 * common/audio_system_config.h
 *
 */

#ifndef _AUDIO_FRAMEWORK_ARM_H_
#define _AUDIO_FRAMEWORK_ARM_H_

// Set audio system parameters in this file
#include "common/audio_system_config.h"

#if defined(AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN) && AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN
#include "audio_frameworks/audio_framework_8ch_sam_and_audioproj_fin_arm.h"

#elif defined(AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN) && AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN
#include "audio_frameworks/audio_framework_16ch_sam_and_automotive_fin_arm.h"

#elif defined(AUDIO_FRAMEWORK_A2B_BYPASS_SC589) && AUDIO_FRAMEWORK_A2B_BYPASS_SC589
#include "audio_frameworks/audio_framework_a2b_bypass_sc589_arm.h"

#endif // Audio frameworks

#endif // _AUDIO_FRAMEWORK_ARM_H_
