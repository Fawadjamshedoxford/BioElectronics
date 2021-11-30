#ifndef _ICLGUI_FKT_H_
#define _ICLGUI_FKT_H_


#define ACK 0x06
#define NAK 0x15
#define CR 0x0d
#define LF 0x0a
#define CH_W 87
#define CH_R 82


#define DATA_NONE 	0
#define DATA_REAL	2
#define DATA_IMAG	4
#define DATA_MAG	32
#define DATA_FREQ	128
#define NOISE_REAL	8
#define NOISE_IMAG	16
#define NOISE_MAG	64
#define DATA_RE_FILT 256
#define DATA_IM_FILT 512


#define ADDR_DATA_REAL		0xF00
#define ADDR_DATA_IMAG		0xF01
#define ADDR_DATA_MAG		0xF04
#define ADDR_DATA_FREQ		0xF06
#define ADDR_NOISE_REAL		0xF02
#define ADDR_NOISE_IMAG		0xF03
#define ADDR_NOISE_MAG		0xF05
#define ADDR_REAL_FILT		0xF07
#define ADDR_IMAG_FILT		0xF08

#define RIGHT_SCALE_FACTOR	0.015625f	// 8g/512 for 10-bit resolution

#define MIN_GRAPH_SCALE 0
#define MAX_GRAPH_SCALE 30

#define THRSTAP		0
#define THRSACT		1
#define THRSINA		2

#define INT_NEG_MASK	~((1 << 8) - 1)
typedef int (CVICALLBACK *myCtrlCallbackPtr) (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

extern char title[];
extern int comstatus;
extern int comport;
extern char outbuf[100];
extern int panelHandle, panelCOM;
extern volatile int result;
extern int baudrate;
extern char write_buf[];
extern char auto_read;
extern int src, dest, flag, addr;
extern int mess_file;
extern int event_file;
extern int config_file;
extern char cmd_buf[];
extern char manual_read;
extern char messfilename[];
extern char eventfilename[];
extern volatile float rightScaleFactor;
extern volatile char x_act_en;
extern volatile char y_act_en;
extern volatile char z_act_en;
extern volatile char x_inact_en;
extern volatile char y_inact_en;
extern volatile char z_inact_en;
extern volatile char isActive;
extern volatile char isInactive;
extern volatile char softActInactDetect;
extern int update_regs_array[];
extern int ctrl_regs_array[];
extern float dead_zone_array[];
extern myCtrlCallbackPtr callback_array[];
extern int divisor_bw_array[];
extern volatile unsigned int regs_updated;
extern volatile unsigned int cbfrdy;
extern volatile unsigned int timeoutcnt;
extern volatile float thresholds[];
extern volatile unsigned int showThrsInd;

void show_help(void);
void bye(void);
void CVICALLBACK ComCallback (int eventMask,void *callbackData);
void dispatch_data(int data_mode, int data);
void scan4coms(void);
void parse_accel_data(void);
void parse_batt_data(void);
int selectStripchart(void);
int decode_adxl_data (char *rx_buf, float *graph_buffer);
void command_decoder(void);
void update_reg(void);
void log_regs(void);
void display_source(int val);
int store_config_registers(void);
int load_config_registers(void);
void check_act_inact(float *accel_xyz);
void turn_stim_prog_1_on(void);
void turn_stim_prog_2_on(void);

#endif
