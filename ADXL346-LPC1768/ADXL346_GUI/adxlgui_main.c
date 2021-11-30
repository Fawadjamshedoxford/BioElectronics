#include <utility.h>
#include <formatio.h>
#include "adxlgui_uir.h"
#include "adxlgui_com.h"
#include "adxlgui_regs.h"
#include <ansi_c.h>
#include <rs232.h>
#include <cvirte.h>		
#include <userint.h>
#include <math.h>

/********************************************************************************************/
/* Prototypes                                                                               */
/********************************************************************************************/

#define YES 1
#define NO 0
#define ON 1
#define OFF 0



#define COM_SRC	  	0
#define COM_DEST	1
#define COM_FLAG	2
#define COM_ADDR	3
#define COM_DATA	4
#define COM_TERM	5

// COM Port Parameters
#define COMMAXREAD  1000   // Maximum Byte read from COM port

#define MAXBUF      1024

#define TOP_OFFSET	68

char title[]= {"ADXL346 GUI V.1.015"};

int panelHandle, panelCOM;
int comstatus = 0;
int comport=-1;
char outbuf[100];
char cmd_buf[50000];
volatile float rightScaleFactor = RIGHT_SCALE_FACTOR;
volatile char x_act_en = 0;
volatile char y_act_en = 0;
volatile char z_act_en = 0;
volatile char x_inact_en = 0;
volatile char y_inact_en = 0;
volatile char z_inact_en = 0;
volatile char isActive = 0;
volatile char isInactive = 0;
volatile char softActInactDetect = 0;
volatile unsigned int regs_updated = 0;
volatile unsigned int cbfrdy;
volatile unsigned int timeoutcnt = 0;
volatile float thresholds[3];
volatile unsigned int showThrsInd = 0;

char write_buf[MAXBUF];
char messfilename[MAX_PATHNAME_LEN];
char eventfilename[MAX_PATHNAME_LEN];
char configfilename[MAX_PATHNAME_LEN];
int mess_file = -1;
int event_file = -1;
int config_file = -1;
char manual_read = 0; 
char auto_read = -1;

/* ascending addr order */
int update_regs_array[] = {ADDR_THRESH_TAP,ADDR_OFSX,ADDR_OFSY,ADDR_OFSZ,ADDR_DUR,ADDR_LATENT,ADDR_WINDOW,ADDR_THRESH_ACT,ADDR_THRESH_INACT,\
						   ADDR_TIME_INACT,ADDR_ACT_INACT_CTL,ADDR_TAP_AXES,ADDR_BW_RATE,ADDR_POWER_CTL,ADDR_INT_ENABLE,ADDR_INT_MAP,\
						   ADDR_DATA_FORMAT,ADDR_ORIENT_CONF,0};
int ctrl_regs_array[] = {MAIN_NUM_THRESH_TAP,MAIN_NUM_X_OFFSET,MAIN_NUM_Y_OFFSET,MAIN_NUM_Z_OFFSET,MAIN_NUM_DUR,MAIN_NUM_Latent,MAIN_NUM_Window,\
						 MAIN_NUM_THRESH_ACT,MAIN_NUM_THRESH_INACT,MAIN_NUM_TIME_INACT,MAIN_NUM_ACT_INACT_CTL,MAIN_NUM_TAP_AXES,MAIN_NUM_BW_RATE, \
						 MAIN_NUM_POWER_CTL,MAIN_NUM_INT_ENABLE,MAIN_NUM_INT_MAP,MAIN_NUM_DATA_FORMAT,MAIN_NUM_ORIENT_CONFIG,0};

myCtrlCallbackPtr callback_array[] = {CB_THRESH_TAP,CB_DUR,CB_Latent,CB_Window,CB_TAP_AXES,CB_THRESH_ACT,CB_THRESH_INACT,CB_TIME_INACT,CB_ACT_INACT_CTL,\
									 CB_ORIENT_CONFIG,CB_INT_ENABLE,CB_INT_MAP,CB_X_OFFSET,CB_Y_OFFSET,CB_Z_OFFSET,CB_DATA_FORMAT,CB_BW_RATE,CB_POWER_CTL};
float dead_zone_array[] = {5.1,10.2,15.2,20.4,25.5,30.8,36.1,41.4};
int divisor_bw_array[] = {9,22,50,100,200,400,800,1600};
 int src, dest, flag, addr;
int baudrate=-1;
int openCloseVal=-1;
double numericSlideVal = 0;

int smlscreen = 0;

int main (int argc, char *argv[])
{
  int menu_handle;

	// initialise GUI
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, "adxlgui_uir.uir", MAIN)) < 0)
		return -1;
	if ((panelCOM = LoadPanel (0, "adxlgui_uir.uir", SELECT_COM)) < 0)
			return -1;
	DisplayPanel (panelHandle);
	SetCtrlAttribute (panelHandle,MAIN_STRIPCHART,ATTR_ACTIVE_YAXIS,VAL_RIGHT_YAXIS);
	SetCtrlAttribute (panelHandle,MAIN_STRIPCHART,ATTR_YLABEL_VISIBLE,0);
	SetCtrlAttribute (panelHandle,MAIN_STRIPCHART,ATTR_ACTIVE_YAXIS,VAL_LEFT_YAXIS);
	SetCtrlAttribute (panelHandle,MAIN_STRIPCHART,ATTR_YLABEL_VISIBLE,1);

	menu_handle = GetPanelMenuBar(panelHandle);
	RunUserInterface ();
	if(mess_file > 0) 
		CloseFile (mess_file);
	if(event_file > 0) 
		CloseFile (event_file);
	DiscardPanel (panelHandle);
	DiscardPanel(panelCOM);
	return 0;
}


/* Menu bar: File - Exit callback */
void CVICALLBACK CB_EXIT (int menuBar, int menuItem, void *callbackData,int panel)
{
	bye();
}

/* COM Port connected callback */
void CVICALLBACK ComCallback (int eventMask, void *callbackData)
{
	static int com_data_state = COM_FLAG;
	int inbyte;
	static int count;	// number of bytes read
	int inque;
	int a;

	while ((inque=GetInQLen(comport))>0)
	{
		inbyte =ComRdByte(comport);

		// Command decoder
		switch (com_data_state)
		{
			case COM_SRC:
				if (inbyte > 'A')
					src = inbyte + 10 - 'A';
				else
					src = inbyte - '0';
				com_data_state = COM_DEST;
				break;
			case COM_DEST:
				if (inbyte > 'A')
					dest = inbyte + 10 - 'A';
				else
					dest = inbyte - '0';
				com_data_state = COM_FLAG;
				break;
			case COM_FLAG:
				if (inbyte == 'r')
				{
					flag = inbyte;
					count = 0;
					com_data_state = COM_ADDR;
				}
				else
				{
					if ((inbyte == ACK) || (inbyte == NAK))
					{
						result = inbyte;
						com_data_state = COM_TERM;
					}
					else
					{
						com_data_state = COM_FLAG;	// reset statemachine
					}
				}
				break;
			case COM_ADDR:
				cmd_buf[count++]=(char)inbyte;
				if (count == 2)
				{
					cmd_buf[count] = '\0';
					addr = strtol(cmd_buf, NULL, 16);
					com_data_state = COM_DATA;
					count = 0;
				}
				break;
			case COM_DATA:
				if (inbyte == CR)
				{
					com_data_state = COM_FLAG;
					cmd_buf[count]='\0';
					command_decoder();
				}
				else
				{
					if (count < 50000)
					{
						cmd_buf[count++]= (char)inbyte;
					}
					else
					{
						a = 1;
					}
				}
				break;
			case COM_TERM:
				com_data_state = COM_FLAG;		// this state is for the cases who don't know how to handle serial communication single byte wise and need a terminating character
				break;
			default:
				break;
		}
	}
}


