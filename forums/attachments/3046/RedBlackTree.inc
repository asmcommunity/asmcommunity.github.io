; ==================================================================================================
; Title:   RedBlackTree.inc
; Author:  Homer, G. Friedrich
; Version: 1.0.0
; Purpose: ObjAsm32 Support of red-black tree objects.
; Links:   http://www.cs.iupui.edu/~xkzou/teaching/CS580/Red-black_trees.ppt
;          http://en.wikipedia.org/wiki/Red-black_tree
; Notes:   - A red-black tree is a binary search tree where each node has a color attribute, the
;            value of which is either red or black. In addition to the ordinary requirements imposed
;            on binary search trees, the following additional requirements apply to red-black trees:
;            - A node is either red or black.
;            - The root is black. (This rule is used in some definitions and not others. Since the
;              root can always be changed from red to black but not necessarily vice-versa this rule
;              has little effect on analysis.)
;            - All leaves are black.
;            - Both children of every red node are black.
;            - Every simple path from a given node to any of its descendant leaves contains the same
;              number of black nodes.
;          - The keys used to compare the nodes must be unique. This implementation does NOT support
;            duplicate keys yielding to an error if such a node is tried to insert.
;          - In case of duplicate keys, a more complex key must be build to guarantee key 
;            uniqueness.
;          - Nodes can be embedded into other structures or objects. The RB-Tree can handle this
;            situation using a node offset which is the number of bytes needed to add from the
;            hosting structure/object to the RBT_NODE.
;
; Notes:   Version 1.0.0, December 2009
;            - First release.
; ==================================================================================================
;
; Typical tree node structure
;
;                                     NULL
;                                      |
;                                      |
;                                     ———
;                                    | 4 |     Root node (always black)
;                                     ———
;                                   /     \
;                                  /       \
;                                 /         \
;                           / ...             ... \
;             Parent Node  /                       \
;                         /                         \
;                       ———                         ———
;   Node with Key = 2  | 2 |                       | 6 |
;                       ———                         ———
;                     /     \                     /     \
;         Left Node  /       \  Right Node       /       \
;                   /         \                 /         \
;                  /           \               /           \
;                ———           ———           ———           ———
;               | 1 |         | 3 |         | 5 |         | 7 |
;                ———           ———           ———           ———
;              /     \       /     \       /     \       /     \
;             /       \     /       \     /       \     /       \
;            NIL     NIL   NIL     NIL   NIL     NIL   NIL     NIL    Leave nodes (always black)
;                                                                     = NIL-Node



scRED   equ 0
scBLACK equ 1


RBT_NODE struc
  pLeftNode   Pointer   ?
  pRightNode  Pointer   ?
  pParentNode Pointer   ?
  dColor      dword     ?
RBT_NODE ends
PRBT_NODE typedef ptr RBT_NODE


; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Object:    RedBlackTree
; Purpose:   Implementes a Red-Black binaray tree.
; Notes:     - RedBlackTree and descendants are thread safe using the OA32 multithreading support.
;            -

Object RedBlackTree, RedBlackTreeID, Streamable
  DynamicMethod     Compare,        Pointer, Pointer
  VirtualMethod     Delete,         Pointer
  VirtualMethod     DeleteAll
  VirtualMethod     Dispose,        Pointer
  VirtualMethod     DisposeAll
  VirtualMethod     Find,           Pointer
  VirtualMethod     FirstThat,      Pointer, ?          ;-> Proc, Parameters
  VirtualMethod     FirstThatNot,   Pointer, ?          ;-> Proc, Parameters
  VirtualMethod     ForEach,        Pointer, ?          ;-> Proc, Parameters
  VirtualMethod     ForEachRev,     Pointer, ?          ;-> Proc, Parameters
  VirtualMethod     GetFirst
  VirtualMethod     GetLast
  VirtualMethod     GetNext,        Pointer
  VirtualMethod     GetPrev,        Pointer
  RedefineMethod    Init,           Pointer, dword
  VirtualMethod     InitNode,       PRBT_NODE
  VirtualMethod     Insert,         Pointer
  VirtualMethod     LastThat,       Pointer, ?          ;-> Proc, Parameters
  VirtualMethod     LastThatNot,    Pointer, ?          ;-> Proc, Parameters

  DynamicMethod     GetItem,        PTP_Stream          ;-> Stream
  DynamicMethod     PutItem,        PTP_Stream, Pointer ;-> Stream, -> Item
  RedefineMethod    Load,           PTP_Stream, Pointer ;-> Stream, -> Owner
  RedefineMethod    Store,          PTP_Stream          ;-> Stream

  DefineVariable    dNodeOffset,    dword,        0
  DefineVariable    pRootNode,      PRBT_NODE,    NULL
  DefineVariable    NilNode,        RBT_NODE,     {}
  DefineVariable    pNilNode,       PRBT_NODE,    NULL  ;-> NilNode
  DefineVariable    dCount,         dword,        0

  DefineVariable    ObjLock,        ObjectLock,   {}    ;Locking structure for multithreaded access

ObjectEnd


.code
; ==================================================================================================

if IMPLEMENT

; ==================================================================================================
;  Utility routines
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Procedure: RedBlackTree_Show
; Purpose:   Debug procedure to display the tree structure.
; Arguments: Arg1: Key offset into the object.
; Return:    None.

CStr DestWindow, "Red-Black Tree structure"

