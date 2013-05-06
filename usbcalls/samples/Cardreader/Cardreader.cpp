#define INCL_DOS
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\..\usbcalls.h"

#include "GemCore.h"
#include "GCUSBTransport.h"

#define VENDOR_GEMPLUS 0x08E6
#define PRODUCT_GPC430 0x0430
#define USBWRITE_PIPE   0x06
#define USBREAD_PIPE    0x85


#define MAX_CTN 256
USBHANDLE hUSBDevice[MAX_CTN];

const UCHAR pcSetModeROSNOTLP[] = {0x01, 0x00, 0x01};
// Slot specific (GemCore can manage 2 card slots)
const UCHAR pcPowerUp[MAX_SLOT_NB]      = {0x12, 0x1A};
const UCHAR pcPowerDown[MAX_SLOT_NB]    = {0x11, 0x19};
const UCHAR pcISOOutput[MAX_SLOT_NB]    = {0x13, 0x1B};
const UCHAR pcISOInput[MAX_SLOT_NB]     = {0x14, 0x1C};
const UCHAR pcEchAPDU[MAX_SLOT_NB]      = {0x15, 0x1D};
const UCHAR pcCardStatus[MAX_SLOT_NB]   = {0x17, 0x1F};

#define OK               0               /* Success */
#define ERR_INVALID     -1               /* Invalid Data */
#define ERR_CT          -8               /* CT Error */
#define ERR_TRANS       -10              /* Transmission Error */
#define ERR_MEMORY      -11              /* Memory Allocate Error */
#define ERR_HTSI        -128             /* HTSI Error */

#define STATUS_SUCCESS 0
#define STATUS_DEVICE_PROTOCOL_ERROR -128

char GCSendCommand(USHORT Ctn, ULONG nLengthIn,
                   const PUCHAR pcBufferCmd,
                   PULONG pnLengthOut,
                   PUCHAR pcBufferOut);

char GCSendCommand(USHORT Ctn, ULONG nLengthIn,
                   const PUCHAR pcBufferCmd,
                   PULONG pnLengthOut,
                   PUCHAR pcBufferOut)
{
    UCHAR pctr_to_card_buffer[GC_TR_BUF_SIZE];
    UCHAR creturn_value;
    USHORT nlength;

    creturn_value = STATUS_SUCCESS;

    if (GC_TR_BUF_SIZE <= nLengthIn)
    {
        // Buffer is too small (should not happen)
        creturn_value = STATUS_DEVICE_PROTOCOL_ERROR;
        goto finally;
    }

    memcpy(pctr_to_card_buffer+1, pcBufferCmd, nLengthIn);
    pctr_to_card_buffer[TR_OFFSET_LNG] = nLengthIn;
    if(UsbBulkWrite( hUSBDevice[Ctn],
                     USBWRITE_PIPE, 0,
                     nLengthIn+1, pctr_to_card_buffer,
                     100000) != 0)
    {
        creturn_value = STATUS_DEVICE_PROTOCOL_ERROR;
        goto finally;
    }

    nlength = 256; //sizeof(pctr_to_card_buffer);
    if(UsbBulkRead( hUSBDevice[Ctn],
                    USBREAD_PIPE, 0,
                    &nlength, pctr_to_card_buffer,
                    10000) != 0)
    {
        creturn_value = STATUS_DEVICE_PROTOCOL_ERROR;
        goto finally;
    }
    if ( nlength < 1 )
    {
        // length byte not received
        creturn_value = STATUS_DEVICE_PROTOCOL_ERROR;
        goto finally;
    }
    nlength--;
    *pnLengthOut = (*pnLengthOut<nlength) ? *pnLengthOut : nlength;
    memcpy(pcBufferOut, pctr_to_card_buffer+1, *pnLengthOut);
finally:
    if ( creturn_value != STATUS_SUCCESS )
        *pnLengthOut = 0;
    // Clear buffer
    memset(pctr_to_card_buffer, sizeof(pctr_to_card_buffer),0);
    return creturn_value;
}


