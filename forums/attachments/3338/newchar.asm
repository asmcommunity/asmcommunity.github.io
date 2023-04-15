DOSSEG
.MODEL SMALL
.DATA          
          NEWCHAR  DB 11111111b
                               DB 11111110b
                               DB 11111100b
                               DB 11111000b
                               DB 11110000b
                               DB 11100000b
                               DB 11000000b
                               DB 10000000b
                               
                               DB 11111111b
                               DB 11111110b
                               DB 11111100b
                               DB 11111000b
                               DB 11110000b
                               DB 11100000b
                               DB 11000000b
                               DB 10000000b     
                               
                               DB 11111111b
                               DB 11111110b
                               DB 11111100b
                               DB 11111000b
                               DB 11110000b
                               DB 11100000b
                               DB 11000000b
                               DB 10000000b     
                               
                               DB 11111111b
                               DB 11111110b
                               DB 11111100b
                               DB 11111000b
                               DB 11110000b
                               DB 11100000b
                               DB 11000000b
                               DB 10000000b     
                               
                               DB 11111111b
                               DB 11111110b
                               DB 11111100b
                               DB 11111000b
                               DB 11110000b
                               DB 11100000b
                               DB 11000000b
                               DB 10000000b          
.STACK
.CODE

MAIN    PROC
            MOV AX,DGROUP
            MOV DS,AX
            PUSH DS
            POP ES                                  ; make sure ES = DS
            MOV BP,offset NEWCHAR         ; put offset of your bit pattern in ES:BP           
            MOV AX,1110h                        ; Use the BIOS video interrupt service 11h, subservice 00h
            MOV BX,0800h                        ; put in BH, the amount of bytes per char in your bitmap pattern - and in BL font block to load (EGA: 0-3; VGA: 0-7)
            MOV CX,0005h                        ; put in CX, the number of chars to change
            MOV DX,0080h                        ; put in DX, the starting ascii number to change
            INT 10h
           
            MOV AH,4Ch
            INT 21h
 
MAIN   ENDP

END MAIN