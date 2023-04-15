; ==================================================================================================
; Title:   RedBlackTree.inc
; Author:  G. Friedrich, Homer
; Version: 1.0.0
; Purpose: ObjAsm32 Support of red-black tree objects.
; Links:   http://www.cs.iupui.edu/~xkzou/teaching/CS580/Red-black_trees.ppt
;          http://en.wikipedia.org/wiki/Red-black_tree
;          http://en.wikipedia.org/wiki/Tree_traversal
;          http://en.wikipedia.org/wiki/Threaded_binary_tree
;          http://www.eternallyconfuzzled.com/tuts/datastructures/jsw_tut_rbtree.aspx
;          http://en.wikipedia.org/wiki/AA_tree
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
;          - This implementation doesnt use a sentinel (NIL) node. The pointer to the sentinel node
;            is replaced by NULL and the logic is adjusted to it.
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
;                                    | 4 |     Root Node (always black)
;                                     ———
;                                   /     \
;                                  /       \
;                                 /         \
;                           / ...             ... \
;             Parent Node  /                       \
;                         /                         \
;                       ———                         ———
;   Node with key = 2  | 2 |                       | 6 |
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
;           NULL     NULL NULL     NULL NULL     NULL NULL     NULL    Leave Nodes (always black)



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
  VirtualMethod     Insert,         Pointer
  VirtualMethod     LastThat,       Pointer, ?          ;-> Proc, Parameters
  VirtualMethod     LastThatNot,    Pointer, ?          ;-> Proc, Parameters

  DynamicMethod     GetItem,        PTP_Stream          ;-> Stream
  DynamicMethod     PutItem,        PTP_Stream, Pointer ;-> Stream, -> Item
  RedefineMethod    Load,           PTP_Stream, Pointer ;-> Stream, -> Owner
  RedefineMethod    Store,          PTP_Stream          ;-> Stream

  DefineVariable    dNodeOffset,    dword,        0
  DefineVariable    pRootNode,      PRBT_NODE,    NULL
  DefineVariable    dCount,         dword,        0

  DefineVariable    ObjLock,        ObjectLock,   {}    ;Locking structure for multithreaded access

ObjectEnd


.code
; ==================================================================================================

if IMPLEMENT

; ==================================================================================================
;  Utility macros and procedures
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Procedure: RedBlackTree_Show
; Purpose:   Debug procedure to display the tree structure.
; Arguments: Arg1: Key offset into the object.
; Return:    None.

if DEBUGGING
CStr DestWindow, "REDBLACKTREE";"Red-Black Tree structure"

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

;    @invoke DbgOutText, " parent ", dColor, DBG_EFFECT_NORMAL, offset DestWindow
;    mov ecx, ebx
;    mov edx, [ecx].RBT_NODE.pParentNode
;    .if edx != NULL
;      sub edx, [esi].RedBlackTree.dNodeOffset
;      add edx, edi
;      invoke dword2dec, addr bNumBuffer, [edx]
;      invoke DbgOutText, addr bNumBuffer, dColor, DBG_EFFECT_NORMAL, offset DestWindow
;    .else
;      @invoke DbgOutText, " (NULL) ", dColor, DBG_EFFECT_NORMAL, offset DestWindow
;    .endif

    invoke DbgOutText, offset szCRLF, dColor, DBG_EFFECT_NORMAL, offset DestWindow

    add dIndent, 3

    .if ebx != NULL
      mov eax, [ebx].RBT_NODE.pLeftNode
      .if eax != NULL
        invoke ShowRBSubtree, eax, dIndent
      .else
        invoke DbgOutText, addr bIndBuffer, $RGB(0,0,0), DBG_EFFECT_NORMAL, offset DestWindow
        @invoke DbgOutText, "   Null", $RGB(0,0,0), DBG_EFFECT_NORMAL, offset DestWindow
        invoke DbgOutText, offset szCRLF, $RGB(0,0,0), DBG_EFFECT_NORMAL, offset DestWindow
      .endif
      mov eax, [ebx].RBT_NODE.pRightNode
      .if eax != NULL
        invoke ShowRBSubtree, eax, dIndent
      .else
        invoke DbgOutText, addr bIndBuffer, $RGB(0,0,0), DBG_EFFECT_NORMAL, offset DestWindow
        @invoke DbgOutText, "   Null", $RGB(0,0,0), DBG_EFFECT_NORMAL, offset DestWindow
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

