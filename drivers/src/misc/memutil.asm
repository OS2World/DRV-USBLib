;/* SCCSID = src/dev/usb/misc/memutil.asm, usb, c.basedd 98/12/08 */
;
;   Licensed Material -- Property of IBM
;
;   (c) Copyright IBM Corp. 1997, 1998  All Rights Reserved
;/

;/*@internal src/dev/usb/misc/memutil.asm, usb, c.basedd
;  @notes
;  The Watcom __fmemcpy intrinsic function resolves to a movsw instruction
;  which requires 4 clock cycles to move 2 bytes of data. the movsd instruction
;  can move 4 bytes in the same 4 clock cycles. Such a deal !!!!!
;  Since the compiler won't let us write a pragma aux to implement this
;  function, we have written it in assembler. There is also a memfill function
;  that uses the stosd instruction to fill memory with the a value in eax.
;  This function is used to write "silence" into the audio buffer
;
;  @version 1.1
;  @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
;   <stack context>.
;  @history
;
;

.386
.seq

RMCode  SEGMENT WORD PUBLIC USE16 'CODE'
        ASSUME cs:RMCode

;    void cdecl movmem(PVOID pdest, PVOID psrc, USHORT count);

public _movmem
_movmem proc near

DEST1_LOW equ  [bp+4]
DEST1_HIGH equ [bp+6]
SRC1_LOW equ   [bp+8]
SRC1_HIGH equ  [bp+0ah]
BUF1_LEN equ   [bp+0ch]

    ; point to parameters and local variables
        push   bp
        mov    bp,sp

    ; save registers
        push   cx
        push   bx
        push   ds
        push   es
        push   si
        push   di

    ; store number of bytes to transfer and calculate number of double words
    ; and set of transfer value
        mov    bx,BUF1_LEN
        mov    cx,bx
        shr    cx,2

    ; set size in bytes of object to be allocated
        mov    es,DEST1_HIGH
        mov    di,DEST1_LOW

    ; set up pointers to source buffer
        mov    ds,SRC1_HIGH
        mov    si,SRC1_LOW

    ; set up transfer direction
        cld

    ; do the transfer
        rep    movsd

    ; transfer number of odd bytes
        mov    cx,bx
        and    cx,03h
        rep    movsb

    ;set up return value and restore registers
        pop    di
        pop    si
        pop    es
        pop    ds
        pop    bx
        pop    cx
        pop    bp

   ;done
        ret
_movmem endp


;VOID memfill(UCHAR *destP, ULONG length, UCHAR value)
;    This function fills a block of memory with a particular value.
;    The function receives address of the memory, the length of the memory, and
; the fill value as parameters on the stack.
;    The function does not return a value.

public _setmem
_setmem proc near

DEST2_LOW equ  [bp+4]
DEST2_HIGH equ [bp+6]
VALUE2 equ     [bp+8]
BUF2_LEN equ   [bp+0ah]

    ; point to parameters and local variables
        push   bp
        mov    bp,sp

    ; save registers
        push   bx
        push   cx
        push   es
        push   di

    ; store number of bytes to transfer and calculate number of double words
    ; and set of transfer value
        mov    bx,BUF2_LEN
        mov    cx,bx
        shr    cx,2

    ; set size in bytes of object to be allocated
        mov    es,DEST2_HIGH
        mov    di,DEST2_LOW

    ; set up value
        mov    ax,VALUE2
        mov    ah,al    ; use single byte value
        shl    eax,010h
        mov    ax,VALUE2
        mov    ah,al    ; use single byte value

    ; set up transfer direction
        cld

    ; do the transfer
        rep    stosd

    ; transfer number of odd bytes
        mov    cx,bx
        and    cx,03h
        rep    stosb

    ;set up return value and restore registers
        pop    di
        pop    es
        pop    cx
        pop    bx
        pop    bp

   ;done
        ret
_setmem endp

;ULONG NEAR ULDif(LONG A, ULONG B)
; this functiosn does a 32bit div);
 PUBLIC _ULDiv
_ULDiv PROC NEAR
  enter 0,0
  A  equ [bp+4]
  B  equ [bp+8]
  push ebx
  xor edx,edx;
  mov eax, DWORD PTR A
  mov ebx, DWORD PTR B
  DIV ebx
  mov DWORD PTR A, eax
  mov ax,WORD PTR [bp+4]
  mov dx,WORD PTR [bp+6]
  pop ebx
  leave
  ret
_ULDiv endp

;ULONG NEAR ULMUL(LONG A, ULONG B)
; this functiosn does a 32bit mul);

 PUBLIC _ULMul
_ULMul PROC NEAR
  enter 0,0
  A  equ [bp+4]
  B  equ [bp+8]
  push ebx
  xor edx,edx;
  mov eax, DWORD PTR A
  mov ebx, DWORD PTR B
  MUL ebx
  mov DWORD PTR A, eax
  mov ax,WORD PTR [bp+4]
  mov dx,WORD PTR [bp+6]
  pop ebx
  leave
  ret
_ULMul endp


;USHORT NEAR GetDS(VOID)
;    This function returns current DS value.
public  _GetDS
_GetDS  proc near
        mov    ax,ds
        ret
_GetDS  endp

;USHORT NEAR CLISave(VOID)
;    This function returns current status register value and disables interrupts
public  _CLISave
_CLISave  proc near
         pushf
         cli
         pop    ax
         ret
_CLISave  endp

EXTRN _writeChar:NEAR
  PUBLIC  _HexLongToASCII
_HexLongToASCII PROC NEAR

;|*** char far *
;|*** near HexLongToASCII(char far * StrPtr, unsigned long wHexVal, unsigned short Option)
;|*** {
; Line 5
  enter 12,0
; fNonZero = -2
; Digit = -6
; Power = -10
; ShiftVal = -12
; hexStr = -14
; Option = 12
; wHexVal = 8
; StrPtr = 4
fNonZero        equ     [bp-2]
Digit           equ     [bp-6]
Power           equ     [bp-10]
ShiftVal        equ     [bp-12]
Option          equ     [bp+12]
wHexVal         equ     [bp+8]
StrPtr          equ     [bp+4]

FALSE           equ     0
TRUE            equ     1
LEADING_ZEROES  equ     80h

;|***    int       fNonZero=FALSE;
; Line 6
  mov WORD PTR fNonZero,FALSE ;fNonZero
;|***    unsigned long    Digit;
;|***    unsigned long    Power=0xF0000000;
; Line 8
  mov DWORD PTR Power,-268435456  ;f0000000H  ;Power
;|***    unsigned short   ShiftVal=28;
; Line 9
  mov WORD PTR ShiftVal,28  ;001cH  ;ShiftVal
;|***    char     hexStr;
;|***
;|***    while (Power)
; Line 12
          $FC117:
  cmp DWORD PTR Power,0 ;Power
  je  $FB118
;|***    {
;|***       Digit=(wHexVal & Power)>>ShiftVal;
; Line 14
  mov ax,WORD PTR Power ;Power
  mov dx,WORD PTR [bp-8]
  and ax,WORD PTR wHexVal ;wHexVal
  and dx,WORD PTR [bp+10]
  mov cl,BYTE PTR ShiftVal  ;ShiftVal
  or  cl,cl
  je  SHORT $L139
          $L138:
  shr dx,1
  rcr ax,1
  dec cl
  jne SHORT $L138
          $L139:
  mov WORD PTR Digit,ax ;Digit
;|***       if (Digit)
; Line 15
  mov ax,dx
  or  ax,WORD PTR Digit ;Digit
  je  SHORT $I119
;|***          fNonZero=TRUE;
; Line 16
  mov WORD PTR fNonZero,TRUE  ;fNonZero
;|***
;|***       if (Digit || fNonZero || (Option & LEADING_ZEROES) ||
; Line 18
          $I119:
;|***           ((Power==0x0F) && (fNonZero==FALSE)))
; Line 19
  mov ax,dx
  or  ax,WORD PTR Digit ;Digit
  jne SHORT $I121
  cmp WORD PTR fNonZero,FALSE ;fNonZero
  jne SHORT $I121
  test  BYTE PTR [bp+13],LEADING_ZEROES ;0080H
  jne SHORT $I121
  cmp DWORD PTR Power,15  ;0000000fH  ;Power
  jne SHORT $I120
  cmp WORD PTR fNonZero,FALSE ;fNonZero
  jne SHORT $I120
          $I121:
;|***       {
;|***          if (Digit<=9)
; Line 21
  or  dx,dx
  jne SHORT $I122
  cmp WORD PTR Digit,9  ;Digit
  ja  SHORT $I122
;|***             hexStr=(char)('0'+Digit);
; Line 22
  mov al,BYTE PTR Digit ;Digit
  add al,48 ;0030H
  jmp SHORT $L141
          $I122:
;|***          else
;|***             hexStr=(char)('A'+Digit-10);
; Line 24
  mov al,BYTE PTR Digit ;Digit
  add al,55 ;0037H
          $L141:
;|***          writeChar(hexStr);
; Line 25
  push  ax
  call  _writeChar
  add sp,2
;|***       }
;|***
;|***       Power=Power>>4;
; Line 28
          $I120:
  mov eax,DWORD PTR Power ;Power
  shr eax,4
  mov DWORD PTR Power,eax ;Power
;|***
;|***       ShiftVal-=4;
; Line 30
  sub WORD PTR ShiftVal,4 ;ShiftVal
;|***    } // end while
; Line 31
  jmp $FC117
  nop
          $FB118:
;|***
;|***    return (StrPtr);
; Line 33
  mov ax,WORD PTR [bp+4]  ;StrPtr
  mov dx,WORD PTR [bp+6]
;|*** }
; Line 34
  leave
  ret

_HexLongToASCII ENDP

RMCode      ENDS

_TEXT   SEGMENT WORD PUBLIC USE16 'CODE'
        ASSUME cs:_TEXT
;/********************** START OF SPECIFICATIONS ***********************/
;/*                                                                    */
;/* SUBROUTINE NAME:  SetDec                                           */
;/*                                                                    */
;/* DESCRIPTIVE NAME:  Convert binary to decimal.                      */
;/*                                                                    */
;/* FUNCTION:  The function of this routine is to convert long binary  */
;/*            to decimal string and store in target buffer.           */
;/*                                                                    */
;/* NOTES:                                                             */
;/*                                                                    */
;/* CONTEXT: Initialization time                                       */
;/*                                                                    */
;/* ENTRY POINT:  SetDec                                               */
;/*    LINKAGE:  CALL NEAR                                             */
;/*                                                                    */
;/* INPUT:  ULONG value - binary value to be converted to decimal      */
;/*         PSZ converted - target buffer                              */
;/*                                                                    */
;/* EXIT-NORMAL: n/a                                                   */
;/*                                                                    */
;/* EXIT-ERROR:  n/a                                                   */
;/*                                                                    */
;/* EFFECTS:  none                                                     */
;/*                                                                    */
;/* INTERNAL REFERENCES:                                               */
;/*                                                                    */
;/* EXTERNAL REFERENCES:                                               */
;/*                                                                    */
;/************************ END OF SPECIFICATIONS ***********************/
public  _SetDec
_SetDec PROC NEAR
;|*** unsigned char SetDec( unsigned long value, char far * converted)
VALUE           equ     [bp+4]
CONVERTED       equ     [bp+8]
shiftIndex      equ     [bp-2]
decDigit        equ     [bp-4]
targetLength    equ     [bp-6]
tenPower        equ     [bp-10]
;|*** {
; Line 2
  enter 10,0
; shiftIndex = -2
; decDigit = -4
; decChar = -6
; targetLength = -8
; tenPower = -12
; converted = 10
; value = 6
;|***    unsigned char    shiftIndex;
;|***    unsigned char    decDigit, decChar, targetLength;
;|***    unsigned long    tenPower=1000000000;
; Line 5
        mov DWORD PTR tenPower,1000000000 ;3b9aca00H  ;tenPower
;|***
;|***    // ignore leading 0s
;|***    for (shiftIndex=0;shiftIndex<9; tenPower/=10, shiftIndex++)
; Line 8
  mov BYTE PTR shiftIndex,0 ;shiftIndex
  jmp SHORT $F111
;|***    {
;|***       if (value/tenPower)
;|***          break;
;|***    }
; Line 12
          $FC112:
  mov eax,DWORD PTR tenPower  ;tenPower
  mov ecx,10  ;0000000aH
  xor edx,edx
  div ecx
  mov DWORD PTR tenPower,eax  ;tenPower
  inc BYTE PTR shiftIndex ;shiftIndex
          $F111:
  cmp BYTE PTR shiftIndex,9 ;shiftIndex
  jae SHORT $FB113
;|***       if (value/tenPower)
; Line 10
  mov eax,DWORD PTR VALUE ;value
  xor edx,edx
  div DWORD PTR tenPower  ;tenPower
  or  eax,eax
  je  SHORT $FC112
;|***          break;
; Line 11
          $FB113:
;|***    }
;|***
;|***    for (targetLength=0; shiftIndex<10; tenPower/=10, shiftIndex++, targetLength++)
; Line 14
  mov BYTE PTR targetLength,0 ;targetLength
  jmp SHORT $F115
  nop
          $FC116:
