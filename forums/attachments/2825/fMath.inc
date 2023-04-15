; ==================================================================================================
; Title:   fMath.inc
; Author:  G. Friedrich
; Version: 1.0.8
; Purpose: Support of mathematical macros for ObjAsm32.
; Link:    http://www.rism.com/Trig/Trig02.htm
; Note:    Based on the work of Eóin O'Callaghan [Eóin] and bitRAKE.
;          Version 1.0.0, October 2003
;            - First release.
;          Version 1.0.1, January 2004
;            - fldStr / fstStr converted to macros. Since FloatToStr has several bugs, an
;              adapted routine of Raymond Filiatreault was used.
;            - Random moved to ObjMem32
;            - MULTI_FPU added by NaN.
;          Version 1.0.2, March 2004
;            - Stack corrections in fldReg, fCheckStatus and fSetPrecision made by Raymond.
;            - Comment notes corrected by Raymond.
;            - fReduceTrigRange corrected by Raymond.
;            - Small optimizations on stack space reservation.
;          Version 1.0.3, April 2004
;            - Conditional jumps added.
;            - fGetFlags added.
;            - fCheckStatus stack reservation corrected.
;            - fstReg, fstpReg and integer versions added.
;          Version 1.0.4, November 2004
;            - fcmp added, based on Ultrano's work.
;          Version 1.0.5, June 2005
;            - fSgn corrected by K. Usanov.
;          Version 1.0.6, January 2006
;            - fMin, fMax, fAbsMin, fAbsMax added by Homer.
;          Version 1.0.7, March 2006
;            - Local macro variables redefined as global symbols.
;          Version 1.0.8, March 2008
;            - fMod macro added.
;            - $IsLess, $IsLessOrEqual, $IsPositive, $IsNegative, $IsZero, $IsGreater,
;              $IsGreaterOrEqual macros added by Homer.
; ==================================================================================================

;Remark: The IEEE format uses a bias for the exponent - 7Fh for real4 format.
;        This bias allows floating point numbers to be compared with integer compares!
;        An exponent of 7Fh is zero - note the one bit is implied - so the number is one (1).

;General purpose functions
;fildReg        Loads a CPU register containing an integer onto the FPU stack.
;fistReg        Stores the integer part of the FPU st(0) register to a CPU register.
;fistpReg       Stores the integer part of the FPU st(0) register to a CPU register & pops it.
;fldReg         Loads a CPU register containing a real4 onto the FPU stack.
;fstReg         Stores the FPU st(0) register to a CPU register in real4 format.
;fstpReg        Stores and pops the FPU st(0) register to a CPU register in real4 format.
;fSgn           Computes {if st0 < 0 => -1, if st0 = 0 => 0, if st0 > 0 => 1}.
;fCheckStatus   Checks the status word of the FPU to detect fp computation errors.
;fGetFlags      Sets EFlags with FPU status flags.
;fUnload        Unloads several stack registers.
;fSetPrecision  Sets the precision bits in the control register.
;MULTI_FPU      Condenses FPU instructions onto one line.
;fMin           Returns the smallest value of 2 floats.
;fMax           Returns the largeest value of 2 floats.
;fAbsMin        Returns the absolute smallest value of 2 floats.
;fAbsMax        Returns the absolute largest value of 2 floats.

;Rounding functions
;fRnd           Rounds to an integer the content of st0.
;fRndUp         Rounds up to an integer the content of st0.
;fRndDn         Rounds down to an integer the content of st0.
;fInt           Computes the integer part of the content of st0.
;fFrac          Computes the fractional part of the content of st0.
;fRound         Rounds the content of st0 to x decimals.

;Power functions
;fExp2          Computes 2^x = 2^st0.
;fExpN          Computes e^x = e^st0.
;fPower         Computes x^y = st0^st(1).
;fLogN          Computes the neperian logarithm of st0.
;fExpT          Computes 10^x = 10^st0.
;fLogT          Computes the logarithm base 10 of st0.
;fLog           Computes Log(st0, st1) = Log2(st0)/Log2(st1).

;Trigonometric functions (Argument range |x| < 2^63 in radians)
;fFitTrigRange  Reduces the value in st0 to fit in the range of trig. functions: |x| < 2^63.
;fTan           Computes tan(st0).
;fSec           Computes 1 / cos(st0).
;fCsc           Computes 1 / sin(st0).
;fCot           Computes 1 / tan(st0).

;Inverse trigonometric functions (Argument range |x| <= 1, return value in radians)
;fArcSin        Computes arctan(st0 / sqrt((1 - st0)(1 + st0))).
;fArcCos        Computes arctan(sqrt((1 - st0)(1 + st0)) / st0).
;fArcTan        Computes arctan(st0).
;fArcSec        Computes arctan(sqrt((st0 - 1)(st0 + 1))).
;fArcCsc        Computes arctan(1 / sqrt((st0 - 1)(st0 + 1))).
;fArcCot        Computes arctan(1 / st0).

;Hyperbolic functions
;fSinh          Computes (exp(st0) - exp(-st0)) / 2.
;fCosh          Computes (exp(st0) + exp(-st0)) / 2.
;fTanh          Computes (exp(st0) - exp(-st0)) / (exp(st0) + exp(-st0)).
;fSech          Computes 2 / (exp(st0) + exp(-st0)).
;fCsch          Computes 2 / (exp(st0) - exp(-st0)).
;fCoth          Computes (exp(st0) + exp(-st0)) / (exp(st0) - exp(-st0)).

