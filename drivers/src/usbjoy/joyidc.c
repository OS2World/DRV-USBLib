/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  JOYIDC.C                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB Joystick driver IDC routines                      */
/*                                                                            */
/*   FUNCTION: These routines handle the IDC for the USB Joystick driver.     */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:                                                            */
/*             JOYidc         IDC Entry Point                                 */
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
/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME:  JOYidc                                           */
/*                                                                    */
/* DESCRIPTIVE NAME:  USB Joystick IDC entry point                    */
/*                                                                    */
/* FUNCTION:  This routine is the USB Joystick IDC entry point        */
/*            router/handler. IDC function requests are routed to the */
/*            appropriate worker routine. The address of this routine */
/*            is returned to other device drivers via the DevHlp      */
/*            AttachDD call.                                          */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT :  JOYidc                                              */
/*    LINKAGE  :  CALL FAR                                            */
/*                                                                    */
/* INPUT:  RP_GENIOCTL FAR *pRP_GENIOCTL                              */
/*                                                                    */
/* EXIT-NORMAL:  JOY IDC RC OK                                        */
/*                                                                    */
/* EXIT-ERROR:  device driver error code                              */
/*                                                                    */
/* EFFECTS:  None                                                     */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void FAR JOYidc (RP_GENIOCTL FAR *pRP_GENIOCTL)
{
  USHORT status;

  #ifdef DEBUG
    dsPrint3 ( DBG_HLVLFLOW, "USBJOY: IDC Enter> Category = 0x%x, Function = 0x%x, Status 0x%x\r\n",
               pRP_GENIOCTL->Category, pRP_GENIOCTL->Function, pRP_GENIOCTL->rph.Status);
  #endif

  status = pRP_GENIOCTL->rph.Status;
  pRP_GENIOCTL->rph.Status = USB_IDC_RC_OK;

  if ( (pRP_GENIOCTL->rph.Cmd != CMDGenIOCTL) ||
       (!pRP_GENIOCTL->ParmPacket) )
  {
    #ifdef DEBUG
      dsPrint2 (DBG_HLVLFLOW, "USBJOY: IDC Wrong paraameter : Cmd= 0x%x, Packet = 0x%x\r\n",
                pRP_GENIOCTL->rph.Cmd, (ULONG)pRP_GENIOCTL->ParmPacket);
    #endif
    pRP_GENIOCTL->rph.Status = USB_IDC_RC_PARMERR;
  }

  if (pRP_GENIOCTL->rph.Status == USB_IDC_RC_OK)
  {
    if (pRP_GENIOCTL->Category == USB_IDC_CATEGORY_CLIENT)
    {
      switch (pRP_GENIOCTL->Function)
         {
         case USB_IDC_FUNCTION_PRCIRQ:
            pRP_GENIOCTL->rph.Status = status;
            JOYirq (pRP_GENIOCTL);
            break;
         case USB_IDC_FUNCTION_CHKSERV:
            JOYserv (pRP_GENIOCTL);
            break;
         case USB_IDC_FUNCTION_DETDEV:
            JOYdet (pRP_GENIOCTL);
            break;
//         case GAME_IDC_LIST_DEVICES;
//            g_usGameDriverId = pRP_GENIOCTL->ParmPacket;
//            JOYEnumDevices(g_usGameDriverId);
         default:
            pRP_GENIOCTL->rph.Status = USB_IDC_RC_WRONGFUNC;
         }
    }
    else
    {
      pRP_GENIOCTL->rph.Status = USB_IDC_RC_WRONGCAT;
    }
  }
#ifdef DEBUG
   dsPrint3 (DBG_HLVLFLOW, "USBJOY: IDC Leave < Category = 0x%x, Function = 0x%x, Status = 0x%x\r\n",
             pRP_GENIOCTL->Category, pRP_GENIOCTL->Function, pRP_GENIOCTL->rph.Status);
#endif
}



void SetIdleTime (USHORT joyIndex, USHORT kbdIRQstatus)
{
   USBRB       rbHID;     // I/O request block
   RP_GENIOCTL rpHID;     // request packet

#ifdef DEBUG
   dsPrint3 (DBG_SPECIFIC, "USBJOY: SetIdleTime = %d, joyIndex = %d, IRQ = %d\r\n",
             HIBYTE(gJOY[joyIndex].setITpack.wValue), joyIndex, kbdIRQstatus);
#endif

   rbHID.buffer1 = (PUCHAR)&gJOY[joyIndex].setITpack;
   rbHID.buffer1Length = sizeof(gJOY[joyIndex].setITpack);
   rbHID.buffer2 = NULL;
   rbHID.buffer2Length = NULL;

   rbHID.controllerId  = gJOY[joyIndex].controllerID;
   rbHID.deviceAddress = gJOY[joyIndex].joyAddr;
   rbHID.endPointId    = USB_DEFAULT_CTRL_ENDPT;
   rbHID.status        = 0; // not used
   rbHID.flags         = USRB_FLAGS_TTYPE_SETUP;
   rbHID.serviceTime   = USB_DEFAULT_SRV_INTV;
   rbHID.maxPacketSize = USB_DEFAULT_PKT_SIZE;
   rbHID.maxErrorCount = USB_MAX_ERROR_COUNT;

   rbHID.usbIDC = (PUSBIDCEntry)JOYidc;       // Address of IRQ processing routine to be called for this request
   rbHID.usbDS = GetDS();

   rbHID.category = USB_IDC_CATEGORY_CLIENT;         // set client layer as IRQ processor
   rbHID.requestData1 = JOY_IRQ_STATUS_IDLESET;//MAKEULONG (kbdIRQstatus, 0);
   rbHID.requestData2 = MAKEULONG (joyIndex, 0);
   rbHID.requestData3 = 0;                        // not used
// rbHID.dsPhyAddr = 0;                  // data segment physical address

   setmem((PSZ)&rpHID, 0, sizeof(rpHID));
   rpHID.rph.Cmd = CMDGenIOCTL;
   rpHID.Category = USB_IDC_CATEGORY_CLASS;
   rpHID.Function = USB_IDC_FUNCTION_ACCIO;
   rpHID.ParmPacket = (PVOID)&rbHID;

   USBCallIDC (gpHIDIDC, gdsHIDIDC, (RP_GENIOCTL FAR *)&rpHID);

#ifdef DEBUG
   dsPrint4 (DBG_SPECIFIC, "USBJOY: SetIdleTime = %d, joyIndex = %d, IRQ = %d, Status = 0x%x\r\n",
             HIBYTE(gJOY[joyIndex].setITpack.wValue), joyIndex, kbdIRQstatus, rpHID.rph.Status);
#endif
}

/******************* END  OF  SPECIFICATIONS **************************/