;|***    {
;|***       decDigit= (unsigned char)(value/tenPower);
; Line 16
  mov eax,DWORD PTR VALUE ;value
  xor edx,edx
  div DWORD PTR tenPower  ;tenPower
  mov BYTE PTR decDigit,al  ;decDigit
;|***       value-=decDigit*tenPower;
; Line 17
  push  DWORD PTR tenPower  ;tenPower
  mov edx,eax
  shr edx,16  ;00000010H
  sub ah,ah
  push  0
  push  ax
  pop eax
  pop ecx
  mul ecx
  sub DWORD PTR VALUE,eax ;value
;|***       decChar=(unsigned char)('0'+decDigit);
; Line 18
  mov al,BYTE PTR decDigit  ;decDigit
  add al,48 ;0030H
;|***       *converted=decChar; converted++;
; Line 19
  les bx,DWORD PTR CONVERTED  ;converted
  inc WORD PTR CONVERTED  ;converted
  mov BYTE PTR es:[bx],al
;|***    }
; Line 20
  mov eax,DWORD PTR tenPower  ;tenPower
  mov ecx,10  ;0000000aH
  xor edx,edx
  div ecx
  mov DWORD PTR tenPower,eax  ;tenPower
  inc BYTE PTR shiftIndex ;shiftIndex
  inc BYTE PTR targetLength ;targetLength
          $F115:
  cmp BYTE PTR shiftIndex,10  ;000aH  ;shiftIndex
  jb  SHORT $FC116