;Inverse Hyperbolic functions
;fArcSinh       Computes logN(st0 + Sqr(st0 * st0 + 1)).
;fArcCosh       Computes logN(st0 + Sqr(st0 * st0 - 1)).
;fArcTanh       Computes logN((1 + st0) / (1 - st0)) / 2.
;fArcSech       Computes logN((Sqr(-st0 * st0 + 1) + 1) / st0).
;fArcCsch       Computes logN((1 + sqrt(1 + st0*st0)) / st0).
;fArcCoth       Computes logN((st0 + 1) / (st0 - 1)) / 2.

;String representation
;fldStr         Loads in st0 the pointed string representation of a floating point number.
;fstStr         Stores in memory the string representation of the FP number in st0.

;Condition jumps
;fjg            Jump if Greater.
;fjl            Jump if Less.
;fje            Jump if Equal.
;fjne           Jump if Not Equal.
;fjge           Jump if Greater or Equal.
;fjle           Jump if Less or Equal.
;fjnc           Jump if Non-Comparable.

;CPU real4 comparison
;fcmp           Fast way to compare real4 floats with sign using the CPU.



fStatusWord record fBusy:1, fC3:1, fStackPtr:3, fC2:1, fC1:1, fC0:1, fExpt:1, fStackFault:1, \
                   fPrecision:1, fUnderflow:1, fOverflow:1, fZeroDiv:1, fDenorm:1, fInvOp:1

FPU_BUFFER struct
  wControlWord  word    ?
  wUnused0      word    ?
  wStatusWord   word    ?
  wUnused1      word    ?
  wTagWord      word    ?
  wUnused2      word    ?
  pInstruction  dword   ?
  wCodeSegment  word    ?
  wUnused3      word    ?
  pOperand      dword   ?
  wDataSegment  word    ?
  wUnused4      word    ?
  tSt0          tbyte   ?
  tSt1          tbyte   ?
  tSt2          tbyte   ?
  tSt3          tbyte   ?
  tSt4          tbyte   ?
  tSt5          tbyte   ?
  tSt6          tbyte   ?
  tSt7          tbyte   ?
FPU_BUFFER ends

.const
  r4MinusOne   real4    -1.0
  r4PlusOne    real4    +1.0

  r8PlusHalf   real8    +0.5
  r8MinusOne   real8    -1.0
  r8PlusOne    real8    +1.0
  r8PlusTwo    real8    +2.0


  f_OK                equ   0
  f_ERROR             equ   1
  f_INFP              equ   2
  f_INFN              equ   3
  f_NAN               equ   4
  f_DENOR             equ   5

  f_PRECISION_24BIT   equ   0
  f_PRECISION_53BIT   equ   BIT09
  f_PRECISION_64BIT   equ   BIT08 or BIT09

  f_NOR               equ   0
  f_SCI               equ   BIT00
  f_TRIM              equ   BIT01


.code

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     ($)(C/D/J)FP4
; Purpose:   Places a real4 FP number in the .const, .data or .code segment.
; Arguments: Arg1: Reference name (optional).
;            Arg2: FP4 number.
; Returns:   Nothing / Reference to the FP4 number.

CFP4 macro FP4Reference, Number:req
    .const
      FP4Reference  real4   Number
    .code
endm

DFP4 macro FP4Reference, Number:req
    .data
      FP4Reference  real4   Number
    .code
endm

JFP4 macro FP4Reference, Number:req
    local Lbl

    jmp Lbl
      FP4Reference  real4   Number
    Lbl:
endm

$CFP4 macro FP4Reference, Number:req
    .const
      FP4Reference  real4   Number
    .code
    exitm <FP4Reference>
endm

$DFP4 macro FP4Reference, Number:req
    .data
      FP4Reference  real4   Number
    .code
    exitm <FP4Reference>
endm

$JFP4 macro FP4Reference, Number:req
    local Lbl

    jmp Lbl
      FP4Reference  real4   Number
    Lbl:
    exitm <FP4Reference>
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     $Ofs(C/D/J)FP4
; Purpose:   Places a real4 FP number in the .const, .data or .code segment.
; Arguments: FP4 number.
; Returns:   FP4 number offset.

$OfsCFP4 macro Number:req
    .const
      ??RealLocation = $
      real4 Number
    .code
    exitm <offset ??RealLocation>
endm

$OfsDFP4 macro Number:req
    .data
      ??RealLocation = $
      real4 Number
    .code
    exitm <offset ??RealLocation>
endm

$OfsJFP4 macro Number:req
    local Lbl

    jmp Lbl
      ??RealLocation = $
      real4 Number
    Lbl:
    exitm <offset ??RealLocation>
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fildReg
; Purpose:   Loads a CPU register containing an integer onto the FPU stack.
; Arguments: Register name.
; Returns:   Nothing.
; Note:      st7 must be empty.

fildReg macro RegName:req
    if (opattr RegName) and 00010000b    ;;is RegName a register?
      ??RegSize = $GetSize(RegName)
      if ??RegSize eq 4
        push RegName
        fild dword ptr [esp]
        add esp, 4
      elseif ??RegSize eq 2
        push RegName
        fild word ptr [esp]
        add esp, 2
      elseif ??RegSize eq 1
        push word ptr 0
        mov byte ptr [esp], RegName
        fild word ptr [esp]
        add esp, 2
      else
        .err <fildReg: unsuported register type>
      endif
    else
      .err <fildReg: argument is not a register>
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fistReg
; Purpose:   Stores the integer part of the FPU st(0) register to a CPU register.
; Arguments: Register name.
; Returns:   Nothing.