ShowRBSubtree proc uses ebx edi pNode:PRBT_NODE, dIndent:dword      ;esi and edi are passed from
    local bIndBuffer[100]:byte, bNumBuffer[20]:byte, dColor:dword   ;  RedBlackTree_Show proc

    mov ebx, pNode                                  ;pNode
    lea eax, bIndBuffer
    xor ecx, ecx
    .while ecx < dIndent
      mov byte ptr [eax], " "
      inc ecx
      inc eax
    .endw
    and byte ptr [eax], 0

    .if [ebx].RBT_NODE.dColor == scRED
      mov dColor, $RGB(255,0,0)
    .elseif [ebx].RBT_NODE.dColor == scBLACK
      mov dColor, $RGB(0,0,0)
    .else
      mov dColor, $RGB(0,0,255)
    .endif

    invoke DbgOutText, addr bIndBuffer, dColor, DBG_EFFECT_NORMAL, offset DestWindow
    @invoke DbgOutText, "Object at ", dColor, DBG_EFFECT_NORMAL, offset DestWindow
    mov edx, ebx
    sub edx, [esi].RedBlackTree.dNodeOffset
    invoke dword2hex, addr bNumBuffer, edx
    invoke DbgOutText, addr bNumBuffer, dColor, DBG_EFFECT_NORMAL, offset DestWindow
    @invoke DbgOutText, " =\] ", dColor, DBG_EFFECT_NORMAL, offset DestWindow
    mov edx, ebx
    sub edx, [esi].RedBlackTree.dNodeOffset
    add edx, edi
    invoke dword2dec, addr bNumBuffer, [edx]
    invoke DbgOutText, addr bNumBuffer, dColor, DBG_EFFECT_NORMAL, offset DestWindow
    invoke DbgOutText, offset szCRLF, dColor, DBG_EFFECT_NORMAL, offset DestWindow

    add dIndent, 3

    .if ebx != [esi].RedBlackTree.pNilNode
      mov eax, [ebx].RBT_NODE.pLeftNode
      .if eax != [esi].RedBlackTree.pNilNode
        invoke ShowRBSubtree, eax, dIndent
      .else
        invoke DbgOutText, addr bIndBuffer, $RGB(0,0,0), DBG_EFFECT_NORMAL, offset DestWindow
        @invoke DbgOutText, "   Nil", $RGB(0,0,0), DBG_EFFECT_NORMAL, offset DestWindow
        invoke DbgOutText, offset szCRLF, $RGB(0,0,0), DBG_EFFECT_NORMAL, offset DestWindow
      .endif
      mov eax, [ebx].RBT_NODE.pRightNode
      .if eax != [esi].RedBlackTree.pNilNode
        invoke ShowRBSubtree, eax, dIndent
      .else
        invoke DbgOutText, addr bIndBuffer, $RGB(0,0,0), DBG_EFFECT_NORMAL, offset DestWindow
        @invoke DbgOutText, "   Nil", $RGB(0,0,0), DBG_EFFECT_NORMAL, offset DestWindow
        invoke DbgOutText, offset szCRLF, $RGB(0,0,0), DBG_EFFECT_NORMAL, offset DestWindow
      .endif
    .endif

    ret
ShowRBSubtree endp

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

RedBlackTree_Show proc Self:PTP_RedBlackTree, dKeyOffset:dword
    pusha
    SetObject esi, RedBlackTree, [esp + 36]             ;esi = Self, passed to ShowRBSubtree
    mov edi, [esp + 40]                                 ;edi = dKeyOffset, passed to ShowRBSubtree
    .if [esi].dCount != 0
      invoke ShowRBSubtree, [esi].pRootNode, 0
    .else
      DbgText "Tree is empty", offset DestWindow
    .endif
    DbgLine offset DestWindow
    popa
    ret
RedBlackTree_Show endp

OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Procedure: RedBlackTree_FindMin
; Purpose:   Searches for the node with the smalest key starting from the specified node.
; Arguments: Arg1: -> Start node.
; Return:    eax -> Node.
; Note:      esi is passed to this proc and is not changed!

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

RedBlackTree_FindMin proc pNode:PRBT_NODE
    mov eax, [esp + 4]                                  ;pNode
    mov edx, [esi].RedBlackTree.pNilNode
    .repeat
      mov ecx, [eax].RBT_NODE.pLeftNode
      .break .if ecx == edx
      mov eax, ecx
    .until FALSE
    ret 4
RedBlackTree_FindMin endp

OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Procedure: RedBlackTree.FindMax
; Purpose:   Searches for the node with the biggest key starting from the specified node.
; Arguments: Arg1: -> Start node.
; Return:    eax -> Node.
; Note:      esi is passed to this proc and is not changed!

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

RedBlackTree_FindMax proc pNode:PRBT_NODE
    mov eax, [esp + 4]                                  ;pNode
    mov edx, [esi].RedBlackTree.pNilNode
    .repeat
      mov ecx, [eax].RBT_NODE.pRightNode
      .break .if ecx == edx
      mov eax, ecx
    .until FALSE
    ret 4
RedBlackTree_FindMax endp

OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Procedure: RedBlackTree_RotateLeft
; Purpose:   Rotates the tree to the left around a specified node.
; Arguments: Arg1: -> Node.
; Return:    eax -> Node.
; Note:      esi is passed to this proc and is not changed!

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

RedBlackTree_RotateLeft proc pNode:PRBT_NODE
    mov eax, [esp + 4]                                  ;pNode
    mov ecx, [eax].RBT_NODE.pRightNode
    m2m [eax].RBT_NODE.pRightNode, [ecx].RBT_NODE.pLeftNode, edx
    .if edx != [esi].RedBlackTree.pNilNode
      mov [edx].RBT_NODE.pParentNode, eax
    .endif

    mov edx, [eax].RBT_NODE.pParentNode
    .if ecx != [esi].RedBlackTree.pNilNode
      mov [ecx].RBT_NODE.pParentNode, edx
    .endif

    .if edx != NULL
      .if eax == [edx].RBT_NODE.pLeftNode
         mov [edx].RBT_NODE.pLeftNode, ecx
      .else
         mov [edx].RBT_NODE.pRightNode, ecx
      .endif
    .else
      mov [esi].RedBlackTree.pRootNode, ecx
    .endif

    mov [ecx].RBT_NODE.pLeftNode, eax
    .if eax != [esi].RedBlackTree.pNilNode
      mov [eax].RBT_NODE.pParentNode, ecx
    .endif
    ret 4
RedBlackTree_RotateLeft endp

OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Procedure: RedBlackTree_RotateRight
; Purpose:   Rotates the tree to the right around a specified node.
; Arguments: Arg1: -> Node.
; Return:    eax -> Node.
; Note:      esi is passed to this proc and is not changed!

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

RedBlackTree_RotateRight proc pNode:PRBT_NODE
    mov eax, [esp + 4]                                  ;pNode
    mov ecx, [eax].RBT_NODE.pLeftNode

    m2m [eax].RBT_NODE.pLeftNode, [ecx].RBT_NODE.pRightNode, edx
    .if edx != [esi].RedBlackTree.pNilNode
      mov [edx].RBT_NODE.pParentNode, eax
    .endif

    mov edx, [eax].RBT_NODE.pParentNode
    .if ecx != [esi].RedBlackTree.pNilNode
      mov [ecx].RBT_NODE.pParentNode, edx
    .endif

    .if edx != NULL
      .if eax == [edx].RBT_NODE.pRightNode
        mov [edx].RBT_NODE.pRightNode, ecx
      .else
        mov [edx].RBT_NODE.pLeftNode, ecx
      .endif
    .else
      mov [esi].RedBlackTree.pRootNode, ecx
    .endif

    mov [ecx].RBT_NODE.pRightNode, eax
    .if eax != [esi].RedBlackTree.pNilNode
      mov [eax].RBT_NODE.pParentNode, ecx
    .endif
    ret 4
