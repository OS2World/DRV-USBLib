/* $ID$ */

/* Copyright 2001,2002 Markus Montkowski                                      */
/* This code is released under the Aladdin Free Public License                */
/*                    (Version 9, September 18, 2000)                         */
/* See License file in root of CVS for details                                */

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME: PRTEXTRN.H                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME: USB Printer Device Driver external data declarations   */
/*                                                                            */
/*   FUNCTION: This module is the USB Printer Device Driver external data     */
/*             declarations include file. See PRTDATA.C for the data items    */
/*             being externalized.                                            */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS: None                                                       */
/*                                                                            */
/*   EXTERNAL REFERENCES: None                                                */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          00/01/14  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

extern  UCHAR      gNumDevices;    // Number of attached Printers
extern  USHORT     gListIndex[];   // Index list of used DeviceList entries
extern  DEVICELIST gUSBDevices[];    // attached Printer array
extern  OFH        gOFH[];    // Opened File Handle array
extern  ULONG      gSEMNewDev[];    // Semaphore array
extern  ULONG      gSEMDevRemove[];
extern  USHORT     gusHandleCounter;
extern  ULONG      gulSemStringBuffer;
extern  USHORT     gusStringBuffer[257];
extern  DEVSEMAPHOREENTRY gDeviceSEM[MAX_SEMS];

extern  USHORT            gusCheck;


extern  USHORT    gNumIoRB;
extern  USB_IORB  gaIoRB[];
extern  ULONG    gulTotalIoRBs;
extern  SEL gSelIoRB[3];

extern  WORD     gHead;
extern  PFN      Device_Help;

extern  PUSBIDCEntry   gpUSBDIDC;
extern  USHORT         gdsUSBDIDC;

#ifdef  DEBUG
extern  USHORT   gUSBDevicesMsg;       // debug message level
#endif
/*---------------------------------------------------------------------------*/
/*                         Initialization Data                               */
/*---------------------------------------------------------------------------*/
extern   BYTE  gInitDataStart;
extern   BYTE  gFirstInit;
extern   BYTE  gSetLPT;

extern   IDCTABLE       gIDCTable;

extern   char  gDDName[];
extern   char  gDDDesc[];
extern   char  gVendorID[];
extern   char  gAdapterName[];

extern   DRIVERSTRUCT   gDriverStruct;
extern   ADAPTERSTRUCT  gAdapterStruct;

extern   HDRIVER  ghDriver;
extern   HADAPTER ghAdapter;

extern   USHORT   gVerbose;
extern   USHORT   gMessageIDs[MAX_INIT_MESSAGE_COUNT];
extern   PSZ      gVMessages[];
extern   USHORT   gMessageCount;

