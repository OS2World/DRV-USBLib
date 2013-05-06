/* $ID$ */

/* Copyright 2001,2002 Markus Montkowski                                      */
/* This code is released under the Aladdin Free Public License                */
/*                    (Version 9, September 18, 2000)                         */
/* See License file in root of CVS for details                                */

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME: RESIRQ.C                                               */
/*                                                                            */
/*   DESCRIPTIVE NAME: USB Resource Manager interrupt processing routines     */
/*                                                                            */
/*   FUNCTION: These routines handle the IRQ IDC calls for the driver.        */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS: IRQSwitch                                                  */
/*                 SetIntfReq                                                 */
/*                 AttachCompleted                                            */
/*                 GetString                                                  */
/*                 CompIEEEStrings                                            */
/*                 ComparePRT                                                 */
/*                                                                            */
/*   EXTERNAL REFERENCES: BufferAddr                                          */
/*                        movmem                                              */
/*                        GetStringD                                          */
/*                        GetDeviceID                                         */
/*                        SimulateModem                                       */
/*                        USBCallIDC                                          */
/*                        setmem                                              */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark      yy/mm/dd  Programmer    Comment                                 */
/*  ------    --------  ----------    -------                                 */
/*            00/01/14  MM                                                    */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#include "res.h"

