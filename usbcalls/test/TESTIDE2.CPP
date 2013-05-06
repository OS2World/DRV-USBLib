#define INCL_DOS
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(1)

typedef struct
{
  USHORT usVendorID;
  USHORT usDeviceID;
  USHORT usBCDDevice;
  USHORT usDeviceNumber; // Get the usDeviceNumber device in the system fi. if 2 aquire the 2nd device
                         // 0 means first not aquired device.
}AQUIREDEV, *NPAQUIREDEV, FAR *PAQUIREDEV;

typedef struct
{
  ULONG  ulHandle;
  USHORT usLangID;
  UCHAR  ucStringID;
  UCHAR  ucReserved;
}GETSTRINGPARM, *NGETSTRINGPARM, FAR *PGETSTRINGPARM;

typedef struct
{
  UCHAR  bRequestType;
  UCHAR  bRequest;
  USHORT wValue;
  USHORT wIndex;
  USHORT wLength;
  ULONG  ulTimeout; /* in milliseconds */
}SETUPPACKET, *NSETUPPACKET, FAR *PSETUPPACKET;

typedef struct
{
  ULONG  ulHandle;
  UCHAR  bRequestType;
  UCHAR  bRequest;
  USHORT wValue;
  USHORT wIndex;
  USHORT wLength;
  ULONG  ulTimeout; /* in milliseconds */
}LIBUSB_CTRL_REQ, *NPLIBUSB_CTRL_REQ, FAR *PLIBUSB_CTRL_REQ;

typedef struct
{
  ULONG  ulHandle;
  ULONG  ulTimeout; /* in milliseconds */
  UCHAR  Endpoint;
  UCHAR  Interface;
  USHORT usDataLen;
  UCHAR  Data[1];
}LIBUSB_BULK_REQ, *NPLIBUSB_BULK_REQ, FAR *PLIBUSB_BULK_REQ;

#pragma pack()

HFILE hDriver;

int sendCtrlReq( ULONG ulUSBHandle,
                  PSETUPPACKET pReq,
                  UCHAR* pData)
{
  ULONG i,ulParmLen, ulDataLen;
  LIBUSB_CTRL_REQ CtrlRequest;
  int rc;
  UCHAR *pSP;
  ulParmLen = sizeof(LIBUSB_CTRL_REQ);
  CtrlRequest.ulHandle     = ulUSBHandle;
  CtrlRequest.bRequestType = pReq->bRequestType;
  CtrlRequest.bRequest     = pReq->bRequest;
  CtrlRequest.wValue       = pReq->wValue;
  CtrlRequest.wIndex       = pReq->wIndex;
  CtrlRequest.wLength      = pReq->wLength;
  CtrlRequest.ulTimeout    = pReq->ulTimeout;
  ulDataLen = pReq->wLength;
#if 0
  pSP = (UCHAR*)pReq;
  printf("SetupPacket : ");
  for(i=0;i<(sizeof(SETUPPACKET)-4);i++)
    printf("%02X ",*(pSP+i));
  printf("\r\n");    
  if((pReq->bRequestType & 0x80)==0 && ulDataLen>0)
  {
    printf("Sending CtrlData: ");
    for(i=0;i<ulDataLen;i++)
      printf("%02X ",*(pData+i));
    printf("\r\n");    
  }
#endif
  rc = DosDevIOCtl( hDriver, 0xA0, 0x36, // Send Ctrl Req
                    (PVOID)&CtrlRequest, ulParmLen, &ulParmLen,
                    ulDataLen>0?(PVOID)pData:NULL, 
                    ulDataLen, 
                    ulDataLen>0?&ulDataLen:NULL);
  if(rc)
    printf("Send Ctrl Req, rc=%d\r\n",rc);
  if(ulDataLen && (pReq->bRequestType &0x80)== 0x80)
  {
    printf("Returned Data: ");    
    for(i=0;i<ulDataLen;i++)
    {
      printf("%02X ",*(pData+i));
      if(((i+1)%16)==0)
       printf("\r\n               ");    
    }
    printf("\r\n");    
  }
  return rc;
}


