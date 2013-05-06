/* $ID$ */

/* Copyright 2001,2002 Markus Montkowski                                      */
/* This code is released under the Aladdin Free Public License                */
/*                    (Version 9, September 18, 2000)                         */
/* See License file in root of CVS for details                                */

/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME: RESIOCTL.C                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME: USB Resource Manager driver IOCtl routines             */
/*                                                                            */
/*   FUNCTION: These routines handle the task time IOCtl commands to          */
/*             the USB Resource Manager driver.                               */
/*                                                                            */
/*   NOTES:                                                                   */
/*                                                                            */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS: FuncError                                                  */
/*                 GetNumDevices    Special IOCtl Category A0, Function 0x31. */
/*                 GetDeviceInfo    Special IOCtl Category A0, Function 0x32. */
/*                 AquireDevice     Special IOCtl Category A0, Function 0x33. */
/*                 ReleaseDevice    Special IOCtl Category A0, Function 0x34. */
/*                 GetString        Special IOCtl Category A0, Function 0x35. */
/*                 SendControlURB   Special IOCtl Category A0, Function 0x36. */
/*                 SendBulkURB      Special IOCtl Category A0, Function 0x37. */
/*                 VerifyParam                                                */
/*                 VerifyData                                                 */
/*                 RegisterSemaphore   Special IOCtl Category 0xA0,           */
/*                                                   Function 0x41.           */
/*                 DeregisterSemaphore Special IOCtl Category 0xA0,           */
/*                                                   Function 0x42.           */
/*                 RegisterDevSemaphore   Special IOCtl Category 0xA0,        */
/*                                                   Function 0x43.           */
/*                 DeregisterDevSemaphore Special IOCtl Category 0xA          */
/*                                                   Function 0x44.           */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          00/01/14  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#include "res.h"

