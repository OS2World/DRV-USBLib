/*****************************************************************
/
/ File   :   ifdhandler.c
/ Authors :   David Corcoran <corcoran@linuxnet.com>
/             Jean-Luc Giraud <jl.giraud@free.fr>
/ Date   :   April 9, 2001
/ Purpose:   This provides reader specific low-level calls
/            for the GemPC430 (USB) of Gemplus. The function
/            stubs were written by D. Corcoran, the 430
/            specific code was added by JL Giraud.
/            This module implements the command level of GemCore
/            See http://www.linuxnet.com for more information.
/ License:   See file LICENSE & COPYING
/
******************************************************************/

#include <pcscdefines.h>
#include <ifdhandler.h>
#include <stdio.h>
#include <string.h>
#include "usbserial.h"
#include "GemCore.h"

// Use this define to log the calls to the various functions
//#define GEMPC430_DEBUG 1

#define log_function(x) fprintf(stderr,(x))

// Command descriptions
// Reader mode without TLP support
const UCHAR pcSetModeROSNOTLP[] = {0x01, 0x00, 0x01};
// Slot specific (GemCore can manage 2 card slots)
const UCHAR pcPowerUp[MAX_SLOT_NB]      = {0x12, 0x1A};
const UCHAR pcPowerDown[MAX_SLOT_NB]    = {0x11, 0x19};
const UCHAR pcISOOutput[MAX_SLOT_NB]    = {0x13, 0x1B};
const UCHAR pcISOInput[MAX_SLOT_NB]     = {0x14, 0x1C};
const UCHAR pcEchAPDU[MAX_SLOT_NB]      = {0x15, 0x1D};
const UCHAR pcCardStatus[MAX_SLOT_NB]   = {0x17, 0x1F};

// Array of structures to hold the ATR of each slot
static GCoreDesc pgSlots[MAX_SLOT_NB];

RESPONSECODE gempc430_status_processing(DWORD nlength, PDWORD RxLength,
                                        PUCHAR pcbuffer, PUCHAR RxBuffer);



RESPONSECODE IFDHCreateChannel ( DWORD Lun, DWORD Channel ) {

  /* Lun - Logical Unit Number, use this for multiple card slots
     or multiple readers. 0xXXXXYYYY -  XXXX multiple readers,
     YYYY multiple slots. The resource manager will set these
     automatically.  By default the resource manager loads a new
     instance of the driver so if your reader does not have more than
     one smartcard slot then ignore the Lun in all the functions.
     Future versions of PC/SC might support loading multiple readers
     through one instance of the driver in which XXXX would be important
     to implement if you want this.
  */

  /* Channel - Channel ID.  This is denoted by the following:
     0x000001 - /dev/pcsc/1
     0x000002 - /dev/pcsc/2
     0x000003 - /dev/pcsc/3

     USB readers may choose to ignore this parameter and query
     the bus for the particular reader.
  */

  /* This function is required to open a communications channel to the
     port listed by Channel.  For example, the first serial reader on COM1 would
     link to /dev/pcsc/1 which would be a sym link to /dev/ttyS0 on some machines
     This is used to help with intermachine independance.

     Once the channel is opened the reader must be in a state in which it is possible
     to query IFDHICCPresence() for card status.

     returns:

     IFD_SUCCESS
     IFD_COMMUNICATION_ERROR
  */
    DWORD nlength;
    UCHAR pcbuffer[RESP_BUF_SIZE];

 #ifdef GEMPC430_DEBUG
 printf("IFD_GemPC430 : entering IFDHCreateChannel\n");
 #endif


    // Reset ATR buffer
    pgSlots[GCoreLunToSlotNb(Lun)].nATRLength = 0;


    if (OpenUSB(Lun) != STATUS_SUCCESS)
    {
        log_function("IFD_GemPC430 : OpenUSB failed\n");
        return IFD_COMMUNICATION_ERROR;
    }

    // Set the mode to ROS but no TLP (ATR should then be fine)
    nlength = sizeof(pcbuffer);
    if ( GCSendCommand(Lun, sizeof(pcSetModeROSNOTLP),
                       pcSetModeROSNOTLP,
                       &nlength, pcbuffer) != STATUS_SUCCESS )
    {
        log_function("IFD_GemPC430 : Setmode failed\n");
        goto error_exit;
    }
    // Check status returned by reader
    if ( pcbuffer[STATUS_OFFSET] != GCORE_OK )
    {
        // Could not set reader mode
        // For now, we consider this a "fatal" error
        log_function("IFD_GemPC430 : Setmode failed\n");
        goto error_exit;
    }
    return IFD_SUCCESS;

error_exit:
    CloseUSB(Lun);
    return IFD_COMMUNICATION_ERROR;

}