int sendBulkReq( ULONG ulUSBHandle,
                 ULONG  ulTimeout,
                 UCHAR  Endpoint,
                 UCHAR  Interface,
                 USHORT usDataLen,
                 UCHAR* pData)
{
  ULONG i,ulParmLen, ulDataLen;
  PLIBUSB_BULK_REQ pBulkRequest;
  int rc;
  UCHAR *pBR;
  if(Endpoint & 0x80) // Dir In
  { 
    ulParmLen = sizeof(LIBUSB_BULK_REQ);
    pBR = (UCHAR*) malloc(ulParmLen);
    pBulkRequest = (PLIBUSB_BULK_REQ) pBR;
    ulDataLen = usDataLen;
  }
  else
  {
    ulParmLen = usDataLen+sizeof(LIBUSB_BULK_REQ);
    pBR = (UCHAR*) malloc(ulParmLen);
    pBulkRequest = (PLIBUSB_BULK_REQ) pBR;
    memcpy(pBulkRequest->Data,pData, usDataLen);
    ulDataLen = 0;
  }
  ulDataLen = sizeof(LIBUSB_CTRL_REQ);
  pBulkRequest->ulHandle  = ulUSBHandle;
  pBulkRequest->Endpoint  = Endpoint;
  pBulkRequest->ulTimeout = ulTimeout;
  pBulkRequest->Interface = pBulkRequest->Interface;
  pBulkRequest->usDataLen = usDataLen;

#if 1
  if((pBulkRequest->Endpoint & 0x80)==0 && usDataLen>0)
  {
    printf("Sending BulkData: ");
    for(i=0;i<usDataLen;i++)
    {
      printf("%02X ",pBulkRequest->Data[i]);
      if(((i+1)%16)==0)
       printf("\r\n               ");    
    }
    printf("\r\n");    
  }
#endif
  rc = DosDevIOCtl( hDriver, 0xA0, 0x37, // Send Bulk Req
                    (PVOID)pBR, ulParmLen, &ulParmLen,
                    ulDataLen>0?(PVOID)pData:NULL, 
                    ulDataLen, 
                    ulDataLen>0?&ulDataLen:NULL);
  if(rc)
    printf("Send Bulk Req, rc=%d\r\n",rc);
  if(ulDataLen && (pBulkRequest->Endpoint &0x80)== 0x80)
  {
    printf("Returned Data: ");    
    for(i=0;i<ulDataLen;i++)
    {
      printf("%02X ",*(pData+i));
      if(((i+1)%16)==0)
       printf("\r\n               ");    
    }
    printf("\r\n");    
  }
  return rc;
}

