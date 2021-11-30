#include "ADXL345sb.h"

ADXL345sb::ADXL345sb(PinName mosi, PinName miso, PinName sck, PinName cs) : spi_(mosi, miso, sck), nCS_(cs) {

    nCS_ = 1;
    
    spi_.frequency(ACCEL_SPI_FREQ);
    spi_.format(8,3);

    wait_us(500);

}

int ADXL345sb::adxlread(int address){
    int d;
    //address MSB must be one as it's a read
    address = address | 0x80;
    nCS_ = 0;
    wait_us(ACCEL_DELAY_MICROSECONDS);
    spi_.write(address);
    d = spi_.write(0x00);
    wait_us(ACCEL_DELAY_MICROSECONDS);
    nCS_ = 1;   
    return d;
}
void ADXL345sb::adxlmultibyteread(int address, int numOfBytes, unsigned char *buf){

    //address MSB must be one as it's a read
    address = address | 0xC0;
    nCS_ = 0;
    wait_us(ACCEL_DELAY_MICROSECONDS);
    spi_.write(address);
    for(int i = 0; i < numOfBytes; i++)
        buf[i] = spi_.write(0x00);
    wait_us(ACCEL_DELAY_MICROSECONDS);
    nCS_ = 1;   
}
// void ADXL345sb::adxlreadXYZ(float *buf){
void ADXL345sb::adxlreadXYZ(unsigned int *buf){
     //address MSB must be one as it's a read
    int address = DATAX0 | 0xC0;
    //set up response array and temp storeage vars
    unsigned char response[6];
    int16_t x = 0, y = 0, z = 0;    //this need to be in a 16bit int as the adxl is 10 bit res with MSB signed
    
    nCS_ = 0;
    wait_us(ACCEL_DELAY_MICROSECONDS);
    spi_.write(address);
    for(int i = 0; i < 6; i++)
        response[i] = spi_.write(0x00);
    wait_us(ACCEL_DELAY_MICROSECONDS);
    nCS_ = 1;
    
    x = (response[1]<<8)|response[0];
    y = (response[3]<<8)|response[2];
    z = (response[5]<<8)|response[4];
    
    // buf[0] = x; // * 0.0078;
    // buf[1] = y; // * 0.0078;
    // buf[2] = z; // * 0.0078;
    buf[0] = response[0];
    buf[1] = response[1];
    buf[2] = response[2];
    buf[3] = response[3];
    buf[4] = response[4];
    buf[5] = response[5];
}
void ADXL345sb::adxlwrite(int address, int data){
    nCS_ = 0;  
    wait_us(ACCEL_DELAY_MICROSECONDS);
    
    spi_.write(address);
    spi_.write(data);
    
    wait_us(ACCEL_DELAY_MICROSECONDS);
    nCS_ = 1;
}