char CT_init ( USHORT Ctn,  USHORT pn )
{
  APIRET rc;
  int IretVal;         /* Return for this function */
  ULONG nlength;
  UCHAR pcbuffer[RESP_BUF_SIZE];

  IretVal = ERR_MEMORY;        /* Could not allocate port */
  if(Ctn< MAX_CTN && hUSBDevice[Ctn] == 0)
  {
    rc = UsbOpen( &hUSBDevice[Ctn],
                  VENDOR_GEMPLUS,
                  PRODUCT_GPC430,
                  USB_ANY_PRODUCTVERSION,
                  pn);
    if(0==rc)
      rc = UsbSetDeviceConfiguration(hUSBDevice[Ctn], 1);
    if(0==rc)
      IretVal = OK;

    nlength = sizeof(pcbuffer);
    if ( GCSendCommand( Ctn, sizeof(pcSetModeROSNOTLP),
                       (const PUCHAR)pcSetModeROSNOTLP,
                       &nlength, pcbuffer) != STATUS_SUCCESS )
    {
      printf("IFD_GemPC430 : Setmode failed\n");
      IretVal = ERR_HTSI;
    }
    else
    {
      // Check status returned by reader
      if ( pcbuffer[STATUS_OFFSET] != GCORE_OK )
      {
        // Could not set reader mode
        // For now, we consider this a "fatal" error
        printf("IFD_GemPC430 : Setmode failed\n");
        IretVal = ERR_HTSI;
      }
    }
  }

  return IretVal;
}

/* Closes the port in which the reader resides */

char CT_close( USHORT Ctn )
{
  if(Ctn< MAX_CTN && hUSBDevice[Ctn] != 0)
  {
    return (UsbClose(hUSBDevice[Ctn])==0)?OK:ERR_CT;
  }
  else
    return ERR_CT;
}

/* Sends/Receives Data to/from the Reader */

#define CMD_CLA 0
#define CMD_INS 1
#define CMD_P1  2
#define CMD_P2  3
#define CMD_LC  4
#define CMD_LE  4
#define TLV_TAG 5
#define TLV_LEN 6
#define TLV_DATA 7

BOOL ResetCT()
{
  return TRUE;
}

BOOL ResetICC(USHORT Ctn, UCHAR ucSlot,UCHAR ucFlags, UCHAR ucNumBytes, USHORT *lr, UCHAR *rsp)
{
  ULONG nlength;
  UCHAR pcbuffer[RESP_BUF_SIZE];
  UCHAR pccmd_buffer[CMD_BUF_SIZE];
  ULONG ncommand_length = 1; // there is at least the command byte

   // By default, assume it won't work :)
  ULONG AtrLength = 0;

  // Build reset command
  pccmd_buffer[OFFSET_CMD] = pcPowerUp[ucSlot-1];
// PTS management should be added here
 //send the command
  nlength = sizeof(pcbuffer);
  if ( GCSendCommand( Ctn,
                      ncommand_length, pccmd_buffer,
                      &nlength, pcbuffer) != STATUS_SUCCESS )
  {
    printf("IFD_GemPC430 : PowerUp failed\n");
    rsp[0] = 0x64;
    rsp[1] = 0x00;
    *lr = 2;
    return FALSE;
  }
  // Response analysis
  if ( nlength < 1)
  {
    rsp[0] = 0x64;
    rsp[1] = 0x00;
    *lr = 2;
    return FALSE;
  }
  // Remove command byte count from length
  nlength--;
  switch ( pcbuffer[STATUS_OFFSET] )
  {
    case GCORE_OK:
    case GCORE_WRONG_TCK:
      AtrLength = (nlength < MAX_ATR_SIZE) ? nlength: MAX_ATR_SIZE;
      switch(ucFlags)
      {
        case 0:
          rsp[0] = 0x90;
          rsp[1] = 0x01; // Check Protocoltype of card from ATR.
          *lr = 2;
          break;
        case 1:
          memcpy(rsp, pcbuffer+ATR_OFFSET, AtrLength);
          rsp[AtrLength]   = 0x90;
          rsp[AtrLength+1] = 0x01; // Check Protocoltype of card from ATR.
          *lr = 2 + AtrLength;
          break;
        case 2:
          // ToDO Extract historical bytes...
          rsp[0] = 0x90;
          rsp[1] = 0x01; // Check Protocoltype of card from ATR.
          *lr = 2;
          break;
      }
      break;
    default:
      // There is a problem in getting the reset
      rsp[0] = 0x64;
      rsp[1] = 0x00;
      *lr = 2;
      return FALSE;
  }
  return TRUE;
}

