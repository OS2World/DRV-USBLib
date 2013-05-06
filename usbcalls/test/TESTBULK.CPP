#define INCL_DOS
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\usbcalls.h"

#define BULK_IOCTL 1

int SenseDevice(USBHANDLE hUSBDev)
{
  int rc;
  ULONG i,ulActual;
  UCHAR ucCommandSense[8] = { 0x00, 0x01, 0x00, 0x00,
                              0x00, 0xa0, 0xec, 0x01};
  UCHAR ucData[512];
#if BULK_IOCTL
  rc = UsbBulkWrite( hUSBDev, 0x01, 0x00, 8, ucCommandSense, 0);
  if(rc)
  {
    printf("Faild to write sense command rc=%d\r\n",rc);
  }
  else
  {
    memset(ucData,sizeof(ucData),0);
    rc = UsbBulkRead(hUSBDev, 0x82, 0x00, 512, ucData, 0);
    ulActual = 512;
    if(rc)
      printf("Faild to Read Sense Data rc=%d\r\n",rc);
  }
#else
  rc = DosWrite(hDrv, ucCommandSense, 8, &ulActual);
  printf("SenseDevice Command written rc=%d, bytes written %d\r\n",rc, ulActual);
  rc = DosRead(hDrv, ucData, 512, &ulActual);
  printf("SenseDevice Read data rc=%d, bytes read %d\r\n",rc, ulActual);
#endif
  //if(rc==0)
  {
    printf("Data : ");
    for(i=0;i<ulActual;i++)
    {
      printf("%02X ",ucData[i]);
      if(((i+1)%16)==0)
       printf("\r\n       ");
    }
    printf("\r\n");
  }
  return rc;
}

int ReadSector(USBHANDLE hUSBDev, ULONG ulSector)
{
  int rc;
  ULONG i,ulActual;
  UCHAR ucCommandRead[8] = { 0x00, 0x01, 0x00, 0x00,
                             0x00, 0xe0, 0x20, 0x01};
  UCHAR ucData[512];

#if BULK_IOCTL
  rc = UsbBulkWrite( hUSBDev, 0x01, 0x00, 8, ucCommandRead, 0);
  if(rc)
  {
    printf("Faild to write command rc=%d\r\n",rc);
  }
  else
  {
    rc = UsbBulkRead(hUSBDev, 0x82, 0x00, 512, ucData, 0);
    ulActual = 512;
    if(rc)
      printf("Faild to Read Data rc=%d\r\n",rc);
  }
#else
  rc = DosWrite(hDrv,ucCommandRead,8, &ulActual);
  printf("ReadSector Command written rc=%d, bytes written %d\r\n",rc, ulActual);
  rc = DosRead(hDrv, ucData, 512, &ulActual);
  printf("ReadSector Read data rc=%d, bytes read %d\r\n",rc, ulActual);
#endif
  if(rc==0)
  {
    printf("Data : ");
    for(i=0;i<ulActual;i++)
    {
      printf("%02X ",ucData[i]);
      if(((i+1)%16)==0)
       printf("\r\n       ");
    }
    printf("\r\n");
  }
  return rc;
}

#define VENDOR_DATAFAB 0x07C4
#define PRODUCT_MD2    0xA002

void main()
{
  int rc,i;
  ULONG ulNumDevices;
  USBHANDLE hUSBDevice;
  UCHAR ucData[20];

  rc = UsbQueryNumberDevices(&ulNumDevices);
  printf("Num devices = %d (rc=%d)\r\n",ulNumDevices,rc);

  rc = UsbOpen( &hUSBDevice,
                VENDOR_DATAFAB,
                PRODUCT_MD2,
                USB_ANY_PRODUCTVERSION,
                USB_OPEN_FIRST_UNUSED);

  if(!rc && hUSBDevice!=0)
  {

    rc = UsbSetDeviceConfiguration(hUSBDevice, 1);
    printf("DataFab MD-2 Opened\r\n SetConfiguration 1 %s\r\n",(rc==0)?"SUCCESSFUL":"FAILED");
    memset(ucData,0,sizeof(ucData));
    if(!rc)
    {
      printf("Checking Configuration Current Configuration ");
      rc = UsbGetDeviceConfiguration(hUSBDevice, ucData);
      if(rc)
        printf("FAILED");
      else
      {
        printf("SUCCESSFUL\r\n  Configuration is %d\r\nTrying to Sense Device\r\n",ucData[0]);
        rc = SenseDevice(hUSBDevice);
        if(!rc)
        {
          printf("Trying to Read Sector 0\r\n");
          rc = ReadSector(hUSBDevice,0);
        }
      }
    }
    UsbClose(hUSBDevice);
  }
}
