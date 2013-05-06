/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  LOGINIT.C                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:  Log Driver initialization routines                    */
/*                                                                            */
/*   FUNCTION: These routines handle USB Log Driver                           */
/*             initialization process.                                        */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:  LogInit                                                   */
/*                  RegisterLog                                               */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          03/01/12  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#include "log.h"

static BOOL RegisterLog (void);

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  LogInit                                          */
/*                                                                    */
/* DESCRIPTIVE NAME: Log Driver initialization                        */
/*                                                                    */
/* FUNCTION:  The function of this routine is to initialize the       */
/*            Log Driver.                                             */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Initialization time                                       */
/*                                                                    */
/* ENTRY POINT:  LogInit                                              */
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
/* INTERNAL REFERENCES:  RegisterLog                                  */
/*                                                                    */
/* EXTERNAL REFERENCES:  LogInfoInit                                  */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

void LogInit (PRPH pRP)
{
  PRPINITIN   pRPI;    // Input Request Packet far pointer
  PRPINITOUT  pRPO;    // Output Request Packet far pointer

  KeyData     keyData[1] = {"V",  CFSTR_TYPE_DEC,    0, 0};
  PSZ         cmdLine;
  ULONG       cmdRStatus, errColumn;
  USHORT      cmdRc;

  #ifdef DEBUG
    dsPrint (DBG_HLVLFLOW, "LOG: Init\r\n");
  #endif

  pRP->Status = STATUS_DONE;

  pRPI = (PRPINITIN)pRP;
  pRPO = (PRPINITOUT)pRP;
  Device_Help = pRPI->DevHlpEP;    // save DevHelp Entry Point
  /*
  process CONFIG.SYS DEVICE= line parameters
  */

  cmdLine    = (PSZ)MAKEP (SELECTOROF (pRPI->InitArgs), OFFSETOF (pRPI->InitArgs));
  cmdRStatus = ProcessConfigString (cmdLine, sizeof(keyData)/sizeof(keyData[0]),
                                    (KeyData FAR *)&keyData);
  cmdRc = LOUSHORT (cmdRStatus);
  errColumn = (ULONG)HIUSHORT (cmdRStatus);

  switch (cmdRc)
  {
    case CFSTR_UNKWN_KEYS:
      SetLongValue (gVMessages[INIT_MESSAGE_UNKNOWNKWD], errColumn);
      gMessageCount = AddToMsgArray (gMessageIDs, INIT_MESSAGE_UNKNOWNKWD, gMessageCount, MAX_INIT_MESSAGE_COUNT);
      break;
    case CFSTR_CONVERR:
      SetLongValue (gVMessages[INIT_MESSAGE_INVNUMERIC], errColumn );
      gMessageCount = AddToMsgArray (gMessageIDs, INIT_MESSAGE_INVNUMERIC, gMessageCount, MAX_INIT_MESSAGE_COUNT);
      break;
    default:
      break;
  }
  gVerbose = keyData[0].keyStatus != CFSTR_STATUS_NOTFOUND;

  setmem(  gucLogBuffer, LOG_BUFFER_SIZE, 0x00);
  gulReadPos  = 0;
  gulWritePos = 0;

  DevHelp_GetDOSVar(DHGETDOSV_SYSINFOSEG, 0, (PPVOID)&gpTime);
  gpTime = MAKEP ((SEL)*gpTime, SYSINFO_TIME_MS);

  if( DevHelp_AllocGDTSelector ( (PSEL)&gGDTSel, 1 ) )
  {
    pRP->Status |= STERR | ERROR_I24_GEN_FAILURE;
    gMessageCount = AddToMsgArray ( gMessageIDs, INIT_MESSAGE_NOT_ALLOC, gMessageCount,
                                    MAX_INIT_MESSAGE_COUNT);
  }
  else
  {
    SetLongValue (gVMessages[INIT_MESSAGE_LOADED], (ULONG)gDriverStruct.MajorVer);
    SetLongValue (gVMessages[INIT_MESSAGE_LOADED], (ULONG)gDriverStruct.MinorVer);
    gMessageCount = AddToMsgArray (gMessageIDs, INIT_MESSAGE_LOADED, gMessageCount, MAX_INIT_MESSAGE_COUNT);
  }
  pRPO->CodeEnd = ((USHORT)&LogInit) - 1;        // set end of code segment
  pRPO->DataEnd = ((USHORT)&gInitDataStart) - 1; // set end of data segment


  RegisterLog();
  if (gVerbose)
    TTYWrite (gVMessages, gMessageIDs, gMessageCount);

  #ifdef DEBUG
     dsPrint (DBG_HLVLFLOW,"LOG: Calling InitLogging\r\n");
  #endif

  #ifdef DEBUG
    dsPrint3 (DBG_HLVLFLOW,
             "LOG: Init Done  S=%x\r\n End of Code at %x\r\n End of Data at %x\r\n",
             pRP->Status,
             pRPO->CodeEnd,
             pRPO->DataEnd );
  #endif
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  RegisterLog                                      */
/*                                                                    */
/* DESCRIPTIVE NAME: Log Driver resource registration                 */
/*                                                                    */
/* FUNCTION:   The function of this routine is to register            */
/*             Log Driver andcorresponding adapter resources.         */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Initialization time                                       */
/*                                                                    */
/* ENTRY POINT:  RegisterLog                                          */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  none                                                       */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR:  N/A                                                   */
/*                                                                    */
/* EFFECTS:  ghDriver, ghAdapter recieves RM handles                  */
/*                                                                    */
/* INTERNAL REFERENCES:  none                                         */
/*                                                                    */
/* EXTERNAL REFERENCES:  none                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

static BOOL RegisterLog (void)
{
  APIRET      rc = RMRC_SUCCESS;
  UCHAR       ResourceBuf[12];
  ADJUNCT     AdjAdaptNum;
  PAHRESOURCE pResourceList = (PAHRESOURCE)ResourceBuf;

  rc = RMCreateDriver (&gDriverStruct, &ghDriver);
  if (rc == RMRC_SUCCESS)
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


