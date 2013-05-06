/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  TSTTYPE.H                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB Log Driver typedefs                               */
/*                                                                            */
/*   FUNCTION: This module is the USB Log Driver typedef include file.        */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:  None                                                      */
/*                                                                            */
/*   EXTERNAL REFERENCES:  None                                               */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#define MAX_INIT_MESSAGE_COUNT 2
#define  MAX_STRAT_CMD  0x1F
#define  SELECTOR_MASK  0xFFF8
#define MSG_REPLACEMENT_STRING   1178


#define LOG_BUFFER_SIZE 40960

typedef struct
{
  ULONG ulTime_s;
  ULONG ulTime_ms;
  UCHAR ucCurTime_h;
  UCHAR ucCurTime_m;
  UCHAR ucCurTime_s;
  UCHAR ucCurTime_s100;
}GIS_TIME, * NPGIS_TIME, FAR * PGIS_TIME;

#define  SYSINFO_TIME_MS     4

#if  0
typedef struct
{
  USHORT usMajor;
  USHORT usMinor;
  char far * pszString;
}LOG_LINE, *NPLOG_LINE, FAR *PLOG_LINE;
#endif

// initialization time message IDs
#define  INIT_MESSAGE_LOADED     0
#define  INIT_MESSAGE_NO_USBD    1
#define  INIT_MESSAGE_UNKNOWNKWD 2
#define  INIT_MESSAGE_INVNUMERIC 3
#define  INIT_MESSAGE_NOT_ALLOC  4
