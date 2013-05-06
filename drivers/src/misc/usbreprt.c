/* SCCSID = "src/dev/usb/MISC/USBREPRT.C, usb, c.basedd 98/07/10" */
/*
*   Licensed Material -- Property of IBM
*
*   (c) Copyright IBM Corp. 1997, 1998, 2000  All Rights Reserved
*/
/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  USBREPRT.C                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB report process                                    */                                                       
/*                                                                            */
/*   FUNCTION: Contains functions to process report from USB device           */
/*                                                                            */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS: GetUsageOffset                                             */
/*                 GetUsageSize                                               */
/*                 GetReportLength                                            */
/*                                                                            */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer           Comment                            */
/*  ----    --------  ----------          -------                             */
/*          98/04/17  Vjacheslav Chibis                                       */
/*          00/03/10  MB                  Fixed offset calculation in         */
/*                                        GetUsageOffset routine for non-zero */
/*                                        report IDs                          */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/


#include "usbmisc.h" /* master header */
#include "usbchid.h" /* header for HID DD */


#ifdef   LIBDEBUG
extern UCHAR gUSBMISCMsgLevel;
   #define         ldsPrint(l,s)              dsPrint5x(gUSBMISCMsgLevel,(l),(s),0,0,0,0,0)
   #define         ldsPrint1(l,s,a)           dsPrint5x(gUSBMISCMsgLevel,(l),(s),(a),0,0,0,0)
   #define         ldsPrint2(l,s,a,b)         dsPrint5x(gUSBMISCMsgLevel,(l),(s),(a),(b),0,0,0)
   #define         ldsPrint3(l,s,a,b,c)       dsPrint5x(gUSBMISCMsgLevel,(l),(s),(a),(b),(c),0,0)
   #define         ldsPrint4(l,s,a,b,c,d)     dsPrint5x(gUSBMISCMsgLevel,(l),(s),(a),(b),(c),(d),0)
#endif



#pragma  alloc_text(RMCode, GetReportLength)
#pragma  alloc_text(RMCode, GetUsageOffset)
#pragma  alloc_text(RMCode, GetUsageSize)
#pragma  alloc_text(RMCode, CheckSUsage)
#pragma  alloc_text(RMCode, CheckGUsage)





/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME:  GetReportlength                                  */
/*                                                                    */
/* DESCRIPTIVE NAME:  get report length                               */
/*                                                                    */
/* FUNCTION:   Calculates interrupt buffer length value               */
/*                                                                    */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT:                                                           */
/*                                                                    */
/* ENTRY POINT :  GetReportlength                                     */
/*    LINKAGE  :  CALL NEAR                                           */
/*                                                                    */
/* INPUT:  PRP_GENIOCTL pRP_GENIOCTL                                  */
/*         UCHAR ReportType                                           */
/*         UCHAR ReportID                                             */
/*         UCHAR Interface                                            */
/*                                                                    */
/* EXIT-NORMAL:  ALWAYS                                               */
/*                                                                    */
/* EXIT-ERROR: None                                                   */
/*                                                                    */
/* EFFECTS:  None                                                     */
/*                                                                    */
/* INTERNAL REFERENCES:  none                                         */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/


