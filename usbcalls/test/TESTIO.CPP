#define INCL_DOS
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\usbcalls.h"

int ReadIKeyData( USBHANDLE hUSBDevice,
                  USHORT usNumBytes,
                  UCHAR *pData)

{
  return UsbCtrlMessage( hUSBDevice,
                         0xC1, 1,0,0,
                         usNumBytes, pData,
                         0);

}

int BlinkLED(USBHANDLE hUSBDevice)
{
  UCHAR ucLEDData[2] ={0,0};
  int rc,i;

  for(i=0;i<200;i++)
  {
    rc = UsbCtrlMessage( hUSBDevice,
                         0xC1, 3,2,0,
                         0x02, ucLEDData,
                         0);
    if(rc)
      break;
    DosSleep(25);
    rc = UsbCtrlMessage( hUSBDevice,
                         0xC1, 3,1,0,
                         0x02, ucLEDData,
                         0);
    DosSleep(25);
    if(rc)
      break;
  }
  return rc;
}

int OpenIKey(USBHANDLE hUSBDevice)
{
  UCHAR ucOpenData[0x20];
  ULONG ulSerial[2];
  UCHAR ucCheckData[6];
  int rc,i;

  rc = UsbCtrlMessage( hUSBDevice,
                       0xC1, 0,0,0,
                       0x20, ucOpenData,
                       0);
  if(!rc)
   rc = UsbCtrlMessage( hUSBDevice,
                        0x41, 2,0,0,
                        0x00, NULL,
                        0);

  if(!rc)
   rc = ReadIKeyData( hUSBDevice, 8, (UCHAR*) &ulSerial[0]);

  if(!rc)
   rc = UsbCtrlMessage( hUSBDevice,
                        0xC1, 2,0,0,
                        0x06, ucCheckData,
                        0);
  if(!rc)
  {
     USHORT *pusMemSize = (USHORT*)&ucOpenData[4];
     printf("Firmware Version ... = %d.%d\r\n",
     ucOpenData[2],ucOpenData[3]);
     printf("Product Code ....... = %x\r\n",ucOpenData[1]);
     printf("Memory Size ........ = %d\r\n",(*pusMemSize)*2);
     printf("Capabilities ....... = %x\r\n",ucOpenData[6]);
     printf("Serial No .......... = %08X%08X\r\n",ulSerial[1],ulSerial[0]);
     printf("F/w Check Sum ...... = %02X%02X\r\n",ucCheckData[1],ucCheckData[0]);
  }
  return rc;
}

#define VENDOR_RAINBOW    0x04B9
#define PRODUCT_IKEY1000  0x1000

void main()
{
  APIRET rc;
  USBHANDLE hUSBDevice;
  ULONG ulNumDevices;
  UCHAR ucBuffer[640];

  rc = UsbQueryNumberDevices(&ulNumDevices);
  printf("Num devices = %d (rc=%d)\r\n",ulNumDevices,rc);

  rc = UsbOpen( &hUSBDevice,
                VENDOR_RAINBOW,
                PRODUCT_IKEY1000,
                USB_ANY_PRODUCTVERSION,
                USB_OPEN_FIRST_UNUSED);

  if(!rc && hUSBDevice!=0 &&
     0==OpenIKey(hUSBDevice))
  {

    BlinkLED(hUSBDevice);

    memset(ucBuffer,sizeof(ucBuffer),0);

    rc = UsbCtrlMessage( hUSBDevice,
                         0x41, 4,0,0,
                         0x02, ucBuffer,
                         0);
    if(!rc)
      ReadIKeyData( hUSBDevice, 0x0E, ucBuffer);

    UsbClose(hUSBDevice);
  }
  else
  {
    printf("USBOpen returned %d\r\n",rc);
  }

}
