/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * Bare-Metal ("BM") device driver header file for Signal Routing Unit (SRU).
 * Provides commonly used SRU / DAI configurations for the SHARC Audio Module board.
 *
 * This relies heavily on the set of SRU macros provided in sruSC589.h.
 *
 */

#include "bm_sru.h"

#include <cdefSC589.h>
#include <defSC589.h>
#include <drivers/spdif/adi_spdif_rx.h>
#include <sruSC589.h>
#include <sys/platform.h>

// Bring this in to get our current SHARC Audio Module board hardware
#include "common/audio_system_config.h"

// GPIO support
#include "drivers/bm_gpio_driver/bm_gpio.h"

/**
 * @brief      This function provides common set up of the DAI on the SHARC Audio Module board
 *
 * It routes the I2S signals between the SC589 and the ADAU1761 to DAI0_13-16
 * so they can be probed / debugged. Furthermore, it sets the I2C address lines
 * for the ADAU1761.
 */
static void sru_init_sharc_sam(void){
    /* PADS0 DAI0/1 Port Input Enable Control Register */
    *pREG_PADS0_DAI0_IE = (unsigned int)0x001FFFFE;
    *pREG_PADS0_DAI1_IE = (unsigned int)0x001FFFFE;

    // ADAU1761 I2C Address pins set low
    SRU(HIGH, DAI0_PBEN05_I);        // configure DAI as an output
    SRU(LOW, DAI0_PB05_I);            // set 1761 I2C address pins low

    // Set pin 6 of port b as an output and drive it low
    BM_GPIO_PORTPIN pb_06 = BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_B, 6);
    gpio_setup(pb_06, GPIO_OUTPUT);  // set this pin as an output
    gpio_write(pb_06, GPIO_LOW);

    // Set DAI0 pin 6 (MCLK) as an input
    SRU(LOW, DAI0_PBEN06_I);        // configure DAI0_06 as an input (MCLK)

    // Enable this code to route the I2S signals to DAI pins on the expansion header for debugging
    #if 1
    // And set unused DAI0 pins as outputs for debug
    SRU(HIGH,  DAI0_PBEN13_I);            // ADAU1761 DAC data (debug) is an output
    SRU(HIGH,  DAI0_PBEN14_I);            // ADAU1761 ADC data (debug) is an output
    SRU(HIGH,  DAI0_PBEN15_I);            // ADAU1761 CLK (debug) is an output
    SRU(HIGH,  DAI0_PBEN16_I);            // ADAU1761 FS (debug) is an output

    // Route ADAU1761 signals to higher DAI pins on expansion header for debug purposes
    SRU(DAI0_PB01_O, DAI0_PB13_I);
    SRU(DAI0_PB02_O, DAI0_PB14_I);
    SRU(DAI0_PB03_O, DAI0_PB15_I);
    SRU(DAI0_PB04_O, DAI0_PB16_I);
    #endif
}

/**
 * @brief      This function routes the SPDIF transmitter and received to SPORT2.
 *
 * The SPDIF receiver is routed through the sample-rate-converters (SRC)
 * so that the SPDIF signals are in the same clock domain as the other audio
 * signals in the system.
 *
 * We want the SPDIF input and output signals to exist in the same clock domain
 * as the rest of the audio on our system (ADAU1761 and A2B).  However, in some
 * cases our bit clock for our audio signals will be much higher (256fs or 512fs
 * for 8 and 16 channel TDM configurations).  While the framesyncs will still
 * run at the system sample rate, we need to generate 64fs bit clocks
 *
 * @param[in]  clock_divider  The clock divider
 *
 * @return     false if invalid clock divider provider, true otherwise
 */