endif

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     RedBlackTree_CallProc
; Purpose:   Used to invoke a function or callback procedure in an iterator method.
; Arguments: edi -> Node.
;            ebp -> Procedure/Callback address.
;            ecx = # of dword arguments to push.
;            ebx -> Last dword argument.
; Return:    eax = Function return value.
; Note:      Unpreserved registers: (eax), edx

RedBlackTree_CallProc macro
    mov edx, edi
    push ecx                                            ;Save ecx
    mov eax, ebx
    sub edx, esi
    .while ecx != 0
      push [eax]                                        ;push parameter
      sub eax, 4
      dec ecx
    .endw
    push edx
    call ebp                                            ;Call procedure, eax = return value
    pop ecx                                             ;Restore ecx
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     RedBlackTree_GetMax
; Purpose:   Returns the node with the biggest key.
; Arguments: eax -> Start node.
; Return:    eax -> Max node.
; Note:      Unpreserved registers: edx

RedBlackTree_GetMax macro
    .repeat
      mov edx, [eax].RBT_NODE.pRightNode
      .break .if edx == NULL
      mov eax, edx
    .until FALSE
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     RedBlackTree_GetMin
; Purpose:   Returns the node with the smallest key.
; Arguments: eax -> Start node.
; Return:    eax -> Min node.
; Note:      Unpreserved registers: edx

RedBlackTree_GetMin macro
    .repeat
      mov edx, [eax].RBT_NODE.pLeftNode
      .break .if edx == NULL
      mov eax, edx
    .until FALSE
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     RedBlackTree_RotateLeft
; Purpose:   Rotates the tree to the left around a specified node.
; Arguments: Reg: -> Node.
; Return:    eax -> Node.
; Note:      Unpreserved registers: ecx, edx

RedBlackTree_RotateLeft macro Reg:req
    mov eax, Reg
    mov ecx, [eax].RBT_NODE.pRightNode
    m2m [eax].RBT_NODE.pRightNode, [ecx].RBT_NODE.pLeftNode, edx
    .if edx != NULL
      mov [edx].RBT_NODE.pParentNode, eax
    .endif

    mov edx, [eax].RBT_NODE.pParentNode
    .if ecx != NULL
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
    .if eax != NULL
      mov [eax].RBT_NODE.pParentNode, ecx
    .endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     RedBlackTree_RotateRight
; Purpose:   Rotates the tree to the right around a specified node.
; Arguments: Reg: -> Node.
; Return:    eax -> Node.
; Note:      Unpreserved registers: ecx, edx

RedBlackTree_RotateRight macro Reg:req
    mov eax, Reg
    mov ecx, [eax].RBT_NODE.pLeftNode

    m2m [eax].RBT_NODE.pLeftNode, [ecx].RBT_NODE.pRightNode, edx
    .if edx != NULL
      mov [edx].RBT_NODE.pParentNode, eax
    .endif

    mov edx, [eax].RBT_NODE.pParentNode
    .if ecx != NULL
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
    .if eax != NULL
      mov [eax].RBT_NODE.pParentNode, ecx
    .endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     RedBlackTree_TraverseInOrder
; Purpose:   Macro used to traverse the tree in order (ascending or descending) and invoke a
;            function or callback procedure in an iterator method.
; Arguments: Branch1/Branch2: Left/Right = forward in order, Right/Left = reverse in order.
;            DoTest: 0 = no test is performed, 1 = test eax != FALSE, 2 = test eax == FALSE.

