
;DataCollection object updated by Homer
;New : MemSize and $MemSize macros (obtain size of MemAlloc'd object)
;New : Load and Store methods 

MemSize macro pMem
	invoke HeapSize,hProcessHeap,NULL,pMem
endm

$MemSize macro pMem
	MemSize pMem
	exitm <eax>
endm

Object DataCollection, DataCollectionID, Collection
  RedefineMethod    DestroyItem,  Pointer
  RedefineMethod	Load, Pointer, Pointer ;pStream, pOwner
  RedefineMethod	Store, Pointer		   ;pStream
ObjectEnd


; =================================================================================================

if IMPLEMENT

Method DataCollection.DestroyItem,, pItem:Pointer
    .if pItem != NULL
      MemFree pItem                ;Frees data from heap
    .endif
MethodEnd

Method DataCollection.Store,uses esi, pStream
LOCAL dSize
	SetObject esi
    OCall pStream::Stream.BinWrite, addr [esi].dCount, 2 * 4	 ;Count, Limit
    OCall pStream::Stream.BinWrite, addr [esi].dDelta , 2 * 4    ;Delta, MaxSize

	xor ecx,ecx
	.while ecx<[esi].dCount
		push ecx
		;
		shl ecx,2
		add ecx,[esi].pItems
		mov ecx,[ecx]
		push ecx
		;
		mov dSize,$MemSize(ecx)
		OCall pStream::Stream.BinWrite,addr dSize,4
		;
		pop ecx
		OCall pStream::Stream.BinWrite,ecx,dSize
		;
		pop ecx
		inc ecx
	.endw	
MethodEnd

Method DataCollection.Load,uses esi, pStream, pOwner
LOCAL dCount,dSize
	SetObject esi
    OCall pStream::Stream.BinRead, addr dCount,4
    OCall pStream::Stream.BinRead, addr [esi].dLimit, 4
    OCall pStream::Stream.BinRead, addr [esi].dDelta , 2 * 4    ;Delta, MaxSize
	
	xor ecx,ecx
	.while ecx<dCount
		push ecx
		OCall pStream::Stream.BinRead,addr dSize,4
		OCall Insert,$MemAlloc(dSize)
		OCall pStream::Stream.BinRead,eax,dSize
		pop ecx
		inc ecx
	.endw
MethodEnd

endif