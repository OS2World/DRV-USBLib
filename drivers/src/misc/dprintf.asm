; SCCSID = src/dev/usb/MISC/DPRINTF.ASM, usb, c.basedd 98/07/10
;
;   Licensed Material -- Property of IBM
;
;   (c) Copyright IBM Corp. 1997, 1998  All Rights Reserved
;
;****************************************************************************
;*
;* dprintf - this routine displays information on the debug terminal. it
;*           provides limited formatting, which is a subset of c's printf
;*           function
;*
;*      calling sequence:       push selector for insert 'n'
;*                              push offset for insert 'n'
;*                              push selector for 'n-1'
;*                              push offset for 'n-1'
;*                                 ...
;*                              push selector for second insert
;*                              push offset for second insert
;*                              push selector for first insert
;*                              push offset for first insert
;*                              push selector raw string
;*                              push offset for raw string
;*                              call dprintf
;*                              add sp,4 + ('n' * 4)
;*
;*                      for "%w", just push one word containing the data to
;*                                be displayed. make sure that the "add sp"
;*                                cleans the stack correctly.
;*
;*                      for "%z", just the repeat count, then the selector
;*                                of the area to display, and then the offset.
;*                                make sure that the "add sp" cleans the stack
;*                                correctly.
;*
;*      formatting:     prior to being displayed, the raw string is formatted
;*                      by scanning it for format control sequences. as each
;*                      format control sequence is encountered, it is replaced
;*                      by appropriately formatted text obtained from the
;*                      corresponding pointer
;*
;*                      the following format control sequences are supported:
;*
;*                      %c -  the corresponding far ptr points to a byte
;*                            which replaces the "%c"
;*
;*                      %u -  the corresponding far ptr points to a word
;*                            which is displayed as an unsigned decimal
;*                            integer, replacing the "%u"
;*
;*                      %x -  the corresponding far ptr points to a word
;*                            which is displayed as upper case hex,
;*                            replacing the "%X"
;*
;*                      %lx - the corresponding far ptr points to a double
;*                            word which is displayed as upper case hex,
;*                            replacing the "%X"
;*
;*                      %s -  the corresponding far ptr points to a null
;*                            terminated string which is displayed unchanged,
;*                            replacing the "%s"
;*
;*                      %p -  the corresponding far ptr is displayed as upper
;*                            case hex in the format "ssss:oooo"
;*
;*                      %w -  the corresponding word is displayed as upper
;*                            case hex replacing the "%w". note that in this
;*                            case, only one word is pushed onto the stack
;*                            for the %w insert
;*
;*                      %z -  using the corresponding repeat count and far
;*                            pointer, a memory dump is produced. note that
;*                            in this case the stack contains a repeat count
;*                            and a far pointer to the area to dump
;*
;*                      %% -  the character "%" is displayed, replacing the
;*                            "%%"
;*

.286p
;       .xlist
;               include addsegs.inc     ; code and data segment definitions
;       .list


COM1_PORT       EQU     03f8h
COM2_PORT       EQU     02f8h

DEFAULT_PORT    EQU     COM1_PORT                                       ;@Vxxxxx

CAR_RET         EQU     0DH
LINE_FEED       EQU     0AH
BELL            EQU     07H
COM_LSR         EQU     05H
COM_DAT         EQU     00H

s_frame         struc

s_bp            dw      ?               ; callers bp.
s_ptr_delta     dw      ?               ; delta (in bytes) to current pointer
                                        ; from first pointer.
s_ret           dd      ?               ; callers cs:ip.
s_string        dd      ?               ; far pointer to raw string.
s_ptrs          dd      ?               ; pointer to first variable.

s_frame         ends

;word_10000      dw      10000
;word_1000       dw      1000
;word_100        dw      100
;word_10         dw      10

RMCode   segment word public 'CODE'
assume cs:RMCode

                public  _dprintf

