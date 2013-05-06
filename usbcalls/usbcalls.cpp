#define INCL_DOSERRORS
#define INCL_DOSMEMMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#include <OS2.h>
#include <string.h>
//#include <stdlib.h>
//#include <stdio.h>

#include "usbcalls.h"

#define  IOCAT_USBRES            0x000000A0  // USB Resource device control
#define  IOCTLF_NUMDEVICE        0x00000031  // Get Number of pluged in Devices
#define  IOCTLF_GETINFO          0x00000032  // Get Info About a device
#define  IOCTLF_AQUIREDEVICE     0x00000033
#define  IOCTLF_RELEASEDEVICE    0x00000034
#define  IOCTLF_GETSTRING        0x00000035
#define  IOCTLF_SENDCONTROLURB   0x00000036
#define  IOCTLF_SENDBULKURB      0x00000037  // Send
#define  IOCTLF_START_IRQ_PROC   0x00000038  // Start IRQ polling in a buffer
#define  IOCTLF_GETDEVINFO       0x00000039  // Get information about device
#define  IOCTLF_STOP_IRQ_PROC    0x0000003A  // Stop IRQ Polling
#define  IOCTLF_START_ISO_PROC   0x0000003B  // Start ISO buffering in a Ringbuffer
#define  IOCTLF_STOP_ISO_PROC    0x0000003C  // Stop ISO buffering
#define  IOCTLF_CANCEL_IORB      0x0000003D  // Abord an IO;
#define  IOCTLF_REG_STATUSSEM    0x00000041  // Register Semaphore for general Statuschange
#define  IOCTLF_DEREG_STATUSSEM  0x00000042  // Deregister Semaphore
#define  IOCTLF_REG_DEVICESEM    0x00000043  // Register Semaphore for a vendor&deviceID
#define  IOCTLF_DEREG_DEVICESEM  0x00000044  // Deregister Semaphore


#define NOTIFY_FREE   0
#define NOTIFY_CHANGE 1
#define NOTIFY_DEVICE 2
#define MAX_NOTIFICATIONS 256

typedef struct
{
  HEV hDeviceAdded;
  HEV hDeviceRemoved;
  USHORT usFlags;
  USHORT usVendor;
  USHORT usProduct;
  USHORT usBCDDevice;
}NOTIFYENTRY, *PNOTIFYENTRY;

#define DEV_SEM_ADD       0x00000001
#define DEV_SEM_REMOVE    0x00000002
#define DEV_SEM_MASK      0x00000003
#define DEV_SEM_VENDORID  0x00000004
#define DEV_SEM_PRODUCTID 0x00000008
#define DEV_SEM_BCDDEVICE 0x00000010

typedef struct{
  ULONG  ulSize;
  ULONG  ulCaps;
  ULONG  ulSemDeviceAdd;
  ULONG  ulSemDeviceRemove;
} STATUSEVENTSET, * PSTATUSEVENTSET;


typedef struct{
  ULONG  ulSize;
  ULONG  ulCaps;
  ULONG  ulSemDeviceAdd;
  ULONG  ulSemDeviceRemove;
  USHORT usVendorID;
  USHORT usProductID;
  USHORT usBCDDevice;
  USHORT usReserved;
} DEVEVENTSET, * PDEVEVENTSET;

typedef struct
{
  USHORT usVendorID;
  USHORT usProductID;
  USHORT usBCDDevice;
  USHORT usDeviceNumber; // Get the usDeviceNumber device in the system fi. if 2 aquire the 2nd device
                         // 0 means first not aquired device.
}AQUIREDEV,  *PAQUIREDEV;

typedef struct
{
  ULONG  ulDevHandle;
  ULONG  ulEventDone;
  UCHAR  ucEndpoint;
  UCHAR  ucInterface;
  USHORT usDataProcessed;
  USHORT usDataRemain;
  USHORT usStatus;
}USBCALLS_BULK_REQ, *PUSBCALLS_BULK_REQ;

typedef struct
{
  UCHAR  bRequestType;
  UCHAR  bRequest;
  USHORT wValue;
  USHORT wIndex;
  USHORT wLength;
  ULONG  ulTimeout; /* in milliseconds */
}SETUPPACKET, *PSETUPPACKET;

typedef struct
{
  ULONG  ulHandle;
  UCHAR  bRequestType;
  UCHAR  bRequest;
  USHORT wValue;
  USHORT wIndex;
  USHORT wLength;
  ULONG  ulTimeout; /* in milliseconds */
}USBCALLS_CTRL_REQ, *PUSBCALLS_CTRL_REQ;

typedef struct
{
  ULONG  ulDevHandle;
  ULONG  ulIrqEvent;
  UCHAR  ucEndpoint;
  UCHAR  ucInterface;
}USBCALLS_IRQ_START, *NPUSBCALLS_IRQ_START, FAR *PUSBCALLS_IRQ_START;

