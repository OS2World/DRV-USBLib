/* SCCSID = "src/dev/usb/MISC/USBMISC.C, usb, c.basedd 98/07/10" */
/*
*   Licensed Material -- Property of IBM
*
*   (c) Copyright IBM Corp. 1997, 1998  All Rights Reserved
*/
/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  USBMISC.C                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB device driver stack miscellaneous                 */
/*                      routines.                                             */
/*                                                                            */
/*   FUNCTION: These routines handle the miscellaneous utility functions      */
/*             for the USB port device driver.                                */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:                                                            */
/*             ConvertCharToStr                                               */
/*             STIRestore                                                     */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          96/03/01  Frank Schroeder Original developer.                     */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#include "usbmisc.h"

#pragma alloc_text( RMCode, ConvertCharToStr )
#pragma alloc_text( RMCode, STIRestore )

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  ConvertCharToStr                                 */
/*                                                                    */
/* DESCRIPTIVE NAME:  convert character to ASCII string               */
/*                                                                    */
/* FUNCTION:  The function of this routine is to convert single byte  */
/*            integer (character) to ASCII string.                    */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  ConvertCharToStr                                     */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  UCHAR source - binary value to convert                     */
/*         PSZ target - receving string                               */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR:  none                                                  */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:  none                                         */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
void ConvertCharToStr( UCHAR source, PSZ target )
{
   BOOL    fNonZero=FALSE;
   UCHAR   Digit;
   UCHAR   Power=100;

   while (Power)
   {
      Digit=0;                                 // Digit=lDecVal/Power
      while (source >=Power)                   // replaced with while loop
      {
         Digit++;
         source-=Power;
      }

      if (Digit)
         fNonZero=TRUE;

      if (Digit || fNonZero ||
          ((Power==1) && (fNonZero==FALSE)))
      {
         *target=(char)('0'+Digit);
         target++;
      }

      if (Power==100)
         Power=10;
      else if (Power==10)
         Power=1;
      else
         Power=0;
   }
   *target=0;
}

USHORT LenStr(PSZ String)
{
  USHORT usLen=0;
  if(String)
  {
    while(*String++)usLen++;
    usLen++;
  }
  return usLen;
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  STIRestore                                       */
/*                                                                    */
/* DESCRIPTIVE NAME:  Restore interrupt flag status                   */
/*                                                                    */
/* FUNCTION:  This function enables interrupts if interrupt enabled   */
/*            flag is on in flags parameter.                          */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  STIRestore                                           */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  USHORT flags - status flags register                       */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR:  none                                                  */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:  none                                         */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
#pragma optimize("eglt", off)
VOID NEAR STIRestore(USHORT flags)
{
   if (flags&EFLAGS_IF)
      STI();
}
#pragma optimize("", on)