/* Pop-up COM SoC port window's Open/Close toggle button callback */
int CVICALLBACK CB_COM_Open_Close_Port (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	int err;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelCOM, SELECT_COM_COM_OPEN_CLOSE_PORT, &openCloseVal);
			GetCtrlVal (panelCOM, SELECT_COM_COM_SELECT, &comport);
			GetCtrlVal (panelCOM, SELECT_COM_COM_BAUD, &baudrate);
			if (openCloseVal>0)
			{
				// Open the port
				err = OpenComConfig (comport, "", baudrate, 0, 8, 1, 2000, 100);
				if (err<0)
				{
					sprintf(outbuf,"Can't open COM %d! \n  Select another port.",comport);
					MessagePopup ("COM Port Initialisation", outbuf);
					openCloseVal = 0;
					SetCtrlVal (panelCOM, SELECT_COM_COM_OPEN_CLOSE_PORT, openCloseVal);
					comport = 0;
					return 0;
				}
				else
				{
					SetCtrlAttribute (panelCOM, SELECT_COM_COM_SELECT, ATTR_CTRL_MODE, VAL_INDICATOR);
					SetCtrlAttribute (panelCOM, SELECT_COM_COM_SELECT, ATTR_DIMMED, 1);
					
					FlushInQ (comport);
					// Install ComCallback (waiting for CR)
					err = InstallComCallback (comport, LWRS_RXCHAR, 0, 0, ComCallback, 0);
					if (err<0)
						MessagePopup ("Error", "Can't Install Com Callback");
					// Empty COM x Input Queue
					FlushInQ (comport);
					comstatus = 1;
					
					FlushInQ (comport);				// Ignore all data as response of the CR
					ComWrt(comport,"W,FF,01\r",8);	// reset LPC1768
				}
			}
			else
			{
				// Close the port
				if (comstatus)
				{
					CloseCom(comport);
					comstatus = 0;
				}
				SetCtrlAttribute (panelCOM, SELECT_COM_COM_SELECT, ATTR_CTRL_MODE, VAL_HOT);
				SetCtrlAttribute (panelCOM, SELECT_COM_COM_SELECT, ATTR_DIMMED, 0);
				comport = 0;
			}
			break;
	}
	return 0;
}


/* Pop-up COM port window's Close button callback */
int CVICALLBACK cb_com_close (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			HidePanel(panelCOM);
			break;
	}
	return 0;
}


/* Menu bar: Options - COM Port callback */
void CVICALLBACK CB_CFG_COM (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	scan4coms();
	DisplayPanel (panelCOM);
	SetCtrlVal (panelCOM, SELECT_COM_COM_SELECT, comport);
}


/* Main panel callback */
int CVICALLBACK CB_Main (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			bye();
			break;
	}
	return 0;
}