RedBlackTree_TraverseInOrder macro Branch1:req, Branch2:req, DoTest:req 
    SetObject edx,, [esp + 04]                          ;ecx = number of additional passed dwords
    .if [edx].dCount != 0
      push ebp
      push ebx
      push edi
      push esi
      mov ebp, [esp + 24]                               ;ebp -> Procedure
      lea ebx, [esp + 4*ecx + 24]                       ;ebx -> Last procedure parameter
      mov esi, [edx].RedBlackTree.dNodeOffset

      ;Start at the root node
      mov edi, [edx].pRootNode
      ReleaseObject edx
      jmp @F
      
      .while edi != NULL
        .while TRUE
          .if edx == [edi].RBT_NODE.p&Branch1&Node
            RedBlackTree_CallProc
            if DoTest eq 1
              or eax, eax
              jnz @@Found
            elseif DoTest eq 2
              or eax, eax
              jz @@Found
            endif
            mov edx, edi
            .if [edi].RBT_NODE.p&Branch2&Node == NULL
              mov edi, [edi].RBT_NODE.pParentNode
              .break
            .endif
            mov edi, [edi].RBT_NODE.p&Branch2&Node
            .continue
            
          .else
            .if edx == [edi].RBT_NODE.p&Branch2&Node
              mov edx, edi
              mov edi, [edi].RBT_NODE.pParentNode
              .break
            .endif
      @@:
            .if [edi].RBT_NODE.p&Branch1&Node == NULL
              RedBlackTree_CallProc
              if DoTest eq 1
                or eax, eax
                jnz @@Found
              elseif DoTest eq 2
                or eax, eax
                jz @@Found
              endif
              mov edx, edi
              .if [edi].RBT_NODE.p&Branch2&Node == NULL
                mov edi, [edi].RBT_NODE.pParentNode
                .break
              .endif
              mov edi, [edi].RBT_NODE.p&Branch2&Node
              .continue
            .endif
            mov edx, edi
            mov edi, [edi].RBT_NODE.p&Branch1&Node
          .endif
        .endw
      .endw
      
      if DoTest ne 0
      xor eax, eax                                      ;Not found => eax = NULL
      jmp @@Exit

      @@Found:
      mov eax, edi                                      ;Found node address
      sub eax, esi                                      ;sub object offset, eax -> Object                                      
      endif

      @@Exit:
      pop esi
      pop edi
      pop ebx
      pop ebp

    if DoTest ne 0
    .else
      xor eax, eax                                      ;Not found => eax = NULL
    endif
    .endif
endm


; ==================================================================================================
;  RedBlackTree implementation
; ==================================================================================================

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

