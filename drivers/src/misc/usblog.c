#define INCL_DOS
#define LOGING
#include "usbcmmon.h"         // USB stack common definitions and OS/2 includes
#include <bsedos16.h>
#include "usbmisc.h"          // common function definitons (usbmisc.lib)

typedef VOID (LOGIDCEntry) (USHORT IDCCode, PLOG_LINE MParam);
typedef LOGIDCEntry FAR *PLOGIDCEntry;
void LogCallIDC( PLOGIDCEntry idcEntry, USHORT callingDS,  PLOG_LINE pMessage);
ULONG ULDiv(ULONG A, ULONG B);
ULONG ULMul(ULONG A, ULONG B);


#pragma alloc_text( RMCode, InitLogging )
#pragma alloc_text( RMCode, LoggingSetInitComplete )
#pragma alloc_text( RMCode, LogCallIDC )
#pragma alloc_text( RMCode, LogString )
#pragma alloc_text( RMCode, LogBuffer )
#pragma alloc_text( RMCode, setmem )
#pragma alloc_text( RMCode, movmem )
#pragma alloc_text( RMCode, ULMul )
#pragma alloc_text( RMCode, ULDiv )
//#pragma alloc_text( RMCode, dsPrint5 )

#define SYSINFO_BOOTDRV 36
#define UC unsigned char
#define UI unsigned short
#define US UI

BOOL InitLogging(MDDATA *pLogging, USHORT usLevel)
{
  setmem((PSZ)pLogging, 0, sizeof(MDDATA));
  movmem((PSZ)pLogging->szLogDriver,"SYSLOG$ ",8);
  pLogging->usLogState = 1;
  pLogging->LogLine.usMajor   = usLevel;
  pLogging->LogLine.pszString = pLogging->szLogLine;
  if(!DevHelp_AttachDD( pLogging->szLogDriver, (NPBYTE)&pLogging->IdcSYSLOG) )
    pLogging->usLogState=2;
  return TRUE;
}


// should be either called in init complete

void LoggingSetInitComplete(MDDATA *pLogging)
{
  if(pLogging && pLogging->usLogState==1)
  {
    if(!DevHelp_AttachDD( pLogging->szLogDriver, (NPBYTE)&pLogging->IdcSYSLOG) )
    {
      pLogging->usLogState=2;
      DevHelp_Beep(220,500);
    }
    else
      DevHelp_Beep(880,1000);
  }
}

#pragma  optimize("cegl",off)

void LogCallIDC( PLOGIDCEntry idcEntry, USHORT callingDS,  PLOG_LINE pMessage)
{
  if (idcEntry && callingDS && pMessage)
  {
    _asm
    {
      push  ds
      push  di
      push  si
      push  bx
      push  cx
      mov   ds,callingDS
      push  WORD PTR pMessage+2
      push  WORD PTR pMessage
      call  idcEntry
      add   sp, 4
      pop   cx
      pop   bx
      pop   si
      pop   di
      pop   ds
    }
  }
  return;
}
#pragma  optimize("cegl",on)

void LogBuffer( MDDATA *pLogging,
                USHORT usLevel,
                const char *pszBufferName,
                const char far *pBuffer,
                USHORT usBufferLength )
{
  PLOG_LINE pMessage;

  USHORT usPosBuf, usPos;
  USHORT usMaxLen, usLine;
  UCHAR  b,hn,ln;
  char * pszString;

//  if( (pLogging->LogLine.usMajor == 0) ||
//      (usLevel < pLogging->LogLine.usMajor) )
//    return;

  if(pLogging->usLogState<2)
    return;

  if (!pszBufferName)
    return;

  usMaxLen = sizeof(pLogging->szLogLine) - 1;
  pszString = pLogging->szLogLine;
  usLine=usPos=usPosBuf=0;

  while( pszBufferName[usPosBuf] && usPos<usMaxLen )
    pszString[usPos++] = pszBufferName[usPosBuf++];
  pszString[usPos++] = ':';
  pszString[usPos++] = '\r';
  pszString[usPos++] = '\n';

  usPosBuf=0;
  while( usPosBuf < usBufferLength )
  {
    b = pBuffer[usPosBuf++];
    hn = (UCHAR)((b&0xF0)>>4);
    ln = (UCHAR)(b&0x0F);
    pszString[usPos++] = (UCHAR)(hn>0x09?'A'+0x0A-hn:'0'+hn);
    pszString[usPos++] = (UCHAR)(ln>0x09?'A'+0x0A-ln:'0'+ln);
    pszString[usPos++] = ' ';
    if( (usPosBuf%16) == 0)
    {
      pszString[usPos++] = '\r';
      pszString[usPos++] = '\n';
      usLine++;
    }

    if(usLine==4)
    {
      pszString[usPos] = 0x00;

      pMessage = &pLogging->LogLine;
      pMessage->usMinor = usLevel;
      LogCallIDC( (PLOGIDCEntry)pLogging->IdcSYSLOG.ProtIDCEntry,
                  pLogging->IdcSYSLOG.ProtIDC_DS,
                  pMessage);
      usLine = 0;
      usPos  = 0;
    }
  }
  if(usPos)
  {
    pszString[usPos] = 0x00;

    pMessage = &pLogging->LogLine;
    pMessage->usMinor = usLevel;
    LogCallIDC( (PLOGIDCEntry)pLogging->IdcSYSLOG.ProtIDCEntry,
                pLogging->IdcSYSLOG.ProtIDC_DS,
                pMessage);
  }

}

#pragma  optimize("cegl",off)