fistReg macro RegName:req
    if (opattr RegName) and 00010000b    ;;is RegName a register?
      ??RegSize = $GetSize(RegName)
      if ??RegSize eq 4
        sub esp, 4
        fist dword ptr [esp]
        pop RegName
      elseif ??RegSize eq 2
        sub esp, 2
        fist word ptr [esp]
        pop RegName
      elseif ??RegSize eq 1
        sub esp, 2
        fist word ptr [esp]
        mov RegName, byte ptr [esp]
        add esp, 2
      else
        .err <fistReg: unsuported register type>
      endif
    else
      .err <fistReg: argument is not a register>
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fistpReg
; Purpose:   Stores the integer part of the FPU st(0) register to a CPU register & pops it.
; Arguments: Register name.
; Returns:   Nothing.

fistpReg macro RegName:req
    if (opattr RegName) and 00010000b    ;;is RegName a register?
      ??RegSize = $GetSize(RegName)
      if ??RegSize eq 4
        sub esp, 4
        fistp dword ptr [esp]
        pop RegName
      elseif ??RegSize eq 2
        sub esp, 2
        fistp word ptr [esp]
        pop RegName
      elseif ??RegSize eq 1
        sub esp, 2
        fistp word ptr [esp]
        mov RegName, byte ptr [esp]
        add esp, 2
      else
        .err <fistpReg: unsuported register type>
      endif
    else
      .err <fistpReg: argument is not a register>
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fldReg
; Purpose:   Loads a CPU register containing a real4 onto the FPU stack.
; Arguments: Register name.
; Returns:   Nothing.
; Note:      st7 must be empty.

fldReg macro RegName:req
    if (opattr RegName) and 00010000b    ;;is RegName a register?
      ??RegSize = $GetSize(RegName)
      if ??RegSize eq 4
        push RegName
        fld dword ptr [esp]
        add esp, 4
      elseif ??RegSize eq 2
        push RegName
        fld word ptr [esp]
        add esp, 2
      elseif ??RegSize eq 1
        sub esp, 2
        mov byte ptr [esp], RegName
        fld word ptr [esp]
        add esp, 2
      else
        .err <fldReg: unsuported register type>
      endif
    else
      .err <fldReg: argument is not a register>
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fstReg
; Purpose:   Stores the FPU st(0) register to a CPU register in real4 format.
; Arguments: Register name.
; Returns:   Nothing.

fstReg macro RegName:req
    if (opattr RegName) and 00010000b    ;;is RegName a register?
      ??RegSize = $GetSize(RegName)
      if ??RegSize eq 4
        sub esp, 4
        fst dword ptr [esp]
        pop RegName
      elseif ??RegSize eq 2
        sub esp, 2
        fst word ptr [esp]
        pop RegName
      elseif ??RegSize eq 1
        sub esp, 2
        fst word ptr [esp]
        mov RegName, byte ptr [esp]
        add esp, 2
      else
        .err <fstReg: unsuported register type>
      endif
    else
      .err <fstReg: argument is not a register>
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fstpReg
; Purpose:   Stores and pops the FPU st(0) register to a CPU register in real4 format.
; Arguments: Register name.
; Returns:   Nothing.

fstpReg macro RegName:req
    if (opattr RegName) and 00010000b    ;;is RegName a register?
      ??RegSize = $GetSize(RegName)
      if ??RegSize eq 4
        sub esp, 4
        fstp dword ptr [esp]
        pop RegName
      elseif ??RegSize eq 2
        sub esp, 2
        fstp word ptr [esp]
        pop RegName
      elseif ??RegSize eq 1
        sub esp, 2
        fstp word ptr [esp]
        mov RegName, byte ptr [esp]
        add esp, 2
      else
        .err <fstpReg: unsuported register type>
      endif
    else
      .err <fstpReg: argument is not a register>
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fSgn
; Purpose:   Computes the signum function of the content os st0.
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.
;            Requires .686 option.
;            Uses eax register.

fSgn macro
    sub esp, sizeof(real4)        ;;Reserve stack place of a real4
    fldz                          ;;Load zero in st0
    xor eax, eax                  ;;Load zero in eax
    fcomip st(0), st(1)           ;;Compare st0 and st1 and pop st0
    cmova eax, r4MinusOne         ;;Move if above
    cmovb eax, r4PlusOne          ;;Move if below
    fstp st(0)                    ;;Pop st0
    mov [esp], eax                ;;Load result in reserved room in stack
    fld dword ptr [esp]           ;;Load result in st0 from stack
    add esp, sizeof(real4)        ;;Restore stack
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fCheckStatus
; Purpose:   Checks the status word of the FPU to detect fp computation errors.
; Arguments: Status bit(s) to be checked.
; Returns:   Nothing.
; Note:      The result affects the zero flag, that can be used to decide.

fCheckStatus macro StatusBits:req
    push eax                      ;;Save eax
    fstsw ax                      ;;Store status word
    test ax, StatusBits           ;;Test the requested StatusWord bit
    pop eax                       ;;Restore eax without changing flags!
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fGetFlags
; Purpose:   Sets EFlags with FPU StatusWord flags.
;
;               15                    8
;              +———————————————————————+
;       +————— |  |C3|  |  |  |C2|C1|C0|   Coprocessor StatusWord register
;       |      +———————————————————————+
;       |
;       |        7                    0
;       |      +———————————————————————+
;       +————> |SF|ZF|  |AF|  |PF|  |CF|   Processor EFlags register
;              +———————————————————————+
;
;                           C3    C2    C0
;      If st(0) > source     0     0     0
;      If st(0) < source     0     0     1
;      If st(0) = source     1     0     0
;      If st(0) ? source     1     1     1
;
; Arguments: None.
; Returns:   Nothing.
; Note:      uses ax.

