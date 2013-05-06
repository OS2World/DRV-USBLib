#include <os2.h>
#include "usbcalls.h"
#define  INCL_REXXSAA
#include <rexxsaa.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if 0
#ifdef __cplusplus
  extern "C" {
#endif
RexxFunctionHandler RxUsbQueryNumberDevices;
RexxFunctionHandler RxUsbQueryDeviceReport;
RexxFunctionHandler RxUsbRegisterChangeNotification;
RexxFunctionHandler RxUsbRegisterDeviceNotification;
RexxFunctionHandler RxUsbDeregisterNotification;
RexxFunctionHandler RxUsbOpen;
RexxFunctionHandler RxUsbClose;
RexxFunctionHandler RxUsbCtrlMessage;
RexxFunctionHandler RxUsbBulkRead;
RexxFunctionHandler RxUsbBulkWrite;
RexxFunctionHandler RxUsbIrqStart;
RexxFunctionHandler RxUsbIrqStop;
RexxFunctionHandler RxUsbIsoStart;
RexxFunctionHandler RxUsbIsoStop;
RexxFunctionHandler RxUsbIsoDequeue;
RexxFunctionHandler RxUsbIsoPeekQueue;
RexxFunctionHandler RxUsbIsoEnqueue;
RexxFunctionHandler RxUsbIsoGetLength;
RexxFunctionHandler RxUsbDeviceGetConfiguration;
RexxFunctionHandler RxUsbDeviceGetDescriptor;
RexxFunctionHandler RxUsbInterfaceGetDescriptor;
RexxFunctionHandler RxUsbEndpointGetDescriptor;
RexxFunctionHandler RxUsbConfigurationGetDescriptor;
RexxFunctionHandler RxUsbStringGetDescriptor;
RexxFunctionHandler RxUsbInterfaceGetAltSetting;
RexxFunctionHandler RxUsbDeviceGetStatus;
RexxFunctionHandler RxUsbInterfaceGetStatus;
RexxFunctionHandler RxUsbEndpointGetStatus;
RexxFunctionHandler RxUsbDeviceSetAddress;
RexxFunctionHandler RxUsbDeviceSetConfiguration;
RexxFunctionHandler RxUsbDeviceSetDescriptor;
RexxFunctionHandler RxUsbInterfaceSetDescriptor;
RexxFunctionHandler RxUsbEndpointSetDescriptor;
RexxFunctionHandler RxUsbConfigurationSetDescriptor;
RexxFunctionHandler RxUsbStringSetDescriptor;
RexxFunctionHandler RxUsbDeviceSetFeature;
RexxFunctionHandler RxUsbInterfaceSetFeature;
RexxFunctionHandler RxUsbEndpointSetFeature;
RexxFunctionHandler RxUsbInterfaceSetAltSetting;
RexxFunctionHandler RxUsbEndpointSynchFrame;

#ifdef __cplusplus
}//  extern "C" {
#endif
#endif

/*********************************************************************/
/*  Various definitions used by various functions.                   */
/*********************************************************************/

#define  MAX_DIGITS     9          /* maximum digits in numeric arg  */
#define  MAX            256        /* temporary buffer length        */
#define  IBUF_LEN       4096       /* Input buffer length            */
#define  AllocFlag      PAG_COMMIT | PAG_WRITE  /* for DosAllocMem   */

static PSZ  RxFncTable[] =
{
  "RxUsbQueryNumberDevices",
  "RxUsbQueryDeviceReport",
  "RxUsbRegisterChangeNotification",
  "RxUsbRegisterDeviceNotification",
  "RxUsbDeregisterNotification",
  "RxUsbOpen",
  "RxUsbClose",
  "RxUsbCtrlMessage",
  "RxUsbBulkRead",
  "RxUsbBulkWrite",
  "RxUsbIrqStart",
  "RxUsbIrqStop",
  "RxUsbIsoStart",
  "RxUsbIsoStop",
  "RxUsbIsoDequeue",
  "RxUsbIsoPeekQueue",
  "RxUsbIsoEnqueue",
  "RxUsbIsoGetLength",
  "RxUsbDeviceGetConfiguration",
  "RxUsbDeviceGetDescriptor",
  "RxUsbInterfaceGetDescriptor",
  "RxUsbEndpointGetDescriptor",
  "RxUsbConfigurationGetDescriptor",
  "RxUsbStringGetDescriptor",
  "RxUsbInterfaceGetAltSetting",
  "RxUsbDeviceGetStatus",
  "RxUsbInterfaceGetStatus",
  "RxUsbEndpointGetStatus",
  "RxUsbDeviceSetAddress",
  "RxUsbDeviceSetConfiguration",
  "RxUsbDeviceSetDescriptor",
  "RxUsbInterfaceSetDescriptor",
  "RxUsbEndpointSetDescriptor",
  "RxUsbConfigurationSetDescriptor",
  "RxUsbStringSetDescriptor",
  "RxUsbDeviceSetFeature",
  "RxUsbInterfaceSetFeature",
  "RxUsbEndpointSetFeature",
  "RxUsbInterfaceSetAltSetting",
  "RxUsbEndpointSynchFrame",
};

/*********************************************************************/
/* Numeric Error Return Strings                                      */
/*********************************************************************/

#define  NO_UTIL_ERROR    "0"          /* No error whatsoever        */
#define  ERROR_NOMEM      "2"          /* Insufficient memory        */

/*********************************************************************/
/* Alpha Numeric Return Strings                                      */
/*********************************************************************/

#define  ERROR_RETSTR   "ERROR:"

/*********************************************************************/
/* Numeric Return calls                                              */
/*********************************************************************/

#define  INVALID_ROUTINE 40            /* Raise Rexx error           */
#define  VALID_ROUTINE    0            /* Successful completion      */

/*********************************************************************/
/* Some useful macros                                                */
/*********************************************************************/

#define BUILDRXSTRING(t, s) { \
  strcpy((t)->strptr,(s));\
  (t)->strlength = strlen((s)); \
}

/*********************************************************************/
/* RxStemData                                                        */
/*   Structure which describes as generic                            */
/*   stem variable.                                                  */
/*********************************************************************/

typedef struct RxStemData {
    SHVBLOCK shvb;                     /* Request block for RxVar    */
    CHAR ibuf[IBUF_LEN];               /* Input buffer               */
    CHAR varname[MAX];                 /* Buffer for the variable    */
                                       /* name                       */
    CHAR stemname[MAX];                /* Buffer for the variable    */
                                       /* name                       */
    ULONG stemlen;                     /* Length of stem.            */
    ULONG vlen;                        /* Length of variable value   */
    ULONG j;                           /* Temp counter               */
    ULONG tlong;                       /* Temp counter               */
    ULONG count;                       /* Number of elements         */
                                       /* processed                  */
} RXSTEMDATA;

