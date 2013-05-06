#define INCL_DOS
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>

#pragma pack(1)

typedef struct
{
  USHORT usVendorID;
  USHORT usDeviceID;
  USHORT usBCDDevice;
  USHORT usDeviceNumber; // Get the usDeviceNumber device in the system fi. if 2 aquire the 2nd device
                         // 0 means first not aquired device.
}AQUIREDEV,  *PAQUIREDEV;

typedef struct
{
  ULONG  ulHandle;
  USHORT usLangID;
  UCHAR  ucStringID;
  UCHAR  ucReserved;
}GETSTRINGPARM, *PGETSTRINGPARM;

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
}LIBUSB_CTRL_REQ, *PLIBUSB_CTRL_REQ;

typedef struct
{
  ULONG  ulHandle;
  ULONG  ulTimeout; /* in milliseconds */
  UCHAR  Endpoint;
  UCHAR  Interface;
  USHORT usDataLen;
  UCHAR  Data[1];
}LIBUSB_BULK_REQ, *PLIBUSB_BULK_REQ;

#define PLUSMAGIC 0x554C4E4B
typedef struct
{
  ULONG ulMagic;
  ULONG ulTag;
  ULONG ulLength;
}LNK_HEADER;

typedef struct
{
  LNK_HEADER Header;
  ULONG ulStatus;
}LNK_STATUS;

typedef struct
{
  LNK_HEADER Header;
  ULONG ulFileSize;
  UCHAR szFileName[256];
}LNK_FILE_NAME;

typedef struct
{
  LNK_HEADER Header;
  ULONG ulHandle;
}LNK_FILE_ACCEPT;

typedef struct
{
  LNK_HEADER Header;
  ULONG ulHandle;
  UCHAR ucData[4];
}LNK_FILE_DATA;

#define LNK_TAG_STATUS      1
#define LNK_TAG_FILE_NAME   2
#define LNK_TAG_FILE_ACCEPT 3
#define LNK_TAG_FILE_DATA   4
#define LNK_TAG_FILE_DONE   5
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

int ulReadHeader(HFILE hDrv, LNK_HEADER *pHeader)
{
  ULONG ulActual;
  int rc;
  rc = DosRead(hDrv, pHeader, sizeof(LNK_HEADER), &ulActual);
  {
   printf("DosRead rc=%d ulActual %d\r\n",rc, ulActual);
    return 1;
  }
  if(ulActual != sizeof(LNK_HEADER))
    return 2;
  if(pHeader->ulMagic != PLUSMAGIC)
    return 3;
  return 0;
}

ULONG ulGetStatus(HFILE hDrv)
{
  LNK_STATUS USBStatus;
  ULONG ulActual, lrc;
  int rc;
  rc = ulReadHeader(hDrv, &USBStatus.Header);
  if(rc)
    return rc;
  if(USBStatus.Header.ulTag != LNK_TAG_STATUS ||
     USBStatus.Header.ulLength != sizeof(ULONG) )
    return 4;
  rc = DosRead(hDrv, &USBStatus.ulStatus, sizeof(ULONG), &ulActual);
  if(sizeof(ULONG)!=ulActual)
    return 5;
  return USBStatus.ulStatus;
}

int SendStatus(HFILE hDrv, ULONG ulStatus)
{
  LNK_STATUS USBStatus;
  ULONG ulActual;
  int rc;
  USBStatus.Header.ulMagic  = PLUSMAGIC;
  USBStatus.Header.ulTag    = LNK_TAG_STATUS;
  USBStatus.Header.ulLength = sizeof(ULONG);
  USBStatus.ulStatus        = ulStatus;
  rc = DosWrite(hDrv, &USBStatus, sizeof(USBStatus), &ulActual);
  if(rc)
    return rc;
  if(ulActual != sizeof(USBStatus))
    return 0xdeadbeef;
  return 0;
}

int SendAccept(HFILE hDrv, HFILE hLocFile)
{
  LNK_FILE_ACCEPT FileAccept;
  ULONG ulActual;
  int rc;

  FileAccept.Header.ulMagic  = PLUSMAGIC;
  FileAccept.Header.ulTag    = LNK_TAG_FILE_ACCEPT;
  FileAccept.Header.ulLength = sizeof(ULONG);
  FileAccept.ulHandle        = (ULONG)hLocFile;
  rc = DosWrite(hDrv, &FileAccept, sizeof(FileAccept), &ulActual);
  if(rc)
    return rc;
  if(ulActual != sizeof(FileAccept))
    return 0xdeadbeef;
  return 0;
}