_dprintf        proc    far

                push    0               ; zero the delta to current pointer.

                push    bp              ; save our callers bp register.
                mov     bp,sp           ; point to our stack frame.

                push    ds              ; save our callers ds register.
                push    es              ; save our callers es register.
                pusha                   ; save all other caller registers.

                lds     si,ss:[bp+s_string] ; point to the raw string.

dprintf_loop:   lodsb                   ; pick up a byte of the string.

                or      al,al           ; is it the end of the string?
                jnz     dprintf_more    ; no, go check for format control.

                popa                    ; restore all other caller registers.
                pop     es              ; restore our callers es register.
                pop     ds              ; restore our callers ds register.
                pop     bp              ; restore our callers bp register.
                add     sp,2            ; unstack s_ptr_delta.

                ret                     ; return to our caller.

dprintf_more:   cmp     al,'%'          ; no, is it the start of a format
                                        ; control sequence?
                je      dprintf_type    ; yes, go see what type of sequence.
                jmp     dprintf_put_ch  ; no, go display character and return.

dprintf_type:   lodsb                   ; pick up a byte of the string.
                cmp     al,'%'          ; is caller trying to display "%"?

                jne     dprintf_try_c   ; no, go see if it is a "c".

                mov     al,'%'          ; yes, go display it
                jmp     dprintf_put_ch  ; and exit.

dprintf_try_c:  cmp     al,'c'          ; is it a string display?
                jne     dprintf_try_s   ; no, go see if it is a "s".

                lea     bx,[bp]+s_ptrs         ; yes, pick up the
                add     bx,ss:[bp+s_ptr_delta] ; corresponding
                les     bx,ss:[bx]             ; pointer.
                add     ss:[bp+s_ptr_delta],4  ; move down to next pointer.

                mov     al,es:[bx]      ; pick up a byte.
                jmp     dprintf_put_ch  ; go display character and return.

dprintf_try_s:  cmp     al,'s'          ; is it a string display?
                jne     dprintf_try_u   ; no, go see if it is a "u".

                lea     bx,[bp]+s_ptrs         ; yes, pick up the
                add     bx,ss:[bp+s_ptr_delta] ; corresponding
                les     bx,ss:[bx]             ; pointer.
                add     ss:[bp+s_ptr_delta],4  ; move down to next pointer.

dprintf_next_s: mov     al,es:[bx]      ; pick up a byte.

                or      al,al           ; is it the end of the string?
                jz      dprintf_loop    ; yes, go do next raw string byte.

                call    put_char        ; no, display the character.

                inc     bx              ; move down to the next character
                jmp     dprintf_next_s  ; and go round again.

dprintf_try_u:  cmp     al,'u'          ; is it an unsigned short display?
                jne     dprintf_try_x   ; no, go see if it is a "X".

                lea     bx,[bp]+s_ptrs         ; yes, pick up the
                add     bx,ss:[bp+s_ptr_delta] ; corresponding
                les     bx,ss:[bx]             ; pointer.
                add     ss:[bp+s_ptr_delta],4  ; move down to next pointer.

                mov     ax,es:[bx]      ; pick up the word to display.

                xor     dx,dx           ; convert the
                mov     cx, 10000
                div     cx              ; ten thousands
;               div     word_10000      ; ten thousands
                or      al,'0'          ; digit and
                call    put_char        ; display it.

                mov     ax,dx           ; convert the
                xor     dx,dx           ; thousands
                mov     cx, 1000
                div     cx              ; digit
;               div     word_1000       ; digit
                or      al,'0'          ; and
                call    put_char        ; display it.

                mov     ax,dx           ; convert the
                xor     dx,dx           ; hundreds
                mov     cx, 100
                div     cx              ; digit
;               div     word_100        ; digit
                or      al,'0'          ; and
                call    put_char        ; display it.

                mov     ax,dx           ; convert the
                xor     dx,dx           ; tens
                mov     cx, 10
                div     cx              ; digit
