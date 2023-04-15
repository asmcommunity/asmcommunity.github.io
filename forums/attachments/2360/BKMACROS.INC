;---------------------------------------------------------------;
; bkmacros.inc - commonly used macros.                          ;
;                                                               ;
; Written by : Bryant Keller                                    ;
; Email      : bkeller@codegurus.org                            ;
; Page       : http://asm.codegurus.org/                        ;
; Compiler   : Netwide Assembler v0.98.39                       ;
; Date       ; 23-Feb-2007                                      ;
;                                                               ;
;---------------------------------------------------------------;
%ifndef MACROS_INC
%define MACROS_INC

%define T tword
%define Q qword
%define D dword
%define W word
%define B byte

%idefine TWORD_size 10
%idefine QWORD_size 8
%idefine DWORD_size 4
%idefine WORD_size  2
%idefine BYTE_size  1

%idefine arg(x) [ebp+4+(4*x)]
%idefine @b %$B
%idefine @f %$F

%macro @@ 0
 %ifctx _masm_jump_
   %$F:
  %pop
 %endif
 %push _masm_jump_
 %$B:
%endmacro

%imacro m2m 2
 %ifdef MEM_MOVE_REG
  mov MEM_MOVE_REG, %{2}
  mov %{1}, MEM_MOVE_REG
 %else
  push %{2}
  pop %{1}
 %endif
%endmacro

%imacro assume 2
 %ifidni %2, NOTHING
  %undef %{1}.
 %else
  %define %{1}.(_x_) [%{1} + %{2}. %+ _x_]
 %endif
%endmacro

%define sizeof(_x_) _x_ %+ _size

%imacro push 1-*
 %push _push_
 %assign %$args 0
 %rep %0
  %ifstr %1
[section .data]
%%str_%$args: db %1, 0
[section .text]
    push dword %%str_%$args
  %else
   push %1
  %endif
  %assign %$args %$args+1
  %rotate 1
 %endrep
 %pop
%endmacro

%imacro call 1
 %push invoke

 %ifdef NASM32_COMPATIBLE

  %ifdef __stdcall_defined_%{1}
   %define %{1} __stdcall_defined_%{1}
   extern __stdcall_defined_%{1}
   %undef _CALL_TYPE_FLAG_
  %endif

  %ifdef __cdecl_defined_%{1}
   %define %{1} __cdecl_defined_%{1}
   extern __cdecl_defined_%{1}
   %undef _CALL_TYPE_FLAG_
  %endif

  %ifdef _UNDERSCORE_
   %define _proc _%{1}
  %else
   %define _proc %{1}
  %endif

  call _proc

 %else

  %ifndef _UNDERSCORE_
   call %{1}
  %else
   call _%{1}
  %endif

 %endif

 %pop
%endmacro

%imacro begin 0-1
 %push _block_
  %if %0 =1
   enter %1,0
  %else
   enter 0,0
  %endif
%endmacro

%imacro leave 0
 %ifctx _block_
  %define _block_leave_
 %else
   leave
 %endif
%endmacro

%imacro ret 0-1
 %ifctx _block_
  %ifdef _block_leave_
   %undef _block_leave_
  %endif
  jmp %$exit
 %else
  %if %0 =1
   ret %1
  %else
   ret
  %endif
 %endif
%endmacro

%imacro end 0
 %ifctx _block_
  %$exit:
  %pop
  leave
  ret
 %endif
%endmacro

%imacro .code 0
  [section .text]
%endmacro

%imacro .data 0
  [section .data]
%endmacro

%imacro .bss 0
  [section .bss]
%endmacro

%endif ; MACROS_INC
