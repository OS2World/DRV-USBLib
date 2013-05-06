/* $ID$ */

/* Copyright 2001,2002 Markus Montkowski                                      */
/* This code is released under the Aladdin Free Public License                */
/*                    (Version 9, September 18, 2000)                         */
/* See License file in root of CVS for details                                */

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME: RESSTRAT.C                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME: USB Resource Manager driver Strategy routines          */
/*                                                                            */
/*   FUNCTION: Strategy routines are called at task-time to handle            */
/*             I/O requests through a request packet interface with           */
/*             the OS/2 kernel as a result of an application I/O request.     */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS: Strategy                                                   */
/*                 CmdError                                                   */
/*                 Open                                                       */
/*                 Close                                                      */
/*                 IOCtl                                                      */
/*                 InitComplete                                               */
/*                                                                            */
/*   EXTERNAL REFERENCES: FuncError                                           */
/*                        GetNumDevices                                       */
/*                        GetDeviceInfo                                       */
/*                        RegisterSemaphore                                   */
/*                        DeregisterSemaphore                                 */
/*                        USBCallIDC                                          */
/*                        GetDS                                               */
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

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME: Strategy                                          */
/*                                                                    */
/* DESCRIPTIVE NAME: Strategy entry point                             */
/*                                                                    */
/* FUNCTION: The function of this routine is to call appropriate      */
/*           worker routine to process the OS/2 kernel request packet.*/
/*                                                                    */
/* NOTES: Strategy routine follows the 16-bit far call/return model.  */
/*        The device driver strategy routine is called with ES:BX     */
/*        pointing to the request packet.                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT: Strategy                                              */
/*     LINKAGE: CALL FAR                                              */
/*                                                                    */
/* INPUT: ES:BX = far pointer to request packet                       */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR: N/A                                                    */
/*                                                                    */
/* EFFECTS: None                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         CmdError                                      */
/*                                                                    */
/* EXTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

#pragma optimize("eglt", off)

void far Strategy (void)
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
   dsPrint2 (DBG_HLVLFLOW, "USBRESMGR: Strategy Cmd=%x unit=%x\r\n", Cmd, pRP->Unit);
#endif

   if (Cmd > CMDInitComplete)
     CmdError (pRP);
   else
     (*gStratList[Cmd])(pRP);

   pRP->Status |= STDON;

#ifdef DEBUG
   dsPrint1 (DBG_HLVLFLOW, "USBRESMGR: Strategy S=%x\r\n", pRP->Status);
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
   dsPrint1 (DBG_CRITICAL, "USBRESMGR: CmdError=%x\r\n", pRP->Cmd);
#endif

   pRP->Status = STDON | STERR | ERROR_I24_BAD_COMMAND;
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME: Open                                              */
/*                                                                    */
/* DESCRIPTIVE NAME: Open the driver                                  */
/*                                                                    */
/* FUNCTION: This strategy routine opens the driver. This is an       */
/*           immediate command that is not put on the FIFO queue.     */
/*                                                                    */
/* NOTES: Strategy CMDOpen = 13 = 0x0D.                               */
/*        The System File Number is a unique number associated with   */
/*        an Open request.                                            */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT: Open                                                  */
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