RESPONSECODE IFDHCloseChannel ( DWORD Lun ) {

  /* This function should close the reader communication channel
     for the particular reader.  Prior to closing the communication channel
     the reader should make sure the card is powered down and the terminal
     is also powered down.

     returns:

     IFD_SUCCESS
     IFD_COMMUNICATION_ERROR
  */
    UCHAR pcbuffer[RESP_BUF_SIZE];
    DWORD nlength;

 #ifdef GEMPC430_DEBUG
 printf("IFD_GemPC430 : entering IFDHCloseChannel\n");
 #endif

    // PowerDown the card
    nlength = sizeof(pcbuffer);
    if ( GCSendCommand(Lun, sizeof(pcPowerDown[GCoreLunToSlotNb(Lun)]),
                     &pcPowerDown[GCoreLunToSlotNb(Lun)],
                     &nlength, pcbuffer) != STATUS_SUCCESS )
    {
        log_function("IFD GemPC430 : PowerDown failed\n");
        CloseUSB(Lun);
        return IFD_COMMUNICATION_ERROR;
    }

    // No reader status check, if it failed, what can you do ? :)

    CloseUSB(Lun);
    return IFD_SUCCESS;
}

RESPONSECODE IFDHGetCapabilities ( DWORD Lun, DWORD Tag,
           PDWORD Length, PUCHAR Value ) {

  /* This function should get the slot/card capabilities for a particular
     slot/card specified by Lun.  Again, if you have only 1 card slot and don't mind
     loading a new driver for each reader then ignore Lun.

     Tag - the tag for the information requested
         example: TAG_IFD_ATR - return the Atr and it's size (required).
         these tags are defined in ifdhandler.h

     Length - the length of the returned data
     Value  - the value of the data

     returns:

     IFD_SUCCESS
     IFD_ERROR_TAG
  */

 #ifdef GEMPC430_DEBUG
 printf("IFD_GemPC430 : entering IFDHGetCapabilities\n");
 #endif


    switch ( Tag )
    {
        case TAG_IFD_ATR:
        {
            // If Length is not zero, powerICC has been performed.
            // Otherwise, return NULL pointer
            // Buffer size is stored in *Length
            *Length = ( *Length < pgSlots[GCoreLunToSlotNb(Lun)].nATRLength) ?
                        *Length : pgSlots[GCoreLunToSlotNb(Lun)].nATRLength;
            if ( *Length )
            {
                memcpy(Value,  pgSlots[GCoreLunToSlotNb(Lun)].pcATRBuffer, *Length);
            }
            break;
        }
        default:
        {
            return IFD_ERROR_TAG;
        }
    }
    return IFD_SUCCESS;
}

RESPONSECODE IFDHSetCapabilities ( DWORD Lun, DWORD Tag,
             DWORD Length, PUCHAR Value ) {

  /* This function should set the slot/card capabilities for a particular
     slot/card specified by Lun.  Again, if you have only 1 card slot and don't mind
     loading a new driver for each reader then ignore Lun.

     Tag - the tag for the information needing set

     Length - the length of the returned data
     Value  - the value of the data

     returns:

     IFD_SUCCESS
     IFD_ERROR_TAG
     IFD_ERROR_SET_FAILURE
     IFD_ERROR_VALUE_READ_ONLY
  */
    // By default, say it worked
#ifdef GEMPC430_DEBUG
 printf("IFD_GemPC430 : entering IFDHSetCapabilities\n");
#endif
    return IFD_SUCCESS;
}

RESPONSECODE IFDHSetProtocolParameters ( DWORD Lun, DWORD Protocol,
           UCHAR Flags, UCHAR PTS1,
           UCHAR PTS2, UCHAR PTS3) {

  /* This function should set the PTS of a particular card/slot using
     the three PTS parameters sent

     Protocol  - 0 .... 14  T=0 .... T=14
     Flags     - Logical OR of possible values:
     IFD_NEGOTIATE_PTS1 IFD_NEGOTIATE_PTS2 IFD_NEGOTIATE_PTS3
     to determine which PTS values to negotiate.
     PTS1,PTS2,PTS3 - PTS Values.

     returns:

     IFD_SUCCESS
     IFD_ERROR_PTS_FAILURE
     IFD_COMMUNICATION_ERROR
     IFD_PROTOCOL_NOT_SUPPORTED
  */

#ifdef GEMPC430_DEBUG
 printf("IFD_GemPC430 : entering IFDHSetProtocolParameters\n");
#endif
    return IFD_SUCCESS;
}