int ReadAccept(HFILE hDrv, ULONG *pulHandle)
{
  LNK_FILE_ACCEPT FileAccept;
  ULONG ulActual;
  int rc;

  rc = ulReadHeader(hDrv, &FileAccept.Header);
  if(rc)
    return rc;
  if(FileAccept.Header.ulTag!=LNK_TAG_FILE_ACCEPT)
    return 108;
  rc = DosRead(hDrv, &FileAccept.ulHandle, sizeof(ULONG), &ulActual);
  if(ulActual != sizeof(ULONG))
    return 0xdeadbeef;
  *pulHandle = FileAccept.ulHandle;
  return 0;
}

int ReadFileBlock(HFILE hDrv, HFILE hLocFile)
{
  LNK_FILE_DATA *pData;
  int rc;
  LNK_HEADER Header;
  ULONG ulActual;

  rc = ulReadHeader(hDrv,&Header);
  if(rc)
  {
    printf("Error %d reading Header",rc);
    return rc;
  }
  if(LNK_TAG_FILE_DATA !=Header.ulTag)
  {
    printf("Error Wrong Tag %d ",Header.ulTag);
    return rc;
  }
  if(Header.ulLength <8)
  {
    printf("Wrong Datasize %d",Header.ulLength);
    return 102;
  }
  pData = (LNK_FILE_DATA*) malloc(Header.ulLength+ sizeof(Header) );
  rc = DosRead(hDrv,&pData->ulHandle, Header.ulLength, &ulActual);
  if(!rc)
    rc = DosWrite(hLocFile,&pData->ucData[0],Header.ulLength-4, &ulActual);
  SendStatus(hDrv,rc);
  free (pData);
  return rc;
}

int SendFileBlock(HFILE hDrv, HFILE hLocFile, ULONG ulRemoteHandle, ULONG ulLength)
{
  LNK_FILE_DATA *pData;
  int rc;
  ULONG ulActual, ulStat;

  pData = (LNK_FILE_DATA*) malloc( ulLength + sizeof(LNK_HEADER) + 4);
  pData->Header.ulMagic  = PLUSMAGIC;
  pData->Header.ulTag    = LNK_TAG_FILE_DATA;
  pData->Header.ulLength = ulLength+4;
  pData->ulHandle = ulRemoteHandle;
  rc = DosRead(hLocFile,&pData->ucData[0], ulLength, &ulActual);
  if(!rc)
    rc = DosWrite(hDrv,pData,ulLength+4+sizeof(LNK_HEADER), &ulActual);
  else
    return rc;
  free(pData);
  ulStat = ulGetStatus(hDrv);
  return ulStat;
}


void RecvFile(HFILE hDrv)
{
  int rc;
  HFILE hLocFile;
  LNK_FILE_NAME FileName;
  ULONG ulActual,ulAction, ulBytesRecv=0;
  char  ch;
  rc = ulReadHeader(hDrv,&FileName.Header);
  if(rc)
  {
    printf("Error %d reading Header",rc);
    return;
  }
  if(LNK_TAG_FILE_NAME !=FileName.Header.ulTag)
  {
    printf("Error Wrong Tag %d ",FileName.Header.ulTag);
    return;
  }
  if(FileName.Header.ulLength !=sizeof(FileName)-sizeof(FileName.Header))
  {
    printf("Error Wrong DataSize %d ",FileName.Header.ulLength);
    return;
  }

  rc = DosRead( hDrv,
                &FileName.ulFileSize,
                sizeof(FileName)-sizeof(FileName.Header),
                &ulActual);
  if(rc)
  {
    printf("Error %d reading Filename",rc);
    return;
  }

  if(ulActual !=(sizeof(FileName)-sizeof(FileName.Header)))
  {
    printf("Not all Data read %d ",ulActual);
    return;
  }

  printf("Receive File %s Size %d Bytes (Y/N)\r\n ?",
         FileName.szFileName,FileName.ulFileSize);
  do
  {
    ch = toupper(_getch());
  }while(ch!='Y' && ch!='N');

  if(ch=='N')
  {
    SendStatus(hDrv,1111);
    return;
  }
  rc = DosOpen( (PCSZ)&FileName.szFileName[0],
                &hLocFile,
                &ulAction,
                FileName.ulFileSize,
                FILE_NORMAL,
                OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_ACCESS_READWRITE |
                OPEN_FLAGS_NOINHERIT |
                OPEN_SHARE_DENYNONE,
                0 );
  if(rc)
  {
    printf("Error %d opening local File\r\n",rc);
    SendStatus(hDrv,100);
  }
  rc = SendStatus(hDrv,0);
  if(rc)
  {
    DosClose(hLocFile);
    printf("Error %d sending Status\r\n",rc);
    return;
  }
  rc = SendAccept(hDrv, hLocFile);
  if(rc)
  {
    DosClose(hLocFile);
    printf("Error %d sending Accept\r\n",rc);
    return;
  }

  while (!rc && ulBytesRecv<FileName.ulFileSize)
  {
    rc = ReadFileBlock(hDrv,hLocFile);
  }
  DosClose(hLocFile);
  if(rc)
  {
    printf("Error %d during File transfer.\r\n");
  }
}

