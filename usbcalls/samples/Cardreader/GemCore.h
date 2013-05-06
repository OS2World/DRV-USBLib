/*
 *  GemPC430.h
 *  ifd-GemPC430
 *
 *  Created by JL Giraud <jl.giraud@free.fr> on Sun Nov 19 2000.
 *  License:   See file COPYING
 *
 */

#ifndef _GEMCORE_H_
#define _GEMCORE_H_

// Communication buffer size (max=cmd+adpu+Lc+data+Le)
#define CMD_BUF_SIZE (1+4+1+255+1)
// Larger communication buffer size (max=reader status+data+sw)
#define RESP_BUF_SIZE (1+256+2)

// Offset of the status byte in a reader answer (CMD level)
#define STATUS_OFFSET 0
// Offset of the first byte of the ATR in a reader answer  (CMD level)
#define ATR_OFFSET 1

// Offset of the command byte in a GemCore command (CMD level)
#define OFFSET_CMD 0
// Offset of the APDU in a GemCore command (CMD level)
#define OFFSET_APDU 1
// Offset of the DATA in a GemCore response (CMD level)
#define OFFSET_DATA 1
// Nb of bytes added to the APDU in a GemCore command (CMD level)
#define FORMAT_SIZE 1
// STAT byte offset in a response to CARD STATUS command (CMD level)
#define OFFSET_STAT_BYTE 1
// Size of an ADPU
#define APDU_SIZE 4

// Bit mask to detect ICC presence
#define MASK_ICC_PRESENCE 0x04

// Gemcore error codes
enum {
    GCORE_OK               = 0x00,
    GCORE_UNKNOWN_CMD      = 0x01,
    GCORE_WRONG_TCK    = 0x1D,
    GCORE_CARD_PROT_ERR    = 0xA1,
    GCORE_CARD_MUTE        = 0xA2,
    GCORE_CARD_EXC_INT     = 0xE5,
    GCORE_NOT_9000       = 0xE7
};

// Protocols
enum
{
    T_0                    = 0,
    T_1                    = 1
};

// Number of slots a GemCore reader can address
#define MAX_SLOT_NB 2

#define MAX_ATR_SIZE       33
typedef struct GCORE_DESC
{
  ULONG nATRLength;
  UCHAR pcATRBuffer[MAX_ATR_SIZE];
} GCoreDesc;

#define GCoreLunToSlotNb(x) (0)
#endif
