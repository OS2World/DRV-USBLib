/* $ID$ */

/* Copyright 2001,2002 Markus Montkowski                                      */
/* This code is released under the Aladdin Free Public License                */
/*                    (Version 9, September 18, 2000)                         */
/* See License file in root of CVS for details                                */

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME: RESTYPE.H                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME: USB Resource Manager Device Driver typedefs            */
/*                                                                            */
/*   FUNCTION: This module is the USB Resource Manager Device Driver          */
/*             typedef include file.                                          */
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
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          00/01/14  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#include <bsedev.h>

#define  MAX_DEVICES    32          // Max number of Devices
#define  MAX_OFHS    64             // Max number of Opened File Handles
#define  MAX_SEMS    64             // Max number of registered Semaphores

/*
   Initialization time message IDs
*/
#define  INIT_MESSAGE_LOADED     0
#define  INIT_MESSAGE_NO_USBD    1
#define  INIT_MESSAGE_UNKNOWNKWD 2
#define  INIT_MESSAGE_INVNUMERIC 3

#define MAX_INIT_MESSAGE_COUNT   1
#define MSG_REPLACEMENT_STRING   1178

/*
         IOCtl Codes
*/
#define  IOC_RES                 0xA0  // USB Resource device control (IOCTL.H)
#define  IOCTLF_NUMDEVICE        0x31  // Get Number of pluged in Devices
#define  IOCTLF_GETINFO          0x32  // Get Info About a device
#define  IOCTLF_AQUIREDEVICE     0x33
#define  IOCTLF_RELEASEDEVICE    0x34
#define  IOCTLF_GETSTRING        0x35
#define  IOCTLF_SENDCONTROLURB   0x36
#define  IOCTLF_SENDBULKURB      0x37  // Send
#define  IOCTLF_START_IRQ_PROC   0x38  // Start IRQ polling in a buffer
#define  IOCTLF_GETDEVINFO       0x39  // Get information about device
#define  IOCTLF_STOP_IRQ_PROC    0x3A  // Stop IRQ Polling
#define  IOCTLF_START_ISO_PROC   0x3B  // Start ISO buffering in a Ringbuffer
#define  IOCTLF_STOP_ISO_PROC    0x3C  // Stop ISO buffering
#define  IOCTLF_CANCEL_IORB      0x3D  // Cancel an IO
#define  IOCTLF_SELECT_BULKPIPE  0x3E  // Select which Bulk endpoints can be used via Read/Write
#define  IOCTLF_REG_STATUSSEM    0x41  // Register Semaphore for general Statuschange
#define  IOCTLF_DEREG_STATUSSEM  0x42  // Deregister Semaphore
#define  IOCTLF_REG_DEVICESEM    0x43  // Register Semaphore for a vendor&deviceID
#define  IOCTLF_DEREG_DEVICESEM  0x44  // Deregister Semaphore

#define  MIN_RES_IOCTLF    IOCTLF_NUMDEVICE
#define  MAX_RES_IOCTLF    IOCTLF_DEREG_DEVICESEM

#define  SELECTOR_MASK  0xFFF8

/*
    IRQ Codes
*/
#define RES_IRQ_STRINGLEN   0x00
#define RES_IRQ_STRING      0x01
#define RES_IRQ_CONTROL     0x02
#define RES_IRQ_RWBULK      0x03
#define RES_IRQ_RWIRQ       0x04
#define RES_IRQ_GETDESC     0x05
#define RES_IRQ_WRITE_DATA  0x07
#define RES_IRQ_READ_DATA   0x08
#define RES_IRQ_PROCESSED   0xFF

typedef struct _SDATA
{                                   // String DATA array element
   USHORT   stringLength;
   ULONG    stringAddr;
} SDATA, *NPSDATA, FAR *PSDATA;

typedef struct
{
  ULONG hEventSem;         // Posted when buffer was changed by the driver
  ULONG hSemAccess;        // Syncronise access to the Pos values
  USHORT usPosWrite;
  USHORT usPosRead;
  USHORT usBufSize;
  USHORT usWindowSize;     // Set by the driver to the size of a single packet
  USHORT usInterval;       // Number of Packets read/write before posting event
  UCHAR  ucReserved[2];
  UCHAR  ucBuffer[16*1000];
}ISORINGBUFFER, * NPIOSRINGBUFFER, FAR * PISORINGBUFFER;

