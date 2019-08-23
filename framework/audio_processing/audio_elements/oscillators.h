/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _OSCILLATORS_H
#define _OSCILLATORS_H

#include "audio_elements_common.h"

float oscillator_sine(float t);
float oscillator_square(float t);
float oscillator_triangle(float t);
float oscillator_pulse(float t, float width);
float oscillator_ramp(float t);

#endif  // _OSCILLATORS_H

