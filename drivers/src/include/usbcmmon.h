#ifndef  _usbcmmon_h_
   #define  _usbcmmon_h_
/* SCCSID = "src/dev/usb/INCLUDE/USBCMMON.H, usb, c.basedd 98/07/10" */
/*
*   Licensed Material -- Property of IBM
*
*   (c) Copyright IBM Corp. 1997 - 2000  All Rights Reserved
*/
/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  USBCMMON.H                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB device driver stack common definitions and        */
/*                      OS/2 related includes.                                */
/*                                                                            */
/*   FUNCTION: This header file contains UHCI specific data structure and     */
/*                      flag definitions                                      */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:  None                                                      */
/*                                                                            */
/*   EXTERNAL REFERENCES:  None                                               */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark      yy/mm/dd  Programmer    Comment                                 */
/*  --------- --------  ----------    -------                                 */
/*            97/12/18  MB                                                    */
/* 08/08/2000 00/08/08  MB            Changed 32 bit register operation       */
/*                                    macro definitions to pure 32 bit in/out */
/*                                    commands (fixes configuration problem   */
/*                                    on Dell Dimension XPS PIII 700)         */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

/*----------------------------------------------*/
/* assembler commands                           */
/*----------------------------------------------*/
   #define CLI()  _asm{cli}
   #define STI()  _asm{sti}
   #define CLC()  _asm{clc}
   #define STC()  _asm{stc}

// eflags register flag definitions
   #define  EFLAGS_IF   0x0200   // interrupt flag

/*
** I/O Instruction macro
*/
/* 8 bit OUT */
   #define outp8(port, data) _asm{ \
      _asm    push ax          \
      _asm    push dx          \
      _asm    mov  al,data     \
      _asm    mov  dx,port     \
      _asm    out  dx,al       \
      _asm    pop  dx          \
      _asm    pop  ax          \
}

/* 8 bit IN */
   #define inp8(port, data) _asm{  \
      _asm    push ax          \
      _asm    push dx          \
      _asm    mov  dx,port     \
      _asm    in   al,dx       \
      _asm    mov  data,al     \
      _asm    pop  dx          \
      _asm    pop  ax          \
}

/* 16 bit OUT */
   #define outp16(port, data) _asm{ \
      _asm    push ax          \
      _asm    push dx          \
      _asm    mov  ax,data     \
      _asm    mov  dx,port     \
      _asm    out  dx,ax       \
      _asm    pop  dx          \
      _asm    pop  ax          \
}

/* 16 bit IN */
   #define inp16(port, data) _asm{  \
      _asm    push ax          \
      _asm    push dx          \
      _asm    xor  ax,ax       \
      _asm    mov  dx,port     \
      _asm    in   ax,dx       \
      _asm    mov  data,ax     \
      _asm    pop  dx          \
      _asm    pop  ax          \
}

/* 32 bit OUT */  // 08/08/2000 MB
   #define outp32(port, data) { \
      USHORT  sData[2];          \
      *((PULONG)&sData)=data;    \
         _asm {                  \
      _asm _emit   66h           \
      _asm    push ax            \
      _asm    push dx            \
      _asm _emit   66h           \
      _asm    mov  ax,sData[0]   \
      _asm    mov  dx,port       \
      _asm _emit   66h           \
      _asm    out  dx,ax         \
      _asm    pop  dx            \
      _asm _emit   66h           \
      _asm    pop  ax            \
      } \
      }

/* 32 bit IN */  // 08/08/2000 MB
   #define inp32(port, data) { \
      USHORT  sData[2];          \
         _asm {                  \
      _asm _emit   66h           \
      _asm    push ax            \
      _asm    push dx            \
      _asm _emit   66h           \
      _asm    xor  ax,ax         \
      _asm    mov  dx,port       \
      _asm _emit   66h           \
      _asm    in   ax,dx         \
      _asm _emit   66h           \
      _asm    mov  sData[0],ax   \
      _asm    pop  dx            \
      _asm _emit   66h           \
      _asm    pop  ax            \
      }                          \
      data=*((PULONG)&sData);    \
      }


   #define INCL_NOBASEAPI
   #define INCL_NOPMAPI
   #define INCL_ERRORS
   #define INCL_16

   #include <os2.h>              /* special types  includes os2defs.h */
   #include <bseerr.h>           /* error definitions */
   #include <OS2DEFP.H>          /* special types  includes os2defs.h */
   #include <devhdr.h>           /* device driver header definitions */
   #include <devcmd.h>           /* device driver strategy commands */
   #include <strat2.h>           /* defines P_DriverCaps for reqpkt.h. */
   #include <reqpkt.h>           /* request packet structure definitions */
   #include <dhcalls.h>          /* device helper calls */
   #include <rmcalls.h>          /* resource manager calls */
   #include <infoseg.h>          /* InfoSegGDT definition */
   #include <dskinit.h>          /* init request packet structure definitions */

   #ifdef DEBUG
      #include "usbdebug.h"         // debug stuff definitions
   #endif

   #include "usbtype.h"         // USB device common data definitions

   #include "usbidc.h"          /* USB IDC common typedefs */
   #ifdef LOGING
     #include "usblog.h"
   #endif
#endif