USHORT GetReportLength(PRP_GENIOCTL pRP_GENIOCTL, UCHAR ReportType, UCHAR ReportID, UCHAR Interface )
{  
   UCHAR IntReportLen;
   USBHIDServe FAR *ServDevData;
   USHORT reportIndex;



   ServDevData=(USBHIDServe FAR *)pRP_GENIOCTL->ParmPacket; /* get pointer to the parameters packet */
   reportIndex=ServDevData->reportItemIndex;

   /*******************/
   /* set head values */
   /*******************/
   IntReportLen=0;

#ifdef LIBDEBUG
   ldsPrint3(DBG_SPECIFIC, "GetReportLength : ReportType=%d ReportID=%d, Interface=%d\r\n",
             ReportType,
             ReportID,
             Interface
            );
#endif

   while (reportIndex!=LAST_INDEX)
   {
      /**********************************************************************************/
      /* we need proper report type, interface No & report identificator for our buffer */
      /**********************************************************************************/
      if ((ServDevData->itemData[reportIndex].mainType==ReportType)&&
          (ServDevData->itemData[reportIndex].interface==Interface)&&
          (ServDevData->itemData[reportIndex].itemFeatures.reportID==ReportID))
      {
#ifdef LIBDEBUG
         ldsPrint3(DBG_SPECIFIC, "GetReportLength : report type=%d interface=%d reportid=%d\r\n",
                   ServDevData->itemData[reportIndex].mainType,
                   ServDevData->itemData[reportIndex].interface,
                   ServDevData->itemData[reportIndex].itemFeatures.reportID
                  );
         ldsPrint2(DBG_SPECIFIC, "GetReportLength : reportSize=%d, reportCount=%d\r\n",
                   ServDevData->itemData[reportIndex].itemFeatures.reportSize,
                   ServDevData->itemData[reportIndex].itemFeatures.reportCount
                  );
#endif
         /*****************************************/
         /* increments interrupt length each time */ 
         /*****************************************/
         IntReportLen+=ServDevData->itemData[reportIndex].itemFeatures.reportSize*
                       ServDevData->itemData[reportIndex].itemFeatures.reportCount;     
      }

      reportIndex=ServDevData->itemData[reportIndex].indexToNextItem; /* get next item data */
   }


#ifdef LIBDEBUG
   ldsPrint1(DBG_DETAILED,"GetReportLength : IntReportLen (in bits) =%d\r\n", IntReportLen);
   ldsPrint(DBG_SPECIFIC,"GetReportLength : ReportType =");

   switch (ReportType)
   {
   case HID_REPORT_TAGS_MAIN_INPUT: ldsPrint(DBG_SPECIFIC," HID_REPORT_TAGS_MAIN_INPUT ");break;
   case HID_REPORT_TAGS_MAIN_OUTPUT: ldsPrint(DBG_SPECIFIC," HID_REPORT_TAGS_MAIN_OUTPUT ");break;
   case HID_REPORT_TAGS_MAIN_FEATURE:ldsPrint(DBG_SPECIFIC," HID_REPORT_TAGS_MAIN_FEATURE ");break;
   default: ldsPrint(DBG_SPECIFIC,"INVALID"); break;
   }

   ldsPrint(DBG_SPECIFIC,"\r\n");
#endif



   /* round value */
   if (IntReportLen%BITS_IN_BYTE)
      IntReportLen+=BITS_IN_BYTE-(IntReportLen%BITS_IN_BYTE);

   IntReportLen>>=BITS_DIVIDER; /* converts value in bit 2 value in bytes */


   /******************************************************************/
   /* if report identificator <> 0,                                  */
   /* interrupt buffer`s first element will contain report Number ID */
   /* so, we have to increment interrupt buffer length               */
   /******************************************************************/
   if (ReportID) IntReportLen++;

   return IntReportLen; // in bytes
}


/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME:  GetUsageOffset                                   */
/*                                                                    */
/* DESCRIPTIVE NAME: Get Usage Offset                                 */
/*                                                                    */
/* FUNCTION:  Calculates offset in the interrupt data buffer          */
/*               for specified usage                                  */
/*                                                                    */
/*                                                                    */
/*                                                                    */
/* NOTES : Calculates ABSOLUTE offset in buffer                       */
/*           (Report ID case included)                                */                             
/*          In array case return starting offset of array             */
/*                                                                    */
/* CONTEXT:                                                           */
/*                                                                    */
/* ENTRY POINT :  GetUsageOffset                                      */
/*    LINKAGE  :  CALL NEAR                                           */
/*                                                                    */
/* INPUT:                                                             */
/*          PRP_GENIOCTL pRP_GENIOCTL - near pointer to request packet*/
/*          UCHAR ReportType - main report type                       */
/*          UCHAR ReportID  - report identificator ( 0 as usual )     */
/*          UCHAR interface  - interface identificator                */ 
/*          USHORT UsagePage - usage page for current usage           */                           
/*          USHORT UsageID   - usage identificator                    */
/*                                                                    */
/*                                                                    */
/* EXIT-NORMAL: always                                                */
/*                                                                    */
/* EXIT-ERROR: None  (-1 if usage is not found )                      */
/*                                                                    */
/* EFFECTS:  None                                                     */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:     CheckGUsage                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:       None                                            */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/



