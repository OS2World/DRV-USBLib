/* $ID$ */
/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME: RESIOCTL.C                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME: USB Resource Manager driver IOCtl routines             */
/*                                                                            */
/*   FUNCTION: These routines handle the task time IOCtl commands to          */
/*             the USB Resource Manager driver.                               */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS: FuncError                                                  */
/*                 GetNumDevices    Special IOCtl Category A0, Function 0x31. */
/*                 GetDeviceInfo    Special IOCtl Category A0, Function 0x32. */
/*                 AquireDevice     Special IOCtl Category A0, Function 0x33. */
/*                 ReleaseDevice    Special IOCtl Category A0, Function 0x34. */
/*                 GetString        Special IOCtl Category A0, Function 0x35. */
/*                 SendControlURB   Special IOCtl Category A0, Function 0x36. */
/*                 SendBulkURB      Special IOCtl Category A0, Function 0x37. */
/*                 VerifyParam                                                */
/*                 VerifyData                                                 */
/*                 RegisterSemaphore   Special IOCtl Category 0xA0,           */
/*                                                   Function 0x41.           */
/*                 DeregisterSemaphore Special IOCtl Category 0xA0,           */
/*                                                   Function 0x42.           */
/*                 RegisterDevSemaphore   Special IOCtl Category 0xA0,        */
/*                                                   Function 0x43.           */
/*                 DeregisterDevSemaphore Special IOCtl Category 0xA          */
/*                                                   Function 0x44.           */
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

#include "gen.h"

static PUCHAR BufferVirtAddr (PUCHAR pStringData);

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME: FuncError                                         */
/*                                                                    */
/* DESCRIPTIVE NAME: IOCtl Function code Error                        */
/*                                                                    */
/* FUNCTION: The function of this routine is to return command not    */
/*           supported (bad command) for the request.                 */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT: FuncError                                             */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to Request Packet                             */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR: N/A                                                    */
/*                                                                    */
/* EFFECTS: pRP->rph.Status                                           */
/*                                                                    */
/* INTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

void FuncError (PRP_GENIOCTL pRP)
{
#ifdef DEBUG
   dsPrint2 (DBG_CRITICAL, "USBRESMGR: FuncError, C=%x, F=%x\r\n",
             pRP->Category, pRP->Function);
#endif

   pRP->rph.Status = STDON | STERR | ERROR_I24_BAD_COMMAND;
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: GetDeviceInfo                                     */
/*                                                                    */
/* DESCRIPTIVE NAME: Get USB Device Descriptor                        */
/*                                                                    */
/* FUNCTION: The Device Descriptor of an attached USB device          */
/*                                                                    */
/* NOTES: Special IOCtl Category 0xA0, Function 0x32.                 */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: RegisterSemaphore                                     */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         VerifyParam                                   */
/*                      VerifyData                                    */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/
void GetVersion(PRP_GENIOCTL pRP)        // 0x01
{
  if (VerifyData  (pRP, sizeof(ULONG)))
  {
    pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
  }
  *(PULONG)pRP->DataPacket = GAME_VERSION;
}

void GetParam(PRP_GENIOCTL pRP)          // 0x02
{
  GAME_PARM_STRUCT FAR  *pGameParms;
  if (VerifyData  (pRP, sizeof(GAME_PARM_STRUCT)))
  {
    pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
  }
  else
  {
    pGameParms = (GAME_PARM_STRUCT FAR  *)pRP->DataPacket;
    movmem((PSZ)pGameParms,
           (PSZ)&gV20Data.Parameters,
           sizeof(GAME_PARM_STRUCT));
  }
}
void SetParam(PRP_GENIOCTL pRP)          // 0x03
{
  GAME_PARM_STRUCT FAR  *pGameParms;
  if (VerifyParam  (pRP, sizeof(GAME_PARM_STRUCT)))
  {
    pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
  }
  else
  {
    pGameParms = (GAME_PARM_STRUCT FAR  *)pRP->ParmPacket;
    movmem((PSZ)&gV20Data.Parameters,
           (PSZ)pGameParms,
           sizeof(GAME_PARM_STRUCT));
  }
}

