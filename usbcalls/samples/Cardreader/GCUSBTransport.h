/*
 *  GCUSBTransport.h
 *  ifd-GemPC430
 *
 *  Created by JL Giraud <jl.giraud@free.fr> on Sun Nov 19 2000.
 *  License:   See file COPYING
 *
 */

#ifndef _GCUSBTRANSPORT_H_
#define _GCUSBTRANSPORT_H_

#include "GemCore.h"

// buffer size at transport level (largest of cmd level ones +1)
#define GC_TR_BUF_SIZE (((CMD_BUF_SIZE<RESP_BUF_SIZE)?RESP_BUF_SIZE:CMD_BUF_SIZE)+1)


// Offset of the length byte in a GemCore command
#define TR_OFFSET_LNG 0



#endif