;|***
;|***    return (targetLength);
; Line 22
  mov al,BYTE PTR targetLength  ;targetLength
;|*** }
; Line 23
  leave
  ret

_SetDec ENDP

;/********************** START OF SPECIFICATIONS ***********************/
;/*                                                                    */
;/* SUBROUTINE NAME:  ConvertDec                                       */
;/*                                                                    */
;/* DESCRIPTIVE NAME:  Convert to decimal.                             */
;/*                                                                    */
;/* FUNCTION:  The function of this routine is to convert ASCII string */
;/*            decimal.                                                */
;/*                                                                    */
;/* NOTES:                                                             */
;/*                                                                    */
;/* CONTEXT: Initialization time                                       */
;/*                                                                    */
;/* ENTRY POINT:  ConvertDec                                           */
;/*    LINKAGE:  CALL NEAR                                             */
;/*                                                                    */
;/* INPUT:  PSZ valStart - pointer to string to be converted           */
;/*         LONG FAR *value - pointer to double word to store          */
;/*                           converted value                          */
;/*                                                                    */
;/* EXIT-NORMAL: CFSTR_STATUS_OK                                       */
;/*              CFSTR_STATUS_NOVALUE - no value found                 */
;/*                                                                    */
;/* EXIT-ERROR:  CFSTR_STATUS_CONVERR - failed to convert              */
;/*                                                                    */
;/* EFFECTS:  none                                                     */
;/*                                                                    */
;/* INTERNAL REFERENCES:                                               */
;/*                                                                    */
;/* EXTERNAL REFERENCES:                                               */
;/*                                                                    */
;/************************ END OF SPECIFICATIONS ***********************/
public  _ConvertDec
_ConvertDec PROC NEAR
;|*** char ConvertDec(char far * valStart, long far *value)
valStart        equ     [bp+4]
value           equ     [bp+8]
;|*** {
; Line 2
  push  bp
  mov bp,sp
  push  di
  push  si
; value = 10
; valStart = 6
;|***    if (!*valStart || *valStart==' ')
; Line 3
  les bx,DWORD PTR valStart ;valStart
  cmp BYTE PTR es:[bx],0
  je  SHORT $I107
  cmp BYTE PTR es:[bx],32 ;0020H
  jne SHORT $F108
          $I107:
;|***       return (CFSTR_STATUS_NOVALUE);
; Line 4
  mov al,4  ;CFSTR_STATUS_NOVALUE
  pop si
  pop di
  leave
  ret
          $I111:
  mov al,BYTE PTR es:[bx]
  cbw
  sub ax,48 ;0030H
        cwd
        les bx,DWORD PTR value  ;value
        mov cx,WORD PTR es:[bx]
        mov si,WORD PTR es:[bx+2]
        mov bx,cx
        mov di,si
        add cx,cx
        adc si,si
        add cx,cx
        adc si,si
        add cx,bx
        adc si,di
        add cx,cx
        adc si,si
        add ax,cx
        adc dx,si
        mov bx,WORD PTR value ;value
        mov WORD PTR es:[bx],ax
        mov WORD PTR es:[bx+2],dx
;|***
;|***    for (; *valStart && *valStart!=' '; valStart++)
;|***    {
;|***       if (*valStart<'0' || *valStart>'9')
;|***          return (CFSTR_STATUS_CONVERR);
;|***       *value=*value*10+(long)(*valStart-'0');
;|***    }
; Line 11
  inc WORD PTR valStart ;valStart
          $F108:
  les bx,DWORD PTR valStart ;valStart
  cmp BYTE PTR es:[bx],0
  je  SHORT $FB110
  cmp BYTE PTR es:[bx],32 ;0020H
  je  SHORT $FB110
;|***       if (*valStart<'0' || *valStart>'9')
; Line 8
  cmp BYTE PTR es:[bx],48 ;0030H
  jl  SHORT $I112
  cmp BYTE PTR es:[bx],57 ;0039H
  jle SHORT $I111
          $I112:
;|***          return (CFSTR_STATUS_CONVERR);
; Line 9
  mov al,3  ;CFSTR_STATUS_CONVERR
  pop si
  pop di
  leave
  ret
          $FB110:
;|***       *value=*value*10+(long)(*valStart-'0');
;|***    }
;|***
;|***    return (CFSTR_STATUS_OK);
; Line 13
  mov al,1  ;CFSTR_STATUS_OK
;|*** }
; Line 14
  pop si
  pop di
  leave
  ret

_ConvertDec ENDP

_TEXT      ENDS

            end