bool sru_config_spdif(uint8_t clock_divider) {

    // For now, this supports 64fs (2 channel), 256fs (8 channel) and 512fs (16 channel)
    if (clock_divider != 1 && clock_divider != 4 && clock_divider != 8) {
        return false;
    }

    // Configure DAI19 (RX) as input and 20 (TX) as an output
    SRU(HIGH, DAI0_PBEN20_I);        // SPDIF TX is an output
    SRU(LOW,  DAI0_PBEN19_I);        // SPDIF RX is an input

    // Connect DAI pins to SPDIF RX pin
    SRU(DAI0_PB19_O, SPDIF0_RX_I);     // route DAI19 to SPDIF RX

    // route SPDIF TX pin to DAI0 pin 20
    SRU(SPDIF0_TX_O, DAI0_PB20_I);

    // Determine if we need to generate a new i2S signal for SPDIF
    if (clock_divider == 4 || clock_divider == 8) {

        /*
         *  If our audio from the ADAU1761 (or A2B or automotive) is running at
         *  256fs or 512fs, we need to generate a set of 64fs signals to drive
         *  SPDIF (since we only have 2 audio channels on SPDIF).  We need to make
         *  sure that the framesyncs are synchronous since the audio framework
         *  is triggered off of one DMA.  We'll use the PCG (precision clock generator)
         *  to divide down our 256fs or 512fs clock to a 64fs clock and we'll use the
         *  external sync capability of the PCG to ensure our framesyncs are in phase.
         */

        // route ADAU1761 BCLK signal to PCGA external clock input
        SRU(DAI0_PB03_O, PCG0_EXTCLKA_I);

        // route ADAU1761 FS signal to sync input
        SRU(DAI0_PB04_O, PCG0_SYNC_CLKA_I);

        // Set up the Precision Clock Generator (PCG) to divide our bit clock by 4 or 8
        *pREG_PCG0_SYNC1 =     BITM_PCG_SYNC1_FSA   |    // Enable external FS synchronization
                           BITM_PCG_SYNC1_CLKA   |     // Enable external FS synchronization
                           0;

        *pREG_PCG0_CTLA1 =     BITM_PCG_CTLA1_CLKSRC |  // Set BCLK source as PCG_EXT_I
                           BITM_PCG_CTLA1_FSSRC  |   // Set FS source as PCG_EXT_I
                           clock_divider |
                           0;

        *pREG_PCG0_CTLA0 =     BITM_PCG_CTLA0_CLKEN |     // Enable CLK output
                           BITM_PCG_CTLA0_FSEN     |   // Enable FS output
                           64 * clock_divider   |     // FS Div = 256fs or 512fs
                           0;

        // Route PCG clock and framesync to SPORT 2
        SRU(PCG0_CLKA_O, SPT2_ACLK_I);     // route PCG 64fs BCLK signal to SPORT2A BCLK
        SRU(PCG0_CLKA_O, SPT2_BCLK_I);     // route PCG 64fs BCLK signal to SPORT2B BCLK
        SRU(PCG0_FSA_O, SPT2_AFS_I);     // route PCG FS signal to SPORT2A FS
        SRU(PCG0_FSA_O, SPT2_BFS_I);     // route PCG FS signal to SPORT2B FS

        // Connect SPDIF RX to SRC 0 "IP" side
        SRU(SPDIF0_RX_CLK_O, SRC0_CLK_IP_I);     // route SPDIF RX BCLK to SRC IP BCLK
        SRU(SPDIF0_RX_FS_O,  SRC0_FS_IP_I);     // route SPDIF RX FS to SRC IP FS
        SRU(SPDIF0_RX_DAT_O, SRC0_DAT_IP_I);     // route SPDIF RX Data to SRC IP Data

        // Connect SPORT 2 to SRC 0 "OP" side
        SRU(PCG0_CLKA_O,     SRC0_CLK_OP_I);     // route PCG 64fs BCLK signal to SRC OP BCLK
        SRU(PCG0_FSA_O,     SRC0_FS_OP_I);         // route PCG FS signal to SRC OP FS
        SRU(SRC0_DAT_OP_O,  SPT2_BD0_I);      // route SRC0 OP Data output to SPORT 2B data
    }
    else {

        /*
         * Otherwise, if we're using 64fs (stereo) I2S signals from the ADAU1761, we
         * can use them directly to drive SPDIF
         */

        // Route the ADAU1761 clock domain signals to SPORT2
        SRU(DAI0_PB03_O, SPT2_ACLK_I);     // route 64fs BCLK signal to SPORT2A BCLK
        SRU(DAI0_PB03_O, SPT2_BCLK_I);     // route 64fs BCLK signal to SPORT2B BCLK
        SRU(DAI0_PB04_O, SPT2_AFS_I);     // route ADAU1761 FS signal to SPORT2A FS
        SRU(DAI0_PB04_O, SPT2_BFS_I);     // route ADAU1761 FS signal to SPORT2B FS

        // Connect SPDIF RX to SRC 0 "IP" side
        SRU(SPDIF0_RX_CLK_O, SRC0_CLK_IP_I);     // route SPDIF RX BCLK to SRC IP BCLK
        SRU(SPDIF0_RX_FS_O,  SRC0_FS_IP_I);     // route SPDIF RX FS to SRC IP FS
        SRU(SPDIF0_RX_DAT_O, SRC0_DAT_IP_I);     // route SPDIF RX Data to SRC IP Data

        // Connect SPORT 2 to SRC 0 "OP" side
        SRU(DAI0_PB03_O,     SRC0_CLK_OP_I);     // route 64fs BCLK signal to SRC OP BCLK
        SRU(DAI0_PB04_O,     SRC0_FS_OP_I);     // route ADAU1761 FS signal to SRC OP FS
        SRU(SRC0_DAT_OP_O,  SPT2_BD0_I);      // route SRC0 OP Data output to SPORT 2B data
    }

    // Configure and enable SRC 0/1
    *pREG_ASRC0_CTL01      = BITM_ASRC_CTL01_EN0 |    // Enable SRC0
                             (0x1 << BITP_ASRC_CTL01_SMODEIN0) | // Input mode = I2S
                             (0x1 << BITP_ASRC_CTL01_SMODEOUT0) | // Output mode = I2S
                             0;

    // Configure and enable SPDIF RX
    *pREG_SPDIF0_RX_CTL =     BITM_SPDIF_RX_CTL_EN |        // Enable the SPDIF RX
                          BITM_SPDIF_RX_CTL_FASTLOCK |      // Enable SPDIF Fastlock (see HRM 32-15)
                          BITM_SPDIF_RX_CTL_RSTRTAUDIO |
                          0;

    // If we're using a newer (v1.3 or later) SHARC Audio Module board, enable SPDIF transmitter
    #if (SHARC_SAM_REV_1_3_OR_LATER)

    // We need a 256fs clock for the SPDIF transmitter and on SHARC Audio Module rev 1.3 and later,
    if (clock_divider == 4) {
        /*
         * If we are using an 8 channel framework, the bit clock from the ADAU1761 is running
         * at 256fs.  The benefit of using this I2S BCLK over the fixed MCLK is that if we
         * switch sample rates on the ADAU1761 (e.g. 44.1KHz, then the BCLK signal will scale
         * accordingly.)
         */
        SRU(DAI0_PB03_O, SPDIF0_TX_HFCLK_I);         // route ADAU1761 I2S BCLK to HFCLK
    }
    else if (clock_divider == 1) {
        /*
         * If we're using a 2 channel framework and the bit clock from the ADAU1761 is running
         * at 64fs, then we need to use the MCLK signal which is now routed to DAI0 pin 6.
         * The issue is that if the sample rate is not set to 48KHz, then this MCLK is no
         * longer a 256fs multiple.
         */
        SRU(DAI0_PB06_O, SPDIF0_TX_HFCLK_I);         // route DAI0 pin 6 (MCLK) to SPDIF TX HFCLK
    }

    // Determine if we need to generate a new i2S signal for SPDIF
    if (clock_divider == 4 || clock_divider == 8) {

        // Drive SPDIF TX with our 64fs clock from the PCG
        SRU(PCG0_CLKA_O, SPDIF0_TX_CLK_I);         // route 64fs BCLK signal to SPDIF TX BCLK
        SRU(PCG0_FSA_O,  SPDIF0_TX_FS_I);         // route ADAU1761 FS signal to SPDIF TX FS
        SRU(SPT2_AD0_O,  SPDIF0_TX_DAT_I);      // SPT2A AD0 output to SPDIF TX data pin
    }
    else {
        // Connect SPDIF TX to SPORT2A
        SRU(DAI0_PB03_O, SPDIF0_TX_CLK_I);         // route ADAU1761 BCLK signal to SPDIF TX BCLK
        SRU(DAI0_PB04_O, SPDIF0_TX_FS_I);         // route ADAU1761 FS signal to SPDIF TX FS
        SRU(SPT2_AD0_O,  SPDIF0_TX_DAT_I);      // SPT2A AD0 output to SPDIF TX data pin
    }

    // Configure SPDIF Transmitter in auto mode
    *pREG_SPDIF0_TX_CTL = (0x1 << BITP_SPDIF_TX_CTL_SMODEIN) |      // I2S Mode
                          BITM_SPDIF_TX_CTL_AUTO |         // Standalone mode
                          0;

    // Enable SPDIF transmitter
    *pREG_SPDIF0_TX_CTL |= BITM_SPDIF_TX_CTL_EN |         // Enable SPDIF TX
                           0;

    #endif

    return true;
}