RedBlackTree_RotateRight endp

OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Procedure: RedBlackTree_NodeInsert
; Purpose:   Performs a node insertion.
; Arguments: Arg1: -> Node.
; Return:    eax = Error code.
; Note:      esi is passed to this proc and is not changed!

RedBlackTree_NodeInsert proc pNode:PRBT_NODE
    local pParentNode:PRBT_NODE, dResult:dword

    push ebx
    mov ebx, [esi].pRootNode
    mov pParentNode, NULL

    .while ebx != [esi].RedBlackTree.pNilNode
      mov eax, pNode
      sub eax, [esi].RedBlackTree.dNodeOffset
      mov edx, ebx
      sub edx, [esi].RedBlackTree.dNodeOffset
      mov dResult, $OCall(esi::RedBlackTree.Compare, eax, edx)
      .if eax == 0
        OCall esi::RedBlackTree.ErrorReport, NULL, RBT_NON_UNIQUE_KEY
        jmp @@Exit
      .endif
      mov pParentNode, ebx
      .ifBitSet eax, BIT31
        mov ebx, [ebx].RBT_NODE.pLeftNode
      .else
        mov ebx, [ebx].RBT_NODE.pRightNode
      .endif
    .endw

    mov eax, pNode
    .if pParentNode != NULL
      m2m [eax].RBT_NODE.pParentNode, pParentNode, edx
      mov edx, pParentNode
      .ifBitSet dResult, BIT31
        mov [edx].RBT_NODE.pLeftNode, eax
      .else
        mov [edx].RBT_NODE.pRightNode, eax
      .endif
    .else
      mov [esi].RedBlackTree.pRootNode, eax
      mov [eax].RBT_NODE.dColor, scBLACK
    .endif

    ;Fixup insertion
    mov edx, [eax].RBT_NODE.pParentNode
    .while eax != [esi].RedBlackTree.pRootNode && [edx].RBT_NODE.dColor == scRED
      mov edx, [eax].RBT_NODE.pParentNode
      mov ecx, [edx].RBT_NODE.pParentNode
      .if edx == [ecx].RBT_NODE.pLeftNode
        mov edx, [edx].RBT_NODE.pParentNode
        mov ecx, [edx].RBT_NODE.pRightNode
        .if [ecx].RBT_NODE.dColor == scRED
          mov edx, [eax].RBT_NODE.pParentNode
          mov [edx].RBT_NODE.dColor, scBLACK
          mov [ecx].RBT_NODE.dColor, scBLACK
          mov eax, [edx].RBT_NODE.pParentNode
          mov [eax].RBT_NODE.dColor, scRED
        .else
          mov edx, [eax].RBT_NODE.pParentNode
          .if eax == [edx].RBT_NODE.pRightNode
            invoke RedBlackTree_RotateLeft, edx         ;eax = edx
          .endif
          mov edx, [eax].RBT_NODE.pParentNode
          mov [edx].RBT_NODE.dColor, scBLACK
          mov edx, [edx].RBT_NODE.pParentNode
          mov [edx].RBT_NODE.dColor, scRED
          invoke RedBlackTree_RotateRight, edx          ;eax = edx
        .endif
      .else
        mov edx, [edx].RBT_NODE.pParentNode
        mov ecx, [edx].RBT_NODE.pLeftNode
        .if [ecx].RBT_NODE.dColor == scRED
          mov edx, [eax].RBT_NODE.pParentNode
          mov [edx].RBT_NODE.dColor, scBLACK
          mov [ecx].RBT_NODE.dColor, scBLACK
          mov eax, [edx].RBT_NODE.pParentNode
          mov [eax].RBT_NODE.dColor, scRED
        .else
          mov edx, [eax].RBT_NODE.pParentNode
          .if eax == [edx].RBT_NODE.pLeftNode
            invoke RedBlackTree_RotateRight, edx        ;eax = edx
          .endif
          mov edx, [eax].RBT_NODE.pParentNode
          mov [edx].RBT_NODE.dColor, scBLACK
          mov edx, [edx].RBT_NODE.pParentNode
          mov [edx].RBT_NODE.dColor, scRED
          invoke RedBlackTree_RotateLeft, edx           ;eax = edx
        .endif
      .endif

      mov edx, [eax].RBT_NODE.pParentNode
    .endw

    mov eax, [esi].RedBlackTree.pRootNode
    mov [eax].RBT_NODE.dColor, scBLACK
    xor eax, eax                                        ;eax = OBJ_OK

@@Exit:
    pop ebx
    ret 4
RedBlackTree_NodeInsert endp

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Procedure: RedBlackTree_NodeDelete
; Purpose:   Performs a node deletion.
; Arguments: Arg1: -> Node.
; Return:    None.
; Note:      esi is passed to this proc and is not changed!

