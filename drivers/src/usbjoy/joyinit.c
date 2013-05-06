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

#include "joy.h"

static BOOL RegisterJOY (void);
void JOYInit();

#pragma alloc_text(_INIT, JOYInit, RegisterJOY, AddToMsgArray, SetLongValue, TTYWrite,ProcessConfigString)


/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  JOYInit                                          */
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

void JOYInit()
{
   KeyData keyData[2] = {"V",       CFSTR_TYPE_DEC, 0, 0,
      "DEVICE:", CFSTR_TYPE_DEC, 0, 0};
   PSZ    kbdCmdLine;
   ULONG  cmdRStatus, errColumn;
   USHORT cmdRc, i;

#ifdef DEBUG
   dsPrint (DBG_HLVLFLOW, "USBJOY: Init Entry\r\n");
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

   switch (cmdRc)
   {
     case CFSTR_UNKWN_KEYS:
        SetLongValue (gVMessages[INIT_MESSAGE_UNKNOWNKWD], errColumn);
        gMessageCount = AddToMsgArray (gMessageIDs, INIT_MESSAGE_UNKNOWNKWD, gMessageCount, MAX_INIT_MESSAGE_COUNT);
      break;
     case CFSTR_CONVERR:
       SetLongValue (gVMessages[INIT_MESSAGE_INVNUMERIC], errColumn );
       gMessageCount = AddToMsgArray (gMessageIDs, INIT_MESSAGE_INVNUMERIC, gMessageCount, MAX_INIT_MESSAGE_COUNT);
   }

   gVerbose   = (BYTE)(keyData[0].keyStatus != CFSTR_STATUS_NOTFOUND);
   if (keyData[1].keyStatus == CFSTR_STATUS_OK)
      gDevice = (keyData[1].value < MAX_JOYS)? (BYTE)keyData[1].value : (BYTE)NULL;
   else gDevice = NULL;

   if (DevHelp_AttachDD ("USBHID$ ", (NPBYTE)&gIDCTable)) // obtain USBHID's IDC entry point
   {
      pRP->Status |= STERR;
      gMessageCount = AddToMsgArray (gMessageIDs, INIT_MESSAGE_NO_HID, gMessageCount, MAX_INIT_MESSAGE_COUNT);
      #ifdef DEBUG
         dsPrint (DBG_HLVLFLOW, "USBJOY: Init Faild to Attach to USBHID$ \r\n");
      #endif
   }
   else
   {
     gpHIDIDC  = (PUSBIDCEntry)gIDCTable.ProtIDCEntry;
     gdsHIDIDC = gIDCTable.ProtIDC_DS;
   }

   if (pRP->Status == STATUS_DONE)
   {
      RegisterJOY();                                              // register driver with RM

      gNoOfJOYs = 0;
      for (i = 0; i < MAX_JOYS; i++)
        gJOY[i].active = 0;

      ((PRPINITOUT)pRP)->CodeEnd = ((USHORT)&JOYInit);        // set end of code segment
      ((PRPINITOUT)pRP)->DataEnd = ((USHORT)&gInitDataStart); // set end of data segment
      SetLongValue (gVMessages[INIT_MESSAGE_LOADED], (ULONG)gDriverStruct.MajorVer);
      SetLongValue (gVMessages[INIT_MESSAGE_LOADED], (ULONG)gDriverStruct.MinorVer);
      SetLongValue (gVMessages[INIT_MESSAGE_LOADED], (ULONG)gDevice);
      gMessageCount = AddToMsgArray (gMessageIDs, INIT_MESSAGE_LOADED, gMessageCount, MAX_INIT_MESSAGE_COUNT);
   }
   else
   {
      ((PRPINITOUT)pRP)->CodeEnd = 0;
      ((PRPINITOUT)pRP)->DataEnd = 0;
   }
   if (gVerbose) TTYWrite (gVMessages, gMessageIDs, gMessageCount);

#ifdef DEBUG
   dsPrint1 (DBG_HLVLFLOW, "USBJOY: Init Status = %x\r\n", pRP->Status);
#endif
   return;
}

//-----------------------------------------------------------------------------

static BOOL RegisterJOY (void)
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