/**
 * @brief      Configuration: ADAU1761 is I2S master (generates system audio clocks)
 *
 * This is the basic, stand-alone audio configuration for the SHARC Audio Module board where
 * ADAU1761 I2S signals are routed to SPORT0 of the SHARC.  In this configuration,
 * the ADAU1761 is the master as it generates the I2S clock / fs.
 *
 */
void sru_config_sharc_sam_adau1761_master(void) {

    // Initialize standard SRU/DAI settings on SHARC Audio Module board
    sru_init_sharc_sam();

    SRU(HIGH, DAI0_PBEN01_I);        // ADAU1761 DAC data is an output
    SRU(LOW,  DAI0_PBEN02_I);        // ADAU1761 ADC data is an input
    SRU(LOW,  DAI0_PBEN03_I);        // ADAU1761 CLK is an input
    SRU(LOW,  DAI0_PBEN04_I);        // ADAU1761 FS is an input

    /* configuration for the ADAU1761 and monitoring the signals */
    SRU(DAI0_PB03_O, SPT0_ACLK_I);     // route ADAU1761 BCLK output to SPORT0A clock input
    SRU(DAI0_PB03_O, SPT0_BCLK_I);     // route also to SPORT0B clock input

    SRU(DAI0_PB04_O, SPT0_AFS_I);     // route ADAU1761 FS output to SPORT0A frame sync
    SRU(DAI0_PB04_O, SPT0_BFS_I);     // route ADAU1761 FS output to SPORT0B frame sync

    SRU(DAI0_PB02_O, SPT0_BD0_I);    // ADUC1761 ADC pin to SPT0 BD0 input
    SRU(SPT0_AD0_O,  DAI0_PB01_I);    // SPT0A AD0 output to 1761 DAC pin

    // Enable this code to route the I2S signals to DAI pins on the expansion header for debugging
    #if 1
    // And set unused DAI0 pins as outputs for debug
    SRU(HIGH,  DAI0_PBEN13_I);            // ADAU1761 DAC data (debug) is an output
    SRU(HIGH,  DAI0_PBEN14_I);            // ADAU1761 ADC data (debug) is an output
    SRU(HIGH,  DAI0_PBEN15_I);            // ADAU1761 CLK (debug) is an output
    SRU(HIGH,  DAI0_PBEN16_I);            // ADAU1761 FS (debug) is an output

    // Route ADAU1761 signals to higher DAI pins on expansion header for debug purposes
    SRU(DAI0_PB01_O, DAI0_PB13_I);
    SRU(DAI0_PB02_O, DAI0_PB14_I);
    SRU(DAI0_PB03_O, DAI0_PB15_I);
    SRU(DAI0_PB04_O, DAI0_PB16_I);
    #endif
}

