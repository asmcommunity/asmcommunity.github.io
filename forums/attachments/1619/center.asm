sseg segment stack
    db 256 dup (?)
sseg ends

cseg segment
    assume cs:cseg, ss:sseg

; procedura obradjuje red na koji pokazuje BX u video memoriji
centerrow proc
  push bx

  mov dx, 0         ; brojac praznina sa leve strane
  mov cx, 0         ; brojac praznina sa desne strane

  ; izracunaj koliko ima praznih sa leve(DX) i zapamti koji je prvi neprazan karakter sa leve strane (SI pokazuje na njega)
  mov si, bx
next_left:
  mov al, es:[si]
  cmp al, 32
  jne proceed1
  add si, 2
  inc dx
  cmp dx, 80           ; da li smo stigli do kraja reda, ako da izadji iz procedure
  je end_proc
  jmp next_left

proceed1:

   ; izracunaj koliko ima praznih sa desne(CX) i zapamti koji je prvi neprazan karakter sa desne strane (DI pokazuje na njega)
  mov di, bx
  add di, 158
next_right:
  mov al, es:[di]
  cmp al, 32
  jne proceed2
  sub di, 2
  inc cx
  jmp next_right

  ; izracunaj pomeraj u odnosu na DX, CX ce sadrzati pomenuti pomeraj
proceed2:
  add cx, dx
  shr cx, 1       ; deljenje sa 2
  sub cx, dx
  sal cx, 1       ; mnozenje sa 2 sa predznakom
   
   
  cmp cx, 0       ; da li uopste treba pomerati red
  je end_proc
  cmp cx, 0       ; negativan pomeraj -> znaci ceo red ide u levo
  jl to_left
            
  ; BX pokazuje na karakter koji treba da se prekopira
  ; BX + SI pokazuje na mesto gde kopirani karakter treba da dodje
  ; DI pokazuje na poslednji karakter koji ce se kopirati
  ; AX pomeraj u redu za karaktere
            
  ; na desno
  mov bx, di
  mov di, si
  mov ax, -2
  add bx, 2
  jmp process

  ; na levo
to_left:
  mov bx, si
  mov ax, 2
  sub bx, 2

process:
  mov si, cx          ; SI je sada pomeraj
            
  ; obradi prisutne karaktere
next_char:
  add bx, ax
  mov dx, es:[bx]
  mov es:[bx+si], dx
  cmp bx, di
  jne next_char

  ; ciscenje preostalih nepotrebnih
next_blank:
  add si, ax
  mov dl, ' '
  mov es:[bx+si], dl
  cmp si, 0
  jne next_blank

end_proc:
  pop bx
  ret
centerrow endp


main:
  mov ax, 0b800h
  mov es, ax

  ;cekamo da se pritisne neki taster
  mov ah, 7
  int 21h

  mov bx, 0
next:
  call centerrow
  add bx, 160
  cmp bx, 3840
  jle next

  mov ah, 7
  int 21h

end_program:
  mov ax, 4c00h
  int 21h

cseg ends
end main