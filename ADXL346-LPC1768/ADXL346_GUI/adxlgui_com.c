#include "toolbox.h"
#include <stdlib.h>
#include <formatio.h>
#include <utility.h>
#include <ansi_c.h>
#include <rs232.h>
#include <string.h>
#include <cvirte.h>
#include <math.h>

#include "adxlgui_com.h"
#include "adxlgui_uir.h"
#include "adxlgui_regs.h"



volatile int result;
char cmd_buffer[16];


/* Clean-up before closing the GUI */
void bye(void)
{
	if(mess_file > 0) 
		CloseFile (mess_file);
	if(event_file > 0) 
		CloseFile (event_file);

	// Uninstall ComCallback and close COMx ports
	if (comstatus)
	{
		Delay(0.03);
		CloseCom (comport);
		InstallComCallback (comport, 0, 0, 0, ComCallback, 0);
	}
	QuitUserInterface (0);
}


/* scan the registry for available COM Ports and build the select button */
void scan4coms(void)
{
	unsigned char 	string[512];
	unsigned int  	size1,size2,values,i;
	int           	type;
	char          	valueName[512];
	int 			coms[256];
	int 			swaped;
	int 			temp;

	coms[0] = 0;
	ClearListCtrl (panelCOM, SELECT_COM_COM_SELECT);
	RegQueryInfoOnKey (REGKEY_HKLM, "Hardware\\Devicemap\\Serialcomm",NULL, &values,NULL,NULL,NULL);
	for(i=0; i<values; i++)
	{
		size1 = 512;
		size2 = 512;
		RegEnumerateValue (REGKEY_HKLM, "Hardware\\Devicemap\\Serialcomm", i, valueName, &size1, string,
						   &size2, &type);
		if ( type==_REG_SZ )
		{
			coms[i+1]=atoi(&string[3]);
		}
	}
	if (values > 1)
	{
		do
		{
			swaped = 0;
			for (i=1; i<values; i++)
				if (coms[i] < coms[i-1])
				{
					temp = coms[i];
					coms[i] = coms[i-1];
					coms[i-1] = temp;
					swaped = 1;
				}
		}
		while (swaped == 1);
	}
	for (i=0; i<values+1; i++)
	{
		if (coms[i] == 0)
		{
			InsertListItem (panelCOM, SELECT_COM_COM_SELECT, i, "None", 0);
		}
		else
		{
			sprintf (string, "COM%d", coms[i]);
			InsertListItem (panelCOM, SELECT_COM_COM_SELECT, i, string, coms[i]);
		}
	}
}


void update_reg(void)
{
	int reg;
	
	reg = update_regs_array[auto_read++];
	if (reg > 0) {
		sprintf (outbuf,"R,%02X\r",reg);
		ComWrt(comport, outbuf, strlen(outbuf));
	} else {
		regs_updated = 1;
		auto_read = -1;
	}
}


int store_config_registers(void)
{
	unsigned int ix, ctrl, addr;
	int value, res = 0;
	
	if(config_file > 0) {
		ix = 0;
		ctrl = ctrl_regs_array[ix];
		addr = update_regs_array[ix];
		while(addr > 0) {
			GetCtrlVal(panelHandle,ctrl,&value);
			sprintf (write_buf,"%02X,%02X\n",addr,value);
			WriteFile (config_file,write_buf,strlen (write_buf));
			ix++;
			ctrl = ctrl_regs_array[ix];
			addr = update_regs_array[ix];
		}
	} else {
		res = 1;
	}
	return(res);
}