Method RedBlackTree.Delete, NOFRAME, pObject:Pointer
    push ebp
    push ebx
    push edi
    push esi
    SetObject esi,, [esp + 20]                          ;Self
    mov eax, [esp + 24]                                 ;pObject
    .if eax != NULL
      add eax, [esi].dNodeOffset
      mov edi, eax
      mov edx, [eax].RBT_NODE.pLeftNode
      mov ecx, [eax].RBT_NODE.pRightNode
      .if edx != NULL && ecx != NULL
        mov eax, ecx
        RedBlackTree_GetMin
      .endif
  
      mov edx, [eax].RBT_NODE.pLeftNode
      .if edx == NULL
        mov edx, [eax].RBT_NODE.pRightNode
      .endif
      mov ebp, edx
  
      mov ecx, [eax].RBT_NODE.pParentNode
      .if edx != NULL
        mov [edx].RBT_NODE.pParentNode, ecx
      .endif
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
  
      .if eax != edi
        ;Replace node
        mov edx, edi
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
        .if edx != NULL
          mov [edx].RBT_NODE.pParentNode, eax
        .endif
        mov edx, [eax].RBT_NODE.pRightNode
        .if edx != NULL
          mov [edx].RBT_NODE.pParentNode, eax
        .endif
      .endif
  
      .if ebx == scBLACK                                  ;Check if the tree is balanced
        ;Fixup deletion
        mov eax, ebp
        .while eax != NULL && eax != [esi].RedBlackTree.pRootNode && [eax].RBT_NODE.dColor == scBLACK
          mov edx, [eax].RBT_NODE.pParentNode
          .if eax == [edx].RBT_NODE.pLeftNode
            mov ebx, [edx].RBT_NODE.pRightNode
            .if [ebx].RBT_NODE.dColor == scRED
              mov [ebx].RBT_NODE.dColor, scBLACK
              mov edx, [eax].RBT_NODE.pParentNode
              mov [edx].RBT_NODE.dColor, scRED
              RedBlackTree_RotateLeft edx
              mov eax, ebp
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
                RedBlackTree_RotateRight ebx
                mov eax, ebp
                mov edx, [eax].RBT_NODE.pParentNode
                mov ebx, [edx].RBT_NODE.pRightNode
              .endif
              mov edx, [eax].RBT_NODE.pParentNode
              m2m [ebx].RBT_NODE.dColor, [edx].RBT_NODE.dColor, ecx
              mov [edx].RBT_NODE.dColor, scBLACK
              mov ecx, [ebx].RBT_NODE.pRightNode
              mov [ecx].RBT_NODE.dColor, scBLACK
              RedBlackTree_RotateLeft edx
              mov eax, [esi].RedBlackTree.pRootNode
            .endif
          .else
            mov edx, [eax].RBT_NODE.pParentNode
            mov ebx, [edx].RBT_NODE.pLeftNode
            .if [ebx].RBT_NODE.dColor == scRED
              mov [ebx].RBT_NODE.dColor, scBLACK
              mov edx, [eax].RBT_NODE.pParentNode
              mov [edx].RBT_NODE.dColor, scRED
              RedBlackTree_RotateRight edx
              mov eax, ebp
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
                RedBlackTree_RotateLeft ebx
                mov eax, ebp
                mov edx, [eax].RBT_NODE.pParentNode
                mov ebx, [edx].RBT_NODE.pLeftNode
              .endif
              mov edx, [eax].RBT_NODE.pParentNode
              m2m [ebx].RBT_NODE.dColor, [edx].RBT_NODE.dColor, ecx
              mov [edx].RBT_NODE.dColor, scBLACK
              mov edx, [ebx].RBT_NODE.pLeftNode
              mov [edx].RBT_NODE.dColor, scBLACK
              RedBlackTree_RotateRight [eax].RBT_NODE.pParentNode
              mov eax, [esi].RedBlackTree.pRootNode
            .endif
          .endif
        .endw
        .if eax != NULL
          mov [eax].RBT_NODE.dColor, scBLACK
        .endif
      .endif
      dec [esi].dCount
      mov eax, [esp + 24]                               ;pObject
    .endif
    pop esi
    pop edi
    pop ebx
    pop ebp
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.DeleteAll
; Purpose:   Wipes the complete tree.
; Arguments: None.
; Return:    Nothing.

Method RedBlackTree.DeleteAll, NOFRAME
    SetObject ecx,, [esp + 4]
    and [ecx].pRootNode, NULL
    and [ecx].dCount, 0
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.Dispose
; Purpose:   Removes an object from the tree and disposes it.
; Arguments: Arg1: -> Object
; Return:    Nothing.

Method RedBlackTree.Dispose, NOFRAME, pObject:Pointer
    OCall Pointer ptr [esp + 08]::RedBlackTree.Delete, Pointer ptr [esp + 08]
    Destroy eax                                         ;Safe operation
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
    .if eax != NULL
      invoke DisposeRBSubtree, eax
    .endif

    ;Dispose bigger sibling
    mov edx, [esp + 4]                                  ;pNode
    mov eax, [edx].RBT_NODE.pRightNode
    .if eax != NULL
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