static PUCHAR BufferVirtAddr (PUCHAR pStringData);

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME: FuncError                                         */
/*                                                                    */
/* DESCRIPTIVE NAME: IOCtl Function code Error                        */
/*                                                                    */
/* FUNCTION: The function of this routine is to return command not    */
/*           supported (bad command) for the request.                 */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT: FuncError                                             */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to Request Packet                             */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR: N/A                                                    */
/*                                                                    */
/* EFFECTS: pRP->rph.Status                                           */
/*                                                                    */
/* INTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

void FuncError (PRP_GENIOCTL pRP)
{
#ifdef DEBUG
   dsPrint2 (DBG_CRITICAL, "USBRESMGR: FuncError, C=%x, F=%x\r\n",
             pRP->Category, pRP->Function);
#endif

   pRP->rph.Status = STDON | STERR | ERROR_I24_BAD_COMMAND;
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: GetDeviceInfo                                     */
/*                                                                    */
/* DESCRIPTIVE NAME: Get USB Device Descriptor                        */
/*                                                                    */
/* FUNCTION: The Device Descriptor of an attached USB device          */
/*                                                                    */
/* NOTES: Special IOCtl Category 0xA0, Function 0x32.                 */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: RegisterSemaphore                                     */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         VerifyParam                                   */
/*                      VerifyData                                    */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void GetDeviceInfo (PRP_GENIOCTL pRP)
{
   USHORT           Configsize,  Config,i;
   DeviceConfiguration FAR     *pDevConfig;
   DeviceDescriptor    FAR     *pDevDesc;
   UCHAR               FAR     *pucByte;
   UCHAR               FAR     *pucBuf;
   SHORT                       DevIndex;



   if (VerifyParam (pRP, 4) ||
       (*(ULONG FAR*)pRP->ParmPacket == 0) )
   {
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
      return;
   }

   (ULONG)DevIndex = *(ULONG FAR*)pRP->ParmPacket;

   if(DevIndex & 0xFFFF0000)
   {
     // Value seams to be a Handle
     DevIndex = DevIndexFromHandle(DevIndex);
     if(DevIndex<0)
     {
       pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
       return;
     }
   }
   else
   {
     // Value seams to be Device Number.
     if(DevIndex > gNumDevices)
     {
       pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
       return;
     }
     DevIndex = gListIndex[DevIndex-1];
   }

   if(gUSBDevices[DevIndex].pDeviceInfo == NULL)
   {
      pRP->rph.Status |= STERR | ERROR_DEV_NOT_EXIST;
      return;
   }
   pDevDesc = &gUSBDevices[DevIndex].pDeviceInfo->descriptor;
   pucByte = (UCHAR FAR*)pDevDesc;
   pDevConfig=(DeviceConfiguration FAR *)(pucByte+pDevDesc->bLength);

   for(Config=0,Configsize=pDevDesc->bLength;Config<pDevDesc->bNumConfigurations;Config++)
   {
     Configsize += pDevConfig->wTotalLength;
     pucByte = (UCHAR FAR*)pDevConfig;
     pDevConfig=(DeviceConfiguration FAR *)(pucByte+pDevConfig->wTotalLength);
   }

   if(pRP->DataLen==0)
   {
     pRP->rph.Status |= STERR | ERROR_BUFFER_OVERFLOW;
     pRP->DataLen = Configsize;
     return;
   }
   else
   {
     USHORT usNumBytes;
     if(pRP->DataLen<Configsize)
     {
       usNumBytes = pRP->DataLen;
       pRP->rph.Status |= STERR | ERROR_MORE_DATA;
     }
     else
       usNumBytes = Configsize;

     if(VerifyData(pRP,usNumBytes))
     {
        pRP->rph.Status |= STERR | ERROR_INVALID_PARAMETER;
        return;
     }
     pucByte = (UCHAR FAR*) pDevDesc;
     pucBuf  = (UCHAR FAR*) pRP->DataPacket;
     for(i=0;i<usNumBytes;i++)
       pucBuf[i] = pucByte[i];
   }
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: GetNumDevices                                     */
/*                                                                    */
/* DESCRIPTIVE NAME: Get Number of attached USB Devices               */
/*                                                                    */
/* FUNCTION: returns the Number of attached USB device                */
/*                                                                    */
/* NOTES: Special IOCtl Category 0xA0, Function 0x31.                 */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: RegisterSemaphore                                     */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         VerifyParam                                   */
/*                      VerifyData                                    */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void  GetNumDevices (PRP_GENIOCTL pRP)
{

   if (VerifyParam (pRP, 0) || VerifyData  (pRP, sizeof(ULONG)))
   {
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
   }
   *(PULONG)pRP->DataPacket = gNumDevices;
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME: VerifyParam                                       */
/*                                                                    */
/* DESCRIPTIVE NAME: Verify access to the Parameter packet            */
/*                                                                    */
/* FUNCTION: This function is used to verify access to the parameter  */
/*           packet.                                                  */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT: VerifyParam                                           */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*        packetLength                                                */
/*                                                                    */
/* EXIT-NORMAL: 0 if access is verified                               */
/*                                                                    */
/* EXIT-ERROR:                                                        */
/*                                                                    */
/* EFFECTS: None                                                      */
/*                                                                    */
/* INTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

USHORT VerifyParam (PRP_GENIOCTL pRP, USHORT packetLength)
{
   if (packetLength)
   {
      if (!(OFFSETOF(pRP->ParmPacket) ||
            SELECTOROF(pRP->ParmPacket) & SELECTOR_MASK))
      {
         return TRUE;
      }
      return (DevHelp_VerifyAccess (SELECTOROF(pRP->ParmPacket),
                                    packetLength,
                                    OFFSETOF(pRP->ParmPacket),
                                    VERIFY_READONLY));
   }
   else
   {
      return (OFFSETOF(pRP->ParmPacket) ||
              SELECTOROF(pRP->ParmPacket) & SELECTOR_MASK);
   }
}

/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME: VerifyData                                        */
/*                                                                    */
/* DESCRIPTIVE NAME: Verify access to the Data packet                 */
/*                                                                    */
/* FUNCTION: This function is used to verify access to the data       */
/*           packet.                                                  */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT: VerifyData                                            */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*        dataLength                                                  */
/*                                                                    */
/* EXIT-NORMAL: 0 if access is verified                               */
/*                                                                    */
/* EXIT-ERROR: N/A                                                    */
/*                                                                    */
/* EFFECTS: None                                                      */
/*                                                                    */
/* INTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

USHORT VerifyData (PRP_GENIOCTL pRP, USHORT dataLength)
{
   if (dataLength)
   {
      if (!(OFFSETOF(pRP->DataPacket) ||
            SELECTOROF(pRP->DataPacket) & SELECTOR_MASK))
      {
         return TRUE;
      }
      return (DevHelp_VerifyAccess (SELECTOROF(pRP->DataPacket),
                                    dataLength,
                                    OFFSETOF(pRP->DataPacket),
                                    VERIFY_READWRITE));
   }
   else
   {
      return (OFFSETOF(pRP->DataPacket) ||
              SELECTOROF(pRP->DataPacket) & SELECTOR_MASK);
   }
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: RegisterSemaphore                                 */
/*                                                                    */
/* DESCRIPTIVE NAME: Register Semaphore                               */
/*                                                                    */
/* FUNCTION: This function registers an event semaphore.              */
/*                                                                    */
/* NOTES: Special IOCtl Category 0xA0, Function 0x41.                 */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: RegisterSemaphore                                     */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         VerifyParam                                   */
/*                      VerifyData                                    */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void  RegisterStatusSemaphore (PRP_GENIOCTL pRP)
{
  USHORT   semIndex;
  PSTATUSEVENTSET pSems;

  if ( VerifyParam (pRP, 0) ||
       VerifyData  (pRP, sizeof(PSTATUSEVENTSET)) )
  {
    #ifdef DEBUG
      dsPrint (DBG_DETAILED, "USBRESMGR: RegisterSemaphore wrong pointer");
    #endif
    pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
  }
  else
  {
    pSems = (PSTATUSEVENTSET)pRP->DataPacket;

    if (pSems->ulSize < sizeof(STATUSEVENTSET) ||
        (pSems->ulCaps & DEV_SEM_MASK) ==0 ||
        ((pSems->ulCaps & DEV_SEM_ADD) && pSems->ulSemDeviceAdd ==0) ||
        ((pSems->ulCaps & DEV_SEM_REMOVE) && pSems->ulSemDeviceRemove ==0) )
    {
      #ifdef DEBUG
        dsPrint4 (DBG_DETAILED,
                  "USBRESMGR: wrong Data :\r\n"
                  " Size  : %d\r\n"
                  " Flags : %x\r\n"
                  " SemA  : %d\r\n"
                  " SemB  : %d\r\n",
                  pSems->ulSize,
                  pSems->ulCaps,
                  pSems->ulSemDeviceAdd,
                  pSems->ulSemDeviceRemove );
      #endif
      pRP->rph.Status |= STERR | ERROR_INVALID_HANDLE;
    }
    else
    {
      semIndex = 0;
      if(pSems->ulCaps & DEV_SEM_ADD)
      {
        #ifdef DEBUG
          dsPrint1 (DBG_DETAILED, "USBRESMGR: RegisterSemaphore Device add%lx\r\n", pSems->ulSemDeviceAdd);
        #endif
        for (; semIndex < MAX_SEMS; semIndex++)
        {
          if (gSEMNewDev[semIndex] == 0)
          {
            gSEMNewDev[semIndex] = pSems->ulSemDeviceAdd; break;
          }
        }
        if (semIndex >= MAX_SEMS)
        {
          #ifdef DEBUG
            dsPrint (DBG_DETAILED, "USBRESMGR: RegisterSemaphore To many Add sems");
          #endif
          pRP->rph.Status |= STERR | ERROR_TOO_MANY_SEMAPHORES;
        }
      }

      if( (semIndex < MAX_SEMS) &&
          (pSems->ulCaps & DEV_SEM_REMOVE) )
      {
        #ifdef DEBUG
          dsPrint1 (DBG_DETAILED, "USBRESMGR: RegisterSemaphore Device removed %lx\r\n", pSems->ulSemDeviceRemove);
        #endif

        for (semIndex = 0; semIndex < MAX_SEMS; semIndex++)
        {
          if (gSEMDevRemove[semIndex] == 0)
          {
            gSEMDevRemove[semIndex] = pSems->ulSemDeviceRemove; break;
          }
        }
        if (semIndex >= MAX_SEMS)
        {
          #ifdef DEBUG
            dsPrint (DBG_DETAILED, "USBRESMGR: RegisterSemaphore To many Add sems");
          #endif
          pRP->rph.Status |= STERR | ERROR_TOO_MANY_SEMAPHORES;
        }
      }
    }
  }

}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: DeregisterSemaphore                               */
/*                                                                    */
/* DESCRIPTIVE NAME: Deregister Semaphore                             */
/*                                                                    */
/* FUNCTION: This function deregisters an event semaphore.            */
/*                                                                    */
/* NOTES: Special IOCtl Category 0xA0, Function 0x42.                 */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: DeregisterSemaphore                                   */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         VerifyParam                                   */
/*                      VerifyData                                    */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void  DeregisterStatusSemaphore (PRP_GENIOCTL pRP)
{
  USHORT   semIndex;
  PSTATUSEVENTSET pSems;

  if ( VerifyParam (pRP, 0) ||
       VerifyData  (pRP, sizeof(STATUSEVENTSET)) )
  {
    pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
  }
  else
  {
    pSems = (PSTATUSEVENTSET)pRP->DataPacket;

    if (pSems->ulSize < sizeof(STATUSEVENTSET) ||
        (pSems->ulCaps & DEV_SEM_MASK) ==0 ||
        ((pSems->ulCaps & DEV_SEM_ADD) && pSems->ulSemDeviceAdd ==0) ||
        ((pSems->ulCaps & DEV_SEM_REMOVE) && pSems->ulSemDeviceRemove ==0) )
    {
      pRP->rph.Status |= STERR | ERROR_SEM_NOT_FOUND;
    }
    else
    {
      semIndex = 0;
      if(pSems->ulCaps & DEV_SEM_ADD)
      {
        #ifdef DEBUG
          dsPrint1 (DBG_DETAILED, "USBRESMGR: DeregisterSemaphore Device add%lx\r\n", pSems->ulSemDeviceAdd);
        #endif
        for (; semIndex < MAX_SEMS; semIndex++)
        {
          if (gSEMNewDev[semIndex] == pSems->ulSemDeviceAdd)
          {
            gSEMNewDev[semIndex] = 0; break;
          }
        }
        if (semIndex >= MAX_SEMS)
        {
          pRP->rph.Status |= STERR | ERROR_SEM_NOT_FOUND;
        }
      }

      if( (semIndex < MAX_SEMS) &&
          (pSems->ulCaps & DEV_SEM_REMOVE) )
      {
        #ifdef DEBUG
          dsPrint1 (DBG_DETAILED, "USBRESMGR: DeregisterSemaphore Device removed %lx\r\n", pSems->ulSemDeviceRemove);
        #endif
        for (semIndex = 0; semIndex < MAX_SEMS; semIndex++)
        {
          if (gSEMDevRemove[semIndex] == pSems->ulSemDeviceRemove)
          {
            gSEMDevRemove[semIndex] = 0; break;
          }
        }
        if (semIndex >= MAX_SEMS)
        {
          pRP->rph.Status |= STERR | ERROR_SEM_NOT_FOUND;
        }
      }
    }
  }
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: DevIndexFromHandle                                */
/*                                                                    */
/* DESCRIPTIVE NAME: Get Index from a USB handle                      */
/*                                                                    */
/* FUNCTION: This function returns the index into gUSBDevices         */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: DevIndexFromHandle                                    */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: ulUSBHandle = Handle of aquired USB device                  */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

SHORT DevIndexFromHandle(ULONG ulUSBHandle)
{
  SHORT DevIndex;
  USHORT usHandleID, usSFN;
  if(ulUSBHandle!=0)
  {
    usSFN = 0x0000FFFF & ulUSBHandle;
    usHandleID = (ulUSBHandle>>16);
    for(DevIndex=0;DevIndex<MAX_DEVICES;DevIndex++)
      if( (gUSBDevices[DevIndex].usHandleID == usHandleID) &&
          (gUSBDevices[DevIndex].usSFN == usSFN) &&
          (gUSBDevices[DevIndex].bAttached))
        return DevIndex;
  }
  return -1;
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: AquireDevice                                      */
/*                                                                    */
/* DESCRIPTIVE NAME: Aquire a USB device for calling application      */
/*                                                                    */
/* FUNCTION: This function returns a handle to the USB Devices        */
/*                                                                    */
/* NOTES: Special IOCtl Category 0xA0, Function 0x33.                 */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: AquireDevice                                          */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         VerifyParam                                   */
/*                      VerifyData                                    */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void AquireDevice(PRP_GENIOCTL pRP)
{
  PAQUIREDEV pAquire;
  ULONG FAR* pUSBDevHandle;
  USHORT     DevIndex,i;

  #ifdef DEBUG
    dsPrint (DBG_DETAILED, "USBRESMGR: AquireDevice\r\n");
  #endif

  if ( VerifyParam (pRP, sizeof(AQUIREDEV)) ||
       VerifyData  (pRP, sizeof(ULONG)) )
  {
    #ifdef DEBUG
      dsPrint (DBG_DETAILED, "USBRESMGR: AquireDevice wrong pointer\r\n");
    #endif
    pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
  }
  else
  {
    pAquire = (PAQUIREDEV)pRP->ParmPacket;
    pUSBDevHandle = (ULONG FAR*)pRP->DataPacket;
    #ifdef DEBUG
      dsPrint4( DBG_DETAILED,
                "USBRESMGR: AquireDevice ID:%x Product:%x BCD:%x Number:%d\r\n",
                pAquire->usVendorID,
                pAquire->usProductID,
                pAquire->usBCDDevice,
                pAquire->usDeviceNumber);
    #endif

    for(DevIndex=0,i=1;DevIndex<MAX_DEVICES;DevIndex++)
    {
      #ifdef DEBUG
        dsPrint1(DBG_DETAILED, "USBRESMGR: Test device %d\r\n",DevIndex);
        if(gUSBDevices[DevIndex].pDeviceInfo)
          dsPrint3( DBG_DETAILED,
                     "USBRESMGR: Device details %x %x %x\r\n",
                     gUSBDevices[DevIndex].pDeviceInfo->descriptor.idVendor,
                     gUSBDevices[DevIndex].pDeviceInfo->descriptor.idProduct,
                     gUSBDevices[DevIndex].pDeviceInfo->descriptor.bcdDevice);
         else
           dsPrint1( DBG_DETAILED, "USBRESMGR: Device %d Not Used\r\n",DevIndex);
      #endif

      if( (gUSBDevices[DevIndex].bAttached) &&
          (gUSBDevices[DevIndex].pDeviceInfo->descriptor.idVendor==pAquire->usVendorID) &&
          (gUSBDevices[DevIndex].pDeviceInfo->descriptor.idProduct==pAquire->usProductID) &&
          ( (pAquire->usBCDDevice == 0xFFFF) ||
            (pAquire->usBCDDevice == gUSBDevices[DevIndex].pDeviceInfo->descriptor.bcdDevice)) )
      {
        // Found Device
        if(gUSBDevices[DevIndex].usSFN == 0)
        {
          #ifdef DEBUG
            dsPrint( DBG_DETAILED, "USBRESMGR: Unused Device FOUND check Number\r\n");
          #endif
          if(pAquire->usDeviceNumber==i ||
             pAquire->usDeviceNumber==0)
          {

            // is the usDeviceNumber device of this type that is available
            ULONG ulDevHandle;
            gUSBDevices[DevIndex].usHandleID;
            ulDevHandle = gUSBDevices[DevIndex].usHandleID;
            ulDevHandle = (ulDevHandle<<16) + pRP->sfn;
            gUSBDevices[DevIndex].usSFN = pRP->sfn;
            //*pUSBDevHandle = ulDevHandle;
            *(PULONG)pRP->DataPacket = ulDevHandle;
            #ifdef DEBUG
              dsPrint1( DBG_DETAILED, "USBRESMGR: Is req. Device return handle %lx\r\n",ulDevHandle);
            #endif
            break;
          }
          else
          {
            #ifdef DEBUG
              dsPrint( DBG_DETAILED, "USBRESMGR: Device number doesn't match\r\n");
            #endif
            i++;
          }
        }
        else
        {
          #ifdef DEBUG
            dsPrint( DBG_DETAILED, "USBRESMGR: Device In Use\r\n");
          #endif
          i++;
        }
      }
      #ifdef DEBUG
      else
      {
        dsPrint2( DBG_DETAILED,
                 "USBRESMGR: Device Attached : %s\r\n"
                 "           Filehandle      : 0x%x",
                 gUSBDevices[DevIndex].bAttached?"TRUE":"FALSE",
                 gUSBDevices[DevIndex].usSFN);

      }
      #endif
    }
    // No device found return NULL handle
    if(DevIndex==MAX_DEVICES)
    {
      *pUSBDevHandle = 0;
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE; // ToDo Better Errorcode
    }
  }
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: ReleaseDevice                                     */
/*                                                                    */
/* DESCRIPTIVE NAME: Release a USB device aquired by the application  */
/*                                                                    */
/* FUNCTION: This function returns releases a USB device so it can be */
/*           used by by other applications                            */
/*                                                                    */
/* NOTES: Special IOCtl Category 0xA0, Function 0x34.                 */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: ReleaseDevice                                         */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         VerifyParam                                   */
/*                      VerifyData                                    */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void ReleaseDevice(PRP_GENIOCTL pRP)
{
  USHORT DevIndex;
  ULONG FAR* pUSBDevHandle;
  if ( VerifyParam (pRP, sizeof(ULONG)))
  {
    #ifdef DEBUG
      dsPrint (DBG_DETAILED, "USBRESMGR: ReleaseDevice wrong pointer\r\n");
    #endif
    pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
  }
  else
  {
    pUSBDevHandle =(ULONG FAR*)pRP->ParmPacket;
    #ifdef DEBUG
      dsPrint3( DBG_DETAILED,
                "USBRESMGR: try to release handle %lx => sfn %x, ID %x\r\n",
                *pUSBDevHandle,
                *pUSBDevHandle & 0x0000FFFF,
                *pUSBDevHandle>>16);
    #endif
    if((*pUSBDevHandle & 0x0000FFFF) == pRP->sfn)
    {
      USHORT usHandleID;
      #ifdef DEBUG
        dsPrint( DBG_DETAILED,
                  "  Look for Handle ID\r\n");
      #endif

      usHandleID = (*pUSBDevHandle>>16);
      for(DevIndex=0;DevIndex<MAX_DEVICES;DevIndex++)
        if(gUSBDevices[DevIndex].usHandleID == usHandleID)
          break;

      if(DevIndex<MAX_DEVICES)
      {
        #ifdef DEBUG
          dsPrint( DBG_DETAILED,
                    "  ID FOUND\r\n");
        #endif
        gUSBDevices[DevIndex].usSFN = 0;
        // update the ID so we don't return the same handle if device  gets reopened
        gUSBDevices[DevIndex].usHandleID = gusHandleCounter++;
      }
      else
        pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
    }
    else
    {
      #ifdef DEBUG
        dsPrint( DBG_DETAILED,
                  "  Handle doesn't belong to SFN\r\n");
      #endif
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
    }
  }
}

static USHORT Request (PUCHAR pDataBuffer, SHORT DevIndex, ULONG ulLockHandle, USHORT irq)
{
   USBRB          rb;   // USB I/O Request Block
   RP_GENIOCTL    rp;   // IOCTL Request Packet to USBD

#ifdef DEBUG
   dsPrint3 (DBG_HLVLFLOW, "USBINFO: Request=%x,%x,%x\r\n",
             gUSBDevices[DevIndex].setupPack.bmRequestType,
             gUSBDevices[DevIndex].setupPack.bRequest,
             gUSBDevices[DevIndex].setupPack.wValue);
#endif

   setmem ((PSZ)&rb, 0, sizeof(rb));
   rb.controllerId  = gUSBDevices[DevIndex].pDeviceInfo->ctrlID;
   rb.deviceAddress = gUSBDevices[DevIndex].pDeviceInfo->deviceAddress;
   rb.endPointId    = USB_DEFAULT_CTRL_ENDPT;
   rb.flags         = USRB_FLAGS_TTYPE_SETUP;
   rb.buffer1       = (PUCHAR)&gUSBDevices[DevIndex].setupPack;
   rb.buffer1Length = sizeof(SetupPacket);
   rb.buffer2       = pDataBuffer;
   rb.buffer2Length = gUSBDevices[DevIndex].setupPack.wLength;
   rb.serviceTime   = USB_DEFAULT_SRV_INTV;
   rb.maxPacketSize = USB_DEFAULT_PKT_SIZE;
   rb.maxErrorCount = USB_MAX_ERROR_COUNT;
   rb.usbIDC        = (PUSBIDCEntry)IDCEntry;
   rb.usbDS         = GetDS();
   rb.category      = USB_IDC_CATEGORY_CLASS;
   rb.requestData1  = irq;
   rb.requestData2  = DevIndex;
   rb.requestData3  = ulLockHandle;

   setmem((PSZ)&rp, 0, sizeof(rp));
   rp.rph.Cmd = CMDGenIOCTL;
   rp.Category = USB_IDC_CATEGORY_USBD;
   rp.Function = USB_IDC_FUNCTION_ACCIO;
   rp.ParmPacket = (PVOID)&rb;

   USBCallIDC (gpUSBDIDC, gdsUSBDIDC, (PRP_GENIOCTL)&rp);

#ifdef DEBUG
   dsPrint2 (DBG_DETAILED, "USBRES: RequeSt=%x,%x\r\n", rp.rph.Status, rb.status);
#endif
   return rp.rph.Status;
}

void GetStringD (SHORT DevIndex, ULONG ulLockHandle)
{
#ifdef DEBUG
   dsPrint1 (DBG_HLVLFLOW, "USBRES: GetStringD for Device i=%d Length %x\r\n",DevIndex,gusStringBuffer[0]);
   dsPrint2 ( DBG_HLVLFLOW,
              "         SetupPacket:\r\n"
              "          - Type    %x\r\n"
              "          - Request %x\r\n",
              gUSBDevices[DevIndex].setupPack.bmRequestType,
              gUSBDevices[DevIndex].setupPack.bRequest );
   dsPrint2 ( DBG_HLVLFLOW,
              "          - Value   %x\r\n"
              "          - Index   %x\r\n",
              gUSBDevices[DevIndex].setupPack.wValue,
              gUSBDevices[DevIndex].setupPack.wIndex);
#endif

   gUSBDevices[DevIndex].setupPack.wLength = gusStringBuffer[0] & 0x00FF;

   Request (&gusStringBuffer[0], DevIndex, ulLockHandle, RES_IRQ_STRING);
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: GetString                                         */
/*                                                                    */
/* DESCRIPTIVE NAME: Get a String descriptor from the device          */
/*                                                                    */
/* FUNCTION:                                                          */
/*                                                                    */
/* NOTES: Special IOCtl Category 0xA0, Function 0x35.                 */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: GetString                                             */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         VerifyParam                                   */
/*                      VerifyData                                    */
/*                      DevHelp_SemRequest                            */
/*                      Devhelp_Lock                                  */
/*                      DevIndexFromHandle                            */
/*                      USBCallIDC                                    */
/*                      DevHelp_ProcBlock                             */
/*                      DevHelp_UnLock                                */
/*                      DevHelp_SemClear                              */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/

void GetString(PRP_GENIOCTL pRP)
{
  USBRB          rb;   // USB I/O Request Block
  RP_GENIOCTL    rp;   // IOCTL Request Packet to USBD
  PGETSTRINGPARM pParm;
  UCHAR FAR     *pData;

  DevHelp_SemRequest(&gulSemStringBuffer,-1);
  if ( VerifyParam (pRP, sizeof(GETSTRINGPARM)) )
  {
    #ifdef DEBUG
      dsPrint (DBG_DETAILED, "USBRESMGR: GetString wrong Param pointer\r\n");
    #endif
    pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
  }
  else
  {
    ULONG ulLockHandle;
    USHORT rc;
    rc = DevHelp_Lock( SELECTOROF(pRP->DataPacket),
                       LOCKTYPE_SHORT_ANYMEM,
                       0,
                       &ulLockHandle);
    if(0==rc && 0==VerifyData(pRP, 514) )
    {
      SHORT DevIndex;
      pParm = (PGETSTRINGPARM) pRP->ParmPacket;
      pData = (UCHAR FAR*) pRP->DataPacket;
      DevIndex = DevIndexFromHandle(pParm->ulDevHandle);
      if(DevIndex>=0)
      {
        gUSBDevices[DevIndex].setupPack.bmRequestType = REQTYPE_TYPE_STANDARD |
                                                        REQTYPE_RECIPIENT_DEVICE |
                                                        REQTYPE_XFERDIR_DEVTOHOST;
        gUSBDevices[DevIndex].setupPack.bRequest      = REQ_GET_DESCRIPTOR;
        gUSBDevices[DevIndex].setupPack.wValue        = MAKEUSHORT(pParm->ucStringID, DESC_STRING);
        gUSBDevices[DevIndex].setupPack.wIndex        = pParm->usLangID;
        gUSBDevices[DevIndex].setupPack.wLength       = 2;

#if 0
        setmem ((PSZ)&rb, 0, sizeof(rb));
        rb.controllerId  = gUSBDevices[DevIndex].pDeviceInfo->ctrlID;
        rb.deviceAddress = gUSBDevices[DevIndex].pDeviceInfo->deviceAddress;
        rb.endPointId    = USB_DEFAULT_CTRL_ENDPT;
        rb.flags         = USRB_FLAGS_TTYPE_SETUP;
        rb.buffer1       = (PUCHAR)&gUSBDevices[DevIndex].setupPack;
        rb.buffer1Length = sizeof(SetupPacket);
        rb.buffer2       = gusStringBuffer;
        rb.buffer2Length = gUSBDevices[DevIndex].setupPack.wLength;
        rb.serviceTime   = USB_DEFAULT_SRV_INTV;
        rb.maxPacketSize = USB_DEFAULT_PKT_SIZE;
        rb.maxErrorCount = USB_MAX_ERROR_COUNT;
        rb.usbIDC        = (PUSBIDCEntry)IDCEntry;
        rb.usbDS         = GetDS();
        rb.category      = USB_IDC_CATEGORY_CLASS;
        rb.requestData1  = RES_IRQ_STRINGLEN;
        rb.requestData2  = DevIndex;
        rb.requestData3  = ulLockHandle;

        setmem((PSZ)&rp, 0, sizeof(rp));
        rp.rph.Cmd    = CMDGenIOCTL;
        rp.Category   = USB_IDC_CATEGORY_USBD;
        rp.Function   = USB_IDC_FUNCTION_ACCIO;
        rp.ParmPacket = (PVOID)&rb;

        #ifdef DEBUG
          dsPrint (DBG_DETAILED, "USBRESMGR: Send IDC Request\r\n");
        #endif
        gusStringBuffer[0] = 0;
        gusStringBuffer[1] = 0;
        USBCallIDC (gpUSBDIDC, gdsUSBDIDC, (PRP_GENIOCTL)&rp);
        if (rp.rph.Status != USB_IDC_RC_OK)
#else
        gusStringBuffer[0] = 0;
        gusStringBuffer[1] = 0;
        rc = Request(&gusStringBuffer[0], DevIndex, ulLockHandle, RES_IRQ_STRINGLEN);
        if (rc != USB_IDC_RC_OK)
#endif
        {
          #ifdef DEBUG
#if 0
            dsPrint1( DBG_DETAILED,
                      "USBRESMGR: USBCallIDC returned an error: %d \r\n",
                      rp.rph.Status);
#else
            dsPrint1( DBG_DETAILED,
                      "USBRESMGR: USBCallIDC returned an error: %d \r\n",
                      rc);
#endif
          #endif
          pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
        }
        else
        {
          rc = DevHelp_ProcBlock( ulLockHandle,
                                  6000,//-1,
                                  WAIT_IS_INTERRUPTABLE);
          #ifdef DEBUG
            dsPrint4( DBG_DETAILED,
                      "USBRESMGR: ProcBlock returned %d %x\r\n           StringBuffer: %x %x\r\n",
                      rc,
                      rc,
                      gusStringBuffer[0],
                      gusStringBuffer[1]);
          #endif
          if(rc == WAIT_TIMED_OUT && rb.requestData1 != RES_IRQ_PROCESSED)
            pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
          else
          {
            movmem( pData,
                    gusStringBuffer,
                    gusStringBuffer[0]*2<514?gusStringBuffer[0]*2:514);
          }
        }
        DevHelp_UnLock(ulLockHandle);
      }
    }
    else
    {
      DevHelp_UnLock(ulLockHandle);
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
    }
  }
  DevHelp_SemClear(&gulSemStringBuffer);
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: GetDeviceInfo                                     */
/*                                                                    */
/* DESCRIPTIVE NAME: Get Information about device                     */
/*                                                                    */
/* FUNCTION: Gets Connection Information of the Device                */
/*                                                                    */
/* NOTES: Special IOCtl Category 0xA0, Function 0x39.                 */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: SendControlURB                                        */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         VerifyParam                                   */
/*                      VerifyData                                    */
/*                      DevIndexFromHandle                            */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/
void GetDevInfo(PRP_GENIOCTL pRP)
{
  SHORT DevIndex;

  if ( VerifyParam (pRP, sizeof(ULONG)) ||
       VerifyData(pRP, sizeof(GETDEVINFODATA)) )
  {
    #ifdef DEBUG
      dsPrint (DBG_DETAILED, "USBRESMGR: GetDevInfo wrong Param or Data pointer\r\n");
    #endif
    pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
  }
  else
  {
    DevIndex = DevIndexFromHandle(*(ULONG*)pRP->ParmPacket);
    if(DevIndex>=0)
    {
      PGETDEVINFO pData = (PGETDEVINFO)pRP->DataPacket;
      pData->ctrlID              = gUSBDevices[DevIndex].pDeviceInfo->ctrlID;
      pData->deviceAddress       = gUSBDevices[DevIndex].pDeviceInfo->deviceAddress;
      pData->bConfigurationValue = gUSBDevices[DevIndex].pDeviceInfo->bConfigurationValue;
      pData->bInterfaceNumber    = gUSBDevices[DevIndex].pDeviceInfo->bInterfaceNumber;
      pData->lowSpeedDevice      = gUSBDevices[DevIndex].pDeviceInfo->lowSpeedDevice;
      pData->portNum             = gUSBDevices[DevIndex].pDeviceInfo->portNum;
      pData->parentHubIndex      = gUSBDevices[DevIndex].pDeviceInfo->parentHubIndex;
      pData->rmDevHandle         = gUSBDevices[DevIndex].pDeviceInfo->rmDevHandle;
    }
    else
    {
      #ifdef DEBUG
        dsPrint (DBG_DETAILED, "USBRESMGR: GetDevInfo Device not found\r\n");
      #endif
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
    }

  }
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: SendControlURB                                    */
/*                                                                    */
/* DESCRIPTIVE NAME: Send Control URB                                 */
/*                                                                    */
/* FUNCTION: Send a URB to the control endpoint                       */
/*                                                                    */
/* NOTES: Special IOCtl Category 0xA0, Function 0x35.                 */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: SendControlURB                                        */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         VerifyParam                                   */
/*                      VerifyData                                    */
/*                      DevHelp_SemRequest                            */
/*                      Devhelp_Lock                                  */
/*                      DevIndexFromHandle                            */
/*                      USBCallIDC                                    */
/*                      DevHelp_ProcBlock                             */
/*                      DevHelp_UnLock                                */
/*                      DevHelp_SemClear                              */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/
void SendControlURB(PRP_GENIOCTL pRP)
{
  USBRB          rb;   // USB I/O Request Block
  RP_GENIOCTL    rp;   // IOCTL Request Packet to USBD
  PLIBUSB_CTRL_REQ pParm;
  UCHAR FAR     *pData;
  SHORT DevIndex;
  ULONG ulLockHandle;
  if ( VerifyParam (pRP, sizeof(LIBUSB_CTRL_REQ)) )
  {
    #ifdef DEBUG
      dsPrint (DBG_DETAILED, "USBRESMGR: SendControlURB wrong Param pointer\r\n");
    #endif
    pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
  }
  else
  {
    pParm = (PLIBUSB_CTRL_REQ) pRP->ParmPacket;
    pData = (UCHAR FAR*) pRP->DataPacket;
    DevIndex = DevIndexFromHandle(pParm->ulDevHandle);
    if(DevIndex>=0)
    {
      USHORT rc;
      if( (pParm->bRequestType == 0) &&
          (pParm->bRequest ==REQ_SET_CONFIGURATION) &&
          (gUSBDevices[DevIndex].pDeviceInfo->bConfigurationValue == pParm->wValue) )
        return; // Nothing to Do is current config

      if(pParm->wLength)
      {
        rc = DevHelp_Lock( SELECTOROF(pRP->DataPacket),
                           LOCKTYPE_SHORT_ANYMEM,
                           0,
                           &ulLockHandle);
        if(0!=rc)
        {
          #ifdef DEBUG
            dsPrint (DBG_DETAILED, "USBRESMGR: SendControlURB Lock Data failed\r\n");
          #endif
          pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
        }
        else
        {
          if(0!=VerifyData(pRP, pParm->wLength) )
          {
            DevHelp_UnLock(ulLockHandle);
            #ifdef DEBUG
              dsPrint (DBG_DETAILED, "USBRESMGR: SendControlURB Lock Data failed\r\n");
            #endif
            pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
          }
        }
      }

      if(!(pRP->rph.Status & STERR))
      {
        gUSBDevices[DevIndex].setupPack.bmRequestType = pParm->bRequestType;
        gUSBDevices[DevIndex].setupPack.bRequest      = pParm->bRequest;
        gUSBDevices[DevIndex].setupPack.wValue        = pParm->wValue;
        gUSBDevices[DevIndex].setupPack.wIndex        = pParm->wIndex;
        gUSBDevices[DevIndex].setupPack.wLength       = pParm->wLength;
        setmem ((PSZ)&rb, 0, sizeof(rb));
        rb.controllerId  = gUSBDevices[DevIndex].pDeviceInfo->ctrlID;
        rb.deviceAddress = gUSBDevices[DevIndex].pDeviceInfo->deviceAddress;
        rb.endPointId    = USB_DEFAULT_CTRL_ENDPT;
        rb.flags         = USRB_FLAGS_TTYPE_SETUP;
        rb.buffer1       = (PUCHAR)&gUSBDevices[DevIndex].setupPack;
        rb.buffer1Length = sizeof(SetupPacket);
        rb.buffer2       = pData;
        //rb.buffer2       = gusStringBuffer;
        rb.buffer2Length = gUSBDevices[DevIndex].setupPack.wLength;
        //rb.serviceTime   = USB_DEFAULT_SRV_INTV;
        rb.serviceTime   = pParm->ulTimeout;
        rb.maxPacketSize = USB_DEFAULT_PKT_SIZE;
        rb.maxErrorCount = USB_MAX_ERROR_COUNT;
        rb.usbIDC        = (PUSBIDCEntry)IDCEntry;
        rb.usbDS         = GetDS();
        rb.category      = USB_IDC_CATEGORY_CLASS;
        rb.requestData1  = RES_IRQ_CONTROL;
        rb.requestData2  = DevIndex;
        rb.requestData3  = pParm;

        setmem((PSZ)&rp, 0, sizeof(rp));
        rp.rph.Cmd    = CMDGenIOCTL;
        rp.Category   = USB_IDC_CATEGORY_USBD;
        rp.Function   = USB_IDC_FUNCTION_ACCIO;
        rp.ParmPacket = (PVOID)&rb;

        #ifdef DEBUG
          dsPrint (DBG_DETAILED, "USBRESMGR: Send IDC Request\r\n");
        #endif
        USBCallIDC ( gpUSBDIDC, gdsUSBDIDC, (PRP_GENIOCTL)&rp);
        #ifdef DEBUG
          dsPrint1(DBG_DETAILED, "USBRESMGR: IDC Call returned %x\r\n",rp.rph.Status);
        #endif
        rc = DevHelp_ProcBlock( pParm,
                                3000,//-1
                                WAIT_IS_INTERRUPTABLE);
        #ifdef DEBUG
          dsPrint2( DBG_DETAILED,
                    "USBRESMGR: ProcBlock returned %d %x\r\n",
                    rc,
                    rc);
        #endif
        if(rc==WAIT_TIMED_OUT)
          pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
        else
        {
          if(pParm->bRequestType == 0)
          {
            switch(pParm->bRequest)
            {
              case REQ_SET_CONFIGURATION:
                gUSBDevices[DevIndex].pDeviceInfo->bConfigurationValue = pParm->wValue;
              case REQ_SET_INTERFACE:
                gUSBDevices[DevIndex].wToggle[0] = 0x0;
                gUSBDevices[DevIndex].wToggle[1] = 0x0;
                break;
            }
          }
          else
            if( (pParm->bRequestType == REQTYPE_RECIPIENT_ENDPOINT | REQTYPE_XFERDIR_HOSTTODEV) &&
                (pParm->bRequest     == REQ_CLEAR_FEATURE) &&                 
                (pParm->wValue       == ENDPOINT_STALL) )

            {
              gUSBDevices[DevIndex].wToggle[(pParm->wIndex & DEV_ENDPT_DIRIN)?0:1] |= (1<< pParm->wIndex);
            }
        }

        if(pParm->wLength)
        {
          //movmem(pData,gusStringBuffer,pParm->wLength);
          DevHelp_UnLock(ulLockHandle);
        }
      }
    }
    else
    {
      #ifdef DEBUG
        dsPrint (DBG_DETAILED, "USBRESMGR: SendControlURB Lock Data failed\r\n");
      #endif
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
    }
  }
}

PUSB_IORB AllocIoRB(ULONG ulEvent)
{
  USHORT IoRB=0;

  if(ulEvent==0)
    return NULL;

  while(IoRB<USB_MAX_IORBS)
  {
    if (gaIoRB[IoRB].ulEventDone==0)
    {
      gaIoRB[IoRB].ulEventDone = ulEvent;
      gaIoRB[IoRB].ulID        = ++gulTotalIoRBs;
      gNumIoRB++;
      return &gaIoRB[IoRB];
    }
    IoRB++;
  }
  return NULL;
}

BOOL FreeIoRB(PUSB_IORB pIoRB)
{
  USHORT IoRB=0;
  if( (pIoRB < &gaIoRB[0]) ||
      (pIoRB >= &gaIoRB[USB_MAX_IORBS]) ||
//      ((void*)pIoRB - (void*)&gaIoRB[0])%sizeof(gaIoRB[0]) ||
      (pIoRB->ulEventDone==0))
    return FALSE;
  
  pIoRB->ulEventDone = 0;
  pIoRB->pDevice     = NULL;
  pIoRB->pEndpoint   = NULL;
  pIoRB->pParam      = NULL;
  pIoRB->pData       = NULL;
  pIoRB->pPhys       = 0;
  pIoRB->pPhysData   = 0;
  pIoRB->ulLockData  = 0;
  pIoRB->ulLockParam = 0;
  pIoRB->ulID        = 0;
  pIoRB->usDataRemain  = 0;
  pIoRB->usDataProcessed = 0;
  gNumIoRB--;

  return TRUE;
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: SendBulkURB                                       */
/*                                                                    */
/* DESCRIPTIVE NAME: Send a Bulk URB to the Device                    */
/*                                                                    */
/* FUNCTION: Read/Write Data from/to a Bulk Endpoint                  */
/*                                                                    */
/* NOTES: Special IOCtl Category 0xA0, Function 0x37.                 */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: SendBulkURB                                           */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         VerifyParam                                   */
/*                      VerifyData                                    */
/*                      DevHelp_SemRequest                            */
/*                      Devhelp_Lock                                  */
/*                      DevIndexFromHandle                            */
/*                      USBCallIDC                                    */
/*                      DevHelp_ProcBlock                             */
/*                      DevHelp_UnLock                                */
/*                      DevHelp_SemClear                              */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/
void SendBulkURB(PRP_GENIOCTL pRP)
{
  PLIBUSB_BULK_REQ pParm;
  UCHAR FAR     *pData;
  SHORT DevIndex;
  DeviceEndpoint FAR * pEP;
  PUSB_IORB pIoRB;
  USHORT rc;

  #ifdef DEBUG
    dsPrint (DBG_DETAILED, "USBRESMGR: SendBulkURB\r\n");
  #endif

  if ( VerifyParam (pRP, sizeof(LIBUSB_BULK_REQ)) )
  {
    #ifdef DEBUG
      dsPrint (DBG_DETAILED, "USBRESMGR: SendBulkURB wrong Param pointer\r\n");
    #endif
    pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
    return;
  }

  pParm = (PLIBUSB_BULK_REQ) pRP->ParmPacket;
  pData = (UCHAR FAR*) pRP->DataPacket;
  DevIndex = DevIndexFromHandle(pParm->ulDevHandle);
  if(DevIndex>=0)
    pEP = GetEndpointDPtr( gUSBDevices[DevIndex].pDeviceInfo->configurationData,
                           gUSBDevices[DevIndex].pDeviceInfo->descriptor.bNumConfigurations,
                           gUSBDevices[DevIndex].pDeviceInfo->bConfigurationValue,
                           pParm->ucInterface,
                           pParm->ucEndpoint);
  else
    pEP =NULL;

  if( (pEP==NULL) || 
      ((pEP->bmAttributes & DEV_ENDPT_ATTRMASK)!=DEV_ENDPT_BULK) )
  {
    #ifdef DEBUG
      dsPrint (DBG_DETAILED, "USBRESMGR: SendBulkURB wrong Endpoint\r\n");
    #endif
    pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
    return;
  }

  pIoRB = AllocIoRB(pParm->ulEventDone);
  if(pIoRB==NULL)
  {
    #ifdef DEBUG
      dsPrint (DBG_DETAILED, "USBRESMGR: No free IoRB\r\n");
    #endif
    pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
    return;
  }

  rc = DevHelp_Lock( SELECTOROF(pRP->DataPacket),
                     LOCKTYPE_LONG_ANYMEM,
                     0,
                     &pIoRB->ulLockData);
  if(0!=rc)
  {
    #ifdef DEBUG
      dsPrint (DBG_DETAILED, "USBRESMGR: SendBulkURB Lock Memory failed\r\n");
    #endif
    pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
  }
  else
  {
    if( VerifyData(pRP, pRP->DataLen) )
    {
      #ifdef DEBUG
        dsPrint (DBG_DETAILED, "USBRESMGR: SendBulkURB Verify Data failed\r\n");
      #endif
      DevHelp_UnLock(pIoRB->ulLockData);
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
    }
    else
    {
      rc = DevHelp_Lock( SELECTOROF(pRP->ParmPacket),
                         LOCKTYPE_LONG_ANYMEM,
                         0,
                         &pIoRB->ulLockParam);
      if(0!=rc)
      {
        #ifdef DEBUG
          dsPrint (DBG_DETAILED, "USBRESMGR: SendBulkURB Lock Memory failed\r\n");
        #endif
        pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
        DevHelp_UnLock(pIoRB->ulLockData);
      }
      else
      {
        rc = DevHelp_VirtToPhys(pRP->ParmPacket, &pIoRB->pPhys);
        if(0!=rc)
        {
          #ifdef DEBUG
            dsPrint (DBG_DETAILED, "USBRESMGR: VirtToPhys Memory failed for Param\r\n");
          #endif
          pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
          DevHelp_UnLock(pIoRB->ulLockParam);
          DevHelp_UnLock(pIoRB->ulLockData);
        }
        else
        {
          rc = DevHelp_VirtToPhys(pRP->DataPacket, &pIoRB->pPhysData);
          if(0!=rc)
          {
            #ifdef DEBUG
              dsPrint (DBG_DETAILED, "USBRESMGR: VirtToPhys Memory failed for Data\r\n");
            #endif
            pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
            DevHelp_UnLock(pIoRB->ulLockParam);
            DevHelp_UnLock(pIoRB->ulLockData);
          }
        }
      }
    }
  }
  if(pRP->rph.Status & STERR)
  {
    FreeIoRB(pIoRB);
    return;
  }
  pIoRB->pDevice   = &gUSBDevices[DevIndex];
  pIoRB->pEndpoint = pEP;
  pIoRB->pData     = pData;
  pIoRB->usDataProcessed = 0;
  pIoRB->usDataRemain = pRP->DataLen;
  pIoRB->pParam    = pParm;
  pIoRB->pParam->usDataProcessed = 0;
  pIoRB->pParam->usDataRemain = pRP->DataLen;
  pIoRB->pParam->usStatus = USB_IORB_WORKING;

  rc = ProcessIoRB(pIoRB);
  if(rc&STERR)
    pRP->rph.Status = rc;
  #ifdef DEBUG
    dsPrint (DBG_DETAILED, "USBRESMGR: SendBulkURB Done\r\n");
  #endif
}

void ProcessIoRB(PUSB_IORB pIoRB)
{
  USBRB          rb;   // USB I/O Request Block
  RP_GENIOCTL    rp;   // IOCTL Request Packet to USBD
  USHORT usToggleGrp, usBytesLeft, usBytesMoved;
 
  setmem ((PSZ)&rb, 0, sizeof(rb));
  rb.controllerId  = pIoRB->pDevice->pDeviceInfo->ctrlID;
  rb.deviceAddress = pIoRB->pDevice->pDeviceInfo->deviceAddress;
  rb.endPointId    = pIoRB->pEndpoint->bEndpointAddress;
  if(pIoRB->pEndpoint->bEndpointAddress & DEV_ENDPT_DIRIN)
  {
    #ifdef DEBUG
      dsPrint2 ( DBG_DETAILED,
                 "USBRESMGR: ProcessIoRB Read %d Bytes from Endpoint 0x%x\r\n",
                 pIoRB->usDataRemain,
                 pIoRB->pEndpoint->bEndpointAddress);
    #endif
    rb.flags  = USRB_FLAGS_TTYPE_IN | USRB_FLAGS_DET_BULK;
    usToggleGrp =0;
  }
  else
  {
    #ifdef DEBUG
      dsPrint2 ( DBG_DETAILED,
                 "USBRESMGR: ProcessIoRB Write %d Bytes to Endpoint 0x%x\r\n",
                 pIoRB->usDataRemain,
                 pIoRB->pEndpoint->bEndpointAddress);
    #endif
    rb.flags         = USRB_FLAGS_TTYPE_OUT | USRB_FLAGS_DET_BULK;
    usToggleGrp = 1;
  }
  #ifdef DEBUG
    #ifdef BULK_USE_PHYS
        dsPrint2 ( DBG_IRQFLOW,
                   "USBRESMGR: PHYS Toggle Group %d, Toggle %x\r\n",
                   usToggleGrp,
                   pIoRB->pDevice->wToggle[usToggleGrp]);
    #else
        dsPrint2 ( DBG_IRQFLOW,
                   "USBRESMGR: Toggle Group %d, Toggle %x\r\n",
                   usToggleGrp,
                   pIoRB->pDevice->wToggle[usToggleGrp]);
    #endif
  #endif
  
  if(pIoRB->pDevice->wToggle[usToggleGrp] & (1<<(pIoRB->pEndpoint->bEndpointAddress &DEV_ENDPT_ADDRMASK)) )
  {
    #ifdef DEBUG
      dsPrint( DBG_IRQFLOW, "USBRESMGR: Set USRB_FLAGS_DET_DTGGLEON\r\n");
    #endif
    rb.flags |= USRB_FLAGS_DET_DTGGLEON;
  }

  rb.flags |= USRB_FLAGS_BUF1PHYS;
  rb.buffer1       = pIoRB->pPhysData;
  rb.buffer1Length = pIoRB->usDataRemain;
  rb.buffer2       = 0;
  rb.buffer2Length = 0;
  rb.serviceTime   = USB_DEFAULT_SRV_INTV;
  rb.maxPacketSize = pIoRB->pEndpoint->wMaxPacketSize;
  rb.maxErrorCount = USB_MAX_ERROR_COUNT;
  rb.usbIDC        = (PUSBIDCEntry)IDCEntry;
  rb.usbDS         = GetDS();
  rb.category      = USB_IDC_CATEGORY_CLASS;
  rb.requestData1  = RES_IRQ_RWBULK;
  rb.requestData2  = pIoRB;
  rb.requestData3  = pIoRB->ulID;

  setmem((PSZ)&rp, 0, sizeof(rp));
  rp.rph.Cmd    = CMDGenIOCTL;
  rp.Category   = USB_IDC_CATEGORY_USBD;
  rp.Function   = USB_IDC_FUNCTION_ACCIO;
  rp.ParmPacket = (PVOID)&rb;

  #ifdef DEBUG
    dsPrint1 ( DBG_DETAILED,
               "USBRESMGR: Send IDC Request to handle %d bytes\r\n",
               rb.buffer1Length);
  #endif
  USBCallIDC (gpUSBDIDC, gdsUSBDIDC, (PRP_GENIOCTL)&rp);

  #ifdef DEBUG
    dsPrint1( DBG_DETAILED,
              "USBRESMGR: IDC Call returned %x\r\n",
              rp.rph.Status);
  #endif
  if(rp.rph.Status & STERR)
  {
    DevHelp_Beep(20000,1000);
    if(0==DevHelp_PhysToGDTSelector(pIoRB->pPhys,
                                    sizeof(LIBUSB_BULK_REQ),
                                    gSelIoRB[SEL_BULK]))
    {
      PLIBUSB_BULK_REQ pBulkReq = MAKEP(gSelIoRB[SEL_BULK], NULL);
      pBulkReq->usStatus = USB_IORB_FAILED;
    }

    DevHelp_UnLock(pIoRB->ulLockData);
    DevHelp_UnLock(pIoRB->ulLockParam);
    if(DevHelp_OpenEventSem (pIoRB->ulEventDone) == 0)  
    {
      DevHelp_PostEventSem (pIoRB->ulEventDone);
      DevHelp_CloseEventSem (pIoRB->ulEventDone);
    }
    FreeIoRB(pIoRB);
  }
}


void CancelIoRB(PRP_GENIOCTL pRP)
{
  PLIBUSB_BULK_REQ pParm;
  USHORT i;

  if ( VerifyParam (pRP, sizeof(LIBUSB_BULK_REQ)) )
  {
    #ifdef DEBUG
      dsPrint (DBG_DETAILED, "USBRESMGR: SendBulkURB wrong Param pointer\r\n");
    #endif
    pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
    return;
  }
  pParm = (PLIBUSB_BULK_REQ) pRP->ParmPacket;
  for(i=0;i<USB_MAX_IORBS;i++)
  {
    if( (pParm->ulEventDone==gaIoRB[i].ulEventDone) &&
        (gaIoRB[i].pParam))
    {
      if(0==DevHelp_PhysToGDTSelector(gaIoRB[i].pPhys,
                                sizeof(LIBUSB_BULK_REQ),
                                gSelIoRB[SEL_BULK]))
      {
        PLIBUSB_BULK_REQ pBulkReq = MAKEP(gSelIoRB[SEL_BULK], NULL);

        if((pParm->ulDevHandle=pBulkReq->ulDevHandle) &&
           (pParm->ucEndpoint==pBulkReq->ucEndpoint))
        {
          CancelRequests(gaIoRB[i].pDevice, pParm->ucEndpoint);
          FreeIoRB(&gaIoRB[i]);
          break;
        }
      }
    }
  }
}


/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: SendIRQURB                                        */
/*                                                                    */
/* DESCRIPTIVE NAME: Send a Bulk URB to the Device                    */
/*                                                                    */
/* FUNCTION: Read an interrupt endpoint                               */
/*                                                                    */
/* NOTES: Special IOCtl Category 0xA0, Function 0x38.                 */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: SendIRQURB                                            */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         VerifyParam                                   */
/*                      VerifyData                                    */
/*                      DevHelp_SemRequest                            */
/*                      Devhelp_Lock                                  */
/*                      DevIndexFromHandle                            */
/*                      USBCallIDC                                    */
/*                      DevHelp_ProcBlock                             */
/*                      movmem                                        */
/*                      DevHelp_UnLock                                */
/*                      DevHelp_SemClear                              */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/
void SendIRQURB(PRP_GENIOCTL pRP)
{
  USBRB          rb;   // USB I/O Request Block
  RP_GENIOCTL    rp;   // IOCTL Request Packet to USBD
  PLIBUSB_IRQ_START pParm;
  UCHAR FAR     *pData;
  SHORT DevIndex;
  ULONG ulLockHandle;

/*
  if ( VerifyParam (pRP, sizeof(LIBUSB_IRQ_START)) )
  {
    #ifdef DEBUG
      dsPrint (DBG_DETAILED, "USBRESMGR: SendBulkURB wrong Param pointer\r\n");
    #endif
    pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
  }
  else
  {

    DevIndex = DevIndexFromHandle(pParm->ulDevHandle);
    if(DevIndex>=0)
    {
      USHORT rc;
      pParm = (PLIBUSB_IRQ_REQ) pRP->ParmPacket;
      pData = (UCHAR FAR*) pRP->DataPacket;
      if(pParm->usDataLen)
      {
        if(pParm->ucEndpoint & DEV_ENDPT_DIRIN)
        {
          rc = DevHelp_Lock( SELECTOROF(pRP->DataPacket),
                             LOCKTYPE_SHORT_ANYMEM,
                             0,
                             &ulLockHandle);
        }
        else
        {
          rc = DevHelp_Lock( SELECTOROF(pRP->ParmPacket),
                             LOCKTYPE_SHORT_ANYMEM,
                             0,
                             &ulLockHandle);
        }
        if(0!=rc)
        {
          #ifdef DEBUG
            dsPrint (DBG_DETAILED, "USBRESMGR: SendBulkURB Lock Memory failed\r\n");
          #endif
          pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
        }
        else
        {
          if(pParm->Endpoint & DEV_ENDPT_DIRIN)
            if(0!=VerifyData(pRP, pParm->usDataLen) )
            {
              #ifdef DEBUG
                dsPrint (DBG_DETAILED, "USBRESMGR: SendBulkURB Lock Data failed\r\n");
              #endif
              pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
            }
            else
            {
              DevHelp_UnLock(ulLockHandle);
            }
        }

        if(!(pRP->rph.Status & STERR))
        {
          setmem ((PSZ)&rb, 0, sizeof(rb));
          rb.controllerId  = gUSBDevices[DevIndex].pDeviceInfo->ctrlID;
          rb.deviceAddress = gUSBDevices[DevIndex].pDeviceInfo->deviceAddress;
          rb.endPointId    = pParm->Endpoint;
          if(pParm->Endpoint & DEV_ENDPT_DIRIN)
          {
            rb.flags         = USRB_FLAGS_TTYPE_OUT | USRB_FLAGS_DET_INTRPT;
            rb.buffer1       = 0;
            rb.buffer1Length = 0;
            rb.buffer2       = pData;
            rb.buffer2Length = pParm->usDataLen;
          }
          else
          {
            rb.flags         = USRB_FLAGS_TTYPE_OUT | USRB_FLAGS_DET_INTRPT;
            rb.buffer1       = (PUCHAR)&pParm->Data[0];
            rb.buffer1Length = pParm->usDataLen;
            rb.buffer2       = 0;
            rb.buffer2Length = 0;
          }
          //rb.serviceTime   = USB_DEFAULT_SRV_INTV;
          rb.serviceTime   = pParm->ulTimeout;
          rb.maxPacketSize = USB_DEFAULT_PKT_SIZE;
          rb.maxErrorCount = USB_MAX_ERROR_COUNT;
          rb.usbIDC        = (PUSBIDCEntry)IDCEntry;
          rb.usbDS         = GetDS();
          rb.category      = USB_IDC_CATEGORY_CLASS;
          rb.requestData1  = RES_IRQ_RWIRQ;
          rb.requestData2  = DevIndex;
          rb.requestData3  = pParm;

          setmem((PSZ)&rp, 0, sizeof(rp));
          rp.rph.Cmd    = CMDGenIOCTL;
          rp.Category   = USB_IDC_CATEGORY_USBD;
          rp.Function   = USB_IDC_FUNCTION_ACCIO;
          rp.ParmPacket = (PVOID)&rb;

          #ifdef DEBUG
            dsPrint (DBG_DETAILED, "USBRESMGR: Send IDC Request\r\n");
          #endif
          USBCallIDC (gpUSBDIDC, gdsUSBDIDC, (PRP_GENIOCTL)&rp);
          rc = DevHelp_ProcBlock( pParm,
                                  6000,//-1
                                  WAIT_IS_INTERRUPTABLE);
          #ifdef DEBUG
            dsPrint2( DBG_DETAILED,
                      "USBRESMGR: ProcBlock returned %d %x\r\n",
                      rc,
                      rc);
          #endif
          if(rc==WAIT_TIMED_OUT)
            pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;

          DevHelp_UnLock(ulLockHandle);
        }
      }
    }
    else
    {
      #ifdef DEBUG
        dsPrint (DBG_DETAILED, "USBRESMGR: SendBulkURB Device Not found\r\n");
      #endif
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
    }
  }
*/
}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: RegisterDevSemaphore                              */
/*                                                                    */
/* DESCRIPTIVE NAME: Register a Device Notification Semaphore         */
/*                                                                    */
/* FUNCTION: Register Semaphores for notifivation when a certain      */
/*           Device gets attached/removed                             */
/*                                                                    */
/* NOTES: Special IOCtl Category 0xA0, Function 0x43.                 */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: RegisterDevSemaphore                                  */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         VerifyParam                                   */
/*                      VerifyData                                    */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/
void  RegisterDevSemaphore (PRP_GENIOCTL pRP)
{
  USHORT   semIndex;
  PDEVEVENTSET pSems;
  int i;

  if ( VerifyParam (pRP, 0) ||
       VerifyData  (pRP, sizeof(DEVEVENTSET)) )
  {
    #ifdef DEBUG
      dsPrint (DBG_DETAILED, "USBRESMGR: RegisterSemaphore wrong pointer\r\n");
    #endif
               pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
  }
  else
  {
    pSems = (PDEVEVENTSET)pRP->DataPacket;

    if( pSems->ulSize < sizeof(DEVEVENTSET) ||
        (pSems->ulCaps & DEV_SEM_MASK) !=DEV_SEM_MASK ||
        (pSems->ulCaps & (DEV_SEM_VENDORID|DEV_SEM_PRODUCTID) != (DEV_SEM_PRODUCTID|DEV_SEM_VENDORID)) ||
        (pSems->ulSemDeviceAdd == 0) ||
        (pSems->ulSemDeviceRemove == 0) ||
        (pSems->usVendorID == 0) || (pSems->usVendorID == 0xFFFF) ||
        (pSems->usProductID == 0) || (pSems->usProductID == 0xFFFF))
    {
      #ifdef DEBUG
        dsPrint4 (DBG_DETAILED,
                  "USBRESMGR: wrong Data :\r\n"
                  " Size      : %d\r\n"
                  " Flags     : %x\r\n"
                  " SemA      : %d\r\n"
                  " SemB      : %d\r\n",
                  pSems->ulSize,
                  pSems->ulCaps,
                  pSems->ulSemDeviceAdd,
                  pSems->ulSemDeviceRemove );
        dsPrint2 (DBG_DETAILED,
                  " VendorID  : %d\r\n"
                  " DeviceID  : %d\r\n",
                  pSems->usVendorID,
                  pSems->usProductID );
      #endif
      pRP->rph.Status |= STERR | ERROR_INVALID_HANDLE;
    }
    else
    {
      semIndex = 0;
      #ifdef DEBUG
        dsPrint4 ( DBG_DETAILED,
                   "USBRESMGR: Register Semaphores %lx / %lx for Device (%x/%x)\r\n",
                   pSems->ulSemDeviceAdd,
                   pSems->ulSemDeviceRemove,
                   pSems->usVendorID,
                   pSems->usProductID );
      #endif
      for (semIndex=0; semIndex < MAX_SEMS; semIndex++)
      {
        if (gDeviceSEM[semIndex].hSemaphoreAdd == 0)
        {
          gDeviceSEM[semIndex].hSemaphoreAdd = pSems->ulSemDeviceAdd;
          gDeviceSEM[semIndex].hSemaphoreRemove = pSems->ulSemDeviceRemove;
          gDeviceSEM[semIndex].usVendorID  = pSems->usVendorID;
          gDeviceSEM[semIndex].usProductID  = pSems->usProductID;
          gDeviceSEM[semIndex].usBCDDevice = (pSems->ulCaps & DEV_SEM_BCDDEVICE)!=0?pSems->usBCDDevice:0xFFFF;
          for(i=0;i<MAX_DEVICES;i++)
          {
            if( (gDeviceSEM[semIndex].usVendorID == gUSBDevices[i].pDeviceInfo->descriptor.idVendor) &&
                (gDeviceSEM[semIndex].usProductID == gUSBDevices[i].pDeviceInfo->descriptor.idProduct) &&
                ( (gDeviceSEM[semIndex].usBCDDevice == 0xffff) ||
                 ((gDeviceSEM[semIndex].usBCDDevice != 0xffff) &&
                  (gDeviceSEM[semIndex].usBCDDevice ==  gUSBDevices[i].pDeviceInfo->descriptor.bcdDevice)) ) )
            {
              if ((i = DevHelp_OpenEventSem (gDeviceSEM[semIndex].hSemaphoreAdd)) == 0)
              {
                DevHelp_PostEventSem (gDeviceSEM[semIndex].hSemaphoreAdd);
                DevHelp_CloseEventSem (gDeviceSEM[semIndex].hSemaphoreAdd);
              }
              break;
            }
          }
          break;
        }
      }
      if (semIndex >= MAX_SEMS)
      {
        #ifdef DEBUG
          dsPrint (DBG_DETAILED, "USBRESMGR: RegisterSemaphore To many Add sems\r\n");
        #endif
        pRP->rph.Status |= STERR | ERROR_TOO_MANY_SEMAPHORES;
      }

    }
  }

}

/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: DeregisterDevSemaphore                            */
/*                                                                    */
/* DESCRIPTIVE NAME: Deregistered Device Semaphore                    */
/*                                                                    */
/* FUNCTION: Remove Device Semaphore from list                        */
/*                                                                    */
/* NOTES: Special IOCtl Category 0xA0, Function 0x44.                 */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: DeregisterDevSemaphore                                */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         VerifyParam                                   */
/*                      VerifyData                                    */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/
void  DeregisterDevSemaphore (PRP_GENIOCTL pRP)
{
  USHORT   semIndex;
  PDEVEVENTSET pSems;

  if ( VerifyParam (pRP, 0) ||
       VerifyData  (pRP, sizeof(DEVEVENTSET)) )
  {
    pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
  }
  else
  {
    pSems = (PDEVEVENTSET)pRP->DataPacket;

    if (pSems->ulSize < sizeof(DEVEVENTSET) ||
        (pSems->ulCaps & DEV_SEM_MASK) !=DEV_SEM_MASK ||
        (pSems->ulCaps & (DEV_SEM_VENDORID|DEV_SEM_PRODUCTID) != (DEV_SEM_PRODUCTID|DEV_SEM_VENDORID)) ||
        (pSems->ulSemDeviceAdd == 0) ||
        (pSems->ulSemDeviceRemove == 0) ||
        (pSems->usVendorID == 0) || (pSems->usVendorID == 0xFFFF) ||
        (pSems->usProductID == 0) || (pSems->usProductID == 0xFFFF))
    {
      pRP->rph.Status |= STERR | ERROR_SEM_NOT_FOUND;
    }
    else
    {
      semIndex = 0;
      #ifdef DEBUG
        dsPrint1 ( DBG_DETAILED,
                   "USBRESMGR: Deregister Semaphores %lx / %lx for Device (%x/%x)\r\n",
                   pSems->ulSemDeviceAdd,
                   pSems->ulSemDeviceRemove,
                   pSems->usVendorID,
                   pSems->usProductID );
      #endif
      for (; semIndex < MAX_SEMS; semIndex++)
      {
        if ( (gDeviceSEM[semIndex].hSemaphoreAdd == pSems->ulSemDeviceAdd) &&
             (gDeviceSEM[semIndex].hSemaphoreRemove == pSems->ulSemDeviceRemove) &&
             (gDeviceSEM[semIndex].usVendorID == pSems->usVendorID) &&
             (gDeviceSEM[semIndex].usProductID == pSems->usProductID))
        {
          if( !(((pSems->ulCaps & DEV_SEM_BCDDEVICE) != 0 ) &&
                (gDeviceSEM[semIndex].usBCDDevice!=pSems->usBCDDevice)) )
          {
            gDeviceSEM[semIndex].hSemaphoreAdd    = 0;
            gDeviceSEM[semIndex].hSemaphoreRemove = 0;
            gDeviceSEM[semIndex].usVendorID       = 0;
            gDeviceSEM[semIndex].usProductID      = 0;
            gDeviceSEM[semIndex].usBCDDevice      = 0;
            break;
          }
        }
      }
      if (semIndex >= MAX_SEMS)
      {
        pRP->rph.Status |= STERR | ERROR_SEM_NOT_FOUND;
      }

    }
  }
}



/********************** START OF SPECIFICATIONS ***********************/
/*                                                                    */
/* SUBROUTINE NAME: BufferVirtAddr                                    */
/*                                                                    */
/* DESCRIPTIVE NAME: get Buffer Virtual Address                       */
/*                                                                    */
/* FUNCTION: This function is used to allocate a block of fixed       */
/*           memory and convert the 32-bit physical address to a      */
/*           valid selector:offset pair.                              */
/*                                                                    */
/* NOTES:                                                             */
/*                                                                    */
/* CONTEXT: Task time                                                 */
/*                                                                    */
/* ENTRY POINT: BufferVirtAddr                                        */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pStringData = pointer to string data buffer                 */
/*                                                                    */
/* EXIT-NORMAL: N/A                                                   */
/*                                                                    */
/* EXIT-ERROR: N/A                                                    */
/*                                                                    */
/* EFFECTS: None                                                      */
/*                                                                    */
/* INTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/* EXTERNAL REFERENCES: None                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/************************ END OF SPECIFICATIONS ***********************/

static PUCHAR BufferVirtAddr (PUCHAR pStringData)
{
   USHORT   modeFlag;
   PUCHAR   pBuffer;

   if (DevHelp_AllocPhys (((PSDATA)pStringData)->stringLength, MEMTYPE_ABOVE_1M,
                          &((PSDATA)pStringData)->stringAddr))
   {  // High memory not allocated - attempt to allocate low memory
      if (DevHelp_AllocPhys (((PSDATA)pStringData)->stringLength, MEMTYPE_BELOW_1M,
                             &((PSDATA)pStringData)->stringAddr))
      {  // Memory not allocated
         return NULL;
      }
   }
   if (DevHelp_PhysToVirt (((PSDATA)pStringData)->stringAddr, ((PSDATA)pStringData)->stringLength,
                           &pBuffer, &modeFlag))
   {
      DevHelp_FreePhys (((PSDATA)pStringData)->stringAddr);
      return NULL;
   }
   return pBuffer;
}


/******************* START OF SPECIFICATIONS **************************/
/*                                                                    */
/* SUBROUTINE NAME: SendControlURB                                    */
/*                                                                    */
/* DESCRIPTIVE NAME: Send Control URB                                 */
/*                                                                    */
/* FUNCTION: Send a URB to the control endpoint                       */
/*                                                                    */
/* NOTES: Special IOCtl Category 0xA0, Function 0x35.                 */
/*                                                                    */
/* CONTEXT: Task Time                                                 */
/*                                                                    */
/* ENTRY POINT: SendControlURB                                        */
/*     LINKAGE: CALL NEAR                                             */
/*                                                                    */
/* INPUT: pRP = pointer to the IOCtl Request Packet                   */
/*                                                                    */
/* EXIT-NORMAL: none                                                  */
/*                                                                    */
/* EXIT-ERROR: none                                                   */
/*                                                                    */
/* EFFECTS: none                                                      */
/*                                                                    */
/* INTERNAL REFERENCES:                                               */
/*    ROUTINES:         VerifyParam                                   */
/*                      VerifyData                                    */
/*                      DevHelp_SemRequest                            */
/*                      Devhelp_Lock                                  */
/*                      DevIndexFromHandle                            */
/*                      USBCallIDC                                    */
/*                      DevHelp_ProcBlock                             */
/*                      DevHelp_UnLock                                */
/*                      DevHelp_SemClear                              */
/*                                                                    */
/* EXTERNAL REFERENCES: none                                          */
/*    ROUTINES:                                                       */
/*                                                                    */
/******************* END  OF  SPECIFICATIONS **************************/
void StartISOBuffering(PRP_GENIOCTL pRP)
{
  USBRB             rb;   // USB I/O Request Block
  RP_GENIOCTL       rp;   // IOCTL Request Packet to USBD
  PLIBUSB_ISO_START pParm;
  PISORINGBUFFER    pData;
  SHORT DevIndex;
  ULONG ulLockHandle;
  if ( VerifyParam (pRP, sizeof(LIBUSB_ISO_START)) )
  {
    #ifdef DEBUG
      dsPrint (DBG_DETAILED, "USBRESMGR: StartISOBUffering wrong Param pointer\r\n");
    #endif
    pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
  }
  else
  {
    DeviceEndpoint FAR *pEP;

    pParm = (PLIBUSB_ISO_START) pRP->ParmPacket;
    pData = (PISORINGBUFFER)    pRP->DataPacket;
    DevIndex = DevIndexFromHandle(pParm->ulDevHandle);
    if(DevIndex>=0)
    {
      pEP = GetEndpointDPtr( gUSBDevices[DevIndex].pDeviceInfo->configurationData,
                             gUSBDevices[DevIndex].pDeviceInfo->descriptor.bNumConfigurations,
                             gUSBDevices[DevIndex].pDeviceInfo->bConfigurationValue,
                             pParm->ucInterface,
                             pParm->ucEndpoint);
    }
    else
      pEP=NULL;

    if( pEP!=NULL &&
        (pEP->bmAttributes & DEV_ENDPT_ATTRMASK) == DEV_ENDPT_ISOHR &&
        pData->usBufSize >= (2*pEP->wMaxPacketSize) )
    {
      USHORT rc;
      rc = DevHelp_Lock( SELECTOROF(pRP->DataPacket),
                         LOCKTYPE_LONG_ANYMEM,
                         0,
                         &ulLockHandle);
      if(0!=rc)
      {
        #ifdef DEBUG
          dsPrint (DBG_DETAILED, "USBRESMGR: StartISOBuffering Lock Data failed\r\n");
        #endif
        pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
      }
      else
      {
        if(0!=VerifyData(pRP, sizeof(ISORINGBUFFER)) )
        {
          DevHelp_UnLock(ulLockHandle);
          #ifdef DEBUG
            dsPrint (DBG_DETAILED, "USBRESMGR: StartISOBuffering Lock Data failed\r\n");
          #endif
          pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
        }
        else
        {
          // Setup Ringbuffer values
          pData->usWindowSize = pEP->wMaxPacketSize;
          pData->usPosRead    = 0;
          pData->usPosWrite   = 0;
          pData->ucReserved[0]  = pEP->bEndpointAddress;
          pData->ucReserved[1]  = pParm->ucInterface;
          // Do Start Reading/writeing to that endpoint

        }
      }

    }
    else
    {
      #ifdef DEBUG
        dsPrint (DBG_DETAILED, "USBRESMGR: StartISOBuffering Device Not found\r\n");
      #endif
      pRP->rph.Status |= STERR | ERROR_GEN_FAILURE;
    }
  }
}



