/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  TSTIDC.C                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME:  Log Driver inter-device driver                        */
/*                      communication routines                                */
/*                                                                            */
/*   FUNCTION: These routines handle the PDD-PDD IDC for the                  */
/*             USB Log Driver.                                                */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS: LogIDC          PDD - PDD IDC worker switch                */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          03/01/12  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#include "log.h"


int _cdecl
sprintfn( char* pszString, USHORT usMaxLen, const char *fmtStr, ... )
{
  USHORT far *p = (USHORT far*)&fmtStr;
  short lead0;
  short fWidth;
  short charCnt;
  USHORT usPos = 0;
  USHORT usPosFmt = 0;
  usMaxLen --; // Space for the 0 byte

  if (!fmtStr || !pszString)
    return(0);

  while (fmtStr[usPosFmt] && usPos<usMaxLen)
  {
    charCnt =
    fWidth  =
    lead0 = 0;

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
      case 'd':
      {
        USHORT i = 10000;
        USHORT remainder;
        USHORT oneDigitSeen = 0;
        USHORT num = *p;
        for (; i && (usPos<usMaxLen); i /= 10)
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
        break;
      }

      case 's':
      {
        UCHAR *s = (UCHAR *)*p;
        while (*s && (usPos<=usMaxLen))
        {
          charCnt++;
          pszString[usPos++] = (*s++);
        }
        break;
      }

      case 'x':
      {
        USHORT i = 0x1000;
        USHORT remainder;
        USHORT oneDigitSeen = 0;
        USHORT num = *p;
        for (; i && (usPos<=usMaxLen); i/= 0x10)
        {
          remainder = num / i;
          if (remainder || oneDigitSeen || i==1) {
            charCnt++;
            pszString[usPos++] = ( (UCHAR)remainder + (((UCHAR)remainder < 0x0a) ? '0' : ('a'-10)));
            num -= (remainder*i);
            oneDigitSeen++;
          }
          else if (lead0)
          {
            charCnt++;
            pszString[usPos++] = ((UCHAR)'0');
          }
        }
        break;
      }

      case '\0':
        pszString[usPos] = 0x00;
        return(0);

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
  return(0);
}


/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME:  LogIDC                                           */
/*                                                                    */
/* DESCRIPTIVE NAME:  PDD-PDD IDC entry point and request router      */
/*                                                                    */
/* FUNCTION:  This routine is the PDD-PDD IDC entry point and         */
/*            request router. IDC function requests are routed        */
/*            to the appropriate worker routine. The address of       */
/*            this routine is returned to other device drivers via    */
/*            the DevHelp AttachDD call.                              */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT :  LogIDC                                              */
/*    LINKAGE  :  CALL FAR                                            */
/*                                                                    */
/* INPUT:  PRP_GENIOCTL pRP_GENIOCTL                                  */
/*                                                                    */
/* EXIT-NORMAL:  n/a                                                  */
/*                                                                    */
/* EXIT-ERROR:  n/a                                                   */
/*                                                                    */
/* EFFECTS:  sets error code in pRP_GENIOCTL->rph.Status              */
/*           USB_IDC_RC_PARMERR - 1) request command code is not      */
/*                                CMDGenIOCTL; 2) parameter packet    */
/*                                address is NULL;                    */
/*           USB_IDC_RC_WRONGFUNC - wrong function requested          */
/*           USB_IDC_RC_WRONGCAT - wrong request category             */
/*                                                                    */
/* INTERNAL REFERENCES:  LogService                                   */
/*    ROUTINES:          LogDetach                                    */
/*                                                                    */
/* EXTERNAL REFERENCES:  LogIrq                                       */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

#pragma optimize("", off)

void FAR LogIDC (PLOG_LINE pLine)
{
  PSZ pDrvName;
  USHORT i, usLength;
  USHORT usPos;
  ULONG  ulTimeStamp;
  BOOL fCheckOR;
  pDrvName = (PSZ)pLine;
  OFFSETOF (pDrvName) = 10;

  ulTimeStamp = *gpTime;
  #ifdef DEBUG
    dsPrint (DBG_HLVLFLOW, "LOG: IDC\r\n");
  #endif

  // get Length of String;
  CLI();
  for(usLength =0;pLine->pszString[usLength];usLength ++);

  usPos = (USHORT)gulWritePos;
  fCheckOR = gulReadPos>gulWritePos;
  if(gulWritePos+31+usLength < (ULONG)LOG_BUFFER_SIZE)
  {
    //Mark Start
    gucLogBuffer[usPos++] = '#';

    // Copy Driver name
    for(i=0;i<8;i++)
      gucLogBuffer[usPos++] = (UCHAR)(pDrvName[i]?pDrvName[i]:' ');

    sprintfn( &gucLogBuffer[usPos], 21 ,
              "-%0x-%0x-%0x%0x:",
              pLine->usMajor, pLine->usMinor,
              HIUSHORT(ulTimeStamp), LOUSHORT(ulTimeStamp) );
    usPos +=20;
    for(i=0;i<usLength;i++)
      gucLogBuffer[usPos++] = pLine->pszString[i];
    // Mark End Of buffer
    gucLogBuffer[usPos++] = 0x00;
  }
  else
  {
    if(usPos>=LOG_BUFFER_SIZE)
      usPos = 0;
    gucLogBuffer[usPos++] = '#';

    for(i=0;i<8;i++)
    {
      if(usPos>=LOG_BUFFER_SIZE)
        usPos = 0;
      gucLogBuffer[usPos++] = (UCHAR)(pDrvName[i]?pDrvName[i]:' ');
    }
    sprintfn( gszStamp, 21 ,
              "-%0x-%0x-%0x%0x:",
              pLine->usMajor, pLine->usMinor,
              HIUSHORT(ulTimeStamp), LOUSHORT(ulTimeStamp) );
    for(i=0;i<20;i++)
    {
      if(usPos>=LOG_BUFFER_SIZE)
        usPos = 0;
      gucLogBuffer[usPos++] = gszStamp[i];
    }

    for(i=0;i<usLength;i++)
    {
      if(usPos>=LOG_BUFFER_SIZE)
        usPos = 0;
      gucLogBuffer[usPos++] = pLine->pszString[i];
    }
    if(usPos>=LOG_BUFFER_SIZE)
      usPos = 0;

    gucLogBuffer[usPos++] = 0x00;

    if(usPos>=LOG_BUFFER_SIZE)
      usPos = 0;
  }
  // advance writePos
  gulWritePos = usPos;

  if(fCheckOR & gulWritePos>gulReadPos)
  {
     gulReadPos = gulWritePos;
     for( ;gucLogBuffer[gulReadPos] && gulReadPos < LOG_BUFFER_SIZE ; gulReadPos++);
     if(gulReadPos==LOG_BUFFER_SIZE)
       for( gulReadPos=0; gucLogBuffer[gulReadPos] && gulReadPos<LOG_BUFFER_SIZE; gulReadPos++);

  }
  STI();
}

#pragma optimize("", on)



