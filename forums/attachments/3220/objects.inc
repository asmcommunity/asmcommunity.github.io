;; Filename:		objects.inc
;; Developer:		Bryant Keller (bkeller@assembly.ath.cx)
;; Description:		Object-Oriented Programming Support for NASM 
;; Last Modified:	Wed 14 July 2010, 01:17 PM EST - Added 'use' macro for easier multi-file develompent.
;;			Mon 12 July 2010, 07:51 PM EST - Removed unnecessary fallback Dtor and cleaned up Ctor.
;;			Mon 12 July 2010, 06:38 PM EST - Fixed multiple declaration bug.
;;			Mon 12 July 2010, 01:26 PM EST - Cleaned up Ctor/Dtor operations (they are no longer mandatory).
;;			Mon 12 July 2010, 12:56 PM EST - Added virtual overloading support.
;;			Sun 11 July 2010, 11:24 PM EST - Added basic inheritance support.
;;			Sat 03 July 2010, 01:16 PM EST - Defined symbols to be used during inheritance.
;;			Fri 02 July 2010, 12:07 PM EST - Created based object-oriented support.
;; Thanks:		Ultrano, EvilHomer2k, & c051n3
;; Todo:
;;	o Flush Out Registers (Abstract Away 32/64-bit)
;;	o Add MacOSX Support (16-byte Aligned CDECL)

%ifndef NASM_OBJECT_MODEL_SUPPORT
%define NASM_OBJECT_MODEL_SUPPORT

%ifidni __OUTPUT_FORMAT__,win32
	EXTERN GetProcessHeap
	EXTERN HeapAlloc
	EXTERN HeapFree
%else
	EXTERN malloc
	EXTERN free
%endif

%idefine sizeof(_x_) _x_ %+ _size

%idefine byte_size	1
%idefine word_size	2
%idefine dword_size	4
%idefine qword_size	8
%idefine tword_size	10
%idefine oword_size	16

%imacro rpush 1-*
	%rep %0
		%rotate -1
		push %{1}
	%endrep
%endm
%imacro mpush 1-*
	%rep %0
		push %{1}
		%rotate 1
	%endrep
%endm

%macro nasm_alloc 1
	%ifidni __OUTPUT_FORMAT__,win32
		call GetProcessHeap
		push dword %{1}
		push dword 0
		push eax
		call HeapAlloc
	%else
		push dword %{1}
		call malloc
		add esp, 4
	%endif
%endm

%macro nasm_free 1
	%ifidni __OUTPUT_FORMAT__,win32
		call GetProcessHeap
		push dword %{1}
		push dword 0
		push eax
		call HeapFree
	%else
		push dword %{1}
		call free
		add esp, 4
	%endif
%endm

%imacro new 1-2+
	nasm_alloc %{1}_size
	push eax

	push eax
	call %{1}___%{1}

	%if %{1}_has_ctor = 1
		rpush %{2}
		push eax
		call %{1}_%{1}
	%endif
	pop eax
%endm

%imacro delete 1
	%ifnidni %{1},eax
		mov eax, %{1}
	%endif
%push _if_dtor_is_zero
	cmp [eax], dword 0
	je %%no_dtor
	push eax
	call [eax]	;; Call Destructor
%%no_dtor:
%pop
	push eax
	nasm_free %{1}
	pop eax
%endm

%imacro mcall 3-4+
	push edx
	mov edx, %{1}
	%if %0 > 3
		rpush %{4}
	%endif
	push edx
	call [edx + %{2}.%{3}]
	pop edx
%endm

%imacro vcall 3-4+
	push edx
	mov edx, %{1}
	%if %0 > 3
		rpush %{4}
	%endif
	push edx
	mov edx, [edx + %{2}.vmt]
	call [edx + %{2}_%{3}_ID * 4]
	pop edx
%endm

%imacro static 2-3
%ifctx __CLASS__
	%ifidni %{1},method
		%ifidni %{2}, %{$name}
			%assign %$has_ctor 1
		%elifidni %{2}, @%{$name}
			%assign %$has_dtor 1
		%else

			%define %[%{$name}]_static_method_%[%{$number_of_static_methods}]_full %[%{$name}]_%{2}
			%define %[%{$name}]_static_method_%[%{$number_of_static_methods}] %{2}

			%assign %$number_of_static_methods %{$number_of_static_methods} + 1
		%endif
	%elif %0 = 3
		%[%{$name}]_static_data_%[%{$number_of_static_members}]_size equ %{1}_size * %{3}
		%define %[%{$name}]_static_data_%[%{$number_of_static_members}] %{2}
		%assign %$number_of_static_members %{$number_of_static_members} + 1
	%else
		%[%{$name}]_static_data_%[%{$number_of_static_members}]_size equ %{1}_size
		%define %[%{$name}]_static_data_%[%{$number_of_static_members}] %{2}
		%assign %$number_of_static_members %{$number_of_static_members} + 1
	%endif
%else
	%error "'static' used outside of class"
%endif
%endm

