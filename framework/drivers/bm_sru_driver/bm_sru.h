/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * Bare-Metal ("BM") device driver header file for Signal Routing Unit (SRU)
 */

#ifndef _BM_SRU_H
#define _BM_SRU_H

#include <stdbool.h>
#include <stdint.h>

/*
 * This is the basic, stand-alone audio configuration for the SHARC Audio Module board where
 * ADAU1761 I2S signals are routed to SPORT0 of the SHARC.  In this configuration,
 * the ADAU1761 is the master as it generates the I2S clock / fs.
 */
void sru_config_sharc_sam_adau1761_master(void);

/*
 * The SHARC Audio Module board operates as an A2B slave node.  In this configuration, the I2S clock/fs
 * originate from the AD2425W (A2B) which are sent to the ADAU1761 and the SHARC.
 * Thus A2B, SHARC and ADAU1761 are all running off of the same set of I2S clock/fs.
 */
void sru_config_sharc_sam_a2b_slave(void);

/*
 * The SHARC Audio Module board operates as an A2B master node.  In this configuration, the I2S clock/s
 * originate from the ADAU1761 which are then sent to the AD2425W (A2B) and the SHARC.
 * This also drives the clock / FS for the A2B bus.  Thus, A2B, SHARC and ADAU1761 are
 * all running off of the same set of I2S clock/fs.
 */
void sru_config_sharc_sam_a2b_master(void);

/*
 * The SHARC Audio Module board operates as an A2B slave node.
 * This configuration simply routes the A2B I2S signals directly to the ADAU1761
 * without routing them through the SHARC.  This routine is helpful in ensuring that A2B
 * is being configured properly before attempting to insert the SHARC / audio processing
 * into the mix,
 */
void sru_config_sharc_sam_a2b_passthrough_slave(void);

/*
 * The SHARC Audio Module board operates as an A2B master node.
 * This configuration simply routes the ADAU1761 I2S signals directly to the AD2425W
 * without routing them through the SHARC.  This routine is helpful in ensuring that A2B
 * is being configured properly before attempting to insert the SHARC / audio processing
 * into the mix,
 */
void sru_config_sharc_sam_a2b_passthrough_master(void);

/*
 * This configuration supports the automotive expander board which connects to DAI1
 * rather than DAI0.  The TDM serial interface from the ADAU1452 is routed to
 * SPORT 4 (A=output / B=input).
 */
void sru_config_sharc_sam_adau1452_master(void);

/*
 * This sub-configuration sets up the SPDIF and routes to SPORT2.  As SPDIF is typically
 * 48fs or 64fs, we need to specify a clock divider of 4 when the rest of the audio system
 * is running at 256fs or 8 when the audio system is running at 512fs.
 */
bool sru_config_spdif(uint8_t clock_divider);

#endif //_BM_SRU_H