/**
 * @brief      Configuration: ADAU1761 slaves from A2B I2S clocks
 *
 * The SHARC Audio Module board operates as an A2B slave node.  In this configuration, the I2S clock/fs
 * originate from the AD2425W (A2B) which are sent to the ADAU1761 and the SHARC.
 * Thus A2B, SHARC and ADAU1761 are all running off of the same set of I2S clock/fs.
 */
void sru_config_sharc_sam_a2b_slave(void) {

    // Initialize standard SRU/DAI settings on SHARC Audio Module board
    sru_init_sharc_sam();

    // Set up pins for AD2425W (A2B)
    SRU(LOW,  DAI0_PBEN07_I);        // pin for A2B BCLK is an input (from A2B bus)
    SRU(LOW,  DAI0_PBEN08_I);        // pin for A2B FS is an input (from A2B bus)
    SRU(LOW,  DAI0_PBEN09_I);        // pin for DTX0 is always an input (from A2B bus)
    SRU(LOW,  DAI0_PBEN10_I);        // pin for DTX1 is always an input (from A2B bus)
    SRU(HIGH, DAI0_PBEN11_I);        // pin for DRX0 is always an output (to A2B bus)
    SRU(HIGH, DAI0_PBEN12_I);        // pin for DRX1 is always an output (to A2B bus)

    // Set up pins for ADAU1761
    SRU(HIGH,  DAI0_PBEN01_I);        // pin for ADAU1761 DAC data is an output
    SRU(LOW,   DAI0_PBEN02_I);        // pin for ADAU1761 ADC data is an input
    SRU(HIGH,  DAI0_PBEN03_I);        // pin for ADAU1761 CLK is an output
    SRU(HIGH,  DAI0_PBEN04_I);        // pin for ADAU1761 FS is an output

    // Route A2B signals to SPORT1
    SRU(DAI0_PB07_O, SPT1_ACLK_I);     // route A2B BCLK to SPORT1A clock input
    SRU(DAI0_PB07_O, SPT1_BCLK_I);     // route A2B BCLK to SPORT1B clock input

    SRU(DAI0_PB08_O, SPT1_AFS_I);     // route A2B FS to SPORT1A FS input
    SRU(DAI0_PB08_O, SPT1_BFS_I);     // route A2B FS to SPORT1B FS input

    SRU(DAI0_PB09_O, SPT1_BD0_I);    // route A2B DTX0 to SPORT 1B data RX primary
    SRU(DAI0_PB10_O, SPT1_BD1_I);    // route A2B DTX1 to SPORT 1B data RX secondary

    SRU(SPT1_AD0_O, DAI0_PB11_I);    // route SPORT 1A data TX primary to A2B DRX0
    SRU(SPT1_AD1_O, DAI0_PB12_I);    // route SPORT 1A data TX secondary to A2B DRX0

    // Route ADAU1761 signals to SPORT0
    SRU(DAI0_PB02_O, SPT0_BD0_I);    // route ADUC1761 ADC pin to SPT0 BD0 input
    SRU(SPT0_AD0_O, DAI0_PB01_I);    // route SPT0A AD0 output to 1761 DAC pin

    SRU(DAI0_PB03_O, SPT0_ACLK_I);     // route ADAU1761 BCLK pin to SPORT0A clock input
    SRU(DAI0_PB03_O, SPT0_BCLK_I);     // route ADAU1761 BCLK pin to SPORT0B clock input

    SRU(DAI0_PB04_O, SPT0_AFS_I);     // route ADAU1761 FS pin to SPORT0A frame sync
    SRU(DAI0_PB04_O, SPT0_BFS_I);     // route ADAU1761 FS pin to SPORT0B frame sync

    // Route A2B clock / fs to the ADAU1761
    SRU(DAI0_PB07_O, DAI0_PB03_I);    // route A2B BCLK pin to ADAU1761 BCLK pin
    SRU(DAI0_PB08_O, DAI0_PB04_I);    // route A2B FS pin to ADAU1761 FS pin

    // Enable this code to route the I2S signals to DAI pins on the expansion header for debugging
    #if 1
    // And set unused DAI0 pins as outputs for debug
    SRU(HIGH,  DAI0_PBEN13_I);            // ADAU1761 DAC data (debug) is an output
    SRU(HIGH,  DAI0_PBEN14_I);            // ADAU1761 ADC data (debug) is an output
    SRU(HIGH,  DAI0_PBEN15_I);            // ADAU1761 CLK (debug) is an output
    SRU(HIGH,  DAI0_PBEN16_I);            // ADAU1761 FS (debug) is an output

    // Route ADAU1761 signals to higher DAI pins on expansion header for debug purposes
    SRU(DAI0_PB01_O, DAI0_PB13_I);
    SRU(DAI0_PB02_O, DAI0_PB14_I);
    SRU(DAI0_PB03_O, DAI0_PB15_I);
    SRU(DAI0_PB04_O, DAI0_PB16_I);
    #endif
}

