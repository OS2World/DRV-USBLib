/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  TSTSTRAT.C                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB Log Driver strategy routines                      */
/*                                                                            */
/*   FUNCTION: These routines handle the task time routines for the strategy  */
/*             entry point of the USB Log Driver.                             */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:                                                            */
/*             LogStrategy                                                    */
/*             CmdError                                                       */
/*             LogRead                                                        */
/*             LogInStatus                                                    */
/*             LogInFlush                                                     */
/*             LogOpen                                                        */
/*             LogClose                                                       */
/*             LogInitComplete                                                */
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

//static void  COMRequest (SetupPacket FAR *pSetPack, PUCHAR pReqData, ULONG comIRQ);

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  LogStrategy                                      */
/*                                                                    */
/* DESCRIPTIVE NAME: Strategy entry point                             */
/*                   for the USB Communication Device Class Driver    */
/*                                                                    */
/* FUNCTION:   The function of this routine is                        */
/*             to call appropriate worker routine                     */
/*             to process the OS/2 kernel request packet.             */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  LogStrategy                                          */
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
/* INTERNAL REFERENCES:  CmdError                                     */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

#pragma optimize("eglt", off)

void far LogStrategy (void)
{
   PRPH     pRP;  // pointer to Request Packet (Header)
   USHORT   Cmd;  // strategy command

   _asm
   {
      mov   word ptr pRP[0], bx
      mov   word ptr pRP[2], es
   }
   Cmd = pRP->Cmd;

#ifdef DEBUG
   dsPrint1 (DBG_HLVLFLOW, "LOG: Strategy Cmd=%x\r\n", Cmd);
#endif

   if (Cmd > MAX_STRAT_CMD)   CmdError (pRP);
   else                       (*gStratList[Cmd])(pRP);

#ifdef DEBUG
   dsPrint1 (DBG_HLVLFLOW, "LOG: Strategy S=%x\r\n", pRP->Status);
#endif
}

#pragma optimize("", on)

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  CmdError                                         */
/*                                                                    */
/* DESCRIPTIVE NAME:  Command not supported in the device driver      */
/*                                                                    */
/* FUNCTION:  The function of this routine is to return command not   */
/*            supported for the request.                              */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  CmdError                                             */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  pRP-> kernel request packet                                */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR:  N/A                                                   */
/*                                                                    */
/* EFFECTS: pRP->Status = STDON + STERR + ERROR_I24_BAD_COMMAND       */
/*                                                                    */
/* INTERNAL REFERENCES:  None                                         */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