Method RedBlackTree.Find, NOFRAME, pObject:Pointer
    push ebx
    push esi
    SetObject esi,, [esp + 12]
    mov ebx, [esi].pRootNode
    .while ebx != NULL
      mov eax, ebx
      sub eax, [esi].dNodeOffset
      OCall esi.Compare, Pointer ptr [esp + 20], eax    ;pObject
      .if eax == 0
        mov eax, ebx
        sub eax, [esi].dNodeOffset
        jmp @@Exit
      .endif
      .if Sign?                                         ;Check sign flag, if set, the result is < 0
        mov ebx, [ebx].RBT_NODE.pLeftNode
      .else
        mov ebx, [ebx].RBT_NODE.pRightNode
      .endif
    .endw
    xor eax, eax                                        ;eax = NULL => not found!
@@Exit:
    pop esi
    pop ebx
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.FirstThat
; Purpose:   Searchs the first object that doesn't return FALSE in eax.
; Arguments: Arg1: -> (static addr) Function that evaluates to TRUE or FALSE (return value in eax).
;            Arg2-n: (optional) Parameters to be used by the called procedure.
; Return:    eax -> Object or NULL if not found.

Method RedBlackTree.FirstThat, NOFRAME, pTestFunc:Pointer
    RedBlackTree_TraverseInOrder Left, Right, 1
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.FirstThatNot
; Purpose:   Searchs the first object that returns FALSE in eax.
; Arguments: Arg1: -> (static addr) Function that evaluates to TRUE or FALSE (return value in eax).
;            Arg2-n: (optional) Parameters to be used by the called procedure.
; Return:    eax -> object or NULL if not found.

Method RedBlackTree.FirstThatNot, NOFRAME, pTestFunc:Pointer
    RedBlackTree_TraverseInOrder Left, Right, 2
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.ForEach
; Purpose:   Traverses all nodes in the tree in ascending order.
; Arguments: Arg1: -> Processing procedure (static address).
;            Arg2-n: (Optional) Parameters to be used by the processing procedure.
; Return:    Nothing.
; Note:      While the loop is running, no object should be deleted or inserted!

Method RedBlackTree.ForEach, NOFRAME, pActionProc:Pointer
    RedBlackTree_TraverseInOrder Left, Right, 0
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.ForEachRev
; Purpose:   Traverses all objects in the tree in descending order.
; Arguments: Arg1: -> Processing procedure (static address).
;            Arg2-n: (Optional) Parameters to be used by the processing procedure.
; Return:    Nothing.
; Note:      While the loop is running, no object should be deleted or inserted!

Method RedBlackTree.ForEachRev, NOFRAME, pActionProc:Pointer
    RedBlackTree_TraverseInOrder Right, Left, 0
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.GetFirst
; Purpose:   Returns the object with the smallest key.
; Arguments: Nothing.
; Return:    eax -> Object or NULL if failed.

Method RedBlackTree.GetFirst, NOFRAME
    SetObject ecx,, [esp + 4]
    mov eax, [ecx].pRootNode
    RedBlackTree_GetMin
    sub eax, [ecx].dNodeOffset
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.GetLast
; Purpose:   Returns the object with the largest key.
; Arguments: Nothing.
; Return:    eax -> Object or NULL if failed.

Method RedBlackTree.GetLast, NOFRAME
    SetObject ecx,, [esp + 4]
    mov eax, [ecx].pRootNode
    RedBlackTree_GetMax
    sub eax, [ecx].dNodeOffset
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.GetNext
; Purpose:   Returns the object with a key that is larger than the key of the specified object.
; Arguments: Arg1: -> Object.
; Return:    eax -> Next object or NULL if failed.

