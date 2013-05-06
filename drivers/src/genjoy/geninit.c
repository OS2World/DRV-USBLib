/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  JOYINIT.C                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB Joystick driver initialization                    */
/*                                                                            */
/*   FUNCTION: This routine:                                                  */
/*            1) analyzes DEVICE= parameters;                                 */
/*            2) checks for HID layer driver presence and fails               */
/*               to initialize if no HID drivers found;                       */
/*            3) initalizes USB Joystick driver's local structures;           */
/*            4) registers driver & adapter with RM.                          */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:                                                            */
/*             JOYInit                                                        */
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

#include "gen.h"

static BOOL RegisterDrv(void);
void GAMEInit();

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  GAMEInit                                         */
/*                                                                    */
/* DESCRIPTIVE NAME:  USB Joystick driver initialization              */
/*                                                                    */
/* FUNCTION:  The function of this routine is to initialize the       */
/*            USB Joystick driver.                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Initialization time                                       */
/*                                                                    */
/* ENTRY POINT:  JOYInit                                              */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  pRP-> kernel request packet                                */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR:  N/A                                                   */
/*                                                                    */
/* EFFECTS:  pRPO->CodeEnd = end of code segment                      */
/*           pRPO->DataEnd = end of data segment                      */
/*           pRP->Status = STATUS_DONE                                */
/*           pRP->Status = STDON + STERR + ERROR_I24_GEN_FAILURE      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*                                                                    */
/* EXTERNAL REFERENCES:  RMCreateDriver                               */
/*                       RMCreateAdapter                              */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

void GAMEInit(PRPH pRP)
{
  KeyData keyData[1] = {"V",       CFSTR_TYPE_DEC, 0, 0};
  PSZ    kbdCmdLine;
  ULONG  cmdRStatus, errColumn;
  USHORT cmdRc, i;

#ifdef DEBUG
  dsPrint (DBG_HLVLFLOW, "GENGAME: Init Entry\r\n");
#endif

  pRP->Status = STATUS_DONE;

  if (ghDriver)
    return;  // initialization already passed

  Device_Help = ((PRPINITIN)pRP)->DevHlpEP; // save DevHlp Entry Point
  kbdCmdLine = (PSZ)MAKEP (SELECTOROF (((PRPINITIN)pRP)->InitArgs),
                           OFFSETOF   (((PRPINITIN)pRP)->InitArgs));
  cmdRStatus = ProcessConfigString (kbdCmdLine, 2, (KeyData FAR *)&keyData);
  cmdRc      = LOUSHORT (cmdRStatus);
  errColumn  = (ULONG)HIUSHORT (cmdRStatus);

  gVerbose   = (BYTE)(keyData[0].keyStatus != CFSTR_STATUS_NOTFOUND);

  RegisterDrv();                                   // register driver with RM

  gsNumComp20Device = -1;
  gusNumDev20Opens  = 0;
  gusNumDevOpens    = 0;
  gNoOfDevices      = 0;
  gNoOfDrivers      = 0;
  for(i = 0; i < MAX_HANDLES; i++)
  {
    gHandleList[i].usSFN         = 0;
    gHandleList[i].cUsed         = 0;
    gHandleList[i].cDeviceIndex  = DEVICE_NOTUSED;
    gHandleList[i].ucAdvanced    = 0;
    gHandleList[i].ucDevDetached = 1;
  }
  for(i=0; i<MAX_DEVICES;i++)
    setmem((PSZ)&gDevices[i], 0, sizeof(GameDevice));
  for(i=0;i<MAX_DRIVERS;i++)
    setmem((PSZ)&gDrivers[i], 0, sizeof(DriverList));
  setmem((PSZ)&gV20Data,0, sizeof(gV20Data));

  ((PRPINITOUT)pRP)->CodeEnd = ((USHORT)&GAMEInit);        // set end of code segment
  ((PRPINITOUT)pRP)->DataEnd = ((USHORT)&gInitDataStart); // set end of data segment
  SetLongValue (gVMessages[INIT_MESSAGE_LOADED], (ULONG)gDriverStruct.MajorVer);
  SetLongValue (gVMessages[INIT_MESSAGE_LOADED], (ULONG)gDriverStruct.MinorVer);
  gMessageCount = AddToMsgArray (gMessageIDs, INIT_MESSAGE_LOADED, gMessageCount, MAX_INIT_MESSAGE_COUNT);
  if (gVerbose)
    TTYWrite (gVMessages, gMessageIDs, gMessageCount);

#ifdef DEBUG
   dsPrint1 (DBG_HLVLFLOW, "GENGAME: Init Status = %x\r\n", pRP->Status);
#endif
   return;
}

//-----------------------------------------------------------------------------

static BOOL RegisterDrv (void)
{
   APIRET      rc = RMRC_SUCCESS;
   UCHAR       ResourceBuf[12];
   ADJUNCT     AdjAdaptNum;
   PAHRESOURCE pResourceList = (PAHRESOURCE)ResourceBuf;

   rc = RMCreateDriver (&gDriverStruct, &ghDriver);
   if (rc==RMRC_SUCCESS)
   {
      pResourceList->NumResource = 0;
      gAdapterStruct.HostBusType = AS_HOSTBUS_PCI;
      AdjAdaptNum.pNextAdj       = NULL;
      AdjAdaptNum.AdjLength      = sizeof(ADJUNCT);
      AdjAdaptNum.AdjType        = ADJ_ADAPTER_NUMBER;
      AdjAdaptNum.Adapter_Number = 0;
      gAdapterStruct.pAdjunctList = &AdjAdaptNum;

      rc = RMCreateAdapter (ghDriver, &ghAdapter,  &gAdapterStruct, NULL, pResourceList);
   }
   return (rc==RMRC_SUCCESS);
}

