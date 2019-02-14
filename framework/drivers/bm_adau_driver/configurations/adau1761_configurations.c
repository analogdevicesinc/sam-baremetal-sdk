/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 */

#include "../bm_adau_device.h"

// ADAU1761 Master I2S/50% 8 Channel Configuration
uint8_t ADAU1761_8ch_i2s_master_TxBuffer[] = {
#include "drivers/bm_adau_driver/configurations/ss_schematics/ADAU1761_ADAU1761_8ch_i2s_master/Exported_init_files/TxBuffer_ADAU1761.dat"
};

uint16_t ADAU1761_8ch_i2s_master_NumBytes[] = {
#include "drivers/bm_adau_driver/configurations/ss_schematics/ADAU1761_ADAU1761_8ch_i2s_master/Exported_init_files/NumBytes_ADAU1761.dat"
};

BM_ADAU_DEVICE_INIT_DATA adau1761_8ch_i2s_master = {.data_tx_buffer = ADAU1761_8ch_i2s_master_TxBuffer,
                                                    .data_num_bytes = ADAU1761_8ch_i2s_master_NumBytes,
                                                    .total_lines = sizeof(ADAU1761_8ch_i2s_master_NumBytes) / sizeof(uint16_t),
                                                    .ignore_first_byte_of_init_file = false};

// ADAU1761 Master I2S/50% 8 Channel Configuration - Enhanced
// see notes about enhanced driver in audio_system_config.h

uint16_t ADAU1761_8ch_i2s_master_enhanced_NumBytes[] = {
#include "drivers/bm_adau_driver/configurations/ss_schematics/ADAU1761_ADAU1761_8ch_i2s_enhance_master/Exported_init_files/NumBytes_ADAU1761.dat"
};

uint8_t ADAU1761_8ch_i2s_master_enhanced_TxBuffer[] = {
#include "drivers/bm_adau_driver/configurations/ss_schematics/ADAU1761_ADAU1761_8ch_i2s_enhance_master/Exported_init_files/TxBuffer_ADAU1761.dat"
};

BM_ADAU_DEVICE_INIT_DATA adau1761_8ch_i2s_enhanced_master = {.data_tx_buffer = ADAU1761_8ch_i2s_master_enhanced_TxBuffer,
                                                             .data_num_bytes = ADAU1761_8ch_i2s_master_enhanced_NumBytes,
                                                             .total_lines = sizeof(ADAU1761_8ch_i2s_master_enhanced_NumBytes) / sizeof(uint16_t),
                                                             .ignore_first_byte_of_init_file = false};

// ADAU1761 Slave I2S/50% 8 Channel Configuration
uint8_t ADAU1761_8ch_i2s_slave_TxBuffer[] = {
#include "drivers/bm_adau_driver/configurations/ss_schematics/ADAU1761_ADAU1761_8ch_i2s_slave/Exported_init_files/TxBuffer_ADAU1761.dat"
};

uint16_t ADAU1761_8ch_i2s_slave_NumBytes[] = {
#include "drivers/bm_adau_driver/configurations/ss_schematics/ADAU1761_ADAU1761_8ch_i2s_slave/Exported_init_files/NumBytes_ADAU1761.dat"
};

BM_ADAU_DEVICE_INIT_DATA adau1761_8ch_i2s_slave = {.data_tx_buffer = ADAU1761_8ch_i2s_slave_TxBuffer,
                                                   .data_num_bytes = ADAU1761_8ch_i2s_slave_NumBytes,
                                                   .total_lines = sizeof(ADAU1761_8ch_i2s_slave_NumBytes) / sizeof(uint16_t),
                                                   .ignore_first_byte_of_init_file = false};

/**
 * Note: these two channel configurations for the ADAU1761 are primarily used to support A2B boards with the ADAu1761
 * which only rely on the stereo input and outputs.
 */

// ADAU1761 Master I2S/50% 2 Channel Configuration
uint8_t ADAU1761_master_i2s_2ch_TxBuffer[] = {
#include "drivers/bm_adau_driver/configurations/ss_schematics/ADAU1761_ADAU1761_2ch_i2s_master/Exported_init_files/TxBuffer_ADAU1761.dat"
};

uint16_t ADAU1761_master_i2s_2ch_NumBytes[] = {
#include "drivers/bm_adau_driver/configurations/ss_schematics/ADAU1761_ADAU1761_2ch_i2s_master/Exported_init_files/NumBytes_ADAU1761.dat"
};

BM_ADAU_DEVICE_INIT_DATA adau1761_2ch_i2s_master = {.data_tx_buffer = ADAU1761_master_i2s_2ch_TxBuffer,
                                                    .data_num_bytes = ADAU1761_master_i2s_2ch_NumBytes,
                                                    .total_lines = sizeof(ADAU1761_master_i2s_2ch_NumBytes) / sizeof(uint16_t),
                                                    .ignore_first_byte_of_init_file = false};

// ADAU1761 Slave I2S/50% 2 Channel Configuration
uint8_t ADAU1761_2ch_i2s_slave_TxBuffer[] = {
#include "drivers/bm_adau_driver/configurations/ss_schematics/ADAU1761_ADAU1761_2ch_i2s_slave/Exported_init_files/TxBuffer_ADAU1761.dat"
};

uint16_t ADAU1761_2ch_i2s_slave_NumBytes[] = {
#include "drivers/bm_adau_driver/configurations/ss_schematics/ADAU1761_ADAU1761_2ch_i2s_slave/Exported_init_files/NumBytes_ADAU1761.dat"
};

BM_ADAU_DEVICE_INIT_DATA adau1761_2ch_i2s_slave  = {.data_tx_buffer = ADAU1761_2ch_i2s_slave_TxBuffer,
                                                    .data_num_bytes = ADAU1761_2ch_i2s_slave_NumBytes,
                                                    .total_lines = sizeof(ADAU1761_2ch_i2s_slave_NumBytes) / sizeof(uint16_t),
                                                    .ignore_first_byte_of_init_file = false};
