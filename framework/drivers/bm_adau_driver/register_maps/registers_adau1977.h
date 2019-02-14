/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 */

// Register map for the ADAU1977

#ifndef _REGISTERS_ADAU1977_H_
#define _REGISTERS_ADAU1977_H_

#define    ADAU1977_REG_M_POWER              (0x00)   /* M_POWER */
#define    ADAU1977_REG_PLL_CONTROL          (0x01)   /* PLL_CONTROL */
#define    ADAU1977_REG_BST_CONTROL          (0x02)   /* BST_CONTROL */
#define    ADAU1977_REG_MB_BST_CONTROL       (0x03)   /* MB_BST_CONTROL */
#define    ADAU1977_REG_BLOCK_POWER_SAI      (0x04)   /* BLOCK_POWER_SAI */
#define    ADAU1977_REG_SAI_CTRL0            (0x05)   /* SAI_CTRL0 */
#define    ADAU1977_REG_SAI_CTRL1            (0x06)   /* SAI_CTRL1 */
#define    ADAU1977_REG_SAI_CMAP12           (0x07)   /* SAI_CMAP12 */
#define    ADAU1977_REG_SAI_CMAP34           (0x08)   /* SAI_CMAP34 */
#define    ADAU1977_REG_SAI_OVERTEMP         (0x09)   /* SAI_OVERTEMP */
#define    ADAU1977_REG_POSTADC_GAIN1        (0x0A)   /* POSTADC_GAIN1 */
#define    ADAU1977_REG_POSTADC_GAIN2        (0x0B)   /* POSTADC_GAIN2 */
#define    ADAU1977_REG_POSTADC_GAIN3        (0x0C)   /* POSTADC_GAIN3 */
#define    ADAU1977_REG_POSTADC_GAIN4        (0x0D)   /* POSTADC_GAIN4 */
#define    ADAU1977_REG_MISC_CONTROL         (0x0E)   /* MISC_CONTROL */
#define    ADAU1977_REG_DIAG_CONTROL         (0x10)   /* DIAG_CONTROL */
#define    ADAU1977_REG_DIAG_STATUS1         (0x11)   /* DIAG_STATUS1 */
#define    ADAU1977_REG_DIAG_STATUS2         (0x12)   /* DIAG_STATUS2 */
#define    ADAU1977_REG_DIAG_STATUS3         (0x13)   /* DIAG_STATUS3 */
#define    ADAU1977_REG_DIAG_STATUS4         (0x14)   /* DIAG_STATUS4 */
#define    ADAU1977_REG_DIAG_IRQ1            (0x15)   /* DIAG_IRQ1 */
#define    ADAU1977_REG_DIAG_IRQ2            (0x16)   /* DIAG_IRQ2 */
#define    ADAU1977_REG_DIAG_ADJUST1         (0x17)   /* DIAG_ADJUST1 */
#define    ADAU1977_REG_DIAG_ADJUST2         (0x18)   /* DIAG_ADJUST2 */
#define    ADAU1977_REG_ASDC_CLIP            (0x19)   /* ASDC_CLIP */
#define    ADAU1977_REG_DC_HPF_CAL           (0x1A)   /* DC_HPF_CAL */

#endif // _REGISTERS_ADAU1977_H_
