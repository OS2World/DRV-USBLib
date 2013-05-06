/* SCCSID = "src/dev/usb/MISC/USBCIDC.C, usb, c.basedd 98/07/10" */
/*
*   Licensed Material -- Property of IBM
*
*   (c) Copyright IBM Corp. 1997,1998  All Rights Reserved
*/
/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  USBCIDC.C                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:  Common IDC routines.                                  */
/*                                                                            */
/*   FUNCTION: These routines handle IDC calls.                               */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:                                                            */
/*             USBCallIDC                                                     */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          98/01/31  MB                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#include "usbidc.h"


/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  USBCallIDC                                       */
/*                                                                    */
/* DESCRIPTIVE NAME:  Calls IDC routine                               */
/*                                                                    */
/* FUNCTION:  The function of this routine is calling specific IDC    */
/*            routine, setting required DS value and passing  request */
/*            block address as parameter.                             */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  USBCallIDC                                           */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  PUSBIDCEntry idcEntry - far pointer to IDC routine         */
/*         USHORT callingDS - IDC routine data segment                */
/*         RP_GENIOCTL FAR *pRP - far pointer to parameter block      */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR:  N/A                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:  None                                         */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
#pragma alloc_text( RMCode, USBCallIDC )

#pragma  optimize("cegl",off)
void USBCallIDC( PUSBIDCEntry idcEntry, USHORT callingDS, RP_GENIOCTL FAR *pRP )
{
   if (idcEntry && callingDS && pRP)
   {
      _asm
      {
         push  ds
         push  di
         push  si
         push  bx
         push  cx
         mov   ds,callingDS
         push  WORD PTR pRP+2
         push  WORD PTR pRP
         call  idcEntry
         add   sp,4
         pop   cx
         pop   bx
         pop   si
         pop   di
         pop   ds
      }
   }
   return;
}



