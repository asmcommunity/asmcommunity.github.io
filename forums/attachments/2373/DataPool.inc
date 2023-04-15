; ==================================================================================================
; Title:   DataPool.inc
; Author:  G. Friedrich
; Version: 1.0.1
; Purpose: Support of DataPool objects for ObjAsm32.
; Notes:   Version 1.0.0, August 2006
;            - First release.
;          Version 1.0.1, May 2007
;            - Shrink added.
;            - Some small optimizations.
; ==================================================================================================


Object DataPool, DataPoolID, Streamable
  RedefineMethod    Done
  RedefineMethod    Init,           Pointer, dword, dword     ;-> Owner object, Item size, Blob size
  StaticMethod      FreeItem,       Pointer
  StaticMethod      NewItem
  StaticMethod      Shrink

  DefineVariable    dBlobSize,      dword,      0
  DefineVariable    dBlobCapacity,  dword,      0
  DefineVariable    dItemSize,      dword,      0
  DefineVariable    pFirstFreeItem, Pointer,    NULL
  DefineVariable    pFirstBlob,     Pointer,    NULL
  DefineVariable    pAddr,          Pointer,    NULL
  DefineVariable    dSpace,         dword,      0
ObjectEnd


; ==================================================================================================


if IMPLEMENT

; ==================================================================================================
;    DataPool implementation
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DataPool.Done
; Purpose:   Finalizes the DataPool object.
; Arguments: None.
; Return:    Nothing.

Method DataPool.Done, uses ebx
    SetObject ecx
    mov ebx, [ecx].pFirstBlob
    jmp @@1    
    align @WordSize
@@0:    
    push MEM_RELEASE
    push 0
    push ebx
    mov ebx, [ebx]
    call VirtualFree
@@1:
    or ebx, ebx
    jnz @@0
    ACall Done
MethodEnd, NONE

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DataPool.Init
; Purpose:   Initializes the DataPool object.
; Arguments: Arg1: -> Owner.
;            Arg2: Item size.
; Return:    Error code.

Method DataPool.Init, uses esi, pOwner:Pointer, dItemSize:dword, dBlobSize:dword
    SetObject esi
    ACall esi.Init, pOwner
    .if dItemSize < 4                               ;Must be big enough to allocate a Pointer
      OCall esi.ErrorReport, NULL, DPL_ITEM_TOO_SMALL
    .else
      mov eax, dBlobSize
      sub eax, 4
      .if eax < dItemSize
        OCall esi.ErrorReport, NULL, DPL_BLOB_TOO_SMALL
      .else
        mov ecx, dItemSize
        mov edx, dBlobSize
        mov [esi].dBlobSize, edx
        mov [esi].dItemSize, ecx

        mov eax, [esi].dBlobSize
        sub eax, 4
        xor edx, edx
        div ecx
        mov [esi].dBlobCapacity, eax

        xor eax, eax
        mov [esi].pFirstFreeItem, eax
        mov [esi].pFirstBlob, eax

      .endif
    .endif
MethodEnd, eax

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DataPool.NewItem
; Purpose:   Returns a pointer to a free memory item.
; Arguments: None.
; Return:    -> Item or NULL if failed.

Method DataPool.NewItem, NOFRAME
    SetObject edx,, [esp + 4]
    mov eax, [edx].pFirstFreeItem
    or eax, eax                                     ;Have we more free items?
    jz @F
    mov ecx, [eax]                                  ;Get a memory block from the "free linked list"
    mov [edx].pFirstFreeItem, ecx                   ;Update the pointer the the next free item
    ret 4
@@:
    mov ecx, [edx].dItemSize
    cmp ecx ,[edx].dSpace                           ;Have we enough free space?
    ja @F                                           ;Unsigned compare
    mov eax, [edx].pAddr                            ;Get a memory block from top of the stack
    add [edx].pAddr, ecx                            ;Update available space and pointer information
    sub [edx].dSpace, ecx
    ret 4