RESPONSECODE IFDHPowerICC ( DWORD Lun, DWORD Action,
          PUCHAR Atr, PDWORD AtrLength ) {

  /* This function controls the power and reset signals of the smartcard reader
     at the particular reader/slot specified by Lun.

     Action - Action to be taken on the card.

     IFD_POWER_UP - Power and reset the card if not done so
     (store the ATR and return it and it's length).

     IFD_POWER_DOWN - Power down the card if not done already
     (Atr/AtrLength should
     be zero'd)

    IFD_RESET - Perform a quick reset on the card.  If the card is not powered
     power up the card.  (Store and return the Atr/Length)

     Atr - Answer to Reset of the card.  The driver is responsible for caching
     this value in case IFDHGetCapabilities is called requesting the ATR and it's
     length.  This should not exceed MAX_ATR_SIZE.

     AtrLength - Length of the Atr.  This should not exceed MAX_ATR_SIZE.

     Notes:

     Memory cards without an ATR should return IFD_SUCCESS on reset
     but the Atr should be zero'd and the length should be zero

     Reset errors should return zero for the AtrLength and return
     IFD_ERROR_POWER_ACTION.

     returns:

     IFD_SUCCESS
     IFD_ERROR_POWER_ACTION
     IFD_COMMUNICATION_ERROR
     IFD_NOT_SUPPORTED
  */
    DWORD nlength;
    UCHAR pcbuffer[RESP_BUF_SIZE];
    UCHAR pccmd_buffer[CMD_BUF_SIZE];
    DWORD ncommand_length = 1; // there is at least the command byte
 #ifdef GEMPC430_DEBUG
 printf("IFD_GemPC430 : entering IFDHPowerICC\n");
 #endif
     // By default, assume it won't work :)
    *AtrLength = 0;

    switch (Action)
    {
        case IFD_POWER_UP:
        case IFD_RESET:
            // Build reset command
            pccmd_buffer[OFFSET_CMD] = pcPowerUp[GCoreLunToSlotNb(Lun)];
// PTS management should be added here
           //send the command
            nlength = sizeof(pcbuffer);
            if ( GCSendCommand(Lun, ncommand_length, pccmd_buffer,
                        &nlength, pcbuffer) != STATUS_SUCCESS )
            {
                log_function("IFD_GemPC430 : PowerUp failed\n");
                return IFD_COMMUNICATION_ERROR;
            }
            // Response analysis
            if ( nlength < 1)
            {
                pgSlots[GCoreLunToSlotNb(Lun)].nATRLength
                        = *AtrLength = 0;
                return IFD_COMMUNICATION_ERROR;
            }
            // Remove command byte count from length
            nlength--;
            switch ( pcbuffer[STATUS_OFFSET] )
            {
                case GCORE_OK:
                case GCORE_WRONG_TCK:
                    // Reset is returned, even if TCK is wrong
                    pgSlots[GCoreLunToSlotNb(Lun)].nATRLength
                        = *AtrLength = (nlength < MAX_ATR_SIZE)
                                        ? nlength: MAX_ATR_SIZE;
                    memcpy(Atr, pcbuffer+ATR_OFFSET, *AtrLength);
                    memcpy(pgSlots[GCoreLunToSlotNb(Lun)].pcATRBuffer,
                           pcbuffer+ATR_OFFSET,
                           *AtrLength);
                    break;
                default:
                    // There is a problem in getting the reset
                    return IFD_ERROR_POWER_ACTION;
            }
            break;
        case IFD_POWER_DOWN:
            //send the command
            pccmd_buffer[OFFSET_CMD] = pcPowerDown[GCoreLunToSlotNb(Lun)];
            nlength = sizeof(pcbuffer);
            if ( GCSendCommand(Lun, ncommand_length, pccmd_buffer,
                        &nlength, pcbuffer) != STATUS_SUCCESS )
            {
                log_function("IFD_GemPC430 : PowerUp failed\n");
                return IFD_COMMUNICATION_ERROR;
            }
            if ( nlength<1 )
                return IFD_COMMUNICATION_ERROR;
            if ( pcbuffer[STATUS_OFFSET] != GCORE_OK )
            {
                // There is a problem in power down
                return IFD_ERROR_POWER_ACTION;
            }
            break;
        default:
            return IFD_NOT_SUPPORTED;
    }
    return IFD_SUCCESS;
}