Method RedBlackTree.GetNext, NOFRAME, pObject:Pointer
    SetObject ecx,, [esp + 4]                           ;Self
    mov edx, [esp + 8]                                  ;pObject
    add edx, [ecx].dNodeOffset
    mov eax, [edx].RBT_NODE.pRightNode
    .if eax != NULL
      RedBlackTree_GetMin
    .else
      .while [edx].RBT_NODE.pParentNode != NULL
        mov eax, [edx].RBT_NODE.pParentNode
        .break .if edx == [eax].RBT_NODE.pLeftNode
        mov edx, eax
      .endw
      mov eax, [edx].RBT_NODE.pParentNode
    .endif
    .if eax != NULL
      sub eax, [ecx].dNodeOffset
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.GetPrev
; Purpose:   Returns the object with a key that is smaller than the key of the specified object.
; Arguments: Arg1: -> Object.
; Return:    eax -> Previous object or NULL if failed.

Method RedBlackTree.GetPrev, NOFRAME, pObject:Pointer
    SetObject ecx,, [esp + 4]                           ;Self
    mov edx, [esp + 8]                                  ;pObject
    add edx, [ecx].dNodeOffset
    mov eax, [edx].RBT_NODE.pLeftNode
    .if eax != NULL
      RedBlackTree_GetMax
    .else
      .while [edx].RBT_NODE.pParentNode != NULL
        mov eax, [edx].RBT_NODE.pParentNode
        .break .if edx == [eax].RBT_NODE.pRightNode
        mov edx, eax
      .endw
      mov eax, [edx].RBT_NODE.pParentNode
    .endif
    .if eax != NULL
      sub eax, [ecx].dNodeOffset                        ;eax -> object/hosting data structure
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.Init
; Purpose:   Initializes the RedBlackTree object.
; Arguments: Arg1: -> Owner object.
;            Arg2: Node offset within hosting object (i.e. offset Object.RBTN, where RBTN is a
;                  RBT_NODE structure).
; Return:    Nothing.

Method RedBlackTree.Init, NOFRAME, pOwner:Pointer, dNodeOffset:dword
    SetObject ecx,, [esp + 4]
    m2m [ecx].dNodeOffset, [esp + 12], edx              ;dNodeOffset
    and [ecx].dCount, 0
    ACall ecx.Init, Pointer ptr [esp + 8]               ;pOwner
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.Insert
; Purpose:   Inserts an object in the tree. The object must host an uninitialized node at the
;            specified offset used in the Init method.
; Arguments: Arg1: -> Object.
; Return:    eax = Error code.
;            ecx -> (inserted) object.