void SendFile(HFILE hDrv, char* szFileName)
{
  char * pszLastPathDel;
  char * pColon;
  char * pszRemoteName;
  HFILE  hLocFile;
  int rc;
  ULONG ulAction, ulRemoteHandle, ulBytesToSend,ulActual;
  LNK_FILE_NAME FileName;
  FILESTATUS3  fsts3ConfigInfo = {{0}};       /* Buffer for file information */
  ULONG        ulBufSize     = sizeof(FILESTATUS3);  /* Size of above buffer */

  pColon = strrchr(szFileName,':');
  pszLastPathDel = strrchr(szFileName,'\\');
  if(pszLastPathDel>=pColon)
    if(pszLastPathDel==NULL)
      pszRemoteName = szFileName;
    else
      pszRemoteName = pszLastPathDel+1;
  else
    pszRemoteName = pColon;

  FileName.Header.ulMagic  = PLUSMAGIC;
  FileName.Header.ulTag    = LNK_TAG_FILE_NAME;
  FileName.Header.ulLength = sizeof(FileName)-sizeof(FileName.Header);
  memset(&FileName.szFileName[0],0,sizeof(FileName.szFileName) );
  strcpy((PSZ)&FileName.szFileName[0],pszRemoteName);

  rc = DosOpen( szFileName,
                &hLocFile,
                &ulAction,
                0,
                FILE_READONLY,
                OPEN_ACTION_FAIL_IF_NEW|
                OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_ACCESS_READONLY |
                OPEN_FLAGS_NOINHERIT |
                OPEN_SHARE_DENYNONE,
                0 );
  if(rc)
  {
    printf("Error %d opening source file %s",rc,szFileName);
    return;
  }

  rc = DosQueryFileInfo(hLocFile,
                        FIL_STANDARD,
                        &fsts3ConfigInfo,
                        ulBufSize);
  if (rc)
  {
    DosClose(hLocFile);
    printf("DosQueryFileInfo error: return code = %u\n", rc);
    return;
  }
  FileName.ulFileSize      = fsts3ConfigInfo.cbFile;
  printf("Sending Header for file %s of Size %d",FileName.szFileName,FileName.ulFileSize);
  rc = DosWrite(hDrv,&FileName, sizeof(FileName), &ulActual);
  if(rc)
  {
    printf("Error %d sending fileheader\r\n",rc);
    DosClose(hLocFile);
    return;
  }
  rc = ulGetStatus(hDrv);
  if(rc)
  {
    printf("Error %d getting status\r\n",rc);
    DosClose(hLocFile);
    return;
  }
  rc = ReadAccept(hDrv,&ulRemoteHandle);
  if(rc)
  {
    printf("Error %d getting RemoteHandle\r\n",rc);
    DosClose(hLocFile);
    return;
  }
  ulBytesToSend = FileName.ulFileSize;
  while(!rc && ulBytesToSend)
  {
    ULONG ulNumBytes = ulBytesToSend>64*1024?64*1024:ulBytesToSend;
    rc = SendFileBlock(hDrv, hLocFile, ulRemoteHandle, ulNumBytes);
    if(!rc)
      ulBytesToSend -= ulNumBytes;
  }
  if(rc)
  {
    printf("Error %d during send file\r\n",rc);
  }
}

