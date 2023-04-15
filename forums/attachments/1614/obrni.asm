cseg segment
      assume cs:cseg

;***************************************
obrnired proc far		; rec FAR nam govori da ce skok na proceduru biti velik

           mov ax,si	; AX := SI

           mov dx,160	; DX := broj kolona * 2

           mul dx		; AX := AX * DX kako je AX = SI pise AX := SI * DX

           mov bx,ax  	; na pocetku smo reda

           mov bp,bx   	; BP := BX

           add bp,158  	; na kraju smo reda, jer ako stavimo 160, onda prelazimo na
	   			; pocetak sledeceg reda (prvi karakter je na '0')

           mov dx,40	; 40 zamena, jer zamenjujemo pola ekrana
	   			; DX nam je brojac u petlji ZAMENI

zameni:    mov al,es:[bx]	; AL := ES[BX], kako ES sadrzi adresu video memorije,
					; ova linija koda stavlja simbol sa ekrana na mestu
					; BX u registar AL

           xchg bx,bp		; zamenjuje vrednosti BX i BP

           mov ah,es:[bx]	; AH := ES[BX], sto je isto kao gore za AL

           mov es:[bx],al	; vracamo vrednost AL na poziciju BX koja je bila
	   				; na poziciji BP (pre XCHG BX,BP)

           xchg bx,bp		; opet zamenjujemo registre

           mov es:[bx],ah	; i ispisujemo ono sto je bilo u AH na ekran
	   				; isto kao gore za AL

           add bx,2		; povecavamo BX za 2, jer je jedan znak 2 bajta

           sub bp,2		; smanjujemo BP za 2 iz istih razloga

       	   dec dx		; DX := DX - 1, smanjujemo brojac

           cmp dx,0		; poredi vrednosti DX i 0

           jne zameni		; skoci na ZAMENI ako je jednako

           ret			; isto kao RETURN u MODULA-2)

obrnired endp			; pravi kraj procedure
					; (kao END imeProcedure u MODULA-2)
;***************************************

s:					; labela S

     mov bx,0b800h		; postavljamo ES na adresu video memorije 0b800h
     					; preko registra BX

     mov es,bx			; ES := 0b800h

     mov cx,25			; brojac za LOOP petlju, a 25 je zato sto imamo 25 vrsta
     					; (redova) na ekranu

     mov si,0			; y koordinata prvog slova na ekranu

petlja:
     call obrnired		; poziv procedure OBRNIRED

     inc si				; povecavamo broj reda (y koordinatu slova)

     loop petlja			; ako je CX > 0, skoci na labelu PETLJA

     mov ax,4c02h  		; interapt za povratak u dos

     int 21h       		; poziv dos interapta

cseg ends				; kraj CSEG segmenta

     end s				; kraj S labele