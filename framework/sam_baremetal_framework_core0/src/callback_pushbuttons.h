/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 */
#ifndef _CALLBACK_PUSHBUTTONS_H
#define _CALLBACK_PUSHBUTTONS_H

#ifdef __cplusplus
extern "C" {
#endif

void SHARC_SAM_pb1_callback(void *data_object);
void SHARC_SAM_pb2_callback(void *data_object);

void pushbutton_callback_sam_pb1(void *data_object);
void pushbutton_callback_sam_pb2(void *data_object);

void pushbutton_callback_external_1(void *data_object);
void pushbutton_callback_external_2(void *data_object);
void pushbutton_callback_external_3(void *data_object);
void pushbutton_callback_external_4(void *data_object);

#ifdef __cplusplus
}
#endif

#endif    // _CALLBACK_PUSHBUTTONS_H
