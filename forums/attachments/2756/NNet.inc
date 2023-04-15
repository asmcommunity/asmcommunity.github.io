;A quick ripoff of T.Bleek's NeuralNetwork class
;I have no shame!
;EvilHomer, 2005

;Modified April 2009:
;   - use 16 bit data (real8)
;   - use externally-provided RandomNumberGenerator (RNG) object

NNETINFO_MEMUSE	        equ  1      ; Total memory use of network data
NNETINFO_TOTALNEURONS   equ  2      ; Total number of neurons in network
NNETINFO_CONNECTIONS    equ  3      ; Total number of connections in network
NNETINFO_SERDATASIZE    equ  4      ; Total data size of serialized network (for save method)

NNETFLAG_TOFILE         equ  1      ; Output data to file, outData is file handle
NNETFLAG_TOMEM          equ  2      ; Output data to memory, outData is pointer to memory

NNETFLAG_FROMFILE       equ  4      ; Read data from file, inData is file handle
NNETFLAG_FROMMEM        equ  8      ; Read data from mem, inData is mem pointer


NNETERR_NOERROR         equ  0      ; No error has occurred
NNETERR_INVALIDPARAM    equ  1      ; Invalid parameter
NNETERR_FILEACCESS      equ  2      ; File access failed (write or read)
NNETERR_INVALIDDATA     equ  3      ; Invalid serialized data

NNINFO struct
  nInputs         dword     ?       ; Number of inputs
  nOutputs        dword     ?       ; Number of outputs
  nHiddenLayers   dword     ?       ; Number of hidden layers
  nHiddenNeurons  dword     ?       ; Number of neurons in one hidden layer
NNINFO ends

NNETSERIAL_HEADER struct
  dwMagic         dword     ?       ; always "NNET" as dword (4E4E4554h)
  lnWeights       dword     ?       ; size of weight array
NNETSERIAL_HEADER ends


NNetID equ 48302
Object NNet, NNetID, Primer
  ; -------------------- <methods> ----------------------------
  RedefineMethod Init,              Pointer, Pointer, Pointer
  RedefineMethod Done
  VirtualMethod  train,             Pointer, Pointer
  VirtualMethod  initRandom
  VirtualMethod  calcOutputs
  VirtualMethod  calcOutputDeltas,  Pointer
  VirtualMethod  setLearningCoefficient, dword
  VirtualMethod  backPropagate
  VirtualMethod  run,               Pointer
  VirtualMethod  getInfo,           Pointer
  VirtualMethod  save,              dword, dword
  VirtualMethod  load,              dword, dword
  VirtualMethod  getLastError

  DefineVariable nInputs,           dword, 0
  DefineVariable nOutputs,          dword, 0
  DefineVariable nHiddenLayers,     dword, 0
  DefineVariable nHiddenNeurons,    dword, 0
  DefineVariable nLayerNeurons,     dword, 0       ; max number of neurons on one layer (of all layers)
  DefineVariable nTotalWeights,     dword, 0
 
  DefineVariable lpOutputArr,       Pointer, NULL  ; Output array
  DefineVariable lpWeightArr,       Pointer, NULL  ; Weights array
  DefineVariable lpDeltaArr,        Pointer, NULL  ; Temporary delta array
  DefineVariable lnOutputArr,       dword, 0       ; size of output array
  DefineVariable lnWeightArr,       dword, 0       ; size of weight array
  DefineVariable lnDeltaArr,        dword, 0       ; size of delta array

  DefineVariable lnLayerWeights,    dword, 0       ; nr of weights for one layer transition

  DefineVariable r4learnCoeff,      real4, 0.5f    ; learning coefficient
  DefineVariable dwLastError,       dword, 0
  
  DefineVariable random ;ptr to RNG random gen
ObjectEnd


if IMPLEMENT

.data
fRndRange  real4  65535.0
.code

