/* $ID$ */

/* Copyright 2001,2002 Markus Montkowski                                      */
/* This code is released under the Aladdin Free Public License                */
/*                    (Version 9, September 18, 2000)                         */
/* See License file in root of CVS for details                                */

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME: RES.H                                                  */
/*                                                                            */
/*   DESCRIPTIVE NAME: USB Resource Manager Driver master include file        */
/*                                                                            */
/*   FUNCTION: This module is the USB Resource Manager Driver master include  */
/*             file.                                                          */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS: None                                                       */
/*                                                                            */
/*   EXTERNAL REFERENCES: None                                                */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          00/08/14  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

// USB Resource DD specific debug definitions
                        // USB DD stack
#include "usbcmmon.h"   //    common definitions and OS/2 related includes
#include "usbmisc.h"    //    function prototypes defined in USBMISC.LIB
                        // USB Resource Manager DD
#include "restype.h"    //    type definitions
#include "resproto.h"   //    function prototypes
#include "resextrn.h"   //    external data declarations

#ifdef DEBUG
   #define dsPrint(l,s)          dsPrint5x(gUSBDevicesMsg,(l),(s),0,0,0,0,0)
   #define dsPrint1(l,s,a)       dsPrint5x(gUSBDevicesMsg,(l),(s),(a),0,0,0,0)
   #define dsPrint2(l,s,a,b)     dsPrint5x(gUSBDevicesMsg,(l),(s),(a),(b),0,0,0)
   #define dsPrint3(l,s,a,b,c)   dsPrint5x(gUSBDevicesMsg,(l),(s),(a),(b),(c),0,0)
   #define dsPrint4(l,s,a,b,c,d) dsPrint5x(gUSBDevicesMsg,(l),(s),(a),(b),(c),(d),0)
#endif