BOOL GetSlotState(USHORT Ctn, UCHAR ucSlot, UCHAR *ucPresence)
{
  UCHAR pcbuffer[RESP_BUF_SIZE];
  ULONG nlength = sizeof(pcbuffer);
  // Command is 1-byte long
  if ( GCSendCommand( Ctn,
                      1, (const PUCHAR)&pcCardStatus[ucSlot-1],
                      &nlength, pcbuffer) != STATUS_SUCCESS )
  {
    printf("IFD_GemPC430 : CARD STATUS failed\n");
    return FALSE;
  }

  if ( (nlength < (OFFSET_STAT_BYTE+1)) ||
          pcbuffer[STATUS_OFFSET] != GCORE_OK )
  {
    // Length should at least be 2 (nlength + reader status)
    return FALSE;
  }
  *ucPresence =pcbuffer[OFFSET_STAT_BYTE];
  return TRUE;
}

BOOL PowerDownCard(USHORT Ctn,UCHAR ucSlot, USHORT *lr, UCHAR *rsp)
{
  UCHAR pcbuffer[RESP_BUF_SIZE];
  ULONG nlength;
  if ( GCSendCommand( Ctn,
                      sizeof(pcPowerDown[ucSlot-1]),(const PUCHAR) &pcPowerDown[ucSlot-1],
                      &nlength, pcbuffer) != STATUS_SUCCESS )
  {
    printf("IFD GemPC430 : PowerDown failed\n");
    return FALSE;
  }
  return TRUE;
}

// status functions
BOOL GetCTManufacturerDO(USHORT Ctn, USHORT *lr, UCHAR *rsp)
{
  rsp[0] = 0x46;
  rsp[1] = 0x0F;
  memcpy(&rsp[2], " MMDE",5);
  memcpy(&rsp[7], "  USB",5);
  memcpy(&rsp[12]," 0.01",5);
  rsp[17] = 0x90;
  rsp[18] = 0x00;
  *lr = 19;
  return TRUE;
}

BOOL GetICCStatusDO(USHORT Ctn, UCHAR ucSlot, USHORT *lr, UCHAR *rsp)
{
  *lr = 0;
  if(ucSlot)
  {
    if(GetSlotState(Ctn, ucSlot, &rsp[2]) )
    {
      rsp[0] = 0x80;
      rsp[1] = 0x01;
      rsp[3] = 0x90;
      rsp[4] = 0x00;
      *lr=5;
    }
  }
  else
  {
    if(GetSlotState(Ctn, 1, &rsp[2]) &&
       GetSlotState(Ctn, 2, &rsp[3]) )
    {
      rsp[0] = 0x80;
      rsp[1] = 0x02;
      rsp[4] = 0x90;
      rsp[5] = 0x00;
      *lr=6;
    }
  }
  return (*lr>0);
}

BOOL GetFunctionalUnitDO(USHORT Ctn, USHORT *lr, UCHAR *rsp)
{
  // only ICC1 present
  rsp[0] = 0x81;
  rsp[1] = 0x01;
  rsp[2] = 0x01;
  rsp[3] = 0x90;
  rsp[4] = 0x00;
  *lr = 5;
  return TRUE;
}