/********************************************************************
* Function:  string2long(string, number)                            *
*                                                                   *
* Purpose:   Validates and converts an ASCII-Z string from string   *
*            form to an unsigned long.  Returns FALSE if the number *
*            is not valid, TRUE if the number was successfully      *
*            converted.                                             *
*                                                                   *
* RC:        TRUE - Good number converted                           *
*            FALSE - Invalid number supplied.                       *
*********************************************************************/

BOOL string2long(PSZ string, LONG *number)
{
  ULONG    accumulator;                /* converted number           */
  INT      length;                     /* length of number           */
  INT      sign;                       /* sign of number             */

  sign = 1;                            /* set default sign           */
  if (*string == '-') {                /* negative?                  */
    sign = -1;                         /* change sign                */
    string++;                          /* step past sign             */
  }

  length = strlen(string);             /* get length of string       */
  if (length == 0 ||                   /* if null string             */
      length > MAX_DIGITS)             /* or too long                */
    return FALSE;                      /* not valid                  */

  accumulator = 0;                     /* start with zero            */

  while (length) {                     /* while more digits          */
    if (!isdigit(*string))             /* not a digit?               */
      return FALSE;                    /* tell caller                */
                                       /* add to accumulator         */
    accumulator = accumulator *10 + (*string - '0');
    length--;                          /* reduce length              */
    string++;                          /* step pointer               */
  }
  *number = accumulator * sign;        /* return the value           */
  return TRUE;                         /* good number                */
}

/********************************************************************
* Function:  string2ulong(string, number)                           *
*                                                                   *
* Purpose:   Validates and converts an ASCII-Z string from string   *
*            form to an unsigned long.  Returns FALSE if the number *
*            is not valid, TRUE if the number was successfully      *
*            converted.                                             *
*                                                                   *
* RC:        TRUE - Good number converted                           *
*            FALSE - Invalid number supplied.                       *
*********************************************************************/

BOOL string2ulong(PSZ string, ULONG *number)
{
  ULONG    accumulator;                /* converted number           */
  INT      length;                     /* length of number           */

  length = strlen(string);             /* get length of string       */
  if (length == 0 ||                   /* if null string             */
      length > MAX_DIGITS)             /* or too long                */
    return FALSE;                      /* not valid                  */

  accumulator = 0;                     /* start with zero            */

  while (length) {                     /* while more digits          */
    if (!isdigit(*string))             /* not a digit?               */
      return FALSE;                    /* tell caller                */
                                       /* add to accumulator         */
    accumulator = accumulator *10 + (*string - '0');
    length--;                          /* reduce length              */
    string++;                          /* step pointer               */
  }
  *number = accumulator;               /* return the value           */
  return TRUE;                         /* good number                */
}

/********************************************************************
* Function:  string2ushort(string, number)                           *
*                                                                   *
* Purpose:   Validates and converts an ASCII-Z string from string   *
*            form to an unsigned long.  Returns FALSE if the number *
*            is not valid, TRUE if the number was successfully      *
*            converted.                                             *
*                                                                   *
* RC:        TRUE - Good number converted                           *
*            FALSE - Invalid number supplied.                       *
*********************************************************************/

BOOL string2ushort(PSZ string, USHORT *number)
{
  ULONG    accumulator;                /* converted number           */
  INT      length;                     /* length of number           */

  length = strlen(string);             /* get length of string       */
  if (length == 0 ||                   /* if null string             */
      length > 5)                      /* or too long                */
    return FALSE;                      /* not valid                  */

  accumulator = 0;                     /* start with zero            */

  while (length) {                     /* while more digits          */
    if (!isdigit(*string))             /* not a digit?               */
      return FALSE;                    /* tell caller                */
                                       /* add to accumulator         */
    accumulator = accumulator *10 + (*string - '0');
    length--;                          /* reduce length              */
    string++;                          /* step pointer               */
  }
  if(accumulator > 1024*64)
    return FALSE;
  *number = accumulator;               /* return the value           */
  return TRUE;                         /* good number                */
}

/********************************************************************
* Function:  string2ushort(string, number)                           *
*                                                                   *
* Purpose:   Validates and converts an ASCII-Z string from string   *
*            form to an unsigned long.  Returns FALSE if the number *
*            is not valid, TRUE if the number was successfully      *
*            converted.                                             *
*                                                                   *
* RC:        TRUE - Good number converted                           *
*            FALSE - Invalid number supplied.                       *
*********************************************************************/

BOOL string2uchar(PSZ string, UCHAR *number)
{
  USHORT   accumulator;                /* converted number           */
  INT      length;                     /* length of number           */

  length = strlen(string);             /* get length of string       */
  if (length == 0 ||                   /* if null string             */
      length > 3)                      /* or too long                */
    return FALSE;                      /* not valid                  */

  accumulator = 0;                     /* start with zero            */

  while (length) {                     /* while more digits          */
    if (!isdigit(*string))             /* not a digit?               */
      return FALSE;                    /* tell caller                */
                                       /* add to accumulator         */
    accumulator = accumulator *10 + (*string - '0');
    length--;                          /* reduce length              */
    string++;                          /* step pointer               */
  }
  if(accumulator > 255)
   return FALSE;
  *number = accumulator;               /* return the value           */
  return TRUE;                         /* good number                */
}


 /*********************************************************************/
 /*                                                                   */
 /* SetRexxVariable - Set the value of a REXX variable                */
 /*                                                                   */
 /*********************************************************************/

INT SetRexxVariable( PSZ        name,  /* REXX variable to set       */
                     PSZ        value) /* value to assign            */
{
  SHVBLOCK   block;                    /* variable pool control block*/

  block.shvcode = RXSHV_SYSET;         /* do a symbolic set operation*/
  block.shvret=(UCHAR)0;               /* clear return code field    */
  block.shvnext=(PSHVBLOCK)0;          /* no next block              */
                                      /* set variable name string   */
  MAKERXSTRING(block.shvname, name, strlen(name));
                                      /* set value string           */
  MAKERXSTRING(block.shvvalue, value, strlen(value));
  block.shvvaluelen=strlen(value);     /* set value length           */
  return RexxVariablePool(&block);     /* set the variable           */
}

