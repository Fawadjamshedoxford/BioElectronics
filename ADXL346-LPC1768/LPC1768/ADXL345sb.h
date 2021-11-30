#ifndef ADXL345sb_H
#define ADXL345sb_H

#include "mbed.h"

//register codes
#define DEVID           0x00
#define THRESH_TAP      0x1D
#define OFSX            0x1E
#define OFSY            0x1F
#define OFSZ            0x20
#define DUR             0x21
#define LATENT          0x22
#define WINDOW          0x23
#define THRESH_ACT      0x24
#define THRESH_INACT    0x25
#define TIME_INACT      0x26
#define ACT_INACT_CTL   0x27
#define THRESH_FF       0x28
#define TIME_FF         0x29
#define TAP_AXES        0x2A
#define ACT_TAP_STATUS  0x2B
#define BW_RATE         0x2C
#define POWER_CTL       0x2D
#define INT_ENABLE      0x2E
#define INT_MAP         0x2F
#define INT_SOURCE      0x30
#define DATA_FORMAT     0x31
#define DATAX0          0x32
#define DATAX1          0x33
#define DATAY0          0x34
#define DATAY1          0x35
#define DATAZ0          0x36
#define DATAZ1          0x37
#define FIFO_CTL        0x38
#define FIFO_STATUS     0x39
#define TAP_SIGN        0x3A
#define ORIENT_CONF     0x3B
#define ORIENT          0x3C

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

#define ACCEL_DELAY_MICROSECONDS    1
#define ACCEL_SPI_FREQ              1000000 // 750000

#define ADXL346_ID  0xE6
#define LINK_BIT    5

class ADXL345sb{

public:
    ADXL345sb(PinName mosi, PinName miso, PinName sck, PinName cs);
    int adxlread(int address);
    void adxlmultibyteread(int address, int numOfBytes, unsigned char *buf);
    void adxlreadXYZ(unsigned int *buf); // void adxlreadXYZ(float *buf);
    void adxlwrite(int address, int data);

private:
    SPI        spi_;
    DigitalOut nCS_;
};
#endif