int SenseDevice(HFILE hDrv, ULONG ulUSBHandle)
{
  int rc;
//  UCHAR ucCommandSense[8] = { 0x00, 0x01, 0x00, 0x00, 
//                              0x00, 0xa0, 0xec, 0x01};
  UCHAR ucCommandSense[0x1f] = { 0x55, 0x53, 0x42, 0x43, 0x00, 0x00, 0x00, 0x00, 
                                 0x60, 0x00, 0x00, 0x00, 0x80, 0x00, 0x0c, 0x12, 
                                 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  UCHAR ucData[512];
  memset(ucData,0,sizeof(ucData));
  rc = sendBulkReq( ulUSBHandle, 0, 0x02, 0x00, 0x1f, ucCommandSense);
  //if (rc==0)
    rc = sendBulkReq(ulUSBHandle, 0, 0x81, 0x00, 0x60, ucData);
  //if (rc==0)
    rc = sendBulkReq(ulUSBHandle, 0, 0x81, 0x00, 0x0d, ucData);
  return rc;
}

int ReadSector(HFILE hDrv, ULONG ulUSBHandle, ULONG ulSector)
{
  int rc;
  UCHAR ucCommandRead[8] = { 0x00, 0x01, 0x00, 0x00, 
                             0x00, 0xe0, 0x20, 0x01};
  UCHAR ucData[512];
  rc = sendBulkReq( ulUSBHandle, 0, 0x01, 0x00, 8, ucCommandRead);
  return rc==0? sendBulkReq(ulUSBHandle, 0, 0x82, 0x00, 512, ucData):rc;
}


void main()
{
  int rc,i;
  ULONG ulAction, ulParmLen, ulDataLen, ulUSBHandle, *pData;
  AQUIREDEV  USBDevice;
  SETUPPACKET SPConfigure;
  UCHAR ucBuffer[640];
  rc = DosOpen( "USBRESM$",
                &hDriver,
                &ulAction,
                0,
                FILE_NORMAL,
                OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_ACCESS_READWRITE |
                OPEN_FLAGS_NOINHERIT |
                OPEN_SHARE_DENYNONE,
                0 );
  printf("Open returned %d\r\n",rc);

  ulDataLen = sizeof(ulUSBHandle);
  ulUSBHandle = 0;
  rc = DosDevIOCtl( hDriver, 0xA0, 0x31, // Aquire Device
                    NULL,0,0,
                    (PVOID)&ulUSBHandle, ulDataLen, &ulDataLen);
    printf("%d USB Devices\r\n",ulUSBHandle);

  if(!rc)
  {
    USBDevice.usVendorID = 0x04CE;
    USBDevice.usDeviceID = 0x0002;
    USBDevice.usBCDDevice = 0xFFFF; //0x026C;
    USBDevice.usDeviceNumber = 0; // Get the usDeviceNumber device in the system fi. if 2 aquire the 2nd device
    ulParmLen = sizeof(USBDevice);
    ulDataLen = sizeof(ulUSBHandle);
    ulUSBHandle = 0xdeadbeef;
    rc = DosDevIOCtl( hDriver, 0xA0, 0x33, // Aquire Device
                      (PVOID)&USBDevice, ulParmLen, &ulParmLen,
                      (PVOID)&ulUSBHandle, ulDataLen, &ulDataLen);
    printf("Aquire returned %d Handle is 0x%08x\r\n",rc,ulUSBHandle);
  }

  if(!rc && ulUSBHandle!=0)
  {
    UCHAR ucData[20];
    SPConfigure.bRequestType = 0; // standard
    SPConfigure.bRequest     = 9; // Set config
    SPConfigure.wValue       = 1;
    SPConfigure.wIndex       = 0;
    SPConfigure.wLength      = 0;
    SPConfigure.ulTimeout    = 0;

    rc = sendCtrlReq( ulUSBHandle, &SPConfigure, NULL);
    memset(ucData,0,sizeof(ucData));
    SPConfigure.bRequestType = 0x80; // standard
    SPConfigure.bRequest     = 0x08; // Get config
    SPConfigure.wValue       = 0;
    SPConfigure.wIndex       = 0;
    SPConfigure.wLength      = sizeof(ucData);
    SPConfigure.ulTimeout    = 0;
    rc = sendCtrlReq( ulUSBHandle, &SPConfigure, ucData);
    memset(ucData,0,sizeof(ucData));
    ucData[0] = 0x05;
    SPConfigure.bRequestType = 0xC0; 
    SPConfigure.bRequest     = 0xFF; 
    SPConfigure.wValue       = 0x0007;
    SPConfigure.wIndex       = 0xfffc;
    SPConfigure.wLength      = 2;
    SPConfigure.ulTimeout    = 0;
    rc = sendCtrlReq( ulUSBHandle, &SPConfigure, ucData);
    memset(ucData,0,sizeof(ucData));
    SPConfigure.bRequestType = 0xC0; 
    SPConfigure.bRequest     = 0xFF; 
    SPConfigure.wValue       = 0x0007;
    SPConfigure.wIndex       = 0x00ae;
    SPConfigure.wLength      = 2;
    SPConfigure.ulTimeout    = 0;
    rc = sendCtrlReq( ulUSBHandle, &SPConfigure, ucData);
    memset(ucData,0,sizeof(ucData));
    SPConfigure.bRequestType = 0xa0; 
    SPConfigure.bRequest     = 0xFE; 
    SPConfigure.wValue       = 0x0000;
    SPConfigure.wIndex       = 0x0000;
    SPConfigure.wLength      = 1;
    SPConfigure.ulTimeout    = 0;
    rc = sendCtrlReq( ulUSBHandle, &SPConfigure, ucData);

    rc = SenseDevice(hDriver,ulUSBHandle);
//    if(!rc)
//      rc = ReadSector(hDriver,ulUSBHandle,0);

    ulParmLen = sizeof(ulUSBHandle);
    ulDataLen = 0;
    rc = DosDevIOCtl( hDriver, 0xA0, 0x34, // Release Device
                      (PVOID)&ulUSBHandle, ulParmLen, &ulParmLen,
                      NULL, ulDataLen, &ulDataLen);
  }

  DosClose(hDriver);
}