%imacro virtual 2-3
%ifctx __CLASS__
	%ifidni %{1},method
		%define %$current_parent %{$name}
		%rep %[%{$name}]_parent_count

			%if %[%{$current_parent}]_has_parent = 1

				%define %$current_parent %[%{$current_parent}]_parent

				%assign %$ii 0
				%rep %[%{$current_parent}]_total_virtual_methods
					%ifdef %[%{$current_parent}]_virtual_method_%[%{$ii}]_short
						%define %[%{$name}]_%[%{2}]_ID %[%[%{$current_parent}]_virtual_method_%[%{$ii}]]

						%define %[%{$name}]_virtual_method_%[%{$ii}]_full %[%{$name}]_%{2}
						%define %[%{$name}]_virtual_method_%[%{$ii}]_short %{2}
						%define %[%{$name}]_virtual_method_%[%{$ii}] %[%{$name}]_%[%{2}]_ID
					%endif
					%assign %$ii %{$ii} + 1
				%endrep

			%endif

		%endrep

		%ifndef %[%{$name}]_%{2}_ID
			%define %[%{$name}]_%[%{2}]_ID %[%{$number_of_virtual_methods}]

			%define %[%{$name}]_virtual_method_%[%{$number_of_virtual_methods}]_full %[%{$name}]_%{2}
			%define %[%{$name}]_virtual_method_%[%{$number_of_virtual_methods}]_short %{2}
			%define %[%{$name}]_virtual_method_%[%{$number_of_virtual_methods}] %[%{$name}]_%[%{2}]_ID

			%assign %$number_of_virtual_methods %{$number_of_virtual_methods} + 1
		%endif
	%elif %0 = 3
		%[%{$name} %+ . %+ %{2}] equ %{$number_of_virtual_members}
		%[%{$name}]_virtual_data_%[%{$number_of_virtual_members}]_size equ %{1}_size * %{3}
		%define %[%{$name}]_virtual_data_%[%{$number_of_virtual_members}] %[%{$name} %+ . %+ %{2}]
		%assign %$number_of_virtual_members %{$number_of_virtual_members} + 1
	%else
		%[%{$name} %+ . %+ %{2}] equ %{$number_of_virtual_members}
		%[%{$name}]_virtual_data_%[%{$number_of_virtual_members}]_size equ %{1}_size
		%define %[%{$name}]_virtual_data_%[%{$number_of_virtual_members}] %[%{$name} %+ . %+ %{2}]
		%assign %$number_of_virtual_members %{$number_of_virtual_members} + 1
	%endif
%else
	%error "'virtual' used outside of class"
%endif
%endm

%macro INHERIT_PARENT_DATA 1
%push _INHERIT_PARENT_DATA_
%define %$name %1
%define %$parent %1
%rep %[%{$name}]_parent_count

	%if %[%{$parent}]_has_parent = 1

		%define %$parent %[%{$parent}]_parent

		%if %[%{$parent}]_total_static_methods > 0
			%assign %$ii 0
			%rep %[%{$parent}]_total_static_methods
				. %+ %[ %{$parent} %+ _static_method_ %+ %{$ii} ] RESD 1
				%assign %$ii %{$ii} + 1
			%endrep
		%endif

		%if %[%{$parent}]_total_static_members > 0
			%assign %$ii 0
			%rep %[%{$parent}]_total_static_members
				. %+ %[ %{$parent} %+ _static_data_ %+ %{$ii} ] RESB %[ %{$parent} %+ _static_data_ %+ %{$ii} %+ _size ]
				%assign %$ii %{$ii} + 1
			%endrep
		%endif

	%endif
%endrep
%pop
%endm

%macro BUILD_CLASS_STRUCTURE 1
%push _BUILD_CLASS_STRUCTURE_
%define %$name %1

[ABSOLUTE 0]
%{$name}:
.@%[%{$name}]	resd 1
%if %[%{$name}]_total_virtual_methods > 0
.vmt	resd 1
%endif
%if %[%{$name}]_total_virtual_members > 0
.vdt	resd 1
%endif

INHERIT_PARENT_DATA %{$name}

%if %[%{$name}]_total_static_methods > 0
	%assign %$ii 0
	%rep %[%{$name}]_total_static_methods
		. %+ %[ %{$name} %+ _static_method_ %+ %{$ii} ] RESD 1
		%assign %$ii %{$ii} + 1
	%endrep
%endif

%if %[%{$name}]_total_static_members > 0
	%assign %$ii 0
	%rep %[%{$name}]_total_static_members
		. %+ %[ %{$name} %+ _static_data_ %+ %{$ii} ] RESB %[ %{$name} %+ _static_data_ %+ %{$ii} %+ _size ]
		%assign %$ii %{$ii} + 1
	%endrep
%endif

%{$name}_size equ ($-%{$name})

%pop
%endm

%imacro class 1-2
%push __CLASS__
%define %$name %1
%assign %$number_of_static_methods 0
%assign %$number_of_virtual_methods 0
%assign %$number_of_static_members 0
%assign %$number_of_virtual_members 0
%assign %$has_ctor 0
%assign %$has_dtor 0