void main(int argc, char *argv[])
{
  int rc,i;
  ULONG ulAction, ulParmLen, ulDataLen, ulUSBHandle, *pData;
  AQUIREDEV  USBDevice;
  SETUPPACKET SPConfigure;
  UCHAR ucBuffer[640];

  if(argc >2)
  {
    printf("Usage:\r\n-To Receive %s\r\n-To Send %s filename",argv[0],argv[0]);
    return;
  }
  if(argc==2)
  {
    char ch;
    printf("Press S to send File %s to the other PC\r\nPress X to Exit\r\n", argv[1]);
    do
    {
      ch = toupper(_getch());
    }
    while(ch !='S' && ch!= 'X');
    if(ch=='X')
      return;
  }

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
  if(rc)
    printf("Open returned %d\r\n",rc);
  else
    printf("Driver opened\r\n");
  ulDataLen = sizeof(ulUSBHandle);
  ulUSBHandle = 0;
  rc = DosDevIOCtl( hDriver, 0xA0, 0x31, // Aquire Device
                    NULL,0,0,
                    (PVOID)&ulUSBHandle, ulDataLen, &ulDataLen);
  if(rc)
    printf("Error getting number of USB devices\r\n");
  else
    printf("%d USB Devices\r\n",ulUSBHandle);

  if(!rc)
  {
    USBDevice.usVendorID  = 0x067B;
    USBDevice.usDeviceID  = 0x0000;
    USBDevice.usBCDDevice = 0xFFFF; //0x0007;
    USBDevice.usDeviceNumber = 0; // Get the usDeviceNumber device in the system fi. if 2 aquire the 2nd device
    ulParmLen = sizeof(USBDevice);
    ulDataLen = sizeof(ulUSBHandle);
    ulUSBHandle = 0xdeadbeef;
    rc = DosDevIOCtl( hDriver, 0xA0, 0x33, // Aquire Device
                      (PVOID)&USBDevice, ulParmLen, &ulParmLen,
                      (PVOID)&ulUSBHandle, ulDataLen, &ulDataLen);
    if(rc)
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

    printf("Set Configuration ... ");
    rc = sendCtrlReq( ulUSBHandle, &SPConfigure, NULL);
    if(rc)
      printf("failed rc=%d\r\n",rc);
    else
      printf("Ok!\r\n");
    memset(ucData,0,sizeof(ucData));
    SPConfigure.bRequestType = 0x80; // standard
    SPConfigure.bRequest     = 0x08; // Get config
    SPConfigure.wValue       = 0;
    SPConfigure.wIndex       = 0;
    SPConfigure.wLength      = 1;
    SPConfigure.ulTimeout    = 0;
    printf("Get Configuration ... ");
    rc = sendCtrlReq( ulUSBHandle, &SPConfigure, ucData);
    if(rc)
      printf("failed rc=%d\r\n",rc);
    else
      printf("Ok!\r\n");
    SPConfigure.bRequestType = 0x41; // Vendor
    SPConfigure.bRequest     = 0x01; // SetQuickLink Feature;
    SPConfigure.wValue       = 1; // Peer exits
    SPConfigure.wIndex       = 0;
    SPConfigure.wLength      = 0;
    SPConfigure.ulTimeout    = 0;
    printf("SetQuickLinkFeature ... ");
    rc = sendCtrlReq( ulUSBHandle, &SPConfigure, NULL);
    if(rc)
      printf("failed rc=%d\r\n",rc);
    else
      printf("Ok!\r\n");

    if(argc==2)
    {
      SendFile(hDriver,argv[1]);
    }
    else
    {
      RecvFile(hDriver);
    }

    ulParmLen = sizeof(ulUSBHandle);
    ulDataLen = 0;
    rc = DosDevIOCtl( hDriver, 0xA0, 0x34, // Release Device
                      (PVOID)&ulUSBHandle, ulParmLen, &ulParmLen,
                      NULL, ulDataLen, &ulDataLen);
  }

  DosClose(hDriver);
}
