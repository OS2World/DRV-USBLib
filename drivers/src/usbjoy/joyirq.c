/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  JOYIRQ.C                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME:  IDC routine to process legacy driver requests         */
/*                                                                            */
/*   FUNCTION:                                                                */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:                                                            */
/*             JOYirq                                                         */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          00/01/04  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#include "joy.h"

void JOYirq (RP_GENIOCTL FAR *pRP_GENIOCTL)
{
   USBRB FAR *processedRB;
   UCHAR oldCat;

   processedRB = (USBRB FAR *)pRP_GENIOCTL->ParmPacket;

   dsPrint3 (DBG_IRQFLOW, "USBJOY: IRQ = %d, joyIndex = %d Status %x \r\n",
             processedRB->requestData1, processedRB->requestData2,pRP_GENIOCTL->rph.Status);

/*
   if(USB_IDC_RC_IOFAILED == pRP_GENIOCTL->rph.Status)
   {
     dsPrint(DBG_IRQFLOW, "Status is IO-Failed change to OK\r\n");
     pRP_GENIOCTL->rph.Status = USB_IDC_RC_OK;
   }
*/
   if(pRP_GENIOCTL->rph.Status != USB_IDC_RC_OK)
   {
      dsPrint(DBG_IRQFLOW, "Status Not OK\r\n");

      if (processedRB->status & USRB_STATUS_STALLED)
      {
        dsPrint(DBG_IRQFLOW, "Status Stalled\r\n");

        if( processedRB->requestData1!=JOY_IRQ_STATUS_STALLED ) /* send clear stalled request once only */
        {
           oldCat=pRP_GENIOCTL->Category;
           JOYClearStalled(pRP_GENIOCTL);
           pRP_GENIOCTL->Category=oldCat;
        }
        return;
      }
      return;
   }

   switch (processedRB->requestData1)
   {
   case JOY_IRQ_STATUS_IDLESET:
      pRP_GENIOCTL->rph.Status=STATUS_DONE;    //always ok
      ReadInterruptPipe (pRP_GENIOCTL);
      break;
   case JOY_IRQ_STATUS_DURATION:
      break;
   case JOY_IRQ_STATUS_INTPIPE:
      InterruptDataReceived (pRP_GENIOCTL);
      ReadInterruptPipe (pRP_GENIOCTL);
      break;
   case JOY_IRQ_STATUS_SETACK:
      break;
   case JOY_IRQ_STATUS_STALLED:
      ReadInterruptPipe (pRP_GENIOCTL);
      break;
   default:;
#ifdef DEBUG
      dsPrint1 (DBG_CRITICAL, "USBJOY: IRQ = %d ???\r\n", processedRB->requestData1);
#endif
   }
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME:  GetLogValue                                      */
/*                                                                    */
/* DESCRIPTIVE NAME:  Get a Value from the the Reportbuffer           */
/*                                                                    */
/* FUNCTION:  This routine extracts an Vale from the Bitbuffer        */
/*            representing the report. It uses the Items information  */
/*            extracted during the device attachment                  */
/*                                                                    */
/* NOTES:     Maximum valuesize supported is 32bit                    */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT :  InterruptDataReceived                               */
/*    LINKAGE  :  CALL NEAR                                           */
/*                                                                    */
/* INPUT:  PRP_GENIOCTL pRP_GENIOCTL                                  */
/*                                                                    */
/* EXIT-NORMAL:  n/a                                                  */
/*                                                                    */
/* EXIT-ERROR:   n/a                                                  */
/*                                                                    */
/* EFFECTS:  sets return code in pRP_GENIOCTL->rph.Status             */
/*                                                                    */
/* INTERNAL REFERENCES:  ReadInterruptPipe                            */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:  USBCallIDC                                   */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

#pragma optimize("eglt", off)

LONG GetLogValue( USHORT joyIndex, USHORT ItemOfs)
{
  LONG rc = 0;
  USHORT usOffset, usByteOfs, StartBit, usSize;
  BYTE     *pIntData, bRem;
  usOffset = gJOY[joyIndex].Items[ItemOfs].usOffset;
  usSize   = gJOY[joyIndex].Items[ItemOfs].usReportSize;
  pIntData = (BYTE *)&gJOY[joyIndex].buffer;
  // No proper index or Value to long
  if( (FULL_WORD==usOffset) || (usSize>32) )
    return rc;

  StartBit  = usOffset %8;
  usByteOfs = usOffset /8;

  //Check if in bounds of report
  if(usByteOfs>=gJOY[joyIndex].ReportLength)
    return rc;

  if(usSize>1)
  {
    if(!StartBit)
    {
      // probably the easiest
      while(usSize>=8)
      {
        rc *= 256;
        rc += pIntData[usByteOfs++];
        usSize-=8;
      }
      if(usSize)
      {
        rc *= (2*usSize);
        bRem = pIntData[usByteOfs];
        bRem >>=(8-usSize);
        rc += bRem;
      }
    }
    else
    {
      if( (StartBit-usSize)<=0)
      {
        // All bits are in this byte
        bRem = pIntData[usByteOfs];
        bRem &= gRightMask[StartBit];
        bRem >>= (8-usSize-StartBit);
        rc = bRem;
      }
      else
      {
        bRem = pIntData[usByteOfs++];
        bRem &=gRightMask[StartBit];
        rc = bRem;
        usSize -= (8-StartBit);
        while(usSize>=8)
        {
          rc *= 256;
          rc += pIntData[usByteOfs++];
          usSize-=8;
        }
        if(usSize)
        {
          rc *= (2*usSize);
          bRem = pIntData[usByteOfs];
          bRem >>=(8-usSize);
          rc += bRem;
        }
      }
    }
  }
  else
  {
    // 1 Byte only
    bRem = pIntData[usByteOfs] & gBitMask[StartBit];
    bRem >>= (7-StartBit);
    rc = bRem;
  }
  return rc;
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME:  InterruptDataReceived                            */
/*                                                                    */
/* DESCRIPTIVE NAME:  Data received on Joystick interrupt pipe        */
/*                                                                    */
/* FUNCTION:  This routine is called when report has been received    */
/*            throught Joystick interrupt pipe. Received data is      */
/*            processed and key scan codes passed to legacy driver.   */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT :  InterruptDataReceived                               */
/*    LINKAGE  :  CALL NEAR                                           */
/*                                                                    */
/* INPUT:  PRP_GENIOCTL pRP_GENIOCTL                                  */
/*                                                                    */
/* EXIT-NORMAL:  n/a                                                  */
/*                                                                    */
/* EXIT-ERROR:   n/a                                                  */
/*                                                                    */
/* EFFECTS:  sets return code in pRP_GENIOCTL->rph.Status             */
/*                                                                    */
/* INTERNAL REFERENCES:  ReadInterruptPipe                            */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:  USBCallIDC                                   */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/


void InterruptDataReceived (RP_GENIOCTL FAR *pRP_GENIOCTL)
{
   USBRB FAR *processedRB;
   BYTE      *pIntData;
   USHORT     joyIndex, i;
   LONG       lValue;
   processedRB = (USBRB FAR *)pRP_GENIOCTL->ParmPacket;
   joyIndex = LOUSHORT (processedRB->requestData2);

   if (gDevice)
     if (joyIndex != gJoyIndex)
       return;

   pIntData = (BYTE *)&gJOY[joyIndex].buffer;

#ifdef DEBUG
{
  USHORT i;
   dsPrint1 ( DBG_IRQFLOW, "USBJOY: IntData  joyIndex = %d\r\n ReportData: ",
              joyIndex);;

   i=0;
   while( i< gJOY[joyIndex].ReportLength)
   {
     dsPrint1 (DBG_IRQFLOW, "0x%x, ",
               pIntData[i++]);
   }
   dsPrint ( DBG_IRQFLOW, "\r\n");
}
#endif

  setmem((PSZ)&gJOY[joyIndex].joyState, 0, sizeof(JOYSTATE));

  if(gJOY[joyIndex].ulCapsAxes & JOYHAS_X)
  {
    lValue = GetLogValue(joyIndex,JOYOFS_X);
    if( gJOY[joyIndex].AxeUnits[JOYOFS_X].unit)
    {
      lValue *= (gJOY[joyIndex].AxeUnits[JOYOFS_X].phyMax-
                 gJOY[joyIndex].AxeUnits[JOYOFS_X].phyMin)/
                (gJOY[joyIndex].AxeUnits[JOYOFS_X].logMax-
                 gJOY[joyIndex].AxeUnits[JOYOFS_X].logMin);

    }
    dsPrint1(DBG_DETAILED,"X is at %d\r\n",lValue);

    gJOY[joyIndex].joyState.lX = lValue;
  }

  if(gJOY[joyIndex].ulCapsAxes & JOYHAS_Y)
  {
    lValue = GetLogValue(joyIndex,JOYOFS_Y);
    if( gJOY[joyIndex].AxeUnits[JOYOFS_Y].unit)
    {
      lValue *= (gJOY[joyIndex].AxeUnits[JOYOFS_Y].phyMax-
                 gJOY[joyIndex].AxeUnits[JOYOFS_Y].phyMin)/
                (gJOY[joyIndex].AxeUnits[JOYOFS_Y].logMax-
                 gJOY[joyIndex].AxeUnits[JOYOFS_Y].logMin);

    }
    dsPrint1(DBG_DETAILED,"Y is at %d\r\n",lValue);

    gJOY[joyIndex].joyState.lY = lValue;
  }
  if(gJOY[joyIndex].ulCapsAxes & JOYHAS_Z)
  {
    lValue = GetLogValue(joyIndex,JOYOFS_Z);
    if( gJOY[joyIndex].AxeUnits[JOYOFS_Z].unit)
    {
      lValue *= (gJOY[joyIndex].AxeUnits[JOYOFS_Z].phyMax-
                 gJOY[joyIndex].AxeUnits[JOYOFS_Z].phyMin)/
                (gJOY[joyIndex].AxeUnits[JOYOFS_Z].logMax-
                 gJOY[joyIndex].AxeUnits[JOYOFS_Z].logMin);
    }
    dsPrint1(DBG_DETAILED,"Z is at %d\r\n",lValue);

    gJOY[joyIndex].joyState.lZ = lValue;
  }

  if(gJOY[joyIndex].ulCapsAxes & JOYHAS_RX)
  {
    lValue = GetLogValue(joyIndex,JOYOFS_RX);
    if( gJOY[joyIndex].AxeUnits[JOYOFS_RX].unit)
    {
      lValue *= (gJOY[joyIndex].AxeUnits[JOYOFS_RX].phyMax-
                 gJOY[joyIndex].AxeUnits[JOYOFS_RX].phyMin)/
                (gJOY[joyIndex].AxeUnits[JOYOFS_RX].logMax-
                 gJOY[joyIndex].AxeUnits[JOYOFS_RX].logMin);

    }
    dsPrint1(DBG_DETAILED,"RX is at %d\r\n",lValue);

    gJOY[joyIndex].joyState.lRx = lValue;
  }
  if(gJOY[joyIndex].ulCapsAxes & JOYHAS_RY)
  {
    lValue = GetLogValue(joyIndex,JOYOFS_RY);
    if( gJOY[joyIndex].AxeUnits[JOYOFS_RY].unit)
    {
      lValue *= (gJOY[joyIndex].AxeUnits[JOYOFS_RY].phyMax-
                 gJOY[joyIndex].AxeUnits[JOYOFS_RY].phyMin)/
                (gJOY[joyIndex].AxeUnits[JOYOFS_RY].logMax-
                 gJOY[joyIndex].AxeUnits[JOYOFS_RY].logMin);

    }
    dsPrint1(DBG_DETAILED,"RY is at %d \r\n",lValue);

    gJOY[joyIndex].joyState.lRy = lValue;
  }
  if(gJOY[joyIndex].ulCapsAxes & JOYHAS_RZ)
  {
    lValue = GetLogValue(joyIndex,JOYOFS_RZ);
    if( gJOY[joyIndex].AxeUnits[JOYOFS_RZ].unit)
    {
      lValue *= (gJOY[joyIndex].AxeUnits[JOYOFS_RZ].phyMax-
                 gJOY[joyIndex].AxeUnits[JOYOFS_RZ].phyMin)/
                (gJOY[joyIndex].AxeUnits[JOYOFS_RZ].logMax-
                 gJOY[joyIndex].AxeUnits[JOYOFS_RZ].logMin);

    }
    dsPrint1(DBG_DETAILED,"RZ is at %d \r\n",lValue);

    gJOY[joyIndex].joyState.lRz = lValue;
  }

  i = 0;
  while(i<(USHORT) gJOY[joyIndex].DevCapsJoy.ulSliders)
  {
    lValue = GetLogValue(joyIndex, JOYOFS_SLIDER0+i);

    if( gJOY[joyIndex].AxeUnits[JOYOFS_SLIDER0+i].unit)
    {
      // Assume degrees and log 1 as top which is 0ø
      lValue *=
              (gJOY[joyIndex].AxeUnits[JOYOFS_SLIDER0+i].phyMax-
               gJOY[joyIndex].AxeUnits[JOYOFS_SLIDER0+i].phyMin)/
             (gJOY[joyIndex].AxeUnits[JOYOFS_SLIDER0+i].logMax-
              gJOY[joyIndex].AxeUnits[JOYOFS_SLIDER0+i].logMin);

    }

    dsPrint2(DBG_DETAILED,"Slider %d is at %d ø\r\n",i,lValue);

    gJOY[joyIndex].joyState.rglSlider[i] = lValue;
    i++;
  }

  i = 0;
  while(i< (USHORT)gJOY[joyIndex].DevCapsJoy.ulPOVs)
  {
    lValue = GetLogValue(joyIndex, JOYOFS_POV0+i);
    if(lValue)
    {
      if( gJOY[joyIndex].AxeUnits[JOYOFS_POV0+i].unit)
      {
        // Assume degrees and log 1 as top which is 0ø
        lValue = (lValue -1)*
                (gJOY[joyIndex].AxeUnits[JOYOFS_POV0+i].phyMax-
                 gJOY[joyIndex].AxeUnits[JOYOFS_POV0+i].phyMin)/
               (gJOY[joyIndex].AxeUnits[JOYOFS_POV0+i].logMax-
                gJOY[joyIndex].AxeUnits[JOYOFS_POV0+i].logMin);

        // Report in hundredths of degrees
        if( gJOY[joyIndex].AxeUnits[JOYOFS_POV0+i].phyMax>=270 &&
            gJOY[joyIndex].AxeUnits[JOYOFS_POV0+i].phyMax<=360)
          lValue *=100;
      }
      else
      {
        // No Units so no physical values translate to degrees
        lValue = (lValue-1) * (36000/gJOY[joyIndex].AxeUnits[JOYOFS_POV0+i].logMax);
      }
      dsPrint2(DBG_DETAILED,"POV %d is at %d ø\r\n",i,lValue);

    }
    else
    {
      lValue = 0x0000FFFF;
      dsPrint1(DBG_DETAILED,"POV %d is centered\r\n",i);
    }

    gJOY[joyIndex].joyState.rgdwPOV[i] = lValue;
    i++;
  }

  i = 0;
  while(i< (USHORT) gJOY[joyIndex].DevCapsJoy.ulButtons)
  {
    lValue = GetLogValue(joyIndex, JOYOFS_BUTTON0+i);
    gJOY[joyIndex].joyState.rgbButtons[i] = (lValue>0)? 0x80:0x00;

    #ifdef DEBUG
      if(lValue)
        dsPrint1(DBG_DETAILED," Button %d is PRESSED\r\n",i);
    #endif

    i++;
  }

}
#pragma optimize("", on)

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME:  ReadInterruptPipe                                */
/*                                                                    */
/* DESCRIPTIVE NAME:  Read interrupt pipe routine                     */
/*                                                                    */
/* FUNCTION:  This routine opens joysticks interrupt pipe to          */
/*            receive status reports.                                 */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT :  ReadInterruptPipe                                   */
/*    LINKAGE  :  CALL NEAR                                           */
/*                                                                    */
/* INPUT:  PRP_GENIOCTL pRP_GENIOCTL                                  */
/*                                                                    */
/* EXIT-NORMAL:  n/a                                                  */
/*                                                                    */
/* EXIT-ERROR:   n/a                                                  */
/*                                                                    */
/* EFFECTS:  sets error code in pRP_GENIOCTL->rph.Status              */
/*                                                                    */
/* INTERNAL REFERENCES:  none                                         */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:  USBCallIDC                                   */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void ReadInterruptPipe (PRP_GENIOCTL pRP_GENIOCTL)
{
   USBRB FAR  *processedRB;
   USBRB       hcdReqBlock;
   RP_GENIOCTL rp_USBReq;
   USHORT      deviceIndex;

   processedRB = (USBRB FAR *)pRP_GENIOCTL->ParmPacket;

   deviceIndex = LOUSHORT(processedRB->requestData2);

   setmem((PSZ)gJOY[deviceIndex].buffer, UI_RESERV, sizeof(gJOY[deviceIndex].buffer));

   hcdReqBlock.controllerId = processedRB->controllerId;
   hcdReqBlock.deviceAddress = processedRB->deviceAddress; // use default address to set address for unconfigured devices
   hcdReqBlock.endPointId = gJOY[deviceIndex].interruptPipeAddress;
   hcdReqBlock.status = 0; // not used
   hcdReqBlock.flags = USRB_FLAGS_TTYPE_IN | USRB_FLAGS_DET_INTRPT;
   if (!(processedRB->flags & USRB_FLAGS_DET_DTGGLEON))
     hcdReqBlock.flags |= USRB_FLAGS_DET_DTGGLEON;
   hcdReqBlock.buffer1 = (PUCHAR)gJOY[deviceIndex].buffer;
   hcdReqBlock.buffer1Length = gJOY[deviceIndex].ReportLength;
   hcdReqBlock.buffer2 = NULL;     // no additonal data to be sent to/from host
   hcdReqBlock.buffer2Length = 0;  // to complete this request
   hcdReqBlock.serviceTime = USB_DEFAULT_SRV_INTV;
   hcdReqBlock.maxPacketSize = USB_DEFAULT_PKT_SIZE;
   hcdReqBlock.maxErrorCount = USB_MAX_ERROR_COUNT;
   hcdReqBlock.usbIDC = (PUSBIDCEntry)JOYidc; // Address of IRQ processing routine to be called for this request
   hcdReqBlock.usbDS = GetDS();
   hcdReqBlock.category = USB_IDC_CATEGORY_CLIENT;        // set USBD layer as IRQ processor
   hcdReqBlock.requestData1 = JOY_IRQ_STATUS_INTPIPE;  // USBD I/O call type ID - set device address
   hcdReqBlock.requestData2 = MAKEULONG(deviceIndex, 0);        // index in device table to current device
   hcdReqBlock.requestData3 = 0;                        // not used
// hcdReqBlock.dsPhyAddr = 0;                  // data segment physical address

   setmem((PSZ)&rp_USBReq, 0, sizeof(rp_USBReq));
   rp_USBReq.rph.Cmd = CMDGenIOCTL;
   rp_USBReq.Category = USB_IDC_CATEGORY_CLASS;
   rp_USBReq.Function = USB_IDC_FUNCTION_ACCIO;
   rp_USBReq.ParmPacket = (PVOID)&hcdReqBlock;

   USBCallIDC (gpHIDIDC, gdsHIDIDC, (RP_GENIOCTL FAR *)&rp_USBReq);

#ifdef DEBUG
   dsPrint3 (DBG_IRQFLOW, "USBJOY: ReadInterruptPipe joyIndex = %d, endpt = %d, Status = %x\r\n",
             deviceIndex, hcdReqBlock.endPointId, pRP_GENIOCTL->rph.Status);
#endif
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME:  JOYClearStalled                                  */
/*                                                                    */
/* DESCRIPTIVE NAME:  Clears stalled communication pipe               */
/*                                                                    */
/* FUNCTION:  This routine calls USBD driver to clear stalled pipe.   */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT :  JOYClearStalled                                     */
/*    LINKAGE  :  CALL NEAR                                           */
/*                                                                    */
/* INPUT:  RP_GENIOCTL FAR *pRP_GENIOCTL                              */
/*                                                                    */
/* EXIT-NORMAL:  none                                                 */
/*                                                                    */
/* EXIT-ERROR:  none                                                  */
/*                                                                    */
/* EFFECTS:  sets error code in pRP_GENIOCTL->rph.Status              */
/*                                                                    */
/* INTERNAL REFERENCES:  none                                         */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:          USBCallIDC                                   */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void JOYClearStalled (RP_GENIOCTL FAR *pRP_GENIOCTL)
{
   USBRB FAR *processedRB;

   processedRB = (USBRB FAR *)pRP_GENIOCTL->ParmPacket;

#ifdef DEBUG
   dsPrint1 (DBG_DETAILED, "USBJOY: JOYClearStalled, joyIndex = %d\r\n", processedRB->requestData2);
#endif

   processedRB->requestData1 = JOY_IRQ_STATUS_STALLED;

   pRP_GENIOCTL->Category = USB_IDC_CATEGORY_CLASS;
   pRP_GENIOCTL->Function = USB_IDC_FUNCTION_CLRSTALL;

   USBCallIDC (gpHIDIDC, gdsHIDIDC, pRP_GENIOCTL);
}