static void SetConfig (PRP_GENIOCTL pRP);
static void StringDLen (PRP_GENIOCTL pRP);
static void StringD (PRP_GENIOCTL pRP);
static void DeviceIDLen (PRP_GENIOCTL pRP);
static void DeviceID (PRP_GENIOCTL pRP);
static void SetAltIntf (PRP_GENIOCTL pRP);   // 02/18/2000 MB
static void DataOut (PRP_GENIOCTL pRP);
static void DataIn (PRP_GENIOCTL pRP);
static void DataRW (PRP_GENIOCTL pRP);
static void ClearStalled (PDEVICELIST pDevive, UCHAR endPoint);
static void MakeDeviceID (USHORT DevIndex);

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: IRQSwitch                                         */
/*                                                                    */
/* DESCRIPTIVE NAME: IRQ processing Switch                            */
/*                                                                    */
/* FUNCTION: This routine processes USB Resource Manager driver       */
/*             IRQ calls.                                             */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: IRQSwitch                                             */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to Request Packet                             */
/*                                                                    */
/* EXIT-NORMAL: n/a                                                   */
/*                                                                    */
/* EXIT-ERROR: n/a                                                    */
/*                                                                    */
/* EFFECTS: pRP->rph.Status                                           */
/*                                                                    */
/* INTERNAL REFERENCES: SetConfig                                     */
/*                      GetString                                     */
/*                      MakeDeviceID                                  */
/*                      SetIntfReq                                    */
/*                      ClearStalled                                  */
/*                                                                    */
/* EXTERNAL REFERENCES: None                                          */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void IRQSwitch (PRP_GENIOCTL pRP)
{
   USBRB  FAR *pRB = (USBRB FAR *)pRP->ParmPacket;
   SHORT  DevIndex =  (SHORT)pRB->requestData2;
   USHORT awakeCount=0;
   USHORT rc;

#ifdef DEBUG
   dsPrint4 (DBG_IRQFLOW, "USBRESMGR: IRQ %x from Dev[%x], S=%x, s=%x\r\n",
             pRB->requestData1, DevIndex, pRP->rph.Status, pRB->status);
#endif
  switch (pRB->requestData1)
  {
    case RES_IRQ_STRINGLEN:
      GetStringD(DevIndex,pRB->requestData3);
      break;
    case RES_IRQ_STRING:
    case RES_IRQ_CONTROL:
    case RES_IRQ_RWIRQ:
#ifdef DEBUG
   dsPrint1 (DBG_IRQFLOW, "USBRESMGR: IRQ Unblock %x\r\n",
             pRB->requestData3);
#endif
      pRB->requestData1 = RES_IRQ_PROCESSED;
      //DevHelp_Beep(440,100);
      rc = DevHelp_ProcRun(pRB->requestData3,&awakeCount);
#ifdef DEBUG
   dsPrint2 (DBG_IRQFLOW, "USBRESMGR: IRQ Unblock rc=%x, Thread Count = %d\r\n",
             rc, awakeCount);
#endif
      break;
    case RES_IRQ_RWBULK:
      gusCheck++;
      DataRW(pRP);
      break;

    case RES_IRQ_GETDESC:
      break;
  }
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: DataRW                                            */
/*                                                                    */
/* DESCRIPTIVE NAME: Bulk IRQ service routine                         */
/*                                                                    */
/* FUNCTION: This function services the Write Data IRQ.               */
/*                                                                    */
/* NOTES: IRQ code = RES_IRQ_RWBULK                                   */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: DataRW                                                */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to Request Packet                             */
/*                                                                    */
/* EXIT-NORMAL: n/a                                                   */
/*                                                                    */
/* EXIT-ERROR: n/a                                                    */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         ClearStalled                                  */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

static void DataRW (PRP_GENIOCTL pRP)
{
  USBRB  FAR *pRB  = (USBRB FAR *)pRP->ParmPacket;
  USHORT usToggleGrp =  (pRB->flags &USRB_FLAGS_TTYPE_IN)?0:1;
  PUSB_IORB  pIoRB = (PUSB_IORB)pRB->requestData2;
  USHORT rc;
  UCHAR  ucEndpoint  = pRB->endPointId &DEV_ENDPT_ADDRMASK;
  USHORT usToggleBit = 1<< ucEndpoint;
  PLIBUSB_BULK_REQ pBulkReq;

#ifdef DEBUG
  dsPrint2 ( DBG_DETAILED, 
            "USBRESMGR: DataRW to Device[%x], L=%x\r\n",
            pIoRB->pDevice, pRB->buffer1Length);
  dsPrint4 ( DBG_IRQFLOW,
             "USBRESMGR: Endpoint %d Toggle Group %d, Value %0x UpdateBit %0x\r\n",
             pRB->endPointId &DEV_ENDPT_ADDRMASK,
             usToggleGrp,
             pIoRB->pDevice->wToggle[usToggleGrp],
             usToggleBit);
#endif
  // Safty check that the RB wasn't freed and re used while the USB transaction was done
  if (pIoRB->ulID != pRB->requestData3)
  {
    DevHelp_Beep(2200,200);
    return;
  }

  if(DevHelp_PhysToGDTSelector(pIoRB->pPhys,
	                           sizeof(LIBUSB_BULK_REQ),
	                           gSelIoRB[SEL_BULK]))
  {
    DevHelp_Beep(4400,500);
    DevHelp_UnLock(pIoRB->ulLockData);
    DevHelp_UnLock(pIoRB->ulLockParam);
    if(DevHelp_OpenEventSem (pIoRB->ulEventDone) == 0)  
    {
      DevHelp_PostEventSem (pIoRB->ulEventDone);
      DevHelp_CloseEventSem (pIoRB->ulEventDone);
    }
    FreeIoRB(pIoRB);
    return;
  }

  pBulkReq = MAKEP(gSelIoRB[SEL_BULK], NULL);

  if (pRP->rph.Status == USB_IDC_RC_IOFAILED)
  {
    DevHelp_Beep(440,500);
    #ifdef DEBUG
      dsPrint1 (DBG_DETAILED, "USBRESMGR: IO failed Status = %d\r\n",
                pRB->status);
    #endif
    if (pRB->status & USRB_STATUS_STALLED)
    {
      ClearStalled (pIoRB->pDevice, ucEndpoint);
      pIoRB->pDevice->wToggle[usToggleGrp]  |= usToggleBit;
      pBulkReq->usStatus = USB_IORB_STALLED;
    }
    else
      pBulkReq->usStatus = USB_IORB_FAILED;

    DevHelp_UnLock(pIoRB->ulLockData);
    DevHelp_UnLock(pIoRB->ulLockParam);
    if(DevHelp_OpenEventSem (pIoRB->ulEventDone) == 0)  
    {
      DevHelp_PostEventSem (pIoRB->ulEventDone);
      DevHelp_CloseEventSem (pIoRB->ulEventDone);
    }
    FreeIoRB(pIoRB);

  }
  else
  {
    // No I/O Errors
    if (! (pRB->flags & USRB_FLAGS_DET_DTGGLEON))
    {
      #ifdef DEBUG
        dsPrint (DBG_DETAILED, "USBRESMGR: Set Toggle Bit\r\n");
      #endif
      pIoRB->pDevice->wToggle[usToggleGrp]  |= usToggleBit;
    }
    else
    {
      #ifdef DEBUG
        dsPrint (DBG_DETAILED, "USBRESMGR: Clear Toggle Bit\r\n");
      #endif
      pIoRB->pDevice->wToggle[usToggleGrp]  &= ~usToggleBit;
    }
    #ifdef DEBUG
      dsPrint1 ( DBG_DETAILED, "USBRESMGR: New Toggle Value %0x\r\n",
                 pIoRB->pDevice->wToggle[usToggleGrp] );
    #endif
    pBulkReq->usDataProcessed = pBulkReq->usDataProcessed + pRB->buffer1Length;
    pBulkReq->usDataRemain    = pBulkReq->usDataRemain - pRB->buffer1Length;
    pIoRB->usDataProcessed = pBulkReq->usDataProcessed;
    pIoRB->usDataRemain = pBulkReq->usDataRemain;
    #ifdef DEBUG
      dsPrint3 ( DBG_DETAILED, "USBRESMGR: Processed %d bytes, remaining %d (buffersize %d)\r\n",
                 pBulkReq->usDataProcessed, pBulkReq->usDataRemain, pRB->buffer1Length );
    #endif
    if(pBulkReq->usDataRemain)
    {
      #ifdef DEBUG
        dsPrint (DBG_DETAILED, "USBRESMGR: Next Buffer\r\n");
      #endif
      ProcessIoRB(pIoRB);
    }
    else
    {
      #ifdef DEBUG
        dsPrint (DBG_DETAILED, "USBRESMGR: Done\r\n");
      #endif
      pBulkReq->usStatus = USB_IORB_DONE;
      DevHelp_UnLock(pIoRB->ulLockData);
      DevHelp_UnLock(pIoRB->ulLockParam);
      if(DevHelp_OpenEventSem (pIoRB->ulEventDone) == 0)  
      {
        DevHelp_PostEventSem (pIoRB->ulEventDone);
        DevHelp_CloseEventSem (pIoRB->ulEventDone);
      }
      else
      {
        #ifdef DEBUG
          dsPrint (DBG_DETAILED, "USBRESMGR: Open Semaphore failed\r\n");
        #endif
      }
      FreeIoRB(pIoRB);
    }
  }
  
}


/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: IrqRW                                             */
/*                                                                    */
/* DESCRIPTIVE NAME: Irq Pipe IRQ service routine                     */
/*                                                                    */
/* FUNCTION: This function services the Read/write Irq                */
/*                                                                    */
/* NOTES: IRQ code = RES_IRQ_RWIRQ                                    */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: DataIn                                                */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to Request Packet                             */
/*                                                                    */
/* EXIT-NORMAL: n/a                                                   */
/*                                                                    */
/* EXIT-ERROR: n/a                                                    */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         ClearStalled                                  */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

static void IrqRW (PRP_GENIOCTL pRP)
{
  USBRB  FAR *pRB    = (USBRB FAR *)pRP->ParmPacket;
  USHORT usToggleGrp = (pRB->flags &USRB_FLAGS_TTYPE_IN)?0:1;
  PUSB_IORB  pIoRB   = (PUSB_IORB)pRB->requestData2;
  USHORT rc;
  UCHAR  ucEndpoint  = pRB->endPointId &DEV_ENDPT_ADDRMASK;
  USHORT usToggleBit = 1<< ucEndpoint;
  PLIBUSB_IRQ_DATA pIrqData;

#ifdef DEBUG
  dsPrint2 ( DBG_DETAILED, 
            "USBRESMGR: IrqRW to Device[%x], L=%x\r\n",
            pIoRB->pDevice, pRB->buffer1Length);
  dsPrint4 ( DBG_IRQFLOW,
             "USBRESMGR: Endpoint %d Toggle Group %d, Value %0x UpdateBit %0x\r\n",
             pRB->endPointId &DEV_ENDPT_ADDRMASK,
             usToggleGrp,
             pIoRB->pDevice->wToggle[usToggleGrp],
             usToggleBit);
#endif
  // Safty check that the RB wasn't freed and re used while the USB transaction was done
  if (pIoRB->ulID != pRB->requestData3)
  {
    return;
  }

  if(DevHelp_PhysToGDTSelector(pIoRB->pPhysData,
	                           sizeof(LIBUSB_BULK_REQ),
	                           gSelIoRB[SEL_IRQ]))
  {
    DevHelp_Beep(4400,500);
    DevHelp_UnLock(pIoRB->ulLockData);
    DevHelp_UnLock(pIoRB->ulLockParam);
    if(DevHelp_OpenEventSem (pIoRB->ulEventDone) == 0)  
    {
      DevHelp_PostEventSem (pIoRB->ulEventDone);
      DevHelp_CloseEventSem (pIoRB->ulEventDone);
    }
    FreeIoRB(pIoRB);
    return;
  }

  pIrqData = MAKEP(gSelIoRB[SEL_BULK], NULL);

  if (pRP->rph.Status == USB_IDC_RC_IOFAILED)
  {
    DevHelp_Beep(440,500);
    #ifdef DEBUG
      dsPrint1 (DBG_DETAILED, "USBRESMGR: IO failed Status = %d\r\n",
                pRB->status);
    #endif
    if (pRB->status & USRB_STATUS_STALLED)
    {
      ClearStalled (pIoRB->pDevice, ucEndpoint);
      pIoRB->pDevice->wToggle[usToggleGrp]  |= usToggleBit;
      pIrqData->usStatus = USB_IORB_STALLED;
    }
    else
      pIrqData->usStatus = USB_IORB_FAILED;

    DevHelp_UnLock(pIoRB->ulLockData);
    DevHelp_UnLock(pIoRB->ulLockParam);
    if(DevHelp_OpenEventSem (pIoRB->ulEventDone) == 0)  
    {
      DevHelp_PostEventSem (pIoRB->ulEventDone);
      DevHelp_CloseEventSem (pIoRB->ulEventDone);
    }
    FreeIoRB(pIoRB);

  }
  else
  {
    // No I/O Errors
    BOOL fChanged=FALSE;
    UCHAR i;

    if (! (pRB->flags & USRB_FLAGS_DET_DTGGLEON))
    {
      #ifdef DEBUG
        dsPrint (DBG_DETAILED, "USBRESMGR: Set Toggle Bit\r\n");
      #endif
      pIoRB->pDevice->wToggle[usToggleGrp]  |= usToggleBit;
    }
    else
    {
      #ifdef DEBUG
        dsPrint (DBG_DETAILED, "USBRESMGR: Clear Toggle Bit\r\n");
      #endif
      pIoRB->pDevice->wToggle[usToggleGrp]  &= ~usToggleBit;
    }
    #ifdef DEBUG
      dsPrint1 ( DBG_DETAILED, "USBRESMGR: New Toggle Value %0x\r\n",
                 pIoRB->pDevice->wToggle[usToggleGrp] );
    #endif
    
    for(i = 0; i < pRB->buffer1Length; i++)
    {
      if(pIrqData->ucBufferCurrent[i] != pIrqData->ucBufferLast[i])
        fChanged = TRUE;
      pIrqData->ucBufferCurrent[i] = pIrqData->ucBufferLast[i];
    }
    if(fChanged || pIrqData->usRes==0)
    {
      pIrqData->usRes = 1;
      if(DevHelp_OpenEventSem (pIrqData->ulEventChanged) == 0)  
      {
        DevHelp_PostEventSem (pIrqData->ulEventChanged);
        DevHelp_CloseEventSem (pIrqData->ulEventChanged);
      }
    }
    #ifdef DEBUG
      dsPrint (DBG_DETAILED, "USBRESMGR: Next Buffer\r\n");
    #endif
    ProcessIoRB(pIoRB);
  }
}


/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: ClearStalled                                      */
/*                                                                    */
/* DESCRIPTIVE NAME: Clear Stalled pipe                               */
/*                                                                    */
/* FUNCTION: This function calls USBD driver to clear stalled pipe.   */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: ClearStalled                                          */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: USHORT usDevIndex = Device index                            */
/*        UCHAR endPoint = endpoint to be reset                       */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:         USBCallIDC                                    */
/*                      setmem                                        */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

static void ClearStalled (PDEVICELIST pDevice, UCHAR endPoint)
{
   RP_GENIOCTL       rp;
   USBClearStalled   rb;

#ifdef DEBUG
   dsPrint2 (DBG_DETAILED, "USBRESMGR: ClearStalled Device[%x], ep=%x\r\n", pDevice, endPoint);
#endif

   rb.controllerId  = pDevice->pDeviceInfo->ctrlID;
   rb.deviceAddress = pDevice->pDeviceInfo->deviceAddress;
   rb.endPointId = endPoint;
   rb.clientIDCAddr = NULL;   // no irq notification for this request
   rb.clientDS = 0;
   rb.irqSwitchValue = 0;
   rb.requestData2 = 0;
   rb.requestData3 = 0;
   rb.category = USB_IDC_CATEGORY_CLASS;
   rb.clearStalled = &pDevice->setupPack;

   setmem ((PSZ)&rp, 0, sizeof(rp));
   rp.rph.Cmd = CMDGenIOCTL;
   rp.Category = USB_IDC_CATEGORY_USBD;
   rp.Function = USB_IDC_FUNCTION_CLRSTALL;
   rp.ParmPacket = (PVOID)&rb;

   USBCallIDC (gpUSBDIDC, gdsUSBDIDC, (RP_GENIOCTL FAR *)&rp);
}



