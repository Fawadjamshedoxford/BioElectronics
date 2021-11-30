/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  MAIN                             1       /* callback function: CB_Main */
#define  MAIN_NUM_BW_RATE_HEX             2       /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_DATA_FORMAT_HEX         3       /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_ORIENT_CONFIG_HEX       4       /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_ACT_INACT_CTL_HEX       5       /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_TAP_AXES_HEX            6       /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_INT_MAP_HEX             7       /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_POWER_CTL_HEX           8       /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_INT_ENABLE_HEX          9       /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_Z_OFFSET_CONV           10      /* control type: numeric, callback function: CB_Z_OFFSET_CONV */
#define  MAIN_NUM_Y_OFFSET_CONV           11      /* control type: numeric, callback function: CB_Y_OFFSET_CONV */
#define  MAIN_NUM_X_OFFSET_CONV           12      /* control type: numeric, callback function: CB_X_OFFSET_CONV */
#define  MAIN_NUM_TIME_INACT_CONV         13      /* control type: numeric, callback function: CB_TIME_INACT_CONV */
#define  MAIN_NUM_Z_OFFSET_HEX            14      /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_THRESH_INACT_CONV       15      /* control type: numeric, callback function: CB_THRESH_INACT_CONV */
#define  MAIN_NUM_Y_OFFSET_HEX            16      /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_THRESH_ACT_CONV         17      /* control type: numeric, callback function: CB_THRESH_ACT_CONV */
#define  MAIN_NUM_X_OFFSET_HEX            18      /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_DIV_BW_HZ               19      /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_Window_CONV             20      /* control type: numeric, callback function: CB_Window_CONV */
#define  MAIN_NUM_TIME_INACT_HEX          21      /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_Latent_CONV             22      /* control type: numeric, callback function: CB_Latent_CONV */
#define  MAIN_NUM_THRESH_INACT_HEX        23      /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_DUR_CONV                24      /* control type: numeric, callback function: CB_DUR_CONV */
#define  MAIN_NUM_Z_OFFSET                25      /* control type: numeric, callback function: CB_Z_OFFSET */
#define  MAIN_NUM_Y_OFFSET                26      /* control type: numeric, callback function: CB_Y_OFFSET */
#define  MAIN_NUM_X_OFFSET                27      /* control type: numeric, callback function: CB_X_OFFSET */
#define  MAIN_NUM_THRESH_ACT_HEX          28      /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_THRESH_TAP_CONV         29      /* control type: numeric, callback function: CB_THRESH_TAP_CONV */
#define  MAIN_NUM_Window_HEX              30      /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_BW_RATE                 31      /* control type: numeric, callback function: CB_BW_RATE */
#define  MAIN_NUM_Latent_HEX              32      /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_DATA_FORMAT             33      /* control type: numeric, callback function: CB_DATA_FORMAT */
#define  MAIN_NUM_TIME_INACT              34      /* control type: numeric, callback function: CB_TIME_INACT */
#define  MAIN_NUM_THRESH_INACT            35      /* control type: numeric, callback function: CB_THRESH_INACT */
#define  MAIN_NUM_ORIENT_CONFIG           36      /* control type: numeric, callback function: CB_ORIENT_CONFIG */
#define  MAIN_NUM_THRESH_ACT              37      /* control type: numeric, callback function: CB_THRESH_ACT */
#define  MAIN_NUM_ACT_INACT_CTL           38      /* control type: numeric, callback function: CB_ACT_INACT_CTL */
#define  MAIN_NUM_DUR_HEX                 39      /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_TAP_AXES                40      /* control type: numeric, callback function: CB_TAP_AXES */
#define  MAIN_NUM_THRESH_TAP_HEX          41      /* control type: numeric, callback function: (none) */
#define  MAIN_NUM_INT_MAP                 42      /* control type: numeric, callback function: CB_INT_MAP */
#define  MAIN_NUM_POWER_CTL               43      /* control type: numeric, callback function: CB_POWER_CTL */
#define  MAIN_NUM_THRESH_TAP              44      /* control type: numeric, callback function: CB_THRESH_TAP */
#define  MAIN_NUM_INT_ENABLE              45      /* control type: numeric, callback function: CB_INT_ENABLE */
#define  MAIN_NUM_Window                  46      /* control type: numeric, callback function: CB_Window */
#define  MAIN_NUM_Latent                  47      /* control type: numeric, callback function: CB_Latent */
#define  MAIN_NUM_DUR                     48      /* control type: numeric, callback function: CB_DUR */
#define  MAIN_F_MIN                       49      /* control type: numeric, callback function: cb_strip_left */
#define  MAIN_F_MAX                       50      /* control type: numeric, callback function: cb_strip_left */
#define  MAIN_RESULT_X                    51      /* control type: numeric, callback function: (none) */
#define  MAIN_RESULT_Z                    52      /* control type: numeric, callback function: (none) */
#define  MAIN_RESULT_Y                    53      /* control type: numeric, callback function: (none) */
#define  MAIN_RING_CONFIG_FILE            54      /* control type: ring, callback function: CB_CONFIG_FILE */
#define  MAIN_LOG_MODE                    55      /* control type: ring, callback function: cbEnableLog */
#define  MAIN_LED_RX_MESSAGE_ERR          56      /* control type: LED, callback function: (none) */
#define  MAIN_LED_INT2                    57      /* control type: LED, callback function: (none) */
#define  MAIN_LED_INACT                   58      /* control type: LED, callback function: (none) */
#define  MAIN_LED_ACT                     59      /* control type: LED, callback function: (none) */
#define  MAIN_LED_INT1                    60      /* control type: LED, callback function: (none) */
#define  MAIN_CONF_FILENAME               61      /* control type: string, callback function: cb_conf_filename */
#define  MAIN_LED_RX_MESSAGE              62      /* control type: LED, callback function: (none) */
#define  MAIN_LOGFILENAME                 63      /* control type: string, callback function: cb_logfilename */
#define  MAIN_BUTTON_CLEAR                64      /* control type: command, callback function: CB_CLEAR */
#define  MAIN_BUT_DOIT                    65      /* control type: command, callback function: cb_transfer_command */
#define  MAIN_TXT_DATA                    66      /* control type: string, callback function: (none) */
#define  MAIN_TXT_ADDRESS                 67      /* control type: string, callback function: (none) */
#define  MAIN_CHECK_WRITE                 68      /* control type: radioButton, callback function: (none) */
#define  MAIN_STIM_PROG_DISPLAY           69      /* control type: string, callback function: (none) */
#define  MAIN_STATUS                      70      /* control type: string, callback function: (none) */
#define  MAIN_UPDATE_REGS                 71      /* control type: command, callback function: CB_update_regs */
#define  MAIN_LED_V3                      72      /* control type: LED, callback function: (none) */
#define  MAIN_LED_V2                      73      /* control type: LED, callback function: (none) */
#define  MAIN_LED_Z_TAP                   74      /* control type: LED, callback function: (none) */
#define  MAIN_LED_Y_TAP                   75      /* control type: LED, callback function: (none) */
#define  MAIN_LED_X_TAP                   76      /* control type: LED, callback function: (none) */
#define  MAIN_LED_DONT_CARE_3             77      /* control type: LED, callback function: (none) */
#define  MAIN_LED_Z_SIGN                  78      /* control type: LED, callback function: (none) */
#define  MAIN_LED_Y_SIGN                  79      /* control type: LED, callback function: (none) */
#define  MAIN_LED_DONT_CARE_2             80      /* control type: LED, callback function: (none) */
#define  MAIN_LED_X_SIGN                  81      /* control type: LED, callback function: (none) */
#define  MAIN_LED_TAP_Z_SOURCE            82      /* control type: LED, callback function: (none) */
#define  MAIN_LED_TAP_Y_SOURCE            83      /* control type: LED, callback function: (none) */
#define  MAIN_LED_TAP_X_SOURCE            84      /* control type: LED, callback function: (none) */
#define  MAIN_LED_ASLEEP                  85      /* control type: LED, callback function: (none) */
#define  MAIN_LED_ACT_Z_SOURCE            86      /* control type: LED, callback function: (none) */
#define  MAIN_LED_ACT_Y_SOURCE            87      /* control type: LED, callback function: (none) */
#define  MAIN_LED_DONT_CARE               88      /* control type: LED, callback function: (none) */
#define  MAIN_LED_ACT_X_SOURCE            89      /* control type: LED, callback function: (none) */
#define  MAIN_LED_OVERRUN_ORIENT_ON       90      /* control type: LED, callback function: (none) */
#define  MAIN_LED_WATERMARK_ON            91      /* control type: LED, callback function: (none) */
#define  MAIN_LED_FREE_FALL_ON            92      /* control type: LED, callback function: (none) */
#define  MAIN_LED_INACTIVITY_ON           93      /* control type: LED, callback function: (none) */
#define  MAIN_LED_ACTIVITY_ON             94      /* control type: LED, callback function: (none) */
#define  MAIN_LED_DOUBLE_TAP_ON           95      /* control type: LED, callback function: (none) */
#define  MAIN_LED_DATA_READY_ON           96      /* control type: LED, callback function: (none) */
#define  MAIN_LED_SINGLE_TAP_ON           97      /* control type: LED, callback function: (none) */
#define  MAIN_TOGGLE_MEAS_STREAM          98      /* control type: textButton, callback function: CB_meas_stream */
#define  MAIN_STRIPCHART                  99      /* control type: strip, callback function: (none) */
#define  MAIN_TIMER_LED_INT1              100     /* control type: timer, callback function: CB_TIMER_LED_INT1 */
#define  MAIN_TIMER_LED_NAK               101     /* control type: timer, callback function: CB_TIMER_LED_NAK */
#define  MAIN_TIMER_LED_ACK               102     /* control type: timer, callback function: CB_TIMER_LED_ACK */
#define  MAIN_TIMER_LED_INT2              103     /* control type: timer, callback function: CB_TIMER_LED_INT2 */
#define  MAIN_WATERMARK_MAP               104     /* control type: radioButton, callback function: CB_WATERMARK_MAP */
#define  MAIN_FREE_FALL_MAP               105     /* control type: radioButton, callback function: CB_FREE_FALL_MAP */
#define  MAIN_INACTIVITY_MAP              106     /* control type: radioButton, callback function: CB_INACTIVITY_MAP */
#define  MAIN_ACTIVITY_MAP                107     /* control type: radioButton, callback function: CB_ACTIVITY_MAP */
#define  MAIN_DOUBLE_TAP_MAP              108     /* control type: radioButton, callback function: CB_DOUBLE_TAP_MAP */
#define  MAIN_OVR_ORIENT_MAP              109     /* control type: radioButton, callback function: CB_OVR_ORIENT_MAP */
#define  MAIN_SINGLE_TAP_MAP              110     /* control type: radioButton, callback function: CB_SINGLE_TAP_MAP */
#define  MAIN_DTA_RDY_MAP                 111     /* control type: radioButton, callback function: CB_DTA_RDY_MAP */
#define  MAIN_Y_TAP_EN                    112     /* control type: radioButton, callback function: CB_Y_TAP_EN */
#define  MAIN_X_TAP_EN                    113     /* control type: radioButton, callback function: CB_X_TAP_EN */
#define  MAIN_SUPPRESS_EN                 114     /* control type: radioButton, callback function: CB_SUPPRESS_EN */
#define  MAIN_IMPROVE_TAP_EN              115     /* control type: radioButton, callback function: CB_IMPROVE_TAP_EN */
#define  MAIN_DONT_CARE_3                 116     /* control type: radioButton, callback function: (none) */
#define  MAIN_ACT_INACT_SOFT_EN           117     /* control type: radioButton, callback function: CB_ACT_INACT_SOFT_EN */
#define  MAIN_Z_TAP_EN                    118     /* control type: radioButton, callback function: CB_Z_TAP_EN */
#define  MAIN_DONT_CARE_2                 119     /* control type: radioButton, callback function: (none) */
#define  MAIN_DONT_CARE_1                 120     /* control type: radioButton, callback function: (none) */
#define  MAIN_INACT_Z_EN                  121     /* control type: radioButton, callback function: CB_INACT_Z_EN */
#define  MAIN_INACT_Y_EN                  122     /* control type: radioButton, callback function: CB_INACT_Y_EN */
#define  MAIN_INACT_X_EN                  123     /* control type: radioButton, callback function: CB_INACT_X_EN */
#define  MAIN_INACT_AC_DC                 124     /* control type: radioButton, callback function: CB_INACT_AC_DC */
#define  MAIN_ACT_Z_EN                    125     /* control type: radioButton, callback function: CB_ACT_Z_EN */
#define  MAIN_ACT_Y_EN                    126     /* control type: radioButton, callback function: CB_ACT_Y_EN */
#define  MAIN_ACT_X_EN                    127     /* control type: radioButton, callback function: CB_ACT_X_EN */
#define  MAIN_FULL_RES_EN                 128     /* control type: radioButton, callback function: CB_FULL_RES_EN */
#define  MAIN_SELF_TEST_EN                129     /* control type: radioButton, callback function: CB_SELF_TEST_EN */
#define  MAIN_LOW_POWER_EN                130     /* control type: radioButton, callback function: CB_LOW_POWER_EN */
#define  MAIN_INT_ORIENT                  131     /* control type: radioButton, callback function: CB_INT_ORIENT */
#define  MAIN_SLEEP                       132     /* control type: radioButton, callback function: CB_SLEEP */
#define  MAIN_MEASURE_ON                  133     /* control type: radioButton, callback function: CB_MEASURE_ON */
#define  MAIN_AUTO_SLEEP                  134     /* control type: radioButton, callback function: CB_AUTO_SLEEP */
#define  MAIN_LINK_BIT                    135     /* control type: radioButton, callback function: CB_LINK_BIT */
#define  MAIN_ACT_AC_DC                   136     /* control type: radioButton, callback function: CB_ACT_AC_DC */
#define  MAIN_INT_3D                      137     /* control type: radioButton, callback function: CB_INT_3D */
#define  MAIN_WATERMARK_INT               138     /* control type: radioButton, callback function: CB_WATERMARK_INT */
#define  MAIN_FREE_FALL_INT               139     /* control type: radioButton, callback function: CB_FREE_FALL_INT */
#define  MAIN_INACTIVITY_INT              140     /* control type: radioButton, callback function: CB_INACTIVITY_INT */
#define  MAIN_ACTIVITY_INT                141     /* control type: radioButton, callback function: CB_ACTIVITY_INT */
#define  MAIN_DOUBLE_TAP_INT              142     /* control type: radioButton, callback function: CB_DOUBLE_TAP_INT */
#define  MAIN_OVR_ORIENT_INT              143     /* control type: radioButton, callback function: CB_OVR_ORIENT_INT */
#define  MAIN_SINGLE_TAP_INT              144     /* control type: radioButton, callback function: CB_SINGLE_TAP_INT */
#define  MAIN_DTA_RDY_INT                 145     /* control type: radioButton, callback function: CB_DTA_RDY_INT */
#define  MAIN_TEXTMSG                     146     /* control type: textMsg, callback function: (none) */
#define  MAIN_RING_WAKEUP_FREQ            147     /* control type: ring, callback function: CB_WAKEUP_FREQ */
#define  MAIN_RING_OUTPUT_RATE            148     /* control type: ring, callback function: CB_OUTPUT_RATE */
#define  MAIN_RING_RANGE                  149     /* control type: ring, callback function: CB_RANGE */
#define  MAIN_RING_DEAD_ZONE              150     /* control type: ring, callback function: CB_DEAD_ZONE */
#define  MAIN_RING_ORIENT_DIV             151     /* control type: ring, callback function: CB_ORIENT_DIV */
#define  MAIN_PIC_RING_3D_ORIENT          152     /* control type: pictRing, callback function: (none) */
#define  MAIN_PIC_RING_2D_ORIENT          153     /* control type: pictRing, callback function: (none) */
#define  MAIN_DECORATION                  154     /* control type: deco, callback function: (none) */
#define  MAIN_DECORATION_7                155     /* control type: deco, callback function: (none) */
#define  MAIN_DECORATION_2                156     /* control type: deco, callback function: (none) */
#define  MAIN_TIMER_TIMEOUT               157     /* control type: timer, callback function: CB_TIMER_TIMEOUT */
#define  MAIN_DECORATION_9                158     /* control type: deco, callback function: (none) */
#define  MAIN_DECORATION_5                159     /* control type: deco, callback function: (none) */
#define  MAIN_TIMER_ACTIVITY              160     /* control type: timer, callback function: CB_TIMER_ACTIVITY */
#define  MAIN_ENG_DEPT_LOGO               161     /* control type: picture, callback function: (none) */
#define  MAIN_UNIT_LOGO                   162     /* control type: picture, callback function: (none) */
#define  MAIN_PICOSTIM                    163     /* control type: picture, callback function: (none) */
#define  MAIN_TIMER_INACTIVITY            164     /* control type: timer, callback function: CB_TIMER_INACTIVITY */
#define  MAIN_TEXT_ADDR                   165     /* control type: textMsg, callback function: (none) */
#define  MAIN_DECORATION_4                166     /* control type: deco, callback function: (none) */
#define  MAIN_DYNEUMO1_LOGO               167     /* control type: picture, callback function: (none) */
#define  MAIN_V3_SLIDE                    168     /* control type: slide, callback function: (none) */
#define  MAIN_V2_SLIDE                    169     /* control type: slide, callback function: (none) */
#define  MAIN_DECORATION_8                170     /* control type: deco, callback function: (none) */
#define  MAIN_RING_THRS_DISP              171     /* control type: ring, callback function: CB_RING_THRS_DISP */

