.586
.model flat,stdcall
option casemap:none

public HEAP1
public malloc_func
public free_func
public realloc_func

HEAP_ZERO_MEMORY equ 00000008h      

.data
	HEAP1 dd 0
.code

HeapCreate proto :DWORD,:DWORD,:DWORD
HeapAlloc proto :DWORD,:DWORD,:DWORD
HeapFree proto :DWORD,:DWORD,:DWORD
HeapReAlloc proto :DWORD,:DWORD,:DWORD,:DWORD

malloc_func proc PUBLIC uses ebx ecx edx esi edi How
	.if !HEAP1
		invoke HeapCreate,0,10000,0
		mov HEAP1,eax
	.endif
	invoke HeapAlloc,HEAP1,HEAP_ZERO_MEMORY,How	
	ret
malloc_func endp
	
free_func proc PUBLIC uses eax ebx ecx edx esi edi What
	invoke HeapFree,HEAP1,0,What
	ret
free_func endp

realloc_func proc PUBLIC uses ebx ecx edx esi edi What,How
	.if !HEAP1
		invoke HeapCreate,0,10000,0
		mov HEAP1,eax
	.endif
	.if !What
		.if How
			invoke HeapAlloc,HEAP1,HEAP_ZERO_MEMORY,How
		.else
			xor eax,eax
		.endif
	.else
		.if How
			invoke HeapReAlloc,HEAP1,HEAP_ZERO_MEMORY,What,How
		.else
			invoke HeapFree,HEAP1,0,What
			xor eax,eax
		.endif
	.endif
	ret
realloc_func endp



end