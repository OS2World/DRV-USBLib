/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  TSTEXTRN.H                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB Log Driver                                        */
/*                      external data declarations                            */
/*                                                                            */
/*   FUNCTION: This module is the USB Log Driver                              */
/*             external data declarations include file. See tstdata.c         */
/*             for the data items being externalized.                         */
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

#ifdef   DEBUG
extern   USHORT   gLogMsg;  // debug message level
#endif

extern char gDDName[];
extern char gDDDesc[];
extern char gVendorID[];
extern char gAdapterName[];

extern SEL  gGDTSel;
extern ULONG gulReadPos;
extern ULONG gulWritePos;
//extern PGIS_TIME gpTime;
extern PULONG   gpTime;
extern char  gszStamp[];

extern UCHAR gucLogBuffer[];

extern   DRIVERSTRUCT   gDriverStruct;
extern   ADAPTERSTRUCT  gAdapterStruct;

extern   HDRIVER        ghDriver;
extern   HADAPTER       ghAdapter;

extern   PFN            Device_Help;

extern BYTE           gInitDataStart;
extern USHORT         gVerbose;
extern USHORT         gMessageIDs[MAX_INIT_MESSAGE_COUNT];
extern PSZ            gVMessages[];
extern USHORT         gMessageCount;