@@:
    invoke VirtualAlloc, NULL, [edx].dBlobSize, MEM_COMMIT, PAGE_READWRITE
    or eax, eax                                     ;Succeeded?
    jz @F

    ;Update the linked list and return the first memory block
    mov edx, [esp + 4]                              ;Get DataPool object pointer
    mov ecx, [edx].pFirstBlob                       ;Updated linked lists
    mov [eax], ecx                                  ;Link old blob to new blob
    mov [edx].pFirstBlob, eax                       ;Link new blob
    add eax, 4                                      ;Return first memory block
    mov ecx, [edx].dItemSize                        ;Update available space and pointer information
    add ecx, eax
    mov [edx].pAddr, ecx
    mov ecx, [edx].dBlobSize
    sub ecx, 4
    sub ecx, [edx].dItemSize
    mov [edx].dSpace, ecx
    ret 4
@@:
    OCall Pointer ptr [esp + 12]::DataPool.ErrorReport, NULL, DPL_OUT_OF_MEMORY
    xor eax, eax                                    ;Return NULL
MethodEnd 4, eax

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DataPool.FreeItem
; Purpose:   Frees a memory block.
; Arguments: -> Item.
; Return:    Nothing.

Method DataPool.FreeItem, NOFRAME, pItem:Pointer
    SetObject ecx,, [esp + 4]
    mov eax, [esp + 8]                              ;pItem
    mov edx, [ecx].pFirstFreeItem                   ;Update linked lists
    mov [eax], edx
    mov [ecx].pFirstFreeItem, eax
MethodEnd 8, NONE

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DataPool.Shrink
; Purpose:   Reduces the DataPool commited memory freeing unused blobs.
; Arguments: None.
; Return:    Nothing.

DPL_WorkTableEntry struc
  pNextEntry  Pointer ?         ;Must be the first member
  dBase       dword   ?
  dTop        dword   ?
  dCount      dword   ?
  pLinkFirst  Pointer ?
DPL_WorkTableEntry ends

Method DataPool.Shrink, uses ebx edi esi
    local dUsedBlobCount:dword, pUsedBlobs:Pointer, pEmptyBlobs:Pointer
    local pTable:Pointer, dTableEntries:dword

    SetObject esi
    xor edx, edx
    mov pEmptyBlobs, edx
    mov pUsedBlobs, edx

    ;Setup a table with the current info of all blobs
    ;First determine the necessary amount of memory to hold the table
    mov ecx, [esi].pFirstBlob
    xor eax, eax
    jmp @@01
    align @WordSize
@@00:
    mov ecx, [ecx]                                      ;Move to next blob
    inc eax
@@01:
    or ecx, ecx
    jnz @@00

    mov dUsedBlobCount, eax
    mov dTableEntries, eax

    mov ecx, sizeof DPL_WorkTableEntry
    mul ecx
    ;Allocate the work table
    invoke VirtualAlloc, NULL, eax, MEM_COMMIT, PAGE_READWRITE
    .if eax != 0
       mov pTable, eax

      ;Now, fill the table
      mov ecx, [esi].pFirstBlob
      jmp @@11
      align @WordSize
@@10:
      mov edx, pUsedBlobs
      mov pUsedBlobs, eax
      mov [eax].DPL_WorkTableEntry.pNextEntry, edx

      mov [eax].DPL_WorkTableEntry.dBase, ecx
;      mov [eax].DPL_WorkTableEntry.pLinkFirst, NULL       ;VirtualAlloc zeroes the memory pages!
      mov edx, ecx
      add edx, [esi].dBlobSize
      mov [eax].DPL_WorkTableEntry.dTop, edx
      m2m [eax].DPL_WorkTableEntry.dCount, [esi].dBlobCapacity, ebx
      mov ecx, [ecx]                                    ;Move to next blob
      add eax, sizeof DPL_WorkTableEntry