RedBlackTree_NodeDelete proc pNode:PRBT_NODE
    local pAuxNode:PRBT_NODE

    push ebx
    mov eax, pNode
    mov edx, [eax].RBT_NODE.pLeftNode
    mov ecx, [eax].RBT_NODE.pRightNode
    .if edx != [esi].RedBlackTree.pNilNode && ecx != [esi].RedBlackTree.pNilNode
      invoke RedBlackTree_FindMin, ecx
    .endif

    mov edx, [eax].RBT_NODE.pLeftNode
    .if edx == [esi].RedBlackTree.pNilNode
      mov edx, [eax].RBT_NODE.pRightNode
    .endif
    mov pAuxNode, edx

    m2m [edx].RBT_NODE.pParentNode, [eax].RBT_NODE.pParentNode, ecx
    .if ecx != NULL
      .if eax == [ecx].RBT_NODE.pLeftNode
        mov [ecx].RBT_NODE.pLeftNode, edx
      .else
        mov [ecx].RBT_NODE.pRightNode, edx
      .endif
    .else
      mov [esi].RedBlackTree.pRootNode, edx
    .endif

    mov ebx, [eax].RBT_NODE.dColor

    .if eax != pNode
      ;Replace node
      mov edx, pNode
      m2m [eax].RBT_NODE.dColor, [edx].RBT_NODE.dColor, ecx
      m2m [eax].RBT_NODE.pLeftNode, [edx].RBT_NODE.pLeftNode, ecx
      m2m [eax].RBT_NODE.pRightNode, [edx].RBT_NODE.pRightNode, ecx
      m2m [eax].RBT_NODE.pParentNode, [edx].RBT_NODE.pParentNode, ecx

      .if [eax].RBT_NODE.pParentNode != NULL
        mov ecx, [eax].RBT_NODE.pParentNode
        .if [ecx].RBT_NODE.pLeftNode == edx
          mov [ecx].RBT_NODE.pLeftNode, eax
        .else
          mov [ecx].RBT_NODE.pRightNode, eax
        .endif
      .else
        mov [esi].RedBlackTree.pRootNode, eax
      .endif

      mov edx, [eax].RBT_NODE.pLeftNode
      mov [edx].RBT_NODE.pParentNode, eax
      mov edx, [eax].RBT_NODE.pRightNode
      mov [edx].RBT_NODE.pParentNode, eax
    .endif

    .if ebx == scBLACK                                  ;Check if the tree is balanced
      ;Fixup deletion
      mov eax, pAuxNode
      .while eax != [esi].RedBlackTree.pRootNode && [eax].RBT_NODE.dColor == scBLACK
        mov edx, [eax].RBT_NODE.pParentNode
        .if eax == [edx].RBT_NODE.pLeftNode
          mov ebx, [edx].RBT_NODE.pRightNode
          .if [ebx].RBT_NODE.dColor == scRED
            mov [ebx].RBT_NODE.dColor, scBLACK
            mov edx, [eax].RBT_NODE.pParentNode
            mov [edx].RBT_NODE.dColor, scRED
            invoke RedBlackTree_RotateLeft, edx
            mov eax, pAuxNode
            mov edx, [eax].RBT_NODE.pParentNode
            mov ebx, [edx].RBT_NODE.pRightNode
          .endif

          mov edx, [ebx].RBT_NODE.pLeftNode
          mov ecx, [ebx].RBT_NODE.pRightNode
          .if [edx].RBT_NODE.dColor == scBLACK && [ecx].RBT_NODE.dColor == scBLACK
            mov [ebx].RBT_NODE.dColor, scRED
            mov eax, [eax].RBT_NODE.pParentNode
          .else
            mov edx, [ebx].RBT_NODE.pRightNode
            .if [edx].RBT_NODE.dColor == scBLACK
              mov edx, [ebx].RBT_NODE.pLeftNode
              mov [edx].RBT_NODE.dColor, scBLACK
              mov [ebx].RBT_NODE.dColor, scRED
              invoke RedBlackTree_RotateRight, ebx
              mov eax, pAuxNode
              mov edx, [eax].RBT_NODE.pParentNode
              mov ebx, [edx].RBT_NODE.pRightNode
            .endif
            mov edx, [eax].RBT_NODE.pParentNode
            m2m [ebx].RBT_NODE.dColor, [edx].RBT_NODE.dColor, ecx
            mov [edx].RBT_NODE.dColor, scBLACK
            mov ecx, [ebx].RBT_NODE.pRightNode
            mov [ecx].RBT_NODE.dColor, scBLACK
            invoke RedBlackTree_RotateLeft, edx
            mov eax, [esi].RedBlackTree.pRootNode
          .endif
        .else
          mov edx, [eax].RBT_NODE.pParentNode
          mov ebx, [edx].RBT_NODE.pLeftNode
          .if [ebx].RBT_NODE.dColor == scRED
            mov [ebx].RBT_NODE.dColor, scBLACK
            mov edx, [eax].RBT_NODE.pParentNode
            mov [edx].RBT_NODE.dColor, scRED
            invoke RedBlackTree_RotateRight, edx
            mov eax, pAuxNode
            mov edx, [eax].RBT_NODE.pParentNode
            mov ebx, [edx].RBT_NODE.pLeftNode
          .endif
          mov edx, [ebx].RBT_NODE.pRightNode
          mov ecx, [ebx].RBT_NODE.pLeftNode
          .if [edx].RBT_NODE.dColor == scBLACK && [ecx].RBT_NODE.dColor == scBLACK
            mov [ebx].RBT_NODE.dColor, scRED
            mov eax, [eax].RBT_NODE.pParentNode
          .else
            mov edx, [ebx].RBT_NODE.pLeftNode
            .if [edx].RBT_NODE.dColor == scBLACK
              mov edx, [ebx].RBT_NODE.pRightNode
              mov [edx].RBT_NODE.dColor, scBLACK
              mov [ebx].RBT_NODE.dColor, scRED
              invoke RedBlackTree_RotateLeft, ebx
              mov eax, pAuxNode
              mov edx, [eax].RBT_NODE.pParentNode
              mov ebx, [edx].RBT_NODE.pLeftNode
            .endif
            mov edx, [eax].RBT_NODE.pParentNode
            m2m [ebx].RBT_NODE.dColor, [edx].RBT_NODE.dColor, ecx
            mov [edx].RBT_NODE.dColor, scBLACK
            mov edx, [ebx].RBT_NODE.pLeftNode
            mov [edx].RBT_NODE.dColor, scBLACK
            invoke RedBlackTree_RotateRight, [eax].RBT_NODE.pParentNode
            mov eax, [esi].RedBlackTree.pRootNode
          .endif
        .endif
      .endw
      mov [eax].RBT_NODE.dColor, scBLACK
    .endif
    pop ebx

    ret 4
RedBlackTree_NodeDelete endp

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.Compare
; Purpose:   Compares 2 keys.
; Arguments: Arg1: -> Object #1.
;            Arg2: -> Object #2.
; Return:    eax = Comparison result. Negative if Key2 > Key1,
;                  zero if equal and positive if Key1 > Key2.
; Note:      This method MUST be redefined or overridden to provide the correct comparison mechanism
;            for each case. Ascending or descending order can be achieved inverting the sign of the
;            result.

Method RedBlackTree.Compare, NOFRAME, pObject1:Pointer, pObject2:Pointer
    mov eax, pObject2
    sub eax, pObject1
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.Delete
; Purpose:   Deletes a node from the tree.
; Arguments: Arg1: -> Object.
; Return:    eax -> Deleted object.

Method RedBlackTree.Delete, uses esi, pObject:Pointer
    SetObject esi
    mov eax, pObject
    .if eax != NULL
      add eax, [esi].dNodeOffset
      invoke RedBlackTree_NodeDelete, eax
      dec [esi].dCount
      mov eax, pObject
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.DeleteAll
; Purpose:   Wipes the complete tree.
; Arguments: None.
; Return:    Nothing.