Method NNet.Init, uses edi, lpOwner:Pointer, lpNNInfo:dword, pRNG:Pointer
    SetObject edi
    ACall edi.Init, lpOwner
    
    m2m [edi].random,pRNG,edx
    
    mov ecx, lpNNInfo
    assume ecx:ptr NNINFO

    DbgLine
    and [edi].dwLastError, 0

    ; --- Number of input neurons ---
    mov eax, [ecx].nInputs
    mov [edi].nInputs, eax
    mov edx, eax ;store #inputs in edx
    DbgDec eax, "Number of input neurons"

    ; --- Number of output neurons ---
    mov eax, [ecx].nOutputs
    mov [edi].nOutputs, eax
    cmp eax, edx
    jbe @F
    mov edx, eax ;store #outputs in edx if bigger than #inputs
@@:
    DbgDec eax, "Number of output neurons"
    DbgLine

    ; --- Number of hidden neurons per layer ---
    mov eax, [ecx].nHiddenNeurons
    mov [edi].nHiddenNeurons, eax
    cmp eax, edx
    jbe @F
    mov edx, eax ;store #hidden in edx if bigger than #inputs & #outputs
@@:
    DbgDec edx, "Number of hidden neurons per layer"

    ; --- Hidden layers ---
    mov eax, [ecx].nHiddenLayers
    mov [edi].nLayerNeurons, edx ;max neurons per layer (max of input,output and hidden)
    mov [edi].nHiddenLayers, eax
    DbgDec eax, "Number of hidden layers"

    assume ecx:nothing

    ; --- allocate memory for outputs ---

    ; hiddenlayers + 2 = total nr of layers
    add eax, 2
    DbgDec eax, "Total number of layers"
    DbgLine
    inc edx               ;add one to max neurons per layer, for bias.
    mul edx               ;total layers * (max neurons per layer + 1) = total output values

    DbgDec eax, "Total number of output values = (neuronsperlayer+1 * totallayers)"
    shl eax, 3            ; * 8 (real8 entries)
    mov [edi].lnOutputArr, eax

    mov [edi].lpOutputArr, $MemAlloc(eax, MEM_INIT_ZERO)

    ; --- allocate memory for weights ---

    mov eax, [edi].nLayerNeurons
    mov ecx, eax
    inc ecx               ;add one index for bias weight
    mul ecx               ;(layerneurons+1) * (layerneurons)
    mov [edi].lnLayerWeights, eax
    DbgDec eax,"Total number of weights per layer"
    mov ecx, [edi].nHiddenLayers
    inc ecx               ;(hiddenlayers+1) = (totallayers-1)
    mul ecx               ;(totallayers-1)*(layerneurons+1)*(layerneurons)
    DbgDec eax,"Total number of weights for all layers"
    mov [edi].nTotalWeights, eax
    shl eax, 3            ; * 8 (real8 entries)
    mov [edi].lnWeightArr, eax
    mov [edi].lpWeightArr, $MemAlloc(eax)

    ; --- allocate memory for temp delta array ---

    mov eax, [edi].nLayerNeurons
    inc eax               ;add one for bias
    shl eax, 4            ; * 2 (current&next) * 8 (real8 entries) = * 16
    mov [edi].lpDeltaArr, $MemAlloc(eax, MEM_INIT_ZERO)
    DbgLine

    xor ecx, ecx
    inc ecx
MethodEnd

;===============================================================================
; NNet::Destruct
;===============================================================================
Method NNet.Done, uses esi
    DbgWarning "NNet.Done"
    SetObject esi
    MemFree [esi].lpOutputArr
    .if [esi].lpWeightArr!=0
        MemFree [esi].lpWeightArr
    .endif
    MemFree [esi].lpDeltaArr
    DbgText "NNet is done"
MethodEnd

;===============================================================================
; NNet::initRandom
; Fill Weights array with random floats (-1 thru +1)
;===============================================================================
Method NNet.initRandom, uses ebx esi edi
    SetObject  edi
    mov esi, [edi].lpWeightArr
    mov ebx, [edi].lnWeightArr
    and [edi].dwLastError, 0
    shr ebx, 3

    ;Fill with floats between -1.0 and +1.0
    .while ebx
      OCall [edi].random::RNG.Real_Signed_Normal
      fstp real8 ptr [esi]
      dec ebx
      DbgFloat real8 ptr [esi]
      add esi, sizeof real8
    .endw