@@11:
      or ecx, ecx
      jnz @@10

      .if [esi].pFirstBlob != NULL
        ;The first entry (if any) may have different settings
        mov eax, pTable
        mov ecx, [esi].pFirstBlob
        mov edx, [esi].pAddr
        mov [eax].DPL_WorkTableEntry.dTop, edx
        mov eax, edx
        sub eax, ecx
        sub eax, 4
        xor edx, edx
        div [esi].dItemSize
        mov edx, pTable
        mov [edx].DPL_WorkTableEntry.dCount, eax
      .endif

      ;Main loop: process the empty linked list updating the work table
      mov ebx, [esi].pFirstFreeItem
      ReleaseObject esi
      jmp @@21
      align @WordSize
@@20:
      lea edi, pUsedBlobs
      align @WordSize
      .repeat
        mov esi, edi                                    ;Remember previous entry in esi
        mov eax, ebx
        mov edi, [edi].DPL_WorkTableEntry.pNextEntry
        IsInRange eax, dword ptr [edi].DPL_WorkTableEntry.dBase, dword ptr [edi].DPL_WorkTableEntry.dTop
      .until Zero?                              ;0 if it is in range, else -1; Flags set according
      mov edx, [edi].DPL_WorkTableEntry.pLinkFirst
      mov [edi].DPL_WorkTableEntry.pLinkFirst, ebx
      mov ecx, Pointer ptr [ebx]
      mov Pointer ptr [ebx], edx
      dec [edi].DPL_WorkTableEntry.dCount
      mov ebx, ecx
      .if Zero?
        ;We move the entry from the used list to the empty list
        mov ecx, [edi].DPL_WorkTableEntry.pNextEntry    ;Get the pointer to the next entry
        mov [esi], ecx                                  ;Save it in the previous entry
        mov edx, pEmptyBlobs
        mov [edi], edx
        mov pEmptyBlobs, edi
      .endif
@@21:
      or ebx, ebx
      jnz @@20

      ;Relink the linked list of used blobs
      SetObject esi
      mov edi, pUsedBlobs
      lea eax, [esi].pFirstBlob
      lea edx, [esi].pFirstFreeItem
      jmp @@31
      align @WordSize
@@30:
      mov ecx, [edi].DPL_WorkTableEntry.dBase
      mov [eax], ecx
      mov eax, ecx

      mov ecx, [edi].DPL_WorkTableEntry.pLinkFirst
      .if ecx != NULL
        mov [edx], ecx
        jmp @@41                                        ;Move to the end of the local linked list
        align @WordSize
@@40:
        mov edx, ecx
        mov ecx, [ecx]                                  ;Move to next item
@@41:
        or ecx, ecx
        jnz @@40
      .endif

      mov edi, [edi]                                    ;Move to next entry
@@31:
      or edi, edi
      jnz @@30
      mov Pointer ptr [eax], NULL
      mov Pointer ptr [edx], NULL                       ;JustiIn case pFirstFreeItem == NULL

      ;Free the unused blobs
      mov edi, pEmptyBlobs
      mov ebx, [esi].dSpace
      mov esi, [esi].pAddr
      ReleaseObject esi
      jmp @@51
      align @WordSize
@@50:
      invoke VirtualFree, [edi].DPL_WorkTableEntry.dBase, 0, MEM_RELEASE
      mov eax, [edi].DPL_WorkTableEntry.dTop
      cmp eax, esi
      sbb eax, eax
      and ebx, eax                                      ;if eax == [esi].pAddr => [esi].dSpace = 0
      mov edi, [edi]                                    ;Move to next entry
@@51:
      or edi, edi
      jnz @@50
      SetObject esi
      mov [esi].dSpace, ebx

      ;Dispose the work table
      invoke VirtualFree, pTable, 0, MEM_RELEASE

    .else
      OCall esi.ErrorReport, NULL, DPL_OUT_OF_MEMORY
    .endif
MethodEnd

endif
