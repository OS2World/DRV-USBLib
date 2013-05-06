/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  GENSTRAT.C                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:  Game Device Manager strategy routines                 */
/*                                                                            */
/*   FUNCTION: These routines handle the task time routines for the strategy  */
/*             entry point of the Game Device Manager driver.                 */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:                                                            */
/*             GameStrategy                                                   */
/*             GameOpen                                                       */
/*             GameClose                                                      */
/*             GameIOCTL                                                      */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          01/03/06  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#include "gen.h"

#pragma optimize("", on)

void GameOpen (PRPH pRP)
{
  USHORT i;
#if 0
  if (deviceOwner == DEV_VDD_OWNER)
  {
    rp->RPstatus |= STERR | ERROR_TOO_MANY_OPEN_FILES;
  }
#endif

  // only one OS/2 application allowed to open the device at once
  // till it switches over to advanced mode.
  // due to the way booleans for blocking and running are stored in
  // global data space (not per process space) for compatibility device

  if( gusNumDev20Opens == 0 &&
      gusNumDevOpens < MAX_HANDLES &&
      gsNumComp20Device>-1 &&
      gsNumComp20Device < MAX_DEVICES  &&
      !gDevices[gsNumComp20Device].Open)
  {
    //deviceOwner = DEV_PDD_OWNER;
    for(i=0;i<MAX_HANDLES;i++)
    {
      if(!gHandleList[i].cUsed)
      {
        gHandleList[i].usSFN = ((PRP_OPENCLOSE)pRP)->sfn;
        gHandleList[i].cDeviceIndex = DEFAULT_DEVICE;
        gHandleList[i].ucAdvanced = 0;
        gDevices[gsNumComp20Device].Open = 1;
        gHandleList[i].cUsed = 1;
        gusNumDev20Opens++;
        gusNumDevOpens++;
        break;
      }
    }
    if(gusNumDev20Opens == 0)
      pRP->Status |= STERR | ERROR_TOO_MANY_OPEN_FILES;
  }
  else
    pRP->Status |= STERR | ERROR_TOO_MANY_OPEN_FILES;

}
/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME: GameClose                                         */
/*                                                                    */
/* DESCRIPTIVE NAME: Close the driver                                 */
/*                                                                    */
/* FUNCTION: This strategy routine closes the driver. This is an      */
/*           immediate command that is not put on the FIFO queue.     */
/*                                                                    */
/* NOTES: Strategy CMDClose = 14 = 0x0E.                              */
/*        The System File Number is a unique number associated with   */
/*        an Open request.                                            */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT: Close                                                 */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to Request Packet                             */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR: N/A                                                    */
/*                                                                    */
/* EFFECTS: pRP->Status                                               */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:                                                       */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