RESPONSECODE IFDHTransmitToICC ( DWORD Lun, SCARD_IO_HEADER SendPci,
         PUCHAR TxBuffer, DWORD TxLength,
         PUCHAR RxBuffer, PDWORD RxLength,
         PSCARD_IO_HEADER RecvPci ) {

  /* This function performs an APDU exchange with the card/slot specified by
     Lun.  The driver is responsible for performing any protocol specific exchanges
     such as T=0/1 ... differences.  Calling this function will abstract all protocol
     differences.

     SendPci
     Protocol - 0, 1, .... 14
     Length   - Not used.

     TxBuffer - Transmit APDU example (0x00 0xA4 0x00 0x00 0x02 0x3F 0x00)
     TxLength - Length of this buffer.
     RxBuffer - Receive APDU example (0x61 0x14)
     RxLength - Length of the received APDU.  This function will be passed
     the size of the buffer of RxBuffer and this function is responsible for
     setting this to the length of the received APDU.  This should be ZERO
     on all errors.  The resource manager will take responsibility of zeroing
     out any temporary APDU buffers for security reasons.

     RecvPci
     Protocol - 0, 1, .... 14
     Length   - Not used.

     Notes:
     The driver is responsible for knowing what type of card it has.  If the current
     slot/card contains a memory card then this command should ignore the Protocol
     and use the MCT style commands for support for these style cards and transmit
     them appropriately.  If your reader does not support memory cards or you don't
     want to then ignore this.

     RxLength should be set to zero on error.

     returns:

     IFD_SUCCESS
     IFD_COMMUNICATION_ERROR
     IFD_RESPONSE_TIMEOUT
     IFD_ICC_NOT_PRESENT
     IFD_PROTOCOL_NOT_SUPPORTED
  */
    UCHAR pccmd_buffer[CMD_BUF_SIZE];
    UCHAR pcbuffer[RESP_BUF_SIZE];
    DWORD nlength;
    RESPONSECODE return_value = IFD_SUCCESS; // Assume it will work

#ifdef GEMPC430_DEBUG
 printf("IFD_GemPC430 : entering IFDHTransmitToICC\n");
#endif
    switch ( SendPci.Protocol )
    {
        case T_0:
            // Check if command is going to fit in buffer
            if ( CMD_BUF_SIZE < (1 + TxLength) )
            {
                // Buffer too small, send an error
                return_value =IFD_COMMUNICATION_ERROR;
                goto clean_up_and_return;
            }

            // Check if this is an incoming or outgoing command
            // Size should be APDU + one byte of length
            if ( (TxLength == (APDU_SIZE+1)) && (TxBuffer[APDU_SIZE] !=0)  )
            {
                // Buffer only holds an APDU (no Data) and L != 0,
                // output command
                pccmd_buffer[OFFSET_CMD] = pcISOOutput[GCoreLunToSlotNb(Lun)];
                memcpy(pccmd_buffer+OFFSET_APDU, TxBuffer, TxLength);
            }
            else if ( TxLength >= (APDU_SIZE+1) )
            {
                // ">=" to catch the case CLA INS P1 P2 00
                pccmd_buffer[OFFSET_CMD] = pcISOInput[GCoreLunToSlotNb(Lun)];
                memcpy(pccmd_buffer+OFFSET_APDU, TxBuffer, TxLength);
            }
            else
            {
                // RxBuffer  holds too little data to form an APDU+length
                return_value = IFD_COMMUNICATION_ERROR;
                goto clean_up_and_return;
            }
            // Send the command
            nlength = sizeof(pcbuffer);
            if ( GCSendCommand(Lun, TxLength+1,
                                pccmd_buffer,
                                &nlength, pcbuffer) != STATUS_SUCCESS )
            {
                log_function("IFD_GemPC430 : ISO Output failed\n");
                return_value = IFD_COMMUNICATION_ERROR;
                goto clean_up_and_return;
            }
            return_value = gempc430_status_processing(nlength,
                                                        RxLength,
                                                        pcbuffer,
                                                        RxBuffer);
            break;
        case T_1:
            // Check if command is going to fit in buffer
            // cmd byte + TxLength + Le
            if ( CMD_BUF_SIZE < (1 + TxLength + 1) )
            {
                // Buffer too small, send an error
                return_value =IFD_COMMUNICATION_ERROR;
                goto clean_up_and_return;
            }

            // Build the commmand
            pccmd_buffer[OFFSET_CMD] = pcEchAPDU[GCoreLunToSlotNb(Lun)];
            memcpy(pccmd_buffer+OFFSET_APDU, TxBuffer, TxLength);
            // Add length of expected data
            pccmd_buffer[OFFSET_APDU+TxLength] = *RxLength;

            // Send the command
            nlength = sizeof(pcbuffer);
            if ( GCSendCommand(Lun, TxLength+1,
                                pccmd_buffer,
                                &nlength, pcbuffer) != STATUS_SUCCESS )
            {
                log_function("IFD_GemPC430 : ISO Exchange failed\n");
                return_value = IFD_COMMUNICATION_ERROR;
                goto clean_up_and_return;
            }
            return_value = gempc430_status_processing(nlength,
                                                        RxLength,
                                                        pcbuffer,
                                                        RxBuffer);

            break;
        default:
            return_value = IFD_PROTOCOL_NOT_SUPPORTED;
    }

clean_up_and_return:
    // Buffers clean-up
    bzero(pccmd_buffer, sizeof(pccmd_buffer));
    bzero(pcbuffer, sizeof(pcbuffer));
    if ( return_value != IFD_SUCCESS )
        *RxLength = 0;
    return return_value;

}