void CmdError (PRPH pRP)
{
#ifdef DEBUG
   dsPrint1 (DBG_CRITICAL, "LOG: Unknown Strategy Cmd = 0x%x\r\n", pRP->Cmd);
#endif

   pRP->Status = STDON | STERR | ERROR_I24_BAD_COMMAND;
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  LogRead                                          */
/*                                                                    */
/* DESCRIPTIVE NAME:  Log Driver Read                                 */
/*                                                                    */
/* FUNCTION:  This strategy command reads from the Log Driver         */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  LogRead                                              */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  pRP-> kernel request packet                                */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR:  N/A                                                   */
/*                                                                    */
/* EFFECTS: pRP->Status                                               */
/*                                                                    */
/* INTERNAL REFERENCES:  None                                         */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

#pragma optimize("eglt", off)

void LogRead (PRPH pRP)
{
   PRP_RWV pRPRead = (PRP_RWV)pRP;
   PBYTE   pBuffer;
   USHORT usRead;
   pRP->Status = STDON;

  #ifdef DEBUG
    dsPrint2 (DBG_HLVLFLOW, "LOG: Read %x bytes from sfn=%x\r\n",
              pRPRead->NumSectors, pRPRead->sfn);
  #endif

  if( !DevHelp_PhysToGDTSelector( pRPRead->XferAddr,
                                 pRPRead->NumSectors,
                                 gGDTSel) )
  {
    pBuffer = MAKEP (gGDTSel, NULL);
    usRead = 0;
    CLI();
    // Check for wrap around situation
    if(gulWritePos < gulReadPos)
    {
      if(pRPRead->NumSectors >= (USHORT)(LOG_BUFFER_SIZE - gulReadPos) )
      {
        movmem (pBuffer, &gucLogBuffer[gulReadPos], (USHORT) (LOG_BUFFER_SIZE - gulReadPos) );
        usRead = (USHORT)(LOG_BUFFER_SIZE - gulReadPos);
        gulReadPos = 0;
      }
      else
      {
        movmem (pBuffer, &gucLogBuffer[gulReadPos], pRPRead->NumSectors);
        gulReadPos  += pRPRead->NumSectors;
        return;
      }
    }

    if((pRPRead->NumSectors-usRead) >= (USHORT)(gulWritePos - gulReadPos) )
    {
      // All fits into the Read Buffer
      movmem (pBuffer, &gucLogBuffer[gulReadPos], (USHORT)(gulWritePos - gulReadPos));
      pRPRead->NumSectors = usRead + (USHORT)(gulWritePos - gulReadPos);
      gulWritePos = 0;
      gulReadPos  = 0;
    }
    else
    {
      // Not everything fits into the Read Buffer
      movmem (pBuffer, &gucLogBuffer[gulReadPos], pRPRead->NumSectors-usRead);
      gulReadPos  += pRPRead->NumSectors-usRead;
    }

    STI();
  }
  else
  {
    pRP->Status |= STERR | ERROR_I24_GEN_FAILURE;
  }
}

#pragma optimize("", on)

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  LogInStatus                                      */
/*                                                                    */
/* DESCRIPTIVE NAME:  Log Driver input status                         */
/*                                                                    */
/* FUNCTION:  This function determines input status on Log Driver     */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  LogInStatus                                          */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  pRP-> kernel request packet                                */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR:  N/A                                                   */
/*                                                                    */
/* EFFECTS: pRP->Status                                               */
/*                                                                    */
/* INTERNAL REFERENCES:  None                                         */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

void LogInStatus (PRPH pRP)
{
   pRP->Status |= STDON;

#ifdef DEBUG
   dsPrint1 (DBG_HLVLFLOW, "LOG: Input Status = 0x%x\r\n", pRP->Status);
#endif
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  LogInFlush                                       */
/*                                                                    */
/* DESCRIPTIVE NAME:  Log Driver input flush                          */
/*                                                                    */
/* FUNCTION:  This function flushes input queue                       */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  LogInFlush                                           */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  pRP-> kernel request packet                                */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR:  N/A                                                   */
/*                                                                    */
/* EFFECTS: pRP->Status                                               */
/*                                                                    */
/* INTERNAL REFERENCES:  None                                         */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

void LogInFlush (PRPH pRP)
{

#ifdef DEBUG
   dsPrint (DBG_HLVLFLOW, "LOG: Input Flush\r\n");
#endif
   pRP->Status = STDON;
   gulWritePos = 0;
   gulReadPos  = 0;
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  LogOpen                                          */
/*                                                                    */
/* DESCRIPTIVE NAME:  Log Driver open                                 */
/*                                                                    */
/* FUNCTION:  The function of this routine is to open the Log Driver  */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  LogOpen                                              */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  pRP-> kernel request packet                                */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR:  N/A                                                   */
/*                                                                    */
/* EFFECTS: pRP->Status                                               */
/*                                                                    */
/* INTERNAL REFERENCES:  SetSignals                                   */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

void LogOpen (PRPH pRP)
{

#ifdef DEBUG
   dsPrint1 (DBG_HLVLFLOW, "LOG: Open sfn=%x\r\n", ((PRP_OPENCLOSE)pRP)->sfn);
#endif

   pRP->Status = STDON;
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  LogClose                                         */
/*                                                                    */
/* DESCRIPTIVE NAME:  Log Driver close                                */
/*                                                                    */
/* FUNCTION:  The function of this routine is to close the Log Driver */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  LogClose                                             */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  pRP-> kernel request packet                                */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR:  N/A                                                   */
/*                                                                    */
/* EFFECTS: pRP->Status                                               */
/*                                                                    */
/* INTERNAL REFERENCES:  SendBreak                                    */
/*                       SetSignals                                   */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

void LogClose (PRPH pRP)
{

#ifdef DEBUG
   dsPrint1 (DBG_HLVLFLOW, "LOG: Close sfn=%x\r\n", ((PRP_OPENCLOSE)pRP)->sfn);
#endif

   pRP->Status = STDON;
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  LogInitComplete                                  */
/*                                                                    */
/* DESCRIPTIVE NAME:  Initialization complete                         */
/*                                                                    */
/* FUNCTION:  The function of this routine is to finish driver's      */
/*            initialization.                                         */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Initialization time                                       */
/*                                                                    */
/* ENTRY POINT:  LogInitComplete                                      */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  pRP-> kernel request packet                                */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR:  N/A                                                   */
/*                                                                    */
/* EFFECTS: pRP->Status                                               */
/*                                                                    */
/* INTERNAL REFERENCES:  None                                         */
/*                                                                    */
/* EXTERNAL REFERENCES:  USBCallIDC                                   */
/*                       GetDS                                        */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

void LogInitComplete (PRPH pRP)
{
  #ifdef DEBUG
    dsPrint (DBG_HLVLFLOW, "LOG: InitComplete\r\n");
  #endif
//   DevHelp_Beep( 220, 1000 );
   pRP->Status = STDON;
}