void GameClose (PRPH pRP)
{
   USHORT   index,CIndex;
  #ifdef DEBUG
    dsPrint2( DBG_HLVLFLOW,
              "GENGAME: Close unit=%x sfn=%x\r\n",
              pRP->Unit,
              ((PRP_OPENCLOSE)pRP)->sfn);
  #endif

  for (index = 0; index < MAX_HANDLES; index++)
  {
    if (gHandleList[index].usSFN == ((PRP_OPENCLOSE)pRP)->sfn)
    {
      gHandleList[index].usSFN = 0;
      if(gHandleList[index].cDeviceIndex == COMPATIBLE_DEVICE)
      {
        // Close the the current V2.0 Device
        gDevices[gsNumComp20Device].Open = 0;
        gusNumDev20Opens--;
      }
      else
      {
        // close the enhanced device
        gDevices[gHandleList[index].cDeviceIndex].Open = 0;

        if(gsNumComp20Device== -1)
        {
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
      gusNumDevOpens --;
      gHandleList[index].cDeviceIndex = DEVICE_NOTUSED;
      gHandleList[index].cUsed = 0;
      gHandleList[index].ucAdvanced = 0;
      break;
    }
  }

  if (index >= MAX_HANDLES)
  {
    pRP->Status |= STERR | ERROR_INVALID_HANDLE;
  }
}
/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME: IOCtl                                             */
/*                                                                    */
/* DESCRIPTIVE NAME: Generic Input/Output Control                     */
/*                                                                    */
/* FUNCTION: The function of this routine is to call appropriate      */
/*           worker routine to process the generic IOCtl strategy     */
/*           command.                                                 */
/*                                                                    */
/* NOTES: Strategy CMDGenIOCTL = 16 = 0x10.                           */
/*        The System File Number is a unique number associated with   */
/*        an Open request.                                            */
/*        See GENIOCTL.C for the worker routines.                     */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT: IOCtl                                                 */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to Request Packet                             */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR: N/A                                                    */
/*                                                                    */
/* EFFECTS: None                                                      */
/*                                                                    */
/* INTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:         FuncError                                     */
/*                      GetDeviceInfo                                 */
/*                      GetNumDevices                                 */
/*                      RegisterSemaphore                             */
/*                      DeregisterSemaphore                           */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

void GameIOCTL (PRPH pRP)
{
  UCHAR unit = pRP->Unit,
  category = ((PRP_GENIOCTL)pRP)->Category,
  function = ((PRP_GENIOCTL)pRP)->Function;

  #ifdef DEBUG
    dsPrint4( DBG_HLVLFLOW,
              "USBRESMGR: IOCtl unit=%x sfn=%x C=%x F=%x\r\n",
              unit,
              ((PRP_GENIOCTL)pRP)->sfn,
              category,
              function);
  #endif

  if (category != IOC_GENGAME)
  {
    pRP->Status |= STERR | ERROR_I24_BAD_COMMAND;
  }
  else
  {
    if (function < MIN_GENGAME_IOCTLF || function > MAX_GENGAME_IOCTLF)
    {
      FuncError ((PRP_GENIOCTL)pRP);
    }
    else
    {  // GENIOCTL.C
      (*gResFunc[function-MIN_GENGAME_IOCTLF])((PRP_GENIOCTL)pRP);
    }
  }
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  GAMEStrategy                                     */
/*                                                                    */
/* DESCRIPTIVE NAME:  Strategy entry point for the GDM                */
/*                    driver.                                         */
/*                                                                    */
/* FUNCTION:  The function of this routine is to call strategy worker */
/*            routine to process request packet.                      */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  GAMEStrategy                                         */
/*    LINKAGE:  CALL FAR                                              */
/*                                                                    */
/* INPUT:  es:bx -> kernel request packet                             */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR:  N/A                                                   */
/*                                                                    */
/* EFFECTS: None                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:  None                                         */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

#pragma optimize("eglt", off)

void far GameStrategy()
{
   PRPH     pRP;  // Pointer to Request Packet (Header)
   UCHAR    Cmd;  // strategy Command

   _asm
   {  // Strategy routine is called with ES:BX pointing to the Request Packet
      mov   word ptr pRP[0], bx
      mov   word ptr pRP[2], es
   }
   /*
      Request Packet Status field is defined only for Open and Close request packets
      on entry to the Strategy routine (is 0). For all other request packets,
      the Status field is undefined on entry.
   */
   pRP->Status = 0;
   Cmd = pRP->Cmd;

  #ifdef DEBUG
    dsPrint2 (DBG_HLVLFLOW, "GENGAME: Strategy Cmd=%x unit=%x\r\n", Cmd, pRP->Unit);
  #endif

   if (Cmd > CMDInitComplete)
     CmdError (pRP);
   else
     (*gStratList[Cmd])(pRP);

   pRP->Status |= STDON;

  #ifdef DEBUG
    dsPrint1 (DBG_HLVLFLOW, "GENGAME: Strategy S=%x\r\n", pRP->Status);
  #endif
}

#pragma optimize("", on)

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME: CmdError                                          */
/*                                                                    */
/* DESCRIPTIVE NAME: Command code Error                               */
/*                                                                    */
/* FUNCTION: The function of this routine is to return command not    */
/*           supported (bad command) for the request.                 */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT: CmdError                                              */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to Request Packet                             */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR: N/A                                                    */
/*                                                                    */
/* EFFECTS: pRP->Status = STDON | STERR | ERROR_I24_BAD_COMMAND       */
/*                                                                    */
/* INTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

void CmdError (PRPH pRP)
{
  #ifdef DEBUG
    dsPrint1 (DBG_CRITICAL, "GENGAME: CmdError=%x\r\n", pRP->Cmd);
  #endif

   pRP->Status = STDON | STERR | ERROR_I24_BAD_COMMAND;
}
#pragma optimize("", on)