/*
         Flags
*/
#define  WRITE_DATA_INPROGRESS      1
#define  WRITE_DATA_TOGGLE          2
#define  FLUSH_OUT_INPROGRESS       4
#define  WRITE_BYTE_INPROGRESS      8
#define  STOP_TRANSMIT             16
#define  WRITE_DATA_ERROR          32

#define  READ_DATA_INPROGRESS     256
#define  READ_DATA_TOGGLE         512
#define  FLUSH_IN_INPROGRESS     1024
#define  READ_DATA_ERROR         2048

/*
         Request Packet Index
*/
#define  FIRST          0
#define  SECOND         1
#define  LAST           1
#define  CURRENT        2

#define  NUM_POINTERS   (CURRENT+1)

/*
         GDT Selector   Index
*/
#define  READ_SEL          0
#define  WRITE_SEL         1

#define  MAX_GDT_SEL       2

#define  DEV_EP_NOTASSIGNED 0xFF

typedef struct
{
  WORD   wStatus[15];
  USHORT usFlags[15];
}IOINFO;

typedef struct                     // Attached devices array element
{
   UCHAR           bAttached;       // TRUE or FALSE
   DeviceInfo FAR *pDeviceInfo;     // controller, USB address, config, device descriptor
   USHORT          usHandleID;
   USHORT          usSFN;
   UCHAR           ucAltInterface;
   USHORT          wFlags;
   USHORT          wToggle[2];    // Toggle bits for endpoints 0 In, 1 Out
   IOINFO          IoInfo[2];
   PISORINGBUFFER  pIsobufRead;
   PISORINGBUFFER  pIsobufWrite;
   SetupPacket     setupPack;
}DEVICELIST, FAR* PDEVICELIST;

typedef struct _OFH
{                                   // Opened File Handle array element
   UCHAR    configured;             // TRUE or FALSE
   UCHAR    prtIndex;
   USHORT   fileHandle;             // System File Number
   UCHAR    shareMode;
   UCHAR    altInterface;
   USHORT   idVendor;
   USHORT   idProduct;
   USHORT   bcdDevice;              // device release number in binary-coded decimal
} OFH, *NPOFH, FAR * POFH;

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
} STATUSEVENTSET, *NPSTATUSEVENTSET, FAR * PSTATUSEVENTSET;


typedef struct{
  ULONG  ulSize;
  ULONG  ulCaps;
  ULONG  ulSemDeviceAdd;
  ULONG  ulSemDeviceRemove;
  USHORT usVendorID;
  USHORT usProductID;
  USHORT usBCDDevice;
  USHORT usReserved;
} DEVEVENTSET, *NPDEVEVENTSET, FAR * PDEVEVENTSET;

typedef struct
{
  ULONG hSemaphoreAdd;
  ULONG hSemaphoreRemove;
  USHORT usVendorID;
  USHORT usProductID;
  USHORT usBCDDevice;
  USHORT usReserved;
} DEVSEMAPHOREENTRY, *NPDEVSEMAPHOREENTRY, FAR * PDEVSEMAPHOREENTRY;

typedef struct
{
  USHORT usVendorID;
  USHORT usProductID;
  USHORT usBCDDevice;
  USHORT usDeviceNumber; // Get the usDeviceNumber device in the system fi. if 2 aquire the 2nd device
                         // 0 means first not aquired device.
}AQUIREDEV, *NPAQUIREDEV, FAR *PAQUIREDEV;

typedef struct
{
  ULONG  ulDevHandle;
  USHORT usLangID;
  UCHAR  ucStringID;
  UCHAR  ucReserved;
}GETSTRINGPARM, *NGETSTRINGPARM, FAR *PGETSTRINGPARM;