/**
 * @brief      Configuration: ADAU1761 drives A2B I2S clocks (and generates system audio clocks)
 *
 * The SHARC Audio Module board operates as an A2B master node.  In this configuration, the I2S clock/s
 * originate from the ADAU1761 which are then sent to the AD2425W (A2B) and the SHARC.
 * This also drives the clock / FS for the A2B bus.  Thus, A2B, SHARC and ADAU1761 are
 * all running off of the same set of I2S clock/fs.
 *
 */
void sru_config_sharc_sam_a2b_master(void) {

    // Initialize standard SRU/DAI settings on SHARC Audio Module board
    sru_init_sharc_sam();

    // Set up pins for AD2425W (A2B)
    SRU(HIGH,  DAI0_PBEN07_I);        // pin for A2B BCLK is an output (to A2B bus)
    SRU(HIGH,  DAI0_PBEN08_I);        // pin for A2B FS is an output (to A2B bus)
    SRU(LOW,   DAI0_PBEN09_I);        // DTX0 is always an input (from A2B bus)
    SRU(LOW,   DAI0_PBEN10_I);        // DTX1 is always an input (from A2B bus)
    SRU(HIGH,  DAI0_PBEN11_I);        // DRX0 is always an output (to A2B bus)
    SRU(HIGH,  DAI0_PBEN12_I);        // DRX1 is always an output (to A2B bus)

    // Set up pins for ADAU1761
    SRU(HIGH,  DAI0_PBEN01_I);        // pin for ADAU1761 DAC data is an output
    SRU(LOW,   DAI0_PBEN02_I);        // pin for ADAU1761 ADC data is an input
    SRU(LOW,   DAI0_PBEN03_I);        // pin for ADAU1761 CLK is an input
    SRU(LOW,   DAI0_PBEN04_I);        // pin for ADAU1761 FS is an input

    // Connect A2B signals to SPORT1
    SRU(DAI0_PB07_O, SPT1_ACLK_I);     // route A2B BCLK to SPORT1A clock input
    SRU(DAI0_PB07_O, SPT1_BCLK_I);     // route A2B BCLK to SPORT1B clock input

    SRU(DAI0_PB08_O, SPT1_AFS_I);     // route A2B FS to SPORT1A FS input
    SRU(DAI0_PB08_O, SPT1_BFS_I);     // route A2B FS to SPORT1B FS input

    SRU(DAI0_PB09_O, SPT1_BD0_I);    // route A2B DTX0 to SPORT 1B data RX primary
    SRU(DAI0_PB10_O, SPT1_BD1_I);    // route A2B DTX1 to SPORT 1B data RX secondary

    SRU(SPT1_AD0_O, DAI0_PB11_I);    // route SPORT 1A data TX primary to A2B DRX0
    SRU(SPT1_AD1_O, DAI0_PB12_I);    // route SPORT 1A data TX secondary to A2B DRX0

    // Connect ADAU1761 signals to SPORT0
    SRU(DAI0_PB02_O, SPT0_BD0_I);    // route ADUC1761 ADC pin to SPT0 BD0 input
    SRU(SPT0_AD0_O, DAI0_PB01_I);    // route SPT0A AD0 output to 1761 DAC pin

    SRU(DAI0_PB03_O, SPT0_ACLK_I);     // route ADAU1761 BCLK pin to SPORT0A clock input
    SRU(DAI0_PB03_O, SPT0_BCLK_I);     // route ADAU1761 BCLK pin to SPORT0B clock input

    SRU(DAI0_PB04_O, SPT0_AFS_I);     // route ADAU1761 FS pin to SPORT0A frame sync
    SRU(DAI0_PB04_O, SPT0_BFS_I);     // route ADAU1761 FS pin to SPORT0B frame sync

    // Route ADAU1761 clock and framesync to the AD2425W (A2B)
    SRU(DAI0_PB03_O, DAI0_PB07_I);    // route ADAU1761 BCLK to A2B BCLK
    SRU(DAI0_PB04_O, DAI0_PB08_I);    // route ADAU1761 FS to A2B FS
}