/********************************************************************
* Function:  SetRexxVariableToUSHORT(string, number)                  *
*                                                                   *
* Purpose:   Sets a Value of the passed in Variable to the long     *
*                                                                   *
* RC:        TRUE - Good number converted                           *
*            FALSE - Invalid number supplied.                       *
*********************************************************************/
INT SetRexxVariableToUSHORT(PSZ Name, USHORT Value)
{
  char szValue[40];
  sprintf(szValue,"%d",Value);
  return SetRexxVariable(Name,szValue);
}

/********************************************************************
* Function:  SetRexxVariableToUSHORT(string, number)                  *
*                                                                   *
* Purpose:   Sets a Value of the passed in Variable to the long     *
*                                                                   *
* RC:        TRUE - Good number converted                           *
*            FALSE - Invalid number supplied.                       *
*********************************************************************/
INT SetRexxVariableToULONG(PSZ Name, ULONG Value)
{
  char szValue[40];
  sprintf(szValue,"%d",Value);
  return SetRexxVariable(Name,szValue);
}


/********************************************************************
* Function:  SetRexxVariableToLong(string, number)                  *
*                                                                   *
* Purpose:   Sets a Value of the passed in Variable to the long     *
*                                                                   *
* RC:        TRUE - Good number converted                           *
*            FALSE - Invalid number supplied.                       *
*********************************************************************/
INT SetRexxVariableToLong(PSZ Name, LONG Value)
{
  char szValue[40];
  sprintf(szValue,"%d",Value);
  return SetRexxVariable(Name,szValue);
}

/********************************************************************
* Function:  SetRexxVariableToBuffer(string, ULONG, *UCHAR)         *
*                                                                   *
* Purpose:   Creates a HEX-String outof th data in the buffer and   *
*            sets it as the value of the passed in variable         *
*                                                                   *
* RC:        TRUE - Hex String assigned                             *
*            FALSE - Error.                                         *
*********************************************************************/
INT SetRexxVariableToBuffer(PSZ Name, ULONG ulBuffersize, UCHAR *Data)
{
  INT rc;
  ULONG i;
  char *pszValue = (char*)malloc(ulBuffersize*2+1);
  if(NULL==pszValue)
    return 0;
  pszValue[0] = 0;
  for(i=0;i<ulBuffersize;i++)
    sprintf(pszValue+i*2,"%02X",Data[i]);
  rc = SetRexxVariable(Name,pszValue);
  free(pszValue);
  return rc;
}

#ifdef __cplusplus
  extern "C" {
#endif


/*************************************************************************
* Function:  UsbLoadFuncs                                                *
*                                                                        *
* Syntax:    call UsbLoadFuncs                                           *
*                                                                        *
* Return:    NO_UTIL_ERROR - Successful.                                 *
*************************************************************************/
ULONG APIENTRY UsbLoadFuncs( PSZ name,
                             ULONG numargs,
                             RXSTRING args[],
                             PSZ queuename,
                             RXSTRING *retstr)
{
  INT    entries;                      /* Num of entries             */
  INT    j;                            /* Counter                    */

  retstr->strlength = 0;               /* set return value           */
                                       /* check arguments            */
  if (numargs > 0)
    return INVALID_ROUTINE;

  entries = sizeof(RxFncTable)/sizeof(PSZ);

  for (j = 0; j < entries; j++)
  {
    RexxRegisterFunctionDll( RxFncTable[j],
                             "USBCALLS",
                             RxFncTable[j]);
  }
  return VALID_ROUTINE;
}

/*************************************************************************
* Function:  UsbDropFuncs                                                *
*                                                                        *
* Syntax:    call UsbDropFuncs                                           *
*                                                                        *
* Return:    NO_UTIL_ERROR - Successful.                                 *
*************************************************************************/

ULONG APIENTRY UsbDropFuncs( PSZ name,
                             ULONG numargs,
                             RXSTRING args[],
                             PSZ queuename,
                             RXSTRING *retstr)
{
  INT     entries;                     /* Num of entries             */
  INT     j;                           /* Counter                    */

  if (numargs != 0)                    /* no arguments for this      */
    return INVALID_ROUTINE;            /* raise an error             */

  retstr->strlength = 0;               /* return a null string result*/

  entries = sizeof(RxFncTable)/sizeof(PSZ);

  for (j = 0; j < entries; j++)
    RexxDeregisterFunction(RxFncTable[j]);

  return VALID_ROUTINE;                /* no error on call           */
}

/*************************************************************************
* Function:  RxUsbQueryNumberDevices                                     *
*                                                                        *
* Syntax:    call RxUsbQueryNumberDevices NumDevices                     *
*                                                                        *
* Return:    NO_UTIL_ERROR - Successful.                                 *
*************************************************************************/

ULONG APIENTRY RxUsbQueryNumberDevices( PSZ       Name,                   /* name of the function       */
                                        LONG      Argc,                   /* number of arguments        */
                                        RXSTRING  Argv[],                 /* list of argument strings   */
                                        PSZ       Queuename,              /* current queue name         */
                                        PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  ULONG ulNumDev;
  if( Argc!=1 ||
     !RXVALIDSTRING(Argv[0]) )
    return INVALID_ROUTINE;

  rc = UsbQueryNumberDevices( &ulNumDev);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  if(!rc && SetRexxVariableToLong(Argv[0].strptr, ulNumDev) == RXSHV_BADN )
    return INVALID_ROUTINE;            /* error on non-zero          */

  return VALID_ROUTINE;                /* no error on call           */
}

/*************************************************************************
* Function:  RxUsbQueryDeviceReport                                      *
*                                                                        *
* Syntax:    call RxUsbQueryDeviceReport DeviceNum, Report               *
*                                                                        *
* Return:    NO_UTIL_ERROR - Successful.                                 *
*************************************************************************/

ULONG APIENTRY RxUsbQueryDeviceReport( PSZ       Name,                   /* name of the function       */
                                       LONG      Argc,                   /* number of arguments        */
                                       RXSTRING  Argv[],                 /* list of argument strings   */
                                       PSZ       Queuename,              /* current queue name         */
                                       PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  ULONG ulDevNumber;
  ULONG ulBufLen;
  CHAR  Data[4096];

  if( Argc!=2||
      !string2ulong(Argv[0].strptr, &ulDevNumber) ||
      !RXVALIDSTRING(Argv[1]) )
    return INVALID_ROUTINE;

  ulBufLen = sizeof(Data);
  rc =  UsbQueryDeviceReport( ulDevNumber,
                              &ulBufLen,
                              (CHAR*)&Data);
  if(!rc)
  {
    if(SetRexxVariableToBuffer(Argv[1].strptr,ulBufLen, (UCHAR*)&Data))
      return INVALID_ROUTINE;            /* error on non-zero          */
  }

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;

}

/***************************************************************************
* Function:  RxUsbRegisterChangeNotification                               *
*                                                                          *
* Syntax:    call RxUsbRegisterChangeNotification NotifyID hDevAdd hDevRem *
*                                                                          *
* Return:    NO_UTIL_ERROR - Successful.                                   *
***************************************************************************/

ULONG APIENTRY RxUsbRegisterChangeNotification( PSZ       Name,      /* name of the function       */
                                                LONG      Argc,      /* number of arguments        */
                                                RXSTRING  Argv[],    /* list of argument strings   */
                                                PSZ       Queuename, /* current queue name         */
                                                PRXSTRING Retstr)    /* returned result string     */
{
  APIRET rc;
  USBNOTIFY NotifyID;
  HEV hDeviceAdded;
  HEV hDeviceRemoved;
  if( Argc!=3 ||
      !RXVALIDSTRING(Argv[0]) ||
      !string2ulong(Argv[1].strptr, &hDeviceAdded) ||
      !string2ulong(Argv[2].strptr, &hDeviceRemoved) )
    return INVALID_ROUTINE;

  rc = UsbRegisterChangeNotification( &NotifyID,
                                      hDeviceAdded,
                                      hDeviceRemoved);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);

  if(!rc && SetRexxVariableToLong(Argv[0].strptr, NotifyID) == RXSHV_BADN )
    return INVALID_ROUTINE;            /* error on non-zero          */

  return VALID_ROUTINE;
}

