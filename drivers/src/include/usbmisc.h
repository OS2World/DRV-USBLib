#ifndef  _usbmisc_h_
   #define  _usbmisc_h_
/* SCCSID = "src/dev/usb/INCLUDE/USBMISC.H, usb, c.basedd 98/07/10" */
/*
*   Licensed Material -- Property of IBM
*
*   (c) Copyright IBM Corp. 1997, 1998  All Rights Reserved
*/
/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  USBMISC.H                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:  Commom USB device driver stack function prototypes    */ 
/*                      defined in usbmisc.lib                                */
/*                                                                            */
/*   FUNCTION: Common USB device driver stack function prototypes.            */
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
/*          98/02/02  MB                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

   #include "usbcmmon.h"
   #include "usbchid.h"

/* USBMISC.C */
VOID NEAR setmem(PSZ d, USHORT c, USHORT len );
VOID NEAR movmem(PSZ d, PSZ s, USHORT len );
VOID ConvertCharToStr( UCHAR source, PSZ target );
USHORT LenStr(PSZ String);
USHORT NEAR GetDS(VOID);
USHORT NEAR CLISave(VOID);
VOID NEAR STIRestore(USHORT flags);

/* INITCMM.C */
VOID TTYWrite( PSZ *Buf, USHORT msgIds[], USHORT msgCount );
USHORT AddToMsgArray( USHORT msgIds[], USHORT msgIndex, USHORT currCount, USHORT maxCount );
VOID SetLongValue( PSZ Buf, ULONG value );

typedef struct _KEYDATA
{
   CHAR     *key;
   CHAR     type;
   CHAR     keyStatus;
   LONG     value;
} KeyData;

ULONG ProcessConfigString(PSZ confLine, USHORT keyCount, KeyData FAR *keyData);
   #define  CFSTR_TYPE_STRING       1
   #define  CFSTR_TYPE_DEC          2
   #define  CFSTR_STATUS_OK         1
   #define  CFSTR_STATUS_NOTFOUND   2
   #define  CFSTR_STATUS_CONVERR    3
   #define  CFSTR_STATUS_NOVALUE    4
   #define  CFSTR_RC_OK             0
   #define  CFSTR_UNKWN_KEYS        1
   #define  CFSTR_CONVERR           2

// USB device configuration data processing routines
DeviceDescHead FAR *GetNextDescriptor( DeviceDescHead FAR *currHead, UCHAR  FAR *lastBytePtr);  // 28/09/98 MB
UCHAR GetInterruptPipeAddr(UCHAR FAR *configurationData, UCHAR bNumConfigurations, UCHAR configurationValue, UCHAR interface);
UCHAR GetPipeAddr(UCHAR FAR *configurationData, UCHAR bNumConfigurations, UCHAR configurationValue,
                  UCHAR interface, UCHAR altInterface, UCHAR type, UCHAR typeMask, UCHAR attributes);  // 23/09/98 MB
UCHAR GetMaxAltIntf(UCHAR FAR *configurationData, UCHAR bNumConfigurations, UCHAR configurationValue,
                    UCHAR interface); // 23/09/98 MB
USHORT GetHIDReportLength(UCHAR FAR *configurationData, UCHAR bNumConfigurations, UCHAR configurationValue, UCHAR interface);
DeviceInterface FAR * GetInterfaceDPtr(UCHAR FAR *configurationData, UCHAR bNumConfigurations, UCHAR configurationValue, UCHAR interface);
DeviceEndpoint FAR *GetEndpointDPtr(UCHAR FAR *configurationData, UCHAR bNumConfigurations,
                                    UCHAR configurationValue, UCHAR altInterface, UCHAR endpointID);  // 27/08/98 MB
BOOL IsDraft3Compl(UCHAR FAR *configurationData, UCHAR bNumConfigurations);

USHORT SearchConfiguration(UCHAR FAR *configurationData, UCHAR bNumConfigurations,   // 21/09/98 MB
                           UCHAR interfaceClass, UCHAR interfaceSubClass, UCHAR interfaceProtocol);

USHORT GetReportLength(PRP_GENIOCTL pRP_GENIOCTL, UCHAR ReportType, UCHAR ReportID, UCHAR Interface );
USHORT GetUsageOffset(PRP_GENIOCTL pRP_GENIOCTL, UCHAR ReportType, UCHAR ReportID, UCHAR interface, USHORT UsagePage, USHORT UsageID);
ULONG GetUsageSize(PRP_GENIOCTL pRP_GENIOCTL, UCHAR ReportType, UCHAR ReportID, UCHAR interface, USHORT UsagePage, USHORT UsageID);
USHORT CheckGUsage( ItemUsage FAR *pUsageData, USHORT IndexToUsageList, USHORT UsageID);
ULONG CheckSUsage(ItemFeatures FAR *pitemFeatures, ItemUsage FAR *pUsageData, USHORT IndexToUsageList, USHORT UsageID);




#endif