int load_config_registers(void)
{
	unsigned int ix, ctrl, addr, readaddr;
	unsigned int foundit = 0;
	int value, cnt, confrm, res = 0;
	char line[10];
	char *pch;
	myCtrlCallbackPtr cbfnct;
	
	confrm = ConfirmPopup ("Load Config","Stop measurements and disable all interrupts?");
	if(confrm == 0) {
		return(-1);
	}
	SetCtrlVal (panelHandle,MAIN_TOGGLE_MEAS_STREAM,0);
	CB_meas_stream (panelHandle,MAIN_TOGGLE_MEAS_STREAM,EVENT_COMMIT,NULL,0,0);
	SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,0);
	CB_INT_ENABLE (panelHandle,MAIN_NUM_INT_ENABLE,EVENT_COMMIT,NULL,0,0);
	
	if(config_file > 0) {
		cnt = ReadFile (config_file, line, 6);
		line[6] = 0;
		while(cnt == 6) {
			pch = strtok (line,",");
			readaddr = strtol (pch,NULL,16);
			pch = strtok (NULL,",");
			value = strtol (pch,NULL,16);
			ix = 0;
			addr = update_regs_array[ix];
			while((addr > 0) & (foundit == 0)) {
				if(addr == readaddr) {
					foundit = 1;
					ctrl = ctrl_regs_array[ix];
					SetCtrlVal(panelHandle,ctrl,value);
					cbfnct = callback_array[ix];
					cbfnct (panelHandle,ctrl,EVENT_COMMIT,NULL,0,0);
					// Set the timer up
					timeoutcnt = 1000;
					cbfrdy = 0;
					SetCtrlAttribute (panelHandle,MAIN_TIMER_TIMEOUT,ATTR_ENABLED,1);
					while ((cbfrdy == 0) && (timeoutcnt > 0))
					{
						ProcessSystemEvents();
						timeoutcnt--;
					}
					SetCtrlAttribute (panelHandle,MAIN_TIMER_TIMEOUT,ATTR_ENABLED,0);
				} else {
					ix++;
					addr = update_regs_array[ix];
				}
			}
			cnt = ReadFile (config_file, line, 6);
			foundit = 0;
		}
	} else {
		res = 1;
	}
	return(res);
}

																  
void command_decoder(void)
{
	static int plot_ext = -1;
	static int plot_sawbp = -1;
	int intval, sigval, range, bwdiv;
	int meas_type, manual_addr;
	char tbox_read_data[80], manual_addr_txt[3];
	float graph_xyz[5], rate, orient_bw, offset;
	int parsed_length;
	char *pch;
	double value_min, value_max;
	int log_file, logmode;
	unsigned int ix;
	
	GetCtrlVal (panelHandle, MAIN_TXT_ADDRESS, manual_addr_txt);
	manual_addr = (int) strtol (manual_addr_txt,NULL,16);
	if ((manual_read) && (addr == manual_addr))
	{
		pch = strtok (cmd_buf,",");
		SetCtrlVal (panelHandle, MAIN_TXT_DATA, pch);
		manual_read = 0;
	}
	else
	{
		cbfrdy = 1;
		switch (addr)
		{
			case 0x10:		// ADXL X,Y,Z data
				cbfrdy = 0;
				strcpy (tbox_read_data,cmd_buf);
				parsed_length = decode_adxl_data (tbox_read_data,graph_xyz);
				graph_xyz[3] = thresholds[showThrsInd];
				graph_xyz[4] = -thresholds[showThrsInd];
				if (parsed_length == 3)
				{
					PlotStripChart(panelHandle,MAIN_STRIPCHART,graph_xyz,5,0,0,VAL_FLOAT);
					for(ix = 0; ix < 3; ix++) graph_xyz[ix] *= rightScaleFactor;
					SetCtrlVal (panelHandle,MAIN_RESULT_X,graph_xyz[0]);
					SetCtrlVal (panelHandle,MAIN_RESULT_Y,graph_xyz[1]);
					SetCtrlVal (panelHandle,MAIN_RESULT_Z,graph_xyz[2]);
				}
				sprintf (write_buf,"%.3f,%f,%f,%f\n",Timer(),graph_xyz[0],graph_xyz[1],graph_xyz[2]);
				check_act_inact(graph_xyz);
				break;
			case 0x11:
				cbfrdy = 0;
				SetCtrlVal (panelHandle,MAIN_LED_INT1,1);
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				display_source(intval);
				turn_stim_prog_1_on();
				SetCtrlAttribute (panelHandle,MAIN_TIMER_LED_INT1,ATTR_ENABLED,1);
				sprintf (write_buf,"%.3f INT1 %X\n",Timer(),intval);
				break;
			case 0x12:
				cbfrdy = 0;
				SetCtrlVal (panelHandle,MAIN_LED_INT2,1);
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				display_source(intval);
				turn_stim_prog_2_on();
				SetCtrlAttribute (panelHandle,MAIN_TIMER_LED_INT2,ATTR_ENABLED,1);
				sprintf (write_buf,"%.3f INT2 %X\n",Timer(),intval);
				break;
			case ADDR_THRESH_TAP:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_THRESH_TAP,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_THRESH_TAP,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_THRESH_TAP_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_THRESH_TAP_HEX,ATTR_DIMMED,0);
				thresholds[THRSTAP] = intval*62.5e-3;
				SetCtrlVal (panelHandle,MAIN_NUM_THRESH_TAP_CONV,thresholds[THRSTAP]);
				SetCtrlAttribute (panelHandle,MAIN_NUM_THRESH_TAP_CONV,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f THRESH_TAP %X\n",Timer(),intval);
				break;
			case ADDR_DUR:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_DUR,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_DUR,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_DUR_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_DUR_HEX,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_DUR_CONV,intval*625e-6);
				SetCtrlAttribute (panelHandle,MAIN_NUM_DUR_CONV,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f DUR %X\n",Timer(),intval);
				break;
			case ADDR_LATENT:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_Latent,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_Latent,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_Latent_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_Latent_HEX,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_Latent_CONV,intval*1.25e-3);
				SetCtrlAttribute (panelHandle,MAIN_NUM_Latent_CONV,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f LATENT %X\n",Timer(),intval);
				break;
			case ADDR_WINDOW:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_Window,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_Window,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_Window_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_Window_HEX,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_Window_CONV,intval*1.25e-3);
				SetCtrlAttribute (panelHandle,MAIN_NUM_Window_CONV,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f WINDOW %X\n",Timer(),intval);
				break;
			case ADDR_TAP_AXES:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_TAP_AXES,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_TAP_AXES_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_TAP_AXES_HEX,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_IMPROVE_TAP_EN,(intval & 0x10) >> 4);
				SetCtrlAttribute (panelHandle,MAIN_IMPROVE_TAP_EN,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_SUPPRESS_EN,(intval & 0x08) >> 3);
				SetCtrlAttribute (panelHandle,MAIN_SUPPRESS_EN,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_X_TAP_EN,(intval & 0x04) >> 2);
				SetCtrlAttribute (panelHandle,MAIN_X_TAP_EN,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_Y_TAP_EN,(intval & 0x02) >> 1);
				SetCtrlAttribute (panelHandle,MAIN_Y_TAP_EN,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_Z_TAP_EN,intval & 0x01);
				SetCtrlAttribute (panelHandle,MAIN_Z_TAP_EN,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f TAP_AXES %X\n",Timer(),intval);
				break;
			case ADDR_TAP_SIGN:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_LED_X_SIGN,(intval & 0x40) >> 6);
				SetCtrlAttribute (panelHandle,MAIN_LED_X_SIGN,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_Y_SIGN,(intval & 0x20) >> 5);
				SetCtrlAttribute (panelHandle,MAIN_LED_Y_SIGN,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_Z_SIGN,(intval & 0x10) >> 4);
				SetCtrlAttribute (panelHandle,MAIN_LED_Z_SIGN,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_X_TAP,(intval & 0x04) >> 2);
				SetCtrlAttribute (panelHandle,MAIN_LED_X_TAP,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_Y_TAP,(intval & 0x02) >> 1);
				SetCtrlAttribute (panelHandle,MAIN_LED_Y_TAP,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_Z_TAP,intval & 0x01);
				SetCtrlAttribute (panelHandle,MAIN_LED_Z_TAP,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f TAP_SIGN %X\n",Timer(),intval);
				break;
			case ADDR_THRESH_ACT:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_THRESH_ACT,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_THRESH_ACT,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_THRESH_ACT_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_THRESH_ACT_HEX,ATTR_DIMMED,0);
				thresholds[THRSACT] = intval*62.5e-3;
				thresholds[THRSACT] = intval*62.5e-3;
				SetCtrlVal (panelHandle,MAIN_NUM_THRESH_ACT_CONV,thresholds[THRSACT]);
				SetCtrlAttribute (panelHandle,MAIN_NUM_THRESH_ACT_CONV,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f THRESH_ACT %X\n",Timer(),intval);
				break;
			case ADDR_THRESH_INACT:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_THRESH_INACT,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_THRESH_INACT,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_THRESH_INACT_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_THRESH_INACT_HEX,ATTR_DIMMED,0);
				thresholds[THRSINA] = intval*62.5e-3;
				SetCtrlVal (panelHandle,MAIN_NUM_THRESH_INACT_CONV,thresholds[THRSINA]);
				SetCtrlAttribute (panelHandle,MAIN_NUM_THRESH_INACT_CONV,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f THRESH_INACT %X\n",Timer(),intval);
				break;
			case ADDR_TIME_INACT:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_TIME_INACT,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_TIME_INACT,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_TIME_INACT_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_TIME_INACT_HEX,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_TIME_INACT_CONV,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_TIME_INACT_CONV,ATTR_DIMMED,0);
				SetCtrlAttribute (panelHandle,MAIN_TIMER_INACTIVITY,ATTR_INTERVAL,(double) intval);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f TIME_INACT %X\n",Timer(),intval);
				break;
			case ADDR_ACT_INACT_CTL:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_ACT_INACT_CTL,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_ACT_INACT_CTL_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_ACT_INACT_CTL_HEX,ATTR_DIMMED,0);
				SetCtrlAttribute (panelHandle,MAIN_ACT_INACT_SOFT_EN,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_ACT_AC_DC,(intval & 0x80) >> 7);
				SetCtrlAttribute (panelHandle,MAIN_ACT_AC_DC,ATTR_DIMMED,0);
				x_act_en = (intval & 0x40) >> 6;
				SetCtrlVal (panelHandle,MAIN_ACT_X_EN,x_act_en);
				SetCtrlAttribute (panelHandle,MAIN_ACT_X_EN,ATTR_DIMMED,0);
				y_act_en = (intval & 0x20) >> 5;
				SetCtrlVal (panelHandle,MAIN_ACT_Y_EN,y_act_en);
				SetCtrlAttribute (panelHandle,MAIN_ACT_Y_EN,ATTR_DIMMED,0);
				z_act_en = (intval & 0x10) >> 4;
				SetCtrlVal (panelHandle,MAIN_ACT_Z_EN,z_act_en);
				SetCtrlAttribute (panelHandle,MAIN_ACT_Z_EN,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_INACT_AC_DC,(intval & 0x08) >> 3);
				SetCtrlAttribute (panelHandle,MAIN_INACT_AC_DC,ATTR_DIMMED,0);
				x_inact_en = (intval & 0x04) >> 2;
				SetCtrlVal (panelHandle,MAIN_INACT_X_EN,x_inact_en);
				SetCtrlAttribute (panelHandle,MAIN_INACT_X_EN,ATTR_DIMMED,0);
				y_inact_en = (intval & 0x02) >> 1;
				SetCtrlVal (panelHandle,MAIN_INACT_Y_EN,y_inact_en);
				SetCtrlAttribute (panelHandle,MAIN_INACT_Y_EN,ATTR_DIMMED,0);
				z_inact_en = intval & 0x01;
				SetCtrlVal (panelHandle,MAIN_INACT_Z_EN,z_inact_en);
				SetCtrlAttribute (panelHandle,MAIN_INACT_Z_EN,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f ACT_INACT_CTL %X\n",Timer(),intval);
				break;
			case ADDR_ACT_TAP_STATUS:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_LED_ACT_X_SOURCE,(intval & 0x40) >> 6);
				SetCtrlAttribute (panelHandle,MAIN_LED_ACT_X_SOURCE,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_ACT_Y_SOURCE,(intval & 0x20) >> 5);
				SetCtrlAttribute (panelHandle,MAIN_LED_ACT_Y_SOURCE,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_ACT_Z_SOURCE,(intval & 0x10) >> 4);
				SetCtrlAttribute (panelHandle,MAIN_LED_ACT_Z_SOURCE,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_ASLEEP,(intval & 0x08) >> 3);
				SetCtrlAttribute (panelHandle,MAIN_LED_ASLEEP,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_TAP_X_SOURCE,(intval & 0x04) >> 2);
				SetCtrlAttribute (panelHandle,MAIN_LED_TAP_X_SOURCE,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_TAP_Y_SOURCE,(intval & 0x02) >> 1);
				SetCtrlAttribute (panelHandle,MAIN_LED_TAP_Y_SOURCE,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_TAP_Z_SOURCE,intval & 0x01);
				SetCtrlAttribute (panelHandle,MAIN_LED_TAP_Z_SOURCE,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f ACT_TAP_STATUS %X\n",Timer(),intval);
				break;
			case ADDR_ORIENT_CONF:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_ORIENT_CONFIG,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_ORIENT_CONFIG,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_ORIENT_CONFIG_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_ORIENT_CONFIG_HEX,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_INT_ORIENT,(intval & 0x80) >> 7);
				SetCtrlAttribute (panelHandle,MAIN_INT_ORIENT,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_INT_3D,(intval & 0x08) >> 3);
				SetCtrlAttribute (panelHandle,MAIN_INT_3D,ATTR_DIMMED,0);
				SetCtrlVal(panelHandle,MAIN_RING_DEAD_ZONE,(intval & 0x70) >> 4);
				SetCtrlAttribute (panelHandle,MAIN_RING_DEAD_ZONE,ATTR_DIMMED,0);
				SetCtrlVal(panelHandle,MAIN_RING_ORIENT_DIV,intval & 0x07);
				SetCtrlAttribute (panelHandle,MAIN_RING_ORIENT_DIV,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f ORIENT_CONF %X\n",Timer(),intval);
				break;
			case ADDR_ORIENT:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_LED_V2,(intval & 0x40) >> 6);
				if(intval & 0x40) {
					SetCtrlVal (panelHandle,MAIN_PIC_RING_2D_ORIENT,(intval & 0x30)>>4);
					SetCtrlVal (panelHandle,MAIN_V2_SLIDE,(intval & 0x30)>>4);
				} else {
					// set indicators to 4 = undefined
					SetCtrlVal (panelHandle,MAIN_PIC_RING_2D_ORIENT,4);
					SetCtrlVal (panelHandle,MAIN_V2_SLIDE,4);
				}
				SetCtrlVal (panelHandle,MAIN_LED_V3,(intval & 0x08) >> 3);
				if(intval & 0x08) {
					SetCtrlVal (panelHandle,MAIN_PIC_RING_3D_ORIENT,intval & 0x07);
					SetCtrlVal (panelHandle,MAIN_V3_SLIDE,(intval & 0x07));
				} else {
					// set indicators to 0 = undefined
					SetCtrlVal (panelHandle,MAIN_PIC_RING_3D_ORIENT,0);
					SetCtrlVal (panelHandle,MAIN_V3_SLIDE,0);
				}
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f ORIENT %X\n",Timer(),intval);
				break;
			case ADDR_INT_ENABLE:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_INT_ENABLE,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_INT_ENABLE_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_INT_ENABLE_HEX,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_DTA_RDY_INT,(intval & 0x80) >> 7);
				SetCtrlAttribute (panelHandle,MAIN_DTA_RDY_INT,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_SINGLE_TAP_INT,(intval & 0x40) >> 6);
				SetCtrlAttribute (panelHandle,MAIN_SINGLE_TAP_INT,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_DOUBLE_TAP_INT,(intval & 0x20) >> 5);
				SetCtrlAttribute (panelHandle,MAIN_DOUBLE_TAP_INT,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_ACTIVITY_INT,(intval & 0x10) >> 4);
				SetCtrlAttribute (panelHandle,MAIN_ACTIVITY_INT,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_INACTIVITY_INT,(intval & 0x08) >> 3);
				SetCtrlAttribute (panelHandle,MAIN_INACTIVITY_INT,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_FREE_FALL_INT,(intval & 0x04) >> 2);
				SetCtrlAttribute (panelHandle,MAIN_FREE_FALL_INT,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_WATERMARK_INT,(intval & 0x02) >> 1);
				SetCtrlAttribute (panelHandle,MAIN_WATERMARK_INT,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_OVR_ORIENT_INT,intval & 0x01);
				SetCtrlAttribute (panelHandle,MAIN_OVR_ORIENT_INT,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f INT_ENABLE %X\n",Timer(),intval);
				break;
			case ADDR_INT_MAP:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_INT_MAP,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_INT_MAP_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_INT_MAP_HEX,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_DTA_RDY_MAP,(intval & 0x80) >> 7);
				SetCtrlAttribute (panelHandle,MAIN_DTA_RDY_MAP,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_SINGLE_TAP_MAP,(intval & 0x40) >> 6);
				SetCtrlAttribute (panelHandle,MAIN_SINGLE_TAP_MAP,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_DOUBLE_TAP_MAP,(intval & 0x20) >> 5);
				SetCtrlAttribute (panelHandle,MAIN_DOUBLE_TAP_MAP,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_ACTIVITY_MAP,(intval & 0x10) >> 4);
				SetCtrlAttribute (panelHandle,MAIN_ACTIVITY_MAP,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_INACTIVITY_MAP,(intval & 0x08) >> 3);
				SetCtrlAttribute (panelHandle,MAIN_INACTIVITY_MAP,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_FREE_FALL_MAP,(intval & 0x04) >> 2);
				SetCtrlAttribute (panelHandle,MAIN_FREE_FALL_MAP,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_WATERMARK_MAP,(intval & 0x02) >> 1);
				SetCtrlAttribute (panelHandle,MAIN_WATERMARK_MAP,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_OVR_ORIENT_MAP,intval & 0x01);
				SetCtrlAttribute (panelHandle,MAIN_OVR_ORIENT_MAP,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f INT_MAP %X\n",Timer(),intval);
				break;
			case ADDR_INT_SOURCE:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_LED_DATA_READY_ON,(intval & 0x80) >> 7);
				SetCtrlAttribute (panelHandle,MAIN_LED_DATA_READY_ON,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_SINGLE_TAP_ON,(intval & 0x40) >> 6);
				SetCtrlAttribute (panelHandle,MAIN_LED_SINGLE_TAP_ON,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_DOUBLE_TAP_ON,(intval & 0x20) >> 5);
				SetCtrlAttribute (panelHandle,MAIN_LED_DOUBLE_TAP_ON,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_ACTIVITY_ON,(intval & 0x10) >> 4);
				SetCtrlAttribute (panelHandle,MAIN_LED_ACTIVITY_ON,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_INACTIVITY_ON,(intval & 0x08) >> 3);
				SetCtrlAttribute (panelHandle,MAIN_LED_INACTIVITY_ON,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_FREE_FALL_ON,(intval & 0x04) >> 2);
				SetCtrlAttribute (panelHandle,MAIN_LED_FREE_FALL_ON,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_WATERMARK_ON,(intval & 0x02) >> 1);
				SetCtrlAttribute (panelHandle,MAIN_LED_WATERMARK_ON,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LED_OVERRUN_ORIENT_ON,intval & 0x01);
				SetCtrlAttribute (panelHandle,MAIN_LED_OVERRUN_ORIENT_ON,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f INT_SOURCE %X\n",Timer(),intval);
				break;
			case ADDR_OFSX:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_X_OFFSET,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_X_OFFSET,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_X_OFFSET_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_X_OFFSET_HEX,ATTR_DIMMED,0);
				if(intval & 0x80) {
					sigval = intval | INT_NEG_MASK;
				} else {
					sigval = intval;
				}
				offset = sigval*15.6e-3;
				SetCtrlVal (panelHandle,MAIN_NUM_X_OFFSET_CONV,offset);
				SetCtrlAttribute (panelHandle,MAIN_NUM_X_OFFSET_CONV,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f OFSX %X\n",Timer(),intval);
				break;
			case ADDR_OFSY:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_Y_OFFSET,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_Y_OFFSET,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_Y_OFFSET_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_Y_OFFSET_HEX,ATTR_DIMMED,0);
				if(intval & 0x80) {
					sigval = intval | INT_NEG_MASK;
				} else {
					sigval = intval;
				}
				offset = sigval*15.6e-3;
				SetCtrlVal (panelHandle,MAIN_NUM_Y_OFFSET_CONV,offset);
				SetCtrlAttribute (panelHandle,MAIN_NUM_Y_OFFSET_CONV,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f OFSY %X\n",Timer(),intval);
				break;
			case ADDR_OFSZ:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_Z_OFFSET,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_Z_OFFSET,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_Z_OFFSET_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_Z_OFFSET_HEX,ATTR_DIMMED,0);
				if(intval & 0x80) {
					sigval = intval | INT_NEG_MASK;
				} else {
					sigval = intval;
				}
				offset = sigval*15.6e-3;
				SetCtrlVal (panelHandle,MAIN_NUM_Z_OFFSET_CONV,offset);
				SetCtrlAttribute (panelHandle,MAIN_NUM_Z_OFFSET_CONV,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f OFSZ %X\n",Timer(),intval);
				break;
			case ADDR_DATA_FORMAT:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_DATA_FORMAT,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_DATA_FORMAT,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_DATA_FORMAT_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_DATA_FORMAT_HEX,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_RING_RANGE,intval & 0x03);
				SetCtrlAttribute (panelHandle,MAIN_RING_RANGE,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_SELF_TEST_EN,(intval & 0x80) >> 7);
				SetCtrlAttribute (panelHandle,MAIN_SELF_TEST_EN,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_FULL_RES_EN,(intval & 0x08) >> 3);
				SetCtrlAttribute (panelHandle,MAIN_FULL_RES_EN,ATTR_DIMMED,0);
				range = 2 * pow(2,intval & 0x03);
				rightScaleFactor = (float) range / 512.0;
				GetCtrlVal (panelHandle,MAIN_F_MIN,&value_min);
				GetCtrlVal (panelHandle,MAIN_F_MAX,&value_max);
				SetAxisScalingMode (panelHandle,MAIN_STRIPCHART,VAL_LEFT_YAXIS,VAL_MANUAL,value_min,value_max);
				SetAxisScalingMode (panelHandle,MAIN_STRIPCHART,VAL_RIGHT_YAXIS,VAL_MANUAL,value_min*rightScaleFactor,value_max*rightScaleFactor);
				SetCtrlAttribute (panelHandle,MAIN_STRIPCHART,ATTR_ACTIVE_YAXIS,VAL_RIGHT_YAXIS);
				SetCtrlAttribute (panelHandle,MAIN_STRIPCHART,ATTR_YLABEL_VISIBLE,1);
				SetCtrlAttribute (panelHandle,MAIN_STRIPCHART,ATTR_ACTIVE_YAXIS,VAL_LEFT_YAXIS);
				SetCtrlAttribute (panelHandle,MAIN_STRIPCHART,ATTR_YLABEL_VISIBLE,1);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f DATA_FORMAT %X\n",Timer(),intval);
				break;
			case ADDR_BW_RATE:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_BW_RATE,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_BW_RATE,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_BW_RATE_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_BW_RATE_HEX,ATTR_DIMMED,0);
	    		rate = 3200 / (1 << (15-(intval & 0x0F)));
				SetCtrlVal (panelHandle,MAIN_RING_OUTPUT_RATE,intval & 0x0F);
				SetCtrlAttribute (panelHandle,MAIN_RING_OUTPUT_RATE,ATTR_DIMMED,0);
				GetCtrlVal(panelHandle,MAIN_RING_ORIENT_DIV,&bwdiv);
				SetCtrlVal(panelHandle,MAIN_NUM_DIV_BW_HZ,rate/divisor_bw_array[bwdiv & 0x07]);
				SetCtrlAttribute (panelHandle,MAIN_NUM_DIV_BW_HZ,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LOW_POWER_EN,(intval & 0x10) >> 4);
				SetCtrlAttribute (panelHandle,MAIN_LOW_POWER_EN,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f BW_RATE %X\n",Timer(),intval);
				break;
			case ADDR_POWER_CTL:
				pch = strtok (cmd_buf,",");
				intval = strtol (pch,NULL,16);
				SetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_POWER_CTL,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_NUM_POWER_CTL_HEX,intval);
				SetCtrlAttribute (panelHandle,MAIN_NUM_POWER_CTL_HEX,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_LINK_BIT,(intval & 0x20) >> 5);
				SetCtrlAttribute (panelHandle,MAIN_LINK_BIT,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_AUTO_SLEEP,(intval & 0x10) >> 4);
				SetCtrlAttribute (panelHandle,MAIN_AUTO_SLEEP,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_MEASURE_ON,(intval & 0x08) >> 3);
				SetCtrlAttribute (panelHandle,MAIN_MEASURE_ON,ATTR_DIMMED,0);
				SetCtrlVal (panelHandle,MAIN_SLEEP,(intval & 0x04) >> 2);
				SetCtrlAttribute (panelHandle,MAIN_SLEEP,ATTR_DIMMED,0);
				SetCtrlVal(panelHandle,MAIN_RING_WAKEUP_FREQ,intval & 0x03);
				SetCtrlAttribute (panelHandle,MAIN_RING_WAKEUP_FREQ,ATTR_DIMMED,0);
				if (auto_read > 0)
					update_reg();
				sprintf (write_buf,"%.3f POWER_CTL %X\n",Timer(),intval);
				break;
			case 0xFE:
				strcpy (tbox_read_data,cmd_buf);
				SetCtrlVal (panelHandle,MAIN_STATUS,&tbox_read_data[1]);
				sprintf (write_buf,"%.3f DEBUGTXT %s\n",Timer(),&tbox_read_data[1]);
				if (strncmp (tbox_read_data,"ACK",3)) {
					SetCtrlVal (panelHandle,MAIN_LED_RX_MESSAGE,1);
					SetCtrlAttribute (panelHandle,MAIN_TIMER_LED_ACK,ATTR_ENABLED,1);
				} else {
					if (strncmp (tbox_read_data,"NAK",3)) {
						SetCtrlVal (panelHandle,MAIN_LED_RX_MESSAGE_ERR,1);
						SetCtrlAttribute (panelHandle,MAIN_TIMER_LED_NAK,ATTR_ENABLED,1);
					}
				}
				break;
			case 0xFF:
				sprintf (tbox_read_data,"%s, %s",title,&cmd_buf[1]);
				SetCtrlAttribute (panelHandle,MAIN_DYNEUMO1_LOGO,ATTR_LABEL_TEXT,tbox_read_data);
				sprintf (write_buf,"%.3f VERSION %s\n",Timer(),tbox_read_data);
				break;
			default:
				break;
		}
		GetCtrlVal (panelHandle, MAIN_LOG_MODE, &logmode);
		if (logmode == 2)
		{
			if (addr == 0x10)
				log_file = mess_file;
			else
				log_file = event_file;
			if (log_file == -1)
			{
				SetCtrlVal (panelHandle,MAIN_LOG_MODE,0);	//do this before Messagepopup.
				sprintf (write_buf,"Can't open File %s for writing! Logging aborted.",messfilename);
				MessagePopup ("File Open Error",write_buf);
			}
			else
			{
				WriteFile (log_file,write_buf,strlen (write_buf));
			}
		}
		if((timeoutcnt > 0) && (cbfrdy == 0)) {
			cbfrdy = 1;
		}
	}
}


