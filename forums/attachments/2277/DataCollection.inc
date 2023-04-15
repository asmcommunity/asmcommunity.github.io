; =================================================================================================
; Title:   DataCollection.inc
; Author:  G. Friedrich
; Version: 1.0.1
; Purpose: Windows primer object for ObjAsm32.
; Notes:   DataCollection is a simple derivative of Collection. It implements a collection of data
;          structures instead of objects.
;          Version 1.0.0, September 2002
;            - First release.
;          Version 1.0.1, January 2007
;            - GetItem and PutItem redefined. These are general purpose methods that can be
;              optimized if the data size is known.
; =================================================================================================


Object DataCollection, DataCollectionID, Collection
  RedefineMethod    DestroyItem,  Pointer
  RedefineMethod    GetItem,      Pointer
  RedefineMethod    PutItem,      Pointer, Pointer
ObjectEnd

; =================================================================================================

if IMPLEMENT

; —————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DataCollection.DestroyItem
; Purpose:   Auxiliar function to dispose a item from the collection. Don't call directly!
; Arguments: -> item.
; Return:    Nothing.

Method DataCollection.DestroyItem,, pItem:Pointer
    .if pItem != NULL
      MemFree pItem                ;Frees data from heap
    .endif
MethodEnd, NONE

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DataCollection.GetItem
; Purpose:   Loads an item from a stream object.
; Arguments: Stream object containing an object.
; Return:    -> new loaded data.

Method DataCollection.GetItem,, pStream:Pointer
    local dSize:dword

    OCall pStream::Stream.BinRead, addr dSize, 4
    MemAlloc dSize
    push eax
    .if eax != NULL
      OCall pStream::Stream.BinRead, eax, dSize
    .else
      OCall ErrorReport, NULL, COL_OUTOFMEMORY
    .endif
    pop eax
MethodEnd, eax

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DataCollection.PutItem
; Purpose:   Stores an item of the collection in a stream object.
; Arguments: Arg1: -> steam.
;            Arg2: -> item.
; Return:    Nothing.

Method DataCollection.PutItem,, pStream:Pointer, pItem:Pointer
    local dSize:dword

    invoke HeapSize, hProcessHeap, 0, pItem
    mov dSize, eax 
    OCall pStream::Stream.BinWrite, addr dSize, 4
    OCall pStream::Stream.BinWrite, pItem, dSize
MethodEnd, NONE


endif