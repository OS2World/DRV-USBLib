/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  JOYEXTRN.H                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB Joystick driver external data declarations        */
/*                                                                            */
/*   FUNCTION: This module is the USB Joystick driver external                */
/*             data declarations include file.                                */
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
/*          00/01/04  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#ifdef DEBUG
extern USHORT gUJOYMsg;
#endif

extern PRPH    pRP;
extern PFN     Device_Help;

extern IDCTABLE      gIDCTable;
extern IDCTABLE gIDCGameMgr;
extern PUSBIDCEntry  gpHIDIDC;
extern USHORT        gdsHIDIDC;
extern PUSBIDCEntry  gpGameIDC;
extern USHORT        gdsGameIDC;

extern BYTE gInitDataStart;

extern DRIVERSTRUCT  gDriverStruct;
extern ADAPTERSTRUCT gAdapterStruct;
extern DEVICESTRUCT  gDeviceStruct;

extern HDRIVER    ghDriver;
extern HADAPTER   ghAdapter;
extern HDEVICE    ghDevice;

extern char gDDName[];
extern char gDDDesc[];
extern char gVendorID[];
extern char gAdapterName[];

extern JOYList gJOY[];
extern USHORT  gNoOfJOYs;
extern BYTE gRightMask[];

extern BYTE gBitMask[];

extern BYTE gTypeDelay;
extern BYTE gTypeRate;
extern BYTE gScanCode[];
extern BYTE gVerbose;
extern BYTE gDevice;
extern BYTE gLEDs;

extern USHORT gJoyIndex;
extern USHORT g_usGameDriverId;

extern PDDD_PARM_LIST pDDD_Parm_List;
extern MSGTABLE       gInitMsg;

extern USHORT     gMessageIDs[];
extern PSZ        gVMessages[];
extern USHORT     gMessageCount;

