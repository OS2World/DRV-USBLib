/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  GENIDC.C                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME:  GAMEDEV Manager driver IDC routines                   */
/*                                                                            */
/*   FUNCTION: These routines handle the IDC for the Game Devices Manager.    */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:                                                            */
/*             Gameidc        IDC Entry Point                                 */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          01/02/04  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#include "gen.h"
/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME:  Gameidc                                          */
/*                                                                    */
/* DESCRIPTIVE NAME:  GAME$ IDC entry point                           */
/*                                                                    */
/* FUNCTION:  This routine is the GAME$ IDC entry point               */
/*            router/handler. IDC function requests are routed to the */
/*            appropriate worker routine. The address of this routine */
/*            is returned to other device drivers via the DevHlp      */
/*            AttachDD call.                                          */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT :  Gameidc                                             */
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

void FAR Gameidc (RP_GENIOCTL FAR *pRP_GENIOCTL)
{
  USHORT status;

  #ifdef DEBUG
    dsPrint3 ( DBG_HLVLFLOW, "GENGAME: IDC Enter> Category = 0x%x, Function = 0x%x, Status 0x%x\r\n",
               pRP_GENIOCTL->Category, pRP_GENIOCTL->Function, pRP_GENIOCTL->rph.Status);
  #endif

  status = pRP_GENIOCTL->rph.Status;
  pRP_GENIOCTL->rph.Status = USB_IDC_RC_OK;

  if ( (pRP_GENIOCTL->rph.Cmd != CMDGenIOCTL) ||
       (!pRP_GENIOCTL->ParmPacket) )
  {
    #ifdef DEBUG
      dsPrint2 (DBG_HLVLFLOW, "GENGAME: IDC Wrong paraameter : Cmd= 0x%x, Packet = 0x%x\r\n",
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
         case GAME_IDC_REGISTER_CLIENT:
            pRP_GENIOCTL->rph.Status = status;
            GameRegisterClient(pRP_GENIOCTL);
            break;
         case GAME_IDC_ATTACH_DEVICE:
            GameAttachDevice(pRP_GENIOCTL);
            break;
         case GAME_IDC_REMOVE_DEVICE:
            GameDetachDevice(pRP_GENIOCTL);
            break;
         case GAME_IDC_DEVICE_SAMPLE:
            GameProcessSample(pRP_GENIOCTL);
            break;
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
   dsPrint3 (DBG_HLVLFLOW, "GENGAME: IDC Leave < Category = 0x%x, Function = 0x%x, Status = 0x%x\r\n",
             pRP_GENIOCTL->Category, pRP_GENIOCTL->Function, pRP_GENIOCTL->rph.Status);
#endif
}

void GameRegisterClient (RP_GENIOCTL FAR *pRP_GENIOCTL)
{
  USBDClass FAR  *regData;
//  RP_GENIOCTL rp_USBReq;

  if(gNoOfDrivers >= MAX_DRIVERS)
  {  // no space to register client driver
    pRP_GENIOCTL->rph.Status=GAME_IDC_RC_EXCEEDSMAX;

  }
  else
  {
    regData=(USBDClass FAR *)pRP_GENIOCTL->ParmPacket;
    gDrivers[gNoOfDrivers].clientIDCAddr = regData->usbIDC;
    gDrivers[gNoOfDrivers].clientDS      = regData->usbDS;
    regData->usbDS = gNoOfDrivers;
    // Get all devices from new driver
/*    setmem((PSZ)&rp_USBReq, 0, sizeof(rp_USBReq));
    rp_USBReq.rph.Cmd=CMDGenIOCTL;   // IOCTL
    rp_USBReq.Category=GAME_IDC_CATEGORY_CLIENT;
    rp_USBReq.Function=GAME_IDC_LIST_DEVICES;
    rp_USBReq.ParmPacket=gNoOfDrivers;


    USBCallIDC( gDrivers[gNoOfDrivers].clientIDCAddr,
                gDrivers[gNoOfDrivers].clientDS,
                (RP_GENIOCTL FAR *)&rp_USBReq );
*/
    gNoOfDrivers++;
  }
}

void GameAttachDevice(RP_GENIOCTL FAR *pRP_GENIOCTL)
{
  PFGAMEIDCDEVATTACH pAttachData;
  USHORT DevIndex;
  if(gNoOfDevices >= MAX_DEVICES)
  {
    // no space to register client driver
    pRP_GENIOCTL->rph.Status=GAME_IDC_RC_EXCEEDSMAX;
  }
  else
  {
    for(DevIndex=0;DevIndex<MAX_DEVICES;DevIndex++)
      if(!gDevices[DevIndex].Active)
        break;
    if(DevIndex<MAX_DEVICES)
    {
      pAttachData=(PFGAMEIDCDEVATTACH)pRP_GENIOCTL->ParmPacket;
      gDevices[DevIndex].Active    = TRUE;
      gDevices[DevIndex].Open      = FALSE;
      gDevices[DevIndex].usHandle  = pAttachData->Header.usHandle;
      gDevices[DevIndex].usVendor  = pAttachData->Header.usVendor;
      gDevices[DevIndex].usProduct = pAttachData->Header.usProduct;
      movmem( (PSZ)&gDevices[DevIndex].DevCaps,
              (PSZ)&pAttachData->DevCaps,
              sizeof(DEVCAPS));
      movmem( (PSZ)&gDevices[DevIndex].Items,
              (PSZ)&pAttachData->Items,
              sizeof(gDevices[DevIndex].Items));
      movmem( (PSZ)&gDevices[DevIndex].AxeUnits,
              (PSZ)&pAttachData->AxeUnits,
              sizeof(gDevices[DevIndex].AxeUnits));
      if(gsNumComp20Device<0)
      {
        gsNumComp20Device = DevIndex;
      }
    }
    else
      pRP_GENIOCTL->rph.Status=GAME_IDC_RC_EXCEEDSMAX; // We should never get here ...

  }
}


void GameDetachDevice(RP_GENIOCTL FAR *pRP_GENIOCTL)
{
  PFGAMEIDCHEADER pDetachData;
  SHORT DevIndex, CIndex;
  pDetachData = (PFGAMEIDCHEADER)pRP_GENIOCTL->ParmPacket;
  for(DevIndex=0;DevIndex<MAX_DEVICES;DevIndex++)
  {
    if( gDevices[DevIndex].Active &&
        (gDevices[DevIndex].usHandle  == pDetachData->usHandle) &&
        (gDevices[DevIndex].usVendor  == pDetachData->usVendor) &&
        (gDevices[DevIndex].usProduct == pDetachData->usProduct) )
    {
      if(gDevices[DevIndex].Open)
      {
        // @@ Todo Check for polling in progess
      }
      gDevices[DevIndex].Active    = FALSE;
      gDevices[DevIndex].usHandle  = 0;
      gDevices[DevIndex].usVendor  = 0;
      gDevices[DevIndex].usProduct = 0;
      gNoOfDevices--;
      if(gsNumComp20Device == DevIndex)
      {
        gsNumComp20Device = -1; // No compatibility device
        // Set first Not opened Joystick as default if there is no default
        for(CIndex=0;CIndex<MAX_DEVICES;CIndex++)
        {
          if(gDevices[CIndex].Active &&
             !gDevices[CIndex].Open )
          {
            gsNumComp20Device = CIndex;
            break;
          }
        }
      }
    }
  }
}

void GameProcessSample(RP_GENIOCTL FAR *pRP_GENIOCTL)
{
  PFGAMEIDCDEVSTATE pStatusData;
  SHORT DevIndex;
  USHORT usCnt;
  pStatusData = (PFGAMEIDCDEVSTATE)pRP_GENIOCTL->ParmPacket;
  for(DevIndex=0;DevIndex<MAX_DEVICES;DevIndex++)
  {
    if( gDevices[DevIndex].Active &&
        (gDevices[DevIndex].usHandle  == pStatusData->Header.usHandle) &&
        (gDevices[DevIndex].usVendor  == pStatusData->Header.usVendor) &&
        (gDevices[DevIndex].usProduct == pStatusData->Header.usProduct) )
    {
      BYTE bOldBtn[4];
      BOOL fBtnPressed;
      movmem( (PSZ)&bOldBtn,
              (PSZ)&gDevices[DevIndex].State.rgbButtons[0],
              4); // Copy first 4 buttons for compatibility
      movmem( (PSZ)&gDevices[DevIndex].State,
              (PSZ)&pStatusData->State,
              sizeof(JOYSTATE) );
      // Is sample for compatibility device check for Locks
      if(gsNumComp20Device==DevIndex)
      {
        if(gV20Data.ulLockSample)
        {
          DevHelp_ProcRun( gV20Data.ulLockSample,&usCnt);
        }
        fBtnPressed = FALSE;
        if(bOldBtn[0] && !gDevices[DevIndex].State.rgbButtons[0])
        {
          fBtnPressed = TRUE;
          gV20Data.Status.b1cnt++;
        }
        if(bOldBtn[1] && !gDevices[DevIndex].State.rgbButtons[1])
        {
          fBtnPressed = TRUE;
          gV20Data.Status.b2cnt++;
        }
        if(bOldBtn[2] && !gDevices[DevIndex].State.rgbButtons[2])
        {
          fBtnPressed = TRUE;
          gV20Data.Status.b3cnt++;
        }
        if(bOldBtn[3] && !gDevices[DevIndex].State.rgbButtons[3])
        {
          fBtnPressed = TRUE;
          gV20Data.Status.b4cnt++;
        }
        if(fBtnPressed && gV20Data.ulLockButton)
          DevHelp_ProcRun(gV20Data.ulLockButton, &usCnt);

      }
    }
  }
}