int decode_adxl_data (char *rx_buf, float *graph_buffer)
{
	int length;
	char * pch;
	float * pf;
	float data;

	pch = strtok (rx_buf, ",");
	pf	= graph_buffer;
	length = 0;
	while ((pch != NULL) & (length < 3))
	{
		data = atof(pch);
		*(pf++) = data;
		pch = strtok (NULL, ",");
		length++;
	}
	return length;
}



void display_source(int val)
{
	int orient_int;
	SetCtrlVal (panelHandle,MAIN_LED_DATA_READY_ON,val & 0x80);
	SetCtrlVal (panelHandle,MAIN_LED_SINGLE_TAP_ON,val & 0x40);
	SetCtrlVal (panelHandle,MAIN_LED_DOUBLE_TAP_ON,val & 0x20);
	SetCtrlVal (panelHandle,MAIN_LED_ACTIVITY_ON,val & 0x10);
	SetCtrlVal (panelHandle,MAIN_LED_INACTIVITY_ON,val & 0x08);
	SetCtrlVal (panelHandle,MAIN_LED_FREE_FALL_ON,val & 0x04);
	SetCtrlVal (panelHandle,MAIN_LED_WATERMARK_ON,val & 0x02);
	SetCtrlVal (panelHandle,MAIN_LED_OVERRUN_ORIENT_ON,val & 0x01);
	if(val & 0x78) {
		// If it is tap or activity/inactivity get XYZ source
		sprintf(outbuf,"R,%02X\r",ADDR_ACT_TAP_STATUS);
		ComWrt(comport,outbuf,strlen(outbuf));
		if(val & 0x60) {
			// If it is tap, get tap sign
			Delay(0.03);
			sprintf(outbuf,"R,%02X\r",ADDR_TAP_SIGN);
			ComWrt(comport,outbuf,strlen(outbuf));
		}
	}
	GetCtrlVal (panelHandle,MAIN_INT_ORIENT,&orient_int);
	if(orient_int) {
		if(val &0x01) {
			sprintf(outbuf,"R,%02X\r",ADDR_ORIENT);
			ComWrt(comport,outbuf,strlen(outbuf));
		}
	}
}


