/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * Bare-Metal ("BM") device driver header file for audio flow
 */

#ifndef _BM_AUDIO_FLOW_H_
#define _BM_AUDIO_FLOW_H_

#ifndef CORE0
// SHARC specific functionality

#include <services/dma/adi_dma.h>
#include <services/int/adi_int.h>
#include <services/int/adi_sec.h>
#include <sys/adi_core.h>
#include <sys/platform.h>

// Driver for GPIO functionality
#include "drivers/bm_gpio_driver/bm_gpio.h"

// Constants used to initialize the audio DMA engine
#define AUDIO_DMA_MSIZE               (2)
#define AUDIO_DMA_PSIZE               (2)
#define AUDIO_DMA_MP_OFFSET           (uint32_t)0x28000000

// Macros for configuring and enabling DMA and the SPORTS
#define SPORT_DMA_ENABLE(dmaID, enable) \
    *pREG_DMA ## dmaID ## _CFG &= (0xffffffff ^ (0x1 << BITP_DMA_CFG_EN)); \
    *pREG_DMA ## dmaID ## _CFG |= BITM_DMA_CFG_EN;

#define SPORT_ENABLE(deviceID, hSportID, secEnable, priEnable) \
    *pREG_SPORT ## deviceID ## _CTL_ ## hSportID &= (0xffffffff ^ ((0x1 << BITP_SPORT_CTL_SPENSEC) | (0x1 << BITP_SPORT_CTL_SPENPRI))); \
    *pREG_SPORT ## deviceID ## _CTL_ ## hSportID |= (secEnable << BITP_SPORT_CTL_SPENSEC) | (priEnable << BITP_SPORT_CTL_SPENPRI);

typedef enum {
    DMA_INIT_SUCCESS,
    DMA_INIT_ERR_MISSING_ISR
} DMA_INIT_RESULT;

typedef enum {
    SPORT0,
    SPORT1,
    SPORT2,
    SPORT3,
    SPORT4,
    SPORT5,
    SPORT6,
    SPORT7
} SPORT_NUMBER;

// Chained DMA descriptor used to chain our DMAs together
typedef struct
{
    void          *Next_Desc;
    int32_t       *Start_ADDR;
} SPORT_DMA_DESC_INT;

// Contains all info needed to configure a double-buffered audio DMA
typedef struct
{

    SPORT_NUMBER sport_number;

    // Number of audio channels (TDM)
    uint16_t dma_audio_channels;

    // Number of audio samples per block / frame
    uint16_t dma_audio_block_size;

    // DMA descriptors to for DMA ping-pong
    SPORT_DMA_DESC_INT dma_descriptor_tx_0_list;
    SPORT_DMA_DESC_INT dma_descriptor_tx_1_list;
    SPORT_DMA_DESC_INT dma_descriptor_rx_0_list;
    SPORT_DMA_DESC_INT dma_descriptor_rx_1_list;

    uint32_t dma_descriptor_tx_0;
    uint32_t dma_descriptor_rx_0;
    uint32_t dma_descriptor_tx_1;
    uint32_t dma_descriptor_rx_1;

    // Pointer to audio buffers to hold fixed point audio flowing to/from converters
    int *dma_tx_buffer_0;
    int *dma_tx_buffer_1;
    int *dma_rx_buffer_0;
    int *dma_rx_buffer_1;

    // Init values for the SPORT RX control registers
    uint32_t pREG_SPORT_CTL_A;
    uint32_t pREG_SPORT_MCTL_A;
    uint32_t pREG_SPORT_CS0_A;

    // Init values for the SPORT TX control registers
    uint32_t pREG_SPORT_CTL_B;
    uint32_t pREG_SPORT_MCTL_B;
    uint32_t pREG_SPORT_CS0_B;

    // Used to determine which ping pong buffer we should process
    volatile uint32_t *pREG_DMA_RX_DSCPTR_NXT;

    // Used to clear our interrupt in the DMA ISR
    volatile uint32_t *pREG_DMA_RX_STAT;

    /**
     * If this DMA is intended to generate an interrupt upon completion, set
     * generates_interrupts to true and provide a link to the callback function
     * that should be called each time a DMA interrupt completes.
     */

    bool generates_interrupts;
    void (*dma_interrupt_routine)(uint32_t, void *);
} SPORT_DMA_CONFIG;

#ifdef __cplusplus
extern "C" {
#endif

// Convert a block of floating point audio to fixed point
void audioflow_float_to_fixed(float *input,
                              int *output,
                              const uint32_t count);

// Converts a block of fixed point audio to floating point
void audioflow_fixed_to_float(int *input,
                              float *output,
                              const uint32_t count);

// Measures CPU load in MHz
float audioflow_get_cpu_load(uint64_t previous_cycle_cntr_val,
                             uint32_t audio_block_size,
                             float core_clock_frequency,
                             float audio_sample_rate);

// Initializes the DMA using the DMA init struct
DMA_INIT_RESULT audioflow_init_sport_dma(SPORT_DMA_CONFIG *sport_dma_cfg);

/**
 * @brief      Returns the value of the core cycle counter
 *
 * This routine returns the value of the core cycle counter (emuclk).  It can be
 * used in conconjuction with audioflow_get_cpu_load() to determine how many Mhz
 * are being consumed with audio processing routines.
 *
 * @return     Value of cycle counter
 */
inline uint64_t audioflow_get_cpu_cycle_counter(void) {
    return __builtin_emuclk();
}

#ifdef __cplusplus
}
#endif

#endif    // CORE0
#endif       // _BM_AUDIO_FLOW_H_