void GetJoyIndex (void)
{
   USHORT joyIndex;

   if (gNoOfJOYs >= gDevice)
   {
      for (joyIndex = NULL, gJoyIndex = NULL; joyIndex < MAX_JOYS; joyIndex++)
         if (gJOY[joyIndex].active)
            if (++gJoyIndex == gDevice)
            {
               gJoyIndex = joyIndex;
               break;
            }
   }
   else
     gJoyIndex = FULL_WORD;
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME:  Setup...                                         */
/*                                                                    */
/* DESCRIPTIVE NAME:  Setup routines for the supported ReporItems     */
/*                                                                    */
/* FUNCTION:  This routines are used to setup the JoyItems in the     */
/*            global gJOY array for the new attached device.          */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT :                                                      */
/*    LINKAGE  :                                                      */
/*                                                                    */
/* INPUT:  USHORT joyIntex        Index of the new Device gJOY        */
/*         ReportItemData pItem   Pointer to the report Item          */
/*         USHORT *pusOffSet      Pointer to counter for Bit Offset   */
/*                                                                    */
/* EXIT-NORMAL:                                                       */
/*                                                                    */
/* EXIT-ERROR:                                                        */
/*                                                                    */
/* EFFECTS:  Modifies usOffSet in JoyServ via pusOffset               */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/


USHORT SetupXYZAxes(USHORT joyIndex, ReportItemData FAR *pItem, USHORT usOffset)
{
  if(pItem->localFeatures.usageMin == pItem->localFeatures.usageMax)
  {
    // Some Joysticks only seam to report the Upper USAGE they support
    // Saitek reports Min and Max as HID_GDESKTOP_USAGE_Y but a
    // Reportcount of 2 so use the count as indicator which axes are supported
    switch(pItem->itemFeatures.reportCount)
    {
      case 1:
        // Only 1 reportcount so use usagePage
        switch(pItem->localFeatures.usageMin)
        {
          case HID_GDESKTOP_USAGE_X:
            gJOY[joyIndex].Items[JOYOFS_X].bReport       = pItem->itemFeatures.reportID;
            gJOY[joyIndex].Items[JOYOFS_X].usOffset      = usOffset;
            gJOY[joyIndex].Items[JOYOFS_X].usReportSize  = (USHORT)pItem->itemFeatures.reportSize;
            gJOY[joyIndex].AxeUnits[JOYOFS_X].logMin     = pItem->itemFeatures.logMin;
            gJOY[joyIndex].AxeUnits[JOYOFS_X].logMax     = pItem->itemFeatures.logMax;
            gJOY[joyIndex].AxeUnits[JOYOFS_X].phyMin     = pItem->itemFeatures.phyMin;
            gJOY[joyIndex].AxeUnits[JOYOFS_X].phyMax     = pItem->itemFeatures.phyMax;
            gJOY[joyIndex].AxeUnits[JOYOFS_X].unit       = pItem->itemFeatures.unit;
            gJOY[joyIndex].AxeUnits[JOYOFS_X].unitExponent = pItem->itemFeatures.unitExponent;
            gJOY[joyIndex].ulCapsAxes |=  JOYHAS_X;
            gJOY[joyIndex].DevCapsJoy.ulAxes++;
          break;
          case HID_GDESKTOP_USAGE_Y :
            gJOY[joyIndex].Items[JOYOFS_Y].bReport       = pItem->itemFeatures.reportID;
            gJOY[joyIndex].Items[JOYOFS_Y].usOffset      = usOffset;
            gJOY[joyIndex].Items[JOYOFS_Y].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
            gJOY[joyIndex].AxeUnits[JOYOFS_Y].logMin     = pItem->itemFeatures.logMin;
            gJOY[joyIndex].AxeUnits[JOYOFS_Y].logMax     = pItem->itemFeatures.logMax;
            gJOY[joyIndex].AxeUnits[JOYOFS_Y].phyMin     = pItem->itemFeatures.phyMin;
            gJOY[joyIndex].AxeUnits[JOYOFS_Y].phyMax     = pItem->itemFeatures.phyMax;
            gJOY[joyIndex].AxeUnits[JOYOFS_Y].unit       = pItem->itemFeatures.unit;
            gJOY[joyIndex].AxeUnits[JOYOFS_Y].unitExponent = pItem->itemFeatures.unitExponent;
            gJOY[joyIndex].ulCapsAxes |=  JOYHAS_Y;
            gJOY[joyIndex].DevCapsJoy.ulAxes++;
          break;
          case HID_GDESKTOP_USAGE_Z :
            gJOY[joyIndex].Items[JOYOFS_Z].bReport       = pItem->itemFeatures.reportID;
            gJOY[joyIndex].Items[JOYOFS_Z].usOffset      = usOffset;
            gJOY[joyIndex].Items[JOYOFS_Z].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
            gJOY[joyIndex].AxeUnits[JOYOFS_Z].logMin     = pItem->itemFeatures.logMin;
            gJOY[joyIndex].AxeUnits[JOYOFS_Z].logMax     = pItem->itemFeatures.logMax;
            gJOY[joyIndex].AxeUnits[JOYOFS_Z].phyMin     = pItem->itemFeatures.phyMin;
            gJOY[joyIndex].AxeUnits[JOYOFS_Z].phyMax     = pItem->itemFeatures.phyMax;
            gJOY[joyIndex].AxeUnits[JOYOFS_Z].unit       = pItem->itemFeatures.unit;
            gJOY[joyIndex].AxeUnits[JOYOFS_Z].unitExponent = pItem->itemFeatures.unitExponent;
            gJOY[joyIndex].ulCapsAxes |=  JOYHAS_Z;
            gJOY[joyIndex].DevCapsJoy.ulAxes++;
        }
        usOffset+=pItem->itemFeatures.reportSize;
        break;
      case 2:
        //  2 Assume its X and Y
        gJOY[joyIndex].Items[JOYOFS_X].bReport       = pItem->itemFeatures.reportID;
        gJOY[joyIndex].Items[JOYOFS_X].usOffset      = usOffset;
        gJOY[joyIndex].Items[JOYOFS_X].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
        gJOY[joyIndex].AxeUnits[JOYOFS_X].logMin     = pItem->itemFeatures.logMin;
        gJOY[joyIndex].AxeUnits[JOYOFS_X].logMax     = pItem->itemFeatures.logMax;
        gJOY[joyIndex].AxeUnits[JOYOFS_X].phyMin     = pItem->itemFeatures.phyMin;
        gJOY[joyIndex].AxeUnits[JOYOFS_X].phyMax     = pItem->itemFeatures.phyMax;
        gJOY[joyIndex].AxeUnits[JOYOFS_X].unit       = pItem->itemFeatures.unit;
        gJOY[joyIndex].AxeUnits[JOYOFS_X].unitExponent = pItem->itemFeatures.unitExponent;
        usOffset+=pItem->itemFeatures.reportSize;
        gJOY[joyIndex].Items[JOYOFS_Y].bReport       = pItem->itemFeatures.reportID;
        gJOY[joyIndex].Items[JOYOFS_Y].usOffset      = usOffset;
        gJOY[joyIndex].Items[JOYOFS_Y].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
        gJOY[joyIndex].AxeUnits[JOYOFS_Y].logMin     = pItem->itemFeatures.logMin;
        gJOY[joyIndex].AxeUnits[JOYOFS_Y].logMax     = pItem->itemFeatures.logMax;
        gJOY[joyIndex].AxeUnits[JOYOFS_Y].phyMin     = pItem->itemFeatures.phyMin;
        gJOY[joyIndex].AxeUnits[JOYOFS_Y].phyMax     = pItem->itemFeatures.phyMax;
        gJOY[joyIndex].AxeUnits[JOYOFS_Y].unit       = pItem->itemFeatures.unit;
        gJOY[joyIndex].AxeUnits[JOYOFS_Y].unitExponent = pItem->itemFeatures.unitExponent;
        usOffset+=pItem->itemFeatures.reportSize;
        gJOY[joyIndex].ulCapsAxes |=  JOYHAS_X | JOYHAS_Y;
        gJOY[joyIndex].DevCapsJoy.ulAxes+=2;
        break;
      case 3:
        // 3 X, Y and Z
        gJOY[joyIndex].Items[JOYOFS_X].bReport       = pItem->itemFeatures.reportID;
        gJOY[joyIndex].Items[JOYOFS_X].usOffset      = usOffset;
        gJOY[joyIndex].Items[JOYOFS_X].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
        gJOY[joyIndex].AxeUnits[JOYOFS_X].logMin     = pItem->itemFeatures.logMin;
        gJOY[joyIndex].AxeUnits[JOYOFS_X].logMax     = pItem->itemFeatures.logMax;
        gJOY[joyIndex].AxeUnits[JOYOFS_X].phyMin     = pItem->itemFeatures.phyMin;
        gJOY[joyIndex].AxeUnits[JOYOFS_X].phyMax     = pItem->itemFeatures.phyMax;
        gJOY[joyIndex].AxeUnits[JOYOFS_X].unit       = pItem->itemFeatures.unit;
        gJOY[joyIndex].AxeUnits[JOYOFS_X].unitExponent = pItem->itemFeatures.unitExponent;
        usOffset+=pItem->itemFeatures.reportSize;
        gJOY[joyIndex].Items[JOYOFS_Y].bReport       = pItem->itemFeatures.reportID;
        gJOY[joyIndex].Items[JOYOFS_Y].usOffset      = usOffset;
        gJOY[joyIndex].Items[JOYOFS_Y].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
        gJOY[joyIndex].AxeUnits[JOYOFS_Y].logMin     = pItem->itemFeatures.logMin;
        gJOY[joyIndex].AxeUnits[JOYOFS_Y].logMax     = pItem->itemFeatures.logMax;
        gJOY[joyIndex].AxeUnits[JOYOFS_Y].phyMin     = pItem->itemFeatures.phyMin;
        gJOY[joyIndex].AxeUnits[JOYOFS_Y].phyMax     = pItem->itemFeatures.phyMax;
        gJOY[joyIndex].AxeUnits[JOYOFS_Y].unit       = pItem->itemFeatures.unit;
        gJOY[joyIndex].AxeUnits[JOYOFS_Y].unitExponent = pItem->itemFeatures.unitExponent;
        usOffset+=pItem->itemFeatures.reportSize;
        gJOY[joyIndex].Items[JOYOFS_Z].bReport       = pItem->itemFeatures.reportID;
        gJOY[joyIndex].Items[JOYOFS_Z].usOffset      = usOffset;
        gJOY[joyIndex].Items[JOYOFS_Z].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
        gJOY[joyIndex].AxeUnits[JOYOFS_Z].logMin     = pItem->itemFeatures.logMin;
        gJOY[joyIndex].AxeUnits[JOYOFS_Z].logMax     = pItem->itemFeatures.logMax;
        gJOY[joyIndex].AxeUnits[JOYOFS_Z].phyMin     = pItem->itemFeatures.phyMin;
        gJOY[joyIndex].AxeUnits[JOYOFS_Z].phyMax     = pItem->itemFeatures.phyMax;
        gJOY[joyIndex].AxeUnits[JOYOFS_Z].unit       = pItem->itemFeatures.unit;
        gJOY[joyIndex].AxeUnits[JOYOFS_Z].unitExponent = pItem->itemFeatures.unitExponent;
        usOffset+=pItem->itemFeatures.reportSize;
        gJOY[joyIndex].ulCapsAxes |=  JOYHAS_X | JOYHAS_Y | JOYHAS_Z;
        gJOY[joyIndex].DevCapsJoy.ulAxes+=3;
        break;
    } // End switch(pItem->itemFeatures.reportCount)
  }
  else
  {
    // We've got a range of Axes
    if(HID_GDESKTOP_USAGE_X==pItem->localFeatures.usageMin)
    {
      gJOY[joyIndex].Items[JOYOFS_X].bReport       = pItem->itemFeatures.reportID;
      gJOY[joyIndex].Items[JOYOFS_X].usOffset      = usOffset;
      gJOY[joyIndex].Items[JOYOFS_X].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
      gJOY[joyIndex].ulCapsAxes |=  JOYHAS_X;
      gJOY[joyIndex].AxeUnits[JOYOFS_X].logMin     = pItem->itemFeatures.logMin;
      gJOY[joyIndex].AxeUnits[JOYOFS_X].logMax     = pItem->itemFeatures.logMax;
      gJOY[joyIndex].AxeUnits[JOYOFS_X].phyMin     = pItem->itemFeatures.phyMin;
      gJOY[joyIndex].AxeUnits[JOYOFS_X].phyMax     = pItem->itemFeatures.phyMax;
      gJOY[joyIndex].AxeUnits[JOYOFS_X].unit       = pItem->itemFeatures.unit;
      gJOY[joyIndex].AxeUnits[JOYOFS_X].unitExponent = pItem->itemFeatures.unitExponent;
      usOffset += pItem->itemFeatures.reportSize;
      gJOY[joyIndex].DevCapsJoy.ulAxes++;
    }

    gJOY[joyIndex].Items[JOYOFS_Y].bReport       = pItem->itemFeatures.reportID;
    gJOY[joyIndex].Items[JOYOFS_Y].usOffset      = usOffset;
    gJOY[joyIndex].Items[JOYOFS_Y].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
    gJOY[joyIndex].AxeUnits[JOYOFS_Y].logMin     = pItem->itemFeatures.logMin;
    gJOY[joyIndex].AxeUnits[JOYOFS_Y].logMax     = pItem->itemFeatures.logMax;
    gJOY[joyIndex].AxeUnits[JOYOFS_Y].phyMin     = pItem->itemFeatures.phyMin;
    gJOY[joyIndex].AxeUnits[JOYOFS_Y].phyMax     = pItem->itemFeatures.phyMax;
    gJOY[joyIndex].AxeUnits[JOYOFS_Y].unit       = pItem->itemFeatures.unit;
    gJOY[joyIndex].AxeUnits[JOYOFS_Y].unitExponent = pItem->itemFeatures.unitExponent;
    gJOY[joyIndex].ulCapsAxes |=  JOYHAS_Y;
    usOffset += pItem->itemFeatures.reportSize;
    gJOY[joyIndex].DevCapsJoy.ulAxes++;

    if( HID_GDESKTOP_USAGE_Z==pItem->localFeatures.usageMax)
    {
      gJOY[joyIndex].Items[JOYOFS_Z].bReport       = pItem->itemFeatures.reportID;
      gJOY[joyIndex].Items[JOYOFS_Z].usOffset      = usOffset;
      gJOY[joyIndex].Items[JOYOFS_Z].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
      gJOY[joyIndex].ulCapsAxes |=  JOYHAS_Z;
      gJOY[joyIndex].AxeUnits[JOYOFS_Z].logMin     = pItem->itemFeatures.logMin;
      gJOY[joyIndex].AxeUnits[JOYOFS_Z].logMax     = pItem->itemFeatures.logMax;
      gJOY[joyIndex].AxeUnits[JOYOFS_Z].phyMin     = pItem->itemFeatures.phyMin;
      gJOY[joyIndex].AxeUnits[JOYOFS_Z].phyMax     = pItem->itemFeatures.phyMax;
      gJOY[joyIndex].AxeUnits[JOYOFS_Z].unit       = pItem->itemFeatures.unit;
      gJOY[joyIndex].AxeUnits[JOYOFS_Z].unitExponent = pItem->itemFeatures.unitExponent;
      usOffset += pItem->itemFeatures.reportSize;
      gJOY[joyIndex].DevCapsJoy.ulAxes++;
    }
  }
  return usOffset;
}

USHORT SetupRotationAxes(USHORT joyIndex, ReportItemData FAR *pItem, USHORT usOffset)
{

  ULONG ulFlags[] ={JOYHAS_RX,JOYHAS_RY,JOYHAS_RZ};
  if(pItem->localFeatures.usageMin == pItem->localFeatures.usageMax)
  {
    gJOY[joyIndex].Items[pItem->localFeatures.usageMin - HID_GDESKTOP_USAGE_RX + JOYOFS_RX].bReport      =
      pItem->itemFeatures.reportID;
    gJOY[joyIndex].Items[pItem->localFeatures.usageMin - HID_GDESKTOP_USAGE_RX + JOYOFS_RX].usOffset     =
      usOffset;
    gJOY[joyIndex].Items[pItem->localFeatures.usageMin - HID_GDESKTOP_USAGE_RX + JOYOFS_RX].usReportSize =
      (USHORT)pItem->itemFeatures.reportSize;
    gJOY[joyIndex].AxeUnits[pItem->localFeatures.usageMin - HID_GDESKTOP_USAGE_RX + JOYOFS_RX].logMin    =
      pItem->itemFeatures.logMin;
    gJOY[joyIndex].AxeUnits[pItem->localFeatures.usageMin - HID_GDESKTOP_USAGE_RX + JOYOFS_RX].logMax    =
      pItem->itemFeatures.logMax;
    gJOY[joyIndex].AxeUnits[pItem->localFeatures.usageMin - HID_GDESKTOP_USAGE_RX + JOYOFS_RX].phyMin    =
      pItem->itemFeatures.phyMin;
    gJOY[joyIndex].AxeUnits[pItem->localFeatures.usageMin - HID_GDESKTOP_USAGE_RX + JOYOFS_RX].phyMax    =
      pItem->itemFeatures.phyMax;
    gJOY[joyIndex].AxeUnits[pItem->localFeatures.usageMin - HID_GDESKTOP_USAGE_RX + JOYOFS_RX].unit      =
      pItem->itemFeatures.unit;
    gJOY[joyIndex].AxeUnits[pItem->localFeatures.usageMin - HID_GDESKTOP_USAGE_RX + JOYOFS_RX].unitExponent =
      pItem->itemFeatures.unitExponent;
    gJOY[joyIndex].ulCapsAxes |=  ulFlags[pItem->localFeatures.usageMin-HID_GDESKTOP_USAGE_RX];
    usOffset += pItem->itemFeatures.reportSize;
    gJOY[joyIndex].DevCapsJoy.ulAxes++;
  }
  else
  {
    if(HID_GDESKTOP_USAGE_RX==pItem->localFeatures.usageMin)
    {
      gJOY[joyIndex].Items[JOYOFS_RX].bReport       = pItem->itemFeatures.reportID;
      gJOY[joyIndex].Items[JOYOFS_RX].usOffset      = usOffset;
      gJOY[joyIndex].Items[JOYOFS_RX].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
      gJOY[joyIndex].AxeUnits[JOYOFS_RX].logMin     = pItem->itemFeatures.logMin;
      gJOY[joyIndex].AxeUnits[JOYOFS_RX].logMax     = pItem->itemFeatures.logMax;
      gJOY[joyIndex].AxeUnits[JOYOFS_RX].phyMin     = pItem->itemFeatures.phyMin;
      gJOY[joyIndex].AxeUnits[JOYOFS_RX].phyMax     = pItem->itemFeatures.phyMax;
      gJOY[joyIndex].AxeUnits[JOYOFS_RX].unit       = pItem->itemFeatures.unit;
      gJOY[joyIndex].AxeUnits[JOYOFS_RX].unitExponent = pItem->itemFeatures.unitExponent;
      gJOY[joyIndex].ulCapsAxes |=  JOYHAS_RX;
      usOffset += pItem->itemFeatures.reportSize;
      gJOY[joyIndex].DevCapsJoy.ulAxes++;
    }

    gJOY[joyIndex].Items[JOYOFS_RY].bReport       = pItem->itemFeatures.reportID;
    gJOY[joyIndex].Items[JOYOFS_RY].usOffset      = usOffset;
    gJOY[joyIndex].Items[JOYOFS_RY].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
    gJOY[joyIndex].AxeUnits[JOYOFS_RY].logMin     = pItem->itemFeatures.logMin;
    gJOY[joyIndex].AxeUnits[JOYOFS_RY].logMax     = pItem->itemFeatures.logMax;
    gJOY[joyIndex].AxeUnits[JOYOFS_RY].phyMin     = pItem->itemFeatures.phyMin;
    gJOY[joyIndex].AxeUnits[JOYOFS_RY].phyMax     = pItem->itemFeatures.phyMax;
    gJOY[joyIndex].AxeUnits[JOYOFS_RY].unit       = pItem->itemFeatures.unit;
    gJOY[joyIndex].AxeUnits[JOYOFS_RY].unitExponent = pItem->itemFeatures.unitExponent;
    gJOY[joyIndex].ulCapsAxes |=  JOYHAS_RY;
    usOffset += pItem->itemFeatures.reportSize;
    gJOY[joyIndex].DevCapsJoy.ulAxes++;

    if( HID_GDESKTOP_USAGE_RZ==pItem->localFeatures.usageMax)
    {
      gJOY[joyIndex].Items[JOYOFS_RZ].bReport       = pItem->itemFeatures.reportID;
      gJOY[joyIndex].Items[JOYOFS_RZ].usOffset      = usOffset;
      gJOY[joyIndex].Items[JOYOFS_RZ].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
      gJOY[joyIndex].AxeUnits[JOYOFS_RZ].logMin     = pItem->itemFeatures.logMin;
      gJOY[joyIndex].AxeUnits[JOYOFS_RZ].logMax     = pItem->itemFeatures.logMax;
      gJOY[joyIndex].AxeUnits[JOYOFS_RZ].phyMin     = pItem->itemFeatures.phyMin;
      gJOY[joyIndex].AxeUnits[JOYOFS_RZ].phyMax     = pItem->itemFeatures.phyMax;
      gJOY[joyIndex].AxeUnits[JOYOFS_RZ].unit       = pItem->itemFeatures.unit;
      gJOY[joyIndex].AxeUnits[JOYOFS_RZ].unitExponent = pItem->itemFeatures.unitExponent;
      gJOY[joyIndex].ulCapsAxes |=  JOYHAS_RZ;
      usOffset += pItem->itemFeatures.reportSize;
      gJOY[joyIndex].DevCapsJoy.ulAxes++;
    }
  }
  return usOffset;
}

USHORT SetupXYZAxesSpeed(USHORT joyIndex, ReportItemData FAR *pItem, USHORT usOffset)
{
  ULONG ulFlags[] ={JOYHAS_VX, JOYHAS_VY, JOYHAS_VZ};
  if(pItem->localFeatures.usageMin == pItem->localFeatures.usageMax)
  {
    gJOY[joyIndex].Items[pItem->localFeatures.usageMin - HID_GDESKTOP_USAGE_VX + JOYOFS_VX].bReport      =
      pItem->itemFeatures.reportID;
    gJOY[joyIndex].Items[pItem->localFeatures.usageMin - HID_GDESKTOP_USAGE_VX + JOYOFS_VX].usOffset     =
      usOffset;
    gJOY[joyIndex].Items[pItem->localFeatures.usageMin - HID_GDESKTOP_USAGE_VX + JOYOFS_VX].usReportSize =
      (USHORT)pItem->itemFeatures.reportSize;
    gJOY[joyIndex].ulCapsAxes |= ulFlags[pItem->localFeatures.usageMin-HID_GDESKTOP_USAGE_VX];
    usOffset += pItem->itemFeatures.reportSize;
  }
  else
  {
    if(HID_GDESKTOP_USAGE_VX==pItem->localFeatures.usageMin)
    {
      gJOY[joyIndex].Items[JOYOFS_VX].bReport      = pItem->itemFeatures.reportID;
      gJOY[joyIndex].Items[JOYOFS_VX].usOffset     = usOffset;
      gJOY[joyIndex].Items[JOYOFS_VX].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
      gJOY[joyIndex].ulCapsAxes |=  JOYHAS_VX;
      usOffset += pItem->itemFeatures.reportSize;
    }

    gJOY[joyIndex].Items[JOYOFS_VY].bReport      = pItem->itemFeatures.reportID;
    gJOY[joyIndex].Items[JOYOFS_VY].usOffset     = usOffset;
    gJOY[joyIndex].Items[JOYOFS_VY].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
    gJOY[joyIndex].ulCapsAxes |=  JOYHAS_VY;
    usOffset += pItem->itemFeatures.reportSize;

    if( HID_GDESKTOP_USAGE_VZ==pItem->localFeatures.usageMax)
    {
      gJOY[joyIndex].Items[JOYOFS_VZ].bReport      = pItem->itemFeatures.reportID;
      gJOY[joyIndex].Items[JOYOFS_VZ].usOffset     = usOffset;
      gJOY[joyIndex].Items[JOYOFS_VZ].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
      gJOY[joyIndex].ulCapsAxes |=  JOYHAS_VZ;
      usOffset += pItem->itemFeatures.reportSize;
    }
  }
  return usOffset;
}

USHORT SetupRotaionAxesSpeed(USHORT joyIndex, ReportItemData FAR *pItem, USHORT usOffset)
{
  ULONG ulFlags[] = {JOYHAS_VRX, JOYHAS_VRY, JOYHAS_VRZ};
  if(pItem->localFeatures.usageMin == pItem->localFeatures.usageMax)
  {
    gJOY[joyIndex].Items[pItem->localFeatures.usageMin - HID_GDESKTOP_USAGE_VBRX + JOYOFS_RX].bReport      =
      pItem->itemFeatures.reportID;
    gJOY[joyIndex].Items[pItem->localFeatures.usageMin - HID_GDESKTOP_USAGE_VBRX + JOYOFS_RX].usOffset     =
      usOffset;
    gJOY[joyIndex].Items[pItem->localFeatures.usageMin - HID_GDESKTOP_USAGE_VBRX + JOYOFS_RX].usReportSize =
      (USHORT)pItem->itemFeatures.reportSize;
    gJOY[joyIndex].ulCapsAxes |= ulFlags[pItem->localFeatures.usageMin-HID_GDESKTOP_USAGE_VBRX];
    usOffset += pItem->itemFeatures.reportSize;
  }
  else
  {
    if(HID_GDESKTOP_USAGE_VBRX==pItem->localFeatures.usageMin)
    {
      gJOY[joyIndex].Items[JOYOFS_VRX].bReport      = pItem->itemFeatures.reportID;
      gJOY[joyIndex].Items[JOYOFS_VRX].usOffset     = usOffset;
      gJOY[joyIndex].Items[JOYOFS_VRX].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
      gJOY[joyIndex].ulCapsAxes |=  JOYHAS_VRX;
      usOffset += pItem->itemFeatures.reportSize;
    }

    gJOY[joyIndex].Items[JOYOFS_VRY].bReport      = pItem->itemFeatures.reportID;
    gJOY[joyIndex].Items[JOYOFS_VRY].usOffset     = usOffset;
    gJOY[joyIndex].Items[JOYOFS_VRY].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
    gJOY[joyIndex].ulCapsAxes |=  JOYHAS_VRY;
    usOffset += pItem->itemFeatures.reportSize;

    if( HID_GDESKTOP_USAGE_VBRZ==pItem->localFeatures.usageMax)
    {
      gJOY[joyIndex].Items[JOYOFS_VRZ].bReport      = pItem->itemFeatures.reportID;
      gJOY[joyIndex].Items[JOYOFS_VRZ].usOffset     = usOffset;
      gJOY[joyIndex].Items[JOYOFS_VRZ].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
      gJOY[joyIndex].ulCapsAxes |=  JOYHAS_VRZ;
      usOffset += pItem->itemFeatures.reportSize;
    }
  }
  return usOffset;
}

USHORT SetupSliders(USHORT joyIndex, ReportItemData FAR *pItem, USHORT usOffset)
{
  USHORT usCount;
  usCount = 0;
  while( usCount < (USHORT)pItem->itemFeatures.reportCount &&
         gJOY[joyIndex].DevCapsJoy.ulSliders < MAX_SLIDERS)
  {
    gJOY[joyIndex].Items[JOYOFS_SLIDER0+usCount].bReport       = pItem->itemFeatures.reportID;
    gJOY[joyIndex].Items[JOYOFS_SLIDER0+usCount].usOffset      = usOffset;
    gJOY[joyIndex].Items[JOYOFS_SLIDER0+usCount].usReportSize = (USHORT)pItem->itemFeatures.reportSize;
    gJOY[joyIndex].DevCapsJoy.ulSliders++;
    gJOY[joyIndex].AxeUnits[JOYOFS_SLIDER0+usCount].logMin     = pItem->itemFeatures.logMin;
    gJOY[joyIndex].AxeUnits[JOYOFS_SLIDER0+usCount].logMax     = pItem->itemFeatures.logMax;
    gJOY[joyIndex].AxeUnits[JOYOFS_SLIDER0+usCount].phyMin     = pItem->itemFeatures.phyMin;
    gJOY[joyIndex].AxeUnits[JOYOFS_SLIDER0+usCount].phyMax     = pItem->itemFeatures.phyMax;
    gJOY[joyIndex].AxeUnits[JOYOFS_SLIDER0+usCount].unit       = pItem->itemFeatures.unit;
    gJOY[joyIndex].AxeUnits[JOYOFS_SLIDER0+usCount].unitExponent = pItem->itemFeatures.unitExponent;

    usOffset += pItem->itemFeatures.reportSize;
    usCount++;
  }
  // Just in case the device has more than MAX_SLIDERS sliders
  usOffset += pItem->itemFeatures.reportSize  * (pItem->itemFeatures.reportCount- usCount);

  return usOffset;
}

USHORT SetupPOVs(USHORT joyIndex, ReportItemData FAR *pItem, USHORT usOffset)
{
  USHORT usCount;
  usCount = 0;
  while( usCount < (USHORT)pItem->itemFeatures.reportCount &&
         gJOY[joyIndex].DevCapsJoy.ulPOVs < MAX_POVS)
  {
    gJOY[joyIndex].Items[JOYOFS_POV0+usCount].bReport         = pItem->itemFeatures.reportID;
    gJOY[joyIndex].Items[JOYOFS_POV0+usCount].usOffset        = usOffset;
    gJOY[joyIndex].Items[JOYOFS_POV0+usCount].usReportSize    = (USHORT)pItem->itemFeatures.reportSize;
    gJOY[joyIndex].AxeUnits[JOYOFS_POV0+usCount].logMin       = pItem->itemFeatures.logMin;
    gJOY[joyIndex].AxeUnits[JOYOFS_POV0+usCount].logMax       = pItem->itemFeatures.logMax;
    gJOY[joyIndex].AxeUnits[JOYOFS_POV0+usCount].phyMin       = pItem->itemFeatures.phyMin;
    gJOY[joyIndex].AxeUnits[JOYOFS_POV0+usCount].phyMax       = pItem->itemFeatures.phyMax;
    gJOY[joyIndex].AxeUnits[JOYOFS_POV0+usCount].unit         = pItem->itemFeatures.unit;
    gJOY[joyIndex].AxeUnits[JOYOFS_POV0+usCount].unitExponent = pItem->itemFeatures.unitExponent;
    gJOY[joyIndex].DevCapsJoy.ulPOVs++;
    usOffset += pItem->itemFeatures.reportSize;
    usCount++;
  }
  // Just in case the device has more than MAX_POVS Hatswitches
  usOffset += pItem->itemFeatures.reportSize  * (pItem->itemFeatures.reportCount- usCount);

  return usOffset;
}

USHORT SetupButtons(USHORT joyIndex, ReportItemData FAR *pItem, USHORT usOffset)
{
  USHORT usCount;
  usCount = 0;
  while( usCount < (USHORT)pItem->itemFeatures.reportCount &&
         gJOY[joyIndex].DevCapsJoy.ulButtons < MAX_BUTTONS)
  {
    gJOY[joyIndex].Items[JOYOFS_BUTTON0+gJOY[joyIndex].DevCapsJoy.ulButtons].bReport      =
      pItem->itemFeatures.reportID;
    gJOY[joyIndex].Items[JOYOFS_BUTTON0+gJOY[joyIndex].DevCapsJoy.ulButtons].usOffset     =
      usOffset;
    gJOY[joyIndex].Items[JOYOFS_BUTTON0+gJOY[joyIndex].DevCapsJoy.ulButtons].usReportSize =
      (USHORT)pItem->itemFeatures.reportSize;
    gJOY[joyIndex].DevCapsJoy.ulButtons++;
    usOffset += pItem->itemFeatures.reportSize;
    usCount++;
  }
  // Just in case the device has more than MAX_BUTTONS buttons
  usOffset += pItem->itemFeatures.reportSize  * (pItem->itemFeatures.reportCount- usCount);
  return usOffset;
}

#ifdef DEBUG
  void PrintJoyReport(USHORT joyIndex)
  {
    USHORT usCount;

    dsPrint ( DBG_DETAILED, "USBJOY: JOYserv, Device Destription:\r\n  Axes:\r");
    if(gJOY[joyIndex].ulCapsAxes & JOYHAS_X)
    {
      dsPrint3 (DBG_DETAILED, "  - X Present In Report %d at Offset %d Size %d\r\n",
                gJOY[joyIndex].Items[JOYOFS_X].bReport,
                gJOY[joyIndex].Items[JOYOFS_X].usOffset,
                gJOY[joyIndex].Items[JOYOFS_X].usReportSize
                );
      dsPrint4 (DBG_DETAILED, "     Logical (min/max) : %d/%d\r\n     Physical (min/max): %d/%d\r\n",
                      gJOY[joyIndex].AxeUnits[JOYOFS_X].logMin,
                      gJOY[joyIndex].AxeUnits[JOYOFS_X].logMax,
                      gJOY[joyIndex].AxeUnits[JOYOFS_X].phyMin,
                      gJOY[joyIndex].AxeUnits[JOYOFS_X].phyMax);
    }
    else
      dsPrint ( DBG_DETAILED, "  -X Not Present\r\n");

    if(gJOY[joyIndex].ulCapsAxes & JOYHAS_Y)
    {
      dsPrint3 (DBG_DETAILED, "  - Y Present In Report %d at Offset %d Size %d\r\n",
                gJOY[joyIndex].Items[JOYOFS_Y].bReport,
                gJOY[joyIndex].Items[JOYOFS_Y].usOffset,
                gJOY[joyIndex].Items[JOYOFS_Y].usReportSize
                );
      dsPrint4 (DBG_DETAILED, "     Logical (min/max) : %d/%d\r\n     Physical (min/max): %d/%d\r\n",
                      gJOY[joyIndex].AxeUnits[JOYOFS_Y].logMin,
                      gJOY[joyIndex].AxeUnits[JOYOFS_Y].logMax,
                      gJOY[joyIndex].AxeUnits[JOYOFS_Y].phyMin,
                      gJOY[joyIndex].AxeUnits[JOYOFS_Y].phyMax);
    }
    else
      dsPrint ( DBG_DETAILED, "  -Y Not Present\r\n");

    if(gJOY[joyIndex].ulCapsAxes & JOYHAS_Z)
    {
      dsPrint3 (DBG_DETAILED, "  - Z Present In Report %d at Offset %d Size %d\r\n",
                gJOY[joyIndex].Items[JOYOFS_Z].bReport,
                gJOY[joyIndex].Items[JOYOFS_Z].usOffset,
                gJOY[joyIndex].Items[JOYOFS_Z].usReportSize
                );
      dsPrint4 (DBG_DETAILED, "     Logical (min/max) : %d/%d\r\n     Physical (min/max): %d/%d\r\n",
                      gJOY[joyIndex].AxeUnits[JOYOFS_Z].logMin,
                      gJOY[joyIndex].AxeUnits[JOYOFS_Z].logMax,
                      gJOY[joyIndex].AxeUnits[JOYOFS_Z].phyMin,
                      gJOY[joyIndex].AxeUnits[JOYOFS_Z].phyMax);
    }
    else
      dsPrint ( DBG_DETAILED, "  -Z Not Present\r\n");

    if(gJOY[joyIndex].ulCapsAxes & JOYHAS_RX)
    {
      dsPrint3 (DBG_DETAILED, "  - RX Present In Report %d at Offset %d Size %d\r\n",
                gJOY[joyIndex].Items[JOYOFS_RX].bReport,
                gJOY[joyIndex].Items[JOYOFS_RX].usOffset,
                gJOY[joyIndex].Items[JOYOFS_RX].usReportSize
                );
      dsPrint4 (DBG_DETAILED, "     Logical (min/max) : %d/%d\r\n     Physical (min/max): %d/%d\r\n",
                      gJOY[joyIndex].AxeUnits[JOYOFS_RX].logMin,
                      gJOY[joyIndex].AxeUnits[JOYOFS_RX].logMax,
                      gJOY[joyIndex].AxeUnits[JOYOFS_RX].phyMin,
                      gJOY[joyIndex].AxeUnits[JOYOFS_RX].phyMax);
    }
    else
      dsPrint ( DBG_DETAILED, "  -RX Not Present\r\n");

    if(gJOY[joyIndex].ulCapsAxes & JOYHAS_RY)
    {
      dsPrint3 (DBG_DETAILED, "  - RY Present In Report %d at Offset %d Size %d\r\n",
                gJOY[joyIndex].Items[JOYOFS_RY].bReport,
                gJOY[joyIndex].Items[JOYOFS_RY].usOffset,
                gJOY[joyIndex].Items[JOYOFS_RY].usReportSize
                );
      dsPrint4 (DBG_DETAILED, "     Logical (min/max) : %d/%d\r\n     Physical (min/max): %d/%d\r\n",
                      gJOY[joyIndex].AxeUnits[JOYOFS_RY].logMin,
                      gJOY[joyIndex].AxeUnits[JOYOFS_RY].logMax,
                      gJOY[joyIndex].AxeUnits[JOYOFS_RY].phyMin,
                      gJOY[joyIndex].AxeUnits[JOYOFS_RY].phyMax);
    }
    else
      dsPrint ( DBG_DETAILED, "  -RY Not Present\r\n");

    if(gJOY[joyIndex].ulCapsAxes & JOYHAS_RZ)
    {
      dsPrint3 (DBG_DETAILED, "  - RZ Present In Report %d at Offset %d Size %d\r\n",
                gJOY[joyIndex].Items[JOYOFS_RZ].bReport,
                gJOY[joyIndex].Items[JOYOFS_RZ].usOffset,
                gJOY[joyIndex].Items[JOYOFS_RZ].usReportSize
                );
      dsPrint4 (DBG_DETAILED, "     Logical (min/max) : %d/%d\r\n     Physical (min/max): %d/%d\r\n",
                gJOY[joyIndex].AxeUnits[JOYOFS_RZ].logMin,
                gJOY[joyIndex].AxeUnits[JOYOFS_RZ].logMax,
                gJOY[joyIndex].AxeUnits[JOYOFS_RZ].phyMin,
                gJOY[joyIndex].AxeUnits[JOYOFS_RZ].phyMax);
    }
    else
      dsPrint ( DBG_DETAILED, "  -RZ Not Present\r\n");

    dsPrint ( DBG_DETAILED, "\r\n  Sliders:\r\n");
    if(gJOY[joyIndex].DevCapsJoy.ulSliders!=0)
    {
      usCount =0;
      while((ULONG)usCount<gJOY[joyIndex].DevCapsJoy.ulSliders)
      {
        dsPrint4 (DBG_DETAILED, "  - Slider %d Present In Report %d at Offset %d Size %d\r\n",
                  usCount,
                  gJOY[joyIndex].Items[JOYOFS_SLIDER0+usCount].bReport,
                  gJOY[joyIndex].Items[JOYOFS_SLIDER0+usCount].usOffset,
                  gJOY[joyIndex].Items[JOYOFS_SLIDER0+usCount].usReportSize
                  );
        dsPrint4 (DBG_DETAILED, "     Logical (min/max) : %d/%d\r\n     Physical (min/max): %d/%d\r\n",
                        gJOY[joyIndex].AxeUnits[JOYOFS_SLIDER0+usCount].logMin,
                        gJOY[joyIndex].AxeUnits[JOYOFS_SLIDER0+usCount].logMax,
                        gJOY[joyIndex].AxeUnits[JOYOFS_SLIDER0+usCount].phyMin,
                        gJOY[joyIndex].AxeUnits[JOYOFS_SLIDER0+usCount].phyMax);
        usCount++;
      }
    }
    else
      dsPrint ( DBG_DETAILED, "  - NONE Present\r\n");

    dsPrint ( DBG_DETAILED, "\r\n  POVs:\r\n");
    if(gJOY[joyIndex].DevCapsJoy.ulPOVs!=0)
    {
      usCount =0;
      while((ULONG)usCount<gJOY[joyIndex].DevCapsJoy.ulPOVs)
      {
        dsPrint4 (DBG_DETAILED, "  - POV %d Present In Report %d at Offset %d Size %d\r\n",
                  usCount,
                  gJOY[joyIndex].Items[JOYOFS_POV0+usCount].bReport,
                  gJOY[joyIndex].Items[JOYOFS_POV0+usCount].usOffset,
                  gJOY[joyIndex].Items[JOYOFS_POV0+usCount].usReportSize
                  );
        dsPrint4 (DBG_DETAILED, "     Logical (min/max) : %d/%d\r\n     Physical (min/max): %d/%d\r\n",
                        gJOY[joyIndex].AxeUnits[JOYOFS_POV0+usCount].logMin,
                        gJOY[joyIndex].AxeUnits[JOYOFS_POV0+usCount].logMax,
                        gJOY[joyIndex].AxeUnits[JOYOFS_POV0+usCount].phyMin,
                        gJOY[joyIndex].AxeUnits[JOYOFS_POV0+usCount].phyMax);
        usCount++;
      }
    }
    else
      dsPrint ( DBG_DETAILED, "  - NONE Present\r\n");

    dsPrint ( DBG_DETAILED, "\r\n  Buttons:\r\n");
    if(gJOY[joyIndex].DevCapsJoy.ulButtons!=0)
    {
      usCount =0;
      while((ULONG)usCount<gJOY[joyIndex].DevCapsJoy.ulButtons)
      {
        dsPrint4 (DBG_DETAILED, "  - Button %d Present In Report %d at Offset %d Size %d\r\n",
                  usCount,
                  gJOY[joyIndex].Items[JOYOFS_BUTTON0+usCount].bReport,
                  gJOY[joyIndex].Items[JOYOFS_BUTTON0+usCount].usOffset,
                  gJOY[joyIndex].Items[JOYOFS_BUTTON0+usCount].usReportSize
                  );
        usCount++;
      }
    }
    else
      dsPrint ( DBG_DETAILED, "  - NONE Present\r\n");

  }
#endif

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME:  JOYserv                                          */
/*                                                                    */
/* DESCRIPTIVE NAME:  USB Joystick service check                      */
/*                                                                    */
/* FUNCTION:  This routine is used to deterime whether the            */
/*            USB Joystick driver can service a specified device.     */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT :  JOYserv                                             */
/*    LINKAGE  :                                                      */
/*                                                                    */
/* INPUT:  RP_GENIOCTL FAR *pRP_GENIOCTL                              */
/*                                                                    */
/* EXIT-NORMAL:  JOY IDC RC OK                                        */
/*                                                                    */
/* EXIT-ERROR:  device driver error code                              */
/*                                                                    */
/* EFFECTS:  None                                                     */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void JOYserv (RP_GENIOCTL FAR *pRP_GENIOCTL)
{
   USHORT              index, joyIndex;
   USBHIDServe    FAR *pServData;
   ReportItemData FAR *pItem;
   USHORT         usOffSet;
   // Check for free entry

   if (gNoOfJOYs < MAX_JOYS)
   {
      for (joyIndex = 0; joyIndex < MAX_JOYS; joyIndex++)
        if (!gJOY[joyIndex].active)
          break;
   }
   else
   {
      pRP_GENIOCTL->rph.Status = USB_IDC_RC_SERVREJCTD;

      #ifdef DEBUG
        dsPrint2 (DBG_CRITICAL, "USBJOY: JOYserv, gNoOfJOYs = %d, Status = 0x%x\r\n",
                  gNoOfJOYs, pRP_GENIOCTL->rph.Status);
      #endif

      return;
   }

   #ifdef DEBUG
     dsPrint1 (DBG_DETAILED, "USBJOY: JOYserv, joyIndex = %d\r\n", joyIndex);
   #endif

   pServData = (USBHIDServe FAR *)pRP_GENIOCTL->ParmPacket;

   // Check if Usage is Joystick

   index = pServData->reportItemIndex;
   while (index != LAST_INDEX)
   {
      pItem = pServData->itemData + index;

      if ( pItem->mainType == HID_REPORT_TAGS_MAIN_COLL &&
           pItem->itemFeatures.usagePage == HID_USAGE_PAGE_GDESKTOP &&
           pItem->localFeatures.usageMin == HID_GDESKTOP_USAGE_JOYSTICK &&
           pItem->localFeatures.usageMax == HID_GDESKTOP_USAGE_JOYSTICK )
      {
         break;
      }
      index = pItem->indexToNextItem;
   }

   if (index == LAST_INDEX)
   {
      pRP_GENIOCTL->rph.Status = USB_IDC_RC_SERVREJCTD;
      #ifdef DEBUG
        dsPrint1 ( DBG_CRITICAL, "USBJOY: JOYserv, No Joystick Usage, Status = 0x%x\r\n",
                   pRP_GENIOCTL->rph.Status);
      #endif
      return;
   }

   // Check if the total report Length of the device can be handled
   gJOY[joyIndex].ReportLength = 0;
   index = pServData->reportItemIndex;
   while (index != LAST_INDEX)
   {

      pItem = pServData->itemData + index;
      gJOY[joyIndex].ReportLength += pItem->itemFeatures.reportSize*
                                     pItem->itemFeatures.reportCount;
      index = pItem->indexToNextItem;
   }

   gJOY[joyIndex].ReportLength = (gJOY[joyIndex].ReportLength +BITS_IN_BYTE-1)/BITS_IN_BYTE;

   if(gJOY[joyIndex].ReportLength > sizeof(gJOY[joyIndex].buffer))
   {
      pRP_GENIOCTL->rph.Status = USB_IDC_RC_SERVREJCTD;

      #ifdef DEBUG
        dsPrint3 ( DBG_CRITICAL, "USBJOY: JOYserv, length of Report (%d) > Size of Buffer (%d), Status = 0x%x\r\n",
                   gJOY[joyIndex].ReportLength ,
                   sizeof(gJOY[joyIndex].buffer),
                   pRP_GENIOCTL->rph.Status);
      #endif

      return;
   }
   dsPrint1( DBG_DETAILED, "USBJOY: JOYserv, Total ReportLength (%d)\r\n",
             gJOY[joyIndex].ReportLength);

   // Parse Report for needed infos

   index = pServData->reportItemIndex;
   usOffSet = 0;
   gJOY[joyIndex].ulCapsAxes = 0;
   gJOY[joyIndex].ulCapsSliders = 0;
   setmem((PSZ)&gJOY[joyIndex].DevCapsJoy, 0, sizeof(DEVCAPS));
   setmem((PSZ)&gJOY[joyIndex].joyState, 0, sizeof(JOYSTATE));
   setmem((PSZ)&gJOY[joyIndex].AxeUnits, 0, sizeof(JOYAXEUNIT)*JOYMAX_AXES);
   setmem((PSZ)&gJOY[joyIndex].Items, FULL_BYTE, sizeof(JOYITEM)*JOYMAXITEMS);
   while (index != LAST_INDEX)
   {
      pItem = pServData->itemData + index;


      if ( pItem->mainType == HID_REPORT_TAGS_MAIN_INPUT &&
           pItem->itemFeatures.usagePage == HID_USAGE_PAGE_GDESKTOP)
      {
        if(pItem->localFeatures.usageMin >= HID_GDESKTOP_USAGE_X &&
           pItem->localFeatures.usageMax <= HID_GDESKTOP_USAGE_Z )
        {
          usOffSet = SetupXYZAxes(joyIndex, pItem, usOffSet);
          gJOY[joyIndex].inInterface = pItem->interface;
        }
        else
        {
          if( pItem->localFeatures.usageMin >= HID_GDESKTOP_USAGE_RX &&
              pItem->localFeatures.usageMax <= HID_GDESKTOP_USAGE_RZ)
          {
            usOffSet = SetupRotationAxes(joyIndex, pItem, usOffSet);
          }
          else
          {
            if( pItem->localFeatures.usageMin >= HID_GDESKTOP_USAGE_VX &&
                pItem->localFeatures.usageMax <= HID_GDESKTOP_USAGE_VZ)
            {
              usOffSet = SetupXYZAxesSpeed(joyIndex, pItem, usOffSet);
            }
            else
            {
              if( pItem->localFeatures.usageMin >= HID_GDESKTOP_USAGE_VBRX &&
                  pItem->localFeatures.usageMax <= HID_GDESKTOP_USAGE_VBRZ)
              {
                usOffSet = SetupRotaionAxesSpeed(joyIndex, pItem, usOffSet);
              }
              else
              {
                if( pItem->localFeatures.usageMin == HID_GDESKTOP_USAGE_SLIDER &&
                    pItem->localFeatures.usageMax == HID_GDESKTOP_USAGE_SLIDER)
                {
                  usOffSet = SetupSliders(joyIndex, pItem, usOffSet);
                }
                else
                {
                  if( pItem->localFeatures.usageMin == HID_GDESKTOP_USAGE_HATSWITCH &&
                      pItem->localFeatures.usageMax == HID_GDESKTOP_USAGE_HATSWITCH)
                  {
                    usOffSet = SetupPOVs(joyIndex, pItem, usOffSet);
                  }
                  else
                  {
                    usOffSet += pItem->itemFeatures.reportSize  * pItem->itemFeatures.reportCount;
                  }
                }
              }
            }
          }
        }
      }
      else
      {
        if (pItem->mainType == HID_REPORT_TAGS_MAIN_INPUT &&
            pItem->itemFeatures.usagePage == HID_USAGE_PAGE_BUTTON)
        {
          usOffSet = SetupButtons(joyIndex, pItem, usOffSet);
        }
        else
          usOffSet += pItem->itemFeatures.reportSize  * pItem->itemFeatures.reportCount;

      }
      index = pItem->indexToNextItem;

   }

   if ( (ULONG)0==gJOY[joyIndex].DevCapsJoy.ulButtons ||
        (ULONG)0==gJOY[joyIndex].DevCapsJoy.ulAxes)
   {
      pRP_GENIOCTL->rph.Status = USB_IDC_RC_SERVREJCTD;
      #ifdef DEBUG
        dsPrint3 ( DBG_CRITICAL, "USBJOY: JOYserv, Error # Axes (%d) or # Buttons(%d) is zero, Status = 0x%x\r\n",
                   gJOY[joyIndex].DevCapsJoy.ulAxes,
                   gJOY[joyIndex].DevCapsJoy.ulButtons,
                   pRP_GENIOCTL->rph.Status);
      #endif
      return;
   }

  // Get Pointer to the descriptor to be able to report Vendor/Device IDs later
  gJOY[joyIndex].pDevDesc = &(pServData->pDeviceInfo->descriptor);

  #ifdef DEBUG
    PrintJoyReport(joyIndex);
  #endif

   gJOY[joyIndex].joyAddr = pServData->pDeviceInfo->deviceAddress;
   gJOY[joyIndex].controllerID = pServData->pDeviceInfo->ctrlID;
   gJOY[joyIndex].interruptPipeAddress =
   GetInterruptPipeAddr ( pServData->pDeviceInfo->configurationData,
                          pServData->pDeviceInfo->descriptor.bNumConfigurations,
                          pServData->pDeviceInfo->bConfigurationValue,
                          gJOY[joyIndex].inInterface);

   gJOY[joyIndex].setITpack.bmRequestType = REQTYPE_TYPE_CLASS | REQTYPE_RECIPIENT_INTERFACE;
   gJOY[joyIndex].setITpack.bRequest = HID_REQUEST_SET_IDLE;
   gJOY[joyIndex].setITpack.wValue = 0x0000;  // for all reports only if changed
   gJOY[joyIndex].setITpack.wIndex = gJOY[joyIndex].inInterface;
   gJOY[joyIndex].setITpack.wLength = NULL;


   gJOY[joyIndex].active = TURNON;
   gNoOfJOYs++;

   SetIdleTime (joyIndex, JOY_IRQ_STATUS_IDLESET);

   pRP_GENIOCTL->rph.Status = USB_IDC_RC_OK;

   if(gpGameIDC)
     GenJoyRegisterDevice(joyIndex);

#ifdef DEBUG
   dsPrint2 (DBG_DETAILED, "USBJOY: JOYServ OK, joyIndex = %d, Status = 0x%x\r\n",
             joyIndex, pRP_GENIOCTL->rph.Status);
#endif
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME:  JOYdet                                           */
/*                                                                    */
/* DESCRIPTIVE NAME:  USB Joystick detach                             */
/*                                                                    */
/* FUNCTION:  This routine is used to deterime whether the            */
/*            USB Joystick driver can service a specified device.     */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT :  JOYdet                                              */
/*    LINKAGE  :                                                      */
/*                                                                    */
/* INPUT:  RP_GENIOCTL FAR *pRP_GENIOCTL                              */
/*                                                                    */
/* EXIT-NORMAL:  JOY IDC RC OK                                        */
/*                                                                    */
/* EXIT-ERROR:  device driver error code                              */
/*                                                                    */
/* EFFECTS:  None                                                     */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void JOYdet (RP_GENIOCTL FAR *pRP_GENIOCTL)
{
   BYTE           joyAddr;
   USHORT         joyIndex;
   USBDetach FAR *pDetData;

   pDetData = (USBDetach FAR *)pRP_GENIOCTL->ParmPacket;
   joyAddr = pDetData->deviceAddress;

#ifdef DEBUG
   dsPrint1 (DBG_DETAILED, "USBJOY: JOYdet, joyAddr = %d\r\n", joyAddr);
#endif

   if (gNoOfJOYs && pDetData->controllerId == NULL)
   {
      for (joyIndex = 0; joyIndex < MAX_JOYS; joyIndex++)
      {
         if (gJOY[joyIndex].active && gJOY[joyIndex].joyAddr == joyAddr)
         {
            gJOY[joyIndex].active = TURNOFF;
            gNoOfJOYs--;
            pRP_GENIOCTL->rph.Status = USB_IDC_RC_OK;
            break;
         }
      }

   }
   else   pRP_GENIOCTL->rph.Status = USB_IDC_RC_PARMERR;

#ifdef DEBUG
   dsPrint3 (DBG_DETAILED, "USBJOY: JOYdet, kbdAddr = %d, joyIndex = %d, Status = 0x%x\r\n",
             joyAddr, joyIndex, pRP_GENIOCTL->rph.Status);
#endif
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME:  JOYEnumDevices                                   */
/*                                                                    */
/* DESCRIPTIVE NAME:  Joystick Enumeration                            */
/*                                                                    */
/* FUNCTION:  This routine is called after the driver attached to the */
/*            Generic game driver                                     */
/*            It will send Attach IDC calls to that driver for all    */
/*            device which are already registered at this point       */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT :  JOYEnumDevices                                      */
/*    LINKAGE  :                                                      */
/*                                                                    */
/* INPUT:  RP_GENIOCTL FAR *pRP_GENIOCTL                              */
/*                                                                    */
/* EXIT-NORMAL:  JOY IDC RC OK                                        */
/*                                                                    */
/* EXIT-ERROR:  device driver error code                              */
/*                                                                    */
/* EFFECTS:  None                                                     */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void JOYEnumDevices(USHORT usGameDriverId)
{
  USHORT Iter;
  if(gNoOfJOYs)
  {
    for(Iter=0;Iter<MAX_JOYS;Iter++)
    {
      if(gJOY[Iter].active==TURNON)
      {
        if(GenJoyRegisterDevice(Iter))
          break;
      }
    }
  }
}

int GenJoyRegisterDevice(USHORT usDevID)
{
  RP_GENIOCTL  rp;     // GENeric IOCTL Request Packet
  GAMEIDCDEVATTACH AttachData;

  AttachData.Header.usHandle  = ((g_usGameDriverId &0x00FF)<<8) + (usDevID &0x00FF) ;
  AttachData.Header.usVendor  = gJOY[usDevID].pDevDesc->idVendor;
  AttachData.Header.usProduct = gJOY[usDevID].pDevDesc->idProduct;
  movmem( (PSZ)&AttachData.DevCaps,
          (PSZ)&gJOY[usDevID].DevCapsJoy,
          sizeof(DEVCAPS));
  movmem( (PSZ)&AttachData.Items,
          (PSZ)&gJOY[usDevID].Items,
          sizeof(gJOY[usDevID].Items));
  movmem( (PSZ)&AttachData.AxeUnits,
          (PSZ)&gJOY[usDevID].AxeUnits,
          sizeof(gJOY[usDevID].AxeUnits));

  rp.rph.Cmd = CMDGenIOCTL;
  rp.Category = GAME_IDC_CATEGORY_CLIENT;
  rp.Function = GAME_IDC_ATTACH_DEVICE;
  rp.ParmPacket = (PVOID)&AttachData;
  USBCallIDC (gpGameIDC, gdsGameIDC, (RP_GENIOCTL FAR *)&rp); // register Device with Game devices driver
  return (rp.rph.Status == USB_IDC_RC_OK) ? 0:1;
}

void GenJoyUpdateState(USHORT usDevID)
{
  RP_GENIOCTL  rp;     // GENeric IOCTL Request Packet
  GAMEIDCDEVSTATE SampleData;
  if(gpGameIDC)
  {
    SampleData.Header.usHandle  = ((g_usGameDriverId &0x00FF)<<8) + (usDevID &0x00FF) ;
    SampleData.Header.usVendor  = gJOY[usDevID].pDevDesc->idVendor;
    SampleData.Header.usProduct = gJOY[usDevID].pDevDesc->idProduct;
    movmem( (PSZ)&SampleData.State,
            (PSZ)&gJOY[usDevID].joyState,
            sizeof(JOYSTATE));
    rp.rph.Cmd = CMDGenIOCTL;
    rp.Category = GAME_IDC_CATEGORY_CLIENT;
    rp.Function = GAME_IDC_DEVICE_SAMPLE;
    rp.ParmPacket = (PVOID)&SampleData;
    USBCallIDC (gpGameIDC, gdsGameIDC, (RP_GENIOCTL FAR *)&rp); // register Device with Game devices driver
  }
}