MethodEnd

;===============================================================================
; NNet::setLearningCoefficient
;===============================================================================
Method NNet.setLearningCoefficient,, learnCoeff:dword
    SetObject ecx
    mov eax, learnCoeff
    mov [ecx].r4learnCoeff, eax
    and [ecx].dwLastError, 0
MethodEnd

;===============================================================================
; NNet::train
; Input params : ptr to input values, ptr to EXPECTED output values
; Returns      : nothing
;===============================================================================
Method NNet.train, uses edi, lpInputs:Pointer, lpOutputs:Pointer
    SetObject edi
    and [edi].dwLastError, 0

    ; --- Copy inputs ---
    mov eax, [edi].lpOutputArr
    add eax, 8                        ; skip bias value
    mov ecx, [edi].nInputs
    shl ecx, 3
    invoke RtlMoveMemory, eax, lpInputs, ecx
    OCall edi.calcOutputs
    OCall edi.calcOutputDeltas, lpOutputs
    OCall edi.backPropagate
MethodEnd

;===============================================================================
; NNet::calcOutputs
;===============================================================================
Method NNet.calcOutputs, uses esi edi ebx
    local lnLeft:dword
    local lnRight:dword
    local lpCurLayerWeights:dword
    local dwCurLayer:dword
    local dwWeightSkipVal:dword
    local dwOutputSkipVal:dword

    SetObject edi
    and [edi].dwLastError, 0

    mov eax, [edi].nLayerNeurons
    shl eax, 3
    mov dwWeightSkipVal, eax          ; bytes to skip for next source neuron

    add eax, 8
    mov dwOutputSkipVal, eax          ; bytes to skip for next layer output

    mov eax, [edi].lpWeightArr
    mov lpCurLayerWeights, eax

    mov esi, [edi].lpOutputArr
    ; dwCurLayer: current transition calculated: dwCurLayer=0 means transition from layer1 to 2.
    and dwCurLayer, 0
    jmp @layerloop_end

@layerloop:
    ; ecx = dwCurLayer here

    ; get number of neurons in current left layer
    .if ecx == 0
      mov edx, [edi].nInputs
    .else
      mov edx, [edi].nHiddenNeurons
    .endif
    inc edx                           ; bias
    mov lnLeft, edx

    ; get number of neurons in current right layer
    .if ecx == [edi].nHiddenLayers
      mov edx, [edi].nOutputs
    .else
      mov edx, [edi].nHiddenNeurons
    .endif
    inc edx ;bias
    mov lnRight, edx


    mov edx, [edi].nLayerNeurons
    inc edx ;add bias
    shl edx, 3
    add edx, esi

    ; esi: current layer output data
    ; edx: next layer output data

    ; --- loop through all neurons in next (right) layer ---
    xor ebx, ebx
    inc ebx                           ;bias is not outputted
    .while ebx < lnRight
      mov eax, ebx                    ; (i - 1)*8
      dec eax
      shl eax, 3
      add eax, lpCurLayerWeights

      fld1
      fstp real8 ptr [esi]  ; 1.0 as dword, bias value

      fld1
      fldz
      xor ecx, ecx
      .while ecx < lnLeft
        fld real8 ptr [eax]           ; load weight value
        fmul real8 ptr [esi+ecx*8]    ; load input value
        add eax, dwWeightSkipVal
        faddp st(1), st(0)
        inc ecx
      .endw
      fchs
      fExpN
      fadd st(0), st(1)
      fdivp st(1), st(0)

      fstp real8 ptr [edx+ebx*8]

      inc ebx
    .endw

    mov eax, [edi].lnLayerWeights
    shl eax, 3
    add lpCurLayerWeights, eax
    add esi, dwOutputSkipVal
    inc dwCurLayer

@layerloop_end:
    mov ecx, dwCurLayer
    cmp ecx, [edi].nHiddenLayers
    jle @layerloop
MethodEnd


