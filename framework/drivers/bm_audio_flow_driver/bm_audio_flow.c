/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * This provides a number of functions for managing audio processing and the flow of audio
 *
 * @file       audio_flow_simple.c
 * @brief      A generic driver for managing audio flow through the framework
 */

#include "bm_audio_flow.h"

//*******************************************************************************************
// These functions are designed to run on the SHARC cores so don't include if compiled on ARM
//*******************************************************************************************
#ifndef CORE0

/**
 * These are helpful functions to assist with the DMA modifier value calculations
 */
static int32_t calculate_dma_ymod(uint32_t channels,
                                  uint32_t audio_block_size) {
    return (-((channels * audio_block_size) * 4 - audio_block_size * 4) + 4);
}

static uint32_t calculate_dma_xmod(uint32_t audio_block_size) {
    return audio_block_size * 4;
}

/**
 * @brief      Converts a block of audio from floating point to fixed point
 *
 * This routine converts a block of floating point audio (-1.0->0.9999) to
 * signed, fixed point audio, fractional audio
 *
 * @param[in]  input   pointer to input buffer
 * @param        output  pointer to output buffer
 * @param       count   number of points to convert
 * @return     None
 */
#pragma optimize_for_speed
void audioflow_float_to_fixed(float *input,
                              int *output,
                              const uint32_t count) {
    #pragma SIMD_for
    #pragma loop_count(2,,2)
    for (uint32_t i = 0; i < count; i++)
    {
        output[i] = __builtin_conv_fix_by(__builtin_fclipf((input[i]), 0.9999), 31);
    }
}

/**
 * @brief      Converts a block of audio from fixed point to floating point
 *
 * This routine converts a block of signed, fixed point audio, fractional audio
 * to floating point
 *
 * @param[in]  input   pointer to input buffer
 * @param        output  pointer to output buffer
 * @param       count   number of points to convert
 * @return     None
 */
#pragma optimize_for_speed
void audioflow_fixed_to_float(int *input,
                              float *output,
                              const uint32_t count) {

    #pragma SIMD_for
    #pragma loop_count(2,,2)
    for (uint32_t i = 0; i < count; i++)
    {
        output[i] = __builtin_conv_float_by(input[i], -31);
    }
}

/**
 * @brief      Calculates current CPU loading in MHz
 *
 * This routine uses the emuclk cycle counter to calculate the current MHz loading
 * of the SHARC core around a current segment of code.  audioflow_get_cpu_cycle_counter
 * should be called at the beginning of the code segment to benchmark and this function
 * should be called at the end.
 *
 * @param[in]  previous_cycle_cntr_val   value of the cycle counter at beginning of code segment
 * @param        audio_block_size   audio block processing size of the framework
 * @param       core_clock_frequency  clock frequency of CPU in Hz
 * @param       audio_sample_rate  audio sampling rate in Hz
 * @return     Value of current CPU load in MHz
 */
float audioflow_get_cpu_load(uint64_t previous_cycle_cntr_val,
                             uint32_t audio_block_size,
                             float core_clock_frequency,
                             float audio_sample_rate){

    uint64_t cycleCntrResult;

    // Get the current emuclk value (increments each processor cycle)
    cycleCntrResult = __builtin_emuclk() - previous_cycle_cntr_val;

    float fsPeriod    =     1.0 / audio_sample_rate;

    float totalCycles =     fsPeriod *
                        (float)audio_block_size *
                        core_clock_frequency;

    float cpuLoad = (core_clock_frequency * (1 / 1000000.0)) * (float)cycleCntrResult / totalCycles;

    // Single stepping through audio processing can increase cycle counts
    // return 0 if value is about max clock speed of the ADSP-SC589 of 500MHz
    if (cpuLoad > 500.0) return 0;

    return cpuLoad;
}

/**
 * @brief      Initializes SPORT DMA for double-buffered audio flow
 *
 * This routine sets up the DMA and SPORT peripherals properly for this
 * specific framework.  It also sets up the interrupts for both the DMA
 * callback as well as the software interrupt that actually triggers
 * our audio callback function.
 *
 * @param[in]  sport_dma_cfg   pointer to structure containing our init data
 *
 * @return     Returns success or failure based on result of operation
 */
