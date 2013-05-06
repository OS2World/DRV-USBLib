/* $ID$ */

/* Copyright 2001,2002 Markus Montkowski                                      */
/* This code is released under the Aladdin Free Public License                */
/*                    (Version 9, September 18, 2000)                         */
/* See License file in root of CVS for details                                */

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME: RESIDC.C                                               */
/*                                                                            */
/*   DESCRIPTIVE NAME: USB Resource Manager Driver                            */
/*                     inter-device driver communication routines             */
/*                                                                            */
/*   FUNCTION: These routines handle the PDD-PDD IDC for the                  */
/*             USB Resource Manager Driver.                                   */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS: IDCEntry                                                   */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          00/01/14  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#include "res.h"

static void Service (PRP_GENIOCTL pRP);
static void Detach (PRP_GENIOCTL pRP);

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: IDCEntry                                          */
/*                                                                    */
/* DESCRIPTIVE NAME:  PDD-PDD IDC entry point and request router      */
/*                                                                    */
/* FUNCTION:  This routine is the PDD-PDD IDC entry point and         */
/*            request router. IDC function requests are routed        */
/*            to the appropriate worker routine. The address of       */
/*            this routine is returned to other device drivers via    */
/*            the DevHelp AttachDD call.                              */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: IDCEntry                                              */
/*     LINKAGE: CALL FAR                                              */
/*                                                                    */
/* INPUT: pRP = pointer to Request Packet                             */
/*                                                                    */
/* EXIT-NORMAL: n/a                                                   */
/*                                                                    */
/* EXIT-ERROR: n/a                                                    */
/*                                                                    */
/* EFFECTS: pRP->rph.Status                                           */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         Service                                       */
/*                      Detach                                        */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:         IRQSwitch                                     */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void far IDCEntry (PRP_GENIOCTL pRP)
{
   USHORT status = pRP->rph.Status;

#ifdef DEBUG
   dsPrint2 (DBG_IRQFLOW, "USBRESMGR: IDC C=%x, F=%x\r\n", pRP->Category, pRP->Function);
#endif

   pRP->rph.Status = 0;

   if (pRP->rph.Cmd != CMDGenIOCTL || !pRP->ParmPacket)
   {
      pRP->rph.Status |= STERR | USB_IDC_PARMERR;
   }
   else if (pRP->Category != USB_IDC_CATEGORY_CLASS)
   {
      pRP->rph.Status |= STERR | USB_IDC_WRONGCAT;
   }
   else
   {
      switch (pRP->Function)
      {
      case  USB_IDC_FUNCTION_PRCIRQ:             // 0x44
            pRP->rph.Status = status;
            IRQSwitch (pRP);
            break;

      case  USB_IDC_FUNCTION_CHKSERV:            // 0x45
            Service (pRP);
            break;

      case  USB_IDC_FUNCTION_DETDEV:             // 0x46
            Detach (pRP);
            break;

      default:
            pRP->rph.Status |= STERR | USB_IDC_WRONGFUNC;
      }
   }
   pRP->rph.Status |= STDON;

#ifdef DEBUG
   dsPrint1 (DBG_IRQFLOW, "USBRESMGR: IDC S=%x\r\n", pRP->rph.Status);
#endif
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: Service                                           */
/*                                                                    */
/* DESCRIPTIVE NAME: Check for Service worker routine                 */
/*                                                                    */
/* FUNCTION: This routine stores device descriptor data and notifys   */
/*           registered apps via Event that a new device was attached */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: Service                                               */
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
/* INTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

static void Service (PRP_GENIOCTL pRP)
{
  USHORT         DevIndex,index,i;
  DeviceInfo     FAR *pDevInfo;
  UCHAR          ucPipe;
  USHORT         wLength;

  pRP->rph.Status = USB_IDC_RC_SERVREJCTD;

  if (gNumDevices < MAX_DEVICES)
  {
    for (DevIndex = 0; DevIndex < MAX_DEVICES; DevIndex++)
    {
      if (!gUSBDevices[DevIndex].bAttached)
        break;
    }
  }
  else
  {
    #ifdef DEBUG
      dsPrint1 (DBG_CRITICAL, "USBRESMGR: Service REJECTED, gNumDevices=%d\r\n", gNumDevices);
    #endif
    return;
  }

  pDevInfo = ((USBCDServe FAR *)pRP->ParmPacket)->pDeviceInfo;

  gListIndex[gNumDevices++] = DevIndex;
  gUSBDevices[DevIndex].bAttached = TRUE;
  gUSBDevices[DevIndex].pDeviceInfo = pDevInfo;
  gUSBDevices[DevIndex].usHandleID  = gusHandleCounter++; // give Device a more or less Unique ID
  gUSBDevices[DevIndex].usSFN   = 0;
  gUSBDevices[DevIndex].ucAltInterface = 0;
  gUSBDevices[DevIndex].wToggle[0] = 0x0;
  gUSBDevices[DevIndex].wToggle[1] = 0x0;
  #ifdef DEBUG
    dsPrint4 ( DBG_HLVLFLOW,
               "USBRESMGR: Service prt[%x] ctrlID=%x deviceAddress=%x pInfo=%lx\r\n",
               DevIndex,
               gUSBDevices[DevIndex].pDeviceInfo->ctrlID,
               gUSBDevices[DevIndex].pDeviceInfo->deviceAddress,
               (ULONG)gUSBDevices[DevIndex].pDeviceInfo);
  #endif
  
  for (index = 0; index < MAX_SEMS; index++)
  {
    if (gSEMNewDev[index] != 0)
    {
      if ((i = DevHelp_OpenEventSem (gSEMNewDev[index])) == 0)
      {
        DevHelp_PostEventSem (gSEMNewDev[index]);
        DevHelp_CloseEventSem (gSEMNewDev[index]);
      }
      #ifdef DEBUG
      else
      {
        dsPrint2 (DBG_DETAILED, "USBRESMGR: OpenEventSem=%lx err=%x\r\n", gSEMNewDev[index], i);
      }
      #endif
    }

    if( (gDeviceSEM[index].hSemaphoreAdd!=0) &&
        (gDeviceSEM[index].usVendorID == gUSBDevices[DevIndex].pDeviceInfo->descriptor.idVendor) &&
        (gDeviceSEM[index].usProductID ==  gUSBDevices[DevIndex].pDeviceInfo->descriptor.idProduct) &&
        ( (gDeviceSEM[index].usBCDDevice == 0xffff) ||
          ((gDeviceSEM[index].usBCDDevice != 0xffff) &&
           (gDeviceSEM[index].usBCDDevice ==  gUSBDevices[DevIndex].pDeviceInfo->descriptor.bcdDevice)) ) )
    {
      if ((i = DevHelp_OpenEventSem (gDeviceSEM[index].hSemaphoreAdd)) == 0)
      {
        DevHelp_PostEventSem (gDeviceSEM[index].hSemaphoreAdd);
        DevHelp_CloseEventSem (gDeviceSEM[index].hSemaphoreAdd);
      }
      #ifdef DEBUG
      else
      {
        dsPrint2 (DBG_DETAILED, "USBRESMGR: OpenEventSem=%lx err=%x\r\n", gSEMNewDev[index], i);
      }
      #endif
    }
  }
  gusCheck = 0;

  pRP->rph.Status = USB_IDC_RC_OK;
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: Detach                                            */
/*                                                                    */
/* DESCRIPTIVE NAME: Detach USB device                                */
/*                                                                    */
/* FUNCTION: This routine is called when a device is detached         */
/*           The entry gets freed and apps get nitifyed of the detach */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: Detach                                                */
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
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:                                                       */
/* EXTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

static void Detach (PRP_GENIOCTL pRP)
{
  USBDetach FAR *pDetData = (USBDetach FAR *)pRP->ParmPacket;
  USHORT         DevIndex, index, i;
  DeviceInfo FAR *pDeviceInfo;
  if (gNumDevices)
  {
    for (DevIndex = 0; DevIndex < MAX_DEVICES; DevIndex++)
    {
      if (gUSBDevices[DevIndex].bAttached)
      {
        if (gUSBDevices[DevIndex].pDeviceInfo->ctrlID        == pDetData->controllerId &&
            gUSBDevices[DevIndex].pDeviceInfo->deviceAddress == pDetData->deviceAddress)
        {

          #ifdef DEBUG
            dsPrint1 (DBG_IRQFLOW, "USBRESMGR: Detach prt[%x]\r\n", DevIndex);
          #endif
          gNumDevices--;
          pDeviceInfo = gUSBDevices[DevIndex].pDeviceInfo;
          gUSBDevices[DevIndex].pDeviceInfo = NULL;
          gUSBDevices[DevIndex].usHandleID  = 0;
          gUSBDevices[DevIndex].usSFN       = 0;
          gUSBDevices[DevIndex].bAttached   = FALSE;
          break;
        }
      }
    } // for...

    for( index = 0; index < gNumDevices;index++)
    {
      if(gListIndex[index] == DevIndex)
      {
        for(;index< gNumDevices;index++)
          gListIndex[index] = gListIndex[index+1];
      }
    }
    gListIndex[index] = 0;

    for (index = 0; index < MAX_SEMS; index++)
    {
      if (gSEMDevRemove[index] != 0)
      {
        if ((i = DevHelp_OpenEventSem (gSEMDevRemove[index])) == 0)
        {
          DevHelp_PostEventSem (gSEMDevRemove[index]);
          DevHelp_CloseEventSem (gSEMDevRemove[index]);
        }
        #ifdef DEBUG
        else
        {
          dsPrint2 (DBG_DETAILED, "USBRESMGR: OpenEventSem=%lx err=%x\r\n", gSEMNewDev[index], i);
        }
        #endif
      }

      if( (gDeviceSEM[index].hSemaphoreRemove!=0) &&
          (gDeviceSEM[index].usVendorID == pDeviceInfo->descriptor.idVendor) &&
          (gDeviceSEM[index].usProductID == pDeviceInfo->descriptor.idProduct) &&
          ( (gDeviceSEM[index].usBCDDevice == 0xffff) ||
            ((gDeviceSEM[index].usBCDDevice != 0xffff) &&
             (gDeviceSEM[index].usBCDDevice == pDeviceInfo->descriptor.bcdDevice)) ) )
      {
        if ((i = DevHelp_OpenEventSem (gDeviceSEM[index].hSemaphoreRemove)) == 0)
        {
          DevHelp_PostEventSem (gDeviceSEM[index].hSemaphoreRemove);
          DevHelp_CloseEventSem (gDeviceSEM[index].hSemaphoreRemove);
        }
        #ifdef DEBUG
        else
        {
          dsPrint2 (DBG_DETAILED, "USBRESMGR: OpenEventSem=%lx err=%x\r\n", gSEMNewDev[index], i);
        }
        #endif
      }
    }

  }

}


