/* $ID$ */

/* Copyright 2001,2002 Markus Montkowski                                      */
/* This code is released under the Aladdin Free Public License                */
/*                    (Version 9, September 18, 2000)                         */
/* See License file in root of CVS for details                                */

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME: RESPROTO.H                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME: USB Resource Manager Device Driver function prototypes */
/*                                                                            */
/*   FUNCTION: This module is the USB Resource Manager Device Class Driver    */
/*             function prototype include file.                               */
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
/*  Mark      yy/mm/dd  Programmer    Comment                                 */
/*  ------    --------  ----------    -------                                 */
/*            00/01/14  MM                                                    */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/
/*
      RESDATA.C
*/
void (*gStratList[])(PRPH pRP);
void (*gResFunc[])(PRP_GENIOCTL pRP);
/*
         RESSTRAT.C                       Cmd defined in DEVCMD.H
*/
void far Strategy (void);
void     CmdError (PRPH pRP);          // Unknown



void     Open (PRPH pRP);              // CMDOpen           = 13 = 0x0D
void     Close (PRPH pRP);             // CMDClose          = 14 = 0x0E
void     IOCtl (PRPH pRP);             // CMDGenIOCTL       = 16 = 0x10
void     InitComplete (PRPH pRP);      // CMDInitComplete   = 31 = 0x1F
void     InputStatus (PRPH pRP);       // CMDInputS         =  6
void     InputFlush (PRPH pRP);        // CMDInputF         =  7


void     OutputStatus (PRPH pRP);      // CMDOutputS        = 10 = 0x0A
void     OutputFlush (PRPH pRP);       // CMDOutputF        = 11 = 0x0B

/*
         RESREQ.C
*/
#if 0
void WriteData (USHORT usDevIndex);
void ReadData (USHORT usDevIndex);
#endif
void CancelRequests (PDEVICELIST pDevice, UCHAR ucEndPoint);

UCHAR IntGetPipeAddr( UCHAR FAR *configurationData, UCHAR bNumConfigurations,
                      UCHAR configurationValue, UCHAR interface, UCHAR altInterface,
                      UCHAR type, UCHAR typeMask, UCHAR attributes, USHORT FAR* wLength);


/*
         RESINIT.C
*/
void     Init (RPH FAR *pRP);          // CMDInit           = 0
/*
         RESIDC.C
*/
void far IDCEntry (PRP_GENIOCTL pRP_GENIOCTL);
/*
     RESIOCTL.C                     Category A0, Function
*/
void FuncError (PRP_GENIOCTL pRP);              // Unknown
void GetNumDevices (PRP_GENIOCTL pRP);          // 0x31
void GetDeviceInfo (PRP_GENIOCTL pRP);          // 0x32
void AquireDevice(PRP_GENIOCTL pRP);            // 0x33
void ReleaseDevice(PRP_GENIOCTL pRP);           // 0x34
void GetString(PRP_GENIOCTL pRP);               // 0x35
void SendControlURB(PRP_GENIOCTL pRP);          // 0x36
void SendBulkURB(PRP_GENIOCTL pRP);             // 0x37
void SendIRQURB(PRP_GENIOCTL pRP);              // 0x38
void CancelIoRB(PRP_GENIOCTL pRP);              // 0x3D

void  RegisterStatusSemaphore (PRP_GENIOCTL pRP);     // 0x41
void  DeregisterStatusSemaphore (PRP_GENIOCTL pRP);   // 0x42
void  RegisterDevSemaphore (PRP_GENIOCTL pRP);        // 0x43
void  DeregisterDevSemaphore (PRP_GENIOCTL pRP);      // 0x44

void GetStringD (SHORT DevIndex, ULONG ulLockHandle);
void ProcessIoRB(PUSB_IORB pIoRB);


USHORT VerifyParam (PRP_GENIOCTL pRP, USHORT packetLength);
USHORT VerifyData (PRP_GENIOCTL pRP, USHORT dataLength);

/*
   USHORT APIENTRY DevHelp_Yield(); in DHCALLS.H causes
   warning C4071: 'DevHelp_Yield' : no function prototype given
*/
USHORT APIENTRY DevHelp_Yield(void);

