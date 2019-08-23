/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * This file contains a number of basic oscillators that can be used for audio
 * synthesis or as parameters for various effects.
 *
 */
#include <math.h>
#include "oscillators.h"

/**
 * @brief Basic sine wave generator
 *
 * @param t time parameter which then gets multiplied by 2*PI so
 *          oscillator_sine(0.0) == oscillator_sine(1.0)
 * @return Oscillator value for that value of t
 */
#pragma optimize_for_speed
float oscillator_sine(float t) {
	t = t - floor(t);
	return sinf(PI2 * t);
}

/**
 * @brief Basic square wave generator
 *
 * @param t time parameter which then gets multiplied by 2*PI so
 *          oscillator_square(0.0) == oscillator_square(1.0)
 * @return Oscillator value for that value of t
 */
#pragma optimize_for_speed
float oscillator_square(float t) {
	t = t - floor(t);
	return t > 0.5 ? 1.0 : -1.0;
}

/**
 * @brief Basic triangle wave generator
 *
 * @param t time parameter which then gets multiplied by 2*PI so
 *          oscillator_triangle(0.0) == oscillator_triangle(1.0)
 * @return Oscillator value for that value of t
 */
#pragma optimize_for_speed
float oscillator_triangle(float t) {
	t = t - floor(t);

	float result;
	if (t < 0.5) {
		result = 1.0 - (4.0 * t);
	} else {
		t -= 0.5;
		result = -1.0 + (4.0 * t);
	}
	return result;
}

/**
 * @brief Basic ramp wave generator
 *
 * @param t time parameter which then gets multiplied by 2*PI so
 *          oscillator_ramp(0.0) == oscillator_ramp(1.0)
 * @return Oscillator value for that value of t
 */
#pragma optimize_for_speed
float oscillator_ramp(float t) {
	t = t - floor(t);

	return 2.0 * t - 1.0;
}

/**
 * @brief Basic pulse wave generator
 *
 * @param t time parameter which then gets multiplied by 2*PI so
 *          oscillator_pulse(0.0) == oscillator_pulse(1.0)
 * @return Oscillator value for that value of t
 */
#pragma optimize_for_speed
float oscillator_pulse(float t, float width) {
	t = t - floor(t);
	return width < t ? 1.0 : -1.0;
}