typedef struct
{
  ULONG ulDevHandle;
  UCHAR ucEndpoint;
  UCHAR ucInterface;
}USBCALLS_ISO_START, *NPUSBCALLS_ISO_START, FAR *PUSBCALLS_ISO_START;

#define ISO_DIRMASK 0x80
typedef struct
{
  ULONG  hSemAccess;        // Syncronise access to the Pos values
  ULONG  hDevice;
  USHORT usPosWrite;
  USHORT usPosRead;
  USHORT usBufSize;
  UCHAR  ucEndpoint;
  UCHAR  ucInterface;
  UCHAR  ucBuffer[16*1023];
}ISORINGBUFFER, * PISORINGBUFFER;

typedef USBCALLS_ISO_START USBCALLS_ISO_STOP, * NPUSBCALLS_ISO_STOP, FAR *PUSBCALLS_ISO_STOP;
typedef USBCALLS_ISO_START USBCALLS_IRQ_STOP, * NPUSBCALLS_IRQ_STOP, FAR *PUSBCALLS_IRQ_STOP;


HFILE g_hUSBDrv;
BOOL  g_fInit;
ULONG g_ulFreeNotifys;
HMTX  g_hSemNotifytable;
NOTIFYENTRY g_Notifications[MAX_NOTIFICATIONS];

HMTX  g_hSemRingBuffers;
PISORINGBUFFER g_pIsoRingBuffers;
ULONG g_ulNumIsoRingBuffers;

void InitUsbCalls()
{
  ULONG ulAction;
  APIRET rc;

  rc = DosOpen( "USBRESM$",
                &g_hUSBDrv,
                &ulAction,
                0,
                FILE_NORMAL,
                OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_ACCESS_READWRITE |
                OPEN_FLAGS_NOINHERIT |
                OPEN_SHARE_DENYNONE,
                0 );
  if(rc)
  {
    g_hUSBDrv = 0;
    g_fInit   = FALSE;

  }
  else
  {
    // @@ToDO Add EnvVar or INI for dynamically setting the number
    g_ulNumIsoRingBuffers = 8;
    for(int i=0;i<MAX_NOTIFICATIONS;i++)
    {
      g_Notifications[i].usFlags        = NOTIFY_FREE;
      g_Notifications[i].hDeviceAdded   = 0;
      g_Notifications[i].hDeviceRemoved = 0;
      g_Notifications[i].usVendor       = 0;
      g_Notifications[i].usProduct      = 0;
      g_Notifications[i].usBCDDevice    = 0;
    }
    rc = DosAllocMem( (PPVOID)&g_pIsoRingBuffers,
                      g_ulNumIsoRingBuffers * sizeof(ISORINGBUFFER),
                      PAG_WRITE | PAG_COMMIT | OBJ_TILE);
    if(!rc)
    {
      PISORINGBUFFER pIter = g_pIsoRingBuffers;
      for(ULONG i=0;i< g_ulNumIsoRingBuffers;i++,pIter++)
      {
        pIter->hDevice     = 0;
        pIter->hSemAccess  = 0;      // Syncronise access to the Pos values
        pIter->usPosWrite  = 0;
        pIter->usPosRead   = 0;
        pIter->usBufSize   = 16*1023;
        pIter->ucEndpoint  = 0;
        pIter->ucInterface = 0;

        //pIter->ucBuffer
      }
      rc=DosCreateMutexSem(NULL,&g_hSemRingBuffers,DC_SEM_SHARED,FALSE);
      if(!rc)
      {
        rc=DosCreateMutexSem(NULL,&g_hSemNotifytable,DC_SEM_SHARED,FALSE);
        if(rc)
        {
          DosCloseMutexSem(g_hSemRingBuffers);
          DosFreeMem(g_pIsoRingBuffers);
        }
      }
      else
      {
        DosFreeMem(g_pIsoRingBuffers);
      }
    }

    if(rc)
    {
      DosClose(g_hUSBDrv);
      g_hUSBDrv = 0;
      g_fInit   = FALSE;
    }
    else
      g_fInit = TRUE;
  }
}

BOOL IsBadReadPointer(PVOID pBase, ULONG ulSize)
{
  APIRET rc;
  ULONG ulFlags;
  rc = DosQueryMem(pBase, &ulSize, &ulFlags);

  return rc!=0?TRUE:(ulFlags&PAG_READ)&&(ulFlags&PAG_COMMIT)?FALSE:TRUE;
}

BOOL IsBadWritePointer(PVOID pBase, ULONG ulSize)
{
  APIRET rc;
  ULONG ulFlags;
  rc = DosQueryMem(pBase, &ulSize, &ulFlags);

  return rc!=0?TRUE:((ulFlags&PAG_WRITE)==PAG_WRITE&&(ulFlags&PAG_COMMIT)==PAG_COMMIT)?FALSE:TRUE;
}

