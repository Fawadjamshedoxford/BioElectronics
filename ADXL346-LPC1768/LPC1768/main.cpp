#include "mbed.h"
#include "rtos.h"
#include "ADXL345sb.h"

#define SLEEP_TIME      5 // (msec) // 50
#define CHAR_W          87
#define CHAR_R          82

#define VER             "1.027"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
ADXL345sb accel(p5, p6, p7, p8); // mosi, miso, sclk, ncs
Serial pc(USBTX, USBRX);         // tx, rx
InterruptIn adxl_int1(p9);
InterruptIn adxl_int2(p10);
DigitalOut dbg(p11);
AnalogOut aout(p18);

char inBuf[80];
int chCnt = 0;
int printInBuf = 0;
int command_decoder(void);
int source = 0;
char getXYZdata = 0;
unsigned int sleepTimeMsec = SLEEP_TIME;

void tapint() {
    // dbg = 1;
    source = accel.adxlread(INT_SOURCE);
    pc.printf("r11,%X\r\n",source);
    // dbg = 0;
}

void inactint() {
    // dbg = 1;
    source = accel.adxlread(INT_SOURCE);
    pc.printf("r12,%X\r\n",source);
    // dbg = 0;
}

int main() {
    // float readings[3];
    unsigned int readings[6], pwrcreg;
    short int xyz[3];
    int device_id = 0;
    int accelRate = ACCEL_DEFAULT_RATE;
    int accelRange = 3; // +/-16g
    int accelFrequency;
    char c;
    float aouttgl;

    accelFrequency = 3200 / (1 << (15-(accelRate & 0x0f)));
    pwrcreg = 1 << LINK_BIT;
    
    pc.printf("rFF,LPC1768 V.%s\r\n",VER);
    device_id = accel.adxlread(DEVID);
    pc.printf("rFE,device ID: %d\r\n",device_id);
    pc.printf("rFE,ADXL sampling freq: %d\r\n",accelFrequency);
    pc.printf("rFE,SPI freq: %d\r\n",ACCEL_SPI_FREQ);
    pc.printf("rFE,LPC1768 Sleep time: %d\r\n",sleepTimeMsec);
    if(device_id != ADXL346_ID) {
        pc.printf("rFE,ERROR: Incorrect device ID. Exiting...\r\n");
        return(1);
    }
    adxl_int1.mode(PullUp);
    adxl_int2.mode(PullUp);
    adxl_int1.rise(&tapint);    // attach the address of the tapint function to the rising edge
    adxl_int2.rise(&inactint);  // attach the address of the inactint function to the rising edge
	pc.printf("rFE,MZ setup\r\n");
	accelRange = 2; // +/-8g
	accel.adxlwrite(INT_ENABLE,     0x00);  // Disable all interrupts
	accel.adxlwrite(THRESH_TAP,     0x20);  // Tap Threshold: 62.5 mg/LSB (0xff = +16 g) [0x20 = 32 = 2 g]
	accel.adxlwrite(OFSX,           0x00);  // Xaxis Offset: signed 8-bit offset of 15.6 mg/LSB [0x00 = 0 g]
	accel.adxlwrite(OFSY,           0x00);  // Yaxis Offset: signed 8-bit offset of 15.6 mg/LSB [0x00 = 0 g]
	accel.adxlwrite(OFSZ,           0x00);  // Zaxis Offset: signed 8-bit offset of 15.6 mg/LSB [0x00 = 0 g]
	accel.adxlwrite(DUR,            0x20);  // Tap Duration: 625 us/LSB [32 = 20ms] <Maximum duration of a tap above threshold>
	accel.adxlwrite(LATENT,         0xA0);  // Tap Latency: 1.25 ms/LSB [0xa0 = 200ms] <Minimum time between taps>
	accel.adxlwrite(WINDOW,         160);   // Tap Window: 1.25 ms/LSB [0xa0 = 200 ms] <Time after latency to detect second tap>
	accel.adxlwrite(THRESH_ACT,     12);    // Activity Threshld: 62.5 mg/LSB  [0x0C = 0.75g]
	accel.adxlwrite(THRESH_INACT,   12);    // Inactivity Threshold: 62.5 mg/LSB [0x0C = 0.75g]
	accel.adxlwrite(TIME_INACT,     5);     // Inactivity Time: 1 sec/LSB [0x05 = 5s]
	accel.adxlwrite(ACT_INACT_CTL,  0x02);  // Axis Enable CTRL for activity inactivity detection. [0 = DC coupled, 1 = Y-axis inactivity detection ON]
	accel.adxlwrite(THRESH_FF,      0x00);  // FreeFall Threshold 62.5 mg/LSB (300 - 600 mg recommended) [0x00 = 0 g]
	accel.adxlwrite(TIME_FF,        0x00);  // FreeFall Time 5 ms/LSB (100 - 350 ms recommended) [0x00 = 0 ms]
	accel.adxlwrite(TAP_AXES,       0x11);  // Axis control tap/dbl- tap,  improved tap,  (b2=X,b1=Y,b0=Z) [0x11 Improved tap ON, Z tap detection ON]
	accel.adxlwrite(BW_RATE,        0x10 | (accelRate & 0x1f));  // [0x1a = LOW_POWER mode on, 100 Hz]
	accel.adxlwrite(POWER_CTL,      pwrcreg);   // Link bit on, Measure OFF
	accel.adxlwrite(INT_MAP,        0x08);  // Interrupt map, 0 = INT1 pin, 1 = INT2 pin [0x08 = All interrupts go to INT1 pin except inactivity interrupt]
	accel.adxlwrite(DATA_FORMAT,    accelRange); // Data format, Self test, SPI format, Invert, Full resolution, Justify and Range [0x02 = 10-bit resolution, right justify with sign extension, +/- 8 g range] 
	accel.adxlwrite(FIFO_CTL,       ACCEL_DEFAULT_WATERMARK);    // FIFO Control [0x19 = FIFO bypassed, trigger linked to INT1, 0x19 (25) watermark (N/A))
	accel.adxlwrite(INT_ENABLE,     0x48);  // Enable SINGLE_TAP and INACTIVITY interrupts
    aouttgl = 1.0f;
    while (true) {
        led1 = !led1;
        dbg = !dbg;
        Thread::wait(sleepTimeMsec);
        if(getXYZdata) {
            accel.adxlreadXYZ(readings);
            
            xyz[0] = (short) ((readings[1]<<8)|readings[0]);
            xyz[1] = (short) ((readings[3]<<8)|readings[2]);
            xyz[2] = (short) ((readings[5]<<8)|readings[4]);
            aout.write(((float) (1.5*xyz[2]+512))/1024.0);
            pc.printf("r10,   %+6d,  %+6d,  %+6d\r\n",xyz[0],xyz[1],xyz[2]);
        } else {
            aout.write(aouttgl);
            aouttgl = 1.0f - aouttgl;
        }
        if(pc.readable()) {
            c = pc.getc();
            inBuf[chCnt++] = c;
            if(chCnt > 79) {
                pc.printf("rFE,Error command too long! Discarded\r\n");
                chCnt = 0;
            } else {
                if(c == 13) {
                    inBuf[chCnt] = 0;
                    if(command_decoder() > 0) {
                        pc.printf("rFE,NAK Error invalid command %s\r\n",inBuf);
                    } else {
                        pc.printf("rFE,ACK Command %s sent\r\n",inBuf);
                    }
                    chCnt = 0;
                }
            }
        }
    }
}

