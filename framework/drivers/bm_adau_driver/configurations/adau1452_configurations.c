/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 */

#include "../bm_adau_device.h"

uint8_t ADAU1452_16ch_master_TxBuffer[] = {
#include "drivers/bm_adau_driver/configurations/ss_schematics/Automotive_Board_Components/Exported_init_files/TxBuffer_ADAU1452.dat"
};

uint16_t ADAU1452_16ch_master_NumBytes[] = {
#include "drivers/bm_adau_driver/configurations/ss_schematics/Automotive_Board_Components/Exported_init_files/NumBytes_ADAU1452.dat"
};

BM_ADAU_DEVICE_INIT_DATA adau1452_16ch_master = {
    .data_tx_buffer = ADAU1452_16ch_master_TxBuffer,
    .data_num_bytes = ADAU1452_16ch_master_NumBytes,
    .total_lines = sizeof(ADAU1452_16ch_master_NumBytes) / sizeof(uint16_t),
    .ignore_first_byte_of_init_file = false
};
