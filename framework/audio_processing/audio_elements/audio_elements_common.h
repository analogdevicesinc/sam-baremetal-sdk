/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * This file provides various constants that are used across the audio
 * elements.
 */

#ifndef _AUDIO_ELEMENTS_COMMON_H
#define _AUDIO_ELEMENTS_COMMON_H

#ifndef PI
#define PI (3.1415926535897932384626433832795)
#endif
#ifndef PI2
#define PI2 (6.283185307179586476925286766559)
#endif

/**
 * To avoid use of the heap, this preprocessor variable
 * allows functions to declare a local scratch variable
 * that will always be smaller than the current audio
 * block size.
 */
#define MAX_AUDIO_BLOCK_SIZE    (128)

#endif  //_AUDIO_ELEMENTS_COMMON_H
