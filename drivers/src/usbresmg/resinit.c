/* $ID$ */

/* Copyright 2001,2002 Markus Montkowski                                      */
/* This code is released under the Aladdin Free Public License                */
/*                    (Version 9, September 18, 2000)                         */
/* See License file in root of CVS for details                                */

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME: RESINIT.C                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME: USB Resource Manager driver Initialization routines    */
/*                                                                            */
/*   FUNCTION: These routines handle the USB driver initialization            */
/*             process.                                                       */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS: Init                                                       */
/*                                                                            */
/*   EXTERNAL REFERENCES: ProcessConfigString                                 */
/*                        SetLongValue                                        */
/*                        AddToMsgArray                                       */
/*                        TTYWrite                                            */
/*                        movmem                                              */
/*                        setmem                                              */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          00/01/14  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#include "res.h"

static BOOL Register (void);

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME: Init                                              */
/*                                                                    */
/* DESCRIPTIVE NAME: USB Resource Manager driver Initialization       */
/*                                                                    */
/* FUNCTION: The function of this routine is to initialize the        */
/*           USB Resource Manager driver.                             */
/*                                                                    */
/* NOTES: Strategy CMDInit = 0.                                       */
/*                                                                    */
/* CONTEXT: Initialization time                                       */
/*                                                                    */
/* ENTRY POINT: Init                                                  */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to Request Packet                             */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR: N/A                                                    */
/*                                                                    */
/* EFFECTS: pRPO->CodeEnd = end of code segment                       */
/*          pRPO->DataEnd = end of data segment                       */
/*          pRP->Status                                               */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         Register                                      */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:         ProcessConfigString                           */
/*                      SetLongValue                                  */
/*                      AddToMsgArray                                 */
/*                      TTYWrite                                      */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

#pragma optimize("eglt", off)

