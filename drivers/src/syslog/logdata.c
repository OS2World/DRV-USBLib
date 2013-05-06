/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  TSTDATA.C                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB Log Driver data segment                           */
/*                                                                            */
/*   FUNCTION: This module allocates the global data area for the             */
/*             USB Log Driver.                                                */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:                                                            */
/*             None                                                           */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*             None                                                           */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#include "log.h"

/*
   Dispatch table for strategy commands
*/
void (*gStratList[])() =
{
   CmdError,         // 0x00  initialize device driver
   CmdError,         // 0x01  check the media
   CmdError,         // 0x02  build BPB
   CmdError,         // 0x03  reserved
   LogRead,          // 0x04  read
   CmdError,         // 0x05  non-destructive read
   LogInStatus,      // 0x06  input status
   LogInFlush,       // 0x07  input flush
   CmdError,         // 0x08  write
   CmdError,         // 0x09  write with verify
   CmdError,         // 0x0A  output status
   CmdError,         // 0x0B  flush output
   CmdError,         // 0x0C  reserved
   LogOpen,          // 0x0D  open
   LogClose,         // 0x0E  close
   CmdError,         // 0x0F  removable media
   CmdError,         // 0x10  generic IOCTL
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
   LogInit,          // 0x1B  initialize base device driver
   CmdError,         // 0x1C  reserved for Request List code
   CmdError,         // 0x1D  get driver capabilities
   CmdError,         // 0x1E  reserved
   LogInitComplete   // 0x1F  initialization complete
};

/*
    Variables
*/
HDRIVER  ghDriver = NULL;     // global handle to Driver
HADAPTER ghAdapter = NULL;    //   global handle to adapter
PFN      Device_Help = NULL;  // pointer to DevHlp routines
/*
    GLOBAL VARS FOR RM. RM.LIB needs these declared
*/
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
   2000,8,1,                  // Date
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
   AS_SUB_SERIAL,                // SubType
   AS_BASE_COMM,                 // InterfaceType
   NULL,                         // HostBusType
   AS_BUSWIDTH_32BIT,            // HostBusWidth
   NULL,                         // pAdjunctList
   NULL                          // reserved
};


/*----------------------------------------------*/
/* GLOBAL HANDLE VARIABLES                      */
/*                                              */
/* These variables are assigned the handles for */
/* drivers, detected hardware and resources.    */
/*----------------------------------------------*/

//IDCTABLE gIDCTable = {{ 0, 0, 0}, 0, 0}; // structure used by DevHlp AttachDD

//PUSBIDCEntry   gpUSBDIDC = NULL;
//USHORT         gdsUSBDIDC = NULL;

#ifdef DEBUG
USHORT gLogMsg = DBG_CRITICAL | DBG_HLVLFLOW | DBG_IRQFLOW |
                  DBG_DETAILED | DBG_SPECIFIC | DBG_DBGSPCL;

#endif
SEL  gGDTSel;
PULONG   gpTime;  //TM0528 far Pointer to Time in milliseconds in the system info structure
//PGIS_TIME gpTime;
ULONG gulReadPos;
ULONG gulWritePos;
char  gszStamp[30];
UCHAR gucLogBuffer[LOG_BUFFER_SIZE];

BYTE   gInitDataStart = NULL;    // END OF THE DATA SEGMENT

USHORT gVerbose       = NULL;
USHORT gMessageCount  = NULL;
USHORT gMessageIDs[MAX_INIT_MESSAGE_COUNT];
PSZ    gVMessages[] =
{  //                                                                      DrivName
   "ISYSLOG.SYS: USB Driver To Log logging V.%dd.%dd loaded",
   "ESYSLOG.SYS: USBD Driver not found",
   "ESYSLOG.SYS: Invalid parameter in CONFIG.SYS line at column %dd",
   "ESYSLOG.SYS: Invalid numeric value in CONFIG.SYS line at column %dd",
   "ESYSLOG.SYS: I/O queue not allocated"

};
MSGTABLE gInitMsg = {MSG_REPLACEMENT_STRING, 1, 0}; // structure used to write out message during initialization