/**
 * @brief      Configuration: I2S data passed from A2B directly to ADAU1761 (no SC589 involed)
 *
 * The SHARC Audio Module board operates as an A2B slave node.
 * This configuration simply routes the A2B I2S signals directly to the ADAU1761
 * without routing them through the SHARC.  This routine is helpful in ensuring that A2B
 * is being configured properly before attempting to insert the SHARC / audio processing
 * into the mix,
 */
void sru_config_sharc_sam_a2b_passthrough_slave(void){

    // Initialize standard SRU/DAI settings on SHARC Audio Module board
    sru_init_sharc_sam();

    // Set up DAI pins as inputs and outputs
    SRU(HIGH,  DAI0_PBEN01_I);        // ADAU1761 DAC data is an output
    SRU(LOW,   DAI0_PBEN02_I);        // ADAU1761 ADC data is an input
    SRU(HIGH,  DAI0_PBEN03_I);        // ADAU1761 CLK is an output
    SRU(HIGH,  DAI0_PBEN04_I);        // ADAU1761 FS is an output

    SRU(LOW,   DAI0_PBEN07_I);        // A2B CLK (from A2B) is an input
    SRU(LOW,   DAI0_PBEN08_I);        // A2B FS (from A2B) is an input
    SRU(LOW,   DAI0_PBEN09_I);        // A2B DTX0 data (from A2B bus) is an input
    SRU(HIGH,  DAI0_PBEN11_I);        // A2B DRX0 data (to A2B bus) is an output

    // Route A2B to ADAU1761 directly
    SRU(DAI0_PB09_O, DAI0_PB01_I);    // A2B DTX0 -> ADAU1761 DAC
    SRU(DAI0_PB02_O, DAI0_PB11_I);    // ADAU1761 ADC -> A2B DRX0

    SRU(DAI0_PB07_O, DAI0_PB03_I);    // A2B CLK -> ADAU1761 CLK
    SRU(DAI0_PB08_O, DAI0_PB04_I);    // A2B FS -> ADAU1761 FS
}

