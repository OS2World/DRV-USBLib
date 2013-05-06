/* $ID$ */

/* Copyright 2001,2002 Markus Montkowski                                      */
/* This code is released under the Aladdin Free Public License                */
/*                    (Version 9, September 18, 2000)                         */
/* See License file in root of CVS for details                                */

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME: RESDATA.C                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME: USB Resource Manager Data segment                      */
/*                                                                            */
/*   FUNCTION: This module allocates the global data area                     */
/*             for the USB Resource Manager driver.                           */
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

#include "res.h"

UCHAR             gNumDevices;           // Number of attached Devices
USHORT            gListIndex[MAX_DEVICES+1];
DEVICELIST        gUSBDevices[MAX_DEVICES];   // attached Devices array
OFH               gOFH[MAX_OFHS];   // Opened File Handle array
ULONG             gSEMNewDev[MAX_SEMS];      // Semaphore array
ULONG             gSEMDevRemove[MAX_SEMS];   // Semaphore array
DEVSEMAPHOREENTRY gDeviceSEM[MAX_SEMS];
USHORT            gusHandleCounter;
ULONG             gulSemStringBuffer;
USHORT            gusStringBuffer[257];

USHORT            gusCheck;
/*
      Dispatch table for strategy commands
*/
void (*gStratList[])() =
{
   Init,             // 0x00  initialize device driver
   CmdError,         // 0x01  check the media
   CmdError,         // 0x02  build BPB
   CmdError,         // 0x03  reserved
   CmdError,         // 0x04  read
   CmdError,         // 0x05  non-destructive read
   CmdError,         // 0x06  input status
   CmdError,         // 0x07  input flush
   CmdError,         // 0x08  write
   CmdError,         // 0x09  write with verify
   CmdError,         // 0x0A  output status
   CmdError,         // 0x0B  flush output
   CmdError,         // 0x0C  reserved
   Open,             // 0x0D  open
   Close,            // 0x0E  close
   CmdError,         // 0x0F  removable media
   IOCtl,            // 0x10  generic IOCTL
   CmdError,         // 0x11  reset uncertain media
   CmdError,         // 0x12  get Logical Drive Map
   CmdError,         // 0x13  set Logical Drive Map
   CmdError,         // 0x14  de-Install this device
   CmdError,         // 0x15  reserved
   CmdError,         // 0x16  get number of partitions
   CmdError,         // 0x17  get unit map
   CmdError,         // 0x18  no caching read
   CmdError,         // 0x19  no caching write
   CmdError,         // 0x1A  no caching write/verify
   CmdError,         // 0x1B  initialize base device driver
   CmdError,         // 0x1C  reserved for Request List code
   CmdError,         // 0x1D  get driver capabilities
   CmdError,         // 0x1E  reserved
   InitComplete      // 0x1F  initialization complete
};


void (*gResFunc[])() =
{
   GetNumDevices,       // 0x31
   GetDeviceInfo,       // 0x32
   AquireDevice,        // 0x33
   ReleaseDevice,       // 0x34
   GetString,           // 0x35
   SendControlURB,      // 0x36
   SendBulkURB,         // 0x37
   FuncError,           // 0x38
   FuncError,           // 0x39
   FuncError,           // 0x3A
   FuncError,           // 0x3B
   FuncError,           // 0x3C
   CancelIoRB,          // 0x3D
   FuncError,           // 0x3E
   FuncError,           // 0x3F
   FuncError,           // 0x40
   RegisterStatusSemaphore,   // 0x41
   DeregisterStatusSemaphore,  // 0x42
   RegisterDevSemaphore,   // 0x41
   DeregisterDevSemaphore  // 0x42
};

PFN   Device_Help = NULL;  // pointer to DevHlp routines

PUSBIDCEntry   gpUSBDIDC = NULL;
USHORT         gdsUSBDIDC = NULL;
USHORT         gMaxBufferLength = 0;   // max buffer length for bulk USBD transfers

USHORT    gNumIoRB;
ULONG     gulTotalIoRBs;
USB_IORB  gaIoRB[USB_MAX_IORBS];

SEL gSelIoRB[3];

#ifdef DEBUG
USHORT gUSBDevicesMsg = DBG_CRITICAL | DBG_HLVLFLOW | DBG_IRQFLOW |
                        DBG_DETAILED | DBG_SPECIFIC | DBG_DBGSPCL;

#endif
/*---------------------------------------------------------------------------*/
/*                         Initialization Data                               */
/*---------------------------------------------------------------------------*/
BYTE  gInitDataStart = TRUE;
BYTE  gFirstInit     = TRUE;
BYTE  gSetLPT        = TRUE;

IDCTABLE gIDCTable = {{ 0, 0, 0}, 0, 0}; // structure used by DevHlp AttachDD

// Global variables for RM
ULONG RMFlags  = 0L;
PFN   RM_Help0 = 0L;
PFN   RM_Help3 = 0L;

DRIVERSTRUCT gDriverStruct =  // Driver Description
{
   gDDName,                   // DrvrName
   gDDDesc,                   // DrvrDescript
   gVendorID,                 // VendorName
   CMVERSION_MAJOR,           // MajorVer
   CMVERSION_MINOR,           // MinorVer
   1999,12,13,                // Date (year,month,day)
   DRF_STATIC,                // DrvrFlags
   DRT_OS2,                   // DrvrType
   DRS_CHAR,                  // DrvrSubType
   NULL                       // DrvrCallback
};
ADAPTERSTRUCT gAdapterStruct =   // Adapter Description
{
   gAdapterName,                 // AdaptDescriptName
   AS_NO16MB_ADDRESS_LIMIT,      // AdaptFlags
   AS_BASE_COMM,                 // BaseType
   AS_SUB_PARALLEL,              // SubType
   AS_INTF_BIDI,                 // InterfaceType
   NULL,                         // HostBusType
   NULL,                         // HostBusWidth
   NULL,                         // pAdjunctList
   NULL                          // reserved
};

// Global handle variables
HDRIVER  ghDriver = NULL;
HADAPTER ghAdapter = NULL;

USHORT gVerbose       = 0;
USHORT gMessageCount  = 0;
USHORT gMessageIDs[MAX_INIT_MESSAGE_COUNT];
PSZ    gVMessages[] =
{
   "IUSBRESMG.SYS: USB Resource Manager V.%dd.%dd loaded",
   "EUSBRESMG.SYS: USBD Driver not found",
   "EUSBRESMG.SYS: Invalid parameter in CONFIG.SYS line at column %dd",
   "EUSBRESMG.SYS: Invalid numeric value in CONFIG.SYS line at column %dd"
};
// structure used to write out message during initialization
MSGTABLE gInitMsg = {MSG_REPLACEMENT_STRING, 1, 0};

