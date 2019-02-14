/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 */

// Register map for the ADAU1761

#ifndef _REGISTERS_ADAU1452_H_
#define _REGISTERS_ADAU1452_H_

#define    ADAU1452_REG_PLL_CTRL0                      (0xF000)   /* PLL feedback divider */
#define    ADAU1452_REG_PLL_CTRL1                      (0xF001)   /* PLL prescale divider */
#define    ADAU1452_REG_PLL_CLK_SRC                    (0xF002)   /* PLL clock source */
#define    ADAU1452_REG_PLL_ENABLE                     (0xF003)   /* PLL enable */
#define    ADAU1452_REG_PLL_LOCK                       (0xF004)   /* PLL lock */
#define    ADAU1452_REG_MCLK_OUT                       (0xF005)   /* CLKOUT control */
#define    ADAU1452_REG_PLL_WATCHDOG                   (0xF006)   /* Analog PLL watchdog control */
#define    ADAU1452_REG_CLK_GEN1_M                     (0xF020)   /* Denominator (M) for Clock Generator 1 */
#define    ADAU1452_REG_CLK_GEN1_N                     (0xF021)   /* Numerator (N) for Clock Generator 1 */
#define    ADAU1452_REG_CLK_GEN2_M                     (0xF022)   /* Denominator (M) for Clock Generator 2 */
#define    ADAU1452_REG_CLK_GEN2_N                     (0xF023)   /* Numerator (N) for Clock Generator 2 */
#define    ADAU1452_REG_CLK_GEN3_M                     (0xF024)   /* Denominator (M) for Clock Generator 3 */
#define    ADAU1452_REG_CLK_GEN3_N                     (0xF025)   /* Numerator for (N) Clock Generator 3 */
#define    ADAU1452_REG_CLK_GEN3_SRC                   (0xF026)   /* Input Reference for Clock Generator 3 */
#define    ADAU1452_REG_CLK_GEN3_LOCK                  (0xF027)   /* Lock Bit for Clock Generator 3 input reference */
#define    ADAU1452_REG_POWER_ENABLE0                  (0xF050)   /* Power Enable 0 */
#define    ADAU1452_REG_POWER_ENABLE1                  (0xF051)   /* Power Enable 1 */
#define    ADAU1452_REG_ASRC_INPUT0                    (0xF100)   /* ASRC input selector (ASRC 0, Channel 0 and Channel 1) */
#define    ADAU1452_REG_ASRC_INPUT1                    (0xF101)   /* ASRC input selector (ASRC 1, Channel 2 and Channel 3) */
#define    ADAU1452_REG_ASRC_INPUT2                    (0xF102)   /* ASRC input selector (ASRC 2, Channel 4 and Channel 5) */
#define    ADAU1452_REG_ASRC_INPUT3                    (0xF103)   /* ASRC input selector (ASRC 3, Channel 6 and Channel 7) */
#define    ADAU1452_REG_ASRC_INPUT4                    (0xF104)   /* ASRC input selector (ASRC 4, Channel 8 and Channel 9) */
#define    ADAU1452_REG_ASRC_INPUT5                    (0xF105)   /* ASRC input selector (ASRC 5, Channel 10 and Channel 11) */
#define    ADAU1452_REG_ASRC_INPUT6                    (0xF106)   /* ASRC input selector (ASRC 6, Channel 12 and Channel 13) */
#define    ADAU1452_REG_ASRC_INPUT7                    (0xF107)   /* ASRC input selector (ASRC 7, Channel 14 and Channel 15) */
#define    ADAU1452_REG_ASRC_OUT_RATE0                 (0xF140)   /* ASRC output rate (ASRC 0, Channel 0 and Channel 1) */
#define    ADAU1452_REG_ASRC_OUT_RATE1                 (0xF141)   /* ASRC output rate (ASRC 1, Channel 2 and Channel 3) */
#define    ADAU1452_REG_ASRC_OUT_RATE2                 (0xF142)   /* ASRC output rate (ASRC 2, Channel 4 and Channel 5) */
#define    ADAU1452_REG_ASRC_OUT_RATE3                 (0xF143)   /* ASRC output rate (ASRC 3, Channel 6 and Channel 7) */
#define    ADAU1452_REG_ASRC_OUT_RATE4                 (0xF144)   /* ASRC output rate (ASRC 4, Channel 8 and Channel 9) */
#define    ADAU1452_REG_ASRC_OUT_RATE5                 (0xF145)   /* ASRC output rate (ASRC 5, Channel 10 and Channel 11) */
#define    ADAU1452_REG_ASRC_OUT_RATE6                 (0xF146)   /* ASRC output rate (ASRC 6, Channel 12 and Channel 13) */
#define    ADAU1452_REG_ASRC_OUT_RATE7                 (0xF147)   /* ASRC output rate (ASRC 7, Channel 14 and Channel 15) */
#define    ADAU1452_REG_SOUT_SOURCE0                   (0xF180)   /* Source of data for serial output ports (Channel 0 and Channel 1) */
#define    ADAU1452_REG_SOUT_SOURCE1                   (0xF181)   /* Source of data for serial output ports (Channel 2 and Channel 3) */
#define    ADAU1452_REG_SOUT_SOURCE2                   (0xF182)   /* Source of data for serial output ports (Channel 4 and Channel 5) */
#define    ADAU1452_REG_SOUT_SOURCE3                   (0xF183)   /* Source of data for serial output ports (Channel 6 and Channel 7) */
#define    ADAU1452_REG_SOUT_SOURCE4                   (0xF184)   /* Source of data for serial output ports (Channel 8 and Channel 9) */
#define    ADAU1452_REG_SOUT_SOURCE5                   (0xF185)   /* Source of data for serial output ports (Channel 10 and Channel 11) */
#define    ADAU1452_REG_SOUT_SOURCE6                   (0xF186)   /* Source of data for serial output ports (Channel 12 and Channel 13) */
#define    ADAU1452_REG_SOUT_SOURCE7                   (0xF187)   /* Source of data for serial output ports (Channel 14 and Channel 15) */
#define    ADAU1452_REG_SOUT_SOURCE8                   (0xF188)   /* Source of data for serial output ports (Channel 16 and Channel 17) */
#define    ADAU1452_REG_SOUT_SOURCE9                   (0xF189)   /* Source of data for serial output ports (Channel 18 and Channel 19) */
#define    ADAU1452_REG_SOUT_SOURCE10                  (0xF18A)   /* Source of data for serial output ports (Channel 20 and Channel 21) */
#define    ADAU1452_REG_SOUT_SOURCE11                  (0xF18B)   /* Source of data for serial output ports (Channel 22 and Channel 23) */
#define    ADAU1452_REG_SOUT_SOURCE12                  (0xF18C)   /* Source of data for serial output ports (Channel 24 and Channel 25) */
#define    ADAU1452_REG_SOUT_SOURCE13                  (0xF18D)   /* Source of data for serial output ports (Channel 26 and Channel 27) */
#define    ADAU1452_REG_SOUT_SOURCE14                  (0xF18E)   /* Source of data for serial output ports (Channel 28 and Channel 29) */
#define    ADAU1452_REG_SOUT_SOURCE15                  (0xF18F)   /* Source of data for serial output ports (Channel 30 and Channel 31) */
#define    ADAU1452_REG_SOUT_SOURCE16                  (0xF190)   /* Source of data for serial output ports (Channel 32 and Channel 33) */
#define    ADAU1452_REG_SOUT_SOURCE17                  (0xF191)   /* Source of data for serial output ports (Channel 34 and Channel 35) */
#define    ADAU1452_REG_SOUT_SOURCE18                  (0xF192)   /* Source of data for serial output ports (Channel 36 and Channel 37) */
#define    ADAU1452_REG_SOUT_SOURCE19                  (0xF193)   /* Source of data for serial output ports (Channel 38 and Channel 39) */
#define    ADAU1452_REG_SOUT_SOURCE20                  (0xF194)   /* Source of data for serial output ports (Channel 40 and Channel 41) */
#define    ADAU1452_REG_SOUT_SOURCE21                  (0xF195)   /* Source of data for serial output ports (Channel 42 and Channel 43) */
#define    ADAU1452_REG_SOUT_SOURCE22                  (0xF196)   /* Source of data for serial output ports (Channel 44 and Channel 45) */
#define    ADAU1452_REG_SOUT_SOURCE23                  (0xF197)   /* Source of data for serial output ports (Channel 46 and Channel 47) */
#define    ADAU1452_REG_SPDIFTX_INPUT                  (0xF1C0)   /* S/PDIF transmitter data selector */
#define    ADAU1452_REG_SERIAL_BYTE_0_0                (0xF200)   /* Serial Port Control 0 (SDATA_IN0) */
#define    ADAU1452_REG_SERIAL_BYTE_0_1                (0xF201)   /* Serial Port Control 1 (SDATA_IN0) */
#define    ADAU1452_REG_SERIAL_BYTE_1_0                (0xF204)   /* Serial Port Control 0 (SDATA_IN1) */
#define    ADAU1452_REG_SERIAL_BYTE_1_1                (0xF205)   /* Serial Port Control 1 (SDATA_IN1) */
#define    ADAU1452_REG_SERIAL_BYTE_2_0                (0xF208)   /* Serial Port Control 0 (SDATA_IN2) */
#define    ADAU1452_REG_SERIAL_BYTE_2_1                (0xF209)   /* Serial Port Control 1 (SDATA_IN2) */
#define    ADAU1452_REG_SERIAL_BYTE_3_0                (0xF20C)   /* Serial Port Control 0 (SDATA_IN3) */
#define    ADAU1452_REG_SERIAL_BYTE_3_1                (0xF20D)   /* Serial Port Control 1 (SDATA_IN3) */
#define    ADAU1452_REG_SERIAL_BYTE_4_0                (0xF210)   /* Serial Port Control 0 (SDATA_OUT0) */
#define    ADAU1452_REG_SERIAL_BYTE_4_1                (0xF211)   /* Serial Port Control 1 (SDATA_OUT0) */
#define    ADAU1452_REG_SERIAL_BYTE_5_0                (0xF214)   /* Serial Port Control 0 (SDATA_OUT1) */
#define    ADAU1452_REG_SERIAL_BYTE_5_1                (0xF215)   /* Serial Port Control 1 (SDATA_OUT1) */
#define    ADAU1452_REG_SERIAL_BYTE_6_0                (0xF218)   /* Serial Port Control 0 (SDATA_OUT2) */
#define    ADAU1452_REG_SERIAL_BYTE_6_1                (0xF219)   /* Serial Port Control 1 (SDATA_OUT2) */
#define    ADAU1452_REG_SERIAL_BYTE_7_0                (0xF21C)   /* Serial Port Control 0 (SDATA_OUT3) */
#define    ADAU1452_REG_SERIAL_BYTE_7_1                (0xF21D)   /* Serial Port Control 1 (SDATA_OUT3) */
#define    ADAU1452_REG_FTDM_IN0                       (0xF300)   /* FTDM mapping for the serial inputs (Channel 32, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_IN1                       (0xF301)   /* FTDM mapping for the serial inputs (Channel 32, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_IN2                       (0xF302)   /* FTDM mapping for the serial inputs (Channel 32, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_IN3                       (0xF303)   /* FTDM mapping for the serial inputs (Channel 32, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_IN4                       (0xF304)   /* FTDM mapping for the serial inputs (Channel 33, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_IN5                       (0xF305)   /* FTDM mapping for the serial inputs (Channel 33, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_IN6                       (0xF306)   /* FTDM mapping for the serial inputs (Channel 33, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_IN7                       (0xF307)   /* FTDM mapping for the serial inputs (Channel 33, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_IN8                       (0xF308)   /* FTDM mapping for the serial inputs (Channel 34, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_IN9                       (0xF309)   /* FTDM mapping for the serial inputs (Channel 34, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_IN10                      (0xF30A)   /* FTDM mapping for the serial inputs (Channel 34, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_IN11                      (0xF30B)   /* FTDM mapping for the serial inputs (Channel 34, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_IN12                      (0xF30C)   /* FTDM mapping for the serial inputs (Channel 35, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_IN13                      (0xF30D)   /* FTDM mapping for the serial inputs (Channel 35, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_IN14                      (0xF30E)   /* FTDM mapping for the serial inputs (Channel 35, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_IN15                      (0xF30F)   /* FTDM mapping for the serial inputs (Channel 35, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_IN16                      (0xF310)   /* FTDM mapping for the serial inputs (Channel 36, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_IN17                      (0xF311)   /* FTDM mapping for the serial inputs (Channel 36, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_IN18                      (0xF312)   /* FTDM mapping for the serial inputs (Channel 36, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_IN19                      (0xF313)   /* FTDM mapping for the serial inputs (Channel 36, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_IN20                      (0xF314)   /* FTDM mapping for the serial inputs (Channel 37, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_IN21                      (0xF315)   /* FTDM mapping for the serial inputs (Channel 37, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_IN22                      (0xF316)   /* FTDM mapping for the serial inputs (Channel 37, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_IN23                      (0xF317)   /* FTDM mapping for the serial inputs (Channel 37, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_IN24                      (0xF318)   /* FTDM mapping for the serial inputs (Channel 38, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_IN25                      (0xF319)   /* FTDM mapping for the serial inputs (Channel 38, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_IN26                      (0xF31A)   /* FTDM mapping for the serial inputs (Channel 38, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_IN27                      (0xF31B)   /* FTDM mapping for the serial inputs (Channel 38, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_IN28                      (0xF31C)   /* FTDM mapping for the serial inputs (Channel 39, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_IN29                      (0xF31D)   /* FTDM mapping for the serial inputs (Channel 39, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_IN30                      (0xF31E)   /* FTDM mapping for the serial inputs (Channel 39, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_IN31                      (0xF31F)   /* FTDM mapping for the serial inputs (Channel 39, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_IN32                      (0xF320)   /* FTDM mapping for the serial inputs (Channel 40, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_IN33                      (0xF321)   /* FTDM mapping for the serial inputs (Channel 40, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_IN34                      (0xF322)   /* FTDM mapping for the serial inputs (Channel 40, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_IN35                      (0xF323)   /* FTDM mapping for the serial inputs (Channel 40, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_IN36                      (0xF324)   /* FTDM mapping for the serial inputs (Channel 41, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_IN37                      (0xF325)   /* FTDM mapping for the serial inputs (Channel 41, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_IN38                      (0xF326)   /* FTDM mapping for the serial inputs (Channel 41, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_IN39                      (0xF327)   /* FTDM mapping for the serial inputs (Channel 41, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_IN40                      (0xF328)   /* FTDM mapping for the serial inputs (Channel 42, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_IN41                      (0xF329)   /* FTDM mapping for the serial inputs (Channel 42, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_IN42                      (0xF32A)   /* FTDM mapping for the serial inputs (Channel 42, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_IN43                      (0xF32B)   /* FTDM mapping for the serial inputs (Channel 42, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_IN44                      (0xF32C)   /* FTDM mapping for the serial inputs (Channel 43, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_IN45                      (0xF32D)   /* FTDM mapping for the serial inputs (Channel 43, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_IN46                      (0xF32E)   /* FTDM mapping for the serial inputs (Channel 43, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_IN47                      (0xF32F)   /* FTDM mapping for the serial inputs (Channel 43, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_IN48                      (0xF330)   /* FTDM mapping for the serial inputs (Channel 44, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_IN49                      (0xF331)   /* FTDM mapping for the serial inputs (Channel 44, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_IN50                      (0xF332)   /* FTDM mapping for the serial inputs (Channel 44, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_IN51                      (0xF333)   /* FTDM mapping for the serial inputs (Channel 44, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_IN52                      (0xF334)   /* FTDM mapping for the serial inputs (Channel 45, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_IN53                      (0xF335)   /* FTDM mapping for the serial inputs (Channel 45, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_IN54                      (0xF336)   /* FTDM mapping for the serial inputs (Channel 45, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_IN55                      (0xF337)   /* FTDM mapping for the serial inputs (Channel 45, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_IN56                      (0xF338)   /* FTDM mapping for the serial inputs (Channel 46, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_IN57                      (0xF339)   /* FTDM mapping for the serial inputs ts (Channel 46, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_IN58                      (0xF33A)   /* FTDM mapping for the serial inputs (Channel 46, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_IN59                      (0xF33B)   /* FTDM mapping for the serial inputs (Channel 46, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_IN60                      (0xF33C)   /* FTDM mapping for the serial inputs (Channel 47, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_IN61                      (0xF33D)   /* FTDM mapping for the serial inputs (Channel 47, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_IN62                      (0xF33E)   /* FTDM mapping for the serial inputs (Channel 47, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_IN63                      (0xF33F)   /* FTDM mapping for the serial inputs (Channel 47, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_OUT0                      (0xF380)   /* FTDM mapping for the serial outputs (Port 2, Channel 0, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_OUT1                      (0xF381)   /* FTDM mapping for the serial outputs (Port 2, Channel 0, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_OUT2                      (0xF382)   /* FTDM mapping for the serial outputs (Port 2, Channel 0, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_OUT3                      (0xF383)   /* FTDM mapping for the serial outputs (Port 2, Channel 0, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_OUT4                      (0xF384)   /* FTDM mapping for the serial outputs (Port 2, Channel 1, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_OUT5                      (0xF385)   /* FTDM mapping for the serial outputs (Port 2, Channel 1, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_OUT6                      (0xF386)   /* FTDM mapping for the serial outputs (Port 2, Channel 1, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_OUT7                      (0xF387)   /* FTDM mapping for the serial outputs (Port 2, Channel 1, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_OUT8                      (0xF388)   /* FTDM mapping for the serial outputs (Port 2, Channel 2, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_OUT9                      (0xF389)   /* FTDM mapping for the serial outputs (Port 2, Channel 2, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_OUT10                     (0xF38A)   /* FTDM mapping for the serial outputs (Port 2, Channel 2, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_OUT11                     (0xF38B)   /* FTDM mapping for the serial outputs (Port 2, Channel 2, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_OUT12                     (0xF38C)   /* FTDM mapping for the serial outputs (Port 2, Channel 3, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_OUT13                     (0xF38D)   /* FTDM mapping for the serial outputs (Port 2, Channel 3, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_OUT14                     (0xF38E)   /* FTDM mapping for the serial outputs (Port 2, Channel 3, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_OUT15                     (0xF38F)   /* FTDM mapping for the serial outputs (Port 2, Channel 3, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_OUT16                     (0xF390)   /* FTDM mapping for the serial outputs (Port 2, Channel 4, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_OUT17                     (0xF391)   /* FTDM mapping for the serial outputs (Port 2, Channel 4, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_OUT18                     (0xF392)   /* FTDM mapping for the serial outputs (Port 2, Channel 4, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_OUT19                     (0xF393)   /* FTDM mapping for the serial outputs (Port 2, Channel 4, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_OUT20                     (0xF394)   /* FTDM mapping for the serial outputs (Port 2, Channel 5, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_OUT21                     (0xF395)   /* FTDM mapping for the serial outputs (Port 2, Channel 5, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_OUT22                     (0xF396)   /* FTDM mapping for the serial outputs (Port 2, Channel 5, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_OUT23                     (0xF397)   /* FTDM mapping for the serial outputs (Port 2, Channel 5, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_OUT24                     (0xF398)   /* FTDM mapping for the serial outputs (Port 2, Channel 6, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_OUT25                     (0xF399)   /* FTDM mapping for the serial outputs (Port 2, Channel 6, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_OUT26                     (0xF39A)   /* FTDM mapping for the serial outputs (Port 2, Channel 6, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_OUT27                     (0xF39B)   /* FTDM mapping for the serial outputs (Port 2, Channel 6, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_OUT28                     (0xF39C)   /* FTDM mapping for the serial outputs (Port 2, Channel 7, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_OUT29                     (0xF39D)   /* FTDM mapping for the serial outputs (Port 2, Channel 7, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_OUT30                     (0xF39E)   /* FTDM mapping for the serial outputs (Port 2, Channel 7, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_OUT31                     (0xF39F)   /* FTDM mapping for the serial outputs (Port 2, Channel 7, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_OUT32                     (0xF3A0)   /* FTDM mapping for the serial outputs (Port 3, Channel 0, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_OUT33                     (0xF3A1)   /* FTDM mapping for the serial outputs (Port 3, Channel 0, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_OUT34                     (0xF3A2)   /* FTDM mapping for the serial outputs (Port 3, Channel 0, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_OUT35                     (0xF3A3)   /* FTDM mapping for the serial outputs (Port 3, Channel 0, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_OUT36                     (0xF3A4)   /* FTDM mapping for the serial outputs (Port 3, Channel 1, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_OUT37                     (0xF3A5)   /* FTDM mapping for the serial outputs (Port 3, Channel 1, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_OUT38                     (0xF3A6)   /* FTDM mapping for the serial outputs (Port 3, Channel 1, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_OUT39                     (0xF3A7)   /* FTDM mapping for the serial outputs (Port 3, Channel 1, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_OUT40                     (0xF3A8)   /* FTDM mapping for the serial outputs (Port 3, Channel 2, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_OUT41                     (0xF3A9)   /* FTDM mapping for the serial outputs (Port 3, Channel 2, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_OUT42                     (0xF3AA)   /* FTDM mapping for the serial outputs (Port 3, Channel 2, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_OUT43                     (0xF3AB)   /* FTDM mapping for the serial outputs (Port 3, Channel 2, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_OUT44                     (0xF3AC)   /* FTDM mapping for the serial outputs (Port 3, Channel 3, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_OUT45                     (0xF3AD)   /* FTDM mapping for the serial outputs (Port 3, Channel 3, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_OUT46                     (0xF3AE)   /* FTDM mapping for the serial outputs (Port 3, Channel 3, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_OUT47                     (0xF3AF)   /* FTDM mapping for the serial outputs (Port 3, Channel 3, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_OUT48                     (0xF3B0)   /* FTDM mapping for the serial outputs (Port 3, Channel 4, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_OUT49                     (0xF3B1)   /* FTDM mapping for the serial outputs (Port 3, Channel 4, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_OUT50                     (0xF3B2)   /* FTDM mapping for the serial outputs (Port 3, Channel 4, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_OUT51                     (0xF3B3)   /* FTDM mapping for the serial outputs (Port 3, Channel 4, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_OUT52                     (0xF3B4)   /* FTDM mapping for the serial outputs (Port 3, Channel 5, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_OUT53                     (0xF3B5)   /* FTDM mapping for the serial outputs (Port 3, Channel 5, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_OUT54                     (0xF3B6)   /* FTDM mapping for the serial outputs (Port 3, Channel 5, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_OUT55                     (0xF3B7)   /* FTDM mapping for the serial outputs (Port 3, Channel 5, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_OUT56                     (0xF3B8)   /* FTDM mapping for the serial outputs (Port 3, Channel 6, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_OUT57                     (0xF3B9)   /* FTDM mapping for the serial outputs (Port 3, Channel 6, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_OUT58                     (0xF3BA)   /* FTDM mapping for the serial outputs (Port 3, Channel 6, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_OUT59                     (0xF3BB)   /* FTDM mapping for the serial outputs (Port 3, Channel 6, Bits[7:0]) */
#define    ADAU1452_REG_FTDM_OUT60                     (0xF3BC)   /* FTDM mapping for the serial outputs (Port 3, Channel 7, Bits[31:24]) */
#define    ADAU1452_REG_FTDM_OUT61                     (0xF3BD)   /* FTDM mapping for the serial outputs (Port 3, Channel 7, Bits[23:16]) */
#define    ADAU1452_REG_FTDM_OUT62                     (0xF3BE)   /* FTDM mapping for the serial outputs (Port 3, Channel 7, Bits[15:8]) */
#define    ADAU1452_REG_FTDM_OUT63                     (0xF3BF)   /* FTDM mapping for the serial outputs (Port 3, Channel 7, Bits[7:0]) */
#define    ADAU1452_REG_HIBERNATE                      (0xF400)   /* Hibernate setting */
#define    ADAU1452_REG_START_PULSE                    (0xF401)   /* Start pulse selection */
#define    ADAU1452_REG_START_CORE                     (0xF402)   /* Instruction to start the core */
#define    ADAU1452_REG_KILL_CORE                      (0xF403)   /* Instruction to stop the core */
#define    ADAU1452_REG_START_ADDRESS                  (0xF404)   /* Start address of the program */
#define    ADAU1452_REG_CORE_STATUS                    (0xF405)   /* Core status */
#define    ADAU1452_REG_PANIC_CLEAR                    (0xF421)   /* Clear the panic manager */
#define    ADAU1452_REG_PANIC_PARITY_MASK              (0xF422)   /* Panic parity */
#define    ADAU1452_REG_PANIC_SOFTWARE_MASK            (0xF423)   /* Panic Mask 0 */
#define    ADAU1452_REG_PANIC_WD_MASK                  (0xF424)   /* Panic Mask 1 */
#define    ADAU1452_REG_PANIC_STACK_MASK               (0xF425)   /* Panic Mask 2 */
#define    ADAU1452_REG_PANIC_LOOP_MASK                (0xF426)   /* Panic Mask 3 */
#define    ADAU1452_REG_PANIC_FLAG                     (0xF427)   /* Panic flag */
#define    ADAU1452_REG_PANIC_CODE                     (0xF428)   /* Panic code */
#define    ADAU1452_REG_EXECUTE_COUNT                  (0xF432)   /* Execute stage error program count */
#define    ADAU1452_REG_WATCHDOG_MAXCOUNT              (0xF443)   /* Watchdog maximum count */
#define    ADAU1452_REG_WATCHDOG_PRESCALE              (0xF444)   /* Watchdog prescale */
#define    ADAU1452_REG_BLOCKINT_EN                    (0xF450)   /* Enable block interrupts */
#define    ADAU1452_REG_BLOCKINT_VALUE                 (0xF451)   /* Value for the block interrupt counter */
#define    ADAU1452_REG_PROG_CNTR0                     (0xF460)   /* Program counter, Bits[23:16] */
#define    ADAU1452_REG_PROG_CNTR1                     (0xF461)   /* Program counter, Bits[15:0] */
#define    ADAU1452_REG_PROG_CNTR_CLEAR                (0xF462)   /* Program counter clear */
#define    ADAU1452_REG_PROG_CNTR_LENGTH0              (0xF463)   /* Program counter length, Bits[23:16] */
#define    ADAU1452_REG_PROG_CNTR_LENGTH1              (0xF464)   /* Program counter length, Bits[15:0] */
#define    ADAU1452_REG_PROG_CNTR_MAXLENGTH0           (0xF465)   /* Program counter max length, Bits[23:16] */
#define    ADAU1452_REG_PROG_CNTR_MAXLENGTH1           (0xF466)   /* Program counter max length, Bits[15:0] */
#define    ADAU1452_REG_MP0_MODE                       (0xF510)   /* Multipurpose pin mode (SS_M/MP0) */
#define    ADAU1452_REG_MP1_MODE                       (0xF511)   /* Multipurpose pin mode (MOSI_M/MP1) */
#define    ADAU1452_REG_MP2_MODE                       (0xF512)   /* Multipurpose pin mode (SCL_M/SCLK_M/MP2) */
#define    ADAU1452_REG_MP3_MODE                       (0xF513)   /* Multipurpose pin mode (SDA_M,MISO_M/MP3) */
#define    ADAU1452_REG_MP4_MODE                       (0xF514)   /* Multipurpose pin mode (LRCLK_OUT0/MP4) */
#define    ADAU1452_REG_MP5_MODE                       (0xF515)   /* Multipurpose pin mode (LRCLK_OUT1/MP5) */
#define    ADAU1452_REG_MP6_MODE                       (0xF516)   /* Multipurpose pin mode (MP6) */
#define    ADAU1452_REG_MP7_MODE                       (0xF517)   /* Multipurpose pin mode (MP7) */
#define    ADAU1452_REG_MP8_MODE                       (0xF518)   /* Multipurpose pin mode (LRCLK_OUT2/MP8) */
#define    ADAU1452_REG_MP9_MODE                       (0xF519)   /* Multipurpose pin mode (LRCLK_OUT3/MP9) */
#define    ADAU1452_REG_MP10_MODE                      (0xF51A)   /* Multipurpose pin mode (LRCLK_IN0/MP10) */
#define    ADAU1452_REG_MP11_MODE                      (0xF51B)   /* Multipurpose pin mode (LRCLK_IN1/MP11) */
#define    ADAU1452_REG_MP12_MODE                      (0xF51C)   /* Multipurpose pin mode (LRCLK_IN2/MP12) */
#define    ADAU1452_REG_MP13_MODE                      (0xF51D)   /* Multipurpose pin mode (LRCLK_IN3/MP13) */
#define    ADAU1452_REG_MP0_WRITE                      (0xF520)   /* Multipurpose pin write value (SS_M/MP0) */
#define    ADAU1452_REG_MP1_WRITE                      (0xF521)   /* Multipurpose pin write value (MOSI_M/MP1) */
#define    ADAU1452_REG_MP2_WRITE                      (0xF522)   /* Multipurpose pin write value SCL_M/SCLK_M/MP2) */
#define    ADAU1452_REG_MP3_WRITE                      (0xF523)   /* Multipurpose pin write value (SDA_M,MISO_M/MP3) */
#define    ADAU1452_REG_MP4_WRITE                      (0xF524)   /* Multipurpose pin write value (LRCLK_OUT0/MP4) */
#define    ADAU1452_REG_MP5_WRITE                      (0xF525)   /* Multipurpose pin write value (LRCLK_OUT1/MP5) */
#define    ADAU1452_REG_MP6_WRITE                      (0xF526)   /* Multipurpose pin write value (MP6) */
#define    ADAU1452_REG_MP7_WRITE                      (0xF527)   /* Multipurpose pin write value (MP7) */
#define    ADAU1452_REG_MP8_WRITE                      (0xF528)   /* Multipurpose pin write value (LRCLK_OUT2/MP8) */
#define    ADAU1452_REG_MP9_WRITE                      (0xF529)   /* Multipurpose pin write value (LRCLK_OUT3/MP9) */
#define    ADAU1452_REG_MP10_WRITE                     (0xF52A)   /* Multipurpose pin write value (LRCLK_IN0/MP10) */
#define    ADAU1452_REG_MP11_WRITE                     (0xF52B)   /* Multipurpose pin write value (LRCLK_IN1/MP11) */
#define    ADAU1452_REG_MP12_WRITE                     (0xF52C)   /* Multipurpose pin write value (LRCLK_IN2/MP12) */
#define    ADAU1452_REG_MP13_WRITE                     (0xF52D)   /* Multipurpose pin write value (LRCLK_IN3/MP13) */
#define    ADAU1452_REG_MP0_READ                       (0xF530)   /* Multipurpose pin read value (SS_M/MP0) */
#define    ADAU1452_REG_MP1_READ                       (0xF531)   /* Multipurpose pin read value (MOSI_M/MP1) */
#define    ADAU1452_REG_MP2_READ                       (0xF532)   /* Multipurpose pin read value (SCL_M/SCLK_M/MP2) */
#define    ADAU1452_REG_MP3_READ                       (0xF533)   /* Multipurpose pin read value (SDA_M,MISO_M/MP3) */
#define    ADAU1452_REG_MP4_READ                       (0xF534)   /* Multipurpose pin read value (LRCLK_OUT0/MP4) */
#define    ADAU1452_REG_MP5_READ                       (0xF535)   /* Multipurpose pin read value (LRCLK_OUT1/MP5) */
#define    ADAU1452_REG_MP6_READ                       (0xF536)   /* Multipurpose pin read value (MP6) */
#define    ADAU1452_REG_MP7_READ                       (0xF537)   /* Multipurpose pin read value (MP7) */
#define    ADAU1452_REG_MP8_READ                       (0xF538)   /* Multipurpose pin read value (LRCLK_OUT2/MP8) */
#define    ADAU1452_REG_MP9_READ                       (0xF539)   /* Multipurpose pin read value (LRCLK_OUT3/MP9) */
#define    ADAU1452_REG_MP10_READ                      (0xF53A)   /* Multipurpose pin read value (LRCLK_IN0/MP10) */
#define    ADAU1452_REG_MP11_READ                      (0xF53B)   /* Multipurpose pin read value (LRCLK_IN1/MP11) */
#define    ADAU1452_REG_MP12_READ                      (0xF53C)   /* Multipurpose pin read value (LRCLK_IN2/MP12) */
#define    ADAU1452_REG_MP13_READ                      (0xF53D)   /* Multipurpose pin read value (LRCLK_IN3/MP13) */
#define    ADAU1452_REG_DMIC_CTRL0                     (0xF560)   /* Digital PDM microphone control (Channel 0 and Channel 1) */
#define    ADAU1452_REG_DMIC_CTRL1                     (0xF561)   /* Digital PDM microphone control (Channel 2 and Channel 3) */
#define    ADAU1452_REG_ASRC_LOCK                      (0xF580)   /* ASRC lock status */
#define    ADAU1452_REG_ASRC_MUTE                      (0xF581)   /* ASRC mute */
#define    ADAU1452_REG_ASRC0_RATIO                    (0xF582)   /* ASRC ratio (ASRC 0, Channel 0 and Channel 1) */
#define    ADAU1452_REG_ASRC1_RATIO                    (0xF583)   /* ASRC ratio (ASRC 1, Channel 2 and Channel 3) */
#define    ADAU1452_REG_ASRC2_RATIO                    (0xF584)   /* ASRC ratio (ASRC 2, Channel 4 and Channel 5) */
#define    ADAU1452_REG_ASRC3_RATIO                    (0xF585)   /* ASRC ratio (ASRC 3, Channel 6 and Channel 7) */
#define    ADAU1452_REG_ASRC4_RATIO                    (0xF586)   /* ASRC ratio (ASRC 4, Channel 8 and Channel 9) */
#define    ADAU1452_REG_ASRC5_RATIO                    (0xF587)   /* ASRC ratio (ASRC 5, Channel 10 and Channel 11) */
#define    ADAU1452_REG_ASRC6_RATIO                    (0xF588)   /* ASRC ratio (ASRC 6, Channel 12 and Channel 13) */
#define    ADAU1452_REG_ASRC7_RATIO                    (0xF589)   /* ASRC ratio (ASRC 7, Channel 14 and Channel 15) */
#define    ADAU1452_REG_ADC_READ0                      (0xF5A0)   /* Auxiliary ADC read value (AUXADC0) */
#define    ADAU1452_REG_ADC_READ1                      (0xF5A1)   /* Auxiliary ADC read value (AUXADC1) */
#define    ADAU1452_REG_ADC_READ2                      (0xF5A2)   /* Auxiliary ADC read value (AUXADC2) */
#define    ADAU1452_REG_ADC_READ3                      (0xF5A3)   /* Auxiliary ADC read value (AUXADC3) */
#define    ADAU1452_REG_ADC_READ4                      (0xF5A4)   /* Auxiliary ADC read value (AUXADC4) */
#define    ADAU1452_REG_ADC_READ5                      (0xF5A5)   /* Auxiliary ADC read value (AUXADC5) */
#define    ADAU1452_REG_SPDIF_LOCK_DET                 (0xF600)   /* S/PDIF receiver lock bit detection */
#define    ADAU1452_REG_SPDIF_RX_CTRL                  (0xF601)   /* S/PDIF receiver control */
#define    ADAU1452_REG_SPDIF_RX_DECODE                (0xF602)   /* Decoded signals from the S/PDIF receiver */
#define    ADAU1452_REG_SPDIF_RX_COMPRMODE             (0xF603)   /* Compression mode from the S/PDIF receiver */
#define    ADAU1452_REG_SPDIF_RESTART                  (0xF604)   /* Automatically resume S/PDIF receiver audio input */
#define    ADAU1452_REG_SPDIF_LOSS_OF_LOCK             (0xF605)   /* S/PDIF receiver loss of lock detection */
#define    ADAU1452_REG_SPDIF_AUX_EN                   (0xF608)   /* S/PDIF receiver auxiliary outputs enable */
#define    ADAU1452_REG_SPDIF_RX_AUXBIT_READY          (0xF60F)   /* S/PDIF receiver auxiliary bits ready flag */
#define    ADAU1452_REG_SPDIF_RX_CS_LEFT_0             (0xF610)   /* S/PDIF receiver channel status bits (left) */
#define    ADAU1452_REG_SPDIF_RX_CS_LEFT_1             (0xF611)   /* S/PDIF receiver channel status bits (left) */
#define    ADAU1452_REG_SPDIF_RX_CS_LEFT_2             (0xF612)   /* S/PDIF receiver channel status bits (left) */
#define    ADAU1452_REG_SPDIF_RX_CS_LEFT_3             (0xF613)   /* S/PDIF receiver channel status bits (left) */
#define    ADAU1452_REG_SPDIF_RX_CS_LEFT_4             (0xF614)   /* S/PDIF receiver channel status bits (left) */
#define    ADAU1452_REG_SPDIF_RX_CS_LEFT_5             (0xF615)   /* S/PDIF receiver channel status bits (left) */
#define    ADAU1452_REG_SPDIF_RX_CS_LEFT_6             (0xF616)   /* S/PDIF receiver channel status bits (left) */
#define    ADAU1452_REG_SPDIF_RX_CS_LEFT_7             (0xF617)   /* S/PDIF receiver channel status bits (left) */
#define    ADAU1452_REG_SPDIF_RX_CS_LEFT_8             (0xF618)   /* S/PDIF receiver channel status bits (left) */
#define    ADAU1452_REG_SPDIF_RX_CS_LEFT_9             (0xF619)   /* S/PDIF receiver channel status bits (left) */
#define    ADAU1452_REG_SPDIF_RX_CS_LEFT_10            (0xF61A)   /* S/PDIF receiver channel status bits (left) */
#define    ADAU1452_REG_SPDIF_RX_CS_LEFT_11            (0xF61B)   /* S/PDIF receiver channel status bits (left) */
#define    ADAU1452_REG_SPDIF_RX_CS_RIGHT_0            (0xF620)   /* S/PDIF receiver channel status bits (right) */
#define    ADAU1452_REG_SPDIF_RX_CS_RIGHT_1            (0xF621)   /* S/PDIF receiver channel status bits (right) */
#define    ADAU1452_REG_SPDIF_RX_CS_RIGHT_2            (0xF622)   /* S/PDIF receiver channel status bits (right) */
#define    ADAU1452_REG_SPDIF_RX_CS_RIGHT_3            (0xF623)   /* S/PDIF receiver channel status bits (right) */
#define    ADAU1452_REG_SPDIF_RX_CS_RIGHT_4            (0xF624)   /* S/PDIF receiver channel status bits (right) */
#define    ADAU1452_REG_SPDIF_RX_CS_RIGHT_5            (0xF625)   /* S/PDIF receiver channel status bits (right) */
#define    ADAU1452_REG_SPDIF_RX_CS_RIGHT_6            (0xF626)   /* S/PDIF receiver channel status bits (right) */
#define    ADAU1452_REG_SPDIF_RX_CS_RIGHT_7            (0xF627)   /* S/PDIF receiver channel status bits (right) */
#define    ADAU1452_REG_SPDIF_RX_CS_RIGHT_8            (0xF628)   /* S/PDIF receiver channel status bits (right) */
#define    ADAU1452_REG_SPDIF_RX_CS_RIGHT_9            (0xF629)   /* S/PDIF receiver channel status bits (right) */
#define    ADAU1452_REG_SPDIF_RX_CS_RIGHT_10           (0xF62A)   /* S/PDIF receiver channel status bits (right) */
#define    ADAU1452_REG_SPDIF_RX_CS_RIGHT_11           (0xF62B)   /* S/PDIF receiver channel status bits (right) */
#define    ADAU1452_REG_SPDIF_RX_UD_LEFT_0             (0xF630)   /* S/PDIF receiver user data bits (left) */
#define    ADAU1452_REG_SPDIF_RX_UD_LEFT_1             (0xF631)   /* S/PDIF receiver user data bits (left) */
#define    ADAU1452_REG_SPDIF_RX_UD_LEFT_2             (0xF632)   /* S/PDIF receiver user data bits (left) */
#define    ADAU1452_REG_SPDIF_RX_UD_LEFT_3             (0xF633)   /* S/PDIF receiver user data bits (left) */
#define    ADAU1452_REG_SPDIF_RX_UD_LEFT_4             (0xF634)   /* S/PDIF receiver user data bits (left) */
#define    ADAU1452_REG_SPDIF_RX_UD_LEFT_5             (0xF635)   /* S/PDIF receiver user data bits (left) */
#define    ADAU1452_REG_SPDIF_RX_UD_LEFT_6             (0xF636)   /* S/PDIF receiver user data bits (left) */
#define    ADAU1452_REG_SPDIF_RX_UD_LEFT_7             (0xF637)   /* S/PDIF receiver user data bits (left) */
#define    ADAU1452_REG_SPDIF_RX_UD_LEFT_8             (0xF638)   /* S/PDIF receiver user data bits (left) */
#define    ADAU1452_REG_SPDIF_RX_UD_LEFT_9             (0xF639)   /* S/PDIF receiver user data bits (left) */
#define    ADAU1452_REG_SPDIF_RX_UD_LEFT_10            (0xF63A)   /* S/PDIF receiver user data bits (left) */
#define    ADAU1452_REG_SPDIF_RX_UD_LEFT_11            (0xF63B)   /* S/PDIF receiver user data bits (left) */
#define    ADAU1452_REG_SPDIF_RX_UD_RIGHT_0            (0xF640)   /* S/PDIF receiver user data bits (right) */
#define    ADAU1452_REG_SPDIF_RX_UD_RIGHT_1            (0xF641)   /* S/PDIF receiver user data bits (right) */
#define    ADAU1452_REG_SPDIF_RX_UD_RIGHT_2            (0xF642)   /* S/PDIF receiver user data bits (right) */
#define    ADAU1452_REG_SPDIF_RX_UD_RIGHT_3            (0xF643)   /* S/PDIF receiver user data bits (right) */
#define    ADAU1452_REG_SPDIF_RX_UD_RIGHT_4            (0xF644)   /* S/PDIF receiver user data bits (right) */
#define    ADAU1452_REG_SPDIF_RX_UD_RIGHT_5            (0xF645)   /* S/PDIF receiver user data bits (right) */
#define    ADAU1452_REG_SPDIF_RX_UD_RIGHT_6            (0xF646)   /* S/PDIF receiver user data bits (right) */
#define    ADAU1452_REG_SPDIF_RX_UD_RIGHT_7            (0xF647)   /* S/PDIF receiver user data bits (right) */
#define    ADAU1452_REG_SPDIF_RX_UD_RIGHT_8            (0xF648)   /* S/PDIF receiver user data bits (right) */
#define    ADAU1452_REG_SPDIF_RX_UD_RIGHT_9            (0xF649)   /* S/PDIF receiver user data bits (right) */
#define    ADAU1452_REG_SPDIF_RX_UD_RIGHT_10           (0xF64A)   /* S/PDIF receiver user data bits (right) */
#define    ADAU1452_REG_SPDIF_RX_UD_RIGHT_11           (0xF64B)   /* S/PDIF receiver user data bits (right) */
#define    ADAU1452_REG_SPDIF_RX_VB_LEFT_0             (0xF650)   /* S/PDIF receiver validity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_VB_LEFT_1             (0xF651)   /* S/PDIF receiver validity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_VB_LEFT_2             (0xF652)   /* S/PDIF receiver validity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_VB_LEFT_3             (0xF653)   /* S/PDIF receiver validity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_VB_LEFT_4             (0xF654)   /* S/PDIF receiver validity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_VB_LEFT_5             (0xF655)   /* S/PDIF receiver validity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_VB_LEFT_6             (0xF656)   /* S/PDIF receiver validity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_VB_LEFT_7             (0xF657)   /* S/PDIF receiver validity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_VB_LEFT_8             (0xF658)   /* S/PDIF receiver validity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_VB_LEFT_9             (0xF659)   /* S/PDIF receiver validity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_VB_LEFT_10            (0xF65A)   /* S/PDIF receiver validity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_VB_LEFT_11            (0xF65B)   /* S/PDIF receiver validity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_VB_RIGHT_0            (0xF660)   /* S/PDIF receiver validity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_VB_RIGHT_1            (0xF661)   /* S/PDIF receiver validity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_VB_RIGHT_2            (0xF662)   /* S/PDIF receiver validity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_VB_RIGHT_3            (0xF663)   /* S/PDIF receiver validity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_VB_RIGHT_4            (0xF664)   /* S/PDIF receiver validity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_VB_RIGHT_5            (0xF665)   /* S/PDIF receiver validity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_VB_RIGHT_6            (0xF666)   /* S/PDIF receiver validity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_VB_RIGHT_7            (0xF667)   /* S/PDIF receiver validity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_VB_RIGHT_8            (0xF668)   /* S/PDIF receiver validity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_VB_RIGHT_9            (0xF669)   /* S/PDIF receiver validity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_VB_RIGHT_10           (0xF66A)   /* S/PDIF receiver validity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_VB_RIGHT_11           (0xF66B)   /* S/PDIF receiver validity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_PB_LEFT_0             (0xF670)   /* S/PDIF receiver parity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_PB_LEFT_1             (0xF671)   /* S/PDIF receiver parity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_PB_LEFT_2             (0xF672)   /* S/PDIF receiver parity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_PB_LEFT_3             (0xF673)   /* S/PDIF receiver parity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_PB_LEFT_4             (0xF674)   /* S/PDIF receiver parity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_PB_LEFT_5             (0xF675)   /* S/PDIF receiver parity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_PB_LEFT_6             (0xF676)   /* S/PDIF receiver parity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_PB_LEFT_7             (0xF677)   /* S/PDIF receiver parity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_PB_LEFT_8             (0xF678)   /* S/PDIF receiver parity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_PB_LEFT_9             (0xF679)   /* S/PDIF receiver parity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_PB_LEFT_10            (0xF67A)   /* S/PDIF receiver parity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_PB_LEFT_11            (0xF67B)   /* S/PDIF receiver parity bits (left) */
#define    ADAU1452_REG_SPDIF_RX_PB_RIGHT_0            (0xF680)   /* S/PDIF receiver parity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_PB_RIGHT_1            (0xF681)   /* S/PDIF receiver parity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_PB_RIGHT_2            (0xF682)   /* S/PDIF receiver parity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_PB_RIGHT_3            (0xF683)   /* S/PDIF receiver parity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_PB_RIGHT_4            (0xF684)   /* S/PDIF receiver parity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_PB_RIGHT_5            (0xF685)   /* S/PDIF receiver parity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_PB_RIGHT_6            (0xF686)   /* S/PDIF receiver parity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_PB_RIGHT_7            (0xF687)   /* S/PDIF receiver parity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_PB_RIGHT_8            (0xF688)   /* S/PDIF receiver parity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_PB_RIGHT_9            (0xF689)   /* S/PDIF receiver parity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_PB_RIGHT_10           (0xF68A)   /* S/PDIF receiver parity bits (right) */
#define    ADAU1452_REG_SPDIF_RX_PB_RIGHT_11           (0xF68B)   /* S/PDIF receiver parity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_EN                    (0xF690)   /* S/PDIF transmitter enable */
#define    ADAU1452_REG_SPDIF_TX_CTRL                  (0xF691)   /* S/PDIF transmitter control */
#define    ADAU1452_REG_SPDIF_TX_AUXBIT_SOURCE         (0xF69F)   /* S/PDIF transmitter auxiliary bits source select */
#define    ADAU1452_REG_SPDIF_TX_CS_LEFT_0             (0xF6A0)   /* S/PDIF transmitter channel status bits (left) */
#define    ADAU1452_REG_SPDIF_TX_CS_LEFT_1             (0xF6A1)   /* S/PDIF transmitter channel status bits (left) */
#define    ADAU1452_REG_SPDIF_TX_CS_LEFT_2             (0xF6A2)   /* S/PDIF transmitter channel status bits (left) */
#define    ADAU1452_REG_SPDIF_TX_CS_LEFT_3             (0xF6A3)   /* S/PDIF transmitter channel status bits (left) */
#define    ADAU1452_REG_SPDIF_TX_CS_LEFT_4             (0xF6A4)   /* S/PDIF transmitter channel status bits (left) */
#define    ADAU1452_REG_SPDIF_TX_CS_LEFT_5             (0xF6A5)   /* S/PDIF transmitter channel status bits (left) */
#define    ADAU1452_REG_SPDIF_TX_CS_LEFT_6             (0xF6A6)   /* S/PDIF transmitter channel status bits (left) */
#define    ADAU1452_REG_SPDIF_TX_CS_LEFT_7             (0xF6A7)   /* S/PDIF transmitter channel status bits (left) */
#define    ADAU1452_REG_SPDIF_TX_CS_LEFT_8             (0xF6A8)   /* S/PDIF transmitter channel status bits (left) */
#define    ADAU1452_REG_SPDIF_TX_CS_LEFT_9             (0xF6A9)   /* S/PDIF transmitter channel status bits (left) */
#define    ADAU1452_REG_SPDIF_TX_CS_LEFT_10            (0xF6AA)   /* S/PDIF transmitter channel status bits (left) */
#define    ADAU1452_REG_SPDIF_TX_CS_LEFT_11            (0xF6AB)   /* S/PDIF transmitter channel status bits (left) */
#define    ADAU1452_REG_SPDIF_TX_CS_RIGHT_0            (0xF6B0)   /* S/PDIF transmitter channel status bits (right) */
#define    ADAU1452_REG_SPDIF_TX_CS_RIGHT_1            (0xF6B1)   /* S/PDIF transmitter channel status bits (right) */
#define    ADAU1452_REG_SPDIF_TX_CS_RIGHT_2            (0xF6B2)   /* S/PDIF transmitter channel status bits (right) */
#define    ADAU1452_REG_SPDIF_TX_CS_RIGHT_3            (0xF6B3)   /* S/PDIF transmitter channel status bits (right) */
#define    ADAU1452_REG_SPDIF_TX_CS_RIGHT_4            (0xF6B4)   /* S/PDIF transmitter channel status bits (right) */
#define    ADAU1452_REG_SPDIF_TX_CS_RIGHT_5            (0xF6B5)   /* S/PDIF transmitter channel status bits (right)) */
#define    ADAU1452_REG_SPDIF_TX_CS_RIGHT_6            (0xF6B6)   /* S/PDIF transmitter channel status bits (right)) */
#define    ADAU1452_REG_SPDIF_TX_CS_RIGHT_7            (0xF6B7)   /* S/PDIF transmitter channel status bits (right) */
#define    ADAU1452_REG_SPDIF_TX_CS_RIGHT_8            (0xF6B8)   /* S/PDIF transmitter channel status bits (right) */
#define    ADAU1452_REG_SPDIF_TX_CS_RIGHT_9            (0xF6B9)   /* S/PDIF transmitter channel status bits (right) */
#define    ADAU1452_REG_SPDIF_TX_CS_RIGHT_10           (0xF6BA)   /* S/PDIF transmitter channel status bits (right) */
#define    ADAU1452_REG_SPDIF_TX_CS_RIGHT_11           (0xF6BB)   /* S/PDIF transmitter channel status bits (right) */
#define    ADAU1452_REG_SPDIF_TX_UD_LEFT_0             (0xF6C0)   /* S/PDIF transmitter user data bits (left) */
#define    ADAU1452_REG_SPDIF_TX_UD_LEFT_1             (0xF6C1)   /* S/PDIF transmitter user data bits (left) */
#define    ADAU1452_REG_SPDIF_TX_UD_LEFT_2             (0xF6C2)   /* S/PDIF transmitter user data bits (left) */
#define    ADAU1452_REG_SPDIF_TX_UD_LEFT_3             (0xF6C3)   /* S/PDIF transmitter user data bits (left) */
#define    ADAU1452_REG_SPDIF_TX_UD_LEFT_4             (0xF6C4)   /* S/PDIF transmitter user data bits (left) */
#define    ADAU1452_REG_SPDIF_TX_UD_LEFT_5             (0xF6C5)   /* S/PDIF transmitter user data bits (left) */
#define    ADAU1452_REG_SPDIF_TX_UD_LEFT_6             (0xF6C6)   /* S/PDIF transmitter user data bits (left) */
#define    ADAU1452_REG_SPDIF_TX_UD_LEFT_7             (0xF6C7)   /* S/PDIF transmitter user data bits (left) */
#define    ADAU1452_REG_SPDIF_TX_UD_LEFT_8             (0xF6C8)   /* S/PDIF transmitter user data bits (left) */
#define    ADAU1452_REG_SPDIF_TX_UD_LEFT_9             (0xF6C9)   /* S/PDIF transmitter user data bits (left) */
#define    ADAU1452_REG_SPDIF_TX_UD_LEFT_10            (0xF6CA)   /* S/PDIF transmitter user data bits (left)) */
#define    ADAU1452_REG_SPDIF_TX_UD_LEFT_11            (0xF6CB)   /* S/PDIF transmitter user data bits (left) */
#define    ADAU1452_REG_SPDIF_TX_UD_RIGHT_0            (0xF6D0)   /* S/PDIF transmitter user data bits (right) */
#define    ADAU1452_REG_SPDIF_TX_UD_RIGHT_1            (0xF6D1)   /* S/PDIF transmitter user data bits (right) */
#define    ADAU1452_REG_SPDIF_TX_UD_RIGHT_2            (0xF6D2)   /* S/PDIF transmitter user data bits (right) */
#define    ADAU1452_REG_SPDIF_TX_UD_RIGHT_3            (0xF6D3)   /* S/PDIF transmitter user data bits (right) */
#define    ADAU1452_REG_SPDIF_TX_UD_RIGHT_4            (0xF6D4)   /* S/PDIF transmitter user data bits (right) */
#define    ADAU1452_REG_SPDIF_TX_UD_RIGHT_5            (0xF6D5)   /* S/PDIF transmitter user data bits (right) */
#define    ADAU1452_REG_SPDIF_TX_UD_RIGHT_6            (0xF6D6)   /* S/PDIF transmitter user data bits (right) */
#define    ADAU1452_REG_SPDIF_TX_UD_RIGHT_7            (0xF6D7)   /* S/PDIF transmitter user data bits (right) */
#define    ADAU1452_REG_SPDIF_TX_UD_RIGHT_8            (0xF6D8)   /* S/PDIF transmitter user data bits (right) */
#define    ADAU1452_REG_SPDIF_TX_UD_RIGHT_9            (0xF6D9)   /* S/PDIF transmitter user data bits (right) */
#define    ADAU1452_REG_SPDIF_TX_UD_RIGHT_10           (0xF6DA)   /* S/PDIF transmitter user data bits (right) */
#define    ADAU1452_REG_SPDIF_TX_UD_RIGHT_11           (0xF6DB)   /* S/PDIF transmitter user data bits (right) */
#define    ADAU1452_REG_SPDIF_TX_VB_LEFT_0             (0xF6E0)   /* S/PDIF transmitter validity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_VB_LEFT_1             (0xF6E1)   /* S/PDIF transmitter validity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_VB_LEFT_2             (0xF6E2)   /* S/PDIF transmitter validity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_VB_LEFT_3             (0xF6E3)   /* S/PDIF transmitter validity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_VB_LEFT_4             (0xF6E4)   /* S/PDIF transmitter validity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_VB_LEFT_5             (0xF6E5)   /* S/PDIF transmitter validity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_VB_LEFT_6             (0xF6E6)   /* S/PDIF transmitter validity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_VB_LEFT_7             (0xF6E7)   /* S/PDIF transmitter validity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_VB_LEFT_8             (0xF6E8)   /* S/PDIF transmitter validity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_VB_LEFT_9             (0xF6E9)   /* S/PDIF transmitter validity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_VB_LEFT_10            (0xF6EA)   /* S/PDIF transmitter validity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_VB_LEFT_11            (0xF6EB)   /* S/PDIF transmitter validity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_VB_RIGHT_0            (0xF6F0)   /* S/PDIF transmitter validity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_VB_RIGHT_1            (0xF6F1)   /* S/PDIF transmitter validity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_VB_RIGHT_2            (0xF6F2)   /* S/PDIF transmitter validity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_VB_RIGHT_3            (0xF6F3)   /* S/PDIF transmitter validity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_VB_RIGHT_4            (0xF6F4)   /* S/PDIF transmitter validity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_VB_RIGHT_5            (0xF6F5)   /* S/PDIF transmitter validity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_VB_RIGHT_6            (0xF6F6)   /* S/PDIF transmitter validity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_VB_RIGHT_7            (0xF6F7)   /* S/PDIF transmitter validity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_VB_RIGHT_8            (0xF6F8)   /* S/PDIF transmitter validity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_VB_RIGHT_9            (0xF6F9)   /* S/PDIF transmitter validity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_VB_RIGHT_10           (0xF6FA)   /* S/PDIF transmitter validity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_VB_RIGHT_11           (0xF6FB)   /* S/PDIF transmitter validity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_PB_LEFT_0             (0xF700)   /* S/PDIF transmitter parity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_PB_LEFT_1             (0xF701)   /* S/PDIF transmitter parity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_PB_LEFT_2             (0xF702)   /* S/PDIF transmitter parity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_PB_LEFT_3             (0xF703)   /* S/PDIF transmitter parity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_PB_LEFT_4             (0xF704)   /* S/PDIF transmitter parity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_PB_LEFT_5             (0xF705)   /* S/PDIF transmitter parity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_PB_LEFT_6             (0xF706)   /* S/PDIF transmitter parity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_PB_LEFT_7             (0xF707)   /* S/PDIF transmitter parity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_PB_LEFT_8             (0xF708)   /* S/PDIF transmitter parity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_PB_LEFT_9             (0xF709)   /* S/PDIF transmitter parity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_PB_LEFT_10            (0xF70A)   /* S/PDIF transmitter parity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_PB_LEFT_11            (0xF70B)   /* S/PDIF transmitter parity bits (left) */
#define    ADAU1452_REG_SPDIF_TX_PB_RIGHT_0            (0xF710)   /* S/PDIF transmitter parity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_PB_RIGHT_1            (0xF711)   /* S/PDIF transmitter parity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_PB_RIGHT_2            (0xF712)   /* S/PDIF transmitter parity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_PB_RIGHT_3            (0xF713)   /* S/PDIF transmitter parity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_PB_RIGHT_4            (0xF714)   /* S/PDIF transmitter parity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_PB_RIGHT_5            (0xF715)   /* S/PDIF transmitter parity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_PB_RIGHT_6            (0xF716)   /* S/PDIF transmitter parity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_PB_RIGHT_7            (0xF717)   /* S/PDIF transmitter parity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_PB_RIGHT_8            (0xF718)   /* S/PDIF transmitter parity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_PB_RIGHT_9            (0xF719)   /* S/PDIF transmitter parity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_PB_RIGHT_10           (0xF71A)   /* S/PDIF transmitter parity bits (right) */
#define    ADAU1452_REG_SPDIF_TX_PB_RIGHT_11           (0xF71B)   /* S/PDIF transmitter parity bits (right) */
#define    ADAU1452_REG_BCLK_IN0_PIN                   (0xF780)   /* BCLK input pins drive strength and slew rate (BCLK_IN0) */
#define    ADAU1452_REG_BCLK_IN1_PIN                   (0xF781)   /* BCLK input pins drive strength and slew rate (BCLK_IN1) */
#define    ADAU1452_REG_BCLK_IN2_PIN                   (0xF782)   /* BCLK input pins drive strength and slew rate (BCLK_IN2) */
#define    ADAU1452_REG_BCLK_IN3_PIN                   (0xF783)   /* BCLK input pins drive strength and slew rate (BCLK_IN3) */
#define    ADAU1452_REG_BCLK_OUT0_PIN                  (0xF784)   /* BCLK output pins drive strength and slew rate (BCLK_OUT0) */
#define    ADAU1452_REG_BCLK_OUT1_PIN                  (0xF785)   /* BCLK output pins drive strength and slew rate (BCLK_OUT1) */
#define    ADAU1452_REG_BCLK_OUT2_PIN                  (0xF786)   /* BCLK output pins drive strength and slew rate (BCLK_OUT2) */
#define    ADAU1452_REG_BCLK_OUT3_PIN                  (0xF787)   /* BCLK output pins drive strength and slew rate (BCLK_OUT3) */
#define    ADAU1452_REG_LRCLK_IN0_PIN                  (0xF788)   /* LRCLK input pins drive strength and slew rate (LRCLK_IN0) */
#define    ADAU1452_REG_LRCLK_IN1_PIN                  (0xF789)   /* LRCLK input pins drive strength and slew rate (LRCLK_IN1) */
#define    ADAU1452_REG_LRCLK_IN2_PIN                  (0xF78A)   /* LRCLK input pins drive strength and slew rate LRCLK_IN2) */
#define    ADAU1452_REG_LRCLK_IN3_PIN                  (0xF78B)   /* LRCLK input pins drive strength and slew rate (LRCLK_IN3) */
#define    ADAU1452_REG_LRCLK_OUT0_PIN                 (0xF78C)   /* LRCLK output pins drive strength and slew rate (LRCLK_OUT0) */
#define    ADAU1452_REG_LRCLK_OUT1_PIN                 (0xF78D)   /* LRCLK output pins drive strength and slew rate (LRCLK_OUT1) */
#define    ADAU1452_REG_LRCLK_OUT2_PIN                 (0xF78E)   /* LRCLK output pins drive strength and slew rate (LRCLK_OUT2) */
#define    ADAU1452_REG_LRCLK_OUT3_PIN                 (0xF78F)   /* LRCLK output pins drive strength and slew rate (LRCLK_OUT3) */
#define    ADAU1452_REG_SDATA_IN0_PIN                  (0xF790)   /* SDATA input pins drive strength and slew rate (SDATA_IN0) */
#define    ADAU1452_REG_SDATA_IN1_PIN                  (0xF791)   /* SDATA input pins drive strength and slew rate (SDATA_IN1) */
#define    ADAU1452_REG_SDATA_IN2_PIN                  (0xF792)   /* SDATA input pins drive strength and slew rate (SDATA_IN2) */
#define    ADAU1452_REG_SDATA_IN3_PIN                  (0xF793)   /* SDATA input pins drive strength and slew rate (SDATA_IN3) */
#define    ADAU1452_REG_SDATA_OUT0_PIN                 (0xF794)   /* SDATA output pins drive strength and slew rate (SDATA_OUT0) */
#define    ADAU1452_REG_SDATA_OUT1_PIN                 (0xF795)   /* SDATA output pins drive strength and slew rate (SDATA_OUT1) */
#define    ADAU1452_REG_SDATA_OUT2_PIN                 (0xF796)   /* SDATA output pins drive strength and slew rate (SDATA_OUT2) */
#define    ADAU1452_REG_SDATA_OUT3_PIN                 (0xF797)   /* SDATA output pins drive strength and slew rate (SDATA_OUT3) */
#define    ADAU1452_REG_SPDIF_TX_PIN                   (0xF798)   /* S/PDIF transmitter pin drive strength and slew rate */
#define    ADAU1452_REG_SCLK_SCL_PIN                   (0xF799)   /* SCLK/SCL pin drive strength and slew rate */
#define    ADAU1452_REG_MISO_SDA_PIN                   (0xF79A)   /* MISO/SDA pin drive strength and slew rate */
#define    ADAU1452_REG_SS_PIN                         (0xF79B)   /* SS/ADDR0 pin drive strength and slew rate */
#define    ADAU1452_REG_MOSI_ADDR1_PIN                 (0xF79C)   /* MOSI/ADDR1 pin drive strength and slew rate */
#define    ADAU1452_REG_SCLK_SCL_M_PIN                 (0xF79D)   /* SCL_M/SCLK_M/MP2 pin drive strength and slew rate */
#define    ADAU1452_REG_MISO_SDA_M_PIN                 (0xF79E)   /* SDA_M/MISO_M/MP3 pin drive strength and slew rate */
#define    ADAU1452_REG_SS_M_PIN                       (0xF79F)   /* SS_M/MP0 pin drive strength and slew rate */
#define    ADAU1452_REG_MOSI_M_PIN                     (0xF7A0)   /* MOSI_M/MP1 pin drive strength and slew rate */
#define    ADAU1452_REG_MP6_PIN                        (0xF7A1)   /* MP6 pin drive strength and slew rate */
#define    ADAU1452_REG_MP7_PIN                        (0xF7A2)   /* MP7 pin drive strength and slew rate */
#define    ADAU1452_REG_CLKOUT_PIN                     (0xF7A3)   /* CLKOUT pin drive strength and slew rate */
#define    ADAU1452_REG_SOFT_RESET                     (0xF890)   /* Soft reset */

#endif // _REGISTERS_ADAU1452_H_