Method RedBlackTree.DeleteAll, NOFRAME
    SetObject ecx,, [esp + 4]
    m2m [ecx].pRootNode, [ecx].pNilNode, edx
    and [ecx].dCount, 0
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.Dispose
; Purpose:   Removes an object from the tree and disposes it.
; Arguments: Arg1: -> Object
; Return:    Nothing.

Method RedBlackTree.Dispose, NOFRAME, pObject:Pointer
    OCall Pointer ptr [esp + 08]::RedBlackTree.Delete, Pointer ptr [esp + 08]
    Kill eax
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.DisposeAll
; Purpose:   Wipes the complete tree and disposes all node hosting objects.
; Arguments: None.
; Return:    Nothing.

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

DisposeRBSubtree proc pNode:PRBT_NODE                   ;esi is passed from DisposeAll
    ;Dispose smaller sibling
    mov edx, [esp + 4]                                  ;pNode
    mov eax, [edx].RBT_NODE.pLeftNode
    .if eax != [esi].RedBlackTree.pNilNode
      invoke DisposeRBSubtree, eax
    .endif

    ;Dispose bigger sibling
    mov edx, [esp + 4]                                  ;pNode
    mov eax, [edx].RBT_NODE.pRightNode
    .if eax != [esi].RedBlackTree.pNilNode
      invoke DisposeRBSubtree, eax
    .endif

    mov edx, [esp + 4]                                  ;pNode
    sub edx, [esi].RedBlackTree.dNodeOffset             ;edx -> Object
    Kill edx

    ret 4
DisposeRBSubtree endp

OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF

Method RedBlackTree.DisposeAll, NOFRAME
    push esi
    SetObject esi,, [esp + 8]                           ;Self
    .if [esi].dCount != 0
      invoke DisposeRBSubtree, [esi].pRootNode
      OCall esi.DeleteAll
    .endif
    pop esi
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.Find
; Purpose:   Searches for the first node containing the specified key.
; Arguments: Arg1: -> Object with key data to find.
; Return:    eax -> found Object or NULL if failed.

Method RedBlackTree.Find, uses esi ebx, pObject:Pointer
    SetObject esi
    mov ebx, [esi].pRootNode
    .while ebx != [esi].pNilNode
      mov eax, ebx
      sub eax, [esi].dNodeOffset
      OCall esi.Compare, pObject, eax
      .if eax == 0
        mov eax, ebx
        sub eax, [esi].dNodeOffset
        ExitMethod
      .endif
      .if Sign?                                         ;Check sign flag, if set, the result is < 0
        mov ebx, [ebx].RBT_NODE.pLeftNode
      .else
        mov ebx, [ebx].RBT_NODE.pRightNode
      .endif
    .endw
    xor eax, eax                                        ;eax = NULL => not found!
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.FirstThat
; Purpose:   Searchs the first object that doesn't return FALSE in eax.
; Arguments: Arg1: -> (static addr) Function that evaluates to TRUE or FALSE (return value in eax).
;            Arg2-n: (optional) Parameters to be used by the called procedure.
; Return:    eax -> Object or NULL if not found.

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

FirstThatRBSubtree proc pNode:PRBT_NODE                 ;esi, ebx & ebp are passed from FirstThat
    ;Go for the smaller sibling
    mov edx, [esp + 4]                                  ;pNode
    mov eax, [edx].RBT_NODE.pLeftNode
    .if eax != [esi].RedBlackTree.pNilNode
      invoke FirstThatRBSubtree, eax
      .if eax != FALSE
        ret 4
      .endif
    .endif

    ;Call test function
    mov edi, [esp + 4]                                  ;pNode
    push ecx                                            ;Save ecx
    mov eax, ebx
    sub edi, [esi].RedBlackTree.dNodeOffset
    .while ecx != 0
      push [eax]                                        ;push parameter
      sub eax, 4
      dec ecx
    .endw
    push edi
    call ebp                                            ;Call function
    pop ecx                                             ;Restore ecx
    .if eax != FALSE
      mov ebp, [esp + 4]
      ret 4
    .endif

    ;Go for the bigger sibling
    mov edx, [esp + 4]                                  ;pNode
    mov eax, [edx].RBT_NODE.pRightNode
    .if eax != [esi].RedBlackTree.pNilNode
      invoke FirstThatRBSubtree, eax
      .if eax != FALSE
        ret 4
      .endif
    .endif
    xor eax, eax
    ret 4
FirstThatRBSubtree endp

OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF

Method RedBlackTree.FirstThat, NOFRAME, pTestFunc:Pointer
    push esi                                            ;ecx = number of additional passed dwords
    SetObject esi,, [esp + 08]                          ;Self
    .if [esi].dCount != 0
      push ebp
      push ebx
      push edi
      mov ebp, [esp + 24]                               ;ebp = pTestFunc
      lea ebx, [esp + 4*ecx + 24]                       ;ebx -> last function parameter
      invoke FirstThatRBSubtree, [esi].pRootNode
      .if eax != FALSE
        mov eax, ebp
        sub eax, [esi].dNodeOffset
      .else
        xor eax, eax
      .endif
      pop edi
      pop ebx
      pop ebp
    .else
      xor eax, eax
    .endif
    pop esi
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.FirstThatNot
; Purpose:   Searchs the first object that returns FALSE in eax.
; Arguments: Arg1: -> (static addr) Function that evaluates to TRUE or FALSE (return value in eax).
;            Arg2-n: (optional) Parameters to be used by the called procedure.
; Return:    eax -> object or NULL if not found.

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

FirstThatNotRBSubtree proc pNode:PRBT_NODE              ;esi, ebx & ebp are passed from FirstThatNot
    ;Go for the smaller sibling
    mov edx, [esp + 4]                                  ;pNode
    mov eax, [edx].RBT_NODE.pLeftNode
    .if eax != [esi].RedBlackTree.pNilNode
      invoke FirstThatNotRBSubtree, eax
      .if eax == FALSE
        ret 4
      .endif
    .endif

    ;Call test function
    mov edi, [esp + 4]                                  ;pNode
    push ecx                                            ;Save ecx
    mov eax, ebx
    sub edi, [esi].RedBlackTree.dNodeOffset
    .while ecx != 0
      push [eax]                                        ;push parameter
      sub eax, 4
      dec ecx
    .endw
    push edi
    call ebp                                            ;Call function
    pop ecx                                             ;Restore ecx
    .if eax == FALSE
      mov ebp, [esp + 4]
      ret 4
    .endif

    ;Go for the bigger sibling
    mov edx, [esp + 4]                                  ;pNode
    mov eax, [edx].RBT_NODE.pRightNode
    .if eax != [esi].RedBlackTree.pNilNode
      invoke FirstThatNotRBSubtree, eax
      .if eax == FALSE
        ret 4
      .endif
    .endif
    mov eax, TRUE
    ret 4