void check_act_inact(float *accel_xyz)
{
	unsigned int intval;
	char isActiveNow = 0;
	char isInactiveNow = 0;
	float x,y,z;
	
	x = fabs(accel_xyz[0]);
	y = fabs(accel_xyz[1]);
	z = fabs(accel_xyz[2]);
	
	GetCtrlVal (panelHandle,MAIN_TOGGLE_MEAS_STREAM,&intval);
	if(intval == 0)
		return;
	
	isActiveNow = ((x_act_en > 0) & (x > thresholds[THRSACT])) | \
			   	((y_act_en > 0) & (y > thresholds[THRSACT])) | \
				((z_act_en > 0) & (z > thresholds[THRSACT]));
	if((isActive == 0) & (isActiveNow > 0)) {
		ResetTimer (panelHandle,MAIN_TIMER_ACTIVITY);
		ResetTimer (panelHandle,MAIN_TIMER_INACTIVITY);
		SetCtrlAttribute (panelHandle,MAIN_TIMER_ACTIVITY,ATTR_ENABLED,0);
		SetCtrlAttribute (panelHandle,MAIN_TIMER_INACTIVITY,ATTR_ENABLED,0);
		SetCtrlVal (panelHandle,MAIN_LED_ACT,1);
		SetCtrlVal (panelHandle,MAIN_LED_INACT,0);
		if(softActInactDetect > 0) {
			GetCtrlVal (panelHandle,MAIN_ACTIVITY_MAP,&intval);
			if(intval > 0)
				turn_stim_prog_2_on();
			else
				turn_stim_prog_1_on();
		}
	} else {
		if((isActive > 0) & (isActiveNow == 0)) {
			ResetTimer (panelHandle,MAIN_TIMER_ACTIVITY);
			SetCtrlAttribute (panelHandle,MAIN_TIMER_ACTIVITY,ATTR_ENABLED,1);
		}
	}
	isActive = isActiveNow;
	if(isActive > 0) {
		isInactive = 0;
		return;
	}
	
	if(x_inact_en > 0) {
		isInactiveNow = (x < thresholds[THRSINA]);
	}
	if(y_inact_en > 0) {
		isInactiveNow &= (y < thresholds[THRSINA]);
	}
	if(z_inact_en > 0) {
		isInactiveNow &= (z < thresholds[THRSINA]);
	}

	if((isInactive == 0) & (isInactiveNow > 0)) {
		ResetTimer (panelHandle,MAIN_TIMER_INACTIVITY);
		SetCtrlAttribute (panelHandle,MAIN_TIMER_INACTIVITY,ATTR_ENABLED,1);
	} else {
		if((isInactive > 0) & (isInactiveNow == 0)) {
			ResetTimer (panelHandle,MAIN_TIMER_INACTIVITY);
			SetCtrlAttribute (panelHandle,MAIN_TIMER_INACTIVITY,ATTR_ENABLED,0);
			SetCtrlVal (panelHandle,MAIN_LED_INACT,0);
		}
	}
	isInactive = isInactiveNow;
}


void turn_stim_prog_1_on(void) {
	SetCtrlAttribute (panelHandle,MAIN_STIM_PROG_DISPLAY,ATTR_TEXT_BGCOLOR,VAL_CYAN);
	SetCtrlVal (panelHandle,MAIN_STIM_PROG_DISPLAY,"STIM PROGRAM 1");
}


void turn_stim_prog_2_on(void) {
	SetCtrlAttribute (panelHandle,MAIN_STIM_PROG_DISPLAY,ATTR_TEXT_BGCOLOR,VAL_MAGENTA);
	SetCtrlVal (panelHandle,MAIN_STIM_PROG_DISPLAY,"STIM PROGRAM 2");
}



