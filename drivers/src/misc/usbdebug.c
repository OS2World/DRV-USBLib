/* SCCSID = "src/dev/usb/MISC/USBDEBUG.C, usb, c.basedd 98/07/10" */
/*
*   Licensed Material -- Property of IBM
*
*   (c) Copyright IBM Corp. 1997, 1998  All Rights Reserved
*/

/*
 * usbdebug.c
 */

#include "usbdebug.h"
#include "usbmisc.h"

#define CR 0x0d
#define LF 0x0a

#define LEADING_ZEROES          0x8000
#define SIGNIFICANT_FIELD       0x0007

#define NUM_ARGS 5

void writeChar(char c);
PSZ DecWordToASCII(PSZ StrPtr, USHORT wDecVal, USHORT Option);
PSZ DecLongToASCII(PSZ StrPtr, ULONG lDecVal, USHORT Option);
PSZ HexWordToASCII(PSZ StrPtr, USHORT wHexVal, USHORT Option);
PSZ HexLongToASCII(PSZ StrPtr, ULONG wHexVal, USHORT Option);
void strPrint(PSZ BuildString,PSZ DbgStr,ULONG arg1,ULONG arg2,ULONG arg3,ULONG  arg4,ULONG  arg5);

#pragma alloc_text( RMCode, writeChar )
#pragma alloc_text( RMCode, DecWordToASCII, DecLongToASCII )
#pragma alloc_text( RMCode, HexWordToASCII, HexLongToASCII )
#pragma alloc_text( RMCode, strPrint, dsPrint5, dsPrint5x )
#pragma alloc_text( RMCode, setmem )


#pragma  optimize("cegl",off)

void writeChar(char c)
{
   UCHAR        readyByte;
   int          maxWait = 2000;
   int          uartLineStat = COMM_PORT + UART_LINE_STAT;
   int          uartData = COMM_PORT;

   // Wait for com port ready
   do
   {
      inp8(uartLineStat, readyByte);
      if (readyByte & 0x20)
      {
         break;
      }
   } while (maxWait--);

   // Send the character
   outp8(uartData, c);
}

#pragma  optimize("",on)

//-------------------- DecWordToASCII -
PSZ
DecWordToASCII(PSZ StrPtr, USHORT wDecVal, USHORT Option)
{
   BOOL fNonZero=FALSE;
   USHORT  Digit;
   USHORT  Power=10000;

   while (Power)
   {
      Digit=0;
      while (wDecVal >=Power)                   //Digit=wDecVal/Power;
      {
         Digit++;
         wDecVal-=Power;
      }

      if (Digit)
         fNonZero=TRUE;

      if (Digit ||
          fNonZero ||
          (Option & LEADING_ZEROES) ||
          ((Power==1) && (fNonZero==FALSE)))
      {
         writeChar((char)('0'+Digit));
      }

      if (Power==10000)
         Power=1000;
      else if (Power==1000)
         Power=100;
      else if (Power==100)
         Power=10;
      else if (Power==10)
         Power=1;
      else
         Power=0;
   } // end while

   return (StrPtr);
}

//-------------------- DecLongToASCII -
PSZ
DecLongToASCII(PSZ StrPtr, ULONG lDecVal, USHORT Option)
{
   BOOL    fNonZero=FALSE;
   ULONG   Digit;
   ULONG   Power=1000000000;                      // 1 billion

   while (Power)
   {
      Digit=0;                                                                        // Digit=lDecVal/Power
      while (lDecVal >=Power)                   // replaced with while loop
      {
         Digit++;
         lDecVal-=Power;
      }

      if (Digit)
         fNonZero=TRUE;

      if (Digit ||
          fNonZero ||
          (Option & LEADING_ZEROES) ||
          ((Power==1) && (fNonZero==FALSE)))
      {
         writeChar((char)('0'+Digit));
      }

      if (Power==1000000000)                    // 1 billion
         Power=100000000;
      else if (Power==100000000)
         Power=10000000;
      else if (Power==10000000)
         Power=1000000;
      else if (Power==1000000)
         Power=100000;
      else if (Power==100000)
         Power=10000;
      else if (Power==10000)
         Power=1000;
      else if (Power==1000)
         Power=100;
      else if (Power==100)
         Power=10;
      else if (Power==10)
         Power=1;
      else
         Power=0;
   }
   return (StrPtr);
}