;===============================================================================
; NNet::calcOutputDeltas
;===============================================================================
Method NNet.calcOutputDeltas, uses esi edi ebx, lpOutputs:Pointer
    SetObject edi
    and [edi].dwLastError, 0

    ; delta = y . [1 - y] . [d - y]
    mov esi, [edi].nLayerNeurons
    inc esi   ;account for bias value
    shl esi, 3
    neg esi
    add esi, [edi].lpOutputArr
    add esi, [edi].lnOutputArr
    add esi, 8 ; skip bias

    mov ebx, lpOutputs
    mov edx, [edi].lpDeltaArr

    ; esi: points to produced output array (index 1, bias value skipped)
    ; ebx: points to desired output array
    ; edx: points to delta array.
    finit
    xor eax, eax
    mov ecx, [edi].nOutputs
    .while eax < ecx
      fld real8 ptr [esi+eax*8]       ; load y
      fld real8 ptr [ebx+eax*8]       ; load d
      fsub st(0), st(1)               ; st(0)=d-y, st(1)=y
      fld1
      fsub st(0), st(2)               ; st(0)=1-y, st(1)=d-y, st(2)=y
      fmulp st(1), st(0)              ; st(0)=[d-y]*[1-y], st(1)=y
      fmulp st(1), st(0)              ; st(0)=y*[1-y]*[d-y]=delta
      fstp real8 ptr [edx+eax*8+8]    ; store delta (+8 to skip delta)
      inc eax
    .endw
MethodEnd

;===============================================================================
; NNet::backPropagate
;===============================================================================
Method NNet.backPropagate, uses esi edi ebx
    local lnLeft:dword
    local lnRight:dword
    local lpCurLayerWeights:dword
    local dwCurLayer:dword
    local dwWeightSkipVal:dword
    local dwOutputSkipVal:dword
    local lpCurOutputs:dword
    local bSwapFlag:dword

    finit
    SetObject edi
    and [edi].dwLastError, 0

    and bSwapFlag, 0

    mov eax, [edi].nLayerNeurons
    shl eax, 3
    mov dwWeightSkipVal, eax          ; bytes to skip for prev source neuron

    add eax, 8
    mov dwOutputSkipVal, eax          ; bytes to skip for prev layer output

    mov eax, [edi].lpWeightArr
    add eax, [edi].lnWeightArr
    mov ecx, [edi].lnLayerWeights
    shl ecx, 3
    sub eax, ecx                      ; move to last layer transition
    mov lpCurLayerWeights, eax


    mov eax, [edi].nLayerNeurons
    inc eax                           ; account for bias value
    shl eax, 4                        ; * 8 for dword indexes, * 2 for two layers == * 16
    neg eax
    add eax, [edi].lpOutputArr
    add eax, [edi].lnOutputArr
    mov lpCurOutputs, eax


    ; dwCurLayer: current transition calculated: dwCurLayer=0 means transition from layer1 to 2.
    mov eax, [edi].nHiddenLayers
    inc eax
    mov dwCurLayer, eax
    jmp @layerloop_end