void Open (PRPH pRP)
{
   USHORT   index;  // DCB or OFH index

#ifdef DEBUG
   dsPrint2 (DBG_HLVLFLOW, "USBRESMGR: Open unit=%x sfn=%x\r\n",
             pRP->Unit, ((PRP_OPENCLOSE)pRP)->sfn);
#endif

  for (index = 0; index < MAX_OFHS; index++)
  {
    if (gOFH[index].fileHandle == 0)
    {
      gOFH[index].fileHandle = ((PRP_OPENCLOSE)pRP)->sfn;   // System File Number
      gOFH[index].shareMode = 0;
      break;
    }
  } // index = OFH index

  if (index >= MAX_OFHS)
  {
     pRP->Status |= STERR | ERROR_TOO_MANY_OPEN_FILES;
  }
  else
    if (gOFH[index].fileHandle == 0)
    {  // sfn == 0
       pRP->Status |= STERR | ERROR_INVALID_HANDLE;
    }
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME: Close                                             */
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

void Close (PRPH pRP)
{
   USHORT   index,DevIndex; // DCB or OFH

#ifdef DEBUG
   dsPrint2 (DBG_HLVLFLOW, "USBRESMGR: Close unit=%x sfn=%x\r\n",
             pRP->Unit, ((PRP_OPENCLOSE)pRP)->sfn);
#endif

  for (index = 0; index < MAX_OFHS; index++)
  {
    if (gOFH[index].fileHandle == ((PRP_OPENCLOSE)pRP)->sfn)
    {
      gOFH[index].prtIndex = MAX_DEVICES;
      gOFH[index].fileHandle = 0;
      break;
    }
  }
  for (DevIndex = 0; DevIndex < MAX_DEVICES; DevIndex++)
  {
    if(gUSBDevices[DevIndex].usSFN == ((PRP_OPENCLOSE)pRP)->sfn)
    {
      gUSBDevices[DevIndex].usSFN = 0; // Device closed no longer aquired
    }
  }
  if (index >= MAX_OFHS)
  {
    pRP->Status |= STERR | ERROR_INVALID_HANDLE;
  }
}

static USHORT GetDeviceIndex (USHORT sfn)
{
  USHORT Index;
  for(Index=0;Index<MAX_DEVICES;Index++)
  {
    if(gUSBDevices[Index].usSFN == sfn)
      break;
  }
  return Index;
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
/*        See RESIOCTL.C for the worker routines.                     */
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

void IOCtl (PRPH pRP)
{
   UCHAR unit = pRP->Unit,
         category = ((PRP_GENIOCTL)pRP)->Category,
         function = ((PRP_GENIOCTL)pRP)->Function;

#ifdef DEBUG
   dsPrint4 (DBG_HLVLFLOW, "USBRESMGR: IOCtl unit=%x sfn=%x C=%x F=%x\r\n",
             unit, ((PRP_GENIOCTL)pRP)->sfn, category, function);
#endif

   if (category != IOC_RES)
   {
      pRP->Status |= STERR | ERROR_I24_BAD_COMMAND;
   }
   else
   {
     if (function < MIN_RES_IOCTLF || function > MAX_RES_IOCTLF)
     {
       FuncError ((PRP_GENIOCTL)pRP);
     }
     else
     {  // RESIOCTL.C
       (*gResFunc[function-MIN_RES_IOCTLF])((PRP_GENIOCTL)pRP);
     }
   }
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME: InitComplete                                      */
/*                                                                    */
/* DESCRIPTIVE NAME: Initialization Complete                          */
/*                                                                    */
/* FUNCTION: This strategy command informs the driver                 */
/*           that all physical device drivers have been loaded and    */
/*           initialized. The driver can now establish an             */
/*           link to the USB Driver.                                  */
/*                                                                    */
/* NOTES: Strategy CMDInitComplete = 31 = 0x1F.                       */
/*        This command is sent to the device driver only              */
/*        if Bit 4 (DEV_INITCOMPLETE) is set in the Capabilities Bit  */
/*        Strip in the USB driver header (See RESSEGS.ASM).           */
/*                                                                    */
/* CONTEXT: Initialization time                                       */
/*                                                                    */
/* ENTRY POINT: InitComplete                                          */
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
/* INTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:         USBCallIDC                                    */
/*                      GetDS                                         */
/*                      setmem                                        */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

void InitComplete (PRPH pRP)
{
   RP_GENIOCTL rp;      // GENeric IOCTL Request Packet
   USBDClass   prtData;

#ifdef DEBUG
   dsPrint (DBG_HLVLFLOW, "USBRESMGR: InitComplete\r\n");
#endif

   setmem ((PSZ)&rp, 0, sizeof(rp));
   rp.rph.Cmd = CMDGenIOCTL;
   rp.Category = USB_IDC_CATEGORY_USBD;
   rp.Function = USB_IDC_FUNCTION_REGISTER;
   rp.ParmPacket = (PVOID)&prtData;
   prtData.usbIDC = (PUSBIDCEntry)&IDCEntry;
   prtData.usbDS = GetDS();

   USBCallIDC (gpUSBDIDC, gdsUSBDIDC, (RP_GENIOCTL FAR *)&rp);

   pRP->Status |= rp.rph.Status;
}

