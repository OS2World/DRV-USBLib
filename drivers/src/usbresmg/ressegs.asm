; $ID$
        .XCREF
        .XLIST
        INCLUDE devhdr.inc
        .LIST
        .CREF


        EXTRN   _Strategy:NEAR
        EXTRN   _IDCEntry:FAR
        PUBLIC  _gHead

DDHEADER SEGMENT WORD PUBLIC 'DDHEADER'
         EVEN

         dd      -1
         dw      DEV_CHAR_DEV            \
                 OR DEV_IOCTL            \
                 OR DEV_SHARE            \
                 OR DEVLEV_3             \
                 OR DEV_30               \
                 OR DEV_GIOCTL           ; attribute
         dw      OFFSET _Strategy
         dw      OFFSET _IDCEntry
_gHead   db      "USBRESM$"              ; driver name
         dw      0                       ; protect-mode CS strategy selector
         dw      0                       ; protect-mode DS selector
         dw      0                       ; real-mode CS strategy segment
         dw      0                       ; real-mode DS segment
         dd      DEV_INITCOMPLETE        \
                 OR DEV_ADAPTER_DD       \
                 OR DEV_16MB             ; capabilities bit strip


DDHEADER ENDS

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

_TEXT   SEGMENT WORD PUBLIC 'CODE'
_TEXT   ENDS

; Programmer cannot control location of CONST and _BSS class segments
; being grouped.  They are always last.  Do not put anything in these
; segments since they will be truncated after device driver initialization.
DGROUP  GROUP   DDHEADER, CONST, _BSS, _DATA
CGROUP  GROUP   RMCode, Code, _TEXT
        END