fGetFlags macro
    fnstsw ax                     ;;Store status word
    sahf                          ;;Set EFlags
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fSetPrecision
; Purpose:   Sets the precision bits in the control register.
; Arguments: Precision mask.
; Returns:   Nothing.

fSetPrecision macro PrecisionMask:req
    sub esp, 2                    ;;Reserve stack place
    fstcw word ptr [esp]          ;;Store FPU control word
    BitSet word ptr [esp], PrecisionMask    ;;Set precision bits
    fldcw word ptr [esp]          ;;Restore FPU control word
    add esp, 2                    ;;Restore stack
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     MULTI_FPU
; Purpose:   Condenses FPU instructions onto one line.
; Arguments: Various.
; Returns:   Nothing.
; Example:   MULTI_FPU fld1, fld1, fadd, fld1, fadd, fsqrt, fstp REAL8 ptr [edx]
;            Will produce 1.732 in the REAL8 pointed by edx.

MULTI_FPU MACRO Args:VARARG
    for ??Arg, <Args>
      ??Arg
    endm
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fMin
; Purpose:   Compares two floats, returns the smaller value on the FPU.
; Arguments: Arg1: first value.
;            Arg2: Second value.
; Returns:   Smallest value in st(0).
; Note:      uses ax.
;            st7 must be empty.

fMin macro val1:req, val2:req
    local @@1, @@2

    fld val1
    fcomp val2
    fjge @@1
    fld val1
    jmp @@2
@@1:
    fld val2
@@2:
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fMax
; Purpose:   Compares two floats, returns the larger on the FPU.
; Arguments: Arg1: first value.
;            Arg2: Second value.
; Returns:   Largest value in st(0).
; Note:      uses ax.
;            st7 must be empty.

fMax macro val1:req, val2:req
    local @@1, @@2

    fld val1
    fcomp val2
    fjle @@1
    fld val1
    jmp @@2
@@1:
    fld val2
@@2:
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fAbsMin
; Purpose:   Compares two floats, returns the absolute smaller value on the FPU.
; Arguments: Arg1: first value.
;            Arg2: Second value.
; Returns:   Smallest absolute value in st(0).
; Note:      uses ax.
;            st6 and st7 must be empty.

fAbsMin macro val1:req, val2:req
    local @@1, @@2

    fld val1
    fabs
    fld val2
    fabs
    fcompp
    fjle @@1
    fld val1
    jmp @@2
@@1:
    fld val2
@@2:
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fAbsMax
; Purpose:   Compares two floats, returns the absolute larger value on the FPU.
; Arguments: Arg1: first value.
;            Arg2: Second value.
; Returns:   Largest absolute value in st(0).
; Note:      uses ax.
;            st6 and st7 must be empty.

fAbsMax macro val1:req, val2:req
    local @@1, @@2

    fld val1
    fabs
    fld val2
    fabs
    fcompp
    fjge @@1
    fld val1
    jmp @@2
@@1:
    fld val2
@@2:
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fRnd
; Purpose:   Rounds the content of st0.
; Arguments: None.
; Returns:   Nothing.

fRnd macro
    sub esp, 2                    ;;Reserve stack place for one word
    fstcw word ptr [esp]          ;;Store FPU control word
    push word ptr [esp]           ;;Duplicate value
    BitClr word ptr [esp], (BIT10 or BIT11) ;;Modify the control word to return
                                  ;; rnd(x) to the next odd integer
    fldcw word ptr [esp]          ;;Restore modified FPU control word
    frndint                       ;;Round
    fldcw word ptr[esp + sizeof(word)]  ;;Restore previous FPU control word
    add esp, 2 * sizeof(word)     ;;Restore stack. Don't use pop eax. We wan't destroy it.
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fRndUp
; Purpose:   Rounds up the content of st0.
; Arguments: None.
; Returns:   Nothing.

fRndUp macro
    sub esp, 2                    ;;Reserve stack place for one word
    fstcw word ptr [esp]          ;;Store FPU control word
    push word ptr [esp]           ;;Duplicate value
    BitClr word ptr [esp], BIT10  ;;Modify the control word to
    BitSet word ptr [esp], BIT11  ;;  round to +inf
    fldcw word ptr [esp]          ;;Restore modified FPU control word
    frndint                       ;;Round up
    fldcw word ptr[esp + sizeof(word)]  ;;Restore previous FPU control word
    add esp, 2 * sizeof(word)     ;;Restore stack
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fRndDn
; Purpose:   Rounds down the content of st0.
; Arguments: None.
; Returns:   Nothing.

fRndDn macro
    sub esp, 2                    ;;Reserve stack place for one word
    fstcw word ptr [esp]          ;;Store FPU control word
    push word ptr [esp]           ;;Duplicate value
    BitSet word ptr [esp], BIT10  ;;Modify the control word to
    BitClr word ptr [esp], BIT11  ;;  round to -inf
    fldcw word ptr [esp]          ;;Restore modified FPU control word
    frndint                       ;;Round down
    fldcw word ptr[esp + sizeof(word)]  ;;Restore previous FPU control word
    add esp, 2 * sizeof(word)     ;;Restore stack
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fInt
; Purpose:   Calculates the integer part of the content of st0.
; Arguments: None.
; Returns:   Nothing.

