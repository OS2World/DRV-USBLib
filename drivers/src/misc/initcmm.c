/* SCCSID = "src/dev/usb/MISC/INITCMM.C, usb, c.basedd 98/07/10" */
/*
*   Licensed Material -- Property of IBM
*
*   (c) Copyright IBM Corp. 1997 - 1999 All Rights Reserved
*/
/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  INITCMM.C                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:  Common USB stack initialization routines              */
/*                      routines.                                             */
/*                                                                            */
/*   FUNCTION: These routines handle the presence check for the USB           */
/*             ports.                                                         */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:                                                            */
/*             TTYWrite                                                       */
/*             AddToMsgArray                                                  */
/*             SetLongValue                                                   */
/*             SetHex                                                         */
/*             SetDec                                                         */
/*             ProcessConfigString                                            */
/*             strcmp                                                         */
/*             ConvertDec                                                     */
/*             ConvertStr                                                     */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark       yy/mm/dd  Programmer   Comment                                 */
/*  ---------- --------  ----------   -------                                 */
/*             98/01/31  MB                                                   */
/*  22/04/1999 99/04/22  MB           Fixed conversion to hex routine         */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#include "usbmisc.h"

extern USHORT        gVerbose;
extern MSGTABLE      gInitMsg;

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  TTYWrite                                         */
/*                                                                    */
/* DESCRIPTIVE NAME:  "Write to console" routine.                     */
/*                                                                    */
/* FUNCTION:  The function of this routine is to write out driver's   */
/*            initialization time messages.                           */
/*                                                                    */
/* NOTES: Messages are written to console only if gVerbose value is   */
/*        non-zero - /V parameter was specified in CONFIG.SYS         */
/*                                                                    */
/* CONTEXT: Initialization time                                       */
/*                                                                    */
/* ENTRY POINT:  TTYWrite                                             */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  PSZ *Buf - pointer to message string addresses             */
/*         USHORT msgIds[] - message indexes to be written out        */
/*         USHORT msgCount - no of messages to be written out         */
/*                                                                    */
/* EXIT-NORMAL: n/a                                                   */
/*                                                                    */
/* EXIT-ERROR:  n/a                                                   */
/*                                                                    */
/* EFFECTS:  none                                                     */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*                                                                    */
/* EXTERNAL REFERENCES:  DevHelp_Save_Message                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
VOID TTYWrite( PSZ *Buf, USHORT msgIds[], USHORT msgCount )
{
   USHORT      msgIndex;
   UCHAR       severityLevel;
   PSZ         messageString;

   if (!gVerbose)  // message printing disabled
      return;

   for (msgIndex=0; msgIndex<msgCount; msgIndex++)
   {
      messageString=Buf[msgIds[msgIndex]];
      severityLevel=(UCHAR)*messageString;
      if (severityLevel)
         gInitMsg.MsgStrings[0] = messageString+1;

      DevHelp_Save_Message( (NPBYTE) &gInitMsg );
   }
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  AddToMsgArray                                    */
/*                                                                    */
/* DESCRIPTIVE NAME:  Add message index to message index array.       */
/*                                                                    */
/* FUNCTION:  The function of this routine is to add new message      */
/*            index to index array with boundary checking.            */
/*                                                                    */
/* NOTES: This routine ignores request to add new index if there are  */
/*        no space left in index array.                               */
/*                                                                    */
/* CONTEXT: Initialization time                                       */
/*                                                                    */
/* ENTRY POINT:  AddToMsgArray                                        */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  USHORT msgIds[] - message indexes to be written out        */
/*         USHORT msgIndex - message indexe to be added to array      */
/*         USHORT currCount - no of messages currently in array       */
/*         USHORT maxCount - max no of elements in index array        */
/*                                                                    */
/* EXIT-NORMAL: current number of indexes in array                    */
/*                                                                    */
/* EXIT-ERROR:  n/a                                                   */
/*                                                                    */
/* EFFECTS:  none                                                     */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
USHORT AddToMsgArray( USHORT msgIds[], USHORT msgIndex, USHORT currCount, USHORT maxCount )
{
   if (currCount<maxCount)
   {
      msgIds[currCount]=msgIndex;
      currCount++;
   }
   return (currCount);
}


