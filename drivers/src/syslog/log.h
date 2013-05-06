/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  TESTLOG.H                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB Log Driver                                        */
/*                      master include file.                                  */
/*                                                                            */
/*   FUNCTION: This module is the USB Log Driver master include file.         */
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
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

/*
   USBCOM specific debug definitions
*/
#ifdef DEBUG
   #define dsPrint(l,s)          dsPrint5x(gLogMsg,(l),(s),0,0,0,0,0)
   #define dsPrint1(l,s,a)       dsPrint5x(gLogMsg,(l),(s),(a),0,0,0,0)
   #define dsPrint2(l,s,a,b)     dsPrint5x(gLogMsg,(l),(s),(a),(b),0,0,0)
   #define dsPrint3(l,s,a,b,c)   dsPrint5x(gLogMsg,(l),(s),(a),(b),(c),0,0)
   #define dsPrint4(l,s,a,b,c,d) dsPrint5x(gLogMsg,(l),(s),(a),(b),(c),(d),0)
#endif

#define LOGING

#include "usbcmmon.h"   // USB device driver stack common header
#include "usbmisc.h"    // commom USB device driver stack function prototypes

#include "logtype.h"    // typedefs
#include "logproto.h"   // function prototypes
#include "logextrn.h"   // external data declarations

