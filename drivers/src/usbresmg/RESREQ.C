/* $ID$ */

/* Copyright 2001,2002 Markus Montkowski                                      */
/* This code is released under the Aladdin Free Public License                */
/*                    (Version 9, September 18, 2000)                         */
/* See License file in root of CVS for details                                */

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME: RESREQ.C                                               */
/*                                                                            */
/*   DESCRIPTIVE NAME: USB device Requests                                    */
/*                                                                            */
/*   FUNCTION: Contains standard USB device requests                          */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS: GetStringD                                                 */
/*                 GetDeviceID                                                */
/*                 GetPortStatus                                              */
/*                 SoftReset                                                  */
/*                 WriteData                                                  */
/*                 WriteByte                                                  */
/*                 ReadData                                                   */
/*                 CancelRequests                                             */
/*                 BufferAddr                                                 */
/*                                                                            */
/*   EXTERNAL REFERENCES: setmem                                              */
/*                        GetDS                                               */
/*                        USBCallIDC                                          */
/*                        GetString                                           */
/*                        AttachCompleted                                     */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#include "res.h"

#if 0
/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: WriteData                                         */
/*                                                                    */
/* DESCRIPTIVE NAME: Write the Data                                   */
/*                                                                    */
/* FUNCTION: This function requests USB driver to write the data      */
/*           to the USB device.                                       */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: WriteData                                             */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: usDevIndex = Device index                                   */
/*                                                                    */
/* EXIT-NORMAL: n/a                                                   */
/*                                                                    */
/* EXIT-ERROR: n/a                                                    */
/*                                                                    */
/* EFFECTS: None                                                      */
/*                                                                    */
/* INTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:         setmem                                        */
/*                      USBCallIDC                                    */
/*                      GetDS                                         */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void WriteData (USHORT usDevIndex)
{
  USBRB          rb;   // USB I/O Request Block
  RP_GENIOCTL    rp;   // IOCTL Request Packet to USBD

  if(!gUSBDevices[usDevIndex].pDeviceInfo)  // 16/12/1999 MB
    return;

  gUSBDevices[usDevIndex].wFlags ^= WRITE_DATA_TOGGLE;

  setmem ((PSZ)&rb, 0, sizeof(rb));
  rb.controllerId  = gUSBDevices[usDevIndex].pDeviceInfo->ctrlID;
  rb.deviceAddress = gUSBDevices[usDevIndex].pDeviceInfo->deviceAddress;
  rb.endPointId    = gUSBDevices[usDevIndex].ucEPBulkWrite;
  rb.flags         = USRB_FLAGS_DET_BULK | USRB_FLAGS_TTYPE_OUT;// | USRB_FLAGS_ALT_INTF;

  if (gUSBDevices[usDevIndex].wFlags & WRITE_DATA_TOGGLE)
    rb.flags |= USRB_FLAGS_DET_DTGGLEON;
  rb.buffer1 = &gUSBDevices[usDevIndex].pWBuffer[gUSBDevices[usDevIndex].wWCount];
  rb.buffer1Length = (gUSBDevices[usDevIndex].wBulkWriteMax <= gUSBDevices[usDevIndex].wWReqCount - gUSBDevices[usDevIndex].wWCount)?
                     gUSBDevices[usDevIndex].wBulkWriteMax :  gUSBDevices[usDevIndex].wWReqCount - gUSBDevices[usDevIndex].wWCount;
  rb.buffer2       = 0;
  rb.buffer2Length = 0;
  rb.serviceTime   = USB_DEFAULT_SRV_INTV;
  rb.maxPacketSize = USB_DEFAULT_PKT_SIZE;
  rb.usbIDC        = (PUSBIDCEntry)IDCEntry;
  rb.usbDS         = GetDS();
  rb.category      = USB_IDC_CATEGORY_CLASS;
  rb.requestData1  = RES_IRQ_WRITE_DATA;
  rb.requestData2  = usDevIndex;
  rb.requestData3  = 0;
  rb.maxErrorCount = USB_MAX_ERROR_COUNT;
  rb.altInterface  = 0;//gUSBDevices[usDevIndex].altInterface;

  setmem((PSZ)&rp, 0, sizeof(rp));
  rp.rph.Cmd    = CMDGenIOCTL;
  rp.Category   = USB_IDC_CATEGORY_USBD;
  rp.Function   = USB_IDC_FUNCTION_ACCIO;
  rp.ParmPacket = (PVOID)&rb;

  #ifdef DEBUG
    dsPrint3 (DBG_DETAILED, "USBRESMGR: WriteData to prt[%d], L=%x, F=%x\r\n",
              usDevIndex, rb.buffer1Length, gUSBDevices[usDevIndex].wFlags);
  #endif

  USBCallIDC (gpUSBDIDC, gdsUSBDIDC, (RP_GENIOCTL FAR *)&rp);
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: ReadData                                          */
/*                                                                    */
/* DESCRIPTIVE NAME: Read the Data                                    */
/*                                                                    */
/* FUNCTION: This function requests USB driver to read the data       */
/*           from the USB printer.                                    */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: ReadData                                              */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: usDevIndex = Device index                                   */
/*                                                                    */
/* EXIT-NORMAL: n/a                                                   */
/*                                                                    */
/* EXIT-ERROR: n/a                                                    */
/*                                                                    */
/* EFFECTS: None                                                      */
/*                                                                    */
/* INTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:         setmem                                        */
/*                      USBCallIDC                                    */
/*                      GetDS                                         */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void ReadData (USHORT usDevIndex)
{
  USBRB          rb;   // USB I/O Request Block
  RP_GENIOCTL    rp;   // IOCTL Request Packet to USBD

  if(!gUSBDevices[usDevIndex].pDeviceInfo)
    return;

  gUSBDevices[usDevIndex].wFlags ^= READ_DATA_TOGGLE;

  setmem ((PSZ)&rb, 0, sizeof(rb));
  rb.controllerId  = gUSBDevices[usDevIndex].pDeviceInfo->ctrlID;
  rb.deviceAddress = gUSBDevices[usDevIndex].pDeviceInfo->deviceAddress;
  rb.endPointId    = gUSBDevices[usDevIndex].ucEPBulkRead;
  rb.flags         = USRB_FLAGS_DET_BULK | USRB_FLAGS_TTYPE_IN;// | USRB_FLAGS_ALT_INTF;

  if (gUSBDevices[usDevIndex].wFlags & READ_DATA_TOGGLE)
    rb.flags |= USRB_FLAGS_DET_DTGGLEON;
  rb.buffer1 = &gUSBDevices[usDevIndex].pRBuffer[gUSBDevices[usDevIndex].wRCount];
  rb.buffer1Length = (gUSBDevices[usDevIndex].wBulkReadMax <= gUSBDevices[usDevIndex].wRReqCount - gUSBDevices[usDevIndex].wRCount)?
                     gUSBDevices[usDevIndex].wBulkReadMax :  gUSBDevices[usDevIndex].wRReqCount - gUSBDevices[usDevIndex].wRCount;
  rb.buffer2       = 0;
  rb.buffer2Length = 0;
  rb.serviceTime   = USB_DEFAULT_SRV_INTV;
  rb.maxPacketSize = USB_DEFAULT_PKT_SIZE;
  rb.usbIDC        = (PUSBIDCEntry)IDCEntry;
  rb.usbDS         = GetDS();
  rb.category      = USB_IDC_CATEGORY_CLASS;
  rb.requestData1  = RES_IRQ_READ_DATA;
  rb.requestData2  = usDevIndex;
  rb.requestData3  = 0;
  rb.maxErrorCount = USB_MAX_ERROR_COUNT;
  rb.altInterface  = 0;//gUSBDevices[usDevIndex].altInterface;

  setmem((PSZ)&rp, 0, sizeof(rp));
  rp.rph.Cmd    = CMDGenIOCTL;
  rp.Category   = USB_IDC_CATEGORY_USBD;
  rp.Function   = USB_IDC_FUNCTION_ACCIO;
  rp.ParmPacket = (PVOID)&rb;

  #ifdef DEBUG
    dsPrint3 (DBG_DETAILED, "USBRESMGR: ReadData from prt[%d], L=%x, F=%x\r\n",
              usDevIndex, rb.buffer1Length, gUSBDevices[usDevIndex].wFlags);
  #endif

  USBCallIDC (gpUSBDIDC, gdsUSBDIDC, (RP_GENIOCTL FAR *)&rp);
}
#endif 

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: IsoReq                                            */
/*                                                                    */
/* DESCRIPTIVE NAME: Read the Data                                    */
/*                                                                    */
/* FUNCTION: This function requests USB driver to read the data       */
/*           from the device     .                                    */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: ReadData                                              */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: usDevIndex = Device index                                   */
/*                                                                    */
/* EXIT-NORMAL: n/a                                                   */
/*                                                                    */
/* EXIT-ERROR: n/a                                                    */
/*                                                                    */
/* EFFECTS: None                                                      */
/*                                                                    */
/* INTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:         setmem                                        */
/*                      USBCallIDC                                    */
/*                      GetDS                                         */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void IsoReq (USHORT usDevIndex, PISORINGBUFFER pRingbuffer)
{
  USBRB          rb;   // USB I/O Request Block
  RP_GENIOCTL    rp;   // IOCTL Request Packet to USBD

  if(!gUSBDevices[usDevIndex].pDeviceInfo)
    return;

  setmem ((PSZ)&rb, 0, sizeof(rb));
  rb.controllerId  = gUSBDevices[usDevIndex].pDeviceInfo->ctrlID;
  rb.deviceAddress = gUSBDevices[usDevIndex].pDeviceInfo->deviceAddress;
  rb.endPointId    = pRingbuffer->ucReserved[0];
  rb.flags         = USRB_FLAGS_DET_ISOHR;
  if (pRingbuffer->ucReserved[1] )
    rb.flags |= USRB_FLAGS_ALT_INTF;
                     
  if (pRingbuffer->ucReserved[0] & DEV_ENDPT_DIRMASK)
    rb.flags |= USRB_FLAGS_TTYPE_IN ;
  rb.buffer1 = &pRingbuffer->ucBuffer[pRingbuffer->usPosWrite];
  rb.buffer1Length = pRingbuffer->usWindowSize;
  rb.buffer2       = 0;
  rb.buffer2Length = 0;
  rb.serviceTime   = USB_DEFAULT_SRV_INTV;
  rb.maxPacketSize = USB_DEFAULT_PKT_SIZE;
  rb.usbIDC        = (PUSBIDCEntry)IDCEntry;
  rb.usbDS         = GetDS();
  rb.category      = USB_IDC_CATEGORY_CLASS;
  rb.requestData1  = RES_IRQ_READ_DATA;
  rb.requestData2  = usDevIndex;
  rb.requestData3  = pRingbuffer;
  rb.maxErrorCount = USB_MAX_ERROR_COUNT;
  rb.altInterface  = pRingbuffer->ucReserved[1];
  
  setmem((PSZ)&rp, 0, sizeof(rp));
  rp.rph.Cmd    = CMDGenIOCTL;
  rp.Category   = USB_IDC_CATEGORY_USBD;
  rp.Function   = USB_IDC_FUNCTION_ACCIO;
  rp.ParmPacket = (PVOID)&rb;

  #ifdef DEBUG
    dsPrint3 (DBG_DETAILED, "USBRESMGR: ReadData from prt[%d], L=%x, F=%x\r\n",
              usDevIndex, rb.buffer1Length, gUSBDevices[usDevIndex].wFlags);
  #endif

  USBCallIDC (gpUSBDIDC, gdsUSBDIDC, (RP_GENIOCTL FAR *)&rp);
}


/********************** START OF SPECIFICATIONS ***********************/
/*                                                                   */
/* SUBROUTINE NAME: CancelRequests                                    */
/*                                                                    */
/* DESCRIPTIVE NAME: Cancel Requests                                  */
/*                                                                    */
/* FUNCTION: This function is used to cancel all queued I/O requests  */
/*           for the specified USB device and endpoint.               */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT: CancelRequests                                        */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: usDevIndex = Device Index                                   */
/*        endPoint = endpoint number                                  */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR: N/A                                                    */
/*                                                                    */
/* EFFECTS:                                                           */
/*                                                                    */
/* INTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:         setmem                                        */
/*                      USBCallIDC                                    */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

void CancelRequests (PDEVICELIST pDevice, UCHAR ucEndPoint)
{
  USBCancel   rb;   // USB Cancel Request Block
  RP_GENIOCTL rp;   // IOCtl Request Packet to USBD

#ifdef DEBUG
   dsPrint1 (DBG_DETAILED, "USBRESMGR: CancelRequest ep=%x\r\n", ucEndPoint);
#endif

   if (pDevice->pDeviceInfo)
   {
    rb.controllerId = pDevice->pDeviceInfo->ctrlID;
    rb.deviceAddress = pDevice->pDeviceInfo->deviceAddress;
    rb.endPointId    = ucEndPoint;
    setmem((PSZ)&rp, 0, sizeof(rp));
    rp.rph.Cmd    = CMDGenIOCTL;
    rp.Category   = USB_IDC_CATEGORY_USBD;
    rp.Function   = USB_IDC_FUNCTION_CANCEL;
    rp.ParmPacket = (PVOID)&rb;

    USBCallIDC (gpUSBDIDC, gdsUSBDIDC, (PRP_GENIOCTL)&rp);
  }
}

UCHAR IntGetPipeAddr( UCHAR FAR *configurationData, UCHAR bNumConfigurations,
                      UCHAR configurationValue, UCHAR interface,
                      UCHAR altInterface, UCHAR type,
                      UCHAR typeMask, UCHAR attributes, USHORT FAR* wLength)
{
   DeviceEndpoint FAR      *endPointDesc;
   DeviceConfiguration FAR *devConf;
   UCHAR                   configIndex;
   UCHAR FAR               *currBytePtr, FAR *lastBytePtr;
   DeviceDescHead FAR      *descHead;
   DeviceInterface FAR     *interfaceDesc;
   UCHAR                   currInterface=0, currAlternate=0;
   UCHAR                   pipe=0;

  #ifdef DEBUG
    dsPrint3 ( DBG_DETAILED,
               "GetPipeAddr: GetPipe from interface %x altInterface %x type %x",
               interface, altInterface, type
                );
    dsPrint2 ( DBG_DETAILED,
               "typemask %x attributes %x\r\n",
               typeMask, attributes
                );
  #endif


   devConf=(DeviceConfiguration FAR *)configurationData;
   for (configIndex=0; configIndex<bNumConfigurations;
       configIndex++)
   {
      currBytePtr=(UCHAR FAR *)devConf;
      lastBytePtr=currBytePtr+devConf->wTotalLength;
      descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength);
      #ifdef DEBUG
        dsPrint2 ( DBG_DETAILED,
                   "GetPipeAddr: Check configuration %d Value is %d\r\n",
                   configIndex,
                   devConf->bConfigurationValue );
      #endif
      if (devConf->bConfigurationValue==configurationValue)
      {
         for (descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength); descHead && !pipe;
             descHead=GetNextDescriptor( descHead, lastBytePtr))
         {
            #ifdef DEBUG
              dsPrint3 ( DBG_DETAILED,
                         "GetPipeAddr: Descriptor Type %x\r\nInterface is %x alt %x\r\n",
                         descHead->bDescriptorType,
                         currInterface,
                         currAlternate
                         );
            #endif
            switch (descHead->bDescriptorType)
            {
            case DESC_INTERFACE:
               interfaceDesc=(DeviceInterface FAR *)descHead;
               currInterface=interfaceDesc->bInterfaceNumber;
               currAlternate=interfaceDesc->bAlternateSetting;
               break;
            case DESC_ENDPOINT:
               if (currInterface!=interface || altInterface!=currAlternate)
                  break;
               endPointDesc=(DeviceEndpoint FAR *)descHead;
                #ifdef DEBUG
                  dsPrint2 ( DBG_DETAILED,
                             "GetPipeAddr: Endpoint %x %x\r\n",
                             endPointDesc->bmAttributes,
                             endPointDesc->bEndpointAddress
                             );
                #endif
               if (((UCHAR)(endPointDesc->bmAttributes&DEV_ENDPT_ATTRMASK)==attributes) &&
                     ((UCHAR)(endPointDesc->bEndpointAddress & typeMask)==type) )
               {
                  pipe=(UCHAR)(endPointDesc->bEndpointAddress&DEV_ENDPT_ADDRMASK);
                  *wLength = (USHORT)(endPointDesc->wMaxPacketSize);
               }
               break;
            default:
               break;
            }
         }
         break;
      }
      devConf=(DeviceConfiguration FAR *)lastBytePtr; // point to next configuration block
   }

   return (pipe);
}

