
        .text

;;; 
;;; delay_ms - delay for N milliseconds
;;;
;;;   The timing of this function only works on a 16Mhz CPU
;;;
;;;   r24 contains the number of milliseconds (255 max)
;;;   r21/r22 will be used as work registers
;;; 

                .global delay_ms
                .type delay_ms, @function

delay_ms:
                
                ldi     r22,246           ; 1 N A
delay_ms_2:     ldi     r21,15            ; 1 N A B
delay_ms_1:     nop                       ; 1 N A B
                dec     r21               ; 1 N A B
                brne    delay_ms_1        ; 1 N A B + (2 N A)        
                dec     r22               ; 1 N A
                brne    delay_ms_2        ; 1 N A + (2 N)        
                dec     r24               ; 1 N
                brne    delay_ms          ; 1 N + 2
                ret                       ; 4

;;; 
;;; delay_us - delay for N microseconds
;;;
;;;   The timing of this function only works on a 16Mhz CPU
;;;
;;;   r24 contains the number of microseconds (255 max)
;;;   r21/r22 will be used as work registers
;;; 

                .global delay_us
                .type delay_us, @function

delay_us:       nop                     ; 1
                nop                     ; 1
                nop                     ; 1
                nop                     ; 1
                nop                     ; 1
                dec     r24             ; 1
                brne    delay_us_more   ; 1 or 2
                ret                     ; 4

delay_us_more:  nop                     ; 1
                nop                     ; 1
                nop                     ; 1
                nop                     ; 1
                nop                     ; 1
                nop                     ; 1
                nop                     ; 1
                nop                     ; 1
                nop                     ; 1
                nop                     ; 1
                nop                     ; 1
                nop                     ; 1
                nop                     ; 1
                dec     r24             ; 1
                brne    delay_us_more   ; 2

                ret                     ; 4

;;; 
;;; delay - delay for N seconds
;;;
;;;   The timing of this function only works on a 16Mhz CPU
;;;
;;;   r24 contains the number of microseconds (255 max)
;;;   r21/r22 will be used as work registers
;;; 

                .global delay
                .type delay, @function

delay:          push    r28             ;
                push    r29             ;
                mov     r28,r24         ;        
delay_1:        ldi     r29,4           ;
delay_2:        ldi     r24,250         ;
                rcall    delay_ms        ;
                dec     r29             ;
                brne    delay_2         ;
                dec     r28             ;
                brne    delay_1         ;
                pop     r29             ;
                pop     r28             ;
                ret                     ;