@layerloop:
    dec ecx
    ; ecx = dwCurLayer here

    ; get number of neurons in current left layer
    .if ecx == 0
      mov edx, [edi].nInputs
    .else
      mov edx, [edi].nHiddenNeurons
    .endif
    inc edx                           ; bias
    mov lnLeft, edx

    ; get number of neurons in current right layer
    .if ecx == [edi].nHiddenLayers
      mov edx, [edi].nOutputs
    .else
      mov edx, [edi].nHiddenNeurons
    .endif
    inc edx ;bias
    mov lnRight, edx


    mov esi, [edi].lpDeltaArr
    mov edx, esi
    .if bSwapFlag
      add esi, dwOutputSkipVal
    .else
      add edx, dwOutputSkipVal
    .endif

    mov eax, lpCurLayerWeights

    ; esi: points to delta array of right layer [in]
    ; edx: points to delta array of left layer [out]
    ; eax: points to current layer weights array [in]

    ; --- loop through all neurons in current (left) layer ---
    push edi

    mov edi, lpCurOutputs
    ; edi: points to output array of left layer [in]

    ; --- calculate left layer deltas loop ---
    xor ebx, ebx
    .while ebx < lnLeft
      xor ecx, ecx
      fldz                            ; start with zero for SUM
      inc ecx                         ; skip bias
      .while ecx < lnRight
        fld real8 ptr [eax+ecx*8-8]   ; load weight value (-4 is for the 1 in (i-1))
        fmul real8 ptr  [esi+ecx*8]   ; multiply by delta value of right neuron
        faddp st(1), st(0)            ; add to current sum
        inc ecx
      .endw
      ; out . [1 - out]

      fld real8 ptr [edi+ebx*8]       ; load output value of left neuron
      fld1
      fsub st(0), st(1)
      fmulp st(1), st(0)
      fmulp st(1), st(0)              ; out . [1-out]
      fstp real8 ptr [edx+ebx*8]      ; store delta value for neuron in left layer
      add eax, dwWeightSkipVal
      inc ebx
    .endw

    mov edx, edi                      ; edx takes over edi (ptr output array of left layer)
    pop edi

    mov eax, lpCurLayerWeights

    ; esi: points to delta array of right layer [in]
    ; edx: points to output array of left layer [in]
    ; eax: points to current layer weights array [out]

    fld [edi].r4learnCoeff            ; load learning coefficient
    ; --- adjust weights loop ---
    xor ebx, ebx
    .while ebx < lnLeft
      xor ecx, ecx
      inc ecx                         ; skip bias
      .while ecx < lnRight
        fld real8 ptr [edx+ebx*8]     ; load output of left neuron
        fmul real8 ptr [esi+ecx*8]    ; multiply by delta value of right neuron
        fmul st(0), st(1)             ; multiply by learning coefficient
        fadd real8 ptr [eax+ecx*8-8]  ; add delta to old weight value (-4 is for the 1 in (i-1))
        fstp real8 ptr [eax+ecx*8-8]  ; store new delta value
        inc ecx
      .endw
      add eax, dwWeightSkipVal
      inc ebx
    .endw

    fstp st(0)                        ; remove learning coefficient

    mov eax, [edi].lnLayerWeights
    shl eax, 3
    sub lpCurLayerWeights, eax
    mov eax, dwOutputSkipVal
    xor bSwapFlag, 1

    sub lpCurOutputs, eax
    dec dwCurLayer

@layerloop_end:
    mov ecx, dwCurLayer
    or ecx, ecx
    jnz @layerloop
MethodEnd

;===============================================================================
; NNet::run
; Input   : ptr to Neuron Input Values (real4)
; Returns : ptr to Neuron Output Value (real4)
;===============================================================================
Method NNet.run, uses edi, lpInputs:Pointer
    SetObject edi
    and [edi].dwLastError, 0

    ; --- Copy inputs ---
    mov eax, [edi].lpOutputArr
    add eax, 8 ;skip bias value
    mov ecx, [edi].nInputs
    shl ecx, 3
    invoke RtlMoveMemory, eax,lpInputs, ecx
    OCall calcOutputs
    mov eax, [edi].nLayerNeurons
    inc eax
    shl eax, 3
    neg eax
    add eax, [edi].lpOutputArr
    add eax, [edi].lnOutputArr
    add eax, 8
MethodEnd

;===============================================================================
; NNet::getInfo
;===============================================================================
Method NNet.getInfo, uses edi, dwType:dword
    SetObject edi
    and [edi].dwLastError, 0

    mov eax, dwType
    .if eax == NNETINFO_MEMUSE
      mov eax, [edi].lnWeightArr
      add eax, [edi].lnDeltaArr
      add eax, [edi].lnOutputArr
    .elseif eax == NNETINFO_TOTALNEURONS
      mov eax, [edi].nHiddenLayers
      mul [edi].nHiddenNeurons
      add eax, [edi].nInputs
      add eax, [edi].nOutputs
    .elseif  eax == NNETINFO_SERDATASIZE
      mov eax, [edi].lnWeightArr
      add eax, sizeof NNETSERIAL_HEADER
    .elseif eax == NNETINFO_CONNECTIONS
      mov eax, [edi].nInputs
      inc eax
      mul [edi].nHiddenNeurons
      mov ecx, eax
      mov eax, [edi].nHiddenLayers
      .if eax > 1
        dec eax
        mov edx, [edi].nHiddenNeurons
        inc edx
        mul edx
        mul [edi].nHiddenNeurons
        add ecx, eax
      .endif
      mov eax, [edi].nHiddenNeurons
      inc eax
      mul [edi].nOutputs
      add eax, ecx
    .else
      mov [edi].dwLastError, NNETERR_INVALIDPARAM
      or eax, -1
    .endif