//-------------------- HexWordToASCII -
PSZ
HexWordToASCII(PSZ StrPtr, USHORT wHexVal, USHORT Option)
{
   BOOL fNonZero=FALSE;
   USHORT Digit;
   USHORT Power = 0xF000;
   USHORT  ShiftVal=12;
   char     hexStr;

   while (Power)
   {
      Digit=(wHexVal & Power)>>ShiftVal;
      if (Digit)
         fNonZero=TRUE;

      if (Digit || fNonZero || (Option & LEADING_ZEROES) ||
          ((Power==0x0F) && (fNonZero==FALSE)))
      {
         if (Digit<=9)
            hexStr=(char)('0'+Digit);
         else
            hexStr=(char)('A'+Digit-10);
         writeChar(hexStr);
      }

      Power>>=4;
      ShiftVal-=4;
   } // end while

   return (StrPtr);
}

void
strPrint(PSZ    BuildString,
         PSZ    DbgStr,
         ULONG  arg1,
         ULONG  arg2,
         ULONG  arg3,
         ULONG  arg4,
         ULONG  arg5) {
   PSZ     BuildPtr=BuildString;
   PSZ          pStr=DbgStr;
   PSZ          SubStr;
   USHORT       wBuildOption;
   ULONG        args[NUM_ARGS];
   int          curArg = 0;

   args[0] = arg1;
   args[1] = arg2;
   args[2] = arg3;
   args[3] = arg4;
   args[4] = arg5;

   while (*pStr)
   {
      // don't overflow args
      if (curArg >= NUM_ARGS)
      {
         break;
      }

      switch (*pStr)
      {
      case '%':
         wBuildOption=0;
         pStr++;
         if (*pStr=='0')
         {
            wBuildOption|=LEADING_ZEROES;
            pStr++;
         }
         if (*pStr=='u')                                                         // always unsigned
            pStr++;

         switch (*pStr)
         {
         case 'x':
            BuildPtr=HexWordToASCII(BuildPtr, (USHORT)args[curArg++],wBuildOption);
            pStr++;
            continue;

         case 'd':
            BuildPtr=DecWordToASCII(BuildPtr, (USHORT)args[curArg++],wBuildOption);
            pStr++;
            continue;

         case 's':
            for (SubStr = (PSZ)args[curArg++]; *SubStr; SubStr++)
               writeChar((char)(*SubStr));
            pStr++;
            continue;

         case 'l':
            pStr++;
            switch (*pStr)
            {
            case 'x':
               BuildPtr=HexLongToASCII(BuildPtr, (ULONG)args[curArg++],wBuildOption);
               pStr++;
               continue;

            case 'd':
               BuildPtr=DecLongToASCII(BuildPtr, (ULONG)args[curArg++],wBuildOption);
               pStr++;
               continue;
            } // end switch
            continue;                        // dunno what he wants

         case 0:
            continue;
         } // end switch
         break;

      case '\\':
         pStr++;
         switch (*pStr)
         {
         case 'n':
            writeChar(LF);
            pStr++;
            continue;

         case 'r':
            writeChar(LF);
            pStr++;
            continue;

         case 0:
            continue;
            break;
         } // end switch

         break;
      } // end switch

      writeChar(*pStr);
      pStr++;
   } // end while
}

#pragma  optimize("cegl",off)
void
dsPrint5(PSZ    pFormatStr,
         ULONG  arg1,
         ULONG  arg2,
         ULONG  arg3,
         ULONG  arg4,
         ULONG  arg5)
{
   USHORT   flags;
   USHORT  oldes;

   _asm
   {
      pushf
      pop   flags

      pusha
      mov oldes, es
   }

   CLI();

   strPrint(NULL, pFormatStr, arg1, arg2, arg3, arg4, arg5);

   if (flags&EFLAGS_IF)
   {
      STI();
   }

   _asm{
      popa
      mov  es, oldes
   }
}


void
dsPrint5x(USHORT  currLevel,
          USHORT  msgLevel,
          PSZ    pFormatStr,
          ULONG  arg1,
          ULONG  arg2,
          ULONG  arg3,
          ULONG  arg4,
          ULONG  arg5)
{
   USHORT   flags;
   USHORT   oldes;
   USHORT   level=(UCHAR)(currLevel&DBG_LEVEL_MASK);

//   if ((!(level&msgLevel) && msgLevel) || currLevel==0xffff)   // suppresses output at all if level set to 0xffff
//      return;

   _asm
   {
      pushf
      pop   flags

      pusha
      mov oldes, es
   }

   if (currLevel&DBG_FLAGS_CLI)   //  disable interrupts during message printing if required
      CLI();

   strPrint(NULL, pFormatStr, arg1, arg2, arg3, arg4, arg5);

   if ((currLevel&DBG_FLAGS_CLI) && (flags&EFLAGS_IF))
   {
      STI();
   }

   _asm{
      popa
      mov  es, oldes
   }
}
#pragma  optimize("",on)
