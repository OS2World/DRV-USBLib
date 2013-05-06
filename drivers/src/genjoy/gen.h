/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  JOY.H                                                 */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB Joystick driver master include file               */
/*                                                                            */
/*   FUNCTION: This module is the USB Joystick driver master                  */
/*             include file.                                                  */
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
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          00/01/04  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

/*
    USBJOY specific debug definitions
*/
#ifdef DEBUG
   #define     dsPrint(l,s)          dsPrint5x(gUJOYMsg,(l),(s),0,0,0,0,0)
   #define     dsPrint1(l,s,a)       dsPrint5x(gUJOYMsg,(l),(s),(a),0,0,0,0)
   #define     dsPrint2(l,s,a,b)     dsPrint5x(gUJOYMsg,(l),(s),(a),(b),0,0,0)
   #define     dsPrint3(l,s,a,b,c)   dsPrint5x(gUJOYMsg,(l),(s),(a),(b),(c),0,0)
   #define     dsPrint4(l,s,a,b,c,d) dsPrint5x(gUJOYMsg,(l),(s),(a),(b),(c),(d),0)
#else
   #define     dsPrint(l,s)
   #define     dsPrint1(l,s,a)
   #define     dsPrint2(l,s,a,b)
   #define     dsPrint3(l,s,a,b,c)
   #define     dsPrint4(l,s,a,b,c,d)
#endif

#include "usbcmmon.h"   /* USB device driver stack common definitions and
                           OS/2 relateded includes.   */
#include "usbmisc.h"    /* Commom USB device driver stack function prototypes
                           defined in usbmisc.lib.    */
#include "usbchid.h"
#include "gentype.h"    // USB Joystick driver typedefs
#include "genproto.h"   // USB Joystick driver function prototypes
#include "genextrn.h"   // USB Joystick driver external data declarations

#define FP_SEG(fp) (*((USHORT FAR *)&(fp)+1))
#define FP_OFF(fp) (*((USHORT FAR *)&(fp)))
#define MK_FP(seg,off) (void FAR *)(((ULONG)(((ULONG)(seg)<<16)+(ULONG)(off))))