FirstThatNotRBSubtree endp

OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF

Method RedBlackTree.FirstThatNot, NOFRAME, pTestFunc:Pointer
    push esi                                            ;ecx = number of additional passed dwords
    SetObject esi,, [esp + 08]                          ;Self
    .if [esi].dCount != 0
      push ebp
      push ebx
      push edi
      mov ebp, [esp + 24]                               ;ebp = pTestFunc
      lea ebx, [esp + 4*ecx + 24]                       ;ebx -> last procedure parameter
      invoke FirstThatNotRBSubtree, [esi].pRootNode
      .if eax == FALSE
        mov eax, ebp
        sub eax, [esi].dNodeOffset
      .else
        xor eax, eax
      .endif
      pop edi
      pop ebx
      pop ebp
    .else
      xor eax, eax
    .endif
    pop esi
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.ForEach
; Purpose:   Traverses all nodes in the tree in ascending order.
; Arguments: Arg1: -> Processing procedure (static address).
;            Arg2-n: (Optional) Parameters to be used by the processing procedure.
; Return:    Nothing.
; Note:      While the loop is running, no object should be deleted or inserted!

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

ForEachRBSubtree proc pNode:PRBT_NODE                   ;esi, ebx & ebp are passed from ForEach
    ;Go for the smaller sibling
    mov edx, [esp + 4]                                  ;pNode
    mov eax, [edx].RBT_NODE.pLeftNode
    .if eax != [esi].RedBlackTree.pNilNode
      invoke ForEachRBSubtree, eax
    .endif

    ;Call action procedure
    mov edi, [esp + 4]                                  ;pNode
    push ecx                                            ;Save ecx
    mov eax, ebx
    sub edi, [esi].RedBlackTree.dNodeOffset
    .while ecx != 0
      push [eax]                                        ;push parameter
      sub eax, 4
      dec ecx
    .endw
    push edi
    call ebp                                            ;Call procedure
    pop ecx                                             ;Restore ecx

    ;Go for the bigger sibling
    mov edx, [esp + 4]                                  ;pNode
    mov eax, [edx].RBT_NODE.pRightNode
    .if eax != [esi].RedBlackTree.pNilNode
      invoke ForEachRBSubtree, eax
    .endif
    ret 4
ForEachRBSubtree endp

OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF

Method RedBlackTree.ForEach, NOFRAME, pActionProc:Pointer
    push esi                                            ;ecx = number of additional passed dwords
    SetObject esi,, [esp + 08]                          ;Self
    .if [esi].dCount != 0
      push ebp
      push ebx
      push edi
      mov ebp, [esp + 24]                               ;ebp = pActionProc
      lea ebx, [esp + 4*ecx + 24]                       ;ebx -> last procedure parameter
      invoke ForEachRBSubtree, [esi].pRootNode
      pop edi
      pop ebx
      pop ebp
    .endif
    pop esi
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.ForEachRev
; Purpose:   Traverses all objects in the tree in descending order.
; Arguments: Arg1: -> Processing procedure (static address).
;            Arg2-n: (Optional) Parameters to be used by the processing procedure.
; Return:    Nothing.
; Note:      While the loop is running, no object should be deleted or inserted!

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

ForEachRevRBSubtree proc pNode:PRBT_NODE                ;esi, ebx & ebp are passed from ForEachRev
    ;Go for the smaller sibling
    mov edx, [esp + 4]                                  ;pNode
    mov eax, [edx].RBT_NODE.pRightNode
    .if eax != [esi].RedBlackTree.pNilNode
      invoke ForEachRevRBSubtree, eax
    .endif

    ;Call action procedure
    mov ecx, [esp + 4]                                  ;pNode
    push edi                                            ;Save edi
    mov eax, ebx
    sub ecx, [esi].RedBlackTree.dNodeOffset
    .while edi != 0
      mov edx, [eax]                                    ;Parameter
      sub eax, 4
      push edx                                          ;Put this value in the stack
      dec edi
    .endw
    push ecx
    call ebp                                            ;Call procedure
    pop edi                                             ;Restore edi

    ;Go for the bigger sibling
    mov edx, [esp + 4]                                  ;pNode
    mov eax, [edx].RBT_NODE.pLeftNode
    .if eax != [esi].RedBlackTree.pNilNode
      invoke ForEachRevRBSubtree, eax
    .endif
    ret 4
ForEachRevRBSubtree endp

OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF

Method RedBlackTree.ForEachRev, NOFRAME, pActionProc:Pointer
    push esi                                            ;ecx = number of additional passed dwords
    SetObject esi,, [esp + 08]                          ;esi = Self
    .if [esi].dCount != 0
      push ebp
      push ebx
      push edi
      mov ebp, [esp + 24]                               ;ebp = pActionProc
      mov edi, ecx                                      ;edi = number to dword to pass to the
      lea ebx, [esp + 4*ecx + 24]                       ;ebx -> last procedure parameter
      invoke ForEachRevRBSubtree, [esi].pRootNode
      pop edi
      pop ebx
      pop ebp
    .endif
    pop esi
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.GetFirst
; Purpose:   Returns the object with the smallest key.
; Arguments: Nothing.
; Return:    eax -> Object or NULL if failed.

Method RedBlackTree.GetFirst, NOFRAME
    push esi
    SetObject esi,, [esp + 8]
    invoke RedBlackTree_FindMin, [esi].pRootNode
    sub eax, [esi].dNodeOffset
    pop esi
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.GetLast
; Purpose:   Returns the object with the largest key.
; Arguments: Nothing.
; Return:    eax -> Object or NULL if failed.

Method RedBlackTree.GetLast, NOFRAME
    push esi
    SetObject esi,, [esp + 8]
    invoke RedBlackTree_FindMax, [esi].pRootNode
    sub eax, [esi].dNodeOffset
    pop esi
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.GetNext
; Purpose:   Returns the object with a key that is larger than the key of the specified object.
; Arguments: Arg1: -> Object.
; Return:    eax -> Next object or NULL if failed.