void LogString( MDDATA *pLogging,
                USHORT usLevel,
                const char *fmtStr, ...)
{
  PLOG_LINE pMessage;
  USHORT far *p   = (USHORT far *)&fmtStr;
  ULONG  far *pl;
  USHORT usPosFmt =0;
  short lead0;
  short fWidth;
  short charCnt;
  USHORT usPos;
  USHORT usMaxLen;
  char * pszString;


//  if( (pLogging->LogLine.usMajor == 0) ||
//      (usLevel < pLogging->LogLine.usMajor) )
//    return;

  if(pLogging->usLogState<2)
    return;

  if (!fmtStr)
    return;

  // usMaxLen is reduced by 10 as thats the max number of digits the format prints + 1 for 0x00
  usMaxLen = sizeof(pLogging->szLogLine) - 11;
  pszString = pLogging->szLogLine;

  while( fmtStr[usPosFmt] )
  {
    usPos=0;
    while( fmtStr[usPosFmt] && usPos<usMaxLen )
    {
      charCnt =
      fWidth  =
      lead0 = 0;
      pl = NULL;
      if (fmtStr[usPosFmt] != '%')
      {
        pszString[usPos++] = fmtStr[usPosFmt++];
        continue;
      }

      usPosFmt++;
      p++;

formatLoop:
      switch (fmtStr[usPosFmt])
      {

        case '0':
          lead0++;
          usPosFmt++;
          goto formatLoop;

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          fWidth = (short)(fmtStr[usPosFmt] - '9');
          /* no break */
        case '-':
          while (fmtStr[usPosFmt] && ((fmtStr[usPosFmt] >= '1' && fmtStr[usPosFmt] <= '9') || fmtStr[usPosFmt] == '-'))
            usPosFmt++;
          goto formatLoop;

        case 'c':
          charCnt++;
          pszString[usPos++] = ((UCHAR)*p);
          break;
        case 'l':
          usPosFmt++;
          pl = (ULONG far *) p;
          p++;
          goto formatLoop;
        case 'd':
        {
          if(pl)
          {
            ULONG  i =1000000000;
            ULONG  remainder;
            USHORT oneDigitSeen = 0;
            ULONG  num = *pl;
            for (; i ; i =ULDiv( i,10 ) )
            {
              remainder = ULDiv( num , i);
              if (remainder || oneDigitSeen || i==1)
              {
                charCnt++;
                pszString[usPos++] = ((UCHAR)'0'+(UCHAR)remainder);
                num -= ULMul(remainder,i);
                oneDigitSeen++;
              }
              else if (lead0)
              {
                charCnt++;
                pszString[usPos++]= (UCHAR)'0';
              }
            }
          }
          else
          {
            USHORT i = 10000;
            USHORT remainder;
            USHORT oneDigitSeen = 0;
            USHORT num = *p;
            for (; i ; i /= 10)
            {
              remainder = num / i;
              if (remainder || oneDigitSeen || i==1)
              {
                charCnt++;
                pszString[usPos++] = ((UCHAR)'0'+(UCHAR)remainder);
                num -= (remainder*i);
                oneDigitSeen++;
              }
              else if (lead0)
              {
                charCnt++;
                pszString[usPos++]= (UCHAR)'0';
              }
            }
          }
          break;
        }

        case 's':
        {
          UC *s = (UC *)*p;
          while (*s && (usPos<=usMaxLen))
          {
            charCnt++;
            pszString[usPos++] = (*s++);
          }
          break;
        }

        case 'x':
        {
          if(pl)
          {
            ULONG i = 0x10000000;
            ULONG remainder;
            USHORT oneDigitSeen = 0;
            ULONG num = *pl;
            for (; i ; i =ULDiv(i, 0x00000010) )
            {
              remainder = ULDiv( num , i);
              if (remainder || oneDigitSeen || i==1) {
                charCnt++;
                pszString[usPos++] = (UCHAR)( (UCHAR)remainder + (((UCHAR)remainder < 0x0a) ? '0' : ('a'-10)));
                num -= ULMul(remainder,i);
                oneDigitSeen++;
              }
              else if (lead0)
              {
                charCnt++;
                pszString[usPos++] = ((UCHAR)'0');
              }
            }
          }
          else
          {
            USHORT i = 0x1000;
            USHORT remainder;
            USHORT oneDigitSeen = 0;
            USHORT num = *p;
            for (; i ; i/= 0x10)
            {
              remainder = num / i;
              if (remainder || oneDigitSeen || i==1) {
                charCnt++;
                pszString[usPos++] = (UCHAR)( (UCHAR)remainder + (((UCHAR)remainder < 0x0a) ? '0' : ('a'-10)));
                num -= (remainder*i);
                oneDigitSeen++;
              }
              else if (lead0)
              {
                charCnt++;
                pszString[usPos++] = ((UCHAR)'0');
              }
            }
          }
          break;
        }

        case '\0':
          pszString[usPos] = 0x00;
          continue;

        default:
          pszString[usPos++] = fmtStr[usPosFmt-1];
          pszString[usPos++] = fmtStr[usPosFmt];
          charCnt = fWidth;
          p--;
          break;
      }
      for (;charCnt<fWidth && (usPos<=usMaxLen);charCnt++)
        pszString[usPos++] = ((UCHAR)' ');
      usPosFmt++;
    }
    pszString[usPos] = 0x00;

    pMessage = &pLogging->LogLine;
    pMessage->usMinor = usLevel;
    LogCallIDC( (PLOGIDCEntry)pLogging->IdcSYSLOG.ProtIDCEntry,
                pLogging->IdcSYSLOG.ProtIDC_DS,
                pMessage);
  }
}

#pragma  optimize("cegl",on)