/******************************************************************************
* Function:  RxUsbRegisterDeviceNotification                                  *
*                                                                             *
* Syntax:    call RxUsbRegisterDeviceNotification NotifyID, hDevAdd, hDevRem, *
*                 VendorId, ProductID, Version                                *
*                                                                             *
*                                                                             *
* Return:    NO_UTIL_ERROR - Successful.                                      *
******************************************************************************/

ULONG APIENTRY RxUsbRegisterDeviceNotification( PSZ       Name,                   /* name of the function       */
                                                LONG      Argc,                   /* number of arguments        */
                                                RXSTRING  Argv[],                 /* list of argument strings   */
                                                PSZ       Queuename,              /* current queue name         */
                                                PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBNOTIFY NotifyID;
  HEV hDeviceAdded;
  HEV hDeviceRemoved;
  USHORT usVendor;
  USHORT usProduct;
  USHORT usBCDVersion;

  if( Argc==6)
  {
    if( !RXVALIDSTRING(Argv[0]) ||
        !string2ulong(Argv[1].strptr, &hDeviceAdded)    ||
        !string2ulong(Argv[2].strptr, &hDeviceRemoved)  ||
        !string2ushort(Argv[3].strptr, &usVendor) ||
        !string2ushort(Argv[4].strptr, &usProduct) ||
        !string2ushort(Argv[5].strptr, &usBCDVersion) )
      return INVALID_ROUTINE;
  }
  else
    if(Argc==5)
    {
      if( !RXVALIDSTRING(Argv[0]) ||
          !string2ulong(Argv[1].strptr, &hDeviceAdded)    ||
          !string2ulong(Argv[2].strptr, &hDeviceRemoved)  ||
          !string2ushort(Argv[3].strptr, &usVendor) ||
          !string2ushort(Argv[4].strptr, &usProduct))
        return INVALID_ROUTINE;
      usBCDVersion = USB_ANY_PRODUCTVERSION;
    }
    else
      return INVALID_ROUTINE;

  rc = UsbRegisterDeviceNotification( &NotifyID,
                                      hDeviceAdded,
                                      hDeviceRemoved,
                                      usVendor,
                                      usProduct,
                                      usBCDVersion);
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);

  if(!rc && SetRexxVariableToLong(Argv[0].strptr, NotifyID) == RXSHV_BADN )
    return INVALID_ROUTINE;            /* error on non-zero          */

  return VALID_ROUTINE;
}
/******************************************************************************
* Function:  RxUsbDeregisterNotification                                      *
*                                                                             *
* Syntax:    call RxUsbDeregisterNotification NotifyID                        *
*                                                                             *
*                                                                             *
* Return:    VALID_ROUTINE - Successful.                                      *
******************************************************************************/

ULONG APIENTRY RxUsbDeregisterNotification( PSZ       Name,                   /* name of the function       */
                                            LONG      Argc,                   /* number of arguments        */
                                            RXSTRING  Argv[],                 /* list of argument strings   */
                                            PSZ       Queuename,              /* current queue name         */
                                            PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBNOTIFY NotifyID;
  if( Argc!=1 ||
      !string2ulong(Argv[0].strptr, &NotifyID) )
    return INVALID_ROUTINE;

  rc = UsbDeregisterNotification(NotifyID);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}
/******************************************************************************
* Function:  RxUsbOpen                                                        *
*                                                                             *
* Syntax:    call RxUsbOpen USBHandle EnumDevice VendorId, ProductID, Version *
*                                                                             *
*                                                                             *
* Return:    VALID_ROUTINE - Successful.                                      *
******************************************************************************/

ULONG APIENTRY RxUsbOpen( PSZ       Name,      /* name of the function       */
                          LONG      Argc,      /* number of arguments        */
                          RXSTRING  Argv[],    /* list of argument strings   */
                          PSZ       Queuename, /* current queue name         */
                          PRXSTRING Retstr)    /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT usVendor;
  USHORT usProduct;
  USHORT usBCDVersion;
  USHORT usEnumDevice;

  if( Argc==5)
  {
    if( !RXVALIDSTRING(Argv[0]) ||
        !string2ushort(Argv[1].strptr, &usEnumDevice) ||
        !string2ushort(Argv[2].strptr, &usVendor) ||
        !string2ushort(Argv[3].strptr, &usProduct) ||
        !string2ushort(Argv[4].strptr, &usBCDVersion) )
      return INVALID_ROUTINE;
  }
  else
    if(Argc==4)
    {
      if( !RXVALIDSTRING(Argv[0]) ||
          !string2ushort(Argv[1].strptr, &usEnumDevice) ||
          !string2ushort(Argv[2].strptr, &usVendor) ||
          !string2ushort(Argv[3].strptr, &usProduct) )
        return INVALID_ROUTINE;
      usBCDVersion = USB_ANY_PRODUCTVERSION;
    }
    else
      return INVALID_ROUTINE;

  rc = UsbOpen( &Handle,
                usVendor,
                usProduct,
                usBCDVersion,
                usEnumDevice);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);

  if(!rc && SetRexxVariableToLong(Argv[0].strptr, Handle) == RXSHV_BADN )
    return INVALID_ROUTINE;            /* error on non-zero          */

  return VALID_ROUTINE;
}