Method RedBlackTree.GetNext, uses esi, pObject:Pointer
    SetObject esi
    mov eax, pObject
    add eax, [esi].dNodeOffset
    mov edx, [eax].RBT_NODE.pRightNode
    .if edx != [esi].pNilNode
      invoke RedBlackTree_FindMin, edx
    .else
      .while [eax].RBT_NODE.pParentNode != NULL
        mov edx, [eax].RBT_NODE.pParentNode
        .break .if eax == [edx].RBT_NODE.pLeftNode
        mov eax, edx
      .endw
      mov eax, [eax].RBT_NODE.pParentNode
    .endif
    .if eax != NULL
      sub eax, [esi].dNodeOffset
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.GetPrev
; Purpose:   Returns the object with a key that is smaller than the key of the specified object.
; Arguments: Arg1: -> Object.
; Return:    eax -> Previous object or NULL if failed.

Method RedBlackTree.GetPrev, uses esi, pObject:Pointer
    SetObject esi
    mov eax, pObject
    add eax, [esi].dNodeOffset
    mov edx, [eax].RBT_NODE.pLeftNode
    .if edx != [esi].pNilNode
      invoke RedBlackTree_FindMax, edx
    .else
      .while [eax].RBT_NODE.pParentNode != NULL
        mov edx, [eax].RBT_NODE.pParentNode
        .break .if eax == [edx].RBT_NODE.pRightNode
        mov eax, edx
      .endw
      mov eax, [eax].RBT_NODE.pParentNode
    .endif
    .if eax != NULL
      sub eax, [esi].dNodeOffset                        ;eax -> object/hosting data structure
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.Init
; Purpose:   Initializes the object.
; Arguments: Arg1: -> Owner object.
;            Arg2: Node offset within hosting object (i.e. offset Object.RBTN, where RBTN is a
;                  RBT_NODE structure).
; Return:    Nothing.

Method RedBlackTree.Init, uses esi, pOwner:Pointer, dNodeOffset:dword
    SetObject esi
    ACall esi.Init, pOwner
    mov edx, dNodeOffset
    lea eax, [esi].NilNode
    mov [esi].dNodeOffset, edx
    mov [esi].pNilNode, eax
    mov [esi].pRootNode, eax
    OCall esi.InitNode, eax
    mov [esi].NilNode.dColor, scBLACK
    and [esi].dCount, 0
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.InitNode
; Purpose:   Performs a node initialization.
; Arguments: Arg1: -> Node.
; Return:    eax -> Node.

Method RedBlackTree.InitNode, NOFRAME, pNode:PRBT_NODE
    SetObject ecx,, [esp + 4]
    mov eax, [esp + 8]                                  ;eax -> RBT_NODE
    and [eax].RBT_NODE.pParentNode, NULL
    mov edx, [ecx].pNilNode
    mov [eax].RBT_NODE.pLeftNode, edx
    mov [eax].RBT_NODE.pRightNode, edx
    mov [eax].RBT_NODE.dColor, scRED                    ;New inserted nodes are red!
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.Insert
; Purpose:   Creates a new node and associates a key and data.
; Arguments: Arg1: -> Object.
; Return:    eax = Error code.
;            ecx -> (inserted) object.

Method RedBlackTree.Insert, uses esi, pObject:Pointer
    SetObject esi
    mov eax, pObject
    add eax, [esi].dNodeOffset
    OCall esi.InitNode, eax
    invoke RedBlackTree_NodeInsert, eax
    .if eax == OBJ_OK
      inc [esi].dCount
    .endif
    mov ecx, pObject
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.LastThat
; Purpose:   Searchs the last object that doesn't return FALSE in eax.
; Arguments: Arg1: -> (static addr) Function that evaluates to TRUE or FALSE (return value in eax).
;            Arg2-n: (optional) Parameters to be used by the called procedure.
; Return:    eax -> Object or NULL if not found.

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

LastThatRBSubtree proc pNode:PRBT_NODE                  ;esi, ebx & ebp are passed from LastThat
    ;Go for the smaller sibling
    mov edx, [esp + 4]                                  ;pNode
    mov eax, [edx].RBT_NODE.pRightNode
    .if eax != [esi].RedBlackTree.pNilNode
      invoke LastThatRBSubtree, eax
      .if eax != FALSE
        ret 4
      .endif
    .endif

    ;Call test function
    mov edi, [esp + 4]                                  ;pNode
    push ecx                                            ;Save ecx
    mov eax, ebx
    sub edi, [esi].RedBlackTree.dNodeOffset
    .while ecx != 0
      push [eax]                                        ;push parameter
      sub eax, 4
      dec ecx
    .endw
    push edi
    call ebp                                            ;Call function
    pop ecx                                             ;Restore ecx
    .if eax != FALSE
      mov ebp, [esp + 4]
      ret 4
    .endif

    ;Go for the bigger sibling
    mov edx, [esp + 4]                                  ;pNode
    mov eax, [edx].RBT_NODE.pLeftNode
    .if eax != [esi].RedBlackTree.pNilNode
      invoke LastThatRBSubtree, eax
      .if eax != FALSE
        ret 4
      .endif
    .endif
    xor eax, eax
    ret 4
LastThatRBSubtree endp

OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF

Method RedBlackTree.LastThat, NOFRAME, pTestFunc:Pointer
    push esi                                            ;ecx = number of additional passed dwords
    SetObject esi,, [esp + 08]                          ;Self
    .if [esi].dCount != 0
      push ebp
      push ebx
      push edi
      mov ebp, [esp + 24]                               ;ebp = pTestFunc
      lea ebx, [esp + 4*ecx + 24]                       ;ebx -> last procedure parameter
      invoke LastThatRBSubtree, [esi].pRootNode
      .if eax != FALSE
        mov eax, ebp
        sub eax, [esi].dNodeOffset
      .else
        xor eax, eax
      .endif
      pop edi
      pop ebx
      pop ebp
    .else
      xor eax, eax
    .endif
    pop esi
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.LastThatNot
; Purpose:   Searchs the first object that returns FALSE in eax.
; Arguments: Arg1: -> (static addr) Function that evaluates to TRUE or FALSE (return value in eax).
;            Arg2-n: (optional) Parameters to be used by the called procedure.
; Return:    eax -> object or NULL if not found.

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