RESPONSECODE IFDHControl ( DWORD Lun, PUCHAR TxBuffer,
       DWORD TxLength, PUCHAR RxBuffer,
       PDWORD RxLength ) {

  /* This function performs a data exchange with the reader (not the card)
     specified by Lun.  Here XXXX will only be used.
     It is responsible for abstracting functionality such as PIN pads,
     biometrics, LCD panels, etc.  You should follow the MCT, CTBCS
     specifications for a list of accepted commands to implement.

     TxBuffer - Transmit data
     TxLength - Length of this buffer.
     RxBuffer - Receive data
     RxLength - Length of the received data.  This function will be passed
     the length of the buffer RxBuffer and it must set this to the length
     of the received data.

     Notes:
     RxLength should be zero on error.
  */
#ifdef GEMPC430_DEBUG
 printf("IFD_GemPC430 : entering IFDHControl\n");
#endif

    return IFD_SUCCESS;
}

RESPONSECODE IFDHICCPresence( DWORD Lun ) {

  /* This function returns the status of the card inserted in the
     reader/slot specified by Lun.  It will return either:

     returns:
     IFD_ICC_PRESENT
     IFD_ICC_NOT_PRESENT
     IFD_COMMUNICATION_ERROR
  */

    UCHAR pcbuffer[RESP_BUF_SIZE];
    DWORD nlength;

#ifdef GEMPC430_DEBUG
 printf("IFD_GemPC430 : entering IFDHICCPresence\n");
#endif

    // Send the command
    nlength = sizeof(pcbuffer);
    // Command is 1-byte long
    if ( GCSendCommand(Lun, 1, &pcCardStatus[GCoreLunToSlotNb(Lun)],
                     &nlength, pcbuffer) != STATUS_SUCCESS )
    {
        log_function("IFD_GemPC430 : CARD STATUS failed\n");
        return IFD_COMMUNICATION_ERROR;
    }

    if ( (nlength < (OFFSET_STAT_BYTE+1)) ||
            pcbuffer[STATUS_OFFSET] != GCORE_OK )
    {
        // Length should at least be 2 (nlength + reader status)
        return IFD_COMMUNICATION_ERROR;
    }

    return (MASK_ICC_PRESENCE & pcbuffer[OFFSET_STAT_BYTE])
                ? IFD_ICC_PRESENT : IFD_ICC_NOT_PRESENT;
}

RESPONSECODE gempc430_status_processing(DWORD nlength, PDWORD RxLength,
                                        PUCHAR pcbuffer, PUCHAR RxBuffer)
{
    if (nlength < 1)
    {
        // Length should at least be 1 (reader status)
        return IFD_COMMUNICATION_ERROR;
    }
    nlength -=1;
    switch ( pcbuffer[STATUS_OFFSET] )
    {
        case GCORE_OK:
        case GCORE_NOT_9000:
        case GCORE_CARD_EXC_INT:   //SW=6700
            // if ( nlength < pcbuffer[OFFSET_LNG] )
            // WHAT SHOULD WE DO
            *RxLength = ( *RxLength < nlength)
                          ? (*RxLength) : nlength;
            memcpy(RxBuffer, pcbuffer+OFFSET_DATA,
                    *RxLength);
            break;
        case GCORE_CARD_MUTE:
            return IFD_RESPONSE_TIMEOUT;
        case GCORE_CARD_PROT_ERR:
            return IFD_PROTOCOL_NOT_SUPPORTED;
        default:
            // There was a problem in sending the command
            return IFD_COMMUNICATION_ERROR;
    }
    return IFD_SUCCESS;
}