/******************************************************************************
* Function:  RxUsbClose                                                       *
*                                                                             *
* Syntax:    call RxUsbClose USBHandle                                        *
*                                                                             *
*                                                                             *
* Return:    NO_UTIL_ERROR - Successful.                                      *
******************************************************************************/

ULONG APIENTRY RxUsbClose( PSZ       Name,                   /* name of the function       */
                           LONG      Argc,                   /* number of arguments        */
                           RXSTRING  Argv[],                 /* list of argument strings   */
                           PSZ       Queuename,              /* current queue name         */
                           PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;

  if( Argc!=1 ||
      !string2ulong(Argv[0].strptr, &Handle) )
    return INVALID_ROUTINE;

  rc = UsbClose(Handle);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

/******************************************************************************
* Function:  RxUsbCtrlMessage                                                 *
*                                                                             *
* Syntax:    call RxUsbCtrlMessage USBHandle, RequestType, Request, Value,    *
*                                  Length, Data, Timeout                      *
*                                                                             *
*                                                                             *
* Return:    NO_UTIL_ERROR - Successful.                                      *
******************************************************************************/

ULONG APIENTRY RxUsbCtrlMessage( PSZ       Name,                   /* name of the function       */
                                 LONG      Argc,                   /* number of arguments        */
                                 RXSTRING  Argv[],                 /* list of argument strings   */
                                 PSZ       Queuename,              /* current queue name         */
                                 PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  UCHAR  ucRequestType;
  UCHAR  ucRequest;
  USHORT usValue;
  USHORT usIndex;
  USHORT usLength;
  UCHAR  *pData;
  ULONG  ulTimeout;

  if(Argc!=8)
    return INVALID_ROUTINE;

  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2uchar(Argv[1].strptr, &ucRequestType) ||
      !string2uchar(Argv[2].strptr, &ucRequest) ||
      !string2ushort(Argv[3].strptr, &usValue) ||
      !string2ushort(Argv[4].strptr, &usLength) ||
      !RXVALIDSTRING(Argv[5]) ||
      !string2ulong(Argv[6].strptr, &ulTimeout) )
    return INVALID_ROUTINE;

  if(usLength)
    pData = (UCHAR*)Argv[6].strptr; // ToDo @@ maybe do Hex->Bin conversion?
  else
    pData = NULL;

  rc = UsbCtrlMessage( Handle,
                       ucRequestType,
                       ucRequest,
                       usValue,
                       usIndex,
                       usLength,
                       pData,
                       ulTimeout);

   // @@@ ToDo Handle pData for returned data
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

/******************************************************************************
* Function:  RxUsbBulkRead                                                    *
*                                                                             *
* Syntax:    call RxUsbBulkRead USBHandle, NotifyID, hDevAdd, hDevRem, *
*                 VendorId, ProductID, Version                                *
*                                                                             *
*                                                                             *
* Return:    NO_UTIL_ERROR - Successful.                                      *
******************************************************************************/

ULONG APIENTRY RxUsbBulkRead( PSZ       Name,                   /* name of the function       */
                              LONG      Argc,                   /* number of arguments        */
                              RXSTRING  Argv[],                 /* list of argument strings   */
                              PSZ       Queuename,              /* current queue name         */
                              PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  UCHAR  ucEndpoint;
  UCHAR  ucInterface;
  ULONG  ulNumBytes;
  UCHAR  *pData;
  ULONG  ulTimeout;

  if(Argc!=6)
    return INVALID_ROUTINE;

  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2uchar(Argv[1].strptr, &ucEndpoint) ||
      !string2uchar(Argv[2].strptr, &ucInterface) ||
      !string2ulong(Argv[3].strptr, &ulNumBytes) ||
      !RXVALIDSTRING(Argv[4]) ||
      !string2ulong(Argv[5].strptr, &ulTimeout) )
    return INVALID_ROUTINE;

  if(ulNumBytes)
  {
    pData = (UCHAR*) malloc(ulNumBytes);

    if(pData==NULL)
      return INVALID_ROUTINE;

    rc = UsbBulkRead( Handle,
                      ucEndpoint,
                      ucInterface,
                      &ulNumBytes,
                      pData,
                      ulTimeout);
    if(!rc)
    {
      if(SetRexxVariableToBuffer(Argv[4].strptr,ulNumBytes, pData))
      {
        free(pData);
        return INVALID_ROUTINE;            /* error on non-zero          */
      }
    }
    free(pData);
    SetRexxVariableToULONG(Argv[3].strptr, ulNumBytes);
  }
  else
    rc = 0;
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

/******************************************************************************
* Function:  RxUsbRegisterDeviceNotification                                  *
*                                                                             *
* Syntax:    call RxUsbRegisterDeviceNotification NotifyID, hDevAdd, hDevRem, *
*                 VendorId, ProductID, Version                                *
*                                                                             *
*                                                                             *
* Return:    NO_UTIL_ERROR - Successful.                                      *
******************************************************************************/

ULONG APIENTRY RxUsbBulkWrite( PSZ       Name,                   /* name of the function       */
                               LONG      Argc,                   /* number of arguments        */
                               RXSTRING  Argv[],                 /* list of argument strings   */
                               PSZ       Queuename,              /* current queue name         */
                               PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  UCHAR  ucEndpoint;
  UCHAR  ucInterface;
  ULONG ulNumBytes;
  UCHAR  *pData;
  ULONG  ulTimeout;

  if(Argc!=6)
    return INVALID_ROUTINE;

  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2uchar(Argv[1].strptr, &ucEndpoint) ||
      !string2uchar(Argv[2].strptr, &ucInterface) ||
      !string2ulong(Argv[3].strptr, &ulNumBytes) ||
      !RXVALIDSTRING(Argv[4]) ||
      !string2ulong(Argv[5].strptr, &ulTimeout) )
    return INVALID_ROUTINE;

  if(ulNumBytes)
  {
    pData = (UCHAR*) Argv[3].strptr; // ToDo @@ maybe do Hex->Bin conversion?

    if(pData==NULL)
      return INVALID_ROUTINE;

    rc = UsbBulkWrite( Handle,
                       ucEndpoint,
                       ucInterface,
                       ulNumBytes,
                       pData,
                       ulTimeout);
  }
  else
    rc = 0;
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbIrqStart( PSZ       Name,                   /* name of the function       */
                              LONG      Argc,                   /* number of arguments        */
                              RXSTRING  Argv[],                 /* list of argument strings   */
                              PSZ       Queuename,              /* current queue name         */
                              PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  if(Argc!=8)
    return INVALID_ROUTINE;
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbIrqStop( PSZ       Name,                   /* name of the function       */
                             LONG      Argc,                   /* number of arguments        */
                             RXSTRING  Argv[],                 /* list of argument strings   */
                             PSZ       Queuename,              /* current queue name         */
                             PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  if(Argc!=8)
    return INVALID_ROUTINE;
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbIsoStart( PSZ       Name,                   /* name of the function       */
                              LONG      Argc,                   /* number of arguments        */
                              RXSTRING  Argv[],                 /* list of argument strings   */
                              PSZ       Queuename,              /* current queue name         */
                              PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  if(Argc!=8)
    return INVALID_ROUTINE;
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbIsoStop( PSZ       Name,                   /* name of the function       */
                             LONG      Argc,                   /* number of arguments        */
                             RXSTRING  Argv[],                 /* list of argument strings   */
                             PSZ       Queuename,              /* current queue name         */
                             PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  if(Argc!=8)
    return INVALID_ROUTINE;
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbIsoDequeue( PSZ       Name,                   /* name of the function       */
                                LONG      Argc,                   /* number of arguments        */
                                RXSTRING  Argv[],                 /* list of argument strings   */
                                PSZ       Queuename,              /* current queue name         */
                                PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  if(Argc!=8)
    return INVALID_ROUTINE;
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbIsoPeekQueue( PSZ       Name,                   /* name of the function       */
                                  LONG      Argc,                   /* number of arguments        */
                                  RXSTRING  Argv[],                 /* list of argument strings   */
                                  PSZ       Queuename,              /* current queue name         */
                                  PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  if(Argc!=8)
    return INVALID_ROUTINE;
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbIsoEnqueue( PSZ       Name,                   /* name of the function       */
                                LONG      Argc,                   /* number of arguments        */
                                RXSTRING  Argv[],                 /* list of argument strings   */
                                PSZ       Queuename,              /* current queue name         */
                                PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  if(Argc!=8)
    return INVALID_ROUTINE;
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbIsoGetLength( PSZ       Name,                   /* name of the function       */
                                  LONG      Argc,                   /* number of arguments        */
                                  RXSTRING  Argv[],                 /* list of argument strings   */
                                  PSZ       Queuename,              /* current queue name         */
                                  PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  if(Argc!=8)
    return INVALID_ROUTINE;
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

/******************************************************************************
* Function:  RxUsbRegisterDeviceNotification                                  *
*                                                                             *
* Syntax:    call RxUsbRegisterDeviceNotification NotifyID, hDevAdd, hDevRem, *
*                 VendorId, ProductID, Version                                *
*                                                                             *
*                                                                             *
* Return:    NO_UTIL_ERROR - Successful.                                      *
******************************************************************************/

ULONG APIENTRY RxUsbDeviceClearFeature( PSZ       Name,                   /* name of the function       */
                                        LONG      Argc,                   /* number of arguments        */
                                        RXSTRING  Argv[],                 /* list of argument strings   */
                                        PSZ       Queuename,              /* current queue name         */
                                        PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT usFeat;

  if(Argc!=2)
    return INVALID_ROUTINE;
  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[2].strptr ,&usFeat ) )
    return INVALID_ROUTINE;

  rc = UsbDeviceClearFeature( Handle,
                              usFeat);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbInterfaceClearFeature( PSZ       Name,                   /* name of the function       */
                                           LONG      Argc,                   /* number of arguments        */
                                           RXSTRING  Argv[],                 /* list of argument strings   */
                                           PSZ       Queuename,              /* current queue name         */
                                           PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT usInterface;
  USHORT usFeat;

  if(Argc!=3)
    return INVALID_ROUTINE;
  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr ,&usInterface) ||
      !string2ushort(Argv[2].strptr ,&usFeat ) )
    return INVALID_ROUTINE;

  rc = UsbInterfaceClearFeature( Handle,
                                 usInterface,
                                 usFeat);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbEndpointClearFeature( PSZ       Name,                   /* name of the function       */
                                          LONG      Argc,                   /* number of arguments        */
                                          RXSTRING  Argv[],                 /* list of argument strings   */
                                          PSZ       Queuename,              /* current queue name         */
                                          PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT usEndpoint;
  USHORT usFeat;

  if(Argc!=3)
    return INVALID_ROUTINE;
  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr ,&usEndpoint) ||
      !string2ushort(Argv[2].strptr ,&usFeat ) )
    return INVALID_ROUTINE;

  rc = UsbEndpointClearFeature( Handle,
                                usEndpoint,
                                usFeat);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbEndpointClearHalt( PSZ       Name,                   /* name of the function       */
                                       LONG      Argc,                   /* number of arguments        */
                                       RXSTRING  Argv[],                 /* list of argument strings   */
                                       PSZ       Queuename,              /* current queue name         */
                                       PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT usEndpoint;

  if(Argc!=2)
      return INVALID_ROUTINE;
  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr ,&usEndpoint ) )
    return INVALID_ROUTINE;

  rc = UsbEndpointClearHalt( Handle,
                             usEndpoint);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbDeviceGetConfiguration( PSZ       Name,                   /* name of the function       */
                                            LONG      Argc,                   /* number of arguments        */
                                            RXSTRING  Argv[],                 /* list of argument strings   */
                                            PSZ       Queuename,              /* current queue name         */
                                            PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  UCHAR  ucConfig;

  if(Argc!=2)
      return INVALID_ROUTINE;
  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !RXVALIDSTRING(Argv[1]) )
    return INVALID_ROUTINE;

  rc = UsbDeviceGetConfiguration( Handle,
                                 (UCHAR*)&ucConfig);
  if(!rc)
  {
    SetRexxVariableToUSHORT(Argv[2].strptr, ucConfig);
  }
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbDeviceGetDescriptor( PSZ       Name,                   /* name of the function       */
                                         LONG      Argc,                   /* number of arguments        */
                                         RXSTRING  Argv[],                 /* list of argument strings   */
                                         PSZ       Queuename,              /* current queue name         */
                                         PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  if(Argc!=8)
    return INVALID_ROUTINE;
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbInterfaceGetDescriptor( PSZ       Name,                   /* name of the function       */
                                            LONG      Argc,                   /* number of arguments        */
                                            RXSTRING  Argv[],                 /* list of argument strings   */
                                            PSZ       Queuename,              /* current queue name         */
                                            PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  if(Argc!=5)
    return INVALID_ROUTINE;
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbEndpointGetDescriptor( PSZ       Name,                   /* name of the function       */
                                           LONG      Argc,                   /* number of arguments        */
                                           RXSTRING  Argv[],                 /* list of argument strings   */
                                           PSZ       Queuename,              /* current queue name         */
                                           PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  if(Argc!=5)
    return INVALID_ROUTINE;
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbConfigurationGetDescriptor( PSZ       Name,                   /* name of the function       */
                                                LONG      Argc,                   /* number of arguments        */
                                                RXSTRING  Argv[],                 /* list of argument strings   */
                                                PSZ       Queuename,              /* current queue name         */
                                                PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  if(Argc!=5)
    return INVALID_ROUTINE;
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbStringGetDescriptor( PSZ       Name,                   /* name of the function       */
                                         LONG      Argc,                   /* number of arguments        */
                                         RXSTRING  Argv[],                 /* list of argument strings   */
                                         PSZ       Queuename,              /* current queue name         */
                                         PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  if(Argc!=5)
    return INVALID_ROUTINE;
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbInterfaceGetAltSetting( PSZ       Name,                   /* name of the function       */
                                            LONG      Argc,                   /* number of arguments        */
                                            RXSTRING  Argv[],                 /* list of argument strings   */
                                            PSZ       Queuename,              /* current queue name         */
                                            PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT usInterface;
  UCHAR  ucSetting;

  if(Argc!=3)
    return INVALID_ROUTINE;
  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr, &usInterface) ||
      !RXVALIDSTRING(Argv[2]) )
    return INVALID_ROUTINE;

  rc = UsbInterfaceGetAltSetting( Handle,
                                  usInterface,
                                  (UCHAR*)&ucSetting);
  if(!rc)
  {
    SetRexxVariableToUSHORT(Argv[2].strptr, ucSetting);
  }
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbDeviceGetStatus( PSZ       Name,                   /* name of the function       */
                                     LONG      Argc,                   /* number of arguments        */
                                     RXSTRING  Argv[],                 /* list of argument strings   */
                                     PSZ       Queuename,              /* current queue name         */
                                     PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT usStatus;

  if(Argc!=2)
      return INVALID_ROUTINE;
  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !RXVALIDSTRING(Argv[1]) )
    return INVALID_ROUTINE;

  rc = UsbDeviceGetStatus( Handle,
                           (UCHAR*)&usStatus);
  if(!rc)
  {
    SetRexxVariableToUSHORT(Argv[2].strptr, usStatus);
  }
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbInterfaceGetStatus( PSZ       Name,                   /* name of the function       */
                                        LONG      Argc,                   /* number of arguments        */
                                        RXSTRING  Argv[],                 /* list of argument strings   */
                                        PSZ       Queuename,              /* current queue name         */
                                        PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT usInterface;
  USHORT usStatus;

  if(Argc!=3)
    return INVALID_ROUTINE;
  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr, &usInterface) ||
      !RXVALIDSTRING(Argv[2]) )
    return INVALID_ROUTINE;

  rc = UsbInterfaceGetStatus( Handle,
                              usInterface,
                              (UCHAR*)&usStatus);
  if(!rc)
  {
    SetRexxVariableToUSHORT(Argv[2].strptr, usStatus);
  }
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbEndpointGetStatus( PSZ       Name,                   /* name of the function       */
                                       LONG      Argc,                   /* number of arguments        */
                                       RXSTRING  Argv[],                 /* list of argument strings   */
                                       PSZ       Queuename,              /* current queue name         */
                                       PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT usEndpoint;
  USHORT usStatus;

  if(Argc==4)
  {
    if(!RXVALIDSTRING(Argv[3]) )
      return INVALID_ROUTINE;
  }
  else
  {
    if(Argc!=3)
      return INVALID_ROUTINE;
  }
  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr, &usEndpoint) ||
      !RXVALIDSTRING(Argv[2]) )
    return INVALID_ROUTINE;

  rc = UsbEndpointGetStatus( Handle,
                             usEndpoint,
                             (UCHAR*)&usStatus);
  if(!rc)
  {
    SetRexxVariableToUSHORT(Argv[2].strptr, usStatus);
    if(Argc==4)
    {
      if(usStatus ==1)
        SetRexxVariable(Argv[4].strptr, "STATUS_ENDPOINT_HALT");
      else
        SetRexxVariable(Argv[4].strptr, "STATUS_ENDPOINT_OK");
    }
  }
  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbDeviceSetAddress( PSZ       Name,                   /* name of the function       */
                                      LONG      Argc,                   /* number of arguments        */
                                      RXSTRING  Argv[],                 /* list of argument strings   */
                                      PSZ       Queuename,              /* current queue name         */
                                      PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT usAddr;

  if(Argc!=2)
    return INVALID_ROUTINE;

  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr, &usAddr) )
    return INVALID_ROUTINE;

  rc = UsbDeviceSetAddress( Handle,
                            usAddr);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbDeviceSetConfiguration( PSZ       Name,                   /* name of the function       */
                                            LONG      Argc,                   /* number of arguments        */
                                            RXSTRING  Argv[],                 /* list of argument strings   */
                                            PSZ       Queuename,              /* current queue name         */
                                            PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT usConfig;

  if(Argc!=2)
    return INVALID_ROUTINE;

  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr, &usConfig) )
    return INVALID_ROUTINE;

  rc = UsbDeviceSetConfiguration( Handle,
                                  usConfig);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbDeviceSetDescriptor( PSZ       Name,                   /* name of the function       */
                                         LONG      Argc,                   /* number of arguments        */
                                         RXSTRING  Argv[],                 /* list of argument strings   */
                                         PSZ       Queuename,              /* current queue name         */
                                         PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT usIndex;
  USHORT usLID;
  USHORT usLength;
  UCHAR  *pData;

  if(Argc!=5)
    return INVALID_ROUTINE;

  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr, &usIndex) ||
      !string2ushort(Argv[2].strptr, &usLID) ||
      !string2ushort(Argv[3].strptr, &usLength) ||
      !RXVALIDSTRING(Argv[4]) )
    return INVALID_ROUTINE;

  pData = (UCHAR*) Argv[4].strptr;

  rc = UsbDeviceSetDescriptor( Handle,
                               usIndex,
                               usLID,
                               usLength,
                               pData);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbInterfaceSetDescriptor( PSZ       Name,                   /* name of the function       */
                                            LONG      Argc,                   /* number of arguments        */
                                            RXSTRING  Argv[],                 /* list of argument strings   */
                                            PSZ       Queuename,              /* current queue name         */
                                            PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT usIndex;
  USHORT usLID;
  USHORT usLength;
  UCHAR  *pData;

  if(Argc!=5)
    return INVALID_ROUTINE;

  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr, &usIndex) ||
      !string2ushort(Argv[2].strptr, &usLID) ||
      !string2ushort(Argv[3].strptr, &usLength) ||
      !RXVALIDSTRING(Argv[4]) )
    return INVALID_ROUTINE;

  pData = (UCHAR*) Argv[4].strptr;

  rc = UsbInterfaceSetDescriptor( Handle,
                                  usIndex,
                                  usLID,
                                  usLength,
                                  pData);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbEndpointSetDescriptor( PSZ       Name,                   /* name of the function       */
                                           LONG      Argc,                   /* number of arguments        */
                                           RXSTRING  Argv[],                 /* list of argument strings   */
                                           PSZ       Queuename,              /* current queue name         */
                                           PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT usIndex;
  USHORT usLID;
  USHORT usLength;
  UCHAR  *pData;

  if(Argc!=5)
    return INVALID_ROUTINE;

  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr, &usIndex) ||
      !string2ushort(Argv[2].strptr, &usLID) ||
      !string2ushort(Argv[3].strptr, &usLength) ||
      !RXVALIDSTRING(Argv[4]) )
    return INVALID_ROUTINE;

  pData = (UCHAR*) Argv[4].strptr;

  rc = UsbEndpointSetDescriptor( Handle,
                                 usIndex,
                                 usLID,
                                 usLength,
                                 pData);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbConfigurationSetDescriptor( PSZ       Name,                   /* name of the function       */
                                                LONG      Argc,                   /* number of arguments        */
                                                RXSTRING  Argv[],                 /* list of argument strings   */
                                                PSZ       Queuename,              /* current queue name         */
                                                PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT usIndex;
  USHORT usLID;
  USHORT usLength;
  UCHAR  *pData;

  if(Argc!=5)
    return INVALID_ROUTINE;

  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr, &usIndex) ||
      !string2ushort(Argv[2].strptr, &usLID) ||
      !string2ushort(Argv[3].strptr, &usLength) ||
      !RXVALIDSTRING(Argv[4]) )
    return INVALID_ROUTINE;

  pData = (UCHAR*) Argv[4].strptr;

  rc = UsbConfigurationSetDescriptor( Handle,
                                      usIndex,
                                      usLID,
                                      usLength,
                                      pData);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbStringSetDescriptor( PSZ       Name,                   /* name of the function       */
                                         LONG      Argc,                   /* number of arguments        */
                                         RXSTRING  Argv[],                 /* list of argument strings   */
                                         PSZ       Queuename,              /* current queue name         */
                                         PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT usIndex;
  USHORT usLID;
  USHORT usLength;
  UCHAR  *pData;

  if(Argc!=5)
    return INVALID_ROUTINE;

  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr, &usIndex) ||
      !string2ushort(Argv[2].strptr, &usLID) ||
      !string2ushort(Argv[3].strptr, &usLength) ||
      !RXVALIDSTRING(Argv[4]) )
    return INVALID_ROUTINE;

  pData = (UCHAR*) Argv[4].strptr;

  rc = UsbStringSetDescriptor( Handle,
                               usIndex,
                               usLID,
                               usLength,
                               pData);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbDeviceSetFeature( PSZ       Name,                   /* name of the function       */
                                      LONG      Argc,                   /* number of arguments        */
                                      RXSTRING  Argv[],                 /* list of argument strings   */
                                      PSZ       Queuename,              /* current queue name         */
                                      PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT    usFeature;
  if(Argc!=2)
    return INVALID_ROUTINE;
  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr, &usFeature))
    return INVALID_ROUTINE;
  rc = UsbDeviceSetFeature(Handle, usFeature);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbInterfaceSetFeature( PSZ       Name,                   /* name of the function       */
                                         LONG      Argc,                   /* number of arguments        */
                                         RXSTRING  Argv[],                 /* list of argument strings   */
                                         PSZ       Queuename,              /* current queue name         */
                                         PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT    usInterface;
  USHORT    usFeature;
  if(Argc!=3)
    return INVALID_ROUTINE;
  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr, &usInterface) ||
      !string2ushort(Argv[2].strptr, &usFeature))
    return INVALID_ROUTINE;
  rc = UsbInterfaceSetFeature(Handle, usInterface, usFeature);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbEndpointSetFeature( PSZ       Name,                   /* name of the function       */
                                        LONG      Argc,                   /* number of arguments        */
                                        RXSTRING  Argv[],                 /* list of argument strings   */
                                        PSZ       Queuename,              /* current queue name         */
                                        PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT    usEndpoint;
  USHORT    usFeature;
  if(Argc!=3)
    return INVALID_ROUTINE;
  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr, &usEndpoint) ||
      !string2ushort(Argv[2].strptr, &usFeature))

  rc = UsbEndpointSetFeature(Handle, usEndpoint, usFeature);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbInterfaceSetAltSetting( PSZ       Name,                   /* name of the function       */
                                            LONG      Argc,                   /* number of arguments        */
                                            RXSTRING  Argv[],                 /* list of argument strings   */
                                            PSZ       Queuename,              /* current queue name         */
                                            PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT    usInterface, usAltSetting;

  if(Argc!=3)
    return INVALID_ROUTINE;

  if( !string2ulong(Argv[0].strptr, &Handle)  ||
      !string2ushort(Argv[1].strptr, &usInterface) ||
      !string2ushort(Argv[2].strptr, &usAltSetting))
    return INVALID_ROUTINE;


  rc = UsbInterfaceSetAltSetting(Handle, usInterface, usAltSetting);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

ULONG APIENTRY RxUsbEndpointSynchFrame( PSZ       Name,                   /* name of the function       */
                                        LONG      Argc,                   /* number of arguments        */
                                        RXSTRING  Argv[],                 /* list of argument strings   */
                                        PCSZ       Queuename,              /* current queue name         */
                                        PRXSTRING Retstr)                 /* returned result string     */
{
  APIRET rc;
  USBHANDLE Handle;
  USHORT    usEndpoint;
  USHORT    usFrameNumber;

  if(Argc!=2)
    return INVALID_ROUTINE;
  if(!string2ulong(Argv[0].strptr, &Handle) )
    return INVALID_ROUTINE;

  rc = UsbEndpointSynchFrame(Handle, usEndpoint, (UCHAR*)&usFrameNumber);

  sprintf(Retstr->strptr, "%d", rc);
  Retstr->strlength = strlen(Retstr->strptr);
  return VALID_ROUTINE;
}

#ifdef __cplusplus
}  //extern "C"
#endif