USHORT GetUsageOffset(PRP_GENIOCTL pRP_GENIOCTL, UCHAR ReportType, UCHAR ReportID, UCHAR interface, USHORT UsagePage, USHORT UsageID)
{  
   USHORT offset;
   USHORT res;
   USHORT UsageMin;
   USHORT UsageMax;
   BOOL found;
   BOOL Array;


   USBHIDServe FAR *ServDevData;
   USHORT reportIndex;

   ServDevData=(USBHIDServe FAR *)pRP_GENIOCTL->ParmPacket; /* get pointer to the parampacket */

   reportIndex=ServDevData->reportItemIndex; /* get Item data address */

   /*******************/
   /* set head values */
   /*******************/
   offset=0; 
   found=FALSE;
   Array=FALSE;

   while (reportIndex!=LAST_INDEX)
   {

      /*********************/
      /* to shorter typing */
      /*********************/
      UsageMax=ServDevData->itemData[reportIndex].localFeatures.usageMax;
      UsageMin=ServDevData->itemData[reportIndex].localFeatures.usageMin;

#ifdef LIBDEBUG
      ldsPrint3(DBG_SPECIFIC,"GetUsageOffset : Usagemin=%d, usagemax=%d, usageid=%d\r\n",
                UsageMin,
                UsageMax,
                UsageID);
      ldsPrint2(DBG_SPECIFIC,"GetUsageOffset : UsagePage=%d usagePage=%d\r\n",
                UsagePage,
                ServDevData->itemData[reportIndex].itemFeatures.usagePage);
#endif
      if ((UsagePage==ServDevData->itemData[reportIndex].itemFeatures.usagePage)&&
          (ReportID==ServDevData->itemData[reportIndex].itemFeatures.reportID)&&
          (interface==ServDevData->itemData[reportIndex].interface)&&
          (ReportType==ServDevData->itemData[reportIndex].mainType)
         )
      {
         Array=!(ServDevData->itemData[reportIndex].itemFlags&2); /* check, if usage is array element */
         if ( (UsageMin<=UsageID)&&
              (UsageMax>=UsageID))
         {
            if ((UsageMin==UsageID)&&(UsageMax==UsageID))
            {
               /* in this case usage is in the generic data */
               offset+=(ServDevData->itemData[reportIndex].itemFeatures.reportSize)*(ServDevData->itemData[reportIndex].itemFeatures.reportCount-1);
#ifdef LIBDEBUG
               ldsPrint(DBG_SPECIFIC, "GetUsageOffset : usagemin==usageid==usagemax\r\n");
#endif
            }
            else

            {
               /************************************************************************/
               /* if the usage is not an element of an array                           */
               /* else offset will point to the start offset of array , where usage is */
               /************************************************************************/

               if (!Array)
                  offset+=(UsageID-UsageMin)*(ServDevData->itemData[reportIndex].itemFeatures.reportSize);
               else
               {
#ifdef LIBDEBUG
                  ldsPrint(DBG_SPECIFIC, "GetUsageOffset : Array found\r\n"); 
#endif
               }

            }
            found=TRUE;
#ifdef LIBDEBUG
            ldsPrint1(DBG_SPECIFIC,"GetUsageOffset : offset=%d\r\n", offset);
#endif

            break;  /* all done */

         }
         else
         {

            /***********************************************/
            /* if usage is not found in generic usage data */
            /***********************************************/
            res=CheckGUsage( ServDevData->itemUsage, ServDevData->itemData[reportIndex].localFeatures.indexToUsageList, UsageID);
            if (res!=LAST_INDEX) /* if usage is found */
            {
#ifdef LIBDEBUG
               ldsPrint1(DBG_SPECIFIC,"GetUsageOffset : CheckGUsage OK! res=%d\r\n", res);
#endif
               offset+=ServDevData->itemData[reportIndex].itemFeatures.reportSize*(res-1);
               found=TRUE;
               break; /* all done */
            }
         }

      }

      /************************************************/
      /* check for proper values and increment offset */
      /************************************************/
      if ((ServDevData->itemData[reportIndex].mainType==ReportType)&&
          (ServDevData->itemData[reportIndex].interface==interface)&&
          (ServDevData->itemData[reportIndex].itemFeatures.reportID==ReportID))
      {
         offset+=ServDevData->itemData[reportIndex].itemFeatures.reportSize*
                 ServDevData->itemData[reportIndex].itemFeatures.reportCount;
      }

      reportIndex=ServDevData->itemData[reportIndex].indexToNextItem;
   }


   /***************************************************************/
   /* if usage can be found in interrupt data with                */
   /* report number <> 0,                                         */ 
   /* interrupt buffer`s first element will contain report Number */                        
   /* so, we have to increment usage offset                       */                        
   /***************************************************************/                        
   if (ReportID) offset+=BITS_IN_BYTE*sizeof(ReportID);   // 02/17/2000 MB

   if (!found) offset=FULL_WORD; /* return -1 if usage is not found */

#ifdef LIBDEBUG
   ldsPrint2(DBG_SPECIFIC,"GetUsageOffset : UsageID =%d  offset (in bits) =%d\r\n", UsageID, offset);
#endif

   return offset;
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME:  CheckGUsage                                      */
/*                                                                    */
/* DESCRIPTIVE NAME: Check generic usage                              */
/*                                                                    */
/* FUNCTION:   GetUsageOffset internal use only                       */
/*                                                                    */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT:                                                           */
/*                                                                    */
/* ENTRY POINT :  CheckGUsage                                         */
/*    LINKAGE  :  CALL NEAR                                           */
/*                                                                    */
/* INPUT: ItemUsage FAR *pUsageData                                   */
/*        USHORT IndexToUsageList                                     */
/*        USHORT UsageID                                              */
/*                                                                    */
/* EXIT-NORMAL:  always                                               */
/*                                                                    */
/* EXIT-ERROR:   None                                                 */
/*                                                                    */
/* EFFECTS:  None                                                     */
/*                                                                    */
/* INTERNAL REFERENCES:  None                                         */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:   None                                        */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

USHORT CheckGUsage( ItemUsage FAR *pUsageData, USHORT IndexToUsageList, USHORT UsageID)
{
   USHORT counter;
   ItemUsage FAR *pUData;

   counter=0; /* default value, 0 means that usage is not found   */



   while (IndexToUsageList!=LAST_INDEX)
   {
      pUData=pUsageData+IndexToUsageList; /* get pointer to the next usage data */
      counter++; /* we need it to calculate usage offset if it is not in generic usage data */

      if ( (pUData->usageMin<=UsageID)&&
           (pUData->usageMax>=UsageID)
         )
      {
#ifdef LIBDEBUG
         ldsPrint1(DBG_SPECIFIC,"CheckGUsage : Usage found! counter=%d\r\n", counter);
#endif
         break; /* we`ve found the usage */
      }

      IndexToUsageList=pUData->indexToNextUsageData; /* get next usage data */
   }//while

   if (IndexToUsageList==LAST_INDEX)
   {
      return IndexToUsageList;
   }

   return counter;
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME:  GetUsageSize                                     */
/*                                                                    */
/* DESCRIPTIVE NAME:  Get usage size                                  */
/*                                                                    */
/* FUNCTION:  Calculates size in interrupt buffer                     */
/*                   for the specified usage                          */
/*                                                                    */
/*                                                                    */
/* NOTES:  return 0 if usage not found                                */
/*                                                                    */
/* CONTEXT:                                                           */
/*                                                                    */
/* ENTRY POINT :  GetUsageSize                                        */
/*    LINKAGE  :  CALL NEAR                                           */
/*                                                                    */
/* INPUT:     PRP_GENIOCTL pRP_GENIOCTL                               */
/*            UCHAR ReportType                                        */
/*            UCHAR ReportID                                          */
/*            UCHAR interface                                         */
/*            USHORT UsagePage                                        */
/*            USHORT UsageID                                          */
/*                                                                    */
/* EXIT-NORMAL: always                                                */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS:  None                                                     */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:     CheckSUsage                                       */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:     None                                              */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/


ULONG GetUsageSize(PRP_GENIOCTL pRP_GENIOCTL, UCHAR ReportType, UCHAR ReportID, UCHAR interface, USHORT UsagePage, USHORT UsageID)
{  
   ULONG  UsageSize;
   USHORT UsageMin;
   USHORT UsageMax;


   USBHIDServe FAR *ServDevData;
   USHORT reportIndex;


   ServDevData=(USBHIDServe FAR *)pRP_GENIOCTL->ParmPacket;/* get pointer to parameters packet */

   reportIndex=ServDevData->reportItemIndex;

   UsageSize=0;          /* default value, 0 means that usage is not found   */

   while (reportIndex!=LAST_INDEX)
   {

      /* to simplify typing */
      UsageMax=ServDevData->itemData[reportIndex].localFeatures.usageMax;
      UsageMin=ServDevData->itemData[reportIndex].localFeatures.usageMin;

      /* compare interface #, reportiD & usage page first */
      if ((UsagePage==ServDevData->itemData[reportIndex].itemFeatures.usagePage)&&
          (ReportID==ServDevData->itemData[reportIndex].itemFeatures.reportID)&&
          (interface==ServDevData->itemData[reportIndex].interface))
      {  /* if we`ve found appropriate reportId interface & usage Page, then try to find our usage */
         if ((UsageID<=UsageMax)&&(UsageID>=UsageMin))
         {
            UsageSize=ServDevData->itemData[reportIndex].itemFeatures.reportSize;
#ifdef LIBDEBUG
            ldsPrint3(5, "GetUsageSize : usageid=%d usagemin=%d usagemax=%d\r\n", UsageID, UsageMin, UsageMax);
#endif
            break; /* usage is found */
         }
         else
         {  /* we are here if usage is not found in generic item data */
            /* let`s try to find it in secondary usage list */
            UsageSize=CheckSUsage((ItemFeatures FAR *)&ServDevData->itemData[reportIndex].itemFeatures, ServDevData->itemUsage, ServDevData->itemData[reportIndex].localFeatures.indexToUsageList, UsageID);
#ifdef LIBDEBUG
            ldsPrint1(DBG_SPECIFIC,"CheckSUsage result=%d\r\n", UsageSize);
#endif

            if (UsageSize) break; /* if usage is found */

         }

      }
      reportIndex=ServDevData->itemData[reportIndex].indexToNextItem; /* get the next item */
   }

#ifdef LIBDEBUG
   ldsPrint1(DBG_SPECIFIC,"GetUsageSize : UsageSize=%d\r\n", UsageSize);
#endif


   return UsageSize;
}
/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME:  CheckSUsage                                      */
/*                                                                    */
/* DESCRIPTIVE NAME: check usage                                      */
/*                                                                    */
/* FUNCTION:   GetUsageSize internal use                              */
/*       searches for the specified usage and return report size      */
/*                                                                    */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT:                                                           */
/*                                                                    */
/* ENTRY POINT :  CheckSUsage                                         */
/*    LINKAGE  :  CALL NEAR                                           */
/*                                                                    */
/* INPUT:                                                             */
/*                                                                    */
/* EXIT-NORMAL:  Always                                               */
/*                                                                    */
/* EXIT-ERROR:   None                                                 */
/*                                                                    */
/* EFFECTS:  None                                                     */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:    None                                               */
/*                                                                    */
/* EXTERNAL REFERENCES:                                               */
/*    ROUTINES:     None                                              */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

ULONG CheckSUsage(ItemFeatures FAR *pitemFeatures, ItemUsage FAR *pUsageData, USHORT IndexToUsageList, USHORT UsageID)
{
   ItemUsage FAR *pUData;

   while (IndexToUsageList!=LAST_INDEX)
   {
      pUData=pUsageData+IndexToUsageList;

      if ((UsageID>=pUData->usageMin)&&(UsageID<=pUData->usageMax))
         return pitemFeatures->reportSize; /* we`ve find the usage */

      IndexToUsageList=pUData->indexToNextUsageData;
   }//while

   return 0; /*  0 means that usage is not found */
}