char  CT_data( USHORT Ctn, UCHAR *dad, UCHAR *sad,
               USHORT lenc, UCHAR *cmd,
               USHORT *lr, UCHAR *rsp )
{

  /* Reader specific CT-BCS commands */

  int IretVal;                                /* Return Value    */
  UCHAR Temp;

  switch( *dad )
  {
    case 0x01:
    {             /* This command goes to the reader */

      // Don't get confused here this is for the return saying
      // the source was the reader and the destination the host

      *dad = *sad;
      *sad = 1;

      /*******************/
      /* CT-BCS Commands */
      /*******************/

      if(cmd[CMD_CLA] != 0x20)
      {
        rsp[0] = 0x6E;
        rsp[1] = 0x00;
        *lr = 2;
      }
      else
      {
        switch(cmd[CMD_INS])
        {
          case 0x10:
          case 0x11:
            // CHANGE:: Reset code goes here.
            /* Get Status - Gets reader status */
            switch(cmd[CMD_P1])
            {
              case 0: // Reset CT
                 if(ResetCT())
                   rsp[0] = 0x90;
                 else
                   rsp[0] = 0x64;
                 rsp[1] = 0x00;
                 *lr = 2;
                break;
              case 1: // Reset ICC1
              case 2: // Reset ICC2
                if(cmd[CMD_P2]<3)
                  ResetICC(Ctn,cmd[CMD_P1],cmd[CMD_P2],cmd[CMD_LE],lr,rsp);
                else
                {
                  rsp[0] = 0x6A;
                  rsp[1] = 0x00;
                  *lr = 2;
                }
                break;
              default:
                rsp[0] = 0x6A;
                rsp[1] = 0x00;
                *lr = 2;
                break;
            }
            break;
          case 0x12:
            /* Request ICC  - Turns on the reader/card */
            // CHANGE:: Request ICC code goes here.
            /* Resets the Card/Terminal and returns Atr */
            switch(cmd[CMD_P1])
            {
              case 1:
              case 2: // Reset ICC2
                if((cmd[CMD_P2]&&0x03)<3)
                {
                  // Check LC for display messages and Timeout
                  if(cmd[CMD_LC])
                  {
                    int i=0;
                    UCHAR *Msg = NULL;
                    USHORT usTimeout = 0;
                    BOOL fErr=FALSE;
                    while (i<cmd[CMD_LC])
                    {
                      switch(cmd[TLV_TAG+i]) // switch on TAG
                      {
                        case 50:
                          Msg = &cmd[TLV_LEN+i];
                          i +=(cmd[TLV_LEN+i]+2);
                          break;
                        case 80:
                          if(cmd[TLV_LEN+i]==1)
                            usTimeout = cmd[TLV_DATA+i];
                          else
                            if(cmd[TLV_LEN+i]==2)
                              usTimeout = cmd[TLV_DATA+i]*256+cmd[TLV_DATA+1+i];
                          i +=(cmd[TLV_LEN+i]+2);
                          break;
                        default:
                          i=cmd[CMD_LC];
                          rsp[0] = 0x6D;
                          rsp[1] = 0x00;
                          *lr = 2;
                          fErr=TRUE;
                          break;
                      }
                    }
                    if(!fErr)
                    {
                      if(Msg)
                      {
                        // Display Message
                      }
                      if(usTimeout)
                      {
                      }
                    }
                  }
                  else
                    ResetICC(Ctn,cmd[CMD_P1],cmd[CMD_P2],cmd[CMD_LE],lr,rsp);
                }
                else
                {
                  rsp[0] = 0x6A;
                  rsp[1] = 0x00;
                  *lr = 2;
                }
                break;
              default:
                rsp[0] = 0x6A;
                rsp[1] = 0x00;
                *lr = 2;
                break;
            }
            break;
          case 0x13:
             // CHANGE:: Get Status code goes here.
            /* Eject ICC - Deactivates Reader  */
            switch(cmd[CMD_P1])
            {
              case 0:
                switch(cmd[CMD_P2])
                {
                  case 0x46:
                    GetCTManufacturerDO(Ctn,lr,rsp);
                    break;
                  case 0x80:
                    GetICCStatusDO(Ctn,cmd[CMD_P1],lr,rsp);
                    break;
                  case 0x81:
                    GetFunctionalUnitDO(Ctn,lr,rsp);
                    break;
                  default:
                    rsp[0] = 0x64;
                    rsp[1] = 0x00;
                    *lr = 2;
                    break;
                }
              case 1:
              case 2:
                GetICCStatusDO(Ctn,cmd[CMD_P1],lr,rsp);
              default:
                rsp[0] = 0x64;
                rsp[1] = 0x00;
                *lr = 2;
                break;
            }
            break;
          case 0x14:
            if( ((cmd[CMD_P1]!=1) && (cmd[CMD_P1]!=2)) || (cmd[CMD_P2]!= 0) )
            {
              rsp[0] = 0x64;
              rsp[1] = 0x00;
              *lr = 2;
              break;
            }
            if( (cmd[CMD_LC]!= 0) || (lenc!=5) )
            {

            }
            else
              PowerDownCard(Ctn,cmd[2],lr,rsp);
            break;
          case 0x15:
            // CHANGE:: Eject ICC code goes here.
            switch(cmd[CMD_P1])
            {
              case 1:
              case 2:
                if( ((cmd[CMD_P2]&0xF0) != 0x00) &&
                    ((cmd[CMD_P2]&0xF0) != 0xF0) )
                {
                  rsp[0] = 0x64;
                  rsp[1] = 0x00;
                  *lr = 2;
                  break;
                }
                // Check LC for display messages and Timeout
                if(cmd[CMD_LC])
                {
                  int i=0;
                  UCHAR *Msg = NULL;
                  USHORT usTimeout = 0;
                  BOOL fErr=FALSE;
                  while (i<cmd[CMD_LC])
                  {
                    switch(cmd[TLV_TAG+i]) // switch on TAG
                    {
                      case 50:
                        Msg = &cmd[TLV_LEN+i];
                        i +=(cmd[TLV_LEN+i]+2);
                        break;
                      case 80:
                        if(cmd[TLV_LEN+i]==1)
                          usTimeout = cmd[TLV_DATA+i];
                        else
                          if(cmd[TLV_LEN+i]==2)
                            usTimeout = cmd[TLV_DATA+i]*256+cmd[TLV_DATA+1+i];
                        i +=(cmd[TLV_LEN+i]+2);
                        break;
                      default:
                        i=cmd[CMD_LC];
                        rsp[0] = 0x6D;
                        rsp[1] = 0x00;
                        *lr = 2;
                        fErr=TRUE;
                        break;
                    }
                  }
                  if(!fErr)
                  {
                    if(Msg)
                    {
                      // Display Message
                    }
                    if(PowerDownCard(Ctn,cmd[CMD_P1],lr,rsp) && usTimeout!=0)
                    {
                      // Check for card removal for the duration of timeout.
                    }
                  }
                }
                else
                  PowerDownCard(Ctn,cmd[CMD_P1],lr,rsp);
                break;
              default:
                break;
            }
            break;
        }// end switch(cmd[CMD_INS])
      }
      break;
    }
    case 0x00:
    case 0x02:
    {
      /* This command goes to the ICC1 or ICC2*/

      // Don't get confused here this is for the return saying
      // the source was the card and the destination the host
       Temp = *dad;
       *dad = *sad;
       *sad = Temp;


       // CHANGE:: Lots of code here.  This sends commands to the
       // smartcard directly and places responses into rsp with size
       // of lr.
       break;
    }
    default:
      IretVal = ERR_INVALID;              /* Invalid SAD/DAD Address */
  }

  if (IretVal != OK)
  {
   *lr = 0;

  }

  return IretVal;
}

void main()
{
  int rc,i;
  ULONG ulNumDevices;
  USBHANDLE hUSBDevice;
  UCHAR ucData[20];
  char brc;
  rc = UsbQueryNumberDevices(&ulNumDevices);
  printf("Num devices = %d (rc=%d)\r\n",ulNumDevices,rc);

  printf("Try to open Card Reader ... ");
  brc = CT_init(0,0);

  if(!brc)
  {
    UCHAR ucReset[] ={0x20,0x11,0x01,0x01,0x00};
    UCHAR ucResponse[64];
    USHORT usRetLen = sizeof(ucResponse);
    UCHAR sad = 2;
    UCHAR dad = 1;
    printf("SUCCESS\r\n try to reset card");
    brc = CT_data(0, &dad, &sad, 5, ucReset, &usRetLen, ucResponse);
    if(brc)
    {
      printf("FAILED!\r\n");
    }
    else
    {
      printf("ATR : ");
      for(USHORT i =0;i< usRetLen;i++)
        printf("%02X ",ucResponse[i]);
      printf("\r\n");
    }
    CT_close(0);
  }
  else
    printf("FAILED!\r\n");
}
