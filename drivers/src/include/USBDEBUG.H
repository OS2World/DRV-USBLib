#ifndef  _usbdebug_h_
   #define  _usbdebug_h_
/* SCCSID = "src/dev/usb/INCLUDE/USBDEBUG.H, usb, c.basedd 98/07/10" */
/*
*   Licensed Material -- Property of IBM
*
*   (c) Copyright IBM Corp. 1997, 1998  All Rights Reserved
*/
   #include  "usbcmmon.h"

void    dsPrint5(PSZ s, ULONG par1, ULONG par2, ULONG par3, ULONG par4, ULONG par5);
void    dsPrint5x(USHORT  currLevel, USHORT  msgLevel, PSZ s, ULONG par1, ULONG par2, ULONG par3, ULONG par4, ULONG par5);

   #ifndef         dsPrint
      #define         dsPrint(s)              dsPrint5((s),0,0,0,0,0)
      #define         dsPrint1(s,a)           dsPrint5((s),(a),0,0,0,0)
      #define         dsPrint2(s,a,b)         dsPrint5((s),(a),(b),0,0,0)
      #define         dsPrint3(s,a,b,c)       dsPrint5((s),(a),(b),(c),0,0)
      #define         dsPrint4(s,a,b,c,d)     dsPrint5((s),(a),(b),(c),(d),0)
   #endif

   #define BUILD_STR_SIZE  512

//   #define COMM_PORT_1     0x180
   #define COMM_PORT_1     0x2f8
   #define COMM_PORT_2     0x3f8
   #define COMM_PORT       COMM_PORT_1
   #define UART_LINE_STAT  0x5

// debug message level definitions
   #define  DBG_CRITICAL      0
   #define  DBG_HLVLFLOW      1  // all strategy calls, hardware IRQ, timer interrupt calls
   #define  DBG_IRQFLOW       2
   #define  DBG_DETAILED      4
   #define  DBG_SPECIFIC      8
   #define  DBG_DBGSPCL       16

   #define  DBG_LEVEL_MASK    0x001f
   #define  DBG_FLAGS_CLI     0x8000

#endif