void GetCalib(PRP_GENIOCTL pRP)          // 0x04
{
  GAME_CALIB_STRUCT FAR *pGameCalib;
  if (VerifyData  (pRP, sizeof(GAME_CALIB_STRUCT)))
  {
    pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
  }
  else
  {
    pGameCalib = (GAME_CALIB_STRUCT FAR  *)pRP->DataPacket;
    movmem((PSZ)pGameCalib,
           (PSZ)&gV20Data.Calibration,
           sizeof(GAME_CALIB_STRUCT));
  }
}

void SetCalib(PRP_GENIOCTL pRP)          // 0x05
{
  GAME_CALIB_STRUCT FAR *pGameCalib;
  if (VerifyParam  (pRP, sizeof(GAME_CALIB_STRUCT)))
  {
    pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
  }
  else
  {
    pGameCalib = (GAME_CALIB_STRUCT FAR  *)pRP->ParmPacket;
    movmem((PSZ)&gV20Data.Calibration,
           (PSZ)pGameCalib,
           sizeof(GAME_CALIB_STRUCT));
  }
}

void GetDigSet(PRP_GENIOCTL pRP)         // 0x06
{
  GAME_DIGSET_STRUCT FAR  *pGameDigset;
  if (VerifyData  (pRP, sizeof(GAME_DIGSET_STRUCT)))
  {
    pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
  }
  else
  {
    pGameDigset = (GAME_DIGSET_STRUCT FAR  *)pRP->DataPacket;
    movmem((PSZ)pGameDigset,
           (PSZ)&gV20Data.DigitalSettings,
           sizeof(GAME_DIGSET_STRUCT));
  }
}

void SetDigSet(PRP_GENIOCTL pRP)         // 0x07
{
  GAME_DIGSET_STRUCT FAR  *pGameDigset;
  if (VerifyParam  (pRP, sizeof(GAME_DIGSET_STRUCT)))
  {
    pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
  }
  else
  {
    pGameDigset = (GAME_DIGSET_STRUCT FAR  *)pRP->ParmPacket;
    movmem((PSZ)&gV20Data.DigitalSettings,
           (PSZ)pGameDigset,
           sizeof(GAME_DIGSET_STRUCT));
  }
}

void GetStatus(PRP_GENIOCTL pRP)         // 0x10
{
  GAME_STATUS_STRUCT FAR  *pGameStatus;
  if (VerifyData  (pRP, sizeof(GAME_STATUS_STRUCT)))
  {
    pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
  }
  else
  {
    pGameStatus = (GAME_STATUS_STRUCT FAR  *)pRP->DataPacket;
    movmem( (PSZ)pGameStatus,
            (PSZ)&gV20Data.Status,
            sizeof(GAME_STATUS_STRUCT));
    gV20Data.Status.b1cnt = 0;
    gV20Data.Status.b2cnt = 0;
    gV20Data.Status.b3cnt = 0;
    gV20Data.Status.b4cnt = 0;
  }
}

void GetStatusButWait(PRP_GENIOCTL pRP)  // 0x11
{
  GAME_STATUS_STRUCT FAR  *pGameStatus;
  if (VerifyData  (pRP, sizeof(GAME_STATUS_STRUCT)))
  {
    pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
  }
  else
  {
    USHORT rc = 0;
    if( (gV20Data.Status.b1cnt == 0) &&
        (gV20Data.Status.b2cnt == 0) &&
        (gV20Data.Status.b3cnt == 0) &&
        (gV20Data.Status.b4cnt == 0))
    {
      gV20Data.ulLockButton = 1;
      rc = DevHelp_ProcBlock( gV20Data.ulLockButton,
                              -1,
                              WAIT_IS_INTERRUPTABLE);
      gV20Data.ulLockButton = 0;
    }
    if(rc==0)
    {
      pGameStatus = (GAME_STATUS_STRUCT FAR  *)pRP->DataPacket;
      movmem( (PSZ)pGameStatus,
              (PSZ)&gV20Data.Status,
              sizeof(GAME_STATUS_STRUCT));
      gV20Data.Status.b1cnt = 0;
      gV20Data.Status.b2cnt = 0;
      gV20Data.Status.b3cnt = 0;
      gV20Data.Status.b4cnt = 0;
    }
    else
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
  }
}