%if %0 = 2
	%[%{$name}]_has_parent equ 1
	%define %[%{$name}]_parent %{2}
	%define %$parent %2
	%if %{1}_has_parent > 0
		%[%{$name}]_parent_count equ (%[%{$parent}]_parent_count + 1)
	%else
		%[%{$name}]_parent_count equ 1
	%endif
	%assign %$number_of_virtual_methods %[%{$parent}]_total_virtual_methods
	%assign %$number_of_virtual_members %[%{$parent}]_total_virtual_members
	%assign %$ii 0
	%rep %{$number_of_virtual_methods}
		%define %[%{$name}]_virtual_method_%[%{$ii}]_full %[%{$parent}]_virtual_method_%[%{$ii}]_full
		%define %[%{$name}]_virtual_method_%[%{$ii}]  %[%{$parent}]_virtual_method_%[%{$ii}]
		%assign %$ii %{$ii} + 1
	%endrep
	%assign %$ii 0
	%rep %{$number_of_virtual_members}
		%define %[%{$name}]_virtual_data_%[%{$ii}]_size %[%{$parent}]_virtual_data_%[%{$ii}]_size
		%define %[%{$name}]_virtual_data_%[%{$ii}]  %[%{$parent}]_virtual_data_%[%{$ii}]
		%assign %$ii %{$ii} + 1
	%endrep
%else
	%[%{$name}]_has_parent equ 0
	%[%{$name}]_parent_count equ 0
%endif
%endm

%imacro endclass 0
%[%{$name}]_total_static_methods equ %{$number_of_static_methods}
%[%{$name}]_total_static_members equ %{$number_of_static_members}
%[%{$name}]_total_virtual_methods equ %{$number_of_virtual_methods}
%[%{$name}]_total_virtual_members equ %{$number_of_virtual_members}
%[%{$name}]_has_ctor equ %{$has_ctor}
%[%{$name}]_has_dtor equ %{$has_dtor}

BUILD_CLASS_STRUCTURE %{$name}

%ifdef SINGLE_FILE_BUILD

	[SECTION .data]
	%if %$number_of_virtual_methods > 0
		%{$name}_VMT:
		%assign %$ii 0
		%rep %{$number_of_virtual_methods}
			DD %{$name}_virtual_method_%{$ii}_full
			%assign %$ii %{$ii} + 1
		%endrep
	%endif

	%if %$number_of_virtual_members > 0
		%{$name}_VDT:

		%assign %$ii 0
		%rep %{$number_of_virtual_members}
			DD %{$name}_VDT_values_%{$ii}
			%assign %$ii %{$ii} + 1
		%endrep

	[SECTION .bss]
		%assign %$ii 0
		%rep %{$number_of_virtual_members}
		%{$name}_VDT_values_%{$ii}:
			RESB %{$name}_virtual_data_%{$ii}_size
			%assign %$ii %{$ii} + 1
		%endrep
	
	%endif

	[SECTION .text]
	ALIGN 4
	GLOBAL %{$name}___%{$name}
	%{$name}___%{$name}:
	 ;; Internal Constructor

		push ebp
		mov ebp, esp
		push ecx
		push ebx

			mov ecx, [ebp + 8]
			%if %[%{$name}]_has_dtor = 1
				mov ebx, %{$name}_@%{$name}
			%else
				xor ebx, ebx
			%endif
			mov [ecx + %[%{$name}].@%[%{$name}]], ebx

			%if %[%{$name}]_has_parent = 1
				%if %[%{$parent}]_total_static_methods > 0
					%assign %$ii 0
					%rep %[%{$parent}]_total_static_methods
						mov ebx, dword %[%{$parent}]_static_method_%[%{$ii}]_full
						mov dword [ecx + %[%{$name}].%[%[%{$parent}]_static_method_%[%{$ii}]]], ebx 
						%assign %$ii %{$ii} + 1
					%endrep
				%endif
			%endif

			%if %[%{$name}]_total_static_methods > 0
				%assign %$ii 0
				%rep %[%{$name}]_total_static_methods
					mov ebx, dword %[%{$name}]_static_method_%[%{$ii}]_full
					mov dword [ecx + %[%{$name}].%[%[%{$name}]_static_method_%[%{$ii}]]], ebx 
					%assign %$ii %{$ii} + 1
				%endrep
			%endif
			%if %{$number_of_virtual_methods} > 0
				mov ebx, dword %{$name}_VMT
				mov dword [ecx + %{$name}.vmt], ebx
			%endif
			%if %{$number_of_virtual_members} > 0
				mov ebx, dword %{$name}_VDT
				mov dword [ecx + %{$name}.vdt], ebx
			%endif

		pop ebx
		pop ecx
		leave
		ret 4

	%ifdef __SECT__
	__SECT__
	%endif
%else
	Extern %{$name}___%{$name}
%endif
%pop
%endm

;; by c051n3
%imacro use 1
%push
	; Note: originally import, renamed to avoid conflicts. - Bryant Keller
	%define SINGLE_FILE_BUILD
	%pathsearch %$fname %[%{1}]
	%include %{$fname}
	%undef SINGLE_FILE_BUILD
%pop
%endmacro

%endif ;NASM_OBJECT_MODEL_SUPPORT