#define  SELECT_COM                       2
#define  SELECT_COM_COM_BAUD              2       /* control type: ring, callback function: (none) */
#define  SELECT_COM_COM_SELECT            3       /* control type: ring, callback function: (none) */
#define  SELECT_COM_COM_CLOSE             4       /* control type: command, callback function: cb_com_close */
#define  SELECT_COM_COM_OPEN_CLOSE_PORT   5       /* control type: textButton, callback function: CB_COM_Open_Close_Port */
#define  SELECT_COM_DECORATION_2          6       /* control type: deco, callback function: (none) */


     /* Control Arrays: */

#define  LEFT_CTRL_ARRAY                  1
#define  RIGHT_CTRL_ARRAY                 2
#define  TOP_CTRL_ARRAY                   3

     /* Menu Bars, Menus, and Menu Items: */

#define  MENU                             1
#define  MENU_FILE                        2
#define  MENU_FILE_EXIT                   3       /* callback function: CB_EXIT */
#define  MENU_OPTIONS                     4
#define  MENU_OPTIONS_CFG_COM             5       /* callback function: CB_CFG_COM */
#define  MENU_OPTIONS_SMLSCRNSIZE         6       /* callback function: CB_SCRNSIZE */


     /* Callback Prototypes: */

int  CVICALLBACK CB_ACT_AC_DC(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_ACT_INACT_CTL(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_ACT_INACT_SOFT_EN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_ACT_X_EN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_ACT_Y_EN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_ACT_Z_EN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_ACTIVITY_INT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_ACTIVITY_MAP(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_AUTO_SLEEP(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_BW_RATE(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK CB_CFG_COM(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK CB_CLEAR(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_com_close(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_COM_Open_Close_Port(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_conf_filename(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_CONFIG_FILE(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_DATA_FORMAT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_DEAD_ZONE(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_DOUBLE_TAP_INT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_DOUBLE_TAP_MAP(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_DTA_RDY_INT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_DTA_RDY_MAP(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_DUR(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_DUR_CONV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK CB_EXIT(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK CB_FREE_FALL_INT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_FREE_FALL_MAP(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_FULL_RES_EN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_IMPROVE_TAP_EN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_INACT_AC_DC(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_INACT_X_EN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_INACT_Y_EN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_INACT_Z_EN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_INACTIVITY_INT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_INACTIVITY_MAP(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_INT_3D(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_INT_ENABLE(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_INT_MAP(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_INT_ORIENT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_Latent(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_Latent_CONV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_LINK_BIT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_logfilename(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_LOW_POWER_EN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_Main(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_meas_stream(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_MEASURE_ON(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_ORIENT_CONFIG(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_ORIENT_DIV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_OUTPUT_RATE(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_OVR_ORIENT_INT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_OVR_ORIENT_MAP(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_POWER_CTL(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_RANGE(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_RING_THRS_DISP(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK CB_SCRNSIZE(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK CB_SELF_TEST_EN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_SINGLE_TAP_INT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_SINGLE_TAP_MAP(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_SLEEP(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_strip_left(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_SUPPRESS_EN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_TAP_AXES(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_THRESH_ACT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_THRESH_ACT_CONV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_THRESH_INACT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_THRESH_INACT_CONV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_THRESH_TAP(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_THRESH_TAP_CONV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_TIME_INACT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_TIME_INACT_CONV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_TIMER_ACTIVITY(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_TIMER_INACTIVITY(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_TIMER_LED_ACK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_TIMER_LED_INT1(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_TIMER_LED_INT2(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_TIMER_LED_NAK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_TIMER_TIMEOUT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_transfer_command(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_update_regs(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_WAKEUP_FREQ(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_WATERMARK_INT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_WATERMARK_MAP(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_Window(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_Window_CONV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_X_OFFSET(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_X_OFFSET_CONV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_X_TAP_EN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_Y_OFFSET(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_Y_OFFSET_CONV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_Y_TAP_EN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_Z_OFFSET(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_Z_OFFSET_CONV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CB_Z_TAP_EN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbEnableLog(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