fInt macro
    sub esp, 2                    ;;Reserve stack place for one word
    fstcw word ptr [esp]          ;;Store FPU control word
    push word ptr [esp]           ;;Duplicate value
    BitSet word ptr [esp], (BIT10 or BIT11) ;;Modify the control word to return int(x)
    fldcw word ptr [esp]          ;;Restore modified FPU control word
    frndint                       ;;Round down
    fldcw word ptr[esp + sizeof(word)]  ;;Restore previous FPU control word
    add esp, 2 * sizeof(word)     ;;Restore stack
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fFrac
; Purpose:   Calculates the fractional part of the content of st0.
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fFrac macro
    fld st(0)                     ;;Duplicate st0
    fInt                          ;;Compute int(st0)
    fsubp st(1), st(0)            ;;Frac(x) = x - Int(x)
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fRound
; Purpose:   Rounds the content of st0 to x decimals.
; Arguments: Decimals (dword).
; Returns:   Nothing.
; Note:      st5, st6 and st7 must be empty.

fRound macro dDecimals:=<0>
    push dword ptr dDecimals      ;;Store number of decimals
    fild dword ptr [esp]          ;;Load Decimals
    add esp, 4                    ;;Restore stack
    fExpT                         ;;Compute 10^x
    frndint                       ;;Round to an integer
    fxch                          ;;Swap st0 and st1
    fmul st(0), st(1)             ;;Compute Number * 10^Decimals
    fRnd                          ;;Compute fRnd
    fdivrp st(1), st(0)           ;;Compute Number / 10^Decimals
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fExp2
; Purpose:   Computes 2^x = 2^st0.
; Arguments: None.
; Returns:   Nothing.
; Note:      st5, st6 and st7 must be empty.

fExp2 macro                       ;;st0      st1      st2
    fld st(0)                     ;;x        x        ?         Duplicate st0
    frndint                       ;;int(x)   x        ?         Round to integer
    fsub st(1), st(0)             ;;int(x)   frac(x)  ?         Compute fractional part of x
    fld1                          ;;1        int(x)   frac(x)   Load 1
    fscale                        ;;2^int(x) int(x)   frac(x)   Scale 1 by int(x)
    fxch                          ;;int(x)   2^int(x) frac(x)   Exchange st0 and st1
    fstp st(0)                    ;;2^int(x) frac(x)  ?         Eliminate st0 and pop
    fxch                          ;;frac(x)  2^int(x) ?         Exchange st0 and st1
    f2xm1                         ;;2^frac-1 2^int(x) ?         Compute 2xm1
    fld1                          ;;1        2^frac-1 2^int(x)  Load 1
    faddp st(1), st(0)            ;;2^frac   2^int(x) ?         Add st0 and st1 and pop
    fmulp st(1), st(0)            ;;2^x      ?        ?         Multiplay st1 by st0 and pop
endm

fExp2_ macro                      ;;st0      st1      st2
    fld st(0)                     ;;x        x        ?         Duplicate st0
    frndint                       ;;int(x)   x        ?         Round to integer
    fsub st(1), st(0)             ;;int(x)   frac(x)  ?         Compute fractional part of x
    fxch                          ;;int(x)   2^int(x) frac(x)   Exchange st0 and st1
    f2xm1                         ;;2^frac-1 2^int(x) ?         Compute 2xm1
    fld1                          ;;1        int(x)   frac(x)   Load 1
    fadd
    fscale                        ;;2^int(x) int(x)   frac(x)   Scale 1 by int(x)
    fstp st(1)                    ;;2^int(x) frac(x)  ?         Eliminate st0 and pop
endm
; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fExpN
; Purpose:   Computes st(0) = e^st0.
; Arguments: None.
; Returns:   Nothing.
; Note:      st5, st6 and st7 must be empty.

fExpN macro
    fldl2e                        ;;Put Log2(e) onto the stack
    fmulp st(1),st(0)             ;;Compute x*Log2(e).
    fExp2                         ;;Compute 2^(x*Log2(e)) = e^x
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fLogN
; Purpose:   Calculates the neperian logarithm of st0 nad stores it in st0.
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fLogN macro
    fldln2                        ;;Put LogN(2) onto the stack
    fxch                          ;;Swap st0 and st1
    fyl2x                         ;;Compute LogN(2)*Log2(st0)
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fExpT
; Purpose:   Computes st0 = 10^st0.
; Arguments: None.
; Returns:   Nothing.
; Note:      st5, st6 and st7 must be empty.

fExpT macro
    fldl2t                        ;;Put Log2(10) onto the stack
    fmulp st(1),st(0)             ;;Compute x*Log2(10)
    fExp2                         ;;Compute 2^(x*Log2(10)) = 10^x
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fLogT
; Purpose:   Calculates the logarithm base 10 of st0 and sotres it in st0.
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fLogT macro
    fldlg2                        ;;Put Log10(2) onto the stack
    fxch                          ;;Swap st0 and st1
    fyl2x                         ;;Compute Log10(2)*Log2(st0)
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fPower
; Purpose:   Computes st0 = st0^st1 (x^y).
; Arguments: None.
; Returns:   Nothing.
; Note:      st5, st6 and st7 must be empty.

fPower macro
    fld1                          ;;Put 1 onto the stack
    fxch                          ;;Swap st0 and st1
    fyl2x                         ;;Compute 1*Log2(x)
    fmulp st(1),st(0)             ;;Compute y*Log2(x)
    fExp2                         ;;Comput2 2^(x*Log2(y)) = x^y
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fLog
; Purpose:   Calculates the logarithm base st1 of st0 and stores the result in st0.
; Arguments: None.
; Returns:   Nothing.
; Note:      st6 and st7 must be empty.
;            Log[st1](st0) = Log2(st0) / Log2(st1).