;               div     word_10         ; digit
                or      al,'0'          ; and
                call    put_char        ; display it.

                mov     al,dl           ; convert the units digit
                or      al,'0'          ; and go display it
                jmp     dprintf_put_ch  ; and return.

dprintf_try_x:  cmp     al,'x'          ; is it an unsigned short hex display?
                jne     dprintf_try_lx  ; no, go see if it is a "lX".

                lea     bx,[bp]+s_ptrs         ; yes, pick up the
                add     bx,ss:[bp+s_ptr_delta] ; corresponding
                les     bx,ss:[bx]             ; pointer.
                add     ss:[bp+s_ptr_delta],4  ; move down to next pointer.

                call    put_hex_word    ; convert and display the word.

                jmp     dprintf_loop    ; go do next raw string byte.

dprintf_try_lx: cmp     al,'l'          ; is it an unsigned long hex display?
                jne     dprintf_try_p   ; no, go see if it is a "p".
                lodsb                   ; maybe, pick up a byte of the string.
                cmp     al,'x'          ; is the second byte correct?
                je      dprintf_do_lx   ; no, go report
                jmp     dprintf_error   ; the error.

dprintf_do_lx:  lea     bx,[bp]+s_ptrs         ; yes, pick up the
                add     bx,ss:[bp+s_ptr_delta] ; corresponding
                les     bx,ss:[bx]             ; pointer.
                add     ss:[bp+s_ptr_delta],4  ; move down to next pointer.

                add     bx,2            ; move down to the second word.
                call    put_hex_word    ; convert and display the second word.
                sub     bx,2            ; move back to the first word.
                call    put_hex_word    ; convert and display the first word.

                jmp     dprintf_loop    ; go do next raw string byte.

dprintf_try_p:  cmp     al,'p'          ; is it a far pointer display?
                jne     dprintf_try_w   ; no, go see if it is a "w".

                lea     bx,[bp]+s_ptrs         ; yes, pick up the
                add     bx,ss:[bp+s_ptr_delta] ; corresponding pointer.
                add     ss:[bp+s_ptr_delta],4  ; move down to next pointer.

                push    es              ; save the callers data selector.

                push    ss              ; set up the proper
                pop     es              ; selector.

                add     bx,2            ; move down to the second word.
                call    put_hex_word    ; convert and display the selector.
                mov     al,':'          ; display
                call    put_char        ; the ":".
                sub     bx,2            ; move back to the first word.
                call    put_hex_word    ; convert and display the offset.

                mov     al,' '          ; display
                call    put_char        ; a couple
                mov     al,' '          ; of
                call    put_char        ; spaces.

                pop     es              ; recover the callers data selector.

                jmp     dprintf_loop    ; go do next raw string byte.

dprintf_try_w:  cmp     al,'w'          ; is it an immediate word display?
                jne     dprintf_try_b   ; no, go see if it is a "b".

                lea     bx,[bp]+s_ptrs         ; yes, pick up the
                add     bx,ss:[bp+s_ptr_delta] ; corresponding pointer.
                add     ss:[bp+s_ptr_delta],2  ; move down to next pointer.

                push    es              ; save the callers data selector.

                push    ss              ; set up the proper
                pop     es              ; selector.

                call    put_hex_word    ; convert and display the word.

                pop     es              ; recover the callers data selector.

                jmp     dprintf_loop    ; go do next raw string byte.

dprintf_try_b:  cmp     al,'b'          ; is it an immediate byte display?
                jne     dprintf_try_z   ; no, go see if it is a "z".

                lea     bx,[bp]+s_ptrs         ; yes, pick up the
                add     bx,ss:[bp+s_ptr_delta] ; corresponding pointer.
                add     ss:[bp+s_ptr_delta],2  ; move down to next pointer.

                push    es              ; save the callers data selector.

                push    ss              ; set up the proper
                pop     es              ; selector.

                call    put_hex_byte    ; convert and display the byte.

                pop     es              ; recover the callers data selector.

                jmp     dprintf_loop    ; go do next raw string byte.