typedef struct
{
  UCHAR              ctrlID;              // (00) controller ID
  UCHAR              deviceAddress;       // (01) USB device address
  UCHAR              bConfigurationValue; // (02) USB device configuration value
  UCHAR              bInterfaceNumber;    // (03) 0 based index in interface array for this item
  UCHAR              lowSpeedDevice;      // (04) 0 for full speed device, nonzero - low speed device
  UCHAR              portNum;             // (05) port number to which device is attached
  USHORT             parentHubIndex;      // (06) index in hub table to parent hub, -1 for root hub device
  HDEVICE            rmDevHandle;         // (08) Resource Manager device handle
}GETDEVINFODATA, *NGETDEVINFO, FAR* PGETDEVINFO;

typedef struct
{
  ULONG  ulDevHandle;
  UCHAR  bRequestType;
  UCHAR  bRequest;
  USHORT wValue;
  USHORT wIndex;
  USHORT wLength;
  ULONG  ulTimeout; /* in milliseconds */
}LIBUSB_CTRL_REQ, *NPLIBUSB_CTRL_REQ, FAR *PLIBUSB_CTRL_REQ;

typedef struct
{
  ULONG  ulDevHandle;
  ULONG  ulEventDone;
  UCHAR  ucEndpoint;
  UCHAR  ucInterface;
  USHORT usDataProcessed;
  USHORT usDataRemain;
  USHORT usStatus;
}LIBUSB_BULK_REQ, *NPLIBUSB_BULK_REQ, FAR *PLIBUSB_BULK_REQ;

#define USB_IORB_DONE     0
#define USB_IORB_WORKING  1
#define USB_IORB_CANCELED 2
#define USB_IORB_DEVICE_REMOVED 3
#define USB_IORB_STALLED 4
#define USB_IORB_FAILED  5

#define USB_MAX_IORBS 64

#define SEL_BULK 0
#define SEL_IRQ  1
#define SEL_ISO  2

typedef struct
{
  ULONG ulEventDone;
  UCHAR FAR *pData;
  ULONG pPhysData;
  PLIBUSB_BULK_REQ pParam;
  USHORT usDataProcessed;
  USHORT usDataRemain;
  ULONG pPhys;
  ULONG ulLockData;
  ULONG ulLockParam;
  ULONG ulID;
  PDEVICELIST pDevice;
  DeviceEndpoint FAR * pEndpoint;
}USB_IORB, FAR *PUSB_IORB;

#define DEV_RES_ISO_BUF_RING  0x01
#define DEV_RES_ISO_BUF_BLOCK 0x02

typedef struct
{
  ULONG  ulDevHandle;
  ULONG  ulIrqEvent;
  UCHAR  ucEndpoint;
  UCHAR  ucInterface;
}LIBUSB_IRQ_START, *NPLIBUSB_IRQ_START, FAR *PLIBUSB_IRQ_START;

typedef struct
{
  UCHAR  ucBufferCurrent[64]; // Max 64 Byte per Irq Req.
  UCHAR  ucBufferLast[64];
  USHORT usStatus;
  USHORT usRes;
  ULONG  ulEventChanged;
}LIBUSB_IRQ_DATA, *NPLIBUSB_IRQ_DATA, FAR *PLIBUSB_IRQ_DATA;

typedef struct
{
  ULONG ulDevHandle;
  UCHAR ucEndpoint;
  UCHAR ucInterface;
}LIBUSB_ISO_START, *NPLIBUSB_ISO_START, FAR *PLIBUSB_ISO_START;

typedef LIBUSB_ISO_START LIBUSB_ISO_STOP, * NPLIBUSB_ISO_STOP, FAR *PLIBUSB_ISO_STOP;
typedef LIBUSB_ISO_START LIBUSB_IRQ_STOP, * NPLIBUSB_IRQ_STOP, FAR *PLIBUSB_IRQ_STOP;

typedef struct
{
  ULONG ulDevHandle;
  UCHAR ucEndpoint;
  UCHAR ucInterface;
  UCHAR ucFlag;
}LIBUSB_BULK_SELECT, * NPLIBUSB_BULK_SELECT, FAR *PLIBUSB_BULK_SELECT;

// big/small endian conversion macro
#define  MAKEUSINTEL(w)                MAKEUSHORT(HIBYTE(w),LOBYTE(w))

