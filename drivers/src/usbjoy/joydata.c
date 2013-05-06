/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  JOYDATA.C                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB Joystick driver data segment                      */
/*                                                                            */
/*   FUNCTION: This module allocates the global data area for the             */
/*             USB device driver.                                             */
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
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          00/01/04  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#include "joy.h"

/*
    Variables
*/
HDRIVER  ghDriver = NULL;     // global handle to Driver
PRPH     pRP = NULL;          // pointer to Request Packet (Header)
PFN      Device_Help = NULL;  // pointer to DevHlp routines

IDCTABLE gIDCTable = {{ 0, 0, 0}, 0, 0}; // structure used by DevHlp AttachDD
IDCTABLE gIDCGameMgr = {{ 0, 0, 0}, 0, 0}; // structure used by DevHlp AttachDD

PUSBIDCEntry gpHIDIDC  = NULL;
USHORT       gdsHIDIDC = NULL;
PUSBIDCEntry gpGameIDC  = NULL;
USHORT       gdsGameIDC = NULL;

USHORT g_usGameDriverId;
/*
    GLOBAL VARS FOR RM. RM.LIB needs these declared
*/
ULONG RMFlags  = 0L;
PFN   RM_Help0 = 0L;
PFN   RM_Help3 = 0L;

DRIVERSTRUCT gDriverStruct = // Driver Description
{
   gDDName,                    // DrvrName
   gDDDesc,                    // DrvrDescript
   gVendorID,                  // VendorName
   CMVERSION_MAJOR,            // MajorVer
   CMVERSION_MINOR,            // MinorVer
   2000,1,4,                   // Date
   DRF_STATIC,                 // DrvrFlags
   DRT_OS2,                    // DrvrType
   DRS_CHAR,                   // DrvrSubType
   NULL                        // DrvrCallback
};

ADAPTERSTRUCT gAdapterStruct = // Adapter Description
{
   gAdapterName,                 // AdaptDescriptName
   AS_NO16MB_ADDRESS_LIMIT,      // AdaptFlags
   AS_BASE_INPUT,                // BaseType
   AS_SUB_DIGIT,                 // SubType
   AS_BASE_COMM,                 // InterfaceType
   NULL,                         // HostBusType
   AS_BUSWIDTH_32BIT,            // HostBusWidth
   NULL,                         // pAdjunctList
   NULL                          // reserved
};
HADAPTER ghAdapter      = NULL; // global handle to RM adapter

BYTE gRightMask[] = { 0xFF, 0x7F, 0x3F, 0x1F,
                       0x0F, 0x07, 0x03, 0x01};

BYTE gBitMask[] = {0x80, 0x40, 0x20,0x10,
                   0x08, 0x04, 0x02,0x01};

JOYList gJOY[MAX_JOYS]; // Joystick list
USHORT  gNoOfJOYs;

#ifdef DEBUG
USHORT gUJOYMsg = DBG_CRITICAL;      // debug message level
#endif

USHORT gJoyIndex      = FULL_WORD;
BYTE   gDevice        = NULL;
BYTE   gInitDataStart = NULL;        // END OF THE DATA SEGMENT
BYTE   gVerbose       = NULL;

USHORT gMessageIDs[MAX_INIT_MESSAGE_COUNT];
USHORT gMessageCount = 0;
PSZ    gVMessages[] = {"IUSBJOY.SYS: USB Joystick Client V.%dd.%dd installed (/DEVICE:%dd)",
   "EUSBJOY.SYS: USB HID Driver not found",
   "EUSBJOY.SYS: Invalid numeric value in CONFIG.SYS line at column %dd",
   "EUSBJOY.SYS: Invalid parameter in CONFIG.SYS line at column %dd",
   "EUSBJOY.SYS: Legacy Joystick Driver not found",
   "EUSBJOY.SYS: Legacy Gameport Driver not found"
};

#define MSG_REPLACEMENT_STRING  1178

MSGTABLE gInitMsg = {MSG_REPLACEMENT_STRING, 1, 0}; //  structure used to write out message during initialization

PDDD_PARM_LIST pDDD_Parm_List = { 0};