dprintf_try_z:  cmp     al,'z'          ; is it a memory dump display?
                je      dprintf_do_z    ; no, go report
                jmp     dprintf_error   ; the error.

dprintf_do_z:
                lea     bx,[bp]+s_ptrs         ; yes, pick up the
                add     bx,ss:[bp+s_ptr_delta] ; corresponding pointer.
                add     ss:[bp+s_ptr_delta],6  ; move down to next pointer.

                mov     cx,ss:[bx+4]    ; pick up the repeat count.

                push    es              ; save the callers data selector.

                les     bx,ss:[bx]      ; point to the area to display.

dprintf_z_a:    mov     ax,es           ; pick up the selector to display.
                xchg    ah,al           ; set up to process the first byte.
                call    put_left_nib    ; display the first byte
                call    put_right_nib   ; of the selector.
                xchg    ah,al           ; set up to process the second byte.
                call    put_left_nib    ; display the second byte
                call    put_right_nib   ; of the selector.

                mov     al,':'          ; display a
                call    put_char        ; colon.

                mov     ax,bx           ; pick up the offset to display.
                xchg    ah,al           ; set up to process the first byte.
                call    put_left_nib    ; display the first byte
                call    put_right_nib   ; of the offset.
                xchg    ah,al           ; set up to process the second byte.
                call    put_left_nib    ; display the second byte
                call    put_right_nib   ; of the offset.

                mov     al,' '          ; display
                call    put_char        ; two
                mov     al,' '          ; seperating
                call    put_char        ; spaces.

                push    cx              ; save the repeat count for later.

                mov     dx,16*3+1       ; initialize the fill count.

                cmp     cx,16           ; are there more than 16 bytes left?
                jbe     dprintf_z_b     ; yes, limit it to 16 bytes
                mov     cx,16           ; for this line.

dprintf_z_b:    push    bx              ; save offset and display count
                push    cx              ; for the character display.

dprintf_z_c:    mov     al,es:[bx]      ; pick up a byte to display.
                call    put_hex_byte    ; display it in hex.

                mov     al,' '          ; set up to display a space.
                cmp     dx,9*3+1        ; should it be a dash?
                jne     dprintf_z_e     ; no, bypass changing it.
                mov     al,'-'          ; yes, set up to display a dash.
dprintf_z_e:    call    put_char        ; display the dash or space.

                sub     dx,3            ; down the fill count by one position.

                inc     bx              ; move down to the next byte.

                loop    dprintf_z_c     ; more to do? yes, go round again?

                mov     cx,dx           ; no, pick up remaining fill count.

dprintf_z_g:    mov     al,' '          ; display a
                call    put_char        ; space.

                loop    dprintf_z_g     ; more to do? yes, go round again.

                pop     cx              ; recover the offset and
                pop     bx              ; display count.

dprintf_z_i:    mov     al,'.'          ; set up to display a dot.

                mov     ah,es:[bx]      ; does the byte
                cmp     ah,20h          ; contain a
                jb      dprintf_z_k     ; valid ascii
                cmp     ah,7fh          ; code?
                ja      dprintf_z_k     ; no, go display the dot.

                xchg    al,ah           ; yes, set up to do byte's contents.

dprintf_z_k:    call    put_char        ; display a dot or the byte contents.

                inc     bx              ; move down to the next byte.

                loop    dprintf_z_i     ; more to do on this line?
                                        ; yes, go round again.

                pop     cx              ; no, recover the repeat count.

                sub     cx,16           ; down the repeat count by one line.

                jle     dprintf_z_z     ; more to do? no, go exit.

                mov     al,CAR_RET      ; perform
                call    put_char        ; a
                mov     al,LINE_FEED    ; new line
                call    put_char        ; operation.

                jmp     dprintf_z_a     ; go round and display another line.

dprintf_z_z:    pop     es              ; recover the callers data selector.

                jmp     dprintf_loop    ; go do next raw string byte.