void Init (PRPH pRP)
{
  PRPINITIN   pRPI = (PRPINITIN)pRP;  //  Input Request Packet far pointer
  PRPINITOUT  pRPO = (PRPINITOUT)pRP; // Output Request Packet far pointer

  struct SysDev3 *pHeader = (NPVOID)&gHead; // See PRTSEGS.ASM

  KeyData  keyData[] = {"V",    CFSTR_TYPE_DEC, 0, 0};
  PSZ      cmdLine;
  ULONG    cmdRStatus, errColumn;
  USHORT   cmdRc, index, pindex;

  pRP->Status = STDON;

  if (!gFirstInit)
  {
    pRPO->Unit = 0;
    pRPO->CodeEnd = (USHORT)&Init;           // End of code segment
    pRPO->DataEnd = (USHORT)&gInitDataStart; // End of data segment
    pRPO->BPBArray = 0;
    return;
  }

  gFirstInit = FALSE;

  #ifdef DEBUG
    dsPrint (DBG_HLVLFLOW, "USBRESMGR: Init\r\n");
  #endif

  Device_Help = pRPI->DevHlpEP;                // Save DevHelp Entry Point
  /*
    Process CONFIG.SYS DEVICE= line parameters
  */
  cmdLine = (PSZ)MAKEP (SELECTOROF (pRPI->InitArgs), OFFSETOF (pRPI->InitArgs));
  cmdRStatus = ProcessConfigString (cmdLine, sizeof(keyData)/sizeof(keyData[0]),
                                     (KeyData FAR *)&keyData);
  cmdRc = LOUSHORT (cmdRStatus);
  errColumn = (ULONG)HIUSHORT (cmdRStatus);

  switch (cmdRc)
  {
    case CFSTR_UNKWN_KEYS:
      SetLongValue (gVMessages[INIT_MESSAGE_UNKNOWNKWD], errColumn);
      gMessageCount = AddToMsgArray (gMessageIDs, INIT_MESSAGE_UNKNOWNKWD,
                                     gMessageCount, MAX_INIT_MESSAGE_COUNT);
      break;

    case CFSTR_CONVERR:
      SetLongValue (gVMessages[INIT_MESSAGE_INVNUMERIC], errColumn );
      gMessageCount = AddToMsgArray (gMessageIDs, INIT_MESSAGE_INVNUMERIC,
                                     gMessageCount, MAX_INIT_MESSAGE_COUNT);
      break;
  }
  gVerbose = keyData[0].keyStatus != CFSTR_STATUS_NOTFOUND;

  if (DevHelp_AttachDD ("USBD$   ", (NPBYTE)&gIDCTable))
  {  // USB Driver not found
    gMessageCount = AddToMsgArray (gMessageIDs, INIT_MESSAGE_NO_USBD,
                                   gMessageCount, MAX_INIT_MESSAGE_COUNT);
    pRP->Status |= STERR | ERROR_I24_GEN_FAILURE;
  }
  else
  {  // Save the USB Driver entry point
    gpUSBDIDC = (PUSBIDCEntry)gIDCTable.ProtIDCEntry;
    gdsUSBDIDC = gIDCTable.ProtIDC_DS;

    for (index = 0; index < MAX_DEVICES; index++)
    {
      gUSBDevices[index].bAttached = FALSE;
      gUSBDevices[index].usHandleID  = 0;
      gUSBDevices[index].usSFN       = 0;
      gUSBDevices[index].pDeviceInfo = NULL;
      gUSBDevices[index].wToggle[0] = 0x0;
      gUSBDevices[index].wToggle[1] = 0x0;
    }
    /*
        Initialize Semaphore array
    */
    for (index = 0; index < MAX_SEMS; index++)
    {
      gSEMNewDev[index] = 0;
      gSEMDevRemove[index] = 0;
      gDeviceSEM[index].hSemaphoreAdd    = 0;
      gDeviceSEM[index].hSemaphoreRemove = 0;
      gDeviceSEM[index].usVendorID       = 0;
      gDeviceSEM[index].usProductID      = 0;
      gDeviceSEM[index].usBCDDevice      = 0;
    }

	// allocate three GDT selectors for IoRB handling
	if (DevHelp_AllocGDTSelector((PSEL)&gSelIoRB, 3))
	{
	  //goto fail;
	}

    gulTotalIoRBs    = 0;
    for (index = 0; index < USB_MAX_IORBS; index++)
    {
      gaIoRB[index].ulEventDone = 0;
      gaIoRB[index].pDevice     = NULL;
      gaIoRB[index].pEndpoint   = NULL;
      gaIoRB[index].pParam      = NULL;
      gaIoRB[index].pData       = NULL;
      gaIoRB[index].ulLockData  = 0;
      gaIoRB[index].ulLockParam = 0;
      gaIoRB[index].ulID        = 0;
      gaIoRB[index].usDataRemain  = 0;
      gaIoRB[index].usDataProcessed = 0;
    }

    gusHandleCounter = 1; // Next attached device will get that handleID
    Register();

    SetLongValue (gVMessages[INIT_MESSAGE_LOADED], (ULONG)gDriverStruct.MajorVer);
    SetLongValue (gVMessages[INIT_MESSAGE_LOADED], (ULONG)gDriverStruct.MinorVer);
    gMessageCount = AddToMsgArray (gMessageIDs, INIT_MESSAGE_LOADED,
                                   gMessageCount, MAX_INIT_MESSAGE_COUNT);

    pRPO->Unit = 0;
    pRPO->CodeEnd = (USHORT)&Init;           // End of code segment
    pRPO->DataEnd = (USHORT)&gInitDataStart; // End of data segment
    pRPO->BPBArray = 0;
  }
  if (gVerbose)
    TTYWrite (gVMessages, gMessageIDs, gMessageCount);
}

#pragma optimize("", on)

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME: Register                                          */
/*                                                                    */
/* DESCRIPTIVE NAME: USB Resource Manager driver resource Registration*/
/*                                                                    */
/* FUNCTION: The function of this routine is to register              */
/*           USB Resource Manager driver and                          */
/*           corresponding adapter resources.                         */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Initialization time                                       */
/*                                                                    */
/* ENTRY POINT: Register                                              */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: none                                                        */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR: N/A                                                    */
/*                                                                    */
/* EFFECTS: ghDriver, ghAdapter recieves RM handles                   */
/*                                                                    */
/* INTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

static BOOL Register (void)
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
   return (rc == RMRC_SUCCESS);
}

