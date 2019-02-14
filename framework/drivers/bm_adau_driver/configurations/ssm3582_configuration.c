/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 */

#include "../bm_adau_device.h"

uint8_t ssm3582_2ch_i2s_slave_TxBuffer[] = {
#include "drivers/bm_adau_driver/configurations/ss_schematics/Class_D_Fin/Exported_init_files/TxBuffer_SSM3582_1.dat"
};

uint16_t ssm3582_2ch_i2s_slave_NumBytes[] = {
#include "drivers/bm_adau_driver/configurations/ss_schematics/Class_D_Fin/Exported_init_files/NumBytes_SSM3582_1.dat"
};

BM_ADAU_DEVICE_INIT_DATA ssm3582_2ch_i2s_slave = {.data_tx_buffer = ssm3582_2ch_i2s_slave_TxBuffer,
                                                  .data_num_bytes = ssm3582_2ch_i2s_slave_NumBytes,
                                                  .total_lines = sizeof(ssm3582_2ch_i2s_slave_NumBytes) / sizeof(uint16_t),
                                                  .ignore_first_byte_of_init_file = true};