void GetStatusSampWait(PRP_GENIOCTL pRP) // 0x12
{
  GAME_STATUS_STRUCT FAR  *pGameStatus;
  if (VerifyData  (pRP, sizeof(GAME_STATUS_STRUCT)))
  {
    pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
  }
  else
  {
    USHORT rc;
    gV20Data.ulLockSample = 1;
    rc = DevHelp_ProcBlock( gV20Data.ulLockSample,
                            -1,
                            WAIT_IS_INTERRUPTABLE);
    gV20Data.ulLockSample = 0;
    if(rc==0)
    {
      pGameStatus = (GAME_STATUS_STRUCT FAR  *)pRP->DataPacket;
      movmem( (PSZ)pGameStatus,
              (PSZ)&gV20Data.Status,
              sizeof(GAME_STATUS_STRUCT));
      gV20Data.Status.b1cnt = 0;
      gV20Data.Status.b2cnt = 0;
      gV20Data.Status.b3cnt = 0;
      gV20Data.Status.b4cnt = 0;
    }
  }
}

void SwitchToAdvanced(PRP_GENIOCTL pRP)  // 0x20
{
  USHORT index;
  if (VerifyData  (pRP, sizeof(ULONG)))
  {
    pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
  }
  for(index=0;index<MAX_HANDLES;index++)
  {
    if( gHandleList[index].cUsed &&
        gHandleList[index].usSFN == pRP->sfn)
     break;
  }
  if(index<MAX_HANDLES)
  {
    gHandleList[index].ucAdvanced = 1;
    gusNumDev20Opens--;
    *(PULONG)pRP->DataPacket = gsNumComp20Device;
  }
  else
  {
    *(PULONG)pRP->DataPacket = 0xdeadbeef;
    pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
  }
}

void GetDeviceString(PRP_GENIOCTL pRP)  // 0x22
{
}