LastThatNotRBSubtree proc pNode:PRBT_NODE               ;esi, ebx & ebp are passed from LastThatNot
    ;Go for the smaller sibling
    mov edx, [esp + 4]                                  ;pNode
    mov eax, [edx].RBT_NODE.pRightNode
    .if eax != [esi].RedBlackTree.pNilNode
      invoke LastThatNotRBSubtree, eax
      .if eax == FALSE
        ret 4
      .endif
    .endif

    ;Call test function
    mov edi, [esp + 4]                                  ;pNode
    push ecx                                            ;Save ecx
    mov eax, ebx
    sub edi, [esi].RedBlackTree.dNodeOffset
    .while ecx != 0
      push [eax]                                        ;push parameter
      sub eax, 4
      dec ecx
    .endw
    push edi
    call ebp                                            ;Call function
    pop ecx                                             ;Restore ecx
    .if eax == FALSE
      mov ebp, [esp + 4]
      ret 4
    .endif

    ;Go for the bigger sibling
    mov edx, [esp + 4]                                  ;pNode
    mov eax, [edx].RBT_NODE.pLeftNode
    .if eax != [esi].RedBlackTree.pNilNode
      invoke LastThatNotRBSubtree, eax
      .if eax == FALSE
        ret 4
      .endif
    .endif
    mov eax, TRUE
    ret 4
LastThatNotRBSubtree endp

OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF

Method RedBlackTree.LastThatNot, NOFRAME, pTestFunc:Pointer
    push esi                                            ;ecx = number of additional passed dwords
    SetObject esi,, [esp + 08]                          ;Self
    .if [esi].dCount != 0
      push ebp
      push ebx
      push edi
      mov ebp, [esp + 24]                               ;ebp = pTestFunc
      lea ebx, [esp + 4*ecx + 24]                       ;ebx -> last procedure parameter
      invoke LastThatNotRBSubtree, [esi].pRootNode
      .if eax == FALSE
        mov eax, ebp
        sub eax, [esi].dNodeOffset
      .else
        xor eax, eax
      .endif
      pop edi
      pop ebx
      pop ebp
    .else
      xor eax, eax
    .endif
    pop esi
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.GetItem
; Purpose:   Loads an item from a stream object.
; Arguments: Arg1: -> Stream object containing an object.
; Return:    eax -> New loaded object.

Method RedBlackTree.GetItem, NOFRAME, pStream:PTP_Stream
    OCall Pointer ptr [esp + 12]::Stream.Get, Pointer ptr [esp + 4]      ;Self
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.PutItem
; Purpose:   Stores an item of the RedBlackTree in a stream object.
; Arguments: Arg1: -> Steam.
;            Arg2: -> Item.
; Return:    Nothing.
; Note:      The hosting object/data structure dont need to save the node information since it is
;            rebuild when loading from stream.

Method RedBlackTree.PutItem, NOFRAME, pStream:PTP_Stream, pItem:Pointer
    OCall Pointer ptr [esp + 12]::Stream.Put, Pointer ptr [esp + 12]
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.Load
; Purpose:   Loads and initializes the RedBlackTree from a stream object.
; Arguments: Arg1: -> Stream object.
;            Arg2: -> Owner object.
; Return:    Nothing.

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

LoadRBSubtree proc pParentNode:PRBT_NODE                ;esi is passed from LastThatNot
    push edi
    OCall esi::RedBlackTree.GetItem, ebx                ;Load the object
    add eax, [esi].RedBlackTree.dNodeOffset
    mov edi, eax
    m2m [edi].RBT_NODE.pParentNode, [esp + 8], edx      ;Set Parent node
    OCall ebx::Stream.BinRead32
    movzx ecx, ax
    mov [edi].RBT_NODE.dColor, ecx                      ;Set color

    push eax
    .ifBitSet eax, BIT16
      invoke LoadRBSubtree, edi
    .else
      mov eax, [esi].RedBlackTree.pNilNode
    .endif
    mov [edi].RBT_NODE.pLeftNode, eax                   ;Set left node

    pop eax
    .ifBitSet eax, BIT17
      invoke LoadRBSubtree, edi
    .else
      mov eax, [esi].RedBlackTree.pNilNode
    .endif
    mov [edi].RBT_NODE.pRightNode, eax                  ;Set right node

    mov eax, edi                                        ;eax -> loaded node
    pop edi
    ret 4
LoadRBSubtree endp

OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF

Method RedBlackTree.Load, uses ebx esi, pStream:PTP_Stream, pOwner:Pointer
    SetObject esi
    OCall esi.Init, pOwner, 0
    mov ebx, pStream
    OCall ebx::Stream.BinRead, addr [esi].dErrorCode, 4
    OCall ebx::Stream.BinRead, addr [esi].dNodeOffset, 4
    OCall ebx::Stream.BinRead, addr [esi].dCount, 4
    .if [esi].dCount != 0
      invoke LoadRBSubtree, NULL
    .else
      mov eax, [esi].pNilNode
    .endif
    mov [esi].pRootNode, eax
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.Store
; Purpose:   Stores the RedBlackTree in a stream object.
; Arguments: Arg1: -> Stream object.
; Return:    Nothing.

OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

StoreRBSubtree proc pNode:PRBT_NODE                     ;esi and ebx are passed from Store
    push edi
    mov edi, [esp + 8]                                  ;pNode
    mov edx, edi
    sub edx, [esi].RedBlackTree.dNodeOffset             ;edx -> Object
    OCall esi::RedBlackTree.PutItem, ebx, edx           ;Save the object

    ;Calc and save node information
    mov eax, [esi].RedBlackTree.pNilNode
    mov ecx, [edi].RBT_NODE.dColor
    .if eax != [edi].RBT_NODE.pLeftNode
      BitSet ecx, BIT16
    .endif
    .if eax != [edi].RBT_NODE.pRightNode
      BitSet ecx, BIT17
    .endif
    OCall ebx::Stream.BinWrite32, ecx                   ;Save it

    ;Save children objects
    mov ecx, [edi].RBT_NODE.pLeftNode
    .if ecx != [esi].RedBlackTree.pNilNode
      invoke StoreRBSubtree, ecx
    .endif
    mov ecx, [edi].RBT_NODE.pRightNode
    .if ecx != [esi].RedBlackTree.pNilNode
      invoke StoreRBSubtree, ecx
    .endif

    pop edi
    ret 4
StoreRBSubtree endp

OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF

Method RedBlackTree.Store, uses ebx esi, pStream:PTP_Stream
    SetObject esi
    mov ebx, pStream
    OCall ebx::Stream.BinWrite32, [esi].dErrorCode
    OCall ebx::Stream.BinWrite32, [esi].dNodeOffset
    OCall ebx::Stream.BinWrite32, [esi].dCount
    .if [esi].dCount != 0
      invoke StoreRBSubtree, [esi].pRootNode
    .endif
MethodEnd


endif