#ifdef __cplusplus
  extern "C" {
#endif

APIRET APIENTRY UsbQueryNumberDevices( ULONG *pulNumDev)
{
  APIRET rc;
  ULONG ulLength;
  if(!g_fInit)
    return USB_NOT_INIT;

  if( IsBadWritePointer(pulNumDev,sizeof(ULONG)) )
    return ERROR_INVALID_PARAMETER;
  ulLength=sizeof(ULONG);
  *pulNumDev = 0;
  rc = DosDevIOCtl( g_hUSBDrv,
                    IOCAT_USBRES, IOCTLF_NUMDEVICE,
                    NULL, 0, NULL,
                    pulNumDev, ulLength, &ulLength);
  return rc;
}

APIRET APIENTRY UsbQueryDeviceReport( ULONG ulDevNumber,
                             ULONG *pulBufLen,
                             CHAR *pData)
{
  APIRET rc;
  ULONG ulParmLen;

  if(!g_fInit)
    return USB_NOT_INIT;

  if( IsBadWritePointer(pulBufLen, sizeof(ULONG)) )
    return ERROR_INVALID_PARAMETER;

  if( pData!=NULL && IsBadWritePointer(pData,*pulBufLen) )
    return ERROR_INVALID_PARAMETER;
  if(pData==NULL)
   *pulBufLen = 0;
  ulParmLen = sizeof(ulDevNumber);
  rc = DosDevIOCtl( g_hUSBDrv,
                    IOCAT_USBRES, IOCTLF_GETINFO,
                     (PVOID)&ulDevNumber, ulParmLen, &ulParmLen,
                    pData, *pulBufLen, pulBufLen);
  return rc;
}

APIRET APIENTRY UsbRegisterChangeNotification( PUSBNOTIFY pNotifyID,
                                      HEV hDeviceAdded,
                                      HEV hDeviceRemoved)
{
  APIRET rc;
  int i;
  STATUSEVENTSET EventSet;
  ULONG ulSize;

  if(!g_fInit)
    return USB_NOT_INIT;

  if( IsBadWritePointer(pNotifyID, sizeof(ULONG)) ||
      (hDeviceAdded==0 && hDeviceRemoved==0) )
    return ERROR_INVALID_PARAMETER;

  ulSize = sizeof(EventSet);
  EventSet.ulSize = ulSize;

  if(hDeviceAdded!=0)
  {
    ULONG ulCnt;
    rc = DosQueryEventSem(hDeviceAdded,&ulCnt);
    if(rc)
      return rc;
    EventSet.ulCaps         = DEV_SEM_ADD;
    EventSet.ulSemDeviceAdd = hDeviceAdded;
  }

  if(hDeviceRemoved!=0)
  {
    ULONG ulCnt;
    rc = DosQueryEventSem(hDeviceRemoved,&ulCnt);
    if(rc)
      return rc;
    EventSet.ulCaps            |= DEV_SEM_REMOVE;
    EventSet.ulSemDeviceRemove = hDeviceRemoved;
  }

  rc = DosRequestMutexSem(g_hSemNotifytable,SEM_INDEFINITE_WAIT);
  if(rc)
    return rc;

  for(i=0;i<MAX_NOTIFICATIONS;i++)
  {
    if( g_Notifications[i].usFlags == NOTIFY_FREE)
    {
      g_Notifications[i].usFlags = NOTIFY_CHANGE;
      g_Notifications[i].hDeviceAdded   = hDeviceAdded;
      g_Notifications[i].hDeviceRemoved = hDeviceRemoved;
      g_Notifications[i].usVendor       = 0;
      g_Notifications[i].usProduct      = 0;
      g_Notifications[i].usBCDDevice    = 0;
      break;
    }
  }
  DosReleaseMutexSem(g_hSemNotifytable);
  if(i==MAX_NOTIFICATIONS)
    return USB_ERROR_NO_MORE_NOTIFICATIONS;

  // @@ToDo come up with a better way to generate IDs
  *pNotifyID = (USBNOTIFY) (&g_Notifications[i]);
  rc = DosDevIOCtl( g_hUSBDrv,
                    IOCAT_USBRES, IOCTLF_REG_STATUSSEM,
                    NULL, 0, NULL,
                    &EventSet,ulSize, &ulSize);
  if(rc)
  {
    g_Notifications[i].usFlags = NOTIFY_FREE;
    *pNotifyID = 0;
  }
  return rc;
}

APIRET APIENTRY UsbRegisterDeviceNotification( PUSBNOTIFY pNotifyID,
                                      HEV hDeviceAdded,
                                      HEV hDeviceRemoved,
                                      USHORT usVendor,
                                      USHORT usProduct,
                                      USHORT usBCDVersion)
{
  APIRET rc;
  ULONG ulCnt,ulSize;
  int i;
  DEVEVENTSET EventSet;

  if(!g_fInit)
    return USB_NOT_INIT;

  if( IsBadWritePointer(pNotifyID, sizeof(ULONG)) ||
      hDeviceAdded==0 || hDeviceRemoved==0 ||
      usVendor  == 0  || usVendor  == 0xFFFF ||
      usProduct == 0  || usProduct == 0xFFFF )
    return ERROR_INVALID_PARAMETER;


  rc = DosQueryEventSem(hDeviceAdded,&ulCnt);
  if(rc)
    return rc;
  rc = DosQueryEventSem(hDeviceRemoved,&ulCnt);
  if(rc)
    return rc;

  ulSize = sizeof(EventSet);
  EventSet.ulSize            = ulSize;
  EventSet.ulCaps            = DEV_SEM_ADD | DEV_SEM_REMOVE |
                               DEV_SEM_VENDORID | DEV_SEM_PRODUCTID |
                               DEV_SEM_BCDDEVICE ;
  EventSet.ulSemDeviceAdd    = hDeviceAdded;
  EventSet.ulSemDeviceRemove = hDeviceRemoved;
  EventSet.usVendorID        = usVendor;
  EventSet.usProductID       = usProduct;
  EventSet.usBCDDevice       = usBCDVersion;
  EventSet.usReserved  = 0;

  rc = DosRequestMutexSem(g_hSemNotifytable,SEM_INDEFINITE_WAIT);
  if(rc)
    return rc;

  for(i=0;i<MAX_NOTIFICATIONS;i++)
  {
    if( g_Notifications[i].usFlags == NOTIFY_FREE)
    {
      g_Notifications[i].usFlags = NOTIFY_DEVICE;
      g_Notifications[i].hDeviceAdded   = hDeviceAdded;
      g_Notifications[i].hDeviceRemoved = hDeviceRemoved;
      g_Notifications[i].usVendor       = usVendor;
      g_Notifications[i].usProduct      = usProduct;
      g_Notifications[i].usBCDDevice    = usBCDVersion;
      break;
    }
  }
  DosReleaseMutexSem(g_hSemNotifytable);
  if(i==MAX_NOTIFICATIONS)
    return USB_ERROR_NO_MORE_NOTIFICATIONS;

  // @@ToDo come up with a better way to generate IDs
  *pNotifyID = (USBNOTIFY) (&g_Notifications[i]);
  rc = DosDevIOCtl( g_hUSBDrv,
                    IOCAT_USBRES, IOCTLF_REG_DEVICESEM,
                    NULL, 0, NULL,
                    &EventSet,ulSize, &ulSize);
  if(rc)
  {
    g_Notifications[i].usFlags = NOTIFY_FREE;
    *pNotifyID = 0;
  }
  return rc;
}

APIRET APIENTRY UsbDeregisterNotification( USBNOTIFY NotifyID)
{
  APIRET rc;
  USBNOTIFY MinID,MaxID;
  ULONG Index, ulFunction, ulSize;
  DEVEVENTSET EventSet;

  if(!g_fInit)
    return USB_NOT_INIT;

  MinID = (USBNOTIFY) (&g_Notifications[0]);
  MaxID = (USBNOTIFY) (&g_Notifications[MAX_NOTIFICATIONS-1]);

  if(NotifyID<MinID || NotifyID>MaxID)
    return ERROR_INVALID_PARAMETER;

  Index = NotifyID - MinID;

  if(Index % sizeof(NOTIFYENTRY))
    return ERROR_INVALID_PARAMETER;

  Index /= sizeof(NOTIFYENTRY);

  rc = DosRequestMutexSem(g_hSemNotifytable,SEM_INDEFINITE_WAIT);

  switch(g_Notifications[Index].usFlags)
  {
    case NOTIFY_FREE:
      DosReleaseMutexSem(g_hSemNotifytable);
      return ERROR_INVALID_PARAMETER;
    case NOTIFY_CHANGE:
      ulFunction  = IOCTLF_DEREG_STATUSSEM;
      ulSize = sizeof(STATUSEVENTSET);
      EventSet.ulSize            = ulSize;
      EventSet.ulCaps            = DEV_SEM_ADD | DEV_SEM_REMOVE;
      EventSet.ulSemDeviceAdd    = g_Notifications[Index].hDeviceAdded;
      EventSet.ulSemDeviceRemove = g_Notifications[Index].hDeviceRemoved;
      break;
    case NOTIFY_DEVICE:
      ulFunction = IOCTLF_DEREG_DEVICESEM;
      ulSize = sizeof(DEVEVENTSET);
      EventSet.ulSize            = ulSize;
      EventSet.ulCaps            = DEV_SEM_ADD | DEV_SEM_REMOVE |
                                   DEV_SEM_VENDORID | DEV_SEM_PRODUCTID |
                                   DEV_SEM_BCDDEVICE ;
      EventSet.ulSemDeviceAdd    = g_Notifications[Index].hDeviceAdded;
      EventSet.ulSemDeviceRemove = g_Notifications[Index].hDeviceRemoved;
      EventSet.usVendorID        = g_Notifications[Index].usVendor;
      EventSet.usProductID       = g_Notifications[Index].usProduct;
      EventSet.usBCDDevice       = g_Notifications[Index].usBCDDevice;
      EventSet.usReserved  = 0;
      break;
    default:
      DosReleaseMutexSem(g_hSemNotifytable);
      return ERROR_GEN_FAILURE;
  }

  rc = DosDevIOCtl( g_hUSBDrv,
                    IOCAT_USBRES, ulFunction,
                    NULL, 0, NULL,
                    &EventSet,ulSize, &ulSize);
  if(0==rc)
  {
    g_Notifications[Index].usFlags        = NOTIFY_FREE;
    g_Notifications[Index].hDeviceAdded   = 0;
    g_Notifications[Index].hDeviceRemoved = 0;
    g_Notifications[Index].usVendor       = 0;
    g_Notifications[Index].usProduct      = 0;
    g_Notifications[Index].usBCDDevice    = 0;
  }
  DosReleaseMutexSem(g_hSemNotifytable);

  return rc;
}

APIRET APIENTRY UsbOpen( PUSBHANDLE pHandle,
                USHORT usVendor,
                USHORT usProduct,
                USHORT usBCDDevice,
                USHORT usEnumDevice)
{
  APIRET rc;
  AQUIREDEV Aquire;
  ULONG   ulParmLen, ulDataLen;

  if(!g_fInit)
    return USB_NOT_INIT;
  if(IsBadWritePointer(pHandle,sizeof(USBHANDLE)) )
    return ERROR_INVALID_PARAMETER;

  Aquire.usVendorID     = usVendor;
  Aquire.usProductID    = usProduct;
  Aquire.usBCDDevice    = usBCDDevice;
  Aquire.usDeviceNumber = usEnumDevice;
  ULONG ulCat, ulFunc;
  ulCat  = 0xA0;
  ulFunc = 0x33;
  ulParmLen = sizeof(Aquire);
  ulDataLen = sizeof(USBHANDLE);
  rc = DosDevIOCtl( g_hUSBDrv,
                    ulCat,ulFunc, //IOCAT_USBRES, IOCTLF_AQUIREDEVICE,
                    &Aquire, ulParmLen, &ulParmLen,
                    pHandle, ulDataLen, &ulDataLen);

  // @@ ToDO maybe gether some info about device here (endpoints etc for savety checks)
  return rc;

}

APIRET APIENTRY UsbClose( USBHANDLE Handle)
{
  APIRET rc;
  ULONG ulDataLen,ulParmLen;
  if(!g_fInit)
    return USB_NOT_INIT;

  ulParmLen = sizeof(USBHANDLE);
  ulDataLen = 0;

  rc = DosDevIOCtl( g_hUSBDrv,
                    IOCAT_USBRES, IOCTLF_RELEASEDEVICE,
                    (PVOID)&Handle, ulParmLen, &ulParmLen,
                    NULL, ulDataLen, &ulDataLen);
  return rc;
}

APIRET APIENTRY UsbCtrlMessage( USBHANDLE Handle,
                       UCHAR  ucRequestType,
                       UCHAR  ucRequest,
                       USHORT usValue,
                       USHORT usIndex,
                       USHORT usLength,
                       UCHAR  *pData,
                       ULONG  ulTimeout)
{
  APIRET rc;
  USBCALLS_CTRL_REQ CtrlRequest;
  ULONG   ulParmLen, ulDataLen;

  if(!g_fInit)
    return USB_NOT_INIT;

  ulParmLen = sizeof(USBCALLS_CTRL_REQ);
  CtrlRequest.ulHandle     = Handle;
  CtrlRequest.bRequestType = ucRequestType;
  CtrlRequest.bRequest     = ucRequest;
  CtrlRequest.wValue       = usValue;
  CtrlRequest.wIndex       = usIndex;
  CtrlRequest.wLength      = usLength;
  CtrlRequest.ulTimeout    = ulTimeout;
  ulDataLen = usLength;

  rc = DosDevIOCtl( g_hUSBDrv,
                    IOCAT_USBRES, IOCTLF_SENDCONTROLURB,
                    (PVOID)&CtrlRequest, ulParmLen, &ulParmLen,
                    ulDataLen>0?(PVOID)pData:NULL,
                    ulDataLen,
                    ulDataLen>0?&ulDataLen:NULL);
  return rc;
}

APIRET APIENTRY UsbBulkRead( USBHANDLE Handle,
                    UCHAR  Endpoint,
                    UCHAR  Interface,
                    ULONG  *ulNumBytes,
                    UCHAR  *pData,
                    ULONG  ulTimeout)
{
  APIRET            rc;
  ULONG             ulParmLen, ulDataLen, ulToProcess, ulTotalProcessed;
  USBCALLS_BULK_REQ BulkRequest;
  ULONG             ulTempEventCount;
  UCHAR             ucTempBuf[0x2000];
  PUCHAR            pucBuffer = (PUCHAR)(((ULONG)&ucTempBuf+0x1000) & 0xFFFFF000);
  BOOL fAligned;

  if(!g_fInit)
    return USB_NOT_INIT;

  // 10 01 2003 - KIEWITZ -> Still @@ToDo Add Endpoint check based on descriptors
  // We currently only allow Endpoint-addresses 80h->8Fh here
  if ((Endpoint<0x80) || (Endpoint>0x8F))
     return USB_ERROR_INVALID_ENDPOINT;

  if(*ulNumBytes==0)
     return 0;

  rc = DosCreateEventSem( NULL,
                         (PHEV)&BulkRequest.ulEventDone,
                         DC_SEM_SHARED,
                         FALSE);
  if(rc)
     return rc;

  BulkRequest.ulDevHandle = Handle;
  BulkRequest.ucEndpoint  = Endpoint;
  BulkRequest.ucInterface = Interface;
  ulParmLen               = sizeof(USBCALLS_BULK_REQ);
  ulToProcess             = *ulNumBytes;
  ulTotalProcessed        = 0;

  fAligned = ((ULONG)pData & 0x0FFF)==0;

  do 
  {
    BulkRequest.usDataProcessed = 0;

    // Copy as much as 4k
    ulDataLen = (ulToProcess<=0x1000)?0x1000:ulToProcess;

    if(fAligned)    
      pucBuffer = pData;

    rc = DosDevIOCtl( g_hUSBDrv,
                      IOCAT_USBRES, IOCTLF_SENDBULKURB,
                      (PVOID)&BulkRequest, ulParmLen, &ulParmLen,
                      pucBuffer, ulDataLen, &ulDataLen);

    if (rc==NO_ERROR) 
    {
      // We made the request, now wait for it to finish...
      rc = DosWaitEventSem( (HEV)BulkRequest.ulEventDone,
                            ulTimeout==0?0xFFFFFFFF:ulTimeout);
      if (rc==ERROR_TIMEOUT) 
      {
        // We experienced a Timeout, so abort Bulk-Writing
        DosDevIOCtl( g_hUSBDrv,
                     IOCAT_USBRES, IOCTLF_CANCEL_IORB,
                     (PVOID)&BulkRequest, ulParmLen, &ulParmLen,
                     NULL, 0, NULL);
        break;
      }
      // Reset semamorph for next block (if available)
      DosResetEventSem( (HEV)BulkRequest.ulEventDone, &ulTempEventCount);
    }
    else 
      break;

    if(!fAligned)
       memcpy(pData,pucBuffer,BulkRequest.usDataProcessed);

    // Adjust count and source pointer
    ulToProcess      -= ulDataLen;
    pData            += ulDataLen;
    ulTotalProcessed += BulkRequest.usDataProcessed;

    if (BulkRequest.usDataProcessed!=ulDataLen) 
    {
      // Transfered less than we wanted? so something is wrong, abort
      rc = USB_ERROR_LESSTRANSFERED; 
      break;
    }
  } while (ulToProcess>0);

  DosCloseEventSem( (HEV)BulkRequest.ulEventDone);
  *ulNumBytes = ulTotalProcessed;
  return rc;
}

APIRET APIENTRY UsbBulkWrite( USBHANDLE Handle,
                     UCHAR  Endpoint,
                     UCHAR  Interface,
                     ULONG ulNumBytes,
                     UCHAR  *pData,
                     ULONG  ulTimeout)
{
  APIRET            rc;
  ULONG             ulParmLen, ulDataLen;
  USBCALLS_BULK_REQ BulkRequest;
  ULONG             ulTempEventCount;
  UCHAR             ucTempBuf[0x2000];
  PUCHAR            pucBuffer = (PUCHAR)(((ULONG)&ucTempBuf+0x1000) & 0xFFFFF000);
  BOOL fAligned;

  if(!g_fInit)
    return USB_NOT_INIT;

  // 10 01 2003 - KIEWITZ -> Still @@ToDo Add Endpoint check based on descriptors
  // We currently only allow Endpoint-addresses 00h->0Fh here
  if ((Endpoint>0x0F))
     return USB_ERROR_INVALID_ENDPOINT;

  if (ulNumBytes==0)
     return 0;

  rc = DosCreateEventSem( NULL,
                         (PHEV)&BulkRequest.ulEventDone,
                         DC_SEM_SHARED,
                         FALSE);
  if(rc)
     return rc;

  BulkRequest.ulDevHandle = Handle;
  BulkRequest.ucEndpoint  = Endpoint;
  BulkRequest.ucInterface = Interface;
  ulParmLen = sizeof(USBCALLS_BULK_REQ);

  fAligned = ((ULONG)pData & 0x0FFF)==0;

  do
  {
    // Copy as much as 4k
    ulDataLen = (ulNumBytes<=0x1000)?0x1000:ulNumBytes;

    if(fAligned)    
      pucBuffer = pData;
    else
      memcpy(pucBuffer, pData, ulDataLen);

    rc = DosDevIOCtl( g_hUSBDrv,
                      IOCAT_USBRES, IOCTLF_SENDBULKURB,
                      (PVOID)&BulkRequest, ulParmLen, &ulParmLen,
                      pucBuffer, ulDataLen, &ulDataLen );

    if (rc==NO_ERROR)
    {
      // We made the request, now wait for it to finish...
      rc = DosWaitEventSem( (HEV)BulkRequest.ulEventDone,
                            ulTimeout==0?0xFFFFFFFF:ulTimeout);

      if (rc==ERROR_TIMEOUT) 
      {
        // We experienced a Timeout, so abort Bulk-Writing
        DosDevIOCtl( g_hUSBDrv,
                     IOCAT_USBRES, IOCTLF_CANCEL_IORB,
                     (PVOID)&BulkRequest, ulParmLen, &ulParmLen,
                     NULL, 0, NULL);
        break;
      }

      // Reset semamorph for next block (if available)
      DosResetEventSem( (HEV)BulkRequest.ulEventDone, &ulTempEventCount);
    } 
    else 
      break;

    // Adjust count and source pointer
    ulNumBytes -= ulDataLen;
    pData      += ulDataLen;
  } while (ulNumBytes>0);

  DosCloseEventSem( (HEV)BulkRequest.ulEventDone);
  return rc;
}

APIRET APIENTRY UsbIrqStart( USBHANDLE Handle,
                    UCHAR  Endpoint,
                    UCHAR  Interface,
                    USHORT ulNumBytes,
                    UCHAR  *pData,
                    PHEV   pHevModified)
{
  APIRET rc;
  ULONG ulParmLen, ulDataLen;
  USBCALLS_IRQ_START IrqStart;
  HEV hEvent;

  if(!g_fInit)
    return USB_NOT_INIT;

  if(0==ulNumBytes || IsBadWritePointer(pData, ulNumBytes))
    return ERROR_INVALID_PARAMETER;

  rc = DosCreateEventSem( NULL,
                          &hEvent,
                          DC_SEM_SHARED,
                          FALSE);
  if(rc)
    return rc;

  IrqStart.ulDevHandle = Handle;
  IrqStart.ulIrqEvent  = hEvent;
  IrqStart.ucEndpoint  = Endpoint;
  IrqStart.ucInterface = Interface;
  ulParmLen = sizeof(IrqStart);
  ulDataLen = ulNumBytes;

  rc = DosDevIOCtl( g_hUSBDrv,
                    IOCAT_USBRES, IOCTLF_START_IRQ_PROC,
                    (PVOID)&IrqStart, ulParmLen, &ulParmLen,
                    pData, ulDataLen,&ulDataLen);
  if(rc)
    DosCloseEventSem(hEvent);
  else
    *pHevModified = hEvent;
  return rc;
}

APIRET APIENTRY UsbIrqStop(  USBHANDLE Handle,
                    HEV       HevModified)
{
  APIRET rc;
  ULONG ulParmLen, ulDataLen;

  if(!g_fInit)
    return USB_NOT_INIT;

  ulParmLen = sizeof(Handle);
  ulDataLen = sizeof(HevModified);
  rc = DosDevIOCtl( g_hUSBDrv,
                    IOCAT_USBRES, IOCTLF_STOP_IRQ_PROC,
                    (PVOID)&Handle, ulParmLen, &ulParmLen,
                    &HevModified, ulDataLen, &ulDataLen);
  if(!rc)
    DosCloseEventSem(HevModified);

  return rc;
}

APIRET APIENTRY UsbIsoStart( USBHANDLE Handle,
                    UCHAR  Endpoint,
                    UCHAR  Interface,
                    ISOHANDLE *phIso)
{
  APIRET rc;
  PISORINGBUFFER pIter = g_pIsoRingBuffers;
  USBCALLS_ISO_START IsoStart;
  ULONG ulParmLen, ulDataLen;

  if(!g_fInit)
    return USB_NOT_INIT;

  rc = DosRequestMutexSem(g_hSemRingBuffers,SEM_INDEFINITE_WAIT);
  if(rc)
    return rc;

  for(ULONG i=0;i< g_ulNumIsoRingBuffers;i++,pIter++)
  {
    if (pIter->hDevice==0)
    {
      pIter->hDevice = Handle;
      break;
    }
  }
  DosReleaseMutexSem(g_hSemRingBuffers);

  if(i==g_ulNumIsoRingBuffers)
    return USB_ERROR_OUTOF_RESOURCES;

  IsoStart.ulDevHandle = Handle;
  IsoStart.ucEndpoint  = Endpoint;
  IsoStart.ucInterface = Interface;
  ulParmLen = sizeof(IsoStart);
  ulDataLen = sizeof(ISORINGBUFFER);

  rc = DosDevIOCtl( g_hUSBDrv,
                    IOCAT_USBRES, IOCTLF_STOP_IRQ_PROC,
                    (PVOID)&IsoStart, ulParmLen, &ulParmLen,
                    pIter, ulDataLen, &ulDataLen);
  if(rc)
  {
    pIter->hDevice = 0;
    *phIso = 0;
  }
  else
  {
    pIter->ucEndpoint  = Endpoint;
    pIter->ucInterface = Interface;
  }
  return rc;
}

APIRET IsInvalidIsoHandle(const ISOHANDLE hIso)
{
  PISORINGBUFFER pIter;
  ULONG i;
  pIter = g_pIsoRingBuffers;

  for(i=0;i<g_ulNumIsoRingBuffers;i++,pIter++)
  {
    if(pIter==(PISORINGBUFFER)hIso && pIter->hDevice)
      return 0;
  }
  return ERROR_INVALID_PARAMETER;
}

APIRET APIENTRY UsbIsoStop( ISOHANDLE hIso)
{

  APIRET rc;
  if(!g_fInit)
    return USB_NOT_INIT;

//  rc = DosDevIOCtl( g_hUSBDrv,
  return rc;
}

APIRET APIENTRY UsbIsoDequeue( ISOHANDLE hIso,
                      UCHAR * pBuffer,
                      ULONG ulNumBytes)
{
  APIRET rc;
  PISORINGBUFFER pRB = (PISORINGBUFFER)hIso;

  rc = IsInvalidIsoHandle(hIso);
  if(rc)
    return rc;
  if(!(pRB->ucEndpoint & ISO_DIRMASK))
    return ERROR_INVALID_PARAMETER;

  return rc;
}

APIRET APIENTRY UsbIsoPeekQueue( ISOHANDLE hIso,
                        UCHAR * pByte,
                        ULONG ulOffset)
{
  APIRET rc;
  PISORINGBUFFER pRB = (PISORINGBUFFER)hIso;

  rc = IsInvalidIsoHandle(hIso);
  if(rc)
    return rc;
  if(!(pRB->ucEndpoint & ISO_DIRMASK))
    return ERROR_INVALID_PARAMETER;
  return rc;
}

APIRET APIENTRY UsbIsoEnqueue( ISOHANDLE hIso,
                      const UCHAR * pBuffer,
                      ULONG ulNumBytes)
{
  APIRET rc;
  PISORINGBUFFER pRB = (PISORINGBUFFER)hIso;

  rc = IsInvalidIsoHandle(hIso);
  if(rc)
    return rc;
  if(pRB->ucEndpoint & ISO_DIRMASK)
    return ERROR_INVALID_PARAMETER;

  return rc;
}

APIRET APIENTRY UsbIsoGetLength( ISOHANDLE hIso,
                        ULONG *pulLength)
{
  APIRET rc;
  PISORINGBUFFER pRB = (PISORINGBUFFER) hIso;
  USHORT ri,wi;

  rc = IsInvalidIsoHandle(hIso);
  if(rc)
    return rc;
  wi = pRB->usPosWrite;
  ri = pRB->usPosRead;

  if (ri == wi)
    *pulLength = 0;
  else if (ri < wi)
    *pulLength =  wi - ri;
  else
    *pulLength = wi + (pRB->usBufSize - ri);

  return 0;
}



 /*+-------------------------------------------------------------------+*/
 /*| _CRT_init is the C run-time environment initialization function.  |*/
 /*|It will return 0 to indicate success and -1 to indicate failure.   |*/
 /*+-------------------------------------------------------------------+*/

// int _CRT_init (void);

 /*+-------------------------------------------------------------------+*/
 /*| _CRT_term is the C run-time environment termination function.     |*/
 /*+-------------------------------------------------------------------+*/

// void _CRT_term (unsigned long);

 /*+-------------------------------------------------------------------+*/
 /*| _DLL_InitTerm is the function that gets called by the operating   |*/
 /*| system loader when it loads and frees this DLL for each process   |*/
 /*| that accesses this DLL.  However, it only gets called the first   |*/
 /*| time the DLL is loaded and the last time it is freed for a        |*/
 /*| particular process.  The system linkage convention must be used   |*/
 /*| because the operating system loader is calling this function.     |*/
 /*+-------------------------------------------------------------------+*/

int _CRT_init (void);
#ifdef STATIC_LINK
void  _CRT_term(0UL);
#endif

ULONG _System _DLL_InitTerm (unsigned long modhandle, unsigned long flag)
{

  /* If flag is zero then the DLL is being loaded so initialization  */
  /* should be performed.  If flag is 1 then the DLL is being freed  */
  /* so termination should be performed.                             */

  switch (flag)
  {
    case 0:
    /* The C run-time environment initialization function must   */
    /* be called before any calls to C run-time functions that   */
    /* are not inlined.                                          */

      if (_CRT_init () == -1)
         return 0UL;
      InitUsbCalls();
      break;

    case 1:
      if(g_fInit)
      {
        for(int i=0;i<MAX_NOTIFICATIONS;i++)
        {
          if( g_Notifications[i].usFlags != NOTIFY_FREE);
          {

            // @@ ToDo deregister all Eventes in the driver on unload
          }
        }
        DosClose(g_hUSBDrv);
      }

#ifdef STATIC_LINK
      _CRT_term(0UL);
#endif
      break;

    default:
      return 0UL;

  }

  /* A nonzero value must be returned to indicate success. */
  return 1UL;
}

#ifdef __cplusplus
} // extern "C"
#endif