void SelectDevice(PRP_GENIOCTL pRP)      // 0x23
{
  CHAR cNewDevIndex;
  USHORT index,CIndex;

  if (VerifyParam  (pRP, sizeof(CHAR)))
  {
    pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
  }
  else
  {
    for(index=0;index<MAX_HANDLES;index++)
    {
      if( gHandleList[index].cUsed &&
          gHandleList[index].usSFN == pRP->sfn)
       break;
    }
    if(index<MAX_HANDLES &&
       gHandleList[index].ucAdvanced)
    {
      cNewDevIndex = *(PCHAR)pRP->ParmPacket;
      if( (cNewDevIndex < MAX_DEVICES) &&
          (cNewDevIndex >= DEFAULT_DEVICE) )
      {
        if(gDevices[cNewDevIndex].Open)
          pRP->rph.Status |= STERR | ERROR_TOO_MANY_OPEN_FILES;
        else
        {
          if(gDevices[cNewDevIndex].Active)
          {
            gDevices[cNewDevIndex].Open = 1;
            gDevices[gHandleList[index].cDeviceIndex==DEFAULT_DEVICE?
                     gsNumComp20Device:gHandleList[index].cDeviceIndex].Open = 0;

            gHandleList[index].cDeviceIndex = cNewDevIndex;
            if(cNewDevIndex==gsNumComp20Device)
            {
              gsNumComp20Device = -1; // No longer a default device
              for(CIndex=0;CIndex<MAX_DEVICES;CIndex++) // Set first Not opened Joysticj as default
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
          else
            pRP->rph.Status |= STERR | ERROR_INVALID_DATA; // @@ Other error code ?
        }
      }
      else
        pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
    }
    else
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE; // @@ToDo find good error value
  }

}

void GetDevCaps(PRP_GENIOCTL pRP)        // 0x24
{
  USHORT index;
  PFDEVCAPS pData = (PFDEVCAPS)pRP->DataPacket;
  if (VerifyData(pRP, sizeof(DEVCAPS)) ||
      pData->ulSize < sizeof(DEVCAPS))
  {
    pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
  }
  else
  {
    for(index=0;index<MAX_HANDLES;index++)
    {
      if( gHandleList[index].cUsed &&
          gHandleList[index].usSFN == pRP->sfn)
       break;
    }
    if(index<MAX_HANDLES &&
       gHandleList[index].ucAdvanced)
    {
      if(gDevices[gHandleList[index].cDeviceIndex].Active)
        movmem( (PSZ)pData,
                (PSZ)&gDevices[gHandleList[index].cDeviceIndex].DevCaps,
                sizeof(DEVCAPS) );
      else
        pRP->rph.Status |= STERR | ERROR_GEN_FAILURE; // @@ToDo find good error value
    }
    else
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE; // @@ToDo find good error value
  }

}

void GetJoyState(PRP_GENIOCTL pRP)       // 0x25
{
  USHORT index;
  PFJOYSTATE pData = (PFJOYSTATE)pRP->DataPacket;
  if (VerifyData(pRP, sizeof(JOYSTATE)) )
  {
    pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
  }
  else
  {
    for(index=0;index<MAX_HANDLES;index++)
    {
      if( gHandleList[index].cUsed &&
          gHandleList[index].usSFN == pRP->sfn)
       break;
    }
    if(index<MAX_HANDLES &&
       gHandleList[index].ucAdvanced)
    {
      if(gDevices[gHandleList[index].cDeviceIndex].Active)
        movmem( (PSZ)pData,
                (PSZ)&gDevices[gHandleList[index].cDeviceIndex].State,
                sizeof(JOYSTATE) );
      else
        pRP->rph.Status |= STERR | ERROR_GEN_FAILURE; // @@ToDo find good error value
    }
    else
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE; // @@ToDo find good error value
  }
}

void GetDeadZone(PRP_GENIOCTL pRP)       // 0x26
{
  USHORT index;

  PFDZSATCAPS pData = (PFDZSATCAPS)pRP->DataPacket;
  if (VerifyData(pRP, sizeof(DZSATCAPS)) )
  {
    pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
  }
  else
  {
    for(index=0;index<MAX_HANDLES;index++)
    {
      if( gHandleList[index].cUsed &&
          gHandleList[index].usSFN == pRP->sfn)
       break;
    }
    if(index<MAX_HANDLES &&
       gHandleList[index].ucAdvanced)
    {
      if(gDevices[gHandleList[index].cDeviceIndex].Active)
        movmem( (PSZ)pData,
                (PSZ)&gDevices[gHandleList[index].cDeviceIndex].DeadZone,
                sizeof(DZSATCAPS) );
      else
        pRP->rph.Status |= STERR | ERROR_GEN_FAILURE; // @@ToDo find good error value
    }
    else
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE; // @@ToDo find good error value
  }
}

void SetDeadZone(PRP_GENIOCTL pRP)       // 0x27
{
  USHORT index;

  PFDZSATCAPS pData = (PFDZSATCAPS)pRP->DataPacket;
  if (VerifyData(pRP, sizeof(DZSATCAPS)) )
  {
    pRP->rph.Status |= STERR | ERROR_INVALID_DATA;
  }
  else
  {
    for(index=0;index<MAX_HANDLES;index++)
    {
      if( gHandleList[index].cUsed &&
          gHandleList[index].usSFN == pRP->sfn)
       break;
    }
    if(index<MAX_HANDLES &&
       gHandleList[index].ucAdvanced)
    {
      if(gDevices[gHandleList[index].cDeviceIndex].Active)
      {
        movmem( (PSZ)&gDevices[gHandleList[index].cDeviceIndex].DeadZone,
                (PSZ)pData,
                sizeof(DZSATCAPS) );
        // @@ToDo Send IDC call to driver to update sample value processing
        // or do dz processing here?
      }
      else
        pRP->rph.Status |= STERR | ERROR_GEN_FAILURE; // @@ToDo find good error value
    }
    else
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE; // @@ToDo find good error value
  }
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: GetNumDevices                                     */
/*                                                                    */
/* DESCRIPTIVE NAME: Get Number of attached Game Devices              */
/*                                                                    */
/* FUNCTION: returns the Number of attached Game devices              */
/*                                                                    */
/* NOTES: Special IOCtl Category 0xA0, Function 0x21.                 */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: GetNumDevices                                         */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         VerifyParam                                   */
/*                      VerifyData                                    */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void  GetNumDevices (PRP_GENIOCTL pRP)
{

   if (VerifyData  (pRP, sizeof(ULONG)))
   {
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
   }
   *(PULONG)pRP->DataPacket = gNoOfDevices;
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME: VerifyParam                                       */
/*                                                                    */
/* DESCRIPTIVE NAME: Verify access to the Parameter packet            */
/*                                                                    */
/* FUNCTION: This function is used to verify access to the parameter  */
/*           packet.                                                  */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT: VerifyParam                                           */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*        packetLength                                                */
/*                                                                    */
/* EXIT-NORMAL: 0 if access is verified                               */
/*                                                                    */
/* EXIT-ERROR:                                                        */
/*                                                                    */
/* EFFECTS: None                                                      */
/*                                                                    */
/* INTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

USHORT VerifyParam (PRP_GENIOCTL pRP, USHORT packetLength)
{
   if (packetLength)
   {
      if (!(OFFSETOF(pRP->ParmPacket) ||
            SELECTOROF(pRP->ParmPacket) & SELECTOR_MASK))
      {
         return TRUE;
      }
      return (DevHelp_VerifyAccess (SELECTOROF(pRP->ParmPacket),
                                    packetLength,
                                    OFFSETOF(pRP->ParmPacket),
                                    VERIFY_READONLY));
   }
   else
   {
      return (OFFSETOF(pRP->ParmPacket) ||
              SELECTOROF(pRP->ParmPacket) & SELECTOR_MASK);
   }
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME: VerifyData                                        */
/*                                                                    */
/* DESCRIPTIVE NAME: Verify access to the Data packet                 */
/*                                                                    */
/* FUNCTION: This function is used to verify access to the data       */
/*           packet.                                                  */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT: VerifyData                                            */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*        dataLength                                                  */
/*                                                                    */
/* EXIT-NORMAL: 0 if access is verified                               */
/*                                                                    */
/* EXIT-ERROR: N/A                                                    */
/*                                                                    */
/* EFFECTS: None                                                      */
/*                                                                    */
/* INTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

USHORT VerifyData (PRP_GENIOCTL pRP, USHORT dataLength)
{
   if (dataLength)
   {
      if (!(OFFSETOF(pRP->DataPacket) ||
            SELECTOROF(pRP->DataPacket) & SELECTOR_MASK))
      {
         return TRUE;
      }
      return (DevHelp_VerifyAccess (SELECTOROF(pRP->DataPacket),
                                    dataLength,
                                    OFFSETOF(pRP->DataPacket),
                                    VERIFY_READWRITE));
   }
   else
   {
      return (OFFSETOF(pRP->DataPacket) ||
              SELECTOROF(pRP->DataPacket) & SELECTOR_MASK);
   }
}


