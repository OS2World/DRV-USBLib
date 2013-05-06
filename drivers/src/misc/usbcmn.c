/* SCCSID = "src/dev/usb/MISC/USBCMN.C, usb, c.basedd 98/07/10" */
/*
*   Licensed Material -- Property of IBM
*
*   (c) Copyright IBM Corp. 1997, 1998  All Rights Reserved
*/
/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  USBCMN.C                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME:  Common USB data service routines.                     */
/*                                                                            */
/*   FUNCTION: These routines handle the presence check for the USB           */
/*             ports.                                                         */
/*                                                                            */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:                                                            */
/*             GetInterruptPipeAddr                                           */
/*             GetHIDReportLength                                             */
/*             GetNextDescriptor                                              */
/*             GetInterfaceDPtr                                               */
/*             GetEndpointDPtr                                                */
/*             IsDraft3Compl                                                  */
/*             SearchConfiguration                                            */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          98/01/31  MB                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/
#include "usbmisc.h"
#include "usbchid.h"

#pragma alloc_text( RMCode, GetInterruptPipeAddr )
#pragma alloc_text( RMCode, GetHIDReportLength )
#pragma alloc_text( RMCode, GetNextDescriptor )     
#pragma alloc_text( RMCode, GetInterfaceDPtr )     
#pragma alloc_text( RMCode, GetEndpointDPtr )     
#pragma alloc_text( RMCode, IsDraft3Compl )     
#pragma alloc_text( RMCode, SearchConfiguration )     
#pragma alloc_text( RMCode, GetPipeAddr )     
#pragma alloc_text( RMCode, GetMaxAltIntf )     

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  GetInterruptPipeAddr                             */
/*                                                                    */
/* DESCRIPTIVE NAME:  Get device interrupt pipe address               */
/*                                                                    */
/* FUNCTION:  The function of this routine is extract 1st interrupt   */
/*            pipe address from device configuration data.            */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  GetInterruptPipeAddr                                 */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  UCHAR FAR *configurationData - far pointer to device       */
/*                      configuration data buffer                     */
/*         UCHAR bNumConfigurations - number of configurations for    */
/*                      device                                        */
/*         UCHAR configurationValue - current configuration           */
/*         UCHAR interface - interface                                */
/*                                                                    */
/* EXIT-NORMAL: non-zero interrrupt pipe address                      */
/*                                                                    */
/* EXIT-ERROR:  0, no interrupt pipe found for current configuration  */
/*              and interface                                         */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:  GetNextDescriptor                            */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
UCHAR GetInterruptPipeAddr(UCHAR FAR *configurationData, UCHAR bNumConfigurations, UCHAR configurationValue, UCHAR interface)
{
   DeviceEndpoint FAR      *endPointDesc;
   DeviceConfiguration FAR *devConf;
   UCHAR                   configIndex;
   UCHAR FAR               *currBytePtr, FAR *lastBytePtr;
   DeviceDescHead FAR      *descHead;
   DeviceInterface FAR     *interfaceDesc;
   UCHAR                   currInterface=0;
   UCHAR                   intPipe=0;

   devConf=(DeviceConfiguration FAR *)configurationData;
   for (configIndex=0; configIndex<bNumConfigurations;
       configIndex++)
   {
      currBytePtr=(UCHAR FAR *)devConf;
      lastBytePtr=currBytePtr+devConf->wTotalLength;
      descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength);
      if (devConf->bConfigurationValue==configurationValue)
      {
         for (descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength); descHead && !intPipe;
             descHead=GetNextDescriptor( descHead, lastBytePtr))
         {
            switch (descHead->bDescriptorType)
            {
            case DESC_INTERFACE:
               interfaceDesc=(DeviceInterface FAR *)descHead;
               currInterface=interfaceDesc->bInterfaceNumber;
               break;
            case DESC_ENDPOINT:
               if (currInterface!=interface)
                  break;
               endPointDesc=(DeviceEndpoint FAR *)descHead;
               if ((endPointDesc->bmAttributes&DEV_ENDPT_ATTRMASK)==DEV_ENDPT_INTRPT)
                  intPipe=(UCHAR)(endPointDesc->bEndpointAddress&DEV_ENDPT_ADDRMASK);
               break;
            default:
               break;
            }
         }
         break;
      }
      devConf=(DeviceConfiguration FAR *)lastBytePtr; // point to next configuration block
   }

   return (intPipe);
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  GetPipeAddr                                      */
/*                                                                    */
/* DESCRIPTIVE NAME:  Retrieves device pipe address                   */
/*                                                                    */
/* FUNCTION:  The function of this routine is extract 1st             */
/*            pipe address from device configuration data with        */
/*            required capabilities.                                  */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  GetPipeAddr                                          */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  UCHAR FAR *configurationData - far pointer to device       */
/*                      configuration data buffer                     */
/*         UCHAR bNumConfigurations - number of configurations for    */
/*                      device                                        */
/*         UCHAR configurationValue - current configuration           */
/*         UCHAR interface - interface                                */
/*         UCHAR altInterface - alternate interface value             */
/*         UCHAR type - pipe data transfer direction (IN, OUT)        */
/*         UCHAR typeMask - pipe type mask (direction mask)           */
/*         UCHAR attributes - required attributes (ISO, BULK, INTRPT) */
/*                                                                    */
/* EXIT-NORMAL: non-zero pipe address                                 */
/*                                                                    */
/* EXIT-ERROR:  0, no pipe found for current configuration            */
/*              and interface&altinterface                            */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:  GetNextDescriptor                            */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
UCHAR GetPipeAddr(UCHAR FAR *configurationData, UCHAR bNumConfigurations, UCHAR configurationValue,
                     UCHAR interface, UCHAR altInterface, UCHAR type, UCHAR typeMask, UCHAR attributes)
{
   DeviceEndpoint FAR      *endPointDesc;
   DeviceConfiguration FAR *devConf;
   UCHAR                   configIndex;
   UCHAR FAR               *currBytePtr, FAR *lastBytePtr;
   DeviceDescHead FAR      *descHead;
   DeviceInterface FAR     *interfaceDesc;
   UCHAR                   currInterface=0, currAlternate=0;
   UCHAR                   pipe=0;

   devConf=(DeviceConfiguration FAR *)configurationData;
   for (configIndex=0; configIndex<bNumConfigurations;
       configIndex++)
   {
      currBytePtr=(UCHAR FAR *)devConf;
      lastBytePtr=currBytePtr+devConf->wTotalLength;
      descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength);
      if (devConf->bConfigurationValue==configurationValue)
      {
         for (descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength); descHead && !pipe;
             descHead=GetNextDescriptor( descHead, lastBytePtr))
         {
            switch (descHead->bDescriptorType)
            {
            case DESC_INTERFACE:
               interfaceDesc=(DeviceInterface FAR *)descHead;
               currInterface=interfaceDesc->bInterfaceNumber;
               currAlternate=interfaceDesc->bAlternateSetting;
               break;
            case DESC_ENDPOINT:
               if (currInterface!=interface || altInterface!=currAlternate)
                  break;
               endPointDesc=(DeviceEndpoint FAR *)descHead;
               if (((UCHAR)(endPointDesc->bmAttributes&DEV_ENDPT_ATTRMASK)==attributes) &&
                     ((UCHAR)(endPointDesc->bEndpointAddress & typeMask)==type) )
                  pipe=(UCHAR)(endPointDesc->bEndpointAddress&DEV_ENDPT_ADDRMASK);
               break;
            default:
               break;
            }
         }
         break;
      }
      devConf=(DeviceConfiguration FAR *)lastBytePtr; // point to next configuration block
   }

   return (pipe);
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  GetMaxAltIntf                                    */
/*                                                                    */
/* DESCRIPTIVE NAME:  Get alternate interface count                   */
/*                                                                    */
/* FUNCTION:  The function of this routine is determine no of         */
/*            alternatives for given interface.                       */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  GetMaxAltIntf                                        */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  UCHAR FAR *configurationData - far pointer to device       */
/*                      configuration data buffer                     */
/*         UCHAR bNumConfigurations - number of configurations for    */
/*                      device                                        */
/*         UCHAR configurationValue - current configuration           */
/*         UCHAR interface - interface                                */
/*                                                                    */
/* EXIT-NORMAL: maximal alternate interface index                     */
/*                                                                    */
/* EXIT-ERROR:  0, no alternatives found                              */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:  GetNextDescriptor                            */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
UCHAR GetMaxAltIntf(UCHAR FAR *configurationData, UCHAR bNumConfigurations,
                    UCHAR configurationValue, UCHAR interface)
{
   DeviceConfiguration FAR *devConf;
   UCHAR                   configIndex;
   UCHAR FAR               *currBytePtr, FAR *lastBytePtr;
   DeviceDescHead FAR      *descHead;
   DeviceInterface FAR     *interfaceDesc;
   UCHAR                   currInterface=0, maxAlt=0;

   devConf=(DeviceConfiguration FAR *)configurationData;
   for (configIndex=0; configIndex<bNumConfigurations;
       configIndex++)
   {
      currBytePtr=(UCHAR FAR *)devConf;
      lastBytePtr=currBytePtr+devConf->wTotalLength;
      descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength);
      if (devConf->bConfigurationValue==configurationValue)
      {
         for (descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength); descHead;
             descHead=GetNextDescriptor( descHead, lastBytePtr))
         {
            switch (descHead->bDescriptorType)
            {
            case DESC_INTERFACE:
               interfaceDesc=(DeviceInterface FAR *)descHead;
               currInterface=interfaceDesc->bInterfaceNumber;
               if(currInterface==interface)
               {
                  if(maxAlt<interfaceDesc->bAlternateSetting)
                     maxAlt=interfaceDesc->bAlternateSetting;
               }
               break;
            default:
               break;
            }
         }
         break;
      }
      devConf=(DeviceConfiguration FAR *)lastBytePtr; // point to next configuration block
   }

   return (maxAlt);
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  GetHIDReportLength                               */
/*                                                                    */
/* DESCRIPTIVE NAME:  Get HID Report length                           */
/*                                                                    */
/* FUNCTION:  The function of this routine is retrieve HID report     */
/*            length from device configuration data.                  */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  GetHIDReportLength                                   */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  UCHAR FAR *configurationData - far pointer to device       */
/*                      configuration data buffer                     */
/*         UCHAR bNumConfigurations - number of configurations for    */
/*                      device                                        */
/*         UCHAR configurationValue - current configuration           */
/*         UCHAR interface - interface                                */
/*                                                                    */
/* EXIT-NORMAL: non-zero HID report length                            */
/*                                                                    */
/* EXIT-ERROR:  0, no HID report found for current configuration      */
/*              and interface                                         */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:  GetNextDescriptor                            */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
USHORT GetHIDReportLength(UCHAR FAR *configurationData, UCHAR bNumConfigurations, UCHAR configurationValue, UCHAR interface)
{
   DeviceConfiguration FAR *devConf;
   UCHAR                   configIndex;
   UCHAR FAR               *currBytePtr, FAR *lastBytePtr;
   DeviceDescHead FAR      *descHead;
   DeviceInterface FAR     *interfaceDesc;
   HIDDeviceDescriptor FAR *hidDesc;
   UCHAR                   currInterface=0;
   USHORT                  hidReportLength=0;

   devConf=(DeviceConfiguration FAR *)configurationData;
   for (configIndex=0; configIndex<bNumConfigurations; configIndex++)
   {
      currBytePtr=(UCHAR FAR *)devConf;
      lastBytePtr=currBytePtr+devConf->wTotalLength;
      descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength);
      if (devConf->bConfigurationValue==configurationValue)
      {
         for (descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength); descHead && !hidReportLength;
             descHead=GetNextDescriptor( descHead, lastBytePtr))
         {
            switch (descHead->bDescriptorType)
            {
            case DESC_INTERFACE:
               interfaceDesc=(DeviceInterface FAR *)descHead;
               currInterface=interfaceDesc->bInterfaceNumber;
               break;
            case HID_CLASS_DESCRIPTORS_HID:
               if (currInterface!=interface)
                  break;
               hidDesc=(HIDDeviceDescriptor FAR *)descHead;
               hidReportLength=hidDesc->descriptorList[0].wDescriptorLength;
               break;
            default:
               break;
            }
         }
         break;
      }
      devConf=(DeviceConfiguration FAR *)lastBytePtr; // point to next configuration block
   }

   return (hidReportLength);
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  GetNextDescriptor                                */
/*                                                                    */
/* DESCRIPTIVE NAME:  Get next USB descriptor                         */
/*                                                                    */
/* FUNCTION:  The function of this routine is retrieve HID report     */
/*            length from device configuration data.                  */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  GetNextDescriptor                                    */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  DeviceDescHead FAR *currHead - pointer to current          */
/*                      decriptor                                     */
/*         UCHAR  FAR *lastBytePtr - pointer to last configuration    */
/*                      data byte                                     */
/*                                                                    */
/* EXIT-NORMAL: pointer to next descriptor                            */
/*                                                                    */
/* EXIT-ERROR:  NULL, no more descriptors                             */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:  none                                         */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
DeviceDescHead FAR *GetNextDescriptor( DeviceDescHead FAR *currHead, UCHAR  FAR *lastBytePtr)
{
   UCHAR FAR   *currBytePtr, FAR *nextBytePtr;

   if (!currHead->bLength)
      return (NULL);
   currBytePtr=(UCHAR FAR *)currHead;
   nextBytePtr=currBytePtr+currHead->bLength;
   if (nextBytePtr>=lastBytePtr)
      return (NULL);
   return ((DeviceDescHead FAR *)nextBytePtr);
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  GetInterfaceDPtr                                 */
/*                                                                    */
/* DESCRIPTIVE NAME:  Get pointer to interface descriptor             */
/*                                                                    */
/* FUNCTION:  The function of this routine is retrieve pointer to     */
/*            required interface descriptor.                          */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  GetInterfaceDPtr                                     */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  UCHAR FAR *configurationData - far pointer to device       */
/*                      configuration data buffer                     */
/*         UCHAR bNumConfigurations - number of configurations for    */
/*                      device                                        */
/*         UCHAR configurationValue - current configuration           */
/*         UCHAR interface - interface                                */
/*                                                                    */
/* EXIT-NORMAL: pointer to interface descriptor                       */
/*                                                                    */
/* EXIT-ERROR:  NULL, no interface descriptor found                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:  GetNextDescriptor                            */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
DeviceInterface FAR * GetInterfaceDPtr(UCHAR FAR *configurationData, UCHAR bNumConfigurations, UCHAR configurationValue, UCHAR interface)
{
   DeviceConfiguration FAR *devConf;
   UCHAR                   configIndex;
   UCHAR FAR               *currBytePtr, FAR *lastBytePtr;
   DeviceDescHead FAR      *descHead;
   DeviceInterface FAR     *interfaceDesc;
   UCHAR                   currInterface=0;

   devConf=(DeviceConfiguration FAR *)configurationData;
   for (configIndex=0; configIndex<bNumConfigurations;
       configIndex++)
   {
      currBytePtr=(UCHAR FAR *)devConf;
      lastBytePtr=currBytePtr+devConf->wTotalLength;
      descHead=(DeviceDescHead FAR *)currBytePtr+devConf->bLength;
      if (devConf->bConfigurationValue==configurationValue)
      {
         for (descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength); descHead;
             descHead=GetNextDescriptor( descHead, lastBytePtr))
         {
            switch (descHead->bDescriptorType)
            {
            case DESC_INTERFACE:
               interfaceDesc=(DeviceInterface FAR *)descHead;
               currInterface=interfaceDesc->bInterfaceNumber;
               if (currInterface==interface)
                  return (interfaceDesc);
               break;
            default:
               break;
            }
         }
         break;
      }
      devConf=(DeviceConfiguration FAR *)lastBytePtr; // point to next configuration block
   }

   return (NULL);
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  GetEndpointDPtr                                  */
/*                                                                    */
/* DESCRIPTIVE NAME:  Get pointer to endpoint descriptor              */
/*                                                                    */
/* FUNCTION:  The function of this routine is retrieve pointer to     */
/*            required endpoint descriptor for given configuration.   */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  GetEndpointDPtr                                      */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  UCHAR FAR *configurationData - far pointer to device       */
/*                      configuration data buffer                     */
/*         UCHAR bNumConfigurations - number of configurations for    */
/*                      device                                        */
/*         UCHAR configurationValue - current configuration           */
/*         UCHAR endpointID - endpoint of interest                    */
/*                                                                    */
/* EXIT-NORMAL: pointer to endpoint descriptor                        */
/*                                                                    */
/* EXIT-ERROR:  NULL, no endpoint descriptor found                    */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:  GetNextDescriptor                            */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
DeviceEndpoint FAR *GetEndpointDPtr(UCHAR FAR *configurationData, UCHAR bNumConfigurations, 
      UCHAR configurationValue, UCHAR altInterface, UCHAR endpointID)
{
   DeviceEndpoint FAR      *endPointDesc;
   DeviceConfiguration FAR *devConf;
   UCHAR                   configIndex;
   UCHAR FAR               *currBytePtr, FAR *lastBytePtr;
   DeviceDescHead FAR      *descHead;
   DeviceInterface FAR     *interfaceDesc;
   UCHAR                   pipeID, currAltInterface=0;

   devConf=(DeviceConfiguration FAR *)configurationData;
   for (configIndex=0; configIndex<bNumConfigurations;
       configIndex++)
   {
      currBytePtr=(UCHAR FAR *)devConf;
      lastBytePtr=currBytePtr+devConf->wTotalLength;
      descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength);
      if (devConf->bConfigurationValue==configurationValue)
      {
         for (descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength); descHead;
             descHead=GetNextDescriptor( descHead, lastBytePtr))
         {
            switch (descHead->bDescriptorType)
            {
            case DESC_INTERFACE:
               interfaceDesc=(DeviceInterface FAR *)descHead;
               currAltInterface=interfaceDesc->bAlternateSetting;
               break;
            case DESC_ENDPOINT:
               endPointDesc=(DeviceEndpoint FAR *)descHead;
               if((endPointDesc->bmAttributes&DEV_ENDPT_ATTRMASK)==DEV_ENDPT_CTRL)  // ignore direction flag
                  pipeID=(UCHAR)(endPointDesc->bEndpointAddress&DEV_ENDPT_ADDRMASK);   // for control pipe
               else
                  pipeID=(UCHAR)(endPointDesc->bEndpointAddress&(DEV_ENDPT_ADDRMASK|DEV_ENDPT_DIRMASK));
               if(altInterface && altInterface!=currAltInterface) // 27/08/98 MB
                  break;
               if (endpointID==pipeID)
                  return (endPointDesc);
               break;
            default:
               break;
            }
         }
         break;
      }
      devConf=(DeviceConfiguration FAR *)lastBytePtr; // point to next configuration block
   }

   return (NULL);
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  IsDraft3Compl                                    */
/*                                                                    */
/* DESCRIPTIVE NAME:  Check HID Draft 3 compliance                    */
/*                                                                    */
/* FUNCTION:  The function of this routine is to check device         */
/*            compliance with HID draft 3.                            */
/*                                                                    */
/* NOTES: compliance test is based on order in which descriptors are  */
/*        reported to host. Interface/Endpoint/HID means that device  */
/*        follows draft 3, but Interface/HID/Endpoint final 1.0.      */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  IsDraft3Compl                                        */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  UCHAR FAR *configurationData - far pointer to device       */
/*                      configuration data buffer                     */
/*         UCHAR bNumConfigurations - number of configurations for    */
/*                      device                                        */
/*                                                                    */
/* EXIT-NORMAL: TRUE, device is draft 3 compliant                     */
/*              FALSE, device is final HID 1.0 compliant              */
/*                                                                    */
/* EXIT-ERROR:  none                                                  */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:  GetNextDescriptor                            */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
BOOL IsDraft3Compl(UCHAR FAR *configurationData, UCHAR bNumConfigurations)
{
   DeviceConfiguration FAR *devConf;
   DeviceDescHead FAR      *descHead;
   UCHAR                   configIndex;
   UCHAR FAR               *currBytePtr, FAR *lastBytePtr;
   BOOL                    hidPassed=FALSE, endpPassed=FALSE, d3Cpml=FALSE;

   devConf=(DeviceConfiguration FAR *)configurationData;
   for (configIndex=0; configIndex<bNumConfigurations; configIndex++)
   {
      currBytePtr=(UCHAR FAR *)devConf;
      lastBytePtr=currBytePtr+devConf->wTotalLength;
      descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength);
      for (descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength); descHead;
          descHead=GetNextDescriptor( descHead, lastBytePtr))
      {
         switch (descHead->bDescriptorType)
         {
         case HID_CLASS_DESCRIPTORS_HID:
            hidPassed=TRUE;
            if (endpPassed) // descriptor order is interface/Endpoint/HID
               d3Cpml=TRUE;
            break;
         case DESC_ENDPOINT:
            endpPassed=TRUE;
            break;
         case DESC_INTERFACE:
            endpPassed=FALSE;
            hidPassed=FALSE;
            break;
         default:
            break;
         }
         if (hidPassed && endpPassed)
            return (d3Cpml);
      }
      devConf=(DeviceConfiguration FAR *)lastBytePtr; // point to next configuration block
   }

   return (d3Cpml);
}