Method RedBlackTree.Insert, NOFRAME, pObject:Pointer
    push ebp
    push ebx
    push edi
    push esi
    SetObject esi,, [esp + 20]                          ;Self
    mov eax, [esp + 24]                                 ;pObject
    add eax, [esi].dNodeOffset
    
    ;Initialize the node
    and [eax].RBT_NODE.pParentNode, NULL
    and [eax].RBT_NODE.pLeftNode, NULL
    and [eax].RBT_NODE.pRightNode, NULL
    mov [eax].RBT_NODE.dColor, scRED                    ;New inserted nodes are red!
    push eax
    
    mov ebx, [esi].pRootNode
    xor edi, edi

    .while ebx != NULL
      mov eax, [esp]
      sub eax, [esi].RedBlackTree.dNodeOffset
      mov edx, ebx
      sub edx, [esi].RedBlackTree.dNodeOffset
      mov ebp, $OCall(esi::RedBlackTree.Compare, eax, edx)
      .if eax == 0
        OCall esi::RedBlackTree.ErrorReport, NULL, RBT_NON_UNIQUE_KEY
        jmp @@Exit
      .endif
      mov edi, ebx
      .ifBitSet eax, BIT31
        mov ebx, [ebx].RBT_NODE.pLeftNode
      .else
        mov ebx, [ebx].RBT_NODE.pRightNode
      .endif
    .endw

    pop eax
    .if edi != NULL
      mov [eax].RBT_NODE.pParentNode, edi
      mov edx, edi
      .ifBitSet ebp, BIT31
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
        mov edx, [eax].RBT_NODE.pParentNode
        .if ecx != NULL && [ecx].RBT_NODE.dColor == scRED
          mov [edx].RBT_NODE.dColor, scBLACK
          mov [ecx].RBT_NODE.dColor, scBLACK
          mov eax, [edx].RBT_NODE.pParentNode
          mov [eax].RBT_NODE.dColor, scRED
        .else
          .if eax == [edx].RBT_NODE.pRightNode
            RedBlackTree_RotateLeft edx
          .endif
          mov edx, [eax].RBT_NODE.pParentNode
          mov [edx].RBT_NODE.dColor, scBLACK
          mov edx, [edx].RBT_NODE.pParentNode
          mov [edx].RBT_NODE.dColor, scRED
          RedBlackTree_RotateRight edx
        .endif
      .else
        mov edx, [edx].RBT_NODE.pParentNode
        mov ecx, [edx].RBT_NODE.pLeftNode
        mov edx, [eax].RBT_NODE.pParentNode
        .if ecx != NULL && [ecx].RBT_NODE.dColor == scRED
          mov [edx].RBT_NODE.dColor, scBLACK
          mov [ecx].RBT_NODE.dColor, scBLACK
          mov eax, [edx].RBT_NODE.pParentNode
          mov [eax].RBT_NODE.dColor, scRED
        .else
          .if eax == [edx].RBT_NODE.pLeftNode
            RedBlackTree_RotateRight edx
          .endif
          mov edx, [eax].RBT_NODE.pParentNode
          mov [edx].RBT_NODE.dColor, scBLACK
          mov edx, [edx].RBT_NODE.pParentNode
          mov [edx].RBT_NODE.dColor, scRED
          RedBlackTree_RotateLeft edx
        .endif
      .endif

      mov edx, [eax].RBT_NODE.pParentNode
    .endw

    mov eax, [esi].RedBlackTree.pRootNode
    mov [eax].RBT_NODE.dColor, scBLACK
    xor eax, eax                                        ;eax = OBJ_OK
    inc [esi].dCount

@@Exit:
    mov ecx, [esp + 24]                                 ;pObject
    pop esi
    pop edi
    pop ebx
    pop ebp
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.LastThat
; Purpose:   Searchs the last object that doesn't return FALSE in eax.
; Arguments: Arg1: -> (static addr) Function that evaluates to TRUE or FALSE (return value in eax).
;            Arg2-n: (optional) Parameters to be used by the called procedure.
; Return:    eax -> Object or NULL if not found.

Method RedBlackTree.LastThat, NOFRAME, pTestFunc:Pointer
    RedBlackTree_TraverseInOrder Right, Left, 1
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    RedBlackTree.LastThatNot
; Purpose:   Searchs the first object that returns FALSE in eax.
; Arguments: Arg1: -> (static addr) Function that evaluates to TRUE or FALSE (return value in eax).
;            Arg2-n: (optional) Parameters to be used by the called procedure.
; Return:    eax -> object or NULL if not found.

Method RedBlackTree.LastThatNot, NOFRAME, pTestFunc:Pointer
    RedBlackTree_TraverseInOrder Right, Left, 2
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
; Arguments: Arg1: -> Stream.
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

LoadRBSubtree proc pParentNode:PRBT_NODE                ;esi is passed from Load
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
      xor eax, eax
    .endif
    mov [edi].RBT_NODE.pLeftNode, eax                   ;Set left node

    pop eax
    .ifBitSet eax, BIT17
      invoke LoadRBSubtree, edi
    .else
      xor eax, eax
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
      xor eax, eax
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
    xor eax, eax
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
    .if ecx != NULL
      invoke StoreRBSubtree, ecx
    .endif
    mov ecx, [edi].RBT_NODE.pRightNode
    .if ecx != NULL
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
