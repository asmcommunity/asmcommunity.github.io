

DOSSEG
.MODEL SMALL
.STACK 200h
.386
.CODE
JUMPS
ASSUME CS:@CODE


dothoriz        db 0
cga_font        db 1024 dup (?)
count           db 8
message         db "Hello World!",0

Palette     DB 0,0,0
            DB 0,0,0
            DB 52,0,0
            DB 28,0,0
            DB 63,0,0
	    DB 32,0,0



SetVGA PROC NEAR
    mov     ax,0013h
    int     10h
    ret
SetVGA ENDP

SetText PROC NEAR
    mov     ax,0003h
    int     10h
    ret
SetText ENDP




;/////////////////////////////////////////////////////////////
START:

    cli
    call    SetVGA


    mov     ax,cs
    mov     es,ax
    mov     ax,1012h
    mov     bx,0
    mov     cx,6
    mov     dx,offset Palette
    int     10h


    mov     ax,0a000h
    mov     es,ax
    mov     ax,cs
    mov     ds,ax
    push    es


;ds=cs
;es=A000

    mov     dl,8
    
    xor     di,di
    add     di,3200

writedotsline:
    mov     ax,0003h
    mov     cx,160
    rep     stosw
  
    xor     ax,ax
    mov     cx,160
    rep     stosw
    dec     dl
    jnz     writedotsline

    push    bp
    mov     ax,1130h
    mov     bh,3
    int     10h
    
    mov    ax,es
    mov    ds,ax
    mov    si,bp
    pop    bp


    push   cs
    push   cs
    pop    es


    mov    di,offset cga_font
    mov    cx,512
    rep    movsw


    pop    ds


    pop    es
    
    mov   ax,0a000h
    mov   es,ax 

Mainloop:

    mov    di,3220
    lea    si,message
    mov    cx,7701
    
nextcolumn:
    dec    dothoriz
    jns    whichbit
    mov    dothoriz, 7

    
        
whichbit:
    movzx  bx,[si]
    push   di
    shl    bx,3
    add    bx, offset cga_font
     
    movzx  dx,dothoriz
    
nextline:
   
    xor    al,al
    bt     [bx],dx
    adc    al,3
                
    inc    bx
    mov    es:[di],al
    add    di,640
      
    cmp    di,cx
    jb     nextline
    
    inc    cx
    inc    cx
    pop    di
    add    di,2
    dec    count
    jnz    nextcolumn

    mov    count,8
    mov    dothoriz,7
    inc    si
    movzx  bx,[si]
    or     bx,bx
    jne    whichbit
    



    in      al,60h
    cmp     al,1
    je      Quit

    mov dothoriz,0
    jmp Mainloop

Quit:
    call SetText

    mov     ax,4c00h
    int     21h

END START
