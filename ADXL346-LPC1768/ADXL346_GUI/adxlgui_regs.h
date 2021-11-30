//==============================================================================
//
// Title:		adxlgui_regs.h
// Purpose:		A short description of the interface.
//
// Created on:	05/06/2019 at 15:17:22 by Mayela Zamora.
// Copyright:	Oxford University. All Rights Reserved.
//
//==============================================================================

#ifndef __adxlgui_regs_H__
#define __adxlgui_regs_H__

//==============================================================================
// Include files

// #include "cvidef.h"

//==============================================================================
// Constants
#define ADDR_DEVID           0x00
#define ADDR_THRESH_TAP      0x1D
#define ADDR_OFSX            0x1E
#define ADDR_OFSY            0x1F
#define ADDR_OFSZ            0x20
#define ADDR_DUR             0x21
#define ADDR_LATENT          0x22
#define ADDR_WINDOW          0x23
#define ADDR_THRESH_ACT      0x24
#define ADDR_THRESH_INACT    0x25
#define ADDR_TIME_INACT      0x26
#define ADDR_ACT_INACT_CTL   0x27
#define ADDR_THRESH_FF       0x28
#define ADDR_TIME_FF         0x29
#define ADDR_TAP_AXES        0x2A
#define ADDR_ACT_TAP_STATUS  0x2B
#define ADDR_BW_RATE         0x2C
#define ADDR_POWER_CTL       0x2D
#define ADDR_INT_ENABLE      0x2E
#define ADDR_INT_MAP         0x2F
#define ADDR_INT_SOURCE      0x30
#define ADDR_DATA_FORMAT     0x31
#define ADDR_DATAX0          0x32
#define ADDR_DATAX1          0x33
#define ADDR_DATAY0          0x34
#define ADDR_DATAY1          0x35
#define ADDR_DATAZ0          0x36
#define ADDR_DATAZ1          0x37
#define ADDR_FIFO_CTL        0x38
#define ADDR_FIFO_STATUS     0x39
#define ADDR_TAP_SIGN		 0x3A
#define ADDR_ORIENT_CONF	 0x3B
#define ADDR_ORIENT			 0x3C

// Accelerometer sampling rate codes (current shown as normal / low-power mode)
#define ACCEL_RATE_3200     0x0f    // 145 uA (no low-power mode)
#define ACCEL_RATE_1600     0x0e    // 100 uA (no low-power mode)
#define ACCEL_RATE_800      0x0d    // 145 uA (no low-power mode)
#define ACCEL_RATE_400      0x0c    // 145 uA / 100 uA
#define ACCEL_RATE_200      0x0b    // 145 uA /  65 uA
#define ACCEL_RATE_100      0x0a    // 145 uA /  55 uA
#define ACCEL_RATE_50       0x09    // 100 uA /  50 uA
#define ACCEL_RATE_25       0x08    //  65 uA /  40 uA
#define ACCEL_RATE_12_5     0x07    //  55 uA /  40 uA
#define ACCEL_RATE_6_25     0x06    //  40 uA (no low-power mode)
#define ACCEL_RATE_3_125    0x05
#define ACCEL_RATE_1_56     0x04

// In this API, the top two bits of the sampling rate value are used to determine the acceleromter's range.
// (For backwards compatibility we treat the top two bits as the reverse of the ADXL345's internal range values)
// 0=±16g, 1=±8g, 2=±4g, 3=±2g
#define ACCEL_RANGE_16G     0x00
#define ACCEL_RANGE_8G      0x40
#define ACCEL_RANGE_4G      0x80
#define ACCEL_RANGE_2G      0xC0

// Default settings
#ifndef ACCEL_DEFAULT_RATE
    #define ACCEL_DEFAULT_RATE  (ACCEL_RATE_100 | ACCEL_RANGE_8G)   // ACCEL_RATE_LOW_POWER
#endif
#ifndef ACCEL_DEFAULT_WATERMARK
    #define ACCEL_DEFAULT_WATERMARK     25  // Define default watermark (up to 31)
#endif
#ifndef ACCEL_HIGH_SPEED_WATERMARK
    #define ACCEL_HIGH_SPEED_WATERMARK  10  // Define high speed watermark (up to 31)
#endif

//==============================================================================
// Types

//==============================================================================
// External variables

//==============================================================================
// Global functions

#endif  /* ndef __adxlgui_regs_H__ */