dprintf_error:  mov     ah,al           ; display
                mov     al,'?'          ; an
                call    put_char        ; eye
                mov     al,'\'          ; catching
                call    put_char        ; "invalid
                mov     al,ah           ; format
                call    put_char        ; control"
                mov     al,'\'          ; message
                call    put_char        ; and
                mov     al,BELL         ; beep.

dprintf_put_ch: call    put_char        ; display the character.
                jmp     dprintf_loop    ; go process next raw string byte.

_dprintf        endp

put_left_nib    proc    near

                push    ax              ; save the callers ax register.

                shr     al,4            ; convert the
                add     al,'0'          ; left nibble
                cmp     al,'9'          ; to an ascii
                jbe     put_left_nib_a  ; hex
                add     al,'A'-'9'-1    ; representation.
put_left_nib_a: call    put_char        ; display the character.

                pop     ax              ; restore the callers ax register.

                ret                     ; return to our caller.

put_left_nib    endp

put_right_nib   proc    near

                push    ax              ; save the callers ax register.

                and     al,0fh          ; convert the
                add     al,'0'          ; right nibble
                cmp     al,'9'          ; to an
                jbe     put_rght_nib_a  ; ascii hex
                add     al,'A'-'9'-1    ; representation.
put_rght_nib_a: call    put_char        ; display the character.

                pop     ax              ; restore the callers ax register.

                ret                     ; return to our caller

put_right_nib   endp

put_hex_byte    proc    near

                mov     al,es:[bx]      ; display the left nibble
                call    put_left_nib    ; in ascii hex.

                mov     al,es:[bx]      ; display the right nibble
                call    put_right_nib   ; in ascii hex.

                ret                     ; return to our caller.

put_hex_byte    endp


put_hex_word    proc    near

                inc     bx              ; set up to process second byte first.

                call    put_hex_byte    ; display the byte in hex.

                dec     bx              ; move back to the first byte.

                call    put_hex_byte    ; display the byte in hex.

                ret                     ; return to our caller.

put_hex_word    endp


;                public  portadr
;portadr         dw      DEFAULT_PORT    ; change config.h to change this.
                                        ; use: com2=02f8H, com1=03f8H

IODelay Macro
    local a
    jmp a
a:
endm

PollC   PROC    NEAR

;       mov     dx, cs:PortAdr
        mov     dx, DEFAULT_PORT
        add     dx, COM_LSR

        in      al,dx                   ; get input status
;       IODelay

        and     al,1                    ; is there a char in RECV buffer?
        jz      plc1                    ; no, go return empty

;       mov     dx, cs:PortAdr
        mov     dx, DEFAULT_PORT
        add     dx, COM_DAT

        in      al,dx                   ; suck char out of buffer
;       IODelay

        and     al,07fh                 ; strip off stupid parity crap
plc1:   ret
PollC   ENDP


;**     PUTC - output a single char to COM2 handling ^S


put_char        proc    near

        push    dx
        push    ax

;       See if ^S

        call    PollC               ; is there a char at input
        jz      pc2                 ; no, go output our char
        cmp     al,'S' - 'A' + 1    ; is it ^S?
        jnz     pc2                 ; no, go output our char

;       Saw ^S.  Wait for and eat next char.

pc1:    call    PollC               ; look for next char
        jz      pc1                 ; no char, go look again
        cmp     al,'S' - 'A' + 1    ; is it ^S again?
        jz      pc1                 ; yes, go look for something else

;pc2:    mov     dx, cs:PortAdr
pc2:    mov     dx, DEFAULT_PORT
        add     dx, COM_LSR
        in      al,dx
;       IODelay
        test    al,020h
        jz      pc2

;       ready.  crank it out!

;       mov     dx, cs:PortAdr
        mov     dx, DEFAULT_PORT
        add     dx, COM_DAT
        pop     ax
        out     dx,al

        pop     dx                  ; restore the callers dx register.

        ret

put_char        endp

RMCode  ends
        end