/**
 * @brief      Configuration: I2S data passed from ADAU1761 to A2B directly (no SC589 involed)
 *
 * The SHARC Audio Module board operates as an A2B master node.
 * This configuration simply routes the ADAU1761 signals directly to the AD2425W
 * without routing them through the SHARC.  This routine is helpful in ensuring that A2B
 * is being configured properly before attempting to insert the SHARC / audio processing
 * into the mix,
 */
void sru_config_sharc_sam_a2b_passthrough_master(void){

    // Initialize standard SRU/DAI settings on SHARC Audio Module board
    sru_init_sharc_sam();

    // Set up DAI pins as inputs and outputs
    SRU(HIGH, DAI0_PBEN01_I);        // ADAU1761 DAC data is an output
    SRU(LOW,  DAI0_PBEN02_I);        // ADAU1761 ADC data is an input
    SRU(LOW,  DAI0_PBEN03_I);        // ADAU1761 CLK is an input
    SRU(LOW,  DAI0_PBEN04_I);        // ADAU1761 FS is an input

    SRU(HIGH,  DAI0_PBEN07_I);        // A2B CLK (to A2B) is an output
    SRU(HIGH,  DAI0_PBEN08_I);        // A2B A2B FS (to A2B) is an output
    SRU(LOW,   DAI0_PBEN09_I);        // A2B DTX0 data (from A2B bus) is an input
    SRU(HIGH,  DAI0_PBEN11_I);        // A2B DRX0 data (to A2B bus) is an output

    // Route ADAU1761 to A2B directly
    SRU(DAI0_PB09_O, DAI0_PB01_I);    // A2B DTX0 -> ADAU1761 DAC
    SRU(DAI0_PB02_O, DAI0_PB11_I);    // ADAU1761 ADC -> A2B DRX0

    SRU(DAI0_PB03_O, DAI0_PB07_I);    // ADAU1761 CLK -> A2B CLK
    SRU(DAI0_PB04_O, DAI0_PB08_I);    // ADAU1761 FS -> A2B FS
}