MethodEnd

;===============================================================================
; NNet::save
;===============================================================================
Method NNet.save, uses edi, dwFlags:dword, outData:dword
    local netHeader:NNETSERIAL_HEADER
    local bytesWritten:dword

    SetObject edi
    and [edi].dwLastError, 0

    mov netHeader.dwMagic, "NNET"
    mov eax, [edi].lnWeightArr
    mov  netHeader.lnWeights, eax

    mov ecx, dwFlags
    .if (ecx & NNETFLAG_TOFILE)
      invoke  WriteFile, outData, addr netHeader, sizeof netHeader, addr bytesWritten, 0
      .if !eax
        mov [edi].dwLastError, NNETERR_FILEACCESS
        xor eax, eax
        ExitMethod
      .endif
      invoke WriteFile, outData, [edi].lpWeightArr, [edi].lnWeightArr, addr bytesWritten, 0
      .if !eax
        mov [edi].dwLastError, NNETERR_FILEACCESS
        xor eax, eax
        ExitMethod
      .endif
      xor eax, eax
      inc eax
    .elseif (ecx & NNETFLAG_TOMEM)
      push edi
      invoke  RtlMoveMemory, outData, addr netHeader,sizeof netHeader
      pop edi
      mov edx, outData
      add edx, sizeof netHeader
      invoke RtlMoveMemory, edx, [edi].lpWeightArr,[edi].lnWeightArr
      xor eax, eax
      inc eax
    .else
      mov [edi].dwLastError, NNETERR_INVALIDPARAM
      xor eax, eax
    .endif
MethodEnd

;===============================================================================
; NNet::load
;===============================================================================
Method NNet.load, uses edi, dwFlags:dword, inData:dword
    local netHeader:NNETSERIAL_HEADER
    local bytesRead:dword

    SetObject edi
    and [edi].dwLastError, 0

    mov ecx, dwFlags
    .if (ecx & NNETFLAG_FROMFILE)
      invoke ReadFile, inData, addr netHeader, sizeof netHeader, addr bytesRead, 0
      .if (!eax) || (bytesRead != sizeof netHeader)
        mov [edi].dwLastError, NNETERR_FILEACCESS
        xor eax, eax
        ExitMethod
      .endif
      mov eax, netHeader.lnWeights
      .if (netHeader.dwMagic != "NNET" || eax != [edi].lnWeightArr)
        mov [edi].dwLastError, NNETERR_INVALIDDATA
        xor eax, eax
        ExitMethod
      .endif
      invoke ReadFile, inData, [edi].lpWeightArr, [edi].lnWeightArr, addr bytesRead, 0
      mov ecx, netHeader.lnWeights
      .if (!eax) || (bytesRead != ecx)
        mov [edi].dwLastError, NNETERR_FILEACCESS
        xor eax, eax
        ExitMethod
      .endif
      xor eax, eax
      inc eax
    .elseif (ecx & NNETFLAG_FROMMEM)
      mov edx, inData
      assume edx:PTR NNETSERIAL_HEADER
      mov eax, [edx].lnWeights
      .if ([edx].dwMagic != "NNET" || eax != [edi].lnWeightArr)
        mov [edi].dwLastError, NNETERR_INVALIDDATA
        xor eax, eax
        ExitMethod
      .endif
      assume edx:nothing
      add edx, 8
      invoke RtlMoveMemory, [edi].lpWeightArr,edx, [edi].lnWeightArr
      xor eax, eax
      inc eax
    .else
      mov [edi].dwLastError, NNETERR_INVALIDPARAM
      xor eax, eax
    .endif
MethodEnd

;===============================================================================
; NNet::getLastError
;===============================================================================
Method NNet.getLastError
    SetObject eax
    mov eax, [eax].dwLastError
MethodEnd

endif