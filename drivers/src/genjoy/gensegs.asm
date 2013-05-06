; SCCSID = src/dev/usb/USBJOY/JOYSEGS.ASM, usb, c.basedd 98/07/10
;
;   Licensed Material -- Property of IBM
;
;   (c) Copyright IBM Corp. 1997, 1998  All Rights Reserved
;

        .XCREF
        .XLIST
        INCLUDE devhdr.inc
        .LIST
        .CREF

        EXTRN   _GameStrategy:FAR
        EXTRN   _Gameidc:FAR

DDHeader segment word public 'DDHEADER'
         EVEN
Header  LABEL  WORD             ; USB Keyboard driver header

dd      -1                      ; far pointer to next header
dw      DEV_CHAR_DEV            \
        OR DEV_IOCTL \
        OR DEV_30 \
        OR DEVLEV_3             ; attribute
dw      OFFSET  _GameStrategy   ; offset to strategy routine
dw      OFFSET  _Gameidc        ; offset to IDC entry point
db      'GAME$   '              ; device driver name
dw      ?                       ; Protect-mode CS strategy selector
dw      ?                       ; Protect-mode DS selector
dw      ?                       ; Real-mode CS strategy segment
dw      ?                       ; Real-mode DS segment
dd      DEV_ADAPTER_DD \
        OR DEV_IOCTL2           ; capabilities bit strip

DDHeader ends

_DATA   segment word public 'DATA'
_DATA   ends

CONST   segment word public 'CONST'
CONST   ends

_BSS    segment word public 'BSS'
_BSS    ends

RMCode  segment word public 'CODE'
RMCode  ends

Code    segment word public 'CODE'
Code    ends

_TEXT   segment word public 'CODE'
_TEXT   ends

;_INIT   segment word public 'CODE'
;_INIT   ends


; Programmer cannot control location of CONST and _BSS class segments
; being grouped.  They are always last.  Do not put anything in these
; segments since they will be truncated after device driver initialization.

DGROUP  GROUP   DDHeader, CONST, _BSS, _DATA
CGROUP  GROUP   RMCode, Code, _TEXT;, _INIT

end