/**
 * @brief      Configuration: Automotive board - ADAU1452 is master
 *
 * This configuration supports the automotive expander board which connects to DAI1
 * rather than DAI0.  The TDM serial interface from the ADAU1452 is routed to
 * SPORT 4 (A=output / B=input).
 *
 * Note that because we're working with DAI1, we use the SRU2 macro to make these
 * connections rather than the SRU marco.
 */
void sru_config_sharc_sam_adau1452_master(void) {

    // Initialize standard SRU/DAI settings on SHARC Audio Module board
    sru_init_sharc_sam();

    // Set up DAI pins as inputs and outputs
    SRU2(LOW,  DAI1_PBEN11_I);        // ADAU1452 BCLK is an input
    SRU2(LOW,  DAI1_PBEN12_I);        // ADAU1452 FS is an input
    SRU2(HIGH, DAI1_PBEN13_I);        // ADAU1452 SDATA In (to ADAU1452) is an output
    SRU2(LOW,  DAI1_PBEN14_I);        // ADAU1452 SDATA Out (from ADAU1452) is an input

    // Route BCLK and FS to SPORT 4 (which connects to DAI1)
    SRU2(DAI1_PB11_O, SPT4_ACLK_I);     // route ADAU1452 BCLK pin to SPORT4A clock input
    SRU2(DAI1_PB11_O, SPT4_BCLK_I);     // route ADAU1452 BCLK pin to SPORT4B clock input

    SRU2(DAI1_PB12_O, SPT4_AFS_I);         // route ADAU1452 FS pin to SPORT4A frame sync
    SRU2(DAI1_PB12_O, SPT4_BFS_I);         // route ADAU1452 FS pin to SPORT4B frame sync

    // Route data signals
    SRU2(DAI1_PB14_O, SPT4_BD0_I);        // route ADAU1452 SDATA Out pin to SPORT4B-D0 input
    SRU2(SPT4_AD0_O,  DAI1_PB13_I);        // route SPORT4A-D0 output to ADAU1452 SDATA In
}