/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME:  SearchConfiguration                              */
/*                                                                    */
/* DESCRIPTIVE NAME:  Search Configuration for specific interface     */
/*                                                                    */
/* FUNCTION:  The function of this routine is to search for interface */
/*            class support, function returns configuration ID,       */
/*            containing required interface and interface index.      */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT:  SearchConfiguration                                  */
/*    LINKAGE:  CALL NEAR                                             */
/*                                                                    */
/* INPUT:  UCHAR FAR *configurationData - far pointer to device       */
/*                      configuration data buffer                     */
/*         UCHAR bNumConfigurations - number of configurations for    */
/*                      device                                        */
/*         UCHAR interfaceClass - interface class to search for;      */
/*         UCHAR interfaceSubClass - subclass to search for, if 0     */
/*                                   value specified - ignored        */
/*         UCHAR interfaceProtocol - protocol to search for, if 0     */
/*                                   specified - field is ignored     */
/*                                                                    */
/* EXIT-NORMAL: non-zero configuration value in low byte and interface*/
/*              index in high byte                                    */
/*                                                                    */
/* EXIT-ERROR:  zero value indicating no interface found              */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:  GetNextDescriptor                            */
/*                                                                    */
/* EXTERNAL REFERENCES:  None                                         */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/
USHORT SearchConfiguration(UCHAR FAR *configurationData, UCHAR bNumConfigurations,
                          UCHAR interfaceClass, UCHAR interfaceSubClass, UCHAR interfaceProtocol)
{
   DeviceConfiguration FAR *devConf;
   DeviceDescHead FAR      *descHead;
   UCHAR                   configIndex;
   UCHAR FAR               *currBytePtr, FAR *lastBytePtr;
   DeviceInterface FAR     *interfaceDesc;

   devConf=(DeviceConfiguration FAR *)configurationData;
   for (configIndex=0; configIndex<bNumConfigurations; configIndex++)
   {
      currBytePtr=(UCHAR FAR *)devConf;
      lastBytePtr=currBytePtr+devConf->wTotalLength;
      descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength);
      for (descHead=(DeviceDescHead FAR *)(currBytePtr+devConf->bLength); descHead;
          descHead=GetNextDescriptor( descHead, lastBytePtr))
      {
         switch (descHead->bDescriptorType)
         {
         case DESC_INTERFACE:
            interfaceDesc=(DeviceInterface FAR *)descHead;
            if(interfaceDesc->bInterfaceClass==interfaceClass &&
               (!interfaceSubClass || interfaceDesc->bInterfaceSubClass==interfaceSubClass) &&
               (!interfaceProtocol || interfaceDesc->bInterfaceProtocol==interfaceProtocol))
            {
               return(MAKEUSHORT(devConf->bConfigurationValue,interfaceDesc->bInterfaceNumber));
            }
            break;
         default:
            break;
         }
      }
      devConf=(DeviceConfiguration FAR *)lastBytePtr; // point to next configuration block
   }

   return (0);
}

