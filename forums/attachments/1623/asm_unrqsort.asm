.586
.model flat,stdcall
option casemap:none

public asm_unrqsort
.code

	; quick-sorts ascending (in order: 5,23,1255,1401,1402,7323...)

asm_unrqsort proc uses eax ebx ecx edx esi edi Arr,count
	local First,Last,StackSize
	mov StackSize,1
		
		mov esi,Arr;
		mov First,0;
		mov eax,count;
		dec eax;
		mov Last,eax;
		align 16
outer_loop:
		mov eax,Last;
		add eax,First;
		shr eax,1;
		mov ebx,[esi+eax*4];
		mov edi,ebx;
		mov ecx,First;
		mov edx,Last;
		align 16
inner_loop:
		cmp [esi+ecx*4],ebx;
		jge wl2;
		inc ecx;
		jmp inner_loop;
		align 16
wl2:
		cmp [esi+edx*4],ebx;
		jle wl2out;
		dec edx;
		jmp wl2;
wl2out:
		cmp ecx,edx;
		jg exit_innerx;
		;------[ swap elements ]-----[
		mov eax, [esi+ecx*4]
		mov ebx, [esi+edx*4]      ; swap elements
		mov [esi+ecx*4], ebx
		mov [esi+edx*4], eax
		;----------------------------/
		mov ebx,edi;
		inc ecx;
		dec edx;
		cmp ecx,edx;
		jle inner_loop;
		align 16
exit_innerx:
		cmp ecx,Last;
		jg iNxt;
		push ecx;
		push Last;
		inc StackSize;
		align 16
iNxt:
		mov ebx,edi;
		mov Last,edx;
		cmp edx,First;
		jg outer_loop;

		dec StackSize
		jz qsOut;
		pop Last;
		pop First;
		mov ebx,edi;
		jmp outer_loop;
qsOut:
	ret
asm_unrqsort endp

end