DMA_INIT_RESULT audioflow_init_sport_dma(SPORT_DMA_CONFIG *sport_dma_cfg) {

    // Ensure the struct has a pointer to an ISR if this SPORT is to generate interrupts
    if (sport_dma_cfg->generates_interrupts && sport_dma_cfg->dma_interrupt_routine == NULL) {
        return DMA_INIT_ERR_MISSING_ISR;
    }

    int DMA_TX_Config = (((0               << BITP_DMA_CFG_WNR) & BITM_DMA_CFG_WNR)     | // SPORT write data (memory read) to DAC
                         ((1               << BITP_DMA_CFG_TWOD) & BITM_DMA_CFG_TWOD)   |
                         ((0               << BITP_DMA_CFG_INT) & BITM_DMA_CFG_INT)     |
                         ((AUDIO_DMA_MSIZE << BITP_DMA_CFG_MSIZE) & BITM_DMA_CFG_MSIZE) | // 4 bytes - memory transfer size
                         ((AUDIO_DMA_PSIZE << BITP_DMA_CFG_PSIZE) & BITM_DMA_CFG_PSIZE) | // 4 bytes - peripheral transfer size
                         ((0               << BITP_DMA_CFG_SYNC) & BITM_DMA_CFG_SYNC)   |
                         ((0               << BITP_DMA_CFG_TWAIT) & BITM_DMA_CFG_TWAIT) |
                         ((0               << BITP_DMA_CFG_TRIG) & BITM_DMA_CFG_TRIG)   |
                         ENUM_DMA_CFG_FETCH02);

    int DMA_RX_Config = (((1               << BITP_DMA_CFG_WNR) & BITM_DMA_CFG_WNR)     | // SPORT receives data (memory write) from ADC
                         ((1               << BITP_DMA_CFG_TWOD) & BITM_DMA_CFG_TWOD)   |
                         ((2               << BITP_DMA_CFG_INT) & BITM_DMA_CFG_INT)     | // enable interrupt (on YCNT expired for 2D-DMA) upon buffer read complete
                         ((AUDIO_DMA_MSIZE << BITP_DMA_CFG_MSIZE) & BITM_DMA_CFG_MSIZE) |
                         ((AUDIO_DMA_PSIZE << BITP_DMA_CFG_PSIZE) & BITM_DMA_CFG_PSIZE) |
                         ((0               << BITP_DMA_CFG_SYNC) & BITM_DMA_CFG_SYNC)   |
                         ((0               << BITP_DMA_CFG_TWAIT) & BITM_DMA_CFG_TWAIT) |
                         ((1               << BITP_DMA_CFG_TRIG) & BITM_DMA_CFG_TRIG)   | // enable trigger
                         ENUM_DMA_CFG_FETCH02);

    // Initialize our DMA chain pointers
    sport_dma_cfg->dma_descriptor_tx_0 = (uint32_t)&sport_dma_cfg->dma_descriptor_tx_0_list | AUDIO_DMA_MP_OFFSET;
    sport_dma_cfg->dma_descriptor_rx_0 = (uint32_t)&sport_dma_cfg->dma_descriptor_rx_0_list | AUDIO_DMA_MP_OFFSET;
    sport_dma_cfg->dma_descriptor_tx_1 = (uint32_t)&sport_dma_cfg->dma_descriptor_tx_1_list | AUDIO_DMA_MP_OFFSET;
    sport_dma_cfg->dma_descriptor_rx_1 = (uint32_t)&sport_dma_cfg->dma_descriptor_rx_1_list | AUDIO_DMA_MP_OFFSET;

    sport_dma_cfg->dma_descriptor_tx_0_list.Next_Desc  = (void *)sport_dma_cfg->dma_descriptor_tx_1;
    sport_dma_cfg->dma_descriptor_tx_0_list.Start_ADDR = (int32_t *)(((uint32_t)sport_dma_cfg->dma_tx_buffer_0 * 4) | AUDIO_DMA_MP_OFFSET);
    sport_dma_cfg->dma_descriptor_tx_1_list.Next_Desc  = (void *)sport_dma_cfg->dma_descriptor_tx_0;
    sport_dma_cfg->dma_descriptor_tx_1_list.Start_ADDR = (int32_t *)(((uint32_t)sport_dma_cfg->dma_tx_buffer_1 * 4) | AUDIO_DMA_MP_OFFSET);

    sport_dma_cfg->dma_descriptor_rx_0_list.Next_Desc  = (void *)sport_dma_cfg->dma_descriptor_rx_1;
    sport_dma_cfg->dma_descriptor_rx_0_list.Start_ADDR = (int32_t *)(((uint32_t)sport_dma_cfg->dma_rx_buffer_0 * 4) | AUDIO_DMA_MP_OFFSET);
    sport_dma_cfg->dma_descriptor_rx_1_list.Next_Desc  = (void *)sport_dma_cfg->dma_descriptor_rx_0;
    sport_dma_cfg->dma_descriptor_rx_1_list.Start_ADDR = (int32_t *)(((uint32_t)sport_dma_cfg->dma_rx_buffer_1 * 4) | AUDIO_DMA_MP_OFFSET);

    switch (sport_dma_cfg->sport_number) {

        // SPORT 0
        case SPORT0:

            // Used to determine which buffer we're processing when double buffering during the DMA ISR
            sport_dma_cfg->pREG_DMA_RX_DSCPTR_NXT = (volatile uint32_t *)pREG_DMA1_DSCPTR_NXT;

            // Used to clear the appropriate DMA interrupt
            sport_dma_cfg->pREG_DMA_RX_STAT = pREG_DMA1_STAT;

            *pREG_SPU0_SECUREP66 = 0x3;    // SPORT 0A = DMA0 = TX
            *pREG_SPU0_SECUREP67 = 0x3;    // SPORT 0B = DMA1 = RX

            *pREG_DMA0_DSCPTR_NXT = (void *)sport_dma_cfg->dma_descriptor_tx_0;
            *pREG_DMA1_DSCPTR_NXT = (void *)sport_dma_cfg->dma_descriptor_rx_0;

            *pREG_DMA0_XCNT = sport_dma_cfg->dma_audio_channels;
            *pREG_DMA1_XCNT = sport_dma_cfg->dma_audio_channels;

            *pREG_DMA0_YCNT = sport_dma_cfg->dma_audio_block_size;
            *pREG_DMA1_YCNT = sport_dma_cfg->dma_audio_block_size;

            *pREG_DMA0_XMOD = calculate_dma_xmod(sport_dma_cfg->dma_audio_block_size);
            *pREG_DMA1_XMOD = calculate_dma_xmod(sport_dma_cfg->dma_audio_block_size);

            *pREG_DMA0_YMOD = calculate_dma_ymod(sport_dma_cfg->dma_audio_channels, sport_dma_cfg->dma_audio_block_size);
            *pREG_DMA1_YMOD = calculate_dma_ymod(sport_dma_cfg->dma_audio_channels, sport_dma_cfg->dma_audio_block_size);

            *pREG_DMA0_CFG = DMA_TX_Config | (4 << BITP_DMA_CFG_FLOW);   //descriptor list mode = 4
            *pREG_DMA1_CFG = DMA_RX_Config | (4 << BITP_DMA_CFG_FLOW);

            // Transmit
            *pREG_SPORT0_CTL_A  = sport_dma_cfg->pREG_SPORT_CTL_A;
            *pREG_SPORT0_MCTL_A = sport_dma_cfg->pREG_SPORT_MCTL_A;
            *pREG_SPORT0_CS0_A  = sport_dma_cfg->pREG_SPORT_CS0_A;

            // Receive
            *pREG_SPORT0_CTL_B  = sport_dma_cfg->pREG_SPORT_CTL_B;
            *pREG_SPORT0_MCTL_B = sport_dma_cfg->pREG_SPORT_MCTL_B;
            *pREG_SPORT0_CS0_B  = sport_dma_cfg->pREG_SPORT_CS0_B;

            if (sport_dma_cfg->generates_interrupts) {
                // Set up interrupt handler for SPORT RX
                adi_int_InstallHandler(INTR_SPORT0_B_DMA,
                                       (ADI_INT_HANDLER_PTR)sport_dma_cfg->dma_interrupt_routine,
                                       (void *)sport_dma_cfg,
                                       true);
            }

            break;

        // SPORT 1
        case SPORT1:

            sport_dma_cfg->pREG_DMA_RX_DSCPTR_NXT = (volatile uint32_t *)pREG_DMA3_DSCPTR_NXT;
            sport_dma_cfg->pREG_DMA_RX_STAT = pREG_DMA3_STAT;

            *pREG_SPU0_SECUREP68 = 0x3;    // SPORT 1A = DMA2 = TX
            *pREG_SPU0_SECUREP69 = 0x3;    // SPORT 1B = DMA3 = RX

            *pREG_DMA2_DSCPTR_NXT = (void *)sport_dma_cfg->dma_descriptor_tx_0;
            *pREG_DMA3_DSCPTR_NXT = (void *)sport_dma_cfg->dma_descriptor_rx_0;

            *pREG_DMA2_XCNT = sport_dma_cfg->dma_audio_channels;
            *pREG_DMA3_XCNT = sport_dma_cfg->dma_audio_channels;

            *pREG_DMA2_YCNT = sport_dma_cfg->dma_audio_block_size;
            *pREG_DMA3_YCNT = sport_dma_cfg->dma_audio_block_size;

            *pREG_DMA2_XMOD = calculate_dma_xmod(sport_dma_cfg->dma_audio_block_size);
            *pREG_DMA3_XMOD = calculate_dma_xmod(sport_dma_cfg->dma_audio_block_size);

            *pREG_DMA2_YMOD = calculate_dma_ymod(sport_dma_cfg->dma_audio_channels, sport_dma_cfg->dma_audio_block_size);
            *pREG_DMA3_YMOD = calculate_dma_ymod(sport_dma_cfg->dma_audio_channels, sport_dma_cfg->dma_audio_block_size);

            *pREG_DMA2_CFG = DMA_TX_Config | (4 << BITP_DMA_CFG_FLOW);   //descriptor list mode = 4
            *pREG_DMA3_CFG = DMA_RX_Config | (4 << BITP_DMA_CFG_FLOW);

            // Transmit
            *pREG_SPORT1_CTL_A  = sport_dma_cfg->pREG_SPORT_CTL_A;
            *pREG_SPORT1_MCTL_A = sport_dma_cfg->pREG_SPORT_MCTL_A;
            *pREG_SPORT1_CS0_A  = sport_dma_cfg->pREG_SPORT_CS0_A;

            // Receive
            *pREG_SPORT1_CTL_B  = sport_dma_cfg->pREG_SPORT_CTL_B;
            *pREG_SPORT1_MCTL_B = sport_dma_cfg->pREG_SPORT_MCTL_B;
            *pREG_SPORT1_CS0_B  = sport_dma_cfg->pREG_SPORT_CS0_B;

            if (sport_dma_cfg->generates_interrupts) {
                // Set up interrupt handler for SPORT RX
                adi_int_InstallHandler(INTR_SPORT1_B_DMA,
                                       (ADI_INT_HANDLER_PTR)sport_dma_cfg->dma_interrupt_routine,
                                       (void *)sport_dma_cfg,
                                       true);
            }

            break;

        // SPORT 2
        case SPORT2:

            sport_dma_cfg->pREG_DMA_RX_DSCPTR_NXT = (volatile uint32_t *)pREG_DMA5_DSCPTR_NXT;
            sport_dma_cfg->pREG_DMA_RX_STAT = pREG_DMA5_STAT;

            *pREG_SPU0_SECUREP70 = 0x3;    // SPORT 2A = DMA4 = TX
            *pREG_SPU0_SECUREP71 = 0x3;    // SPORT 2B = DMA5 = RX

            *pREG_DMA4_DSCPTR_NXT = (void *)sport_dma_cfg->dma_descriptor_tx_0;
            *pREG_DMA5_DSCPTR_NXT = (void *)sport_dma_cfg->dma_descriptor_rx_0;

            *pREG_DMA4_XCNT = sport_dma_cfg->dma_audio_channels;
            *pREG_DMA5_XCNT = sport_dma_cfg->dma_audio_channels;

            *pREG_DMA4_YCNT = sport_dma_cfg->dma_audio_block_size;
            *pREG_DMA5_YCNT = sport_dma_cfg->dma_audio_block_size;

            *pREG_DMA4_XMOD = calculate_dma_xmod(sport_dma_cfg->dma_audio_block_size);
            *pREG_DMA5_XMOD = calculate_dma_xmod(sport_dma_cfg->dma_audio_block_size);

            *pREG_DMA4_YMOD = calculate_dma_ymod(sport_dma_cfg->dma_audio_channels, sport_dma_cfg->dma_audio_block_size);
            *pREG_DMA5_YMOD = calculate_dma_ymod(sport_dma_cfg->dma_audio_channels, sport_dma_cfg->dma_audio_block_size);

            *pREG_DMA4_CFG = DMA_TX_Config | (4 << BITP_DMA_CFG_FLOW);   //descriptor list mode = 4
            *pREG_DMA5_CFG = DMA_RX_Config | (4 << BITP_DMA_CFG_FLOW);

            // Transmit
            *pREG_SPORT2_CTL_A  = sport_dma_cfg->pREG_SPORT_CTL_A;
            *pREG_SPORT2_MCTL_A = sport_dma_cfg->pREG_SPORT_MCTL_A;
            *pREG_SPORT2_CS0_A  = sport_dma_cfg->pREG_SPORT_CS0_A;

            // Receive
            *pREG_SPORT2_CTL_B  = sport_dma_cfg->pREG_SPORT_CTL_B;
            *pREG_SPORT2_MCTL_B = sport_dma_cfg->pREG_SPORT_MCTL_B;
            *pREG_SPORT2_CS0_B  = sport_dma_cfg->pREG_SPORT_CS0_B;

            if (sport_dma_cfg->generates_interrupts) {
                // Set up interrupt handler for SPORT RX
                adi_int_InstallHandler(INTR_SPORT2_B_DMA,
                                       (ADI_INT_HANDLER_PTR)sport_dma_cfg->dma_interrupt_routine,
                                       (void *)sport_dma_cfg,
                                       true);
            }

            break;

        // SPORT 3
        case SPORT3:

            sport_dma_cfg->pREG_DMA_RX_DSCPTR_NXT = (volatile uint32_t *)pREG_DMA7_DSCPTR_NXT;
            sport_dma_cfg->pREG_DMA_RX_STAT = pREG_DMA7_STAT;

            *pREG_SPU0_SECUREP72 = 0x3;    // SPORT 3A = DMA6 = TX
            *pREG_SPU0_SECUREP73 = 0x3;    // SPORT 3B = DMA7 = RX

            *pREG_DMA6_DSCPTR_NXT = (void *)sport_dma_cfg->dma_descriptor_tx_0;
            *pREG_DMA7_DSCPTR_NXT = (void *)sport_dma_cfg->dma_descriptor_rx_0;

            *pREG_DMA6_XCNT = sport_dma_cfg->dma_audio_channels;
            *pREG_DMA7_XCNT = sport_dma_cfg->dma_audio_channels;

            *pREG_DMA6_YCNT = sport_dma_cfg->dma_audio_block_size;
            *pREG_DMA7_YCNT = sport_dma_cfg->dma_audio_block_size;

            *pREG_DMA6_XMOD = calculate_dma_xmod(sport_dma_cfg->dma_audio_block_size);
            *pREG_DMA7_XMOD = calculate_dma_xmod(sport_dma_cfg->dma_audio_block_size);

            *pREG_DMA6_YMOD = calculate_dma_ymod(sport_dma_cfg->dma_audio_channels, sport_dma_cfg->dma_audio_block_size);
            *pREG_DMA7_YMOD = calculate_dma_ymod(sport_dma_cfg->dma_audio_channels, sport_dma_cfg->dma_audio_block_size);

            *pREG_DMA6_CFG = DMA_TX_Config | (4 << BITP_DMA_CFG_FLOW);   //descriptor list mode = 4
            *pREG_DMA7_CFG = DMA_RX_Config | (4 << BITP_DMA_CFG_FLOW);

            // Transmit
            *pREG_SPORT3_CTL_A  = sport_dma_cfg->pREG_SPORT_CTL_A;
            *pREG_SPORT3_MCTL_A = sport_dma_cfg->pREG_SPORT_MCTL_A;
            *pREG_SPORT3_CS0_A  = sport_dma_cfg->pREG_SPORT_CS0_A;

            // Receive
            *pREG_SPORT3_CTL_B  = sport_dma_cfg->pREG_SPORT_CTL_B;
            *pREG_SPORT3_MCTL_B = sport_dma_cfg->pREG_SPORT_MCTL_B;
            *pREG_SPORT3_CS0_B  = sport_dma_cfg->pREG_SPORT_CS0_B;

            if (sport_dma_cfg->generates_interrupts) {
                // Set up interrupt handler for SPORT RX
                adi_int_InstallHandler(INTR_SPORT3_B_DMA,
                                       (ADI_INT_HANDLER_PTR)sport_dma_cfg->dma_interrupt_routine,
                                       (void *)sport_dma_cfg,
                                       true);
            }

            break;

        // DMA channels 8/9 are for MDMA0 SRC/DEST

        // SPORT 4
        case SPORT4:

            sport_dma_cfg->pREG_DMA_RX_DSCPTR_NXT = (volatile uint32_t *)pREG_DMA11_DSCPTR_NXT;
            sport_dma_cfg->pREG_DMA_RX_STAT = pREG_DMA11_STAT;

            *pREG_SPU0_SECUREP74 = 0x3;    // SPORT 4A = DMA10 = TX
            *pREG_SPU0_SECUREP75 = 0x3;    // SPORT 4B = DMA11 = RX

            *pREG_DMA10_DSCPTR_NXT = (void *)sport_dma_cfg->dma_descriptor_tx_0;
            *pREG_DMA11_DSCPTR_NXT = (void *)sport_dma_cfg->dma_descriptor_rx_0;

            *pREG_DMA10_XCNT =  sport_dma_cfg->dma_audio_channels;
            *pREG_DMA11_XCNT =  sport_dma_cfg->dma_audio_channels;

            *pREG_DMA10_YCNT = sport_dma_cfg->dma_audio_block_size;
            *pREG_DMA11_YCNT = sport_dma_cfg->dma_audio_block_size;

            *pREG_DMA10_XMOD = calculate_dma_xmod(sport_dma_cfg->dma_audio_block_size);
            *pREG_DMA11_XMOD = calculate_dma_xmod(sport_dma_cfg->dma_audio_block_size);

            *pREG_DMA10_YMOD = calculate_dma_ymod(sport_dma_cfg->dma_audio_channels, sport_dma_cfg->dma_audio_block_size);
            *pREG_DMA11_YMOD = calculate_dma_ymod(sport_dma_cfg->dma_audio_channels, sport_dma_cfg->dma_audio_block_size);

            *pREG_DMA10_CFG = DMA_TX_Config | (4 << BITP_DMA_CFG_FLOW);   //descriptor list mode = 4
            *pREG_DMA11_CFG = DMA_RX_Config | (4 << BITP_DMA_CFG_FLOW);

            // Transmit
            *pREG_SPORT4_CTL_A  = sport_dma_cfg->pREG_SPORT_CTL_A;
            *pREG_SPORT4_MCTL_A = sport_dma_cfg->pREG_SPORT_MCTL_A;
            *pREG_SPORT4_CS0_A  = sport_dma_cfg->pREG_SPORT_CS0_A;

            // Receive
            *pREG_SPORT4_CTL_B  = sport_dma_cfg->pREG_SPORT_CTL_B;
            *pREG_SPORT4_MCTL_B = sport_dma_cfg->pREG_SPORT_MCTL_B;
            *pREG_SPORT4_CS0_B  = sport_dma_cfg->pREG_SPORT_CS0_B;

            if (sport_dma_cfg->generates_interrupts) {
                // Set up interrupt handler for SPORT RX
                adi_int_InstallHandler(INTR_SPORT4_B_DMA,
                                       (ADI_INT_HANDLER_PTR)sport_dma_cfg->dma_interrupt_routine,
                                       (void *)sport_dma_cfg,
                                       true);
            }

            break;

        // SPORT 5
        case SPORT5:

            sport_dma_cfg->pREG_DMA_RX_DSCPTR_NXT = (volatile uint32_t *)pREG_DMA13_DSCPTR_NXT;
            sport_dma_cfg->pREG_DMA_RX_STAT = pREG_DMA13_STAT;

            *pREG_SPU0_SECUREP76 = 0x3;    // SPORT 5A = DMA12 = TX
            *pREG_SPU0_SECUREP77 = 0x3;    // SPORT 5B = DMA13 = RX

            *pREG_DMA12_DSCPTR_NXT = (void *)sport_dma_cfg->dma_descriptor_tx_0;
            *pREG_DMA13_DSCPTR_NXT = (void *)sport_dma_cfg->dma_descriptor_rx_0;

            *pREG_DMA12_XCNT =  sport_dma_cfg->dma_audio_channels;
            *pREG_DMA13_XCNT =  sport_dma_cfg->dma_audio_channels;

            *pREG_DMA12_YCNT = sport_dma_cfg->dma_audio_block_size;
            *pREG_DMA13_YCNT = sport_dma_cfg->dma_audio_block_size;

            *pREG_DMA12_XMOD = calculate_dma_xmod(sport_dma_cfg->dma_audio_block_size);
            *pREG_DMA13_XMOD = calculate_dma_xmod(sport_dma_cfg->dma_audio_block_size);

            *pREG_DMA12_YMOD = calculate_dma_ymod(sport_dma_cfg->dma_audio_channels, sport_dma_cfg->dma_audio_block_size);
            *pREG_DMA13_YMOD = calculate_dma_ymod(sport_dma_cfg->dma_audio_channels, sport_dma_cfg->dma_audio_block_size);

            *pREG_DMA12_CFG = DMA_TX_Config | (4 << BITP_DMA_CFG_FLOW);   //descriptor list mode = 4
            *pREG_DMA13_CFG = DMA_RX_Config | (4 << BITP_DMA_CFG_FLOW);

            // Transmit
            *pREG_SPORT5_CTL_A  = sport_dma_cfg->pREG_SPORT_CTL_A;
            *pREG_SPORT5_MCTL_A = sport_dma_cfg->pREG_SPORT_MCTL_A;
            *pREG_SPORT5_CS0_A  = sport_dma_cfg->pREG_SPORT_CS0_A;

            // Receive
            *pREG_SPORT5_CTL_B  = sport_dma_cfg->pREG_SPORT_CTL_B;
            *pREG_SPORT5_MCTL_B = sport_dma_cfg->pREG_SPORT_MCTL_B;
            *pREG_SPORT5_CS0_B  = sport_dma_cfg->pREG_SPORT_CS0_B;

            if (sport_dma_cfg->generates_interrupts) {
                // Set up interrupt handler for SPORT RX
                adi_int_InstallHandler(INTR_SPORT5_B_DMA,
                                       (ADI_INT_HANDLER_PTR)sport_dma_cfg->dma_interrupt_routine,
                                       (void *)sport_dma_cfg,
                                       true);
            }

            break;

        // SPORT 6
        case SPORT6:

            sport_dma_cfg->pREG_DMA_RX_DSCPTR_NXT = (volatile uint32_t *)pREG_DMA15_DSCPTR_NXT;
            sport_dma_cfg->pREG_DMA_RX_STAT = pREG_DMA15_STAT;

            *pREG_SPU0_SECUREP78 = 0x3;    // SPORT 6A = DMA14 = TX
            *pREG_SPU0_SECUREP79 = 0x3;    // SPORT 6B = DMA15 = RX

            *pREG_DMA14_DSCPTR_NXT = (void *)sport_dma_cfg->dma_descriptor_tx_0;
            *pREG_DMA15_DSCPTR_NXT = (void *)sport_dma_cfg->dma_descriptor_rx_0;

            *pREG_DMA14_XCNT =  sport_dma_cfg->dma_audio_channels;
            *pREG_DMA15_XCNT =  sport_dma_cfg->dma_audio_channels;

            *pREG_DMA14_YCNT = sport_dma_cfg->dma_audio_block_size;
            *pREG_DMA15_YCNT = sport_dma_cfg->dma_audio_block_size;

            *pREG_DMA14_XMOD = calculate_dma_xmod(sport_dma_cfg->dma_audio_block_size);
            *pREG_DMA15_XMOD = calculate_dma_xmod(sport_dma_cfg->dma_audio_block_size);

            *pREG_DMA14_YMOD = calculate_dma_ymod(sport_dma_cfg->dma_audio_channels, sport_dma_cfg->dma_audio_block_size);
            *pREG_DMA15_YMOD = calculate_dma_ymod(sport_dma_cfg->dma_audio_channels, sport_dma_cfg->dma_audio_block_size);

            *pREG_DMA14_CFG = DMA_TX_Config | (4 << BITP_DMA_CFG_FLOW);   //descriptor list mode = 4
            *pREG_DMA15_CFG = DMA_RX_Config | (4 << BITP_DMA_CFG_FLOW);

            // Transmit
            *pREG_SPORT6_CTL_A  = sport_dma_cfg->pREG_SPORT_CTL_A;
            *pREG_SPORT6_MCTL_A = sport_dma_cfg->pREG_SPORT_MCTL_A;
            *pREG_SPORT6_CS0_A  = sport_dma_cfg->pREG_SPORT_CS0_A;

            // Receive
            *pREG_SPORT6_CTL_B  = sport_dma_cfg->pREG_SPORT_CTL_B;
            *pREG_SPORT6_MCTL_B = sport_dma_cfg->pREG_SPORT_MCTL_B;
            *pREG_SPORT6_CS0_B  = sport_dma_cfg->pREG_SPORT_CS0_B;

            if (sport_dma_cfg->generates_interrupts) {
                // Set up interrupt handler for SPORT RX
                adi_int_InstallHandler(INTR_SPORT6_B_DMA,
                                       (ADI_INT_HANDLER_PTR)sport_dma_cfg->dma_interrupt_routine,
                                       (void *)sport_dma_cfg,
                                       true);
            }

            break;

        // SPORT 7
        case SPORT7:

            sport_dma_cfg->pREG_DMA_RX_DSCPTR_NXT = (volatile uint32_t *)pREG_DMA17_DSCPTR_NXT;
            sport_dma_cfg->pREG_DMA_RX_STAT = pREG_DMA17_STAT;

            *pREG_SPU0_SECUREP80 = 0x3;    // SPORT 7A = DMA16 = TX
            *pREG_SPU0_SECUREP81 = 0x3;    // SPORT 7B = DMA17 = RX

            *pREG_DMA16_DSCPTR_NXT = (void *)sport_dma_cfg->dma_descriptor_tx_0;
            *pREG_DMA17_DSCPTR_NXT = (void *)sport_dma_cfg->dma_descriptor_rx_0;

            *pREG_DMA16_XCNT =  sport_dma_cfg->dma_audio_channels;
            *pREG_DMA17_XCNT =  sport_dma_cfg->dma_audio_channels;

            *pREG_DMA16_YCNT = sport_dma_cfg->dma_audio_block_size;
            *pREG_DMA17_YCNT = sport_dma_cfg->dma_audio_block_size;

            *pREG_DMA16_XMOD = calculate_dma_xmod(sport_dma_cfg->dma_audio_block_size);
            *pREG_DMA17_XMOD = calculate_dma_xmod(sport_dma_cfg->dma_audio_block_size);

            *pREG_DMA16_YMOD = calculate_dma_ymod(sport_dma_cfg->dma_audio_channels, sport_dma_cfg->dma_audio_block_size);
            *pREG_DMA17_YMOD = calculate_dma_ymod(sport_dma_cfg->dma_audio_channels, sport_dma_cfg->dma_audio_block_size);

            *pREG_DMA16_CFG = DMA_TX_Config | (4 << BITP_DMA_CFG_FLOW);   //descriptor list mode = 4
            *pREG_DMA17_CFG = DMA_RX_Config | (4 << BITP_DMA_CFG_FLOW);

            // Transmit
            *pREG_SPORT7_CTL_A  = sport_dma_cfg->pREG_SPORT_CTL_A;
            *pREG_SPORT7_MCTL_A = sport_dma_cfg->pREG_SPORT_MCTL_A;
            *pREG_SPORT7_CS0_A  = sport_dma_cfg->pREG_SPORT_CS0_A;

            // Receive
            *pREG_SPORT7_CTL_B  = sport_dma_cfg->pREG_SPORT_CTL_B;
            *pREG_SPORT7_MCTL_B = sport_dma_cfg->pREG_SPORT_MCTL_B;
            *pREG_SPORT7_CS0_B  = sport_dma_cfg->pREG_SPORT_CS0_B;

            if (sport_dma_cfg->generates_interrupts) {
                // Set up interrupt handler for SPORT RX
                adi_int_InstallHandler(INTR_SPORT7_B_DMA,
                                       (ADI_INT_HANDLER_PTR)sport_dma_cfg->dma_interrupt_routine,
                                       (void *)sport_dma_cfg,
                                       true);
            }

            break;
    }

    return DMA_INIT_SUCCESS;
}

#endif