fLog macro
    fld1                          ;;Load 1
    fxch                          ;;Exchange with argument
    fyl2x                         ;;st1 = 1 * Log2(st0) and pop stack
    fxch                          ;;Exchange Log2(st0) with base
    fld1                          ;;Load 1
    fxch                          ;;Exchange 1 with base
    fyl2x                         ;;st1 = 1 * Log2(st0) and pop stack
    fdivp st(1), st(0)            ;;Log2(st0) / log2(st1)
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fFitTrigRange
; Purpose:   Reduces the value in st0 to fit in the range of trig. functions: |x| < 2^63.
; Arguments: None.
; Returns:   Nothing.
; Note:      st6 and st7 must be empty.
;            The result of fprem1 depends on the state of the FPU rounding bits.

fFitTrigRange macro
    local @@1

    fldpi                         ;;Load Pi
    fadd st(0), st(0)             ;;Double it
    fxch                          ;;Exchange argument and 2*Pi
@@1:
    fprem1                        ;;Reduce the exponent of st(0) by no more
    fnstsw ax                     ;;   than 63 in one execution
    sahf                          ;;Check FPU flags
    jp @@1                        ;;C2 flag in the FPU set => reduce again
    fstp st(1)                    ;;Eliminate 2*Pi
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fTan
; Purpose:   Calculates st0 = tan(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fTan macro
    fptan
    fstp st(0)                    ;;Pop st(0) from stack
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fSec
; Purpose:   Calculates st0 = Sec(st0).
; Arguments: None.
; Returns:   Nothing.

fSec macro
    fcos
    fdivr r8PlusOne              ;;Computes 1 / cos(x)
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fCsc
; Purpose:   Calculates st0 = Csc(st0).
; Arguments: None.
; Returns:   Nothing.

fCsc macro
    fsin
    fdivr r8PlusOne              ;;Computes 1 / sin(x)
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fCot
; Purpose:   Calculates st0 = Cot(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fCot macro
    fptan
    fdivrp st(1), st(0)           ;;Computes 1 / tan(x)
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fArcSin
; Purpose:   Calculates st0 = ArcSin(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fArcSin macro
    fld st(0)
    fmul st(0), st(0)
    fsubr r8PlusOne
    fsqrt
    fpatan
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fArcCos
; Purpose:   Calculates st0 = ArcCos(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fArcCos macro
    fld st(0)
    fmul st(0), st(0)
    fsubr r8PlusOne
    fsqrt
    fxch
    fpatan
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fArcTan
; Purpose:   Calculates st0 = ArcTan(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fArcTan macro
    fld1
    fpatan
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fArcSec
; Purpose:   Calculates st0 = ArcSec(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fArcSec macro
    fmul st(0), st(0)
    fsub r8PlusOne
    fsqrt
    fld1
    fpatan
endm


; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fArcCsc
; Purpose:   Calculates st0 = ArcCsc(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fArcCsc macro
    fmul st(0), st(0)
    fsub r8PlusOne
    fsqrt
    fld1
    fdivr st(1), st(0)
    fpatan
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fArcCot
; Purpose:   Calculates st0 = ArcCot(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fArcCot macro
    fdivr r8PlusOne
    fld1
    fpatan
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fSinh
; Purpose:   Calculates st0 = Sinh(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st5, st6 and st7 must be empty.

fSinh macro
    fExpN
    fld st(0)
    fdivr r8PlusOne
    fsub
    fmul r8PlusHalf
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fCosh
; Purpose:   Calculates st0 = Cosh(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st5, st6 and st7 must be empty.

fCosh macro
    fExpN
    fld st(0)
    fdivr r8PlusOne
    fadd
    fmul r8PlusHalf
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fTanh
; Purpose:   Calculates st0 = Tanh(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st5, st6 and st7 must be empty.

fTanh macro
    fExpN
    fmul st(0), st(0)
    fld st(0)
    fadd r8PlusOne
    fxch
    fsub r8PlusOne
    fdivr
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fSech
; Purpose:   Calculates st0 = Sech(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st5, st6 and st7 must be empty.

fSech macro
    fExpN
    fld st(0)
    fdivr r8PlusOne
    fadd
    fdivr r8PlusTwo
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fCsch
; Purpose:   Calculates st0 = Csch(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st5, st6 and st7 must be empty.

fCsch macro
    fExpN
    fld st(0)
    fdivr r8PlusOne
    fsub
    fdivr r8PlusTwo
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fCoth
; Purpose:   Calculates st0 = Coth(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st5, st6 and st7 must be empty.

fCoth macro
    fExpN
    fmul st(0), st(0)
    fld st(0)
    fadd r8PlusOne
    fxch
    fsub r8PlusOne
    fdiv
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fArcSinh
; Purpose:   Calculates st0 = ArcSinh(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fArcSinh macro
    fld st(0)
    fmul st(0), st(0)
    fadd r8PlusOne
    fsqrt
    fadd
    fLogN
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fArcCosh
; Purpose:   Calculates st0 = ArcCosh(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fArcCosh macro
    fld st(0)
    fmul st(0), st(0)
    fsub r8PlusOne
    fsqrt
    fadd
    fLogN
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fArcTanh
; Purpose:   Calculates st0 = ArcTanh(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fArcTanh macro
    fadd r8PlusOne
    fld st(0)
    fsubr r8PlusTwo
    fdiv
    fLogN
    fmul r8PlusHalf
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fArcSech
; Purpose:   Calculates st0 = ArcSech(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fArcSech macro
    fld st(0)
    fmul st(0), st(0)
    fsubr r8PlusOne
    fsqrt
    fadd r8PlusOne
    fdivr
    fLogN
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fArcCsch
; Purpose:   Calculates st0 = ArcCsch(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fArcCsch macro
    fld st(0)
    fmul st(0), st(0)
    fadd r8PlusOne
    fsqrt
    fadd r8PlusOne
    fdivr
    fLogN
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fArcCoth
; Purpose:   Calculates st0 = ArcCoth(st0).
; Arguments: None.
; Returns:   Nothing.
; Note:      st7 must be empty.

fArcCoth macro
    fadd r8PlusOne
    fld st(0)
    fsub r8PlusTwo
    fdiv
    fLogN
    fmul r8PlusHalf
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fUnload
; Purpose:   Unloads several stack registers
; Arguments: Number of FPU registers.
; Returns:   Nothing.

fUnload macro NbrRegs:=<1>
    repeat NbrRegs
      fstp st(0)
    endm
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fstStr
; Purpose:   Creates an ASCII string representation of the content of the st0 FPU register.
; Arguments: Arg1: -> destionation buffer (at best 20 bytes + padding bytes).
;            Arg2: Number of padding bytes.
;            Arg3: Number of decimal places.
;            Arg4: Format flag (f_NOR or f_SCI)
; Returns:   Operation code.
; Note:      st4, st5, st6 and st7 must be empty.
;            Destination buffer should have at least 20 bytes (add padding bytes)!

fstStr macro pDest:req, dPadding:=<0>, dDecimals:=<5>, dFlags:=<f_SCI>
    invoke St0ToStr, pDest, dPadding, dDecimals, dFlags
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fLdStr
; Purpose:   Loads an ASCII string representation of a floating point number into
;            the st0 FPU register.
; Arguments: -> ASCII string.
; Returns:   Operation code.
; Note:      st4, st5, st6 and st7 must be empty.

fldStr macro pSource:req
    invoke StrToSt0, pSource
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fjg
; Purpose:   Jump to label if the result of the previous fcmp instruction indicates greater.
; Arguments: Destination label.
; Returns:   Nothing.

fjg macro DestinationLabel:req
    fGetFlags
    ja DestinationLabel           ;;Conditional jump
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fjl
; Purpose:   Jump to label if the result of the previous fcmp instruction indicates less.
; Arguments: Destination label.
; Returns:   Nothing.

fjl macro DestinationLabel:req
    fGetFlags
    jb DestinationLabel           ;;Conditional jump
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fje
; Purpose:   Jump to label if the result of the previous fcmp instruction indicates equal.
; Arguments: Destination label.
; Returns:   Nothing.

fje macro DestinationLabel:req
    fGetFlags
    je DestinationLabel           ;;Conditional jump
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fjne
; Purpose:   Jump to label if the result of the previous fcmp instruction indicates not equal.
; Arguments: Destination label.
; Returns:   Nothing.

fjne macro DestinationLabel:req
    fGetFlags
    jne DestinationLabel          ;;Conditional jump
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fjge
; Purpose:   Jump to label if the result of the previous fcmp instruction indicates greater
;            or equal.
; Arguments: Destination label.
; Returns:   Nothing.

fjge macro DestinationLabel:req
    fGetFlags
    jae DestinationLabel          ;;Conditional jump
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fjle
; Purpose:   Jump to label if the result of the previous fcmp instruction indicates less
;            or equal.
; Arguments: Destination label.
; Returns:   Nothing.

fjle macro DestinationLabel:req
    fGetFlags
    jbe DestinationLabel          ;;Conditional jump
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fjnc
; Purpose:   Jump to label if the result of the previous fcmp instruction indicates that the
;            arguments are non-comparable.
; Arguments: Destination label.
; Returns:   Nothing.

fjnc macro DestinationLabel:req
    fGetFlags
    jp DestinationLabel           ;;Conditional jump
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fcmp
; Purpose:   Fast way to compare real4 floats with sign using the CPU.
; Arguments: Arg1: first comparand.
;            Arg2: Second comparand.
; Returns:   Modified flags.
; Note:      This macro distinguishs between -0.0 and +0.0!

fcmp macro var1:req, var2:req
   if (opattr var1) and 00010000b         ;;Is var1 a register?
     ??Reg1 textequ <var1>
     if (opattr var2) and 00010000b       ;;Is var2 a register?
       ??Reg2 textequ <var2>
     else
       ifdifi ??Reg1, <eax>
         ??Reg2 textequ <eax>
       elseifdifi ??Reg1, <ecx>
         ??Reg2 textequ <ecx>
       elseifdifi ??Reg1, <edx>
         ??Reg2 textequ <edx>
       endif
       mov ??Reg2, var2
     endif
   else
     if (opattr var2) and 00010000b       ;;Is var2 a register?
       ??Reg2 textequ <var2>
       ifdifi ??Reg2, <eax>
         ??Reg1 textequ <eax>
       elseifdifi ??Reg2, <ecx>
         ??Reg1 textequ <ecx>
       elseifdifi ??Reg2, <edx>
         ??Reg1 textequ <edx>
       endif
       mov ??Reg1, var1
     else
       ??Reg1 textequ <eax>
       ??Reg2 textequ <ecx>
       mov ??Reg1, var1
       mov ??Reg2, var2
     endif
   endif

   ifdifi ??Reg1, <eax>
     ifdifi ??Reg2, <eax>
       ??Reg3 textequ <eax>
     endif
   endif

   ifndef <??Reg3>
     ifdifi ??Reg1, <ecx>
       ifdifi ??Reg2, <ecx>
         ??Reg3 textequ <ecx>
       endif
     endif
   endif

   ifndef <??Reg3>
     ifdifi ??Reg1, <edx>
       ifdifi ??Reg2, <edx>
         ??Reg3 textequ <edx>
       endif
     endif
   endif

   mov ??Reg3, ??Reg1
   and ??Reg3, ??Reg2
   sar ??Reg3, 31
   xor ??Reg1, ??Reg3
   xor ??Reg2, ??Reg3
   cmp ??Reg1, ??Reg2
endm

;; ——————————————————————————————————————————————————————————————————————————————————————————————————
;; Macro:     ffjl
;; Purpose:   Jump to label if the result of the previous fcmp instruction indicates less.
;; Arguments: Destination label.
;; Returns:   Nothing.
;; Note:      uses ax.
;
;;               15                    8
;;              +———————————————————————+
;;       +————— |  |C3|  |  |  |C2|C1|C0|   Coprocessor StatusWord register
;;       |      +———————————————————————+
;;       |
;;       |        7                    0
;;       |      +———————————————————————+
;;       +————> |  |C3|  |  |  |C2|  |C0|   AX register
;;              +———————————————————————+
;;
;;                           C3    C2    C0
;;      If st(0) > source     0     0     0
;;      If st(0) < source     0     0     1
;;      If st(0) = source     1     0     0
;;      If st(0) ? source     1     1     1
;
;ffjl macro DestinationLabel
;    fnstsw ax
;    and ah, 01000101b
;    shr ah, 1
;    jna DestinationLabel        ;CF=1 and ZF=1
;endm
;
;; ——————————————————————————————————————————————————————————————————————————————————————————————————
;; Macro:     ffjg
;; Purpose:   Jump to label if the result of the previous fcmp instruction indicates greater.
;; Arguments: Destination label.
;; Returns:   Nothing.
;; Note:      uses ax.
;
;ffjg macro DestinationLabel
;    fnstsw ax
;    and ah, 01000101b
;    jz DestinationLabel
;endm
;
;; ——————————————————————————————————————————————————————————————————————————————————————————————————
;; Macro:     fcjz
;; Purpose:   compare and jump to label if the content of st(0) is zero.
;; Arguments: Destination label.
;; Returns:   Nothing.
;; Note:      uses ax.
;
;fcjz macro DestinationLabel
;    ftst
;    fnstsw ax
;    and ah, 01000000b
;    jnz DestinationLabel
;endm

;; ——————————————————————————————————————————————————————————————————————————————————————————————————
;; Macro:     ffjl
;; Purpose:   Calculate "arg1 mod arg2" (arg1 % arg2).

fMod macro arg1:req, arg2:req
    fld arg2
    fld arg1
    fprem
endm

;; ——————————————————————————————————————————————————————————————————————————————————————————————————
;; Macro:     $IsLess
;; Purpose:   Alternative to fMin. Is float A less than float B, returns eax = TRUE/FALSE.

$IsLess macro val1:req,val2:req
    local @@1, @@2

    fld val1
    fcomp val2
    fjge @@1
    mov eax, TRUE
    jmp @@2
@@1:
    xor eax, eax
@@2:
    exitm <eax>
endm

;; ——————————————————————————————————————————————————————————————————————————————————————————————————
;; Macro:     $IsLessOrEqual
;; Purpose:   If A <= B then return TRUE, else return FALSE.

$IsLessOrEqual macro val1:req,val2:req
    local @@1, @@2
    fld val1
    fcomp val2
    fjg @@1
    mov eax, TRUE
    jmp @@2
@@1:
    xor eax, eax
@@2:
    exitm <eax>
endm

;; ——————————————————————————————————————————————————————————————————————————————————————————————————
;; Macro:     $IsPositive
;; Purpose:   If val>0 => TRUE, else => FALSE.

$IsPositive macro val:req
    fld val
    ;fSgn
    fstpReg eax
    .ifBitSet eax,BIT31
      xor eax, eax        ;FALSE
    .else
      mov eax, TRUE
    .endif
    exitm <eax>
endm

;; ——————————————————————————————————————————————————————————————————————————————————————————————————
;; Macro:     $IsNegative
;; Purpose:   If val>0 => TRUE, else => FALSE.

$IsNegative macro val:req
    fld val
    ;fSgn
    fstpReg eax
    .ifBitClr eax, BIT31
      xor eax, eax        ;FALSE
    .else
      mov eax, TRUE
    .endif
    exitm <eax>
endm

;; ——————————————————————————————————————————————————————————————————————————————————————————————————
;; Macro:     $IsZero
;; Purpose:   If val=0 => TRUE, else => FALSE.

$IsZero macro val:req
    fld val
    fstpReg eax
    .if eax == 0 || eax == 80000000h
      mov eax, TRUE
    .else
       xor eax, eax       ;FALSE
    .endif
    exitm <eax>
endm

;; ——————————————————————————————————————————————————————————————————————————————————————————————————
;; Macro:     $IsGreater
;; Purpose:   Alternative to fMin. Is float A greater than float B, returns eax = TRUE/FALSE.

$IsGreater macro val1:req, val2:req
    local @@1, @@2
    fld val1
    fcomp val2
    fjge @@1
    mov eax,TRUE
    jmp @@2
@@1:
    xor eax, eax
@@2:
    exitm <eax>
endm



;; ——————————————————————————————————————————————————————————————————————————————————————————————————
;; Macro:     $IsGreaterOrEqual
;; Purpose:   If A >= B then return TRUE, else return FALSE.

$IsGreaterOrEqual macro val1:req, val2:req
    local @@1, @@2
    fld val1
    fcomp val2
    fjl @@1
    mov eax, TRUE
    jmp @@2
@@1:
    xor eax, eax
@@2:
    exitm <eax>
endm


$IsAbsGreater macro val1:req, val2:req
    local @@1, @@2
    fld val1
    fabs
    fld val2
    fabs
    fcompp
    fjge @@1
    mov eax,TRUE
    jmp @@2
@@1:
    xor eax, eax
@@2:
    exitm <eax>
endm
