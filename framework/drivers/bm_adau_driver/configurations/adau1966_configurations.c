/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 */

#include "../bm_adau_device.h"

uint8_t ADAU1966_8ch_dual_i2s_slave_TxBuffer[] = {
#include "drivers/bm_adau_driver/configurations/ss_schematics/Automotive_Board_Components/Exported_init_files/TxBuffer_ADAU1966.dat"
};

uint16_t ADAU1966_8ch_dual_i2s_slave_NumBytes[] = {
#include "drivers/bm_adau_driver/configurations/ss_schematics/Automotive_Board_Components/Exported_init_files/NumBytes_ADAU1966.dat"
};

BM_ADAU_DEVICE_INIT_DATA adau1966_8ch_dual_i2s_slave = {.data_tx_buffer = ADAU1966_8ch_dual_i2s_slave_TxBuffer,
                                                        .data_num_bytes = ADAU1966_8ch_dual_i2s_slave_NumBytes,
                                                        .total_lines = sizeof(ADAU1966_8ch_dual_i2s_slave_NumBytes) / sizeof(uint16_t),
                                                        .ignore_first_byte_of_init_file = true};
