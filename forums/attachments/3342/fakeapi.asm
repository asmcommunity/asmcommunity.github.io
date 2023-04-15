; nasm -f elf32 fakeapi.asm
; use it like:
; ld -o myfile mywinfile.o fakeapi.o
; probably wanna strip it!

global WSAStartup@8
global socket@12
global connect@12
global send@16
global recv@16
global bind@12
global listen@8
global accept@12
global closesocket@4
global WSACleanup@0

global GetStdHandle@4
global WriteFile@20

global CreateThread@24
global ExitThread@4

global ExitProcess@4

%define STDIN 0
%define STDOUT 1
%define STDERR 2

; sys_calls
%define __NR_exit 1
%define __NR_read 3
%define __NR_write 4
%define __NR_open 5
%define __NR_close 6
%define __NR_ioctl 54
%define __NR_socketcall 102
%define __NR_clone 120
%define __NR_nanosleep 162

; flags for sys_open
%define O_RDWR 2
%define O_APPEND 2000q
%define O_CREATE 100q
%define O_TRUNC 1000q

; commands for sys_socketcall
%define SYS_SOCKET 1
%define SYS_BIND 2
%define SYS_CONNECT 3
%define SYS_LISTEN 4
%define SYS_ACCEPT 5
%define SYS_SEND 9
%define SYS_RECV 10

; error numbers
%define EINTR 4

; flags for sys_clone
%define CSIGNAL 0FFh
%define CLONE_VM 100h
%define CLONE_FS 200h
%define CLONE_FILES 400h
; more....

WSAStartup@8:
    xor eax, eax
    ret 8

socket@12:
    push ebx
    lea ecx, [esp + 4 + 4]
    mov ebx, SYS_SOCKET
    mov eax, __NR_socketcall
    int 80h
    pop ebx
    ret 12

connect@12:
    push ebx
    lea ecx, [esp + 4 + 4]
    mov ebx, SYS_CONNECT
    mov eax, __NR_socketcall
    int 80h
    pop ebx
    ret 12

bind@12:
    push ebx
    lea ecx, [esp + 4 + 4]
    mov ebx, SYS_BIND
    mov eax, __NR_socketcall
    int 80h
    pop ebx
    ret 12

listen@8:
    push ebx
    lea ecx, [esp + 4 + 4]
    mov ebx, SYS_LISTEN
    mov eax, __NR_socketcall
    int 80h
    pop ebx
    ret 8

accept@12:
    push ebx
    lea ecx, [esp + 4 + 4]
    mov ebx, SYS_ACCEPT
    mov eax, __NR_socketcall
    int 80h
    pop ebx
    ret 12

send@16:
    push ebx
    lea ecx, [esp + 4 + 4]
    mov ebx, SYS_SEND
    mov eax, __NR_socketcall
    int 80h
    pop ebx
    ret 16

recv@16:
    push ebx
    lea ecx, [esp + 4 + 4]
    mov ebx, SYS_RECV
    mov eax, __NR_socketcall
    int 80h
    pop ebx
    ret 16

closesocket@4:
    push ebx
    mov ebx, [esp + 4 + 4]
    mov eax, __NR_close
    int 80h
    pop ebx
    ret 4

WSACleanup@0:
    xor eax, eax
    ret

GetStdHandle@4:
    mov ecx, [esp + 4]
    mov eax, STDIN
    cmp ecx, -10
    je .done
    mov eax, STDOUT
    cmp ecx, -11
    je .done
    mov eax, STDERR ; ???
.done:    
    ret 4

WriteFile@20:
    push ebx

    mov edx, [esp + 12 + 4]
    mov ecx, [esp + 8 + 4]
    mov ebx, [esp + 4 + 4]
    mov eax, __NR_write
    int 80h
    mov edx, [esp + 16 + 4]
    mov [edx],eax
    
    pop ebx
    ret 20

ExitProcess@4:
    mov ebx, [esp + 4]
    mov eax, 1
    int 80h
; ret hah!

CreateThread@24:
    push ebp
    mov ebp, esp

%define security_attributes ebp + 8
%define stack_size ebp + 12
%define the_work ebp + 16
%define the_parameter ebp + 20
%define flags ebp + 24
%define thread_ID ebp + 28
    
    push ebx

    mov eax, __NR_clone
    mov ebx, CSIGNAL | CLONE_FS | CLONE_FILES
    mov ecx, 0
    mov edx, 0
    int 80h
    test eax, eax
    jnz .parent

.child:
    push dword [the_parameter]
    call [the_work]
    ; when/if we return?
    mov eax, __NR_exit
    int 80h

.parent:
    mov ecx, [thread_ID]
    mov [ecx], eax

    pop ebx
    mov esp, ebp
    pop ebp

%undef security_attributes
%undef stack_size
%undef the_work
%undef the_parameter
%undef flags
%undef thread_ID

    ret 24
    
ExitThread@4:
    mov ebx, [esp + 4]
    mov eax, __NR_exit
    int 80h
;     ret 4 
    