static UCHAR SetHex( ULONG value, PSZ converted);
UCHAR SetDec( ULONG value, PSZ converted);

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  SetLongValue                                     */
/*                                                                    */
/* DESCRIPTIVE NAME:  Set long value in string template.              */
/*                                                                    */
/* FUNCTION:  The function of this routine is to convert long binary  */
/*            to ASCII (decimal or hexadecimal string) and store      */
/*            resulting string in target string.                      */
/*                                                                    */
/* NOTES: This routine scans target string for %dd...dd or %xx...xx   */
/*        substring to detect conversion type (d - convert to decimal,*/
/*        x - convert to hexadecimal). dd...dd denotes variable length*/
/*        substring consisting solely symbol d. All this substring    */
/*        including leading % is replaced by converted value, tailing */
/*        are cutted of when necessarry.                              */
/*                                                                    */
/* CONTEXT: Initialization time                                       */
/*                                                                    */
/* ENTRY POINT:  SetLongValue                                         */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  PSZ Buf - pointer to target string                         */
/*         ULONG value - value to be converted and inserted           */
/*                       into target                                  */
/*                                                                    */
/* EXIT-NORMAL: n/a                                                   */
/*                                                                    */
/* EXIT-ERROR:  n/a                                                   */
/*                                                                    */
/* EFFECTS:  none                                                     */
/*                                                                    */
/* INTERNAL REFERENCES: movmem                                        */
/*                      SetHex                                        */
/*                      SetDec                                        */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
VOID SetLongValue( PSZ Buf, ULONG value )
{
   PSZ      fieldStart, cPos;
   UCHAR    fieldType;
   UCHAR    converted[16];
   UCHAR    valLength=0;
   USHORT   fieldLength, excessBytes;

   // find 1st
   for (fieldStart=Buf; *fieldStart!='%' && *fieldStart; fieldStart++);
   if (!*fieldStart)
      return;

   fieldType=*(fieldStart+1);
   *fieldStart=fieldType;

   for (cPos=fieldStart, fieldLength=0; *cPos==fieldType && *cPos; cPos++, fieldLength++);

   switch (fieldType)
   {
   case 'x':
      valLength=SetHex( value, converted );
      break;
   case 'd':
      valLength=SetDec( value, converted);
      break;
   default:
      valLength=0;
      break;
   }

   if (valLength)
   {
      if ((USHORT)valLength<fieldLength)
      {
         movmem(fieldStart, converted, valLength);
         fieldStart+=valLength;
         excessBytes=fieldLength-valLength;
         for (;*fieldStart; fieldStart++)
         {
            *fieldStart=*(fieldStart+excessBytes);
            if (!*fieldStart)
               break;
         }
      }
      else
         movmem(fieldStart, converted, fieldLength);
   }
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  SetHex                                           */
/*                                                                    */
/* DESCRIPTIVE NAME:  Convert binary to hexadecimal.                  */
/*                                                                    */
/* FUNCTION:  The function of this routine is to convert long binary  */
/*            to hexadecimal string) and store in target buffer.      */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Initialization time                                       */
/*                                                                    */
/* ENTRY POINT:  SetHex                                               */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  ULONG value - binary value to be converted to hexadecimal  */
/*         PSZ converted - target buffer                              */
/*                                                                    */
/* EXIT-NORMAL: n/a                                                   */
/*                                                                    */
/* EXIT-ERROR:  n/a                                                   */
/*                                                                    */
/* EFFECTS:  none                                                     */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
static UCHAR SetHex( ULONG value, PSZ converted)
{
   UCHAR       hexDigit, hexChar, targetLength=0;
   UCHAR FAR   *xValue=(UCHAR FAR *)&value;
   SHORT       halfByteIndex, byteIndex;
   BOOL        nonZeroDigitFound=FALSE;

   for (halfByteIndex=7, byteIndex=3; halfByteIndex>=0; halfByteIndex--)
   {
      if (halfByteIndex&1)  // ms digit
      {
         hexDigit=(UCHAR)(xValue[byteIndex]>>4);
      }
      else  // ls digit
      {
         hexDigit=(UCHAR)(xValue[byteIndex]&0x0f);
         byteIndex--;
      }
      if (hexDigit || !halfByteIndex)
         nonZeroDigitFound=TRUE;
      if (!nonZeroDigitFound)
         continue;

      if (hexDigit<=9)     // 22/04/1999 MB
         hexChar=(UCHAR)('0'+hexDigit);
      else
         hexChar=(UCHAR)('a'+hexDigit-10);   // 22/04/1999 MB
      *converted=hexChar; converted++;
      targetLength++;
   }

   return (targetLength);
}

static PSZ ustrcmp(PSZ cLine, CHAR *key);
CHAR ConvertDec(PSZ valStart, LONG FAR *value);
static CHAR ConvertStr(PSZ cfStart, PSZ valStart, LONG FAR *value);

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  ProcessConfigString                              */
/*                                                                    */
/* DESCRIPTIVE NAME:  Process Configuration String.                   */
/*                                                                    */
/* FUNCTION:  The function of this routine is to process device       */
/*            driver configuration string.                            */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Initialization time                                       */
/*                                                                    */
/* ENTRY POINT:  ProcessConfigString                                  */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  PSZ confLine - driver's configuration string               */
/*         USHORT keyCount - configuration key count                  */
/*         KeyData FAR *keyData - key description array               */
/*          CHAR     *key; - points to parameter key string           */
/*          CHAR     type; - specifies key type - decimal or string   */
/*          CHAR     keyStatus; - assigned during processing:         */
/*                    CFSTR_STATUS_OK - key found and value assigned  */
/*                    CFSTR_STATUS_NOTFOUND - key not found           */
/*                    CFSTR_STATUS_CONVERR - failed to convert        */
/*                    CFSTR_STATUS_NOVALUE- key found, but no value   */
/*                                          given.                    */
/*          LONG     value; - for decimal keys contains converted     */
/*                            value or 0; for strings - indexes for   */
/*                            the 1st and last characters in low and  */
/*                            high order words respectively.          */
/*                                                                    */
/* EXIT-NORMAL: CFSTR_RC_OK                                           */
/*                                                                    */
/* EXIT-ERROR:  low word contains error code:                         */
/*              CFSTR_CONVERR - conversion error detected             */
/*              CFSTR_UNKWN_KEYS - unknown keys detected              */
/*              high word contains position in configuration string   */
/*              where error has been detected (relatively to the 1st  */
/*              parameter field).                                     */
/*                                                                    */
/* EFFECTS:  none                                                     */
/*                                                                    */
/* INTERNAL REFERENCES: ustrcmp                                       */
/*                      ConvertDec                                    */
/*                      ConvertStr                                    */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
ULONG ProcessConfigString(PSZ confLine, USHORT keyCount, KeyData FAR *keyData)
{
   PSZ      valStart, lineStart=confLine;
   USHORT   rc=CFSTR_RC_OK;
   BOOL     keyValid;
   USHORT   keyIndex;
   USHORT   errorAtCol;

   // set value status to 'not found'
   for (keyIndex=0; keyIndex<keyCount; keyIndex++)
   {
      keyData[keyIndex].keyStatus=CFSTR_STATUS_NOTFOUND;
      keyData[keyIndex].value=0;
   }

   while (confLine && *confLine)
   {
      for (;*confLine && *confLine!='/'; confLine++);
      if (!*confLine)
         break;
      if (*confLine=='/')
         confLine++;

      keyValid=FALSE;
      for (keyIndex=0; keyIndex<keyCount; keyIndex++)
      {
         valStart=ustrcmp(confLine, keyData[keyIndex].key);
         if (!valStart)
            continue;
         keyValid=TRUE;
         if (keyData[keyIndex].keyStatus!=CFSTR_STATUS_NOTFOUND)
            continue;
         switch (keyData[keyIndex].type)
         {
         case CFSTR_TYPE_DEC:
            keyData[keyIndex].keyStatus=ConvertDec(valStart, &keyData[keyIndex].value);
            break;
         case CFSTR_TYPE_STRING:
            keyData[keyIndex].keyStatus=ConvertStr(lineStart, valStart, &keyData[keyIndex].value);
            break;
         }
         if (keyData[keyIndex].keyStatus==CFSTR_STATUS_CONVERR)
         {
            errorAtCol=(USHORT)(confLine-lineStart);
            rc=CFSTR_CONVERR;
         }
      }

      if (!keyValid && rc==CFSTR_RC_OK)
      {
         errorAtCol=(USHORT)(confLine-lineStart);
         rc=CFSTR_UNKWN_KEYS;
      }
   }

   return (MAKEULONG(rc,errorAtCol));
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  ustrcmp                                          */
/*                                                                    */
/* DESCRIPTIVE NAME:  string compare and search.                      */
/*                                                                    */
/* FUNCTION:  The function of this routine is to search for specified */
/*            substring in configuration line.                        */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Initialization time                                       */
/*                                                                    */
/* ENTRY POINT:  ustrcmp                                              */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  PSZ cLine - pointer to configuration line                  */
/*         CHAR *key - substring (key) to search for                  */
/*                                                                    */
/* EXIT-NORMAL: pointer to next to substring character                */
/*                                                                    */
/* EXIT-ERROR:  NULL - substring not found in configuration line      */
/*                                                                    */
/* EFFECTS:  none                                                     */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
static PSZ ustrcmp(PSZ cLine, CHAR *key)
{
   for (;*key && *cLine; key++, cLine++)
   {
      if (((UCHAR)*key)!=*cLine)
         return (NULL);
   }

   if (*key!=0)
      return (NULL);
   else
      return (cLine);
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  ConvertStr                                       */
/*                                                                    */
/* DESCRIPTIVE NAME:  Convert to string .                             */
/*                                                                    */
/* FUNCTION:  The function of this routine is to extract (convert)    */
/*            ASCII string value                                      */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Initialization time                                       */
/*                                                                    */
/* ENTRY POINT:  ConvertStr                                           */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  PSZ cfStart - pointer to 1st character in line             */
/*         PSZ valStart - pointer to 1st value character              */
/*                           converted value                          */
/*         LONG FAR *value - pointer to double word to receive indexes*/
/*                           to 1st and last value characters in low  */
/*                           and high words respectively              */
/*                                                                    */
/* EXIT-NORMAL: CFSTR_STATUS_OK                                       */
/*              CFSTR_STATUS_NOVALUE - no string found                */
/*                                                                    */
/* EXIT-ERROR:  none                                                  */
/*                                                                    */
/* EFFECTS:  none                                                     */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
static CHAR ConvertStr(PSZ cfStart, PSZ valStart, LONG FAR *value)
{
   PSZ   valEnd;

   for (valEnd=valStart; *valEnd && *valEnd!=' '; valEnd++);

   if (valEnd==valStart)
      return (CFSTR_STATUS_NOVALUE);
   *value=MAKELONG(valStart-cfStart, valEnd-cfStart);

   return (CFSTR_STATUS_OK);
}