int command_decoder(void)
{
    char *pch;
    char rw;
    int reg = 0;
    int val = 0;
    int res = 0;
    unsigned int pwrctrl;

    pch = strtok(inBuf, ",");
    rw = *pch;
    pch = strtok(NULL, ",");
    sscanf(pch, "%X", &reg);        
    pch = strtok(NULL, ",");
    if(pch != NULL) {
        sscanf(pch, "%X", &val);
    }
    switch(reg) {
        case 0xFF:
            NVIC_SystemReset();
            break;
        case 0x10:
            if(rw == CHAR_W) {
                getXYZdata = val;
                pwrctrl = accel.adxlread(POWER_CTL);
                if(val > 0) {
                    accel.adxlwrite(POWER_CTL,(pwrctrl & 0xF7) | 0x08);
                } else {
                    accel.adxlwrite(POWER_CTL,pwrctrl & 0xF7);
                }
            }
            break;
        case 0x13:
            if(rw == CHAR_W) {
                if(val > 0) {
                    sleepTimeMsec = val;
                }
            } else if(rw == CHAR_R) {
                pc.printf("r%02X,%X\r\n",reg,sleepTimeMsec);
            }
            break;
        default:
            if((reg > 0x1C) & (reg < 0x3D)) {
                if(rw == CHAR_W) {
                    accel.adxlwrite(reg,val);
                } else {
                    val = accel.adxlread(reg);
                    pc.printf("r%02X,%X\r\n",reg,val);
                }
            } else {
                res = 1;
            }
    }
    return(res);
}