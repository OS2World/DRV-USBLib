/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  JOYSTRAT.C                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB Joystick driver strategy routines                 */
/*                                                                            */
/*   FUNCTION: These routines handle the task time routines for the strategy  */
/*             entry point of USB Joystick driver.                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:                                                            */
/*             JOYStrategy                                                    */
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

#pragma optimize("", on)

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  JOYInitComplete                                  */
/*                                                                    */
/* DESCRIPTIVE NAME:  Initialization complete                         */
/*                                                                    */
/* FUNCTION:  The function of this routine is to clear the global     */
/*            initialization time flag.                               */
/*                                                                    */
/* NOTES: This is an immediate command that is not put on the FIFO    */
/*        queue.                                                      */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  JOYInitComplete                                      */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  pRP-> kernel request packet                                */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR:  N/A                                                   */
/*                                                                    */
/* EFFECTS: pRP->Status = STATUS_DONE, fInitTime                      */
/*                                                                    */
/* INTERNAL REFERENCES:  None                                         */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

void JOYInitComplete()
{
   RP_GENIOCTL  rp;     // GENeric IOCTL Request Packet
   USBRegClient joydata;

#ifdef DEBUG
   dsPrint (DBG_HLVLFLOW, "USBJOY: InitComplete\r\n");
#endif

   setmem ((PSZ)&rp, 0, sizeof(rp));
   rp.rph.Cmd = CMDGenIOCTL;
   rp.Category = USB_IDC_CATEGORY_CLASS;
   rp.Function = USB_IDC_FUNCTION_REGISTER;
   rp.ParmPacket = (PVOID)&joydata;
   joydata.clientIDCAddr = (PUSBIDCEntry)&JOYidc;
   joydata.clientDS = GetDS();

   USBCallIDC (gpHIDIDC, gdsHIDIDC, (RP_GENIOCTL FAR *)&rp); // register with USB HID class driver

   pRP->Status = rp.rph.Status;

   if (DevHelp_AttachDD ("GAME$   ", (NPBYTE)&gIDCGameMgr)) // obtain USBHID's IDC entry point
   {
      gpGameIDC  = 0;
      gdsGameIDC = 0;
      pRP->Status |= STERR;
      #ifdef DEBUG
         dsPrint (DBG_HLVLFLOW, "USBJOY: Init Faild to Attach to GAME$ \r\n");
      #endif
   }
   else
   {
     USBDClass  GameData;
     gpGameIDC  = (PUSBIDCEntry)gIDCTable.ProtIDCEntry;
     gdsGameIDC = gIDCTable.ProtIDC_DS;
     rp.rph.Cmd = CMDGenIOCTL;
     rp.Category = USB_IDC_CATEGORY_CLIENT;
     rp.Function = USB_IDC_FUNCTION_REGISTER;
     rp.ParmPacket = (PVOID)&GameData;
     GameData.usbIDC = (PUSBIDCEntry)&JOYidc;
     GameData.usbDS = GetDS();
     USBCallIDC (gpGameIDC, gdsGameIDC, (RP_GENIOCTL FAR *)&rp); // register with Gamedevices driver

     if(USB_IDC_RC_OK == rp.rph.Status)
     {
       g_usGameDriverId = GameData.usbDS;
       #ifdef DEBUG
         dsPrint (DBG_HLVLFLOW, "USBJOY: Registered with GAME$\r\n");
       #endif
       JOYEnumDevices(g_usGameDriverId);
     }
     pRP->Status = rp.rph.Status;
   }

#ifdef DEBUG
   dsPrint1 (DBG_HLVLFLOW, "USBJOY: InitComplete Status = %x\r\n", pRP->Status);
#endif

   return;
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  JOYStrategy                                      */
/*                                                                    */
/* DESCRIPTIVE NAME:  Strategy entry point for the USB Joystick       */
/*                    driver.                                         */
/*                                                                    */
/* FUNCTION:  The function of this routine is to call strategy worker */
/*            routine to process request packet.                      */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  JOYStrategy                                          */
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

void far JOYStrategy()
{
   _asm {                      // pointer to Request Packet (Header)
      mov word ptr pRP[0], bx
      mov word ptr pRP[2], es
   }
#ifdef DEBUG
   dsPrint1 (DBG_HLVLFLOW, "USBJOY: Strategy Command = %x\r\n", pRP->Cmd);
#endif

   switch (pRP->Cmd)
   {
     case CMDInit:
       JOYInit();
       break;
     case CMDInitComplete:
       JOYInitComplete();
       break;
     default:
       pRP->Status = STATUS_DONE | STERR | STATUS_ERR_UNKCMD;
   }
#ifdef DEBUG
   dsPrint1 (DBG_HLVLFLOW, "USBJOY: Strategy Status = %x\r\n", pRP->Status );
#endif
}
#pragma optimize("", on)

