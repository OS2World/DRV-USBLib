/***********************************************************
/       Title: usbserial.c
/       Author: David Corcoran
/      Purpose: Abstracts usb API to serial like calls
************************************************************/

#include <stdio.h>
#include <pcscdefines.h>
#include <usbserial.h>
#include <stdlib.h>
#include <usblinux.h>

// #define USBDEBUG	1
#define USBWRITE_PIPE   0x06
#define USBREAD_PIPE    0x85
#define USBMAX_READERS  4

/* Change the following to uniquely match your reader. */
enum {
    kMyVendorID			= 0x08E6,   /* change to match your reader */
    kMyProductID		= 0x0430,   /* change to match your reader */
};

static int usbDevice = 0;
 
RESPONSECODE OpenUSB( DWORD lun )
{
  int rv;

  usbDevice = open_linux_usb_dev( kMyVendorID, kMyProductID, lun );

  if ( usbDevice <= 0 ) {
    return STATUS_UNSUCCESSFUL;
  }

  return STATUS_SUCCESS;  
}

RESPONSECODE WriteUSB( DWORD lun, DWORD length, unsigned char *buffer )
{
  int rv, len;
  int i;
  
#ifdef USBDEBUG
  printf("-> ");
  for (i=0; i < length; i++ ) {
    printf("%x ", buffer[i]);
  } printf("\n");
#endif  

  len = length;
  
  rv = bulk_linux_usb_dev( usbDevice, USBWRITE_PIPE, buffer, 
			   &len, 100000 );
  
  
  if ( rv < 0 ) {
    return STATUS_UNSUCCESSFUL;
  }

  return STATUS_SUCCESS;
}

RESPONSECODE ReadUSB( DWORD lun, DWORD *length, unsigned char *buffer )
{
  int rv, len, i;

  len = 256;

  rv = bulk_linux_usb_dev( usbDevice, USBREAD_PIPE, buffer, 
			   &len, 10000 );
  *length = len;

  if ( rv < 0 ) {
    return STATUS_UNSUCCESSFUL;
  }

#ifdef USBDEBUG
  printf("<- ");
  for (i=0; i < len; i++ ) {
    printf("%x ", buffer[i]);
  } printf("\n");
#endif  

  return STATUS_SUCCESS;
}

RESPONSECODE CloseUSB( DWORD lun )
{
    close_linux_usb_dev( usbDevice );
    return STATUS_SUCCESS;
}