/* THRESH_TAP input callback */
int CVICALLBACK CB_THRESH_TAP (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_THRESH_TAP,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_THRESH_TAP_HEX,value);
			thresholds[THRSTAP] = value*62.5e-3;
			SetCtrlVal (panelHandle,MAIN_NUM_THRESH_TAP_CONV,thresholds[THRSTAP]);
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_THRESH_TAP,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_THRESH_TAP_CONV (int panel, int control, int event,
									void *callbackData, int eventData1, int eventData2)
{
	float value;
	unsigned int intval;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_THRESH_TAP_CONV,&value);
			if(value > 15.9375) {
				value = 15.9375;
				SetCtrlVal (panelHandle,MAIN_NUM_THRESH_TAP_CONV,value);
			}
			intval = (unsigned int) floor (value/62.5e-3);
			thresholds[THRSTAP] = intval*62.5e-3;
			SetCtrlVal (panelHandle,MAIN_NUM_THRESH_TAP_CONV,thresholds[THRSTAP]);
			SetCtrlVal (panelHandle,MAIN_NUM_THRESH_TAP,intval);
			SetCtrlVal (panelHandle,MAIN_NUM_THRESH_TAP_HEX,intval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_THRESH_TAP,intval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


/* DUR input callback */
int CVICALLBACK CB_DUR (int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_DUR,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_DUR_HEX,value);
			SetCtrlVal (panelHandle,MAIN_NUM_DUR_CONV,value*625e-6);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_DUR,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_DUR_CONV (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	float value;
	unsigned int intval;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_DUR_CONV,&value);
			if(value > 0.159375) {
				value = 0.159375;
				SetCtrlVal (panelHandle,MAIN_NUM_DUR_CONV,value);
			}
			intval = (unsigned int) floor (value/625e-6);
			SetCtrlVal (panelHandle,MAIN_NUM_DUR_CONV,intval*625e-6);
			SetCtrlVal (panelHandle,MAIN_NUM_DUR,intval);
			SetCtrlVal (panelHandle,MAIN_NUM_DUR_HEX,intval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_DUR,intval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


/* Latent callback */
int CVICALLBACK CB_Latent (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_Latent,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_Latent_HEX,value);
			SetCtrlVal (panelHandle,MAIN_NUM_Latent_CONV,value*1.25e-3);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_LATENT,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_Latent_CONV (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	float value;
	unsigned int intval;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_Latent_CONV,&value);
			if(value > 0.31875) {
				value = 0.31875;
				SetCtrlVal (panelHandle,MAIN_NUM_Latent_CONV,value);
			}
			intval = (unsigned int) floor (value/1.25e-3);
			SetCtrlVal (panelHandle,MAIN_NUM_Latent_CONV,intval*1.25e-3);
			SetCtrlVal (panelHandle,MAIN_NUM_Latent,intval);
			SetCtrlVal (panelHandle,MAIN_NUM_Latent_HEX,intval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_LATENT,intval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


/* Window callback */
int CVICALLBACK CB_Window (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_Window,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_Window_HEX,value);
			SetCtrlVal (panelHandle,MAIN_NUM_Window_CONV,value*1.25e-3);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_WINDOW,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_Window_CONV (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	float value;
	unsigned int intval;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_Window_CONV,&value);
			if(value > 0.31875) {
				value = 0.31875;
				SetCtrlVal (panelHandle,MAIN_NUM_Window_CONV,value);
			}
			intval = (unsigned int) floor (value/1.25e-3);
			SetCtrlVal (panelHandle,MAIN_NUM_Window_CONV,intval*1.25e-3);
			SetCtrlVal (panelHandle,MAIN_NUM_Window,intval);
			SetCtrlVal (panelHandle,MAIN_NUM_Window_HEX,intval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_WINDOW,intval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_TAP_AXES (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES_HEX,value);
			
			SetCtrlVal (panelHandle,MAIN_IMPROVE_TAP_EN,(value & 0x10) >> 4);
			SetCtrlVal (panelHandle,MAIN_SUPPRESS_EN,(value & 0x08) >> 3);
			
			SetCtrlVal (panelHandle,MAIN_X_TAP_EN,(value & 0x04) >> 2);
			SetCtrlVal (panelHandle,MAIN_Y_TAP_EN,(value & 0x02) >> 1);
			SetCtrlVal (panelHandle,MAIN_Z_TAP_EN,value & 0x01);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_TAP_AXES,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_IMPROVE_TAP_EN (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_IMPROVE_TAP_EN,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES,&oldval);
			newval = oldval & 0xEF;
			if(value) {
				newval |= 0x10;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_TAP_AXES,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_SUPPRESS_EN (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_SUPPRESS_EN,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES,&oldval);
			newval = oldval & 0xF7;
			if(value) {
				newval |= 0x08;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_TAP_AXES,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_X_TAP_EN (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_X_TAP_EN,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES,&oldval);
			newval = oldval & 0xFB;
			if(value) {
				newval |= 0x04;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_TAP_AXES,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_Y_TAP_EN (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_Y_TAP_EN,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES,&oldval);
			newval = oldval & 0xFD;
			if(value) {
				newval |= 0x02;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_TAP_AXES,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_Z_TAP_EN (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_Z_TAP_EN,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES,&oldval);
			newval = oldval & 0xFE;
			if(value) {
				newval |= 0x01;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_TAP_AXES,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_THRESH_ACT (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_THRESH_ACT,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_THRESH_ACT_HEX,value);
			thresholds[THRSACT] = value*62.5e-3;
			SetCtrlVal (panelHandle,MAIN_NUM_THRESH_ACT_CONV,thresholds[THRSACT]);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_THRESH_ACT,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_THRESH_ACT_CONV (int panel, int control, int event,
									void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_THRESH_ACT_CONV,&thresholds[THRSACT]);
			if(thresholds[THRSACT] > 15.9375) {
				thresholds[THRSACT] = 15.9375;
			}
			value = (unsigned int) floor (thresholds[THRSACT]/62.5e-3);
			thresholds[THRSACT] = value*62.5e-3;
			SetCtrlVal (panelHandle,MAIN_NUM_THRESH_ACT_CONV,thresholds[THRSACT]);
			SetCtrlVal (panelHandle,MAIN_NUM_THRESH_ACT,value);
			SetCtrlVal (panelHandle,MAIN_NUM_THRESH_ACT_HEX,value);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_THRESH_ACT,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_THRESH_INACT (int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_THRESH_INACT,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_THRESH_INACT_HEX,value);
			thresholds[THRSINA] = value*62.5e-3;
			SetCtrlVal (panelHandle,MAIN_NUM_THRESH_INACT_CONV,thresholds[THRSINA]);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_THRESH_INACT,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_THRESH_INACT_CONV (int panel, int control, int event,
									  void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_THRESH_INACT_CONV,&thresholds[THRSINA]);
			if(thresholds[THRSINA] > 15.9375) {
				thresholds[THRSINA] = 15.9375;
			}
			value = (unsigned int) floor (thresholds[THRSINA]/62.5e-3);
			thresholds[THRSINA] = value*62.5e-3;
			SetCtrlVal (panelHandle,MAIN_NUM_THRESH_INACT_CONV,thresholds[THRSINA]);
			SetCtrlVal (panelHandle,MAIN_NUM_THRESH_INACT,value);
			SetCtrlVal (panelHandle,MAIN_NUM_THRESH_INACT_HEX,value);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_THRESH_INACT,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_TIME_INACT (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_TIME_INACT,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_TIME_INACT_HEX,value);
			SetCtrlVal (panelHandle,MAIN_NUM_TIME_INACT_CONV,value);
			SetCtrlAttribute (panelHandle,MAIN_TIMER_INACTIVITY,ATTR_INTERVAL,(double) value);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_TIME_INACT,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_TIME_INACT_CONV (int panel, int control, int event,
									void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_TIME_INACT_CONV,&value);
			if(value > 255) {
				value = 255;
				SetCtrlVal (panelHandle,MAIN_NUM_TIME_INACT_CONV,value);
			}
			SetCtrlVal (panelHandle,MAIN_NUM_TIME_INACT,value);
			SetCtrlVal (panelHandle,MAIN_NUM_TIME_INACT_HEX,value);
			SetCtrlAttribute (panelHandle,MAIN_TIMER_INACTIVITY,ATTR_INTERVAL,(double) value);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_TIME_INACT,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_ACT_INACT_CTL (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL_HEX,value);
			
			SetCtrlVal (panelHandle,MAIN_ACT_AC_DC,(value & 0x80) >> 7);
			
			x_act_en = (value & 0x40) >> 6;
			SetCtrlVal (panelHandle,MAIN_ACT_X_EN,x_act_en);
			y_act_en = (value & 0x20) >> 5;
			SetCtrlVal (panelHandle,MAIN_ACT_Y_EN,y_act_en);
			z_act_en = (value & 0x10) >> 4;
			SetCtrlVal (panelHandle,MAIN_ACT_Z_EN,z_act_en);
			
			SetCtrlVal (panelHandle,MAIN_INACT_AC_DC,(value & 0x08) >> 3);
			
			x_inact_en = (value & 0x04) >> 2;
			SetCtrlVal (panelHandle,MAIN_INACT_X_EN,x_inact_en);
			y_inact_en = (value & 0x02) >> 1;
			SetCtrlVal (panelHandle,MAIN_INACT_Y_EN,y_inact_en);
			z_inact_en = value & 0x01;
			SetCtrlVal (panelHandle,MAIN_INACT_Z_EN,z_inact_en);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_ACT_INACT_CTL,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_ACT_AC_DC (int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_ACT_AC_DC,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,&oldval);
			newval = oldval & 0x7F;
			if(value) {
				newval |= 0x80;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_ACT_INACT_CTL,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_ACT_X_EN (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_ACT_X_EN,&value);
			x_act_en = value & 0x01;
			GetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,&oldval);
			newval = oldval & 0xBF;
			if(value) {
				newval |= 0x40;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_ACT_INACT_CTL,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_ACT_Y_EN (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_ACT_Y_EN,&value);
			y_act_en = value & 0x01;
			GetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,&oldval);
			newval = oldval & 0xDF;
			if(value) {
				newval |= 0x20;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_ACT_INACT_CTL,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_ACT_Z_EN (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_ACT_Z_EN,&value);
			z_act_en = value & 0x01;
			GetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,&oldval);
			newval = oldval & 0xEF;
			if(value) {
				newval |= 0x10;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_ACT_INACT_CTL,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_INACT_AC_DC (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_INACT_AC_DC,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,&oldval);
			newval = oldval & 0xF7;
			if(value) {
				newval |= 0x08;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_ACT_INACT_CTL,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_INACT_X_EN (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_INACT_X_EN,&value);
			x_inact_en = value & 0x01;
			GetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,&oldval);
			newval = oldval & 0xFB;
			if(value) {
				newval |= 0x04;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_ACT_INACT_CTL,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_INACT_Y_EN (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_INACT_Y_EN,&value);
			y_inact_en = value & 0x01;
			GetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,&oldval);
			newval = oldval & 0xFD;
			if(value) {
				newval |= 0x02;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_ACT_INACT_CTL,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_INACT_Z_EN (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_INACT_Z_EN,&value);
			z_inact_en = value & 0x01;
			GetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,&oldval);
			newval = oldval & 0xFE;
			if(value) {
				newval |= 0x01;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_ACT_INACT_CTL,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_ORIENT_CONFIG (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, bwval;
	float rate;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_ORIENT_CONFIG,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_ORIENT_CONFIG_HEX,value);
			SetCtrlVal (panelHandle,MAIN_INT_ORIENT,(value & 0x80) >> 7);
			SetCtrlVal (panelHandle,MAIN_RING_DEAD_ZONE,(value & 0x70) >> 4);
			SetCtrlVal (panelHandle,MAIN_INT_3D,(value & 0x08) >> 3);
			SetCtrlVal (panelHandle,MAIN_RING_ORIENT_DIV,value & 0x07);

			GetCtrlVal (panelHandle,MAIN_NUM_BW_RATE,&bwval);
    		rate = 3200 / (1 << (15-(bwval & 0x0F)));
			SetCtrlVal(panelHandle,MAIN_NUM_DIV_BW_HZ,rate/divisor_bw_array[value & 0x07]);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_ORIENT_CONF,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_DEAD_ZONE (int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_RING_DEAD_ZONE,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_ORIENT_CONFIG,&oldval);
			newval = (oldval & 0x8F) | ((value & 0x07) << 4);
			SetCtrlVal (panelHandle,MAIN_NUM_ORIENT_CONFIG,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_ORIENT_CONFIG_HEX,newval);

			sprintf(outbuf,"W,%02X,%02X\r",ADDR_ORIENT_CONF,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_ORIENT_DIV (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval, bwval;
	float rate;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_RING_ORIENT_DIV,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_ORIENT_CONFIG,&oldval);
			newval = (oldval & 0xF8) | (value & 0x07);
			SetCtrlVal (panelHandle,MAIN_NUM_ORIENT_CONFIG,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_ORIENT_CONFIG_HEX,newval);

			GetCtrlVal (panelHandle,MAIN_NUM_BW_RATE,&bwval);
    		rate = 3200 / (1 << (15-(bwval & 0x0F)));
			SetCtrlVal(panelHandle,MAIN_NUM_DIV_BW_HZ,rate/divisor_bw_array[value & 0x07]);

			sprintf(outbuf,"W,%02X,%02X\r",ADDR_ORIENT_CONF,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_INT_ORIENT (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_INT_ORIENT,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_ORIENT_CONFIG,&oldval);
			newval = oldval & 0x7F;
			if(value) {
				newval |= 0x80;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_ORIENT_CONFIG,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_ORIENT_CONFIG_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_ORIENT_CONF,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_INT_3D (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_INT_3D,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_ORIENT_CONFIG,&oldval);
			newval = oldval & 0xF7;
			if(value) {
				newval |= 0x08;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_ORIENT_CONFIG,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_ORIENT_CONFIG_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_ORIENT_CONF,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


/* INT_ENABLE callback */
int CVICALLBACK CB_INT_ENABLE (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE_HEX,value);
			
			SetCtrlVal (panelHandle,MAIN_DTA_RDY_INT,(value & 0x80) >> 7);
			SetCtrlVal (panelHandle,MAIN_SINGLE_TAP_INT,(value & 0x40) >> 6);
			SetCtrlVal (panelHandle,MAIN_DOUBLE_TAP_INT,(value & 0x20) >> 5);
			SetCtrlVal (panelHandle,MAIN_ACTIVITY_INT,(value & 0x10) >> 4);
			SetCtrlVal (panelHandle,MAIN_INACTIVITY_INT,(value & 0x08) >> 3);
			SetCtrlVal (panelHandle,MAIN_FREE_FALL_INT,(value & 0x04) >> 2);
			SetCtrlVal (panelHandle,MAIN_WATERMARK_INT,(value & 0x02) >> 1);
			SetCtrlVal (panelHandle,MAIN_OVR_ORIENT_INT,value & 0x01);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_ENABLE,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_DTA_RDY_INT (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_DTA_RDY_INT,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,&oldval);
			newval = oldval & 0x7F;
			if(value) {
				newval |= 0x80;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_ENABLE,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_SINGLE_TAP_INT (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_SINGLE_TAP_INT,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,&oldval);
			newval = oldval & 0xBF;
			if(value) {
				newval |= 0x40;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_ENABLE,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_DOUBLE_TAP_INT (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_DOUBLE_TAP_INT,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,&oldval);
			newval = oldval & 0xDF;
			if(value) {
				newval |= 0x20;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_ENABLE,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_ACTIVITY_INT (int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_ACTIVITY_INT,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,&oldval);
			newval = oldval & 0xEF;
			if(value) {
				newval |= 0x10;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_ENABLE,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_INACTIVITY_INT (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_INACTIVITY_INT,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,&oldval);
			newval = oldval & 0xF7;
			if(value) {
				newval |= 0x08;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_ENABLE,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_FREE_FALL_INT (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_FREE_FALL_INT,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,&oldval);
			newval = oldval & 0xFB;
			if(value) {
				newval |= 0x04;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_ENABLE,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_WATERMARK_INT (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_WATERMARK_INT,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,&oldval);
			newval = oldval & 0xFD;
			if(value) {
				newval |= 0x02;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_ENABLE,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_OVR_ORIENT_INT (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_OVR_ORIENT_INT,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,&oldval);
			newval = oldval & 0xFE;
			if(value) {
				newval |= 0x01;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_ENABLE,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_INT_MAP (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP_HEX,value);
			
			SetCtrlVal (panelHandle,MAIN_DTA_RDY_MAP,(value & 0x80) >> 7);
			SetCtrlVal (panelHandle,MAIN_SINGLE_TAP_MAP,(value & 0x40) >> 6);
			SetCtrlVal (panelHandle,MAIN_DOUBLE_TAP_MAP,(value & 0x20) >> 5);
			SetCtrlVal (panelHandle,MAIN_ACTIVITY_MAP,(value & 0x10) >> 4);
			SetCtrlVal (panelHandle,MAIN_INACTIVITY_MAP,(value & 0x08) >> 3);
			SetCtrlVal (panelHandle,MAIN_FREE_FALL_MAP,(value & 0x04) >> 2);
			SetCtrlVal (panelHandle,MAIN_WATERMARK_MAP,(value & 0x02) >> 1);
			SetCtrlVal (panelHandle,MAIN_OVR_ORIENT_MAP,value & 0x01);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_MAP,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_DTA_RDY_MAP (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_DTA_RDY_MAP,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,&oldval);
			newval = oldval & 0x7F;
			if(value) {
				newval |= 0x80;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_MAP,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_SINGLE_TAP_MAP (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_SINGLE_TAP_MAP,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,&oldval);
			newval = oldval & 0xBF;
			if(value) {
				newval |= 0x40;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_MAP,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_DOUBLE_TAP_MAP (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_DOUBLE_TAP_MAP,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,&oldval);
			newval = oldval & 0xDF;
			if(value) {
				newval |= 0x20;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_MAP,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_ACTIVITY_MAP (int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_ACTIVITY_MAP,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,&oldval);
			newval = oldval & 0xEF;
			if(value) {
				newval |= 0x10;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_MAP,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_INACTIVITY_MAP (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_INACTIVITY_MAP,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,&oldval);
			newval = oldval & 0xF7;
			if(value) {
				newval |= 0x08;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_MAP,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_FREE_FALL_MAP (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_FREE_FALL_MAP,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,&oldval);
			newval = oldval & 0xFB;
			if(value) {
				newval |= 0x04;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_MAP,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_WATERMARK_MAP (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_WATERMARK_MAP,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,&oldval);
			newval = oldval & 0xFD;
			if(value) {
				newval |= 0x02;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_MAP,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_OVR_ORIENT_MAP (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_OVR_ORIENT_MAP,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,&oldval);
			newval = oldval & 0xFE;
			if(value) {
				newval |= 0x01;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_INT_MAP,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_X_OFFSET (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;
	int sigval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_X_OFFSET,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_X_OFFSET_HEX,value);
			if(value & 0x80) {
				sigval = value | INT_NEG_MASK;
			} else {
				sigval = value;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_X_OFFSET_CONV,sigval*15.6e-3);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_OFSX,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_X_OFFSET_CONV (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	float value;
	unsigned int intval;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_X_OFFSET_CONV,&value);
			if(value > 1.9812) {
				value = 1.9812;
				SetCtrlVal (panelHandle,MAIN_NUM_X_OFFSET_CONV,value);
			} else if(value < -1.9968) {
				value = -1.9968;
				SetCtrlVal (panelHandle,MAIN_NUM_X_OFFSET_CONV,value);
			}
			value = floor (value/15.6e-3);
			intval = ((unsigned int) value) & 0xFF;
			SetCtrlVal (panelHandle,MAIN_NUM_X_OFFSET_CONV,value*15.6e-3);
			SetCtrlVal (panelHandle,MAIN_NUM_X_OFFSET,intval);
			SetCtrlVal (panelHandle,MAIN_NUM_X_OFFSET_HEX,intval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_OFSX,intval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_Y_OFFSET (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;
	int sigval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_Y_OFFSET,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_Y_OFFSET_HEX,value);
			if(value & 0x80) {
				sigval = value | INT_NEG_MASK;
			} else {
				sigval = value;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_Y_OFFSET_CONV,sigval*15.6e-3);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_OFSY,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_Y_OFFSET_CONV (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	float value;
	unsigned int intval;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_Y_OFFSET_CONV,&value);
			if(value > 1.9812) {
				value = 1.9812;
				SetCtrlVal (panelHandle,MAIN_NUM_Y_OFFSET_CONV,value);
			} else if(value < -1.9968) {
				value = -1.9968;
				SetCtrlVal (panelHandle,MAIN_NUM_Y_OFFSET_CONV,value);
			}
			value = floor (value/15.6e-3);
			intval = ((unsigned int) value) & 0xFF;
			SetCtrlVal (panelHandle,MAIN_NUM_Y_OFFSET_CONV,value*15.6e-3);
			SetCtrlVal (panelHandle,MAIN_NUM_Y_OFFSET,intval);
			SetCtrlVal (panelHandle,MAIN_NUM_Y_OFFSET_HEX,intval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_OFSY,intval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_Z_OFFSET (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;
	int sigval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_Z_OFFSET,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_Z_OFFSET_HEX,value);
			if(value & 0x80) {
				sigval = value | INT_NEG_MASK;
			} else {
				sigval = value;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_Z_OFFSET_CONV,sigval*15.6e-3);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_OFSZ,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_Z_OFFSET_CONV (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	float value;
	unsigned int intval;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_Z_OFFSET_CONV,&value);
			if(value > 1.9812) {
				value = 1.9812;
				SetCtrlVal (panelHandle,MAIN_NUM_Z_OFFSET_CONV,value);
			} else if(value < -1.9968) {
				value = -1.9968;
				SetCtrlVal (panelHandle,MAIN_NUM_Z_OFFSET_CONV,value);
			}
			value = floor (value/15.6e-3);
			intval = ((unsigned int) value) & 0xFF;
			SetCtrlVal (panelHandle,MAIN_NUM_Z_OFFSET_CONV,value*15.6e-3);
			SetCtrlVal (panelHandle,MAIN_NUM_Z_OFFSET,intval);
			SetCtrlVal (panelHandle,MAIN_NUM_Z_OFFSET_HEX,intval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_OFSZ,intval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_DATA_FORMAT (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, range;
	double value_min, value_max;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_DATA_FORMAT,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_DATA_FORMAT_HEX,value);
			SetCtrlVal (panelHandle,MAIN_RING_RANGE,value & 0x03);
			SetCtrlVal (panelHandle,MAIN_SELF_TEST_EN,(value & 0x80) >> 7);
			SetCtrlVal (panelHandle,MAIN_FULL_RES_EN,(value & 0x08) >> 3);
			
			range = 2 * pow(2,value & 0x03);
			rightScaleFactor = (float) range / 512.0;
			GetCtrlVal (panelHandle,MAIN_F_MIN,&value_min);
			GetCtrlVal (panelHandle,MAIN_F_MAX,&value_max);
			SetAxisScalingMode (panelHandle,MAIN_STRIPCHART,VAL_LEFT_YAXIS,VAL_MANUAL,value_min,value_max);
			SetAxisScalingMode (panelHandle,MAIN_STRIPCHART,VAL_RIGHT_YAXIS,VAL_MANUAL,value_min*rightScaleFactor,value_max*rightScaleFactor);

			sprintf(outbuf,"W,%02X,%02X\r",ADDR_DATA_FORMAT,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_RANGE (int panel, int control, int event,
						  void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval, range;
	double value_min, value_max;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_RING_RANGE,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_DATA_FORMAT,&oldval);
			newval = (oldval & 0xFC) | (value & 0x03);
			SetCtrlVal (panelHandle,MAIN_NUM_DATA_FORMAT,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_DATA_FORMAT_HEX,newval);

			range = 2 * pow(2,value & 0x03);
			rightScaleFactor = (float) range / 512.0;
			GetCtrlVal (panelHandle,MAIN_F_MIN,&value_min);
			GetCtrlVal (panelHandle,MAIN_F_MAX,&value_max);
			SetAxisScalingMode (panelHandle,MAIN_STRIPCHART,VAL_LEFT_YAXIS,VAL_MANUAL,value_min,value_max);
			SetAxisScalingMode (panelHandle,MAIN_STRIPCHART,VAL_RIGHT_YAXIS,VAL_MANUAL,value_min*rightScaleFactor,value_max*rightScaleFactor);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_DATA_FORMAT,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_SELF_TEST_EN (int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_SELF_TEST_EN,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_DATA_FORMAT,&oldval);
			newval = oldval & 0x7F;
			if(value) {
				newval |= 0x80;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_DATA_FORMAT,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_DATA_FORMAT_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_DATA_FORMAT,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_FULL_RES_EN (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_FULL_RES_EN,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_DATA_FORMAT,&oldval);
			newval = oldval & 0xF7;
			if(value) {
				newval |= 0x08;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_DATA_FORMAT,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_DATA_FORMAT_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_DATA_FORMAT,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_BW_RATE (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, bwdiv;
	float rate;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_BW_RATE,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_BW_RATE_HEX,value);
			SetCtrlVal (panelHandle,MAIN_LOW_POWER_EN,(value & 0x10) >> 4);
			SetCtrlVal (panelHandle,MAIN_RING_OUTPUT_RATE,value & 0x0F);
    		rate = 3200 / (1 << (15-(value & 0x0F)));
			GetCtrlVal(panelHandle,MAIN_RING_ORIENT_DIV,&bwdiv);
			SetCtrlVal(panelHandle,MAIN_NUM_DIV_BW_HZ,rate/divisor_bw_array[bwdiv & 0x07]);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_BW_RATE,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_OUTPUT_RATE (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval, bwdiv;
	float rate;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_RING_OUTPUT_RATE,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_BW_RATE,&oldval);
			newval = (oldval & 0xF0) | (value & 0x0F);
			SetCtrlVal (panelHandle,MAIN_NUM_BW_RATE,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_BW_RATE_HEX,newval);
    		rate = 3200 / (1 << (15-(value & 0x0f)));
			GetCtrlVal(panelHandle,MAIN_RING_ORIENT_DIV,&bwdiv);
			SetCtrlVal(panelHandle,MAIN_NUM_DIV_BW_HZ,rate/divisor_bw_array[bwdiv & 0x07]);

			sprintf(outbuf,"W,%02X,%02X\r",ADDR_BW_RATE,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_LOW_POWER_EN (int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_LOW_POWER_EN,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_BW_RATE,&oldval);
			newval = oldval & 0xEF;
			if(value) {
				newval |= 0x10;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_BW_RATE,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_BW_RATE_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_BW_RATE,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_POWER_CTL (int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL,&value);
			SetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL_HEX,value);
			SetCtrlVal (panelHandle,MAIN_RING_WAKEUP_FREQ,value & 0x03);
			SetCtrlVal (panelHandle,MAIN_LINK_BIT,(value & 0x20) >> 5);
			SetCtrlVal (panelHandle,MAIN_AUTO_SLEEP,(value & 0x10) >> 4);
			SetCtrlVal (panelHandle,MAIN_MEASURE_ON,(value & 0x08) >> 3);
			SetCtrlVal (panelHandle,MAIN_SLEEP,(value & 0x04) >> 2);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_POWER_CTL,value);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_WAKEUP_FREQ (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_RING_WAKEUP_FREQ,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL,&oldval);
			newval = (oldval & 0xFC) | (value & 0x03);
			SetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL_HEX,newval);

			sprintf(outbuf,"W,%02X,%02X\r",ADDR_POWER_CTL,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_LINK_BIT (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_LINK_BIT,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL,&oldval);
			newval = oldval & 0xDF;
			if(value) {
				newval |= 0x20;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_POWER_CTL,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_AUTO_SLEEP (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_AUTO_SLEEP,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL,&oldval);
			newval = oldval & 0xEF;
			if(value) {
				newval |= 0x10;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_POWER_CTL,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_MEASURE_ON (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_MEASURE_ON,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL,&oldval);
			newval = oldval & 0xF7;
			if(value) {
				newval |= 0x08;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_POWER_CTL,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_SLEEP (int panel, int control, int event,
						  void *callbackData, int eventData1, int eventData2)
{
	unsigned int value, oldval, newval;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_MEASURE_ON,&value);
			GetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL,&oldval);
			newval = oldval & 0xFB;
			if(value) {
				newval |= 0x04;
			}
			SetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL,newval);
			SetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL_HEX,newval);
			
			sprintf(outbuf,"W,%02X,%02X\r",ADDR_POWER_CTL,newval);
			ComWrt(comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


/* Left Y-Axis Min. F. and Max. F. inputs callback */
int CVICALLBACK cb_strip_left (int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	double value_min, value_max;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle, MAIN_F_MIN, &value_min);
			GetCtrlVal (panelHandle, MAIN_F_MAX, &value_max);
			SetAxisScalingMode (panelHandle, MAIN_STRIPCHART,VAL_LEFT_YAXIS, VAL_MANUAL, value_min,  value_max);
			SetAxisScalingMode (panelHandle, MAIN_STRIPCHART,VAL_RIGHT_YAXIS, VAL_MANUAL, value_min*rightScaleFactor,  value_max*rightScaleFactor);
			break;
	}
	return 0;
}


/* Log Filename text input callback */
int CVICALLBACK cb_logfilename (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			FileSelectPopup ("", "*.txt", "*.txt;*.log", "Select File", VAL_OK_BUTTON, 0, 0, 1, 1, messfilename);
			SetCtrlVal (panelHandle, MAIN_LOGFILENAME, messfilename);
			break;
		case EVENT_COMMIT:
			break;
	}
	return 0;
}


/* Logfile list callback */
int CVICALLBACK cbEnableLog (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	int value, dotloc;
	time_t WinTime;
	struct tm *LocalTime;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_LOG_MODE,&value);
			if (value) {
				if (strlen (messfilename) == 0) {
					FileSelectPopup ("","*.txt","*.txt;*.log","Select File",VAL_OK_BUTTON,0,0,1,1,messfilename);
					SetCtrlVal (panelHandle,MAIN_LOGFILENAME,messfilename);
				}
			}
			if (value == 2) {
				// write absolute timestamp as header
				mess_file = OpenFile (messfilename,VAL_WRITE_ONLY,VAL_APPEND,VAL_ASCII);
				if (mess_file == -1) {
					SetCtrlVal (panelHandle,MAIN_LOG_MODE,0);	//do this before Messagepopup.  
					sprintf (write_buf,"Can't open log File %s for writing! Logging aborted.",messfilename);
					MessagePopup ("File Open Error",write_buf);
				} else {
					time(&WinTime);
					LocalTime = localtime (&WinTime);
					if (strftime (write_buf,100,"%Y-%m-%dT%H:%M:%S\n",LocalTime) > 0)
						WriteFile (mess_file,write_buf,strlen(write_buf));	// windows timestamp
				}
				dotloc = strcspn (messfilename,".");
				strncpy (eventfilename, messfilename, dotloc);
				sprintf (eventfilename,"%s_events.txt",eventfilename);
				event_file = OpenFile (eventfilename,VAL_WRITE_ONLY,VAL_APPEND,VAL_ASCII);
				if (event_file == -1) {
					sprintf (write_buf,"Can't open event File %s for writing!",eventfilename);
					MessagePopup ("File Open Error",write_buf);
				} else {
					time(&WinTime);
					LocalTime = localtime (&WinTime);
					if (strftime (write_buf,100,"%Y-%m-%dT%H:%M:%S\n",LocalTime) > 0)
						WriteFile (event_file,write_buf,strlen(write_buf));	// windows timestamp
					CB_update_regs (panelHandle,MAIN_UPDATE_REGS,EVENT_COMMIT,NULL,0,0);
				}
			} else {
				if (value == 0) {
					if (mess_file > 0) 
						CloseFile (mess_file);
				}
			}
			break;
	}
	return 0;
}


int CVICALLBACK cb_conf_filename (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			FileSelectPopup ("", "*.txt", "*.txt;*.log", "Select File", VAL_OK_BUTTON, 0, 0, 1, 1, configfilename);
			SetCtrlVal (panelHandle, MAIN_CONF_FILENAME, configfilename);
			break;
		case EVENT_COMMIT:
			break;
	}
	return 0;
}


int CVICALLBACK CB_CONFIG_FILE (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	int value, res = 0;

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_RING_CONFIG_FILE,&value);
			if (value) {
				if (strlen (configfilename) == 0) {
					FileSelectPopup ("","*.txt","*.txt;*.log","Select File",VAL_OK_BUTTON,0,0,1,1,configfilename);
					SetCtrlVal (panelHandle,MAIN_CONF_FILENAME,configfilename);
				}
			}
			switch(value) {
				case 0:
					// no action
					if (config_file > 0) 
						CloseFile (config_file);
					break;
				case 1:
					// LOAD config file
					config_file = OpenFile (configfilename,VAL_READ_ONLY,0,VAL_ASCII);
					if (config_file == -1) {
						SetCtrlVal (panelHandle,MAIN_RING_CONFIG_FILE,0);	//do this before Messagepopup.  
						sprintf (write_buf,"Can't open File %s for reading! Loading aborted.",configfilename);
						MessagePopup ("File Open Error",write_buf);
					} else {
						res = load_config_registers();
						CloseFile (config_file);
						SetCtrlVal (panelHandle,MAIN_RING_CONFIG_FILE,0);
						if(res == 0) {
							MessagePopup ("Load Config","DONE!");
						} else {
							if(res < 0) {
								MessagePopup ("Load Config","CANCELLED");
							} else {
								MessagePopup ("Load Config","ERROR");
							}
						}
					}
					break;
				case 2:
					// STORE config file
					config_file = OpenFile (configfilename,VAL_WRITE_ONLY,VAL_TRUNCATE,VAL_ASCII);
					if (config_file == -1) {
						SetCtrlVal (panelHandle,MAIN_RING_CONFIG_FILE,0);	//do this before Messagepopup.  
						sprintf (write_buf,"Can't open File %s for reading! Loading aborted.",configfilename);
						MessagePopup ("File Open Error",write_buf);
					} else {
						if(regs_updated) {
							res = store_config_registers();
							SetCtrlVal (panelHandle,MAIN_RING_CONFIG_FILE,0);
							if(res == 0) {
								MessagePopup ("Store Config","DONE!");
							} else {
								MessagePopup ("Store Config","ERROR");
							}
						} else {
							SetCtrlVal (panelHandle,MAIN_RING_CONFIG_FILE,0);
							MessagePopup ("Store Config Error","Update registers and try again.");
						}
						CloseFile (config_file);
					}
					break;
				default:
					return 1;
			}
			break;
	}
	return 0;
}


/* Do it (transfer raw command) button callback */
int CVICALLBACK cb_transfer_command (int panel, int control, int event,
									 void *callbackData, int eventData1, int eventData2)
{
	int write, addr_int;	// dest,
	char rw, addr[10], data[20], outbuf[30];

	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_TXT_ADDRESS, addr);
			GetCtrlVal (panelHandle,MAIN_CHECK_WRITE, &write);
			addr_int = strtol (addr,NULL, 16);
			sprintf (outbuf,"%02X",addr_int);
			SetCtrlVal (panelHandle,MAIN_TXT_ADDRESS,outbuf);		// reformat addr if leading '0' are missing
			if (write == 0)
			{
				// Read command
				rw = CH_R;
				data[0] = 0;
				manual_read = 1;
			}
			else
			{
				// Write command
				rw = CH_W;
				GetCtrlVal (panelHandle,MAIN_TXT_DATA,data);
				manual_read = 0;
			}
			// SendCmd('0'+dest, addr_int, data, 'W', 1);				// registers are all transfered in hex
			sprintf (outbuf,"%c,%02X,%s\r",rw,addr_int,data);
			ComWrt (comport,outbuf,strlen(outbuf));
			break;
	}
	return 0;
}


int CVICALLBACK CB_update_regs (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if (auto_read < 0) {
				auto_read = 0;
				update_reg();
			} else {
				MessagePopup ("Previous run of Update Registers has not finished yet", outbuf);
			}
	}
	return 0;
}


int CVICALLBACK CB_meas_stream (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	int startStopMeas;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_TOGGLE_MEAS_STREAM,&startStopMeas);
			sprintf(outbuf,"W,10,%X\r",startStopMeas);
			ComWrt(comport, outbuf, strlen(outbuf));

			isActive = 0;
			isInactive = 0;
			ResetTimer (panelHandle,MAIN_TIMER_ACTIVITY);
			SetCtrlAttribute (panelHandle,MAIN_TIMER_ACTIVITY,ATTR_ENABLED,0);
			ResetTimer (panelHandle,MAIN_TIMER_INACTIVITY);
			SetCtrlAttribute (panelHandle,MAIN_TIMER_INACTIVITY,ATTR_ENABLED,0);
			SetCtrlVal (panelHandle,MAIN_LED_ACT,0);
			SetCtrlVal (panelHandle,MAIN_LED_INACT,0);
			SetCtrlAttribute (panelHandle,MAIN_STIM_PROG_DISPLAY,ATTR_TEXT_BGCOLOR,VAL_WHITE);
			SetCtrlVal (panelHandle,MAIN_STIM_PROG_DISPLAY,"");

			if(startStopMeas) {
				SetCtrlAttribute (panelHandle,MAIN_RESULT_X,ATTR_DIMMED,0);
				SetCtrlAttribute (panelHandle,MAIN_RESULT_Y,ATTR_DIMMED,0);
				SetCtrlAttribute (panelHandle,MAIN_RESULT_Z,ATTR_DIMMED,0);
				SetCtrlAttribute (panelHandle,MAIN_STIM_PROG_DISPLAY,ATTR_DIMMED,0);
			} else {
				SetCtrlAttribute (panelHandle,MAIN_RESULT_X,ATTR_DIMMED,1);
				SetCtrlAttribute (panelHandle,MAIN_RESULT_Y,ATTR_DIMMED,1);
				SetCtrlAttribute (panelHandle,MAIN_RESULT_Z,ATTR_DIMMED,1);
				SetCtrlAttribute (panelHandle,MAIN_STIM_PROG_DISPLAY,ATTR_DIMMED,1);
			}
			break;
	}
	return 0;
}


int CVICALLBACK CB_TIMER_LED_INT1 (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
			SetCtrlVal (panelHandle,MAIN_LED_INT1,0);
			SetCtrlAttribute (panelHandle,MAIN_TIMER_LED_INT1,ATTR_ENABLED,0);
			break;
	}
	return 0;
}


int CVICALLBACK CB_TIMER_LED_INT2 (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
			SetCtrlVal (panelHandle,MAIN_LED_INT2,0);
			SetCtrlAttribute (panelHandle,MAIN_TIMER_LED_INT2,ATTR_ENABLED,0);
			break;
	}
	return 0;
}


int CVICALLBACK CB_TIMER_LED_NAK (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
			SetCtrlVal (panelHandle,MAIN_LED_RX_MESSAGE_ERR,0);
			SetCtrlAttribute (panelHandle,MAIN_TIMER_LED_NAK,ATTR_ENABLED,0);
			break;
	}
	return 0;
}


int CVICALLBACK CB_TIMER_LED_ACK (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
			SetCtrlVal (panelHandle,MAIN_LED_RX_MESSAGE,0);
			SetCtrlAttribute (panelHandle,MAIN_TIMER_LED_ACK,ATTR_ENABLED,0);
			break;
	}
	return 0;
}


int CVICALLBACK CB_CLEAR (int panel, int control, int event,
						  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
				SetCtrlVal (panelHandle,MAIN_LED_DATA_READY_ON,0);
				SetCtrlVal (panelHandle,MAIN_LED_SINGLE_TAP_ON,0);
				SetCtrlVal (panelHandle,MAIN_LED_DOUBLE_TAP_ON,0);
				SetCtrlVal (panelHandle,MAIN_LED_ACTIVITY_ON,0);
				SetCtrlVal (panelHandle,MAIN_LED_INACTIVITY_ON,0);
				SetCtrlVal (panelHandle,MAIN_LED_FREE_FALL_ON,0);
				SetCtrlVal (panelHandle,MAIN_LED_WATERMARK_ON,0);
				SetCtrlVal (panelHandle,MAIN_LED_OVERRUN_ORIENT_ON,0);

				SetCtrlVal (panelHandle,MAIN_LED_ACT_X_SOURCE,0);
				SetCtrlVal (panelHandle,MAIN_LED_ACT_Y_SOURCE,0);
				SetCtrlVal (panelHandle,MAIN_LED_ACT_Z_SOURCE,0);
				SetCtrlVal (panelHandle,MAIN_LED_ASLEEP,0);
				SetCtrlVal (panelHandle,MAIN_LED_TAP_X_SOURCE,0);
				SetCtrlVal (panelHandle,MAIN_LED_TAP_Y_SOURCE,0);
				SetCtrlVal (panelHandle,MAIN_LED_TAP_Z_SOURCE,0);

				SetCtrlVal (panelHandle,MAIN_LED_X_SIGN,0);
				SetCtrlVal (panelHandle,MAIN_LED_Y_SIGN,0);
				SetCtrlVal (panelHandle,MAIN_LED_Z_SIGN,0);
				SetCtrlVal (panelHandle,MAIN_LED_X_TAP,0);
				SetCtrlVal (panelHandle,MAIN_LED_Y_TAP,0);
				SetCtrlVal (panelHandle,MAIN_LED_Z_TAP,0);

				SetCtrlVal (panelHandle,MAIN_LED_V2,0);
				SetCtrlVal (panelHandle,MAIN_LED_V3,0);
				// set indicators to undefined
				SetCtrlVal (panelHandle,MAIN_PIC_RING_2D_ORIENT,4);
				SetCtrlVal (panelHandle,MAIN_V2_SLIDE,4);
				SetCtrlVal (panelHandle,MAIN_PIC_RING_3D_ORIENT,0);
				SetCtrlVal (panelHandle,MAIN_V3_SLIDE,0);
			break;
	}
	return 0;
}


int CVICALLBACK CB_TIMER_TIMEOUT (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
			// do nothing
			break;
	}
	return 0;
}


int CVICALLBACK CB_ACT_INACT_SOFT_EN (int panel, int control, int event,
									  void *callbackData, int eventData1, int eventData2)
{
	unsigned int value;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_ACT_INACT_SOFT_EN,&value);
			softActInactDetect = value & 0xFF;
			break;
	}
	return 0;
}


int CVICALLBACK CB_TIMER_ACTIVITY (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
			SetCtrlVal (panelHandle,MAIN_LED_ACT,0);
			break;
	}
	return 0;
}


int CVICALLBACK CB_TIMER_INACTIVITY (int panel, int control, int event,
									 void *callbackData, int eventData1, int eventData2)
{
	unsigned int intval;
	
	switch (event)
	{
		case EVENT_TIMER_TICK:
			if(isInactive > 0) {
				SetCtrlVal (panelHandle,MAIN_LED_INACT,1);
				SetCtrlVal (panelHandle,MAIN_LED_ACT,0);
				if(softActInactDetect > 0) {
					GetCtrlVal (panelHandle,MAIN_INACTIVITY_MAP,&intval);
					if(intval > 0)
						turn_stim_prog_2_on();
					else
						turn_stim_prog_1_on();
				}
			}
			break;
	}
	return 0;
}

void CVICALLBACK CB_SCRNSIZE (int menuBar, int menuItem, void *callbackData,
							  int panel)
{
	int menu_handle,rightCtrlArrHandle,leftCtrlArrHandle,topCtrlArrHandle;
	int rightnumctrl,leftnumctrl,topnumctrl,ctrlindx;
	int ctrlid,topval;

	menu_handle = GetPanelMenuBar (panelHandle);
	rightCtrlArrHandle = GetCtrlArrayFromResourceID (panelHandle,RIGHT_CTRL_ARRAY);
	leftCtrlArrHandle = GetCtrlArrayFromResourceID (panelHandle,LEFT_CTRL_ARRAY);
	topCtrlArrHandle = GetCtrlArrayFromResourceID (panelHandle,TOP_CTRL_ARRAY);
   	GetNumCtrlArrayItems (rightCtrlArrHandle,&rightnumctrl);
   	GetNumCtrlArrayItems (leftCtrlArrHandle,&leftnumctrl);
   	GetNumCtrlArrayItems (topCtrlArrHandle,&topnumctrl);
	if(smlscreen) {
		SetMenuBarAttribute (menu_handle,MENU_OPTIONS_SMLSCRNSIZE,ATTR_CHECKED,0);
		smlscreen = 0;
		ctrlindx = 0;
		while (ctrlindx < rightnumctrl) {
			ctrlid = GetCtrlArrayItem (rightCtrlArrHandle,ctrlindx);
			GetCtrlAttribute (panelHandle,ctrlid,ATTR_TOP,&topval);
			SetCtrlAttribute (panelHandle,ctrlid,ATTR_TOP,topval+TOP_OFFSET);
			ctrlindx++;
		}
		ctrlindx = 0;
		while (ctrlindx < leftnumctrl) {
			ctrlid = GetCtrlArrayItem (leftCtrlArrHandle,ctrlindx);
			GetCtrlAttribute (panelHandle,ctrlid,ATTR_TOP,&topval);
			SetCtrlAttribute (panelHandle,ctrlid,ATTR_TOP,topval+TOP_OFFSET);
			ctrlindx++;
		}
		ctrlindx = 0;
		while (ctrlindx < topnumctrl) {
			ctrlid = GetCtrlArrayItem (topCtrlArrHandle,ctrlindx);
			SetCtrlAttribute (panelHandle,ctrlid,ATTR_VISIBLE,1);
			ctrlindx++;
		}
	}
	else {
		SetMenuBarAttribute (menu_handle,MENU_OPTIONS_SMLSCRNSIZE,ATTR_CHECKED,1);
		smlscreen = 1;
		ctrlindx = 0;
		while (ctrlindx < rightnumctrl) {
			ctrlid = GetCtrlArrayItem (rightCtrlArrHandle,ctrlindx);
			GetCtrlAttribute (panelHandle,ctrlid,ATTR_TOP,&topval);
			SetCtrlAttribute (panelHandle,ctrlid,ATTR_TOP,topval-TOP_OFFSET);
			ctrlindx++;
		}
		ctrlindx = 0;
		while (ctrlindx < leftnumctrl) {
			ctrlid = GetCtrlArrayItem (leftCtrlArrHandle,ctrlindx);
			GetCtrlAttribute (panelHandle,ctrlid,ATTR_TOP,&topval);
			SetCtrlAttribute (panelHandle,ctrlid,ATTR_TOP,topval-TOP_OFFSET);
			ctrlindx++;
		}
		ctrlindx = 0;
		while (ctrlindx < topnumctrl) {
			ctrlid = GetCtrlArrayItem (topCtrlArrHandle,ctrlindx);
			SetCtrlAttribute (panelHandle,ctrlid,ATTR_VISIBLE,0);
			ctrlindx++;
		}
	}
}

int CVICALLBACK CB_RING_THRS_DISP (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	int value;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle,MAIN_RING_THRS_DISP,&value);
			showThrsInd = value;
			break;
	}
	return 0;
}
