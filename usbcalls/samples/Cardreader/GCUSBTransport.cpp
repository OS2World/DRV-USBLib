/*
 *  GCUSBTransport.c.c
 *  ifd-GemPC430
 *
 *  Created by JL Giraud <jl.giraud@free.fr> on Sun Nov 19 2000.
 *
 * Transport level for the GemPC430 of Gemplus.
 *
 *  License:   See file COPYING
 *
 */
 
#include <pcscdefines.h>
#include <ifdhandler.h>
#include <string.h>
#include "usbserial.h"
#include "GemCore.h"
#include "GCUSBTransport.h"


             
RESPONSECODE GCSendCommand(DWORD Lun, DWORD nLengthIn, 
                           const PUCHAR pcBufferCmd,
                           PDWORD pnLengthOut,
                           PUCHAR pcBufferOut)
{
    UCHAR pctr_to_card_buffer[GC_TR_BUF_SIZE];
    UCHAR creturn_value;
    DWORD nlength;
    
    creturn_value = STATUS_SUCCESS;
    
    if (GC_TR_BUF_SIZE <= nLengthIn)
    {
        // Buffer is too small (should not happen)
        creturn_value = STATUS_DEVICE_PROTOCOL_ERROR;
        goto finally;
    }
    
    memcpy(pctr_to_card_buffer+1, pcBufferCmd, nLengthIn);
    pctr_to_card_buffer[TR_OFFSET_LNG] = nLengthIn;
    if (WriteUSB(Lun, nLengthIn+1, pctr_to_card_buffer) != STATUS_SUCCESS)
    {        
        creturn_value = STATUS_DEVICE_PROTOCOL_ERROR;
        goto finally;
    }

    nlength = sizeof(pctr_to_card_buffer);
    if (ReadUSB(Lun, &nlength, pctr_to_card_buffer) != STATUS_SUCCESS)
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
    bzero(pctr_to_card_buffer, sizeof(pctr_to_card_buffer));
    return creturn_value;
}

