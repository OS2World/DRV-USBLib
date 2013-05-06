/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  GENDATA.C                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:  Game Device Manager driver data segment               */
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

#include "gen.h"

/*
    Variables
*/
HDRIVER  ghDriver = NULL;     // global handle to Driver
PFN      Device_Help = NULL;  // pointer to DevHlp routines

void (*gStratList[])() =
{
   GameInit,         // 0x00  initialize device driver
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
   GameOpen,         // 0x0D  open
   GameClose,        // 0x0E  close
   CmdError,         // 0x0F  removable media
   GameIOCTL,        // 0x10  generic IOCTL
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
   CmdError          // 0x1F  initialization complete
};
#define GAME_GET_VERSION                0x01
#define GAME_GET_PARMS                  0x02
#define GAME_SET_PARMS                  0x03
#define GAME_GET_CALIB                  0x04
#define GAME_SET_CALIB                  0x05
#define GAME_GET_DIGSET                 0x06
#define GAME_SET_DIGSET                 0x07
#define GAME_GET_STATUS                 0x10
#define GAME_GET_STATUS_BUTWAIT         0x11
#define GAME_GET_STATUS_SAMPWAIT        0x12

void (*gResFunc[])() =
{
   GetVersion,          // 0x01
   GetParam,            // 0x02
   SetParam,            // 0x03
   GetCalib,            // 0x04
   SetCalib,            // 0x05
   GetDigSet,           // 0x06
   SetDigSet,           // 0x07
   FuncError,           // 0x08
   FuncError,           // 0x09
   FuncError,           // 0x0A
   FuncError,           // 0x0B
   FuncError,           // 0x0C
   FuncError,           // 0x0D
   FuncError,           // 0x0E
   FuncError,           // 0x0F
   GetStatus,           // 0x10
   GetStatusButWait,    // 0x11
   GetStatusSampWait,   // 0x12
   FuncError,           // 0x13
   FuncError,           // 0x14
   FuncError,           // 0x15
   FuncError,           // 0x16
   FuncError,           // 0x17
   FuncError,           // 0x18
   FuncError,           // 0x19
   FuncError,           // 0x1A
   FuncError,           // 0x1B
   FuncError,           // 0x1C
   FuncError,           // 0x1D
   FuncError,           // 0x1E
   FuncError,           // 0x1F
   SwitchToAdvanced,    // 0x20
   GetNumDevices,       // 0x21
   GetDeviceString,     // 0x22
   SelectDevice,        // 0x23
   GetDevCaps,          // 0x24
   GetJoyState,         // 0x25
   GetDeadZone,         // 0x26
   SetDeadZone          // 0x27
};

IDCTABLE gIDCTable = {{ 0, 0, 0}, 0, 0}; // structure used by DevHlp AttachDD

PUSBIDCEntry gpHIDIDC  = NULL;
USHORT       gdsHIDIDC = NULL;


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

SHORT   gsNumComp20Device; // Index Number of 2.0 driver compatibility device -1 = NONE
USHORT  gusNumDev20Opens; // Number of Open compatibilities
USHORT  gusNumDevOpens;
USHORT  gNoOfDevices;
USHORT  gNoOfDrivers;
HANDLEENTRY gHandleList[MAX_HANDLES];
GameDevice gDevices[MAX_DEVICES];
DriverList gDrivers[MAX_DRIVERS];

GAME_V2COMP_DATA gV20Data;

#ifdef DEBUG
USHORT gUGAMEMsg = DBG_CRITICAL;      // debug message level
#endif

USHORT gJoyIndex      = FULL_WORD;

BYTE   gInitDataStart = NULL;        // END OF THE DATA SEGMENT
BYTE   gVerbose       = NULL;

USHORT gMessageIDs[MAX_INIT_MESSAGE_COUNT];
USHORT gMessageCount = 0;
PSZ    gVMessages[] = {"IGENGAME.SYS: Generic Game Device Manager V.%dd.%dd installed",
};

#define MSG_REPLACEMENT_STRING  1178

MSGTABLE gInitMsg = {MSG_REPLACEMENT_STRING, 1, 0}; //  structure used to write out message during initialization

PDDD_PARM_LIST pDDD_Parm_List = { 0};

