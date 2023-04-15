; ==================================================================================================
; Title:   Pixelmap.inc
; Author:  G. Friedrich. Thanks to Homer for his support.
; Version: 1.0.3
; Purpose: Support of Pixelmap objects for ObjAsm32. Pixelmaps have a pixel color resolution of 8
;          bits per color channel and 8 bits for a alfa channel. Total color depth of 32 bits.
; Links:   - http://www.3dsoftware.com/Programming/WindowsAPI/DIB_Section/  => hSection info
;          - http://student.kuleuven.be/~m0216922/CG/index.html
;          - http://www.teamten.com/lawrence/graphics/gamma/
;          - http://www.graphics.cornell.edu/~westin/gamma/gamma.html
;          - http://www.poynton.com/notes/reducing_eyestrain/index.html
;          - http://msdn2.microsoft.com/en-us/library/ms534885.aspx
; Note:    Requires Gdi32 and OleAut32 libraries.
;
;          Transparency rules:
;          A. The alfa value of the Foreground pixel is used to calculazte the blending factors.
;          B. All color operations are done in the linear color space.
;          C. If a pixel is blended using a Setxxx method on another, the alfa value of the
;             resulting pixel is the value of the forground pixel.
;          D. If a pixel is blended using a Blendxxx method on another, the alfa value of the
;             resulting pixel is the value of the background pixel.
;
;          Version 1.0.0, August 2006
;            - First release.
;          Version 1.0.1, November 2006
;            - pCritSect added and Done method modified to support async access.
;          Version 1.0.2, November 2007
;            - Device context (hDC) added for each pixelmap instance.
;            - GetVirtualComponent/SetVirtualComponent added.
;            - Reset added.
;            - ForEachPixel iterator added.
;            - Stretch removed.
;            - Color channel definitions changed to match DirectX.
;          Version 1.0.3, February 2009
;            - pCritSect replaced by ObjLock.
;	   Version 1.0.4, June 2010
;            - Added Line, Circle and PrintAt methods
; ==================================================================================================


; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Gamma correction tables initialized with a Gamma value of 2.20.
; These tables can be reinitialized using the InitGammaTables method.

.data
Lin2GamTable label byte
  byte  000h,01Ch,025h,02Bh,030h,035h,039h,03Dh,040h,043h,046h,049h,04Bh,04Eh,050h,052h
  byte  054h,056h,058h,05Ah,05Ch,05Eh,060h,061h,063h,065h,066h,068h,069h,06Bh,06Ch,06Eh
  byte  06Fh,071h,072h,073h,075h,076h,077h,078h,07Ah,07Bh,07Ch,07Dh,07Eh,07Fh,081h,082h
  byte  083h,084h,085h,086h,087h,088h,089h,08Ah,08Bh,08Ch,08Dh,08Eh,08Fh,090h,091h,092h
  byte  093h,094h,095h,095h,096h,097h,098h,099h,09Ah,09Bh,09Bh,09Ch,09Dh,09Eh,09Fh,0A0h
  byte  0A0h,0A1h,0A2h,0A3h,0A4h,0A4h,0A5h,0A6h,0A7h,0A7h,0A8h,0A9h,0AAh,0AAh,0ABh,0ACh
  byte  0ADh,0ADh,0AEh,0AFh,0AFh,0B0h,0B1h,0B1h,0B2h,0B3h,0B3h,0B4h,0B5h,0B6h,0B6h,0B7h
  byte  0B7h,0B8h,0B9h,0B9h,0BAh,0BBh,0BBh,0BCh,0BDh,0BDh,0BEh,0BEh,0BFh,0C0h,0C0h,0C1h
  byte  0C2h,0C2h,0C3h,0C3h,0C4h,0C5h,0C5h,0C6h,0C6h,0C7h,0C7h,0C8h,0C9h,0C9h,0CAh,0CAh
  byte  0CBh,0CBh,0CCh,0CDh,0CDh,0CEh,0CEh,0CFh,0CFh,0D0h,0D0h,0D1h,0D1h,0D2h,0D3h,0D3h
  byte  0D4h,0D4h,0D5h,0D5h,0D6h,0D6h,0D7h,0D7h,0D8h,0D8h,0D9h,0D9h,0DAh,0DAh,0DBh,0DBh
  byte  0DCh,0DCh,0DDh,0DDh,0DEh,0DEh,0DFh,0DFh,0E0h,0E0h,0E1h,0E1h,0E2h,0E2h,0E3h,0E3h
  byte  0E4h,0E4h,0E5h,0E5h,0E6h,0E6h,0E6h,0E7h,0E7h,0E8h,0E8h,0E9h,0E9h,0EAh,0EAh,0EBh
  byte  0EBh,0EBh,0ECh,0ECh,0EDh,0EDh,0EEh,0EEh,0EFh,0EFh,0F0h,0F0h,0F0h,0F1h,0F1h,0F2h
  byte  0F2h,0F3h,0F3h,0F3h,0F4h,0F4h,0F5h,0F5h,0F6h,0F6h,0F6h,0F7h,0F7h,0F8h,0F8h,0F8h
  byte  0F9h,0F9h,0FAh,0FAh,0FBh,0FBh,0FBh,0FCh,0FCh,0FDh,0FDh,0FDh,0FEh,0FEh,0FFh,0FFh

Gam2LinTable label byte
  byte  000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
  byte  000h,000h,000h,000h,000h,000h,001h,001h,001h,001h,001h,001h,001h,001h,001h,001h
  byte  001h,002h,002h,002h,002h,002h,002h,002h,002h,003h,003h,003h,003h,003h,004h,004h
  byte  004h,004h,004h,005h,005h,005h,005h,006h,006h,006h,006h,007h,007h,007h,007h,008h
  byte  008h,008h,009h,009h,009h,00Ah,00Ah,00Ah,00Bh,00Bh,00Ch,00Ch,00Ch,00Dh,00Dh,00Eh
  byte  00Eh,00Fh,00Fh,00Fh,010h,010h,011h,011h,012h,012h,013h,013h,014h,014h,015h,016h
  byte  016h,017h,017h,018h,019h,019h,01Ah,01Ah,01Bh,01Ch,01Ch,01Dh,01Eh,01Eh,01Fh,020h
  byte  021h,021h,022h,023h,024h,024h,025h,026h,027h,028h,028h,029h,02Ah,02Bh,02Ch,02Dh
  byte  02Eh,02Eh,02Fh,030h,031h,032h,033h,034h,035h,036h,037h,038h,039h,03Ah,03Bh,03Ch
  byte  03Dh,03Eh,03Fh,040h,041h,043h,044h,045h,046h,047h,048h,049h,04Bh,04Ch,04Dh,04Eh
  byte  050h,051h,052h,053h,055h,056h,057h,059h,05Ah,05Bh,05Dh,05Eh,05Fh,061h,062h,063h
  byte  065h,066h,068h,069h,06Bh,06Ch,06Eh,06Fh,071h,072h,074h,075h,077h,079h,07Ah,07Ch
  byte  07Dh,07Fh,081h,082h,084h,086h,087h,089h,08Bh,08Dh,08Eh,090h,092h,094h,096h,097h
  byte  099h,09Bh,09Dh,09Fh,0A1h,0A3h,0A5h,0A6h,0A8h,0AAh,0ACh,0AEh,0B0h,0B2h,0B4h,0B6h
  byte  0B8h,0BAh,0BDh,0BFh,0C1h,0C3h,0C5h,0C7h,0C9h,0CCh,0CEh,0D0h,0D2h,0D4h,0D7h,0D9h
  byte  0DBh,0DDh,0E0h,0E2h,0E4h,0E7h,0E9h,0EBh,0EEh,0F0h,0F3h,0F5h,0F8h,0FAh,0FDh,0FFh


Object Pixelmap, PixelmapID, Streamable
  VirtualMethod   BlendPixel,           dword, dword, dword
  VirtualMethod   BlendVirtualPixel,    real4, real4, dword
  VirtualMethod   Clone
  VirtualMethod   CopyBmp,              Handle, Handle, RECT, POINT
  VirtualMethod   CopyDisplay,          Handle, RECT, POINT
  VirtualMethod   CopyPixelmap,         Pointer, RECT, POINT
  VirtualMethod   CreateDDB,            Handle                        ;Device Dependant Bitmap
  VirtualMethod   CreateDIB                                           ;Device Independent Bitmap
  RedefineMethod  Done
  VirtualMethod   ForEachPixel,         Pointer, ?
  VirtualMethod   GetPixel,             dword, dword
  VirtualMethod   GetVirtualComponent,  real4, real4, dword
  VirtualMethod   GetVirtualPixel,      real4, real4
  RedefineMethod  Init,                 Pointer, dword, dword         ;Constructor
  VirtualMethod   InitGammaTables,      real4                         ;Gamma value
  RedefineMethod  Load,                 Pointer, Pointer              ;Constructor; -> Stream, -> Owner
  VirtualMethod   LoadBmp,              Pointer, Handle, Handle       ;Constructor
  VirtualMethod   LoadFile,             Pointer, Pointer              ;Constructor
  VirtualMethod   Reset
  VirtualMethod   SaveFile,             Pointer, dword
  VirtualMethod   SetPixel,             dword, dword, dword
  VirtualMethod   SetVirtualComponent,  real4, real4, dword, dword
  VirtualMethod   SetVirtualPixel,      real4, real4, dword
  RedefineMethod  Store,                Pointer                       ;-> Stream
  VirtualMethod   UV2XY,                Pointer, Pointer, ?           ;Move to D3D_Pixelmap?

  Private
  VirtualMethod   Gamma2Linear,         dword
  VirtualMethod   Linear2Gamma,         dword
  PrivateEnd

  StaticMethod    AlphaBlend,           Pointer, RECT, POINT
  StaticMethod    Brightness,           dword
  StaticMethod    Convolve3x3,          Pointer, RECT, POINT, Pointer
  StaticMethod    FillBkGnd,            dword
  StaticMethod    FillRect,             RECT, dword
  StaticMethod    FillRectChannel,      RECT, dword, dword
  StaticMethod    FillRectGradHor,      RECT, dword, dword
  StaticMethod    FillRectGradVer,      RECT, dword, dword
  StaticMethod    FlipRectHor,          RECT
  StaticMethod    FlipRectVer,          RECT
  StaticMethod    Gray,                 RECT
  StaticMethod    Litho,                RECT, dword
  StaticMethod    Replace_ARGB_ARGB,    RECT, dword, dword
  StaticMethod    Replace_RGB_ARGB,     RECT, dword, dword
  StaticMethod    Replace_RGB_RGB,      RECT, dword, dword
  StaticMethod    SetAlphaMask,         Pointer, RECT, POINT, dword
  
  StaticMethod	  Create_Text,			HANDLE,LPSTR,POINT
  StaticMethod	  Circle,				dword,dword,dword,dword
  StaticMethod	  Line,					dword,dword,dword,dword,dword

  DefineVariable  dWidth,         dword,      0       ;Don't change the order of the
  DefineVariable  dHeight,        dword,      0       ;  following 2 dwords
  DefineVariable  hDC,            Handle,     0
  DefineVariable  hDIBSection,    Handle,     0
  DefineVariable  pPixels,        Pointer,    NULL    ;Memory block with pixel color information
  DefineVariable  pLin2GamTable,  Pointer,    offset Lin2GamTable
  DefineVariable  pGam2LinTable,  Pointer,    offset Gam2LinTable
  DefineVariable  ObjLock,        ObjectLock, {}

  DefineVariable  BmpInfo,        BITMAPINFO, {{sizeof BITMAPINFOHEADER,0,0,1,32,BI_RGB,0,0,0,0,0}}
  DefineVariable  hPrvBmp,        Handle,     0

ObjectEnd

 

; ——————————————————————————————————————————————————————————————————————————————————————————————————
;
;   Pixel format in memory (BGRA):                  Loaded into a register (ARGB):
;
;   ——————————————————————————————                  ——————————————————————————————
;  | Blue | Green |  Red  | Alpha |       =>       | Alpha | Red |  Green  | Blue |
;   ——————————————————————————————                  ——————————————————————————————
;


;Some usual 3x3 convolution filter data
;.const
;Blur              real4   +0.0625, +0.125, +0.0625, +0.125, +0.250, +0.125, +0.0625, +0.125, +0.0625
;GaussianBlur      real4   +0.0450, +0.122, +0.0450, +0.122, +0.332, +0.122, +0.0450, +0.122, +0.0450
;GaussianBlur2     real4   +0.0000, +0.200, +0.0000, +0.200, +0.200, +0.200, +0.0000, +0.200, +0.0000
;BoxBlur           real4   +0.1111, +0.111, +0.1111, +0.111, +0.111, +0.111, +0.1111, +0.111, +0.1111
;Sharpen           real4   +0.0000, -1.000, +0.0000, -1.000, +5.000, -1.000, +0.0000, -1.000, +0.0000
;Sharpen1          real4   -1.0000, -1.000, -1.0000, -1.000, +9.000, -1.000, -1.0000, -1.000, -1.0000
;Sharpen2          real4   -1.0000, -1.000, -1.0000, -1.000, +16.00, -1.000, -1.0000, -1.000, -1.0000
;EdgeDetect        real4   -0.1250, -0.125, -0.1250, -0.125, +1.000, -0.125, -0.1250, -0.125, -0.1250
;EdgeDetect2       real4   -1.0000, -1.000, -1.0000, -1.000, +8.000, -1.000, -1.0000, -1.000, -1.0000
;EdgeDetect3       real4   -5.0000, +0.000, +0.0000, +0.000, +0.000, +0.000, +0.0000, +0.000, +5.0000
;EdgeDetect4       real4   -1.0000, -1.000, -1.0000, +0.000, +0.000, +0.000, +1.0000, +1.000, +1.0000
;EdgeDetect5       real4   -1.0000, -1.000, -1.0000, +2.000, +2.000, +2.000, -1.0000, -1.000, -1.0000
;EdgeDetect6       real4   -5.0000, -5.000, -5.0000, -5.000, +39.00, -5.000, -5.0000, -5.000, -5.0000
;SobelHorizontal   real4   +1.0000, +2.000, +1.0000, +0.000, +0.000, +0.000, -1.0000, -2.000, -1.0000
;SobelVertical     real4   +1.0000, +0.000, -1.0000, +2.000, +0.000, -2.000, +1.0000, +0.000, -1.0000
;PrevitHorizontal  real4   +1.0000, +1.000, +1.0000, +0.000, +0.000, +0.000, -1.0000, -1.000, -1.0000
;PrevitVertical    real4   +1.0000, +0.000, -1.0000, +1.000, +0.000, -1.000, +1.0000, +0.000, -1.0000
;Emboss            real4   -2.0000, -1.000, -0.0000, -1.000, +1.000, +1.000, +0.0000, +1.000, +2.0000
;Engrave           real4   +2.0000, +1.000, +0.0000, +1.000, -1.000, -1.000, -0.0000, -1.000, -2.0000


PXM_CHANNEL_RED     equ   1
PXM_CHANNEL_BLUE    equ   2
PXM_CHANNEL_GREEN   equ   4
PXM_CHANNEL_ALPHA   equ   8

PXM_FORMAT_PXM      equ   0

PXM_ADDR_LIN        equ   0
PXM_ADDR_WRAP       equ   1
PXM_ADDR_MIRROR     equ   2
PXM_ADDR_CLAMP      equ   3


; ==================================================================================================

if IMPLEMENT

; ——————————————————————————————————————————————————————————————————————————————————————————————————

;Some used constants
PXM_MASK_0001             equ   0000000FFh
PXM_MASK_0010             equ   00000FF00h
PXM_MASK_0100             equ   000FF0000h
PXM_MASK_0101             equ   000FF00FFh
PXM_MASK_1010             equ   0FF00FF00h
PXM_MASK_REG_RGB          equ   000FFFFFFh
PXM_MASK_REG_ARGB         equ   0FFFFFFFFh
PXM_MASK_REG_ALPHA        equ   0FF000000h
PXM_MASK_REG_RED          equ   000FF0000h
PXM_MASK_REG_GREEN        equ   00000FF00h
PXM_MASK_REG_BLUE         equ   0000000FFh
PXM_MASK_MEM_RGB          equ   0FFFFFF00h
PXM_MASK_MEM_ARGB         equ   0FFFFFFFFh
PXM_MASK_MEM_ALPHA        equ   0000000FFh
PXM_MASK_MEM_RED          equ   00000FF00h
PXM_MASK_MEM_GREEN        equ   000FF0000h
PXM_MASK_MEM_BLUE         equ   0FF000000h
PXM_CHANNEL_OFFSET_BLUE   equ   0
PXM_CHANNEL_OFFSET_GREEN  equ   1
PXM_CHANNEL_OFFSET_RED    equ   2
PXM_CHANNEL_OFFSET_ALPHA  equ   3


.const
PXM_CHANNEL_OFFSET_TABLE   byte  -1, PXM_CHANNEL_OFFSET_RED, PXM_CHANNEL_OFFSET_BLUE, -1, \
                                 PXM_CHANNEL_OFFSET_GREEN, -1, -1, -1, PXM_CHANNEL_OFFSET_ALPHA
PXM_CHANNEL_MEM_MASK_TABLE dword PXM_MASK_MEM_ARGB, PXM_MASK_MEM_RED, PXM_MASK_MEM_BLUE, \
                                 PXM_MASK_MEM_ARGB, PXM_MASK_MEM_GREEN, PXM_MASK_MEM_ARGB, \
                                 PXM_MASK_MEM_ARGB, PXM_MASK_MEM_ARGB, PXM_MASK_MEM_ALPHA
PXM_CHANNEL_REG_MASK_TABLE dword PXM_MASK_REG_ARGB, PXM_MASK_REG_RED, PXM_MASK_REG_BLUE, \
                                 PXM_MASK_REG_ARGB, PXM_MASK_REG_GREEN, PXM_MASK_REG_ARGB, \
                                 PXM_MASK_REG_ARGB, PXM_MASK_REG_ARGB, PXM_MASK_REG_ALPHA

.code
; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Purpose: orders the RECT arguments so that RECT.left <= RECT.right and RECT.top <= RECT.bottom.
; Uses: eax

Pxm_FlipInvertedRect macro Rct:req
    mov eax, Rct.right
    .if sdword ptr eax < Rct.left
      push Rct.left
      mov Rct.left, eax
      pop Rct.right
    .endif
    mov eax, Rct.bottom
    .if sdword ptr eax < Rct.top
      push Rct.top
      mov Rct.top, eax
      pop Rct.bottom
    .endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Purpose: Calculates the intersection of 2 RECTs (my replacement of the IntersectRect API).
; Uses: eax, ecx

Pxm_ClipRect macro DstRct:req, SrcRct1:req, SrcRct2:req
    mov ecx, SrcRct1.left
    cmp ecx, SrcRct2.left
    if @Cpu and CPU_686
      cmovl ecx, SrcRct2.left
    else
      .if sdword ptr ecx < SrcRct2.left
        mov ecx, SrcRct2.left
      .endif
    endif
    mov eax, SrcRct1.right
    cmp eax, SrcRct2.right
    if @Cpu and CPU_686
      cmovg eax, SrcRct2.right
    else
      .if sdword ptr eax > SrcRct2.right
        mov eax, SrcRct2.right
      .endif
    endif
    cmp eax, ecx
    jle @@EOM                               ;Early method exit
    mov DstRct.left, ecx
    mov DstRct.right, eax
    mov ecx, SrcRct1.top
    cmp ecx, SrcRct2.top
    if @Cpu and CPU_686
      cmovl ecx, SrcRct2.top
    else
      .if sdword ptr ecx < SrcRct2.top
        mov ecx, SrcRct2.top
      .endif
    endif
    mov eax, SrcRct1.bottom
    cmp eax, SrcRct2.bottom
    if @Cpu and CPU_686
      cmovg eax, SrcRct2.bottom
    else
      .if sdword ptr eax > SrcRct2.bottom
        mov eax, SrcRct2.bottom
      .endif
    endif
    cmp eax, ecx
    jle @@EOM                               ;Early method exit
    mov DstRct.top, ecx
    mov DstRct.bottom, eax
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Purpose: Get address of a pixel in the pixelmap.
; Uses: eax

Pxm_PixelAddr macro XPos:req, YPos:req, ObjReg:=<esi>
    mov eax, YPos
    mul [ObjReg].dWidth
    add eax, XPos
    shl eax, 2
    add eax, [ObjReg].pPixels
endm

$Pxm_PixelAddr macro XPos:req, YPos:req, ObjReg:=<esi>
    Pxm_PixelAddr XPos, YPos, ObjReg
    exitm <eax>
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Purpose: Bytes to reach the next line
; Uses: eax

$Pxm_Bytes2Skip macro DimX:req, ObjReg:=<esi>
    mov eax, [ObjReg].dWidth
    sub eax, DimX
    shl eax, 2
    exitm <eax>
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Purpose: Transformation from destination to source space => DstRct/SrcPnt to SrcRct/DstPnt
; Uses: eax, ecx, edx

Dst2SrcSpace macro Rct:req, Pnt:req
    mov ecx, Pnt.x
    mov edx, Rct.left
    mov eax, Rct.right
    sub eax, edx
    add eax, ecx
    mov Rct.left, ecx
    mov Rct.right, eax
    mov Pnt.x, edx

    mov ecx, Pnt.y
    mov edx, Rct.top
    mov eax, Rct.bottom
    sub eax, edx
    add eax, ecx
    mov Rct.top, ecx
    mov Rct.bottom, eax
    mov Pnt.y, edx
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————

.code

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.BlendPixel
; Purpose:   Blends the ARGB value over another pixel.
; Arguments: Arg1: X location.
;            Arg2: Y location.
;            Arg3: ARGB value to blend.
; Return:    Nothing.

Method Pixelmap.BlendPixel, uses ebx esi, dCoorX:dword, dCoorY:dword, dARGB:dword
    local dDens1:dword, dDens2:dword
    SetObject esi
    .if [esi].hDIBSection != 0
      mov eax, dCoorY
      .if eax >= 0 && eax < [esi].dHeight
        mov ecx, dCoorX
        .if ecx >= 0 && ecx < [esi].dWidth
          shl ecx, 2
          add ecx, [esi].pPixels
          mul [esi].dWidth
          lea esi, [4*eax + ecx]
          mov edx, dARGB
          RGB2BGR edx
          push edx

          mov eax, edx
          shr eax, 32-8
          mov dDens1, eax
          neg ax
          add ax, 0FFh
          mov dDens2, eax                     ;Dens2 = Complement to dDens1

          ;Get background pixel and transform it into linear color space
          OCall Self::Pixelmap.Gamma2Linear, dword ptr [esi]
          mov ebx, eax
          and eax, PXM_MASK_0010
          and ebx, PXM_MASK_0101
          imul eax, dDens2
          imul ebx, dDens2

          pop ecx                             ;Get pixel to blend
          mov edx, ecx
          and ecx, PXM_MASK_0010
          and edx, PXM_MASK_0101
          imul ecx, dDens1
          imul edx, dDens1

          add eax, ecx                        ;Merge nibbles
          add ebx, edx
          and eax, PXM_MASK_0100
          and ebx, PXM_MASK_1010
          or eax, ebx
          shr eax, 8

          OCall Self::Pixelmap.Linear2Gamma, eax
          mov ecx, dword ptr [esi]            ;Set original alfa value
          and ecx, PXM_MASK_REG_ALPHA
          or eax, ecx
          mov dword ptr [esi], eax
        .else
          DbgWarning "X coordinate out of range in BlendPixel"
        .endif
      .else
        DbgWarning "Y coordinate out of range in BlendPixel"
      .endif
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.BlendVirtualPixel
; Purpose:   Sets an ARGB value of a given virtual pixel and performs a Gamma Correction on the
;            modified pixels. Alpha values of the background pixels are not changed.
; Arguments: Arg1: X location.
;            Arg2: Y location.
;            Arg3: new ARGB value.
; Return:    Nothing.

.const
d255 dword 255
.code

PXM_BlendColors macro
    mov dBackLinColor, $OCall(Self::Pixelmap.Gamma2Linear, dword ptr [esi])   ;ABGR

    movzx ecx, byte ptr dForeLinColor + 3           ;Alpha
    mov dBuffer, ecx
    fild dBuffer
    fmulp st(1), st(0)
    fidiv d255

    fld1                                            ;Calc complement to 1
    fsub st(0), st(1)

    movzx ecx, byte ptr dForeLinColor + 2           ;Blue
    mov dBuffer, ecx
    fild dBuffer
    movzx ecx, byte ptr dBackLinColor + 0
    fmul st(0), st(2)
    mov dBuffer, ecx
    fild dBuffer
    fmul st(0), st(2)
    faddp st(1), st(0)
    fistp sdBlue

    movzx ecx, byte ptr dForeLinColor + 1           ;Green
    mov dBuffer, ecx
    fild dBuffer
    movzx ecx, byte ptr dBackLinColor + 1
    fmul st(0), st(2)
    mov dBuffer, ecx
    fild dBuffer
    fmul st(0), st(2)
    faddp st(1), st(0)
    fistp sdGreen

    movzx ecx, byte ptr dForeLinColor + 0           ;Red
    mov dBuffer, ecx
    fild dBuffer
    movzx ecx, byte ptr dBackLinColor + 2
    fmul st(0), st(2)
    mov dBuffer, ecx
    fild dBuffer
    fmul st(0), st(2)
    faddp st(1), st(0)
    fistp sdRed

    fUnload

    ;Compose the new ABGR color and prevent a channel over- or underflow.
    if @Cpu and CPU_686
      mov edx, 255
      xor ecx, ecx
      mov eax, sdRed
      cmp eax, ecx
      cmovl eax, ecx
      cmp eax, edx
      cmovg eax, edx
      mov sdRed, eax

      mov eax, sdGreen
      cmp eax, ecx
      cmovl eax, ecx
      cmp eax, edx
      cmovg eax, edx
      mov sdGreen, eax

      mov eax, sdBlue
      cmp eax, ecx
      cmovl eax, ecx
      cmp eax, edx
      cmovg eax, edx
      mov sdBlue, eax
    else
      .if sdRed > 255
        mov sdRed, 255
      .elseif sdRed < 0
        and sdRed, 0
      .endif
      .if sdGreen > 255
        mov sdGreen, 255
      .elseif sdGreen < 0
        and sdGreen, 0
      .endif
      .if sdBlue > 255
        mov sdBlue, 255
      .elseif sdBlue < 0
        and sdBlue, 0
      .endif
    endif
    mov ah, byte ptr dBackLinColor + 3      ;Preserve background alfa value
    mov al, byte ptr sdRed
    shl eax, 16
    mov ah, byte ptr sdGreen
    mov al, byte ptr sdBlue

    mov dword ptr [esi], $OCall(Self::Pixelmap.Linear2Gamma, eax)
endm

Method Pixelmap.BlendVirtualPixel, uses esi, r4CoorX:real4, r4CoorY:real4, dARGB:dword
    local sdCoorX0:sdword, sdCoorX1:sdword, sdCoorY0:sdword, sdCoorY1:sdword
    local dBuffer:dword, wCurFpuCtrl:word, wPrvFpuCtrl:word
    local dSkipX:sdword, dSkipY:dword
    local sdRed:sdword, sdGreen:sdword, sdBlue:sdword
    local dBackLinColor:dword, dForeLinColor:dword

    SetObject esi
    fstcw wPrvFpuCtrl                         ;Store FPU control word
    m2m wCurFpuCtrl, wPrvFpuCtrl, ax          ;Duplicate value

    fld r4CoorY

    BitSet wCurFpuCtrl, BIT10                 ;Modify the control word to
    BitClr wCurFpuCtrl, BIT11                 ;  round to -inf
    fldcw wCurFpuCtrl                         ;Restore modified FPU control word
    fist sdCoorY0

    BitClr wCurFpuCtrl, BIT10                 ;Modify the control word to
    BitSet wCurFpuCtrl, BIT11                 ;  round to +inf
    fldcw wCurFpuCtrl                         ;Restore modified FPU control word
    fist sdCoorY1

    mov eax, sdCoorY1
    .if sdCoorY0 >= 0 && eax < [esi].dHeight
      fisub sdCoorY0                          ;B = y - y0
      xor edx, edx
      fld r4CoorX
      cmp eax, sdCoorY0
      je @F
      mov edx, [esi].dWidth
      shl edx, 2
@@:
      mov dSkipY, edx

      fist sdCoorX1

      BitSet wCurFpuCtrl, BIT10               ;Modify the control word to
      BitClr wCurFpuCtrl, BIT11               ;  round to -inf
      fldcw wCurFpuCtrl                       ;Restore modified FPU control word
      fist sdCoorX0

      mov ecx, sdCoorX1
      .if sdCoorX0 >= 0 && ecx < [esi].dWidth
        BitClr wCurFpuCtrl, (BIT10 or BIT11)  ;Modify the control word to return rnd(x)
        fldcw wCurFpuCtrl                     ;Restore modified FPU control word
        xor edx, edx
        fisub sdCoorX0                        ;A = x - x0, B = y - y0
        cmp ecx, sdCoorX0
        je @F
        mov edx, 4
@@:
        mov dSkipX, edx

        shl ecx, 2
        add ecx, [esi].pPixels
        mul [esi].dWidth
        lea esi, [4*eax + ecx]                ;esi -> Poisition right/bottom
        mov dForeLinColor, $OCall(Self::Pixelmap.Gamma2Linear, dARGB)   ;ARGB

        .if dSkipX == 0 && dSkipY == 0
          fUnload
          fld1
          PXM_BlendColors                     ;esi -> Position 00

        .elseif dSkipX == 0                   ;A = 0, B <> 0
          fUnload                             ;Discard A = 0
          fld1
          fsub st(0), st(1)                   ;1 - B, B
          fxch st(1)

          PXM_BlendColors                     ;esi -> Position 01
          fUnload
          sub esi, dSkipY                     ;esi -> Position 00
          PXM_BlendColors

        .elseif dSkipY == 0                   ;A <> 0, B = 0
          fxch st(1)
          fUnload                             ;Discard B = 0
          fld1
          fsub st(0), st(1)                   ;1 - A, A
          fxch st(1)

          PXM_BlendColors                     ;esi -> Position 10
          fUnload
          sub esi, dSkipX                     ;esi -> Position 00
          PXM_BlendColors

        .else
          fld st(0)
          fmul st(0), st(2)                   ;f11 = AxB
          fsub st(1), st(0)                   ;f10 = A - AxB
          fsub st(2), st(0)                   ;f01 = B - AxB
          fld1
          fsub st(0), st(1)
          fsub st(0), st(2)                   ;f00 = 1 - f11 - f10 - f01
          fsub st(0), st(3)                   ;f00, f11, f10, f01

          sub esi, dSkipX                     ;esi -> Poisition 01
          sub esi, dSkipY                     ;esi -> Poisition 00
          PXM_BlendColors
          fUnload
          add esi, dSkipX                     ;esi -> Poisition 10
          add esi, dSkipY                     ;esi -> Poisition 11
          PXM_BlendColors
          fUnload
          sub esi, dSkipY                     ;esi -> Poisition 10
          PXM_BlendColors
          fUnload
          sub esi, dSkipX                     ;esi -> Poisition 00
          add esi, dSkipY                     ;esi -> Poisition 01
          PXM_BlendColors
        .endif
        jmp @F
      .else
        DbgWarning "X coordinate out of range in BlendVirtualPixel"
      .endif
      fUnload
    .else
      DbgWarning "Y coordinate out of range in BlendVirtualPixel"
    .endif
@@:
    fUnload
    fldcw wPrvFpuCtrl                         ;Restore previous FPU control word
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.Clone
; Purpose:   Duplicates the Pixelmap object.
; Arguments: None.
; Return:    eax -> New cloned Pixelmap object or NULL if failed.

Method Pixelmap.Clone, uses esi
    SetObject esi
    New Pixelmap, Init, [esi].pOwner, [esi].dWidth, [esi].dHeight
    .if eax != NULL
      .if [eax].Pixelmap.dErrorCode == OBJ_OK
        push eax
        invoke MemClone, [eax].Pixelmap.pPixels, [esi].pPixels, \
                         [eax].Pixelmap.BmpInfo.BITMAPINFO.bmiHeader.biSizeImage
        pop eax
      .else
        Kill eax
        xor eax, eax
      .endif
    .else
      OCall esi.ErrorReport, NULL, PXM_OUT_OF_MEMORY
      xor eax, eax
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.CopyBmp
; Purpose:   Copies the pixels of a Bitmap (DDB or DIB) RECT to the destination on the pixelmap.
; Arguments: Arg1: Source DC handle compatible to the DDB. Don't need to be selected.
;            Arg2: Source Bitmap handle.
;            Arg3: Source RECT.
;            Arg4: Destination start point.
; Return:    eax = Error code.

Method Pixelmap.CopyBmp, uses edi esi, hDC:Handle, hBmp:Handle, SrcRct:RECT, DstPnt:POINT
    local dWidth:dword, dHeight:dword, dSrcSkipY:dword, dDstSkipY:dword
    local SrcSpace:RECT, DstSpace:RECT, OutRct:RECT
    local BmpInfo:BITMAPINFO, Bmp:BITMAP, pScanlineBuffer:Pointer

    SetObject edi
    .if [edi].hDIBSection != 0
      invoke GetObject, hBmp, sizeof Bmp, addr Bmp    ;Works only on memory DCs!!!!
      .if eax != 0
        ;Flip SrcRct if necessary
        Pxm_FlipInvertedRect SrcRct

        ;Clip SrcRct to SrcSpace
        and SrcSpace.left, 0
        and SrcSpace.top, 0
        m2m SrcSpace.right, Bmp.bmWidth, eax
        m2m SrcSpace.bottom, Bmp.bmHeight, eax
        Pxm_ClipRect OutRct, SrcRct, SrcSpace

        ;Clip to DstSpace
        mov eax, SrcRct.left
        sub eax, DstPnt.x
        mov DstSpace.left, eax
        add eax, [edi].dWidth
        mov DstSpace.right, eax
        mov eax, SrcRct.top
        sub eax, DstPnt.y
        mov DstSpace.top, eax
        add eax, [edi].dHeight
        mov DstSpace.bottom, eax
        Pxm_ClipRect OutRct, OutRct, DstSpace

        ;Calc transfer sizes
        sub eax, ecx
        jz @@EOM
        mov dHeight, eax
        mov ecx, OutRct.right                   ;Width of a scan line
        mov BmpInfo.BITMAPINFO.bmiHeader.biWidth, ecx
        sub ecx, OutRct.left
        jz @@EOM
        mov dWidth, ecx

        xor edx, edx
        mov BmpInfo.bmiHeader.biSize, sizeof BITMAPINFOHEADER
        neg eax                                 ;Top down bitmap
        mov BmpInfo.bmiHeader.biHeight, eax
        mov BmpInfo.bmiHeader.biPlanes, 1
        mov BmpInfo.bmiHeader.biBitCount, 32
        mov BmpInfo.bmiHeader.biCompression, edx
        mov BmpInfo.bmiHeader.biXPelsPerMeter, edx
        mov BmpInfo.bmiHeader.biYPelsPerMeter, edx
        mov BmpInfo.bmiHeader.biClrUsed, edx
        mov BmpInfo.bmiHeader.biClrImportant, edx

        mov eax, dHeight
        mul BmpInfo.BITMAPINFO.bmiHeader.biWidth
        shl eax, 2
        mov BmpInfo.BITMAPINFO.bmiHeader.biSizeImage, eax

        MemAlloc eax
        .if eax != NULL
          mov pScanlineBuffer, eax
          invoke GetDIBits, hDC, hBmp, 0, dHeight, pScanlineBuffer, addr BmpInfo, 0

          ;Now copy from Scanline Buffer to the Pixelmap
          ;Calc skip values
          mov dDstSkipY, $Pxm_Bytes2Skip(dWidth, edi)
          mov eax, OutRct.left
          shl eax, 2
          mov dSrcSkipY, eax

          ;Calc position of first pixel to transfer in destination space
          mov eax, OutRct.top
          sub eax, SrcRct.top
          add eax, DstPnt.y
          mul [edi].dWidth
          add eax, OutRct.left
          sub eax, SrcRct.left
          add eax, DstPnt.x
          shl eax, 2
          add eax, [edi].pPixels
          mov edi, eax

          ;Initialize transfer loop
          mov esi, pScanlineBuffer

          ;Do transfer of pixels
          mov edx, dHeight
          align ALIGN_CODE
@@NextLine:
          mov ecx, dWidth
          add esi, dSrcSkipY
          rep movsd
          add edi, dDstSkipY
          dec edx
          jnz @@NextLine

          MemFree pScanlineBuffer
          xor eax, eax                      ;Return OBJ_OK
        .else
          OCall edi.ErrorReport, NULL, PXM_OUT_OF_MEMORY
        .endif
      .endif
    .else
      OCall edi.ErrorReport, NULL, PXM_INVALID_BMP_HANDLE
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.CopyDisplay
; Purpose:   Copies the pixels of a display RECT to the destination on the pixelmap.
; Arguments: Arg1: Source Display Device Context.
;            Arg2: Source RECT.
;            Arg3: Destination start POINT.
; Return:    eax = Error code.

Method Pixelmap.CopyDisplay, uses esi, hDispDC:Handle, SrcRct:RECT, DstPnt:POINT
    local hMemDDB:Handle, hMemDC:Handle, hOldDDB:Handle
    local dWidth:dword, dHeight:dword

    SetObject esi
    .if [esi].hDIBSection != 0
      ;Flip SrcRct if necessary
      Pxm_FlipInvertedRect SrcRct
      mov eax, SrcRct.RECT.right
      sub eax, SrcRct.RECT.left
      jz @@EOM
      mov dWidth, eax
      mov ecx, SrcRct.RECT.bottom
      sub ecx, SrcRct.RECT.top
      jz @@EOM
      mov dHeight, ecx

      invoke CreateCompatibleDC, hDispDC
      .if eax != 0
        mov hMemDC, eax
        mov hMemDDB, $invoke(CreateCompatibleBitmap, hDispDC, dWidth, dHeight)
        mov hOldDDB, $invoke(SelectObject, hMemDC, hMemDDB)
        invoke BitBlt, hMemDC, 0, 0, dWidth, dHeight, hDispDC, SrcRct.left, SrcRct.top, SRCCOPY
        OCall esi.CopyBmp, hMemDC, hMemDDB, 0,0,dWidth,dHeight, DstPnt.x,DstPnt.y
        invoke DeleteObject, $invoke(SelectObject, hMemDC, hOldDDB)
        invoke DeleteObject, hMemDC
        xor eax, eax                        ;Return OBJ_OK
      .else
        OCall esi.ErrorReport, NULL, PXM_INVALID_DC_HANDLE
      .endif
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.CopyPixelmap
; Purpose:   Copies the pixels of a source Pixelmap RECT to the destination on the pixelmap.
; Arguments: Arg1: Source Pixelmap.
;            Arg2: Source RECT.
;            Arg3: Destination start POINT.
; Return:    Nothing.

Method Pixelmap.CopyPixelmap, uses edi esi, pSrcPxm:Pointer, SrcRct:RECT, DstPnt:POINT
    local dDstSkipY:dword, dSrcSkipY:dword
    local dWidth:dword, dHeight:dword
    local SrcSpace:RECT, DstSpace:RECT, OutRct:RECT

    SetObject edi
    SetObject esi,,pSrcPxm

    .if [esi].hDIBSection != 0 && [edi].hDIBSection != 0
      ;Flip SrcRct if necessary
      Pxm_FlipInvertedRect SrcRct

      ;Clip SrcRct to SrcSpace
      and SrcSpace.left, 0
      and SrcSpace.top, 0
      m2m SrcSpace.right, [esi].dWidth, eax
      m2m SrcSpace.bottom, [esi].dHeight, eax
      Pxm_ClipRect OutRct, SrcRct, SrcSpace

      ;Clip to DstSpace
      mov eax, SrcRct.left
      sub eax, DstPnt.x
      mov DstSpace.left, eax
      add eax, [edi].dWidth
      mov DstSpace.right, eax
      mov eax, SrcRct.top
      sub eax, DstPnt.y
      mov DstSpace.top, eax
      add eax, [edi].dHeight
      mov DstSpace.bottom, eax
      Pxm_ClipRect OutRct, OutRct, DstSpace

      ;Calc transfer sizes
      sub eax, ecx
      jz @@EOM
      mov dHeight, eax
      mov eax, OutRct.right
      sub eax, OutRct.left
      jz @@EOM
      mov dWidth, eax

      ;Calc skip values
      mov dDstSkipY, $Pxm_Bytes2Skip(dWidth, edi)
      mov dSrcSkipY, $Pxm_Bytes2Skip(dWidth, esi)

      ;Calc position of first pixel to transfer in source space
      mov esi, $Pxm_PixelAddr(OutRct.left, OutRct.top, esi)

      ;Calc position of first pixel to transfer in destination space
      mov eax, OutRct.top
      sub eax, SrcRct.top
      add eax, DstPnt.y
      mul [edi].dWidth
      add eax, OutRct.left
      sub eax, SrcRct.left
      add eax, DstPnt.x
      shl eax, 2
      add eax, [edi].pPixels
      mov edi, eax

      ;Do transfer of pixels
      mov edx, dHeight
      align ALIGN_CODE
@@NextLine:
      mov ecx, dWidth
      rep movsd
      add edi, dDstSkipY
      add esi, dSrcSkipY
      dec edx
      jnz @@NextLine
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.CreateDDB
; Purpose:   Creates a Device compatible Bitmap from the Pixelmap.
; Arguments: Arg1: Device Context (DC) handle.
; Return:    eax = Bitmap handle or zero if failed.

Method Pixelmap.CreateDDB, NOFRAME, hDC:Handle
    SetObject ecx,, [esp + 4]               ;Self
    .if [ecx].hDIBSection != 0
      lea eax, [ecx].BmpInfo
      invoke CreateDIBitmap, [esp + 28], eax, CBM_INIT, [ecx].pPixels, eax, 0
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.CreateDIB
; Purpose:   Creates A Device Independant Bitmap (DIB) from the Pixelmap.
; Arguments: None.
; Return:    eax = DIB handle or zero if failed.

Method Pixelmap.CreateDIB
    local Bmp:BITMAP

    SetObject ecx
    .if [ecx].hDIBSection != 0
      and Bmp.BITMAP.bmType, 0
      m2m Bmp.BITMAP.bmWidth, [ecx].dWidth, eax
      m2m Bmp.BITMAP.bmHeight, [ecx].dHeight, edx
      shl eax, 2
      mov Bmp.BITMAP.bmWidthBytes, eax
      mov Bmp.BITMAP.bmPlanes, 1
      mov Bmp.BITMAP.bmBitsPixel, 32
      m2m Bmp.BITMAP.bmBits, [ecx].pPixels, edx
      invoke CreateBitmapIndirect, esp      ;-> BITMAP structure
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Destructor: Pixelmap.Done
; Purpose:    Finalizes the Pixelmap object.
; Arguments:  None.
; Return:     Nothing.

Method Pixelmap.Done, uses esi
    SetObject esi
    OCall esi.Reset
    ACall esi::Pixelmap.Done
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.ForEachPixel
; Purpose:   Calls Action for each pixel in thePixelmap.
; Arguments: Arg1: -> Processing procedure.
;            Arg2-n: (0ptional) Parameters to be used by the processing procedure.
; Return:    Nothing.
; Note:      On entry, ecx holds the number of dwords passed to the processing procedure.

Method Pixelmap.ForEachPixel, NOFRAME, pActionProc:Pointer
    push ebp
    push ebx
    push edi
    push esi
    mov ebp, [esp + 24]             ;ebp = pActionProc
    SetObject esi,, [esp + 20]      ;Self
    mov edi, ecx                    ;edi = number to dword to pass to the called procedure (pActionProc)
    mov eax, [esi].dWidth
    mul [esi].dHeight
    mov ebx, eax                    ;ebx = number of pixels
    mov esi, [esi].pPixels
    ReleaseObject
    or ebx, ebx
    jmp @@4
align ALIGN_CODE
@@1:
    mov ecx, edi
    lea eax, [esp + 4*edi + 24]     ;eax -> last procedure parameter
    or ecx, ecx
    jmp @@3
align ALIGN_CODE
@@2:
    mov edx, [eax]                  ;Load parameter
    sub eax, 4
    push edx                        ;Put this value in the stack
    dec ecx
@@3:
    jnz @@2
    push esi
    call ebp                        ;Call procedure
    add esi, 4                      ;Point to next item the collection
    dec ebx
@@4:
    jnz @@1
    pop esi
    pop edi
    pop ebx
    pop ebp
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.GetPixel
; Purpose:   Returns the ARGB value of a given pixel.
; Arguments: Arg1: X location.
;            Arg2: Y location.
; Return:    eax = ARGB value.

Method Pixelmap.GetPixel,, dCoorX:dword, dCoorY:dword                                 ;NOFRAME!!!!!!!!
    SetObject edx
    mov eax, dCoorY
    .if eax >= 0 && eax < [edx].dHeight
      mov ecx, dCoorX
      .if ecx >= 0 && ecx < [edx].dWidth
        shl ecx, 2
        add ecx, [edx].pPixels
        mul [edx].dWidth
        mov eax, [4*eax + ecx]
        BGR2RGB eax
      .else
        DbgWarning "Y coordinate out of range in GetPixel"
        OCall edx.ErrorReport, NULL, PXM_OUT_OF_Y_RANGE
        xor eax, eax
      .endif
    .else
      DbgWarning "X coordinate out of range in GetPixel"
      OCall edx.ErrorReport, NULL, PXM_OUT_OF_X_RANGE
      xor eax, eax
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.GetVirtualComponent
; Purpose:   Returns the color component of a virtual pixel.
; Arguments: Arg1: X location (real4).
;            Arg2: Y location (real4).
;            Arg3: Color channel (PXM_CHANNEL_XXX).
; Return:    eax = Color component value [0..255].
;
;       —————————————— X
;       |  |  |  |  |
;       |——|——|——|——|—
;       |  |00|10|  |
;       |——|——|——|——|—
;       |  |01|11|  |
;       |——|——|——|——|—
;       |  |  |  |  |
;       Y

Method Pixelmap.GetVirtualComponent, uses ebx esi, r4CoorX:real4, r4CoorY:real4, dChannel:dword
    local sdCoorX0:sdword, sdCoorX1:sdword, sdCoorY0:sdword, sdCoorY1:sdword
    local dBuffer:dword, wCurFpuCtrl:word, wPrvFpuCtrl:word
    local dSkipX:sdword, dSkipY:dword
    local dColor00:dword, dColor10:dword, dColor01:dword, dColor11:dword

    SetObject esi
    fstcw wPrvFpuCtrl                         ;Store FPU control word
    m2m wCurFpuCtrl, wPrvFpuCtrl, ax          ;Duplicate value

    fld r4CoorY

    BitSet wCurFpuCtrl, BIT10                 ;Modify the control word to
    BitClr wCurFpuCtrl, BIT11                 ;  round to -inf
    fldcw wCurFpuCtrl                         ;Restore modified FPU control word
    fist sdCoorY0

    BitClr wCurFpuCtrl, BIT10                 ;Modify the control word to
    BitSet wCurFpuCtrl, BIT11                 ;  round to +inf
    fldcw wCurFpuCtrl                         ;Restore modified FPU control word
    fist sdCoorY1

    mov eax, sdCoorY1
    .if sdCoorY0 >= 0 && eax < [esi].dHeight
      fisub sdCoorY0
      xor edx, edx
      fld r4CoorX
      cmp eax, sdCoorY0
      je @F
      mov edx, [esi].dWidth
      shl edx, 2
@@:
      mov dSkipY, edx

      BitSet wCurFpuCtrl, BIT10               ;Modify the control word to
      BitClr wCurFpuCtrl, BIT11               ;  round to -inf
      fldcw wCurFpuCtrl                       ;Restore modified FPU control word
      fist sdCoorX0

      BitClr wCurFpuCtrl, BIT10               ;Modify the control word to
      BitSet wCurFpuCtrl, BIT11               ;  round to +inf
      fldcw wCurFpuCtrl                       ;Restore modified FPU control word
      fist sdCoorX1

      mov ecx, sdCoorX1
      .if sdCoorX0 >= 0 && ecx < [esi].dWidth
        BitClr wCurFpuCtrl, (BIT10 or BIT11)  ;Modify the control word to return rnd(x)
        fldcw wCurFpuCtrl                     ;Restore modified FPU control word
        mov ebx, [esi].pGam2LinTable
        fisub sdCoorX0                        ;A = x - x0, B = y - y0
        xor edx, edx
        fld st(0)
        cmp ecx, sdCoorX0
        je @F
        mov edx, 4
@@:
        mov dSkipX, edx

        fmul st(0), st(2)                     ;f11 = AxB
        shl ecx, 2
        fsub st(1), st(0)                     ;f10 = A - AxB
        add ecx, [esi].pPixels
        fsub st(2), st(0)                     ;f01 = B - AxB
        mul [esi].dWidth
        fld1
        lea edx, [4*eax + ecx]
        fsub st(0), st(1)
        fsub st(0), st(2)                     ;f00 = 1 - f11 - f10 - f01
        fsub st(0), st(3)                     ;f00, f11, f10, f01

        ;Get the 4 colors we need
        m2m dColor11, dword ptr [edx], eax
        sub edx, dSkipX
        m2m dColor01, dword ptr [edx], eax
        sub edx, dSkipY
        m2m dColor00, dword ptr [edx], eax
        add edx, dSkipX
        m2m dColor10, dword ptr [edx], eax

        ;Process the channel
        mov edx, dChannel
        .if edx == PXM_CHANNEL_ALPHA            ;Alpha channel ?
          movzx ecx, byte ptr [dColor00 + PXM_CHANNEL_OFFSET_ALPHA]
          mov dBuffer, ecx                      ;Alpha channel is linear per definition =>
          fild dBuffer                          ;  no need of of gamma correction
          movzx ecx, byte ptr [dColor11 + PXM_CHANNEL_OFFSET_ALPHA]
          fmul st(0), st(1)
          mov dBuffer, ecx
          fild dBuffer
          movzx ecx, byte ptr [dColor10 + PXM_CHANNEL_OFFSET_ALPHA]
          fmul st(0), st(3)
          mov dBuffer, ecx
          faddp st(1), st(0)
          movzx ecx, byte ptr [dColor01 + PXM_CHANNEL_OFFSET_ALPHA]
          fild dBuffer
          fmul st(0), st(4)
          mov dBuffer, ecx
          faddp st(1), st(0)
          fild dBuffer
          fmul st(0), st(5)
          faddp st(1), st(0)
          fistp dBuffer
          mov eax, dBuffer
        .else
          movzx edx, byte ptr [PXM_CHANNEL_OFFSET_TABLE + edx]  ;Convert channel to mem offset
          movzx ecx, byte ptr [dColor00 + edx]
          mov cl, byte ptr [ebx + ecx]          ;Gamma to linear color space
          mov dBuffer, ecx
          fild dBuffer
          movzx ecx, byte ptr [dColor11 + edx]
          mov cl, byte ptr [ebx + ecx]          ;Gamma to linear color space
          fmul st(0), st(1)
          mov dBuffer, ecx
          fild dBuffer
          movzx ecx, byte ptr [dColor10 + edx]
          mov cl, byte ptr [ebx + ecx]          ;Gamma to linear color space
          fmul st(0), st(3)
          mov dBuffer, ecx
          faddp st(1), st(0)
          movzx ecx, byte ptr [dColor01 + edx]
          fild dBuffer
          mov cl, byte ptr [ebx + ecx]          ;Gamma to linear color space
          fmul st(0), st(4)
          mov dBuffer, ecx
          faddp st(1), st(0)
          fild dBuffer
          fmul st(0), st(5)
          faddp st(1), st(0)
          fistp dBuffer
          mov edx, [esi].pLin2GamTable
          add edx, dBuffer
          movzx eax, byte ptr [edx]             ;Linear to Gamma space
        .endif
        fUnload 2

      .else
        DbgWarning "X coordinate out of range in GetVirtualComponent"
        OCall esi.ErrorReport, NULL, PXM_OUT_OF_X_RANGE
        xor eax, eax
      .endif
      fUnload
    .else
      DbgWarning "Y coordinate out of range in GetVirtualComponent"
      OCall esi.ErrorReport, NULL, PXM_OUT_OF_Y_RANGE
      xor eax, eax
    .endif
    fUnload
    fldcw wPrvFpuCtrl                         ;Restore previous FPU control word
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.GetVirtualPixel
; Purpose:   Returns the ARGB value of a given Virtual Pixel.
; Arguments: Arg1: X location (real4).
;            Arg2: Y location (real4).
; Return:    eax = ARGB value.
;
;       —————————————— X
;       |  |  |  |  |
;       |——|——|——|——|—
;       |  |00|10|  |
;       |——|——|——|——|—
;       |  |01|11|  |
;       |——|——|——|——|—
;       |  |  |  |  |
;       Y

PXM_AverangeChannel macro ChOfs:req
    movzx ecx, byte ptr [dColor00 + ChOfs]
    if ChOfs lt 3                           ;Alpha channel is linear per definition
      mov cl, byte ptr [ebx + ecx]          ;Gamma to linear color space
    endif
    mov dBuffer, ecx
    fild dBuffer
    movzx ecx, byte ptr [dColor11 + ChOfs]
    if ChOfs lt 3                           ;Alpha channel is linear per definition
      mov cl, byte ptr [ebx + ecx]          ;Gamma to linear color space
    endif
    fmul st(0), st(1)
    mov dBuffer, ecx
    fild dBuffer
    movzx ecx, byte ptr [dColor10 + ChOfs]
    if ChOfs lt 3                           ;Alpha channel is linear per definition
      mov cl, byte ptr [ebx + ecx]          ;Gamma to linear color space
    endif
    fmul st(0), st(3)
    mov dBuffer, ecx
    faddp st(1), st(0)
    movzx ecx, byte ptr [dColor01 + ChOfs]
    fild dBuffer
    if ChOfs lt 3                           ;Alpha channel is linear per definition
      mov cl, byte ptr [ebx + ecx]          ;Gamma to linear color space
    endif
    fmul st(0), st(4)
    mov dBuffer, ecx
    faddp st(1), st(0)
    fild dBuffer
    fmul st(0), st(5)
    faddp st(1), st(0)
endm

Method Pixelmap.GetVirtualPixel, uses ebx esi, r4CoorX:real4, r4CoorY:real4
    local sdCoorX0:sdword, sdCoorX1:sdword, sdCoorY0:sdword, sdCoorY1:sdword
    local dBuffer:dword, wCurFpuCtrl:word, wPrvFpuCtrl:word
    local dSkipX:sdword, dSkipY:dword
    local dRed:dword, dGreen:dword, dBlue:dword, dAlpha:dword
    local dColor00:dword, dColor10:dword, dColor01:dword, dColor11:dword

    SetObject esi
    fstcw wPrvFpuCtrl                         ;Store FPU control word
    m2m wCurFpuCtrl, wPrvFpuCtrl, ax          ;Duplicate value

    fld r4CoorY

    BitSet wCurFpuCtrl, BIT10                 ;Modify the control word to
    BitClr wCurFpuCtrl, BIT11                 ;  round to -inf
    fldcw wCurFpuCtrl                         ;Restore modified FPU control word
    fist sdCoorY0

    BitClr wCurFpuCtrl, BIT10                 ;Modify the control word to
    BitSet wCurFpuCtrl, BIT11                 ;  round to +inf
    fldcw wCurFpuCtrl                         ;Restore modified FPU control word
    fist sdCoorY1

    mov eax, sdCoorY1
    .if sdCoorY0 >= 0 && eax < [esi].dHeight
      fisub sdCoorY0
      xor edx, edx
      fld r4CoorX
      cmp eax, sdCoorY0
      je @F
      mov edx, [esi].dWidth
      shl edx, 2
@@:
      mov dSkipY, edx

      BitSet wCurFpuCtrl, BIT10               ;Modify the control word to
      BitClr wCurFpuCtrl, BIT11               ;  round to -inf
      fldcw wCurFpuCtrl                       ;Restore modified FPU control word
      fist sdCoorX0

      BitClr wCurFpuCtrl, BIT10               ;Modify the control word to
      BitSet wCurFpuCtrl, BIT11               ;  round to +inf
      fldcw wCurFpuCtrl                       ;Restore modified FPU control word
      fist sdCoorX1

      mov ecx, sdCoorX1
      .if sdCoorX0 >= 0 && ecx < [esi].dWidth
        BitClr wCurFpuCtrl, (BIT10 or BIT11)  ;Modify the control word to return rnd(x)
        fldcw wCurFpuCtrl                     ;Restore modified FPU control word
        mov ebx, [esi].pGam2LinTable
        fisub sdCoorX0                        ;A = x - x0, B = y - y0
        xor edx, edx
        fld st(0)
        cmp ecx, sdCoorX0
        je @F
        mov edx, 4
@@:
        mov dSkipX, edx

        fmul st(0), st(2)                     ;f11 = AxB
        shl ecx, 2
        fsub st(1), st(0)                     ;f10 = A - AxB
        add ecx, [esi].pPixels
        fsub st(2), st(0)                     ;f01 = B - AxB
        mul [esi].dWidth
        fld1
        lea edx, [4*eax + ecx]
        fsub st(0), st(1)
        fsub st(0), st(2)                     ;f00 = 1 - f11 - f10 - f01
        fsub st(0), st(3)                     ;f00, f11, f10, f01

        ;Get the 4 colors we need
        m2m dColor11, dword ptr [edx], eax
        sub edx, dSkipX
        m2m dColor01, dword ptr [edx], eax
        sub edx, dSkipY
        m2m dColor00, dword ptr [edx], eax
        add edx, dSkipX
        m2m dColor10, dword ptr [edx], eax

        ;Process each channel
        PXM_AverangeChannel PXM_CHANNEL_OFFSET_BLUE     ;Blue channel
        fistp dBlue
        PXM_AverangeChannel PXM_CHANNEL_OFFSET_GREEN    ;Green channel
        fistp dGreen
        PXM_AverangeChannel PXM_CHANNEL_OFFSET_RED      ;Red channel
        fistp dRed
        PXM_AverangeChannel PXM_CHANNEL_OFFSET_ALPHA    ;Alpha channel
        fistp dAlpha

        ;Compose it
        mov ah, byte ptr dAlpha
        fUnload
        mov al, byte ptr dBlue
        fUnload
        shl eax, 16
        mov ah, byte ptr dGreen
        mov al, byte ptr dRed
        OCall Self::Pixelmap.Linear2Gamma, eax

      .else
        DbgWarning "X coordinate out of range in GetVirtualPixel"
        OCall esi.ErrorReport, NULL, PXM_OUT_OF_X_RANGE
        xor eax, eax
      .endif
      fUnload
    .else
      DbgWarning "Y coordinate out of range in GetVirtualPixel"
      OCall esi.ErrorReport, NULL, PXM_OUT_OF_Y_RANGE
      xor eax, eax
    .endif
    fUnload
    fldcw wPrvFpuCtrl                         ;Restore previous FPU control word
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Constructor:  Pixelmap.Init
; Purpose:      Initializes the Pixelmap object
; Arguments:    Arg1: -> Owner object.
;               Arg2: Pixelmap width.
;               Arg3: Pixelmap height.
; Return:       Nothing.

Method Pixelmap.Init, uses esi, pOwner:Pointer, dWidth:dword, dHeight:dword
    local hDesktop:Handle, hDC:Handle

    SetObject esi
    ACall esi.Init, pOwner

    mov hDesktop, $invoke(GetDesktopWindow)
    mov hDC, $invoke(GetDC, hDesktop)
    mov [esi].hDC, $invoke(CreateCompatibleDC, hDC)
    invoke ReleaseDC, hDesktop, hDC

    m2m [esi].dWidth, dWidth, eax
    mov [esi].BmpInfo.bmiHeader.biWidth, eax
    m2m [esi].dHeight, dHeight, ecx
    mul ecx
    shl eax, 2
    mov [esi].BmpInfo.BITMAPINFO.bmiHeader.biSizeImage, eax
    neg ecx                                 ;Top down scanline arrangement
    mov [esi].BmpInfo.bmiHeader.biHeight, ecx

    invoke CreateDIBSection, 0, addr [esi].BmpInfo, 0, addr [esi].pPixels, 0, 0
    .if eax != 0
      mov [esi].hDIBSection, eax            ;System object handle
      mov [esi].hPrvBmp, $invoke(SelectObject, [esi].hDC, [esi].hDIBSection)
      invoke GdiFlush
    .else
      OCall esi.ErrorReport, NULL, PXM_INIT_FAILED
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.InitGammaTables
; Purpose:   Initializes 2 Gamma/Linear Space transformation tables. By default, all Pixelmap
;            instances share the same tables that are initialized with a Gamma value of 2.2.
;            If a single instance should use customized tables, the pGam2Lin and pLin2Gam pointers
;            have to be used to attach the new customized tables to the object instance.
; Arguments: Arg1: Gamma value. Usually a value of 2.2 or 2.5is used for standard monitors. 
;                  Value 1.0 is used to switch off the Gamma correction.
; Return:    Nothing.

Method Pixelmap.InitGammaTables, uses esi, r4Gamma:real4
    local dSrcBuffer:dword, dDstBuffer:dword
;    local Stm:DiskStream

    SetObject esi
    mov eax, 255
    mov dSrcBuffer, eax
    fild dSrcBuffer
    fld r4Gamma
    fld1
    fdivrp st(1), st(0)
    mov ecx, offset Lin2GamTable + 255
@@:
    fld st(0)                               ;Duplicate exponent
    fild dSrcBuffer
    fdiv st(0), st(3)
    fPower
    fmul st(0), st(2)
    fistp dDstBuffer
    mov dl, byte ptr dDstBuffer
    mov byte ptr [ecx], dl
    dec ecx
    dec dSrcBuffer
    jnz @B
    fUnload 2
    mov [esi].pLin2GamTable, offset Lin2GamTable

    mov eax, 255
    mov dSrcBuffer, eax
    fild dSrcBuffer
    fld r4Gamma
    mov ecx, offset Gam2LinTable + 255
@@:
    fld st(0)                               ;Duplicate exponent
    fild dSrcBuffer
    fdiv st(0), st(3)
    fPower
    fmul st(0), st(2)
    fistp dDstBuffer
    mov dl, byte ptr dDstBuffer
    mov byte ptr [ecx], dl
    dec ecx
    dec dSrcBuffer
    jnz @B
    fUnload 2
    mov [esi].pGam2LinTable, offset Gam2LinTable

;    LNew Stm, DiskStream
;    OCall Stm::DiskStream.Init, NULL, "Data.stm"
;    OCall Stm::DiskStream.BinWrite, offset Lin2GamTable, 256
;    OCall Stm::DiskStream.BinWrite, offset Gam2LinTable, 256
;    OCall Stm::DiskStream.Done

MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Constructor:  Pixelmap.Load
; Purpose:      Loads the Pixelmap from a stream object.
; Arguments:    Arg1: -> Stream object.
;               Arg2: -> Owner object.
; Return:       Nothing.

Method Pixelmap.Load, uses esi, pStream:Pointer, pOwner:Pointer
    SetObject esi
    OCall pStream::Stream.BinRead, addr [esi].dWidth, 2 * 4
    OCall esi.Init, pOwner, [esi].dWidth, [esi].dHeight
    .if [esi].hDIBSection != 0
      OCall pStream::Stream.BinRead, [esi].pPixels, [esi].BmpInfo.BITMAPINFO.bmiHeader.biSizeImage
    .else
      OCall esi.ErrorReport, NULL, PXM_STREAMLOAD_FAILED
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Constructor:  Pixelmap.LoadBmp
; Purpose:      Copies the pixels of a Bitmap (DDB or DIB) on the pixelmap.
; Arguments:    Arg1: -> Owner.
;               Arg2: Source Device Context handle.
;               Arg3: Source Bitmap handle.
; Return:       Nothing.

Method Pixelmap.LoadBmp, uses esi, pOwner:Pointer, hDC:Handle, hBmp:Handle
    local Bmp:BITMAP

    SetObject esi
    invoke GetObject, hBmp, sizeof Bmp, addr Bmp
    .if eax != 0
      OCall esi.Init, pOwner, Bmp.BITMAP.bmWidth, Bmp.BITMAP.bmHeight
      invoke GetDIBits, hDC, hBmp, 0, [esi].dHeight, [esi].pPixels, addr [esi].BmpInfo, 0
      .if eax == 0
        OCall esi.ErrorReport, NULL, PXM_LOADBMP_FAILED
      .endif
    .else
      OCall esi.ErrorReport, NULL, PXM_INVALID_BMP_HANDLE
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Constructor:  Pixelmap.LoadFile
; Purpose:      Loads a Bitmap from a file.
; Arguments:    Arg1: -> Owner object.
;               Arg2: -> File Name.
; Return:       Nothing.

ifndef IID_IPicture
  DefGUID IID_IPicture, %sIID_IPicture
endif

HIMETRIC_INCH   equ   2540      ;25.4 mm/inch * 100

Method Pixelmap.LoadFile, uses ebx esi, pOwner:Pointer, pFileName:Pointer
    local pWideFileName:Pointer, pPicture:Pointer
    local hTmpDC:Handle, hCmpDC:Handle, hPrvBmp:Handle
    local dHmWidth:dword, dHmHeight:dword, dNegHmHeight:dword
    local hDDB:Handle, bFileName[MAX_PATH + 1]:byte

    SetObject esi
    invoke GetFullPathName, pFileName, sizeof bFileName, addr bFileName, NULL
    .if eax < sizeof bFileName
      .if eax != 0
        ;First, we need the filename in Unicode
        invoke UStrAlloc, eax
        .if eax == NULL
          OCall esi.ErrorReport, NULL, PXM_OUT_OF_MEMORY
        .else
          mov pWideFileName, eax
          invoke Str2UStr, pWideFileName, addr bFileName
          ;Now we can create out picture object
          lea eax, pPicture
          invoke OleLoadPicturePath, pWideFileName, NULL, NULL, NULL, offset IID_IPicture, eax
          .if FAILED(eax)
            ;We didn't get the file to open, assume bad filename
            invoke UStrDispose, pWideFileName
            OCall esi.ErrorReport, NULL, PXM_LOADFILE_FAILED
          .else
            ;Now we are ready to get the hBipmap, we farm this out for reuseability
            mov hCmpDC, $invoke(GetDC, 0)           ;Get a DC to work with => Screen DC
            .if $invoke(CreateCompatibleDC, eax) == 0
              xor ebx, ebx
            .else
              mov hTmpDC, eax
              ;Read out the width and height of the IPicture object
              ICall pPicture::IPicture.get_Width, addr dHmWidth
              ICall pPicture::IPicture.get_Height, addr dHmHeight

              ;Convert himetric to pixels
              invoke GetDeviceCaps, hCmpDC, LOGPIXELSX
              invoke MulDiv, dHmWidth, eax, HIMETRIC_INCH
              mov [esi].dWidth, eax

              invoke GetDeviceCaps, hCmpDC, LOGPIXELSY
              invoke MulDiv, dHmHeight, eax, HIMETRIC_INCH
              mov [esi].dHeight, eax
              mov eax, dHmHeight
              neg eax
              mov dNegHmHeight, eax

              .if $invoke(CreateCompatibleBitmap, hCmpDC, [esi].dWidth, [esi].dHeight) == 0
                OCall esi.ErrorReport, NULL, PXM_LOADFILE_FAILED
              .else
                .if $invoke(SelectObject, hTmpDC, eax) == 0
                  OCall esi.ErrorReport, NULL, PXM_LOADFILE_FAILED
                .else
                  mov hPrvBmp, eax
                  ;Now we have our bitmap mounted onto our temporary DC, let's blit to it
                  ICall pPicture::IPicture.Render, hTmpDC, 0, 0, [esi].dWidth, [esi].dHeight, 0, \
                                                   dHmHeight, dHmWidth, dNegHmHeight, NULL
                  .if FAILED(eax)
                    OCall esi.ErrorReport, NULL, PXM_IPICTURE_FAILED
                    invoke DeleteObject, $invoke(SelectObject, hTmpDC, hPrvBmp)
                    OCall esi.ErrorReport, NULL, PXM_LOADFILE_FAILED
                  .else
                    ;We now have the bitmap blitted, let's get it off the dc and clean up.
                    ;We're not going to check for errors, cause we did our importaint thing
                    ;and if these fail now, other things will fall apart anyway
                    mov hDDB, $invoke(SelectObject, hTmpDC, hPrvBmp)
                    OCall esi.LoadBmp, esi, hCmpDC, hDDB
                    invoke DeleteObject, hDDB
                  .endif
                .endif
              .endif
              invoke DeleteDC, hTmpDC
            .endif
            invoke ReleaseDC, 0, hCmpDC

            ICall pPicture::IPicture.Release
          .endif
          invoke UStrDispose, pWideFileName
        .endif
      .else
        OCall esi.ErrorReport, NULL, PXM_INVALID_FILENAME
      .endif
    .else
      OCall esi.ErrorReport, NULL, PXM_FILENAME_TOO_LONG
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.SaveFile
; Purpose:   Saves the Pixelmap to disk.
; Arguments: Arg1: -> FileName
;            Arg2: File format (PXM_FORMAT_PXM, ...)
; Return:    eax = Error code.

Method Pixelmap.SaveFile, uses esi, pFileName:Pointer, dFileFormat:dword
    local BmpFileHdr:BITMAPFILEHEADER
    local hFile:Handle, dBytesWritten:dword

    SetObject esi
    .if [esi].hDIBSection != 0
      .if dFileFormat == PXM_FORMAT_PXM
        mov BmpFileHdr.BITMAPFILEHEADER.bfType, "MB"    ;"BM" signature
        mov eax, [esi].BmpInfo.BITMAPINFO.bmiHeader.biSizeImage
        add eax, sizeof BmpFileHdr + sizeof BITMAPINFOHEADER
        mov BmpFileHdr.BITMAPFILEHEADER.bfSize, eax
        and BmpFileHdr.BITMAPFILEHEADER.bfReserved1, 0
        and BmpFileHdr.BITMAPFILEHEADER.bfReserved2, 0
        mov BmpFileHdr.BITMAPFILEHEADER.bfOffBits, sizeof BmpFileHdr + sizeof BITMAPINFOHEADER

        invoke CreateFile, pFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0
        .if eax == INVALID_HANDLE_VALUE
          OCall esi.ErrorReport, NULL, PXM_FILEOPEN_FAILED
        .else
          mov hFile, eax
          invoke WriteFile, hFile, addr BmpFileHdr, sizeof BmpFileHdr, addr dBytesWritten, NULL
          .if eax == FALSE
            OCall esi.ErrorReport, NULL, PXM_FILEWRITE_FAILED
          .else
            invoke WriteFile, hFile, addr [esi].BmpInfo, sizeof BITMAPINFOHEADER, addr dBytesWritten, NULL
            .if eax == FALSE
              OCall esi.ErrorReport, NULL, PXM_FILEWRITE_FAILED
            .else
              invoke WriteFile, hFile, [esi].pPixels, [esi].BmpInfo.BITMAPINFO.bmiHeader.biSizeImage,\
                                addr dBytesWritten, NULL
              .if eax == FALSE
                OCall esi.ErrorReport, NULL, PXM_FILEWRITE_FAILED
              .endif
            .endif
          .endif
          invoke CloseHandle, hFile
          xor eax, eax
        .endif
      .else
        OCall esi.ErrorReport, NULL, PXM_INVALID_FILE_FORMAT
      .endif
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.SetPixel
; Purpose:   Sets the ARGB value of a given pixel.
; Arguments: Arg1: X location.
;            Arg2: Y location.
;            Arg3: New ARGB value.
; Return:    Nothing.

Method Pixelmap.SetPixel,, dCoorX:dword, dCoorY:dword, dARGB:dword
    SetObject edx
    .if [edx].hDIBSection != 0
      mov eax, dCoorY
      .if eax >= 0 && eax < [edx].dHeight
        mov ecx, dCoorX
        .if ecx >= 0 && ecx < [edx].dWidth
          shl ecx, 2
          add ecx, [edx].pPixels
          mul [edx].dWidth
          mov edx, dARGB
          RGB2BGR edx
          mov [4*eax + ecx], edx
        .else
;          DbgWarning "X coordinate out of range in SetPixel"
        .endif
      .else
;        DbgWarning "Y coordinate out of range in SetPixel"
      .endif
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.SetVirtualComponent
; Purpose:   Sets a channel component of a given virtual pixel and performs a Gamma Correction on
;            the modified pixels. Alpha values of the background pixels are adjusted.
; Arguments: Arg1: X location.
;            Arg2: Y location.
;            Arg3: Channel (PXM_CHANNEL_XXX).
;            Arg3: Component value.
; Return:    Nothing.

PXM_SetComponent macro
    local @@Blue, @@Green, @@Red, @@Alpha

    mov edx, dChannel
    movzx eax, byte ptr [PXM_CHANNEL_OFFSET_TABLE + edx]
    movzx edx, byte ptr [esi + eax]
    add edx, pGam2LinTable
    movzx ecx, byte ptr [edx]                       ;ecx = back linear component

    fld1                                            ;Calc complement to 1
    fsub st(0), st(1)

    JumpOn eax, @@Blue, @@Green, @@Red, @@Alpha

@@Blue:
    fild dComponent
    fmul st(0), st(2)
    mov dBuffer, ecx
    fild dBuffer
    fmul st(0), st(2)
    faddp st(1), st(0)
    fistp dBuffer
    .if sdword ptr dBuffer > 255
      mov ecx, 255
    .elseif sdword ptr dBuffer < 0
      xor ecx, ecx
    .else
      mov edx, pLin2GamTable
      add edx, dBuffer
      mov cl, byte ptr [edx]
    .endif
    mov byte ptr [esi], cl                                    ;Blue
    jmp @F

@@Green:
    fild dComponent
    fmul st(0), st(2)
    mov dBuffer, ecx
    fild dBuffer
    fmul st(0), st(2)
    faddp st(1), st(0)
    fistp dBuffer
    .if sdword ptr dBuffer > 255
      mov ecx, 255
    .elseif sdword ptr dBuffer < 0
      xor ecx, ecx
    .else
      mov edx, pLin2GamTable
      add edx, dBuffer
      mov cl, byte ptr [edx]
    .endif
    mov byte ptr [esi + PXM_CHANNEL_OFFSET_GREEN], cl         ;Green
    jmp @F

@@Red:
    fild dComponent
    fmul st(0), st(2)
    mov dBuffer, ecx
    fild dBuffer
    fmul st(0), st(2)
    faddp st(1), st(0)
    fistp dBuffer
    .if sdword ptr dBuffer > 255
      mov ecx, 255
    .elseif sdword ptr dBuffer < 0
      xor ecx, ecx
    .else
      mov edx, pLin2GamTable
      add edx, dBuffer
      mov cl, byte ptr [edx]
    .endif
    mov byte ptr [esi + PXM_CHANNEL_OFFSET_RED], cl           ;Red
    jmp @F

@@Alpha:
    fild dComponent
    fmul st(0), st(2)
    mov dBuffer, ecx
    fild dBuffer
    fmul st(0), st(2)
    faddp st(1), st(0)
    fistp dBuffer
    .if sdword ptr dBuffer > 255
      mov ecx, 255
    .elseif sdword ptr dBuffer < 0
      xor ecx, ecx
    .else
      mov ecx, dBuffer
    .endif
    mov byte ptr [esi + PXM_CHANNEL_OFFSET_ALPHA], cl         ;Alpha

@@:
    fUnload
endm

Method Pixelmap.SetVirtualComponent, uses esi, r4CoorX:real4, r4CoorY:real4, dChannel:dword, dComponent:dword
    local sdCoorX0:sdword, sdCoorX1:sdword, sdCoorY0:sdword, sdCoorY1:sdword
    local dBuffer:dword, wCurFpuCtrl:word, wPrvFpuCtrl:word
    local dSkipX:sdword, dSkipY:dword
    local sdRed:sdword, sdGreen:sdword, sdBlue:sdword, sdAlpha:sdword
    local dBackLinColor:dword, dForeLinColor:dword
    local dBackLinComp:dword, dForeLinComp:dword, pGam2LinTable:Pointer, pLin2GamTable:Pointer

    SetObject esi
    m2m pGam2LinTable, [esi].pGam2LinTable, eax
    m2m pLin2GamTable, [esi].pLin2GamTable, eax

    fstcw wPrvFpuCtrl                         ;Store FPU control word
    m2m wCurFpuCtrl, wPrvFpuCtrl, ax          ;Duplicate value

    fld r4CoorY

    BitSet wCurFpuCtrl, BIT10                 ;Modify the control word to
    BitClr wCurFpuCtrl, BIT11                 ;  round to -inf
    fldcw wCurFpuCtrl                         ;Restore modified FPU control word
    fist sdCoorY0

    BitClr wCurFpuCtrl, BIT10                 ;Modify the control word to
    BitSet wCurFpuCtrl, BIT11                 ;  round to +inf
    fldcw wCurFpuCtrl                         ;Restore modified FPU control word
    fist sdCoorY1

    mov eax, sdCoorY1
    .if sdCoorY0 >= 0 && eax < [esi].dHeight
      fisub sdCoorY0                          ;B = y - y0
      xor edx, edx
      fld r4CoorX
      cmp eax, sdCoorY0
      je @F
      mov edx, [esi].dWidth
      shl edx, 2
@@:
      mov dSkipY, edx

      fist sdCoorX1

      BitSet wCurFpuCtrl, BIT10               ;Modify the control word to
      BitClr wCurFpuCtrl, BIT11               ;  round to -inf
      fldcw wCurFpuCtrl                       ;Restore modified FPU control word
      mov ecx, sdCoorX1
      fist sdCoorX0

      .if sdCoorX0 >= 0 && ecx < [esi].dWidth
        BitClr wCurFpuCtrl, (BIT10 or BIT11)  ;Modify the control word to return rnd(x)
        fldcw wCurFpuCtrl                     ;Restore modified FPU control word
        xor edx, edx
        fisub sdCoorX0                        ;A = x - x0, B = y - y0
        cmp ecx, sdCoorX0
        je @F
        mov edx, 4
@@:
        mov dSkipX, edx

        shl ecx, 2
        add ecx, [esi].pPixels
        mul [esi].dWidth
        lea esi, [4*eax + ecx]                ;esi -> Poisition right/bottom

        .if dChannel != PXM_CHANNEL_ALPHA     ;Alpha is unchanged
          mov edx, pGam2LinTable              ;  transform from Gamma space to Lin space
          add edx, dComponent
          movzx ecx, byte ptr [edx]
          mov dComponent, ecx
        .endif

        .if dSkipX == 0 && dSkipY == 0
          fUnload
          fld1
          PXM_SetComponent                    ;esi -> Position 00
          fUnload

        .elseif dSkipX == 0                   ;A = 0, B <> 0
          fUnload
          fld1
          fsub st(0), st(1)                   ;1 - B, B
          fxch st(1)

          PXM_SetComponent                    ;esi -> Position 01
          fUnload
          sub esi, dSkipY                     ;esi -> Position 00
          PXM_SetComponent

        .elseif dSkipY == 0                   ;A <> 0, B = 0
          fxch st(1)
          fUnload
          fld1
          fsub st(0), st(1)                   ;1 - A, A
          fxch st(1)

          PXM_SetComponent                    ;esi -> Position 10
          fUnload
          sub esi, dSkipX                     ;esi -> Position 00
          PXM_SetComponent

        .else
          fld st(0)
          fmul st(0), st(2)                   ;f11 = AxB
          fsub st(1), st(0)                   ;f10 = A - AxB
          fsub st(2), st(0)                   ;f01 = B - AxB
          fld1
          fsub st(0), st(1)
          fsub st(0), st(2)                   ;f00 = 1 - f11 - f10 - f01
          fsub st(0), st(3)                   ;f00, f11, f10, f01

          sub esi, dSkipX                     ;esi -> Poisition 01
          sub esi, dSkipY                     ;esi -> Poisition 00
          PXM_SetComponent
          fUnload
          add esi, dSkipX                     ;esi -> Poisition 10
          add esi, dSkipY                     ;esi -> Poisition 11
          PXM_SetComponent
          fUnload
          sub esi, dSkipY                     ;esi -> Poisition 10
          PXM_SetComponent
          fUnload
          sub esi, dSkipX                     ;esi -> Poisition 00
          add esi, dSkipY                     ;esi -> Poisition 01
          PXM_SetComponent
        .endif
        jmp @F
      .else
        DbgWarning "X coordinate out of range in SetVirtualComponent"
      .endif
      fUnload
    .else
      DbgWarning "Y coordinate out of range in SetVirtualComponent"
    .endif
@@:
    fUnload
    fldcw wPrvFpuCtrl                         ;Restore previous FPU control word
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.SetVirtualPixel
; Purpose:   Sets an ARGB value of a given virtual pixel and performs a Gamma Correction on the
;            modified pixels. Alpha values of the background pixels are adjusted.
; Arguments: Arg1: X location.
;            Arg2: Y location.
;            Arg3: New ARGB value.
; Return:    Nothing.

PXM_SetColors macro
    mov dBackLinColor, $OCall(Self::Pixelmap.Gamma2Linear, dword ptr [esi])   ;ABGR

    fld1                                            ;Calc complement to 1
    fsub st(0), st(1)

    movzx ecx, byte ptr dForeLinColor + 2           ;Blue
    mov dBuffer, ecx
    fild dBuffer
    movzx ecx, byte ptr dBackLinColor + PXM_CHANNEL_OFFSET_BLUE
    fmul st(0), st(2)
    mov dBuffer, ecx
    fild dBuffer
    fmul st(0), st(2)
    faddp st(1), st(0)
    fistp sdBlue

    movzx ecx, byte ptr dForeLinColor + 1           ;Green
    mov dBuffer, ecx
    fild dBuffer
    movzx ecx, byte ptr dBackLinColor + PXM_CHANNEL_OFFSET_GREEN
    fmul st(0), st(2)
    mov dBuffer, ecx
    fild dBuffer
    fmul st(0), st(2)
    faddp st(1), st(0)
    fistp sdGreen

    movzx ecx, byte ptr dForeLinColor + 0           ;Red
    mov dBuffer, ecx
    fild dBuffer
    movzx ecx, byte ptr dBackLinColor + PXM_CHANNEL_OFFSET_RED
    fmul st(0), st(2)
    mov dBuffer, ecx
    fild dBuffer
    fmul st(0), st(2)
    faddp st(1), st(0)
    fistp sdRed

    movzx ecx, byte ptr dForeLinColor + 3           ;Alpha
    mov dBuffer, ecx
    fild dBuffer
    movzx ecx, byte ptr dBackLinColor + PXM_CHANNEL_OFFSET_ALPHA
    fmul st(0), st(2)
    mov dBuffer, ecx
    fild dBuffer
    fmul st(0), st(2)
    faddp st(1), st(0)
    fistp sdAlpha

    fUnload

    ;Compose the new color and prevent a channel over- or underflow.
    if @Cpu and CPU_686
      mov edx, 255
      xor ecx, ecx
      mov eax, sdRed
      cmp eax, ecx
      cmovl eax, ecx
      cmp eax, edx
      cmovg eax, edx
      mov sdRed, eax

      mov eax, sdGreen
      cmp eax, ecx
      cmovl eax, ecx
      cmp eax, edx
      cmovg eax, edx
      mov sdGreen, eax

      mov eax, sdBlue
      cmp eax, ecx
      cmovl eax, ecx
      cmp eax, edx
      cmovg eax, edx
      mov sdBlue, eax

      mov eax, sdAlpha
      cmp eax, ecx
      cmovl eax, ecx
      cmp eax, edx
      cmovg eax, edx
      mov sdAlpha, eax
    else
      .if sdRed > 255
        mov sdRed, 255
      .elseif sdRed < 0
        and sdRed, 0
      .endif
      .if sdGreen > 255
        mov sdGreen, 255
      .elseif sdGreen < 0
        and sdGreen, 0
      .endif
      .if sdBlue > 255
        mov sdBlue, 255
      .elseif sdBlue < 0
        and sdBlue, 0
      .endif
      .if sdAlpha > 255
        mov sdAlpha, 255
      .elseif sdAlpha < 0
        and sdAlpha, 0
      .endif
    endif
    mov ah, byte ptr sdAlpha
    mov al, byte ptr sdRed
    shl eax, 16
    mov ah, byte ptr sdGreen
    mov al, byte ptr sdBlue

    OCall Self::Pixelmap.Linear2Gamma, eax

    mov dword ptr [esi], eax
endm

Method Pixelmap.SetVirtualPixel, uses esi, r4CoorX:real4, r4CoorY:real4, dARGB:dword
    local sdCoorX0:sdword, sdCoorX1:sdword, sdCoorY0:sdword, sdCoorY1:sdword
    local dBuffer:dword, wCurFpuCtrl:word, wPrvFpuCtrl:word
    local dSkipX:sdword, dSkipY:dword
    local sdRed:sdword, sdGreen:sdword, sdBlue:sdword, sdAlpha:sdword
    local dBackLinColor:dword, dForeLinColor:dword

    SetObject esi
    fstcw wPrvFpuCtrl                         ;Store FPU control word
    m2m wCurFpuCtrl, wPrvFpuCtrl, ax          ;Duplicate value

    fld r4CoorY

    BitSet wCurFpuCtrl, BIT10                 ;Modify the control word to
    BitClr wCurFpuCtrl, BIT11                 ;  round to -inf
    fldcw wCurFpuCtrl                         ;Restore modified FPU control word
    fist sdCoorY0

    BitClr wCurFpuCtrl, BIT10                 ;Modify the control word to
    BitSet wCurFpuCtrl, BIT11                 ;  round to +inf
    fldcw wCurFpuCtrl                         ;Restore modified FPU control word
    fist sdCoorY1

    mov eax, sdCoorY1
    .if sdCoorY0 >= 0 && eax < [esi].dHeight
      fisub sdCoorY0                          ;B = y - y0
      xor edx, edx
      fld r4CoorX
      cmp eax, sdCoorY0
      je @F
      mov edx, [esi].dWidth
      shl edx, 2
@@:
      mov dSkipY, edx

      fist sdCoorX1

      BitSet wCurFpuCtrl, BIT10               ;Modify the control word to
      BitClr wCurFpuCtrl, BIT11               ;  round to -inf
      fldcw wCurFpuCtrl                       ;Restore modified FPU control word
      mov ecx, sdCoorX1
      fist sdCoorX0

      .if sdCoorX0 >= 0 && ecx < [esi].dWidth
        BitClr wCurFpuCtrl, (BIT10 or BIT11)  ;Modify the control word to return rnd(x)
        fldcw wCurFpuCtrl                     ;Restore modified FPU control word
        xor edx, edx
        fisub sdCoorX0                        ;A = x - x0, B = y - y0
        cmp ecx, sdCoorX0
        je @F
        mov edx, 4
@@:
        mov dSkipX, edx

        shl ecx, 2
        add ecx, [esi].pPixels
        mul [esi].dWidth
        lea esi, [4*eax + ecx]                ;esi -> Poisition right/bottom

        .if dSkipX == 0 && dSkipY == 0
          fUnload
          mov dForeLinColor, $OCall(Self::Pixelmap.Gamma2Linear, dARGB)   ;ARGB
          fld1
          PXM_SetColors                       ;esi -> Position 00

        .elseif dSkipX == 0                   ;A = 0, B <> 0
          mov dForeLinColor, $OCall(Self::Pixelmap.Gamma2Linear, dARGB)   ;ARGB

          fUnload
          fld1
          fsub st(0), st(1)                   ;1 - B, B
          fxch st(1)

          PXM_SetColors                       ;esi -> Position 01
          fUnload
          sub esi, dSkipY                     ;esi -> Position 00
          PXM_SetColors

        .elseif dSkipY == 0                   ;A <> 0, B = 0
          mov dForeLinColor, $OCall(Self::Pixelmap.Gamma2Linear, dARGB)   ;ARGB

          fxch st(1)
          fUnload
          fld1
          fsub st(0), st(1)                   ;1 - A, A
          fxch st(1)

          PXM_SetColors                       ;esi -> Position 10
          fUnload
          sub esi, dSkipX                     ;esi -> Position 00
          PXM_SetColors

        .else
          mov dForeLinColor, $OCall(Self::Pixelmap.Gamma2Linear, dARGB)   ;ARGB

          fld st(0)
          fmul st(0), st(2)                   ;f11 = AxB
          fsub st(1), st(0)                   ;f10 = A - AxB
          fsub st(2), st(0)                   ;f01 = B - AxB
          fld1
          fsub st(0), st(1)
          fsub st(0), st(2)                   ;f00 = 1 - f11 - f10 - f01
          fsub st(0), st(3)                   ;f00, f11, f10, f01

          sub esi, dSkipX                     ;esi -> Poisition 01
          sub esi, dSkipY                     ;esi -> Poisition 00
          PXM_SetColors
          fUnload
          add esi, dSkipX                     ;esi -> Poisition 10
          add esi, dSkipY                     ;esi -> Poisition 11
          PXM_SetColors
          fUnload
          sub esi, dSkipY                     ;esi -> Poisition 10
          PXM_SetColors
          fUnload
          sub esi, dSkipX                     ;esi -> Poisition 00
          add esi, dSkipY                     ;esi -> Poisition 01
          PXM_SetColors
        .endif
        jmp @F
      .else
        DbgWarning "X coordinate out of range in SetVirtualPixel"
      .endif
      fUnload
    .else
      DbgWarning "Y coordinate out of range in SetVirtualPixel"
    .endif
@@:
    fUnload
    fldcw wPrvFpuCtrl                         ;Restore previous FPU control word
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.Store
; Purpose:   Stores the Pixelmap in a stream object.
; Arguments: Arg1: -> stream object.
; Return:    Nothing.

Method Pixelmap.Store, uses esi, pStream:Pointer
    SetObject esi
    .if [esi].hDIBSection != 0
      lea eax, [esi].dWidth
      OCall pStream::Stream.BinWrite, eax, 2 * 4    ;dWidth, dHeight
      OCall pStream::Stream.BinWrite, [esi].pPixels, [esi].BmpInfo.BITMAPINFO.bmiHeader.biSizeImage
    .else
      push 0
      push 0
      mov eax, esp
      OCall pStream::Stream.BinWrite, eax, 2 * 4    ;dWidth = 0, dHeight = 0
      add esp, 8
    .endif
MethodEnd


; ==================================================================================================
; Bound methods
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.AlphaBlend
; Purpose:   Blends a source Pixelmap on a destination RECT
; Arguments: Arg1: -> source Pixelmap.
;            Arg2: Destination RECT.
;            Arg3: Source starting POINT
; Return:    Nothing.

Method Pixelmap.AlphaBlend, uses ebx edi esi, pSrcPxm:Pointer, SrcRct:RECT, DstPnt:POINT
    local dDstSkipY:dword, dSrcSkipY:dword
    local dWidth:dword, dHeight:dword
    local SrcSpace:RECT, DstSpace:RECT, OutRct:RECT
    local dDens1:dword, dDens2:dword

    SetObject edi
    SetObject esi,,pSrcPxm

    .if [esi].hDIBSection != 0 && [edi].hDIBSection != 0
      ;Flip SrcRct if necessary
      Pxm_FlipInvertedRect SrcRct

      ;Clip SrcRct to SrcSpace
      and SrcSpace.left, 0
      and SrcSpace.top, 0
      m2m SrcSpace.right, [esi].dWidth, eax
      m2m SrcSpace.bottom, [esi].dHeight, eax
      Pxm_ClipRect OutRct, SrcRct, SrcSpace

      ;Clip to DstSpace
      mov eax, SrcRct.left
      sub eax, DstPnt.x
      mov DstSpace.left, eax
      add eax, [edi].dWidth
      mov DstSpace.right, eax
      mov eax, SrcRct.top
      sub eax, DstPnt.y
      mov DstSpace.top, eax
      add eax, [edi].dHeight
      mov DstSpace.bottom, eax
      Pxm_ClipRect OutRct, OutRct, DstSpace

      ;Calc transfer sizes
      sub eax, ecx
      jz @@EOM
      mov dHeight, eax
      mov eax, OutRct.right
      sub eax, OutRct.left
      jz @@EOM
      mov dWidth, eax

      ;Calc skip values
      mov dDstSkipY, $Pxm_Bytes2Skip(dWidth, edi)
      mov dSrcSkipY, $Pxm_Bytes2Skip(dWidth, esi)

      ;Calc position of first pixel to transfer in source space
      mov esi, $Pxm_PixelAddr(OutRct.left, OutRct.top, esi)
      ReleaseObject

      ;Calc position of first pixel to transfer in destination space
      mov eax, OutRct.top
      sub eax, SrcRct.top
      add eax, DstPnt.y
      mul [edi].dWidth
      add eax, OutRct.left
      sub eax, SrcRct.left
      add eax, DstPnt.x
      shl eax, 2
      add eax, [edi].pPixels
      mov edi, eax
      ReleaseObject

      ;Do alfablending of pixels
      push dHeight
      align ALIGN_CODE
@@YLoop:
      push dWidth
@@XLoop:
      mov eax, [esi]
      shr eax, 32-8
      mov dDens1, eax
      neg ax
      add ax, 0FFh
      mov dDens2, eax                     ;Dens2 = Complement to dDens1

      mov eax, [edi]                      ;Get destiantion original pixel
      mov ebx, eax
      and eax, PXM_MASK_1010
      and ebx, PXM_MASK_0101
      shr eax, 8
      imul eax, dDens2
      imul ebx, dDens2

      mov ecx, [esi]                      ;Get source original pixel
      mov edx, ecx
      and ecx, PXM_MASK_1010
      and edx, PXM_MASK_0101
      shr ecx, 8
      imul ecx, dDens1
      imul edx, dDens1

      add eax, ecx
      add ebx, edx
      and eax, PXM_MASK_1010
      shr ebx, 8
      and ebx, PXM_MASK_0101
      or eax, ebx
      mov [edi], eax                      ;Store new color
      add esi, 4
      add edi, 4
      dec dword ptr [esp]                 ;Decrement width
      jnz @@XLoop
      add esp, 4                          ;Restore stack
      add edi, dDstSkipY
      add esi, dSrcSkipY
      dec dword ptr [esp]                 ;Decrement Height
      jnz @@YLoop
      add esp, 4                          ;Restore stack
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.Brightness
; Purpose:   Adjusts the brightness of the source pixelmap.
; Arguments: Arg1: Luminicense value (0 = dark, 255 = bright)
; Return:    Nothing.
; Notes:     - Original code by Edgar Hansen - Donkey -

Method Pixelmap.Brightness, uses ebx edi esi, dLum:dword
    local dLuma:dword

    SetObject esi
    .if [esi].hDIBSection != 0
      and dLuma, 0FFh
      mov ebx, [esi].BmpInfo.BITMAPINFO.bmiHeader.biSizeImage
      shr ebx, 2
      mov esi, [esi].pPixels
      .if dLum > 127
        mov ecx, 256
        sub ecx, dLum
      .else
        mov ecx, dLum
      .endif
      mov dLuma, ecx
      ReleaseObject

align 16
      .while !Zero?
        mov eax, [esi]
        .if dLum > 127
          not eax
        .endif
        mov edi, eax

        ;Copy alfa channel
        mov ecx, edi
        shr ecx, 16

        ;Process red channel
        mov eax, edi
        shr eax, 16
        and eax, 0FFh
        mul byte ptr dLuma
        shr eax, 7
        mov cl, al
        shl ecx, 16

        ;Process green channel
        mov eax, edi
        shr eax, 8
        and eax, 0FFh
        mul byte ptr dLuma
        shr eax, 7
        mov ch, al

        ;Process blue channel
        mov eax, edi
        and eax, 0FFh
        mul byte ptr dLuma
        shr eax, 7
        mov cl, al

        .if dLum > 127
          not ecx
        .endif

        mov [esi], ecx
        add esi, 4
        dec ebx
      .endw
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.Convolve3x3
; Purpose:   Performs a 3x3 pixel convolution on the source pixelmap.
; Arguments: Arg1: -> source Pixelmap.
;            Arg2: Source RECT.
;            Arg3: Destination origin.
;            Arg4: -> filter arguments.
; Return:    Nothing.
;
;       —————————————————
;       |1|     2     |3|           1,3,7,9 Corners
;       |—|———————————|—|           2,4,6,8 bands
;       | |           | |           5 inner rect
;       |4|     5     |6|
;       | |           | |
;       |—|———————————|—|
;       |7|     8     |9|
;       —————————————————


PXM_Conv_Eval macro Factor:=<1>
      ;Load the pixel pointed to by ecx
      mov edx, [ecx]
      movzx eax, dl
      mov dBuffer, eax
      fild dBuffer
      movzx eax, dh
      mov dBuffer, eax
      fild dBuffer
      shr edx, 16
      movzx eax, dl
      mov dBuffer, eax
      fild dBuffer

      ;Multiply the color components by the filter data pointed to by ebx
      fld real4 ptr [ebx]
      repeat Factor - 1
        fadd st(0), st(0)
      endm
      fmul st(3), st(0)
      fmul st(2), st(0)
      fmulp st(1), st(0)
endm

PXM_Conv_Add macro
      ;Add color components to the previous values in the FPU stack
      faddp st(3), st(0)
      faddp st(3), st(0)
      faddp st(3), st(0)
endm

$PXM_GetNewColor macro
      ;Compute new color
      fistp sdRed
      fistp sdGreen
      fistp sdBlue

      ;Compose the new color and prevent a channel over- or underflow
      .if sdRed > 255
        mov sdRed, 255
      .elseif sdRed < 0
        and sdRed, 0
      .endif
      .if sdGreen > 255
        mov sdGreen, 255
      .elseif sdGreen < 0
        and sdGreen, 0
      .endif
      .if sdBlue > 255
        mov sdBlue, 255
      .elseif sdBlue < 0
        and sdBlue, 0
      .endif
      mov ah, 255
      mov al, byte ptr sdRed
      shl eax, 16
      mov ah, byte ptr sdGreen
      mov al, byte ptr sdBlue
      exitm <eax>
endm

Method Pixelmap.Convolve3x3, uses ebx edi esi, pSrcPxm:Pointer, SrcRct:RECT, DstPnt:POINT, pFilter:Pointer
    local dDstSkipY:dword, dSrcSkipY:dword, dFilSkipYNeg:dword, dFilSkipYPos:dword
    local dWidth:dword, dHeight:dword
    local SrcSpace:RECT, DstSpace:RECT, OutRct:RECT
    local dBuffer:dword, sdRed:sdword, sdGreen:sdword, sdBlue:sdword

    SetObject edi
    SetObject esi,,pSrcPxm

    .if [esi].hDIBSection != 0 && [edi].hDIBSection != 0
      ;Flip SrcRct if necessary
      Pxm_FlipInvertedRect SrcRct

      ;Clip SrcRct to SrcSpace
      and SrcSpace.left, 0
      and SrcSpace.top, 0
      m2m SrcSpace.right, [esi].dWidth, eax
      m2m SrcSpace.bottom, [esi].dHeight, eax
      Pxm_ClipRect OutRct, SrcRct, SrcSpace

      ;Clip to DstSpace
      mov eax, SrcRct.left
      sub eax, DstPnt.x
      mov DstSpace.left, eax
      add eax, [edi].dWidth
      mov DstSpace.right, eax
      mov eax, SrcRct.top
      sub eax, DstPnt.y
      mov DstSpace.top, eax
      add eax, [edi].dHeight
      mov DstSpace.bottom, eax
      Pxm_ClipRect OutRct, OutRct, DstSpace

      ;Calc transfer sizes
      sub eax, ecx
      jle @@EOM
      mov dHeight, eax
      mov eax, OutRct.right
      sub eax, OutRct.left
      jle @@EOM
      mov dWidth, eax

      ;Calc filter skip values
      mov eax, [esi].dWidth
      shl eax, 2
      lea ecx, [eax - 8]
      mov dFilSkipYPos, ecx
      add eax, 4
      mov dFilSkipYNeg, eax

      ;Calc position of first pixel to transfer in source space
      mov esi, $Pxm_PixelAddr(OutRct.left, OutRct.top, esi)
      ReleaseObject

      ;Calc position of first pixel to transfer in destination space
      mov eax, OutRct.top
      sub eax, SrcRct.top
      add eax, DstPnt.y
      mul [edi].dWidth
      add eax, OutRct.left
      sub eax, SrcRct.left
      add eax, DstPnt.x
      shl eax, 2
      add eax, [edi].pPixels
      mov edi, eax
      ReleaseObject

      push edi
      push esi

;##\  ———— Top-Left Corner —————————————————————————————————————————————————————
      mov ebx, pFilter                      ;ebx -> filter data
      add ebx, 16
      mov ecx, esi
      PXM_Conv_Eval                         ;Evaluate color at +0,+0
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval 2                       ;Evaluate color at +1,+0
      PXM_Conv_Add
      add ecx, dFilSkipYPos
      add ebx, 8
      add ecx, 4
      PXM_Conv_Eval 2                       ;Evaluate color at +0,+1
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval 3                       ;Evaluate color at +1,+1
      PXM_Conv_Add

      mov [edi], $PXM_GetNewColor()         ;Store the new generated color

      fUnload 1                             ;Free the last FPU register
;##/
;##\  ———— Upper band ——————————————————————————————————————————————————————————
      ;Calc position of first pixel to transfer in source space position +1,0
      add esi, 4

      ;Calc position of first pixel to transfer in destination space
      add edi, 4

      ;Do convolution of upper band
      mov ebx, pFilter                      ;ebx -> filter data
      mov eax, dWidth
      add ebx, 12                           ;ebx -> filter 4th data element
      sub eax, 2
      push ebx
      push eax
      align ALIGN_CODE
@@XLoopUB:
      mov ebx, [esp + 4]                    ;Recover original ebx pointer
      mov ecx, esi
      sub ecx, 4
      PXM_Conv_Eval                         ;Evaluate color at -1,+0
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at +0,+0
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at +1,+0
      PXM_Conv_Add

      add ecx, dFilSkipYPos
      add ebx, 4
      PXM_Conv_Eval 2                       ;Evaluate color at -1,+1
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval 2                       ;Evaluate color at +0,+1
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval 2                       ;Evaluate color at +1,+1
      PXM_Conv_Add

      mov [edi], $PXM_GetNewColor()         ;Store the new generated color

      add esi, 4
      add edi, 4
      dec dword ptr [esp]
      jnz @@XLoopUB
      add esp, 8                            ;Restore stack

      fUnload 1                             ;Free the last FPU register
;##/
;##\  ———— Top-Right Corner ————————————————————————————————————————————————————
      mov ebx, pFilter                      ;ebx -> filter data
      add ebx, 12
      mov ecx, esi
      sub ecx, 4
      PXM_Conv_Eval 2                       ;Evaluate color at -1,+0
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at +0,+0
      PXM_Conv_Add
      add ecx, dFilSkipYPos
      add ebx, 8
      add ecx, 4
      PXM_Conv_Eval 3                       ;Evaluate color at -1,+1
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval 2                       ;Evaluate color at +0,+1
      PXM_Conv_Add

      mov [edi], $PXM_GetNewColor()         ;Store the new generated color

      fUnload 1                             ;Free the last FPU register
;##/
;##\  ———— Lower band ——————————————————————————————————————————————————————————
      ;Calc position of first pixel to transfer in source space +1,+n-1
      SetObject esi,, pSrcPxm
      mov eax, dHeight
      dec eax
      mul [esi].dWidth
      inc eax
      shl eax, 2
      mov esi, [esp]
      add esi, eax
      ReleaseObject

      ;Calc position of first pixel to transfer in destination space +1,+n+1
      SetObject edi
      mov eax, dHeight
      dec eax
      mul [edi].dWidth
      inc eax
      shl eax, 2
      mov edi, [esp + 4]
      add edi, eax
      ReleaseObject

      ;Do convolution of lower band
      mov eax, dWidth
      sub eax, 2
      push eax
      align ALIGN_CODE
@@XLoopLB:
      mov ebx, pFilter                      ;ebx -> filter data
      mov ecx, esi
      sub ecx, dFilSkipYNeg
      PXM_Conv_Eval 2                       ;Evaluate color at -1,-1
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval 2                       ;Evaluate color at +0,-1
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval 2                       ;Evaluate color at +1,-1
      PXM_Conv_Add

      add ecx, dFilSkipYPos
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at -1,+0
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at +0,+0
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at +1,+0
      PXM_Conv_Add

      mov [edi], $PXM_GetNewColor()         ;Store the new generated color

      add esi, 4
      add edi, 4
      dec dword ptr [esp]
      jnz @@XLoopLB
      add esp, 4                            ;Restore stack

      fUnload 1                             ;Free the last FPU register
;##/
;##\  ———— Bottom-Right Corner —————————————————————————————————————————————————
      mov ebx, pFilter                      ;ebx -> filter data
      mov ecx, esi
      sub ecx, dFilSkipYNeg
      PXM_Conv_Eval 3                       ;Evaluate color at -1,-1
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval 2                       ;Evaluate color at +0,-1
      PXM_Conv_Add
      add ecx, dFilSkipYPos
      add ebx, 8
      add ecx, 4
      PXM_Conv_Eval 2                       ;Evaluate color at -1,+0
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at +0,+0
      PXM_Conv_Add

      mov [edi], $PXM_GetNewColor()         ;Store the new generated color

      fUnload 1                             ;Free the last FPU register
;##/
;##\  ———— Left band ———————————————————————————————————————————————————————————
      ;Calc position of first pixel to transfer in source space position +0,+1
      SetObject esi,, pSrcPxm
      mov eax, [esi].dWidth
      shl eax, 2
      mov esi, [esp]
      add esi, eax
      mov dSrcSkipY, eax
      mov dFilSkipYNeg, eax
      sub eax, 4
      mov dFilSkipYPos, eax
      ReleaseObject

      ;Calc position of first pixel to transfer in destination space
      SetObject edi
      mov eax, [edi].dWidth
      shl eax, 2
      mov dDstSkipY, eax
      mov edi, [esp + 4]
      add edi, eax
      ReleaseObject

      ;Do convolution of left band
      mov eax, dHeight
      sub eax, 2
      mov ebx, pFilter                      ;ebx -> filter data
      add ebx, 4
      push ebx
      push eax
      align ALIGN_CODE
@@YLoopLB:
      mov ebx, [esp + 4]
      mov ecx, esi
      sub ecx, dFilSkipYNeg
      PXM_Conv_Eval                         ;Evaluate color at -1,+0
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval 2                       ;Evaluate color at +0,+0
      PXM_Conv_Add
      add ecx, dFilSkipYPos
      add ebx, 8

      PXM_Conv_Eval                         ;Evaluate color at +1,+0
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval 2                       ;Evaluate color at -1,+1
      PXM_Conv_Add
      add ecx, dFilSkipYPos
      add ebx, 8

      PXM_Conv_Eval                         ;Evaluate color at +0,+1
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval 2                       ;Evaluate color at +1,+1
      PXM_Conv_Add

      mov [edi], $PXM_GetNewColor()         ;Store the new generated color

      add esi, dSrcSkipY
      add edi, dDstSkipY
      dec dword ptr [esp]
      jnz @@YLoopLB
      add esp, 8                            ;Restore stack

      fUnload 1                             ;Free the last FPU register
;##/
;##\  ———— Bottom-Left Corner ——————————————————————————————————————————————————
      mov ebx, pFilter                      ;ebx -> filter data
      add ebx, 4
      mov ecx, esi
      sub ecx, dFilSkipYNeg
      PXM_Conv_Eval 2                       ;Evaluate color at +0,-1
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval 3                       ;Evaluate color at +1,-1
      PXM_Conv_Add
      add ecx, dFilSkipYPos
      add ebx, 8
      PXM_Conv_Eval                         ;Evaluate color at +0,+0
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval 2                       ;Evaluate color at +1,+0
      PXM_Conv_Add

      mov [edi], $PXM_GetNewColor()         ;Store the new generated color

      fUnload 1                             ;Free the last FPU register
;##/
;##\  ———— Right band ——————————————————————————————————————————————————————————
      SetObject esi,, pSrcPxm
      ;Calc skip values in source space
      mov eax, [esi].dWidth
      shl eax, 2
      mov dSrcSkipY, eax
      sub eax, 4
      mov dFilSkipYPos, eax
      add eax, 8
      mov dFilSkipYNeg, eax

      ;Calc position of first pixel to transfer in source space position +0,+1
      mov eax, [esi].dWidth
      add eax, dWidth
      dec eax
      shl eax, 2
      mov esi, [esp]
      add esi, eax
      ReleaseObject

      SetObject edi
      ;Calc skip values in destination space
      mov eax, [edi].dWidth
      shl eax, 2
      mov dDstSkipY, eax

      ;Calc position of first pixel to transfer in destination space
      mov eax, [edi].dWidth
      add eax, dWidth
      dec eax
      shl eax, 2
      mov edi, [esp + 4]
      add edi, eax
      ReleaseObject

      ;Do convolution of right band
      mov eax, dHeight
      sub eax, 2
      push eax
      align ALIGN_CODE
@@YLoopRB:
      mov ebx, pFilter                      ;ebx -> filter data
      mov ecx, esi
      sub ecx, dFilSkipYNeg
      PXM_Conv_Eval 2                       ;Evaluate color at -1,-1
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at +0,-1
      PXM_Conv_Add
      add ecx, dFilSkipYPos
      add ebx, 8

      PXM_Conv_Eval 2                       ;Evaluate color at -1,+0
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at +0,+0
      PXM_Conv_Add
      add ecx, dFilSkipYPos
      add ebx, 8

      PXM_Conv_Eval 2                       ;Evaluate color at -1,+1
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at +0,+1
      PXM_Conv_Add

      mov [edi], $PXM_GetNewColor()         ;Store the new generated color

      add esi, dSrcSkipY
      add edi, dDstSkipY
      dec dword ptr [esp]
      jnz @@YLoopRB
      add esp, 4                            ;Restore stack

      fUnload 1                             ;Free the last FPU register
;##/
;##\  ———— Inner Rect ——————————————————————————————————————————————————————————
      ;Calc inner rect skip values
      SetObject edi
      SetObject esi,,pSrcPxm

      ;Calc filter skip values
      mov eax, [esi].dWidth
      shl eax, 2
      lea ecx, [eax - 8]
      mov dFilSkipYPos, ecx
      add eax, 4
      mov dFilSkipYNeg, eax

      ;Adjust to inner rect
      sub dWidth, 2
      sub dHeight, 2
      mov dSrcSkipY, $Pxm_Bytes2Skip(dWidth, esi)

      ;Calc position of first pixel to transfer in source space
      mov eax, [esi].dWidth
      pop esi
      lea esi, [esi + 4*eax + 4]

      mov dDstSkipY, $Pxm_Bytes2Skip(dWidth, edi)

      ;Calc position of first pixel to transfer in destination space
      mov eax, [edi].dWidth
      pop edi
      lea edi, [edi + 4*eax + 4]

      ReleaseObject
      ReleaseObject

      ;Do convolution of inner RECT
      push dHeight
      align ALIGN_CODE
@@YLoopIR:
      push dWidth
@@XLoopIR:
      mov ebx, pFilter                      ;ebx -> filter data
      mov ecx, esi
      sub ecx, dFilSkipYNeg
      PXM_Conv_Eval                         ;Evaluate color at -1,-1
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at +0,-1
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at +1,-1
      PXM_Conv_Add

      add ecx, dFilSkipYPos
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at -1,+0
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at +0,+0
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at +1,+0
      PXM_Conv_Add

      add ecx, dFilSkipYPos
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at -1,+1
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at +0,+1
      PXM_Conv_Add
      add ecx, 4
      add ebx, 4
      PXM_Conv_Eval                         ;Evaluate color at +1,+1
      PXM_Conv_Add

      mov [edi], $PXM_GetNewColor()         ;Store the new generated color

      add esi, 4
      add edi, 4
      dec dword ptr [esp]
      jnz @@XLoopIR
      add esp, 4                            ;Restore stack
      add edi, dDstSkipY
      add esi, dSrcSkipY
      dec dword ptr [esp]
      jnz @@YLoopIR
      add esp, 4                            ;Restore stack

      fUnload 1                             ;Free the last FPU register
;##/
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.FillBkGnd
; Purpose:   Fills the pixelmap with a given color. No alfablending is applied!
; Arguments: Arg1: ARGB background color.
; Return:    Nothing.

Method Pixelmap.FillBkGnd, uses edi, dARGB:dword
    SetObject edx
    .if [edx].hDIBSection != 0
      mov ecx, [edx].BmpInfo.BITMAPINFO.bmiHeader.biSizeImage
      mov edi, [edx].pPixels
      shr ecx, 2                            ;byte count to dword count
      mov eax, dARGB
      RGB2BGR eax
      rep stosd
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.FillRect
; Purpose:   Fills a rectangular area of the Pixelmap with an ARGB value.
; Arguments: Arg1: RECT to fill.
;            Arg2: ARGB value.
; Return:    Nothing.

Method Pixelmap.FillRect, uses ebx edi esi, DstRct:RECT, dARGB:dword
    local dSkipY:dword, dWidth:dword, dHeight:dword
    local DstSpace:RECT, OutRct:RECT
    local dDens1:dword, dDens2:dword

    SetObject esi
    .if [esi].hDIBSection != 0
      ;Flip SrcRct if necessary
      Pxm_FlipInvertedRect DstRct

      ;Clip SrcRct to SrcSpace
      and DstSpace.left, 0
      and DstSpace.top, 0
      m2m DstSpace.right, [esi].dWidth, eax
      m2m DstSpace.bottom, [esi].dHeight, eax
      Pxm_ClipRect OutRct, DstRct, DstSpace

      mov eax, OutRct.right
      mov ecx, OutRct.bottom
      sub eax, OutRct.left
      jz @@EOM
      sub ecx, OutRct.top
      jz @@EOM
      mov dWidth, eax
      mov dHeight, ecx

      mov dSkipY, $Pxm_Bytes2Skip(dWidth, esi)
      mov esi, $Pxm_PixelAddr(OutRct.left, OutRct.top, esi)
      ReleaseObject

      mov ecx, dARGB
      RGB2BGR ecx
      mov eax, dARGB
      shr eax, 32-8

      .if al == 255
        mov ebx, dHeight
        align ALIGN_CODE
@YLoop2:
        mov edi, dWidth
@XLoop2:
        mov [esi], ecx
        add esi, 4
        dec edi
        jnz @XLoop2
        add esi, dSkipY
        dec ebx
        jnz @YLoop2

      .elseif al > 0
        mov dDens1, eax
        neg ax
        add ax, 0FFh
        mov dDens2, eax                     ;Dens2 = Complement to dDens1

        mov edx, ecx
        and ecx, PXM_MASK_1010
        and edx, PXM_MASK_0101
        shr ecx, 8
        imul ecx, dDens1
        imul edx, dDens1

        push dHeight
        align ALIGN_CODE
@@YLoop1:
        mov edi, dWidth
@@XLoop1:
        mov eax, [esi]                      ;Get original pixel
        mov ebx, eax
        and eax, PXM_MASK_1010
        and ebx, PXM_MASK_0101
        shr eax, 8
        imul eax, dDens2
        imul ebx, dDens2

        add eax, ecx
        add ebx, edx
        and eax, PXM_MASK_1010
        shr ebx, 8
        and ebx, PXM_MASK_0101
        or eax, ebx
        mov [esi], eax                      ;Store new color
        add esi, 4
        dec edi
        jnz @@XLoop1
        add esi, dSkipY
        dec dword ptr [esp]
        jnz @@YLoop1
        add esp, 4                          ;Restore stack
      .endif
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.FillRectChannel
; Purpose:   Fills a c color channel of rectangular area of the Pixelmap with a given value.
; Arguments: Arg1: RECT to fill.
;            Arg2: Color channel (PXM_CHANNEL_XXX).
;            Arg2: Fill value (Byte).
; Return:    Nothing.

Method Pixelmap.FillRectChannel, uses esi, DstRct:RECT, dChannel:dword, dFillValue:dword
    local dSkipY:dword, dWidth:dword, dHeight:dword
    local DstSpace:RECT, OutRct:RECT

    SetObject esi
    .if [esi].hDIBSection != 0
      ;Flip SrcRct if necessary
      Pxm_FlipInvertedRect DstRct

      ;Clip SrcRct to SrcSpace
      and DstSpace.left, 0
      and DstSpace.top, 0
      m2m DstSpace.right, [esi].dWidth, eax
      m2m DstSpace.bottom, [esi].dHeight, eax
      Pxm_ClipRect OutRct, DstRct, DstSpace

      mov eax, OutRct.right
      mov ecx, OutRct.bottom
      sub eax, OutRct.left
      jz @@EOM
      sub ecx, OutRct.top
      jz @@EOM
      mov dWidth, eax
      mov dHeight, ecx

      mov dSkipY, $Pxm_Bytes2Skip(dWidth, esi)
      mov esi, $Pxm_PixelAddr(OutRct.left, OutRct.top, esi)
      mov eax, dChannel
      movzx edx, [PXM_CHANNEL_OFFSET_TABLE + eax]
      add esi, edx
      ReleaseObject

      mov eax, dFillValue
      mov ecx, dHeight
      align ALIGN_CODE
@@YLoop:
      mov edx, dWidth
@@XLoop:
      mov [esi], al
      add esi, 4
      dec edx
      jnz @@XLoop
      add esi, dSkipY
      dec ecx
      jnz @@YLoop
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.FillRectGradHor
; Purpose:   Fills a rect with an horizontal gradient. Alphablending is applied if necessary.
; Arguments: Arg1: RECT to fill
;            Arg2: Left ARGB color.
;            Arg3: Right ARGB color.
; Return:    Nothing.

Method Pixelmap.FillRectGradHor, uses ebx edi esi, DstRct:RECT, dLftColor:dword, dRgtColor:dword
    local dRed:dword, dGreen:dword, dBlue:dword, dAlpha:dword, dBuffer:dword
    local dSkipX:dword, dSkipY:dword, dWidth:dword, dHeight:dword
    local DstSpace:RECT, OutRct:RECT
    local dDens1:dword, dDens2:dword

    SetObject esi
    .if [esi].hDIBSection != 0
      ;Flip SrcRct if necessary
      Pxm_FlipInvertedRect DstRct

      ;Clip SrcRct to SrcSpace
      and DstSpace.left, 0
      and DstSpace.top, 0
      m2m DstSpace.right, [esi].dWidth, eax
      m2m DstSpace.bottom, [esi].dHeight, eax
      Pxm_ClipRect OutRct, DstRct, DstSpace

      mov eax, OutRct.right
      mov ecx, OutRct.bottom
      sub eax, OutRct.left
      jz @@EOM
      sub ecx, OutRct.top
      jz @@EOM
      mov dWidth, eax
      mov dHeight, ecx

      ;Calculate X/Y skip values
      mov eax, [esi].dWidth
      shl eax, 2
      mov dSkipY, eax
      mul ecx
      sub eax, 4
      mov dSkipX, eax

      ;Calculate first pixel position
      mov esi, $Pxm_PixelAddr(OutRct.left, OutRct.top, esi)
      ReleaseObject

      ;Calculate the color steps
      fild dWidth

      ;Red step
      mov edx, dRgtColor
      mov ecx, dLftColor
      movzx eax, dl
      mov dBuffer, eax
      fild dBuffer
      movzx eax, cl
      mov dBuffer, eax
      fild dBuffer
      fsubp st(1), st(0)
      fdiv st(0), st(1)
      fxch st(1)

      ;Green step
      movzx eax, dh
      mov dBuffer, eax
      fild dBuffer
      movzx eax, ch
      mov dBuffer, eax
      fild dBuffer
      fsubp st(1), st(0)
      fdiv st(0), st(1)
      fxch st(1)

      ;Blue step
      shr edx, 16
      movzx eax, dl
      mov dBuffer, eax
      fild dBuffer
      shr ecx, 16
      movzx eax, cl
      mov dBuffer, eax
      fild dBuffer
      fsubp st(1), st(0)
      fdiv st(0), st(1)
      fxch st(1)

      ;Check if we have trasparent colors
      .if dh != 255 || ch != 255
        ;Transparent colors
        ;Alpha step
        movzx eax, dh
        mov dBuffer, eax
        fild dBuffer
        movzx eax, ch
        mov dBuffer, eax
        fild dBuffer
        fsubp st(1), st(0)
        fdiv st(0), st(1)
        fxch st(1)

        fUnload 1

        ;Load LftColor components
        mov edx, dLftColor
        movzx eax, dl
        mov dBuffer, eax
        fild dBuffer
        movzx eax, dh
        mov dBuffer, eax
        fild dBuffer
        shr edx, 16
        movzx eax, dl
        mov dBuffer, eax
        fild dBuffer
        movzx eax, dh
        mov dBuffer, eax
        fild dBuffer

        mov eax, dLftColor

        RGB2BGR eax
        push dWidth
        align ALIGN_CODE
@@XLoop0:
        mov edi, dHeight
@@YLoop0:
        push eax                            ;Save current color

        mov ecx, eax
        shr ecx, 32-8
        mov dDens1, ecx
        neg cx
        add cx, 0FFh
        mov dDens2, ecx                     ;Dens2 = Complement to dDens1

        mov ebx, eax                        ;Get gradient pixel, eax = edi
        and eax, PXM_MASK_1010
        and ebx, PXM_MASK_0101
        shr eax, 8
        imul eax, dDens1
        imul ebx, dDens1

        mov ecx, [esi]                      ;Get source pixel
        mov edx, ecx                        ;edx = ecx
        and ecx, PXM_MASK_1010
        and edx, PXM_MASK_0101
        shr ecx, 8
        imul ecx, dDens2
        imul edx, dDens2

        add eax, ecx                        ;Compose the 2 pixels
        add ebx, edx
        and eax, PXM_MASK_1010
        shr ebx, 8
        and ebx, PXM_MASK_0101
        or eax, ebx
        mov [esi], eax                      ;Store new color

        pop eax                             ;Restore current color

        add esi, dSkipY
        dec edi
        jnz @@YLoop0

        ;Compute new color
        fxch st(3)
        fadd st(0), st(7)
        fist dRed
        fxch st(3)

        fxch st(2)
        fadd st(0), st(6)
        fist dGreen
        fxch st(2)

        fxch st(1)
        fadd st(0), st(5)
        fist dBlue
        fxch st(1)

        fadd st(0), st(4)
        fist dAlpha

        ;Compose it
        mov ah, byte ptr dAlpha
        mov al, byte ptr dRed
        shl eax, 16
        mov ah, byte ptr dGreen
        mov al, byte ptr dBlue

        sub esi, dSkipX
        dec dword ptr [esp]
        jnz @@XLoop0
        add esp, 4

        fUnload 8

      ; ————————————————————————————————————————————————————

      .else
        ;Solid colors
        fUnload 1

        ;Load LftColor components
        mov edx, dLftColor
        movzx eax, dl
        mov dBuffer, eax
        fild dBuffer
        movzx eax, dh
        mov dBuffer, eax
        fild dBuffer
        shr edx, 16
        movzx eax, dl
        mov dBuffer, eax
        fild dBuffer

        mov ebx, dWidth
        mov eax, dLftColor
        RGB2BGR eax
        align ALIGN_CODE
@@XLoop1:
        mov edi, dHeight
@@YLoop1:
        mov [esi], eax
        add esi, dSkipY
        dec edi
        jnz @@YLoop1

        ;Compute new color
        fxch st(2)
        fadd st(0), st(5)
        fist dRed
        fxch st(2)

        fxch st(1)
        fadd st(0), st(4)
        fist dGreen
        fxch st(1)

        fadd st(0), st(3)
        fist dBlue

        ;Compose it
        mov ah, 255
        mov al, byte ptr dRed
        shl eax, 16
        mov ah, byte ptr dGreen
        mov al, byte ptr dBlue

        sub esi, dSkipX

        dec ebx
        jnz @@XLoop1

        fUnload 6

      .endif
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.FillRectGradVer
; Purpose:   Fills a rect with a vertical gradient. Alphablending is applied if necessary.
; Arguments: Arg1: RECT to fill
;            Arg2: Left ARGB color.
;            Arg3: Right ARGB color.
; Return:    Nothing.

Method Pixelmap.FillRectGradVer, uses ebx edi esi, DstRct:RECT, dTopColor:dword, dBtmColor:dword
    local dRed:dword, dGreen:dword, dBlue:dword, dAlpha:dword, dBuffer:dword
    local dSkipY:dword, dWidth:dword, dHeight:dword
    local DstSpace:RECT, OutRct:RECT
    local dDens1:dword, dDens2:dword

    SetObject esi
    .if [esi].hDIBSection != 0
      ;Flip SrcRct if necessary
      Pxm_FlipInvertedRect DstRct

      ;Clip SrcRct to SrcSpace
      and DstSpace.left, 0
      and DstSpace.top, 0
      m2m DstSpace.right, [esi].dWidth, eax
      m2m DstSpace.bottom, [esi].dHeight, eax
      Pxm_ClipRect OutRct, DstRct, DstSpace

      mov eax, OutRct.right
      mov ecx, OutRct.bottom
      sub eax, OutRct.left
      jz @@EOM
      sub ecx, OutRct.top
      jz @@EOM
      mov dWidth, eax
      mov dHeight, ecx

      ;Calculate X/Y skip values
      mov dSkipY, $Pxm_Bytes2Skip(dWidth, esi)

      ;Calculate first pixel position
      mov esi, $Pxm_PixelAddr(OutRct.left, OutRct.top, esi)
      ReleaseObject

      ;Calculate the color steps
      fild dHeight

      ;Red step
      mov edx, dBtmColor
      mov ecx, dTopColor
      movzx eax, dl
      mov dBuffer, eax
      fild dBuffer
      movzx eax, cl
      mov dBuffer, eax
      fild dBuffer
      fsubp st(1), st(0)
      fdiv st(0), st(1)
      fxch st(1)

      ;Green step
      movzx eax, dh
      mov dBuffer, eax
      fild dBuffer
      movzx eax, ch
      mov dBuffer, eax
      fild dBuffer
      fsubp st(1), st(0)
      fdiv st(0), st(1)
      fxch st(1)

      ;Blue step
      shr edx, 16
      movzx eax, dl
      mov dBuffer, eax
      fild dBuffer
      shr ecx, 16
      movzx eax, cl
      mov dBuffer, eax
      fild dBuffer
      fsubp st(1), st(0)
      fdiv st(0), st(1)
      fxch st(1)

      ;Check if we have trasparent colors
      .if dh != 255 || ch != 255
        ;Transparent colors
        ;Alpha step
        movzx eax, dh
        mov dBuffer, eax
        fild dBuffer
        movzx eax, ch
        mov dBuffer, eax
        fild dBuffer
        fsubp st(1), st(0)
        fdiv st(0), st(1)
        fxch st(1)

        fUnload 1

        ;Load LftColor components
        mov edx, dTopColor
        movzx eax, dl
        mov dBuffer, eax
        fild dBuffer
        movzx eax, dh
        mov dBuffer, eax
        fild dBuffer
        shr edx, 16
        movzx eax, dl
        mov dBuffer, eax
        fild dBuffer
        movzx eax, dh
        mov dBuffer, eax
        fild dBuffer

        mov eax, dTopColor
        RGB2BGR eax
        push dHeight
        align ALIGN_CODE
@@YLoop0:
        mov edi, dWidth
@@XLoop0:
        push eax                            ;Save current color

        mov ecx, eax
        shr ecx, 32-8
        mov dDens1, ecx
        neg cx
        add cx, 0FFh
        mov dDens2, ecx                     ;Dens2 = Complement to dDens1

        mov ebx, eax                        ;Get gradient pixel, eax = edi
        and eax, PXM_MASK_1010
        and ebx, PXM_MASK_0101
        shr eax, 8
        imul eax, dDens1
        imul ebx, dDens1

        mov ecx, [esi]                      ;Get source pixel
        mov edx, ecx                        ;edx = ecx
        and ecx, PXM_MASK_1010
        and edx, PXM_MASK_0101
        shr ecx, 8
        imul ecx, dDens2
        imul edx, dDens2

        add eax, ecx                        ;Compose the 2 pixels
        add ebx, edx
        and eax, PXM_MASK_1010
        shr ebx, 8
        and ebx, PXM_MASK_0101
        or eax, ebx
        mov [esi], eax                      ;Store new color

        pop eax                             ;Restore current color

        add esi, 4
        dec edi
        jnz @@XLoop0

        ;Compute new color
        fxch st(3)
        fadd st(0), st(7)
        fist dRed
        fxch st(3)

        fxch st(2)
        fadd st(0), st(6)
        fist dGreen
        fxch st(2)

        fxch st(1)
        fadd st(0), st(5)
        fist dBlue
        fxch st(1)

        fadd st(0), st(4)
        fist dAlpha

        ;Compose it
        mov ah, byte ptr dAlpha
        mov al, byte ptr dRed
        shl eax, 16
        mov ah, byte ptr dGreen
        mov al, byte ptr dBlue

        add esi, dSkipY
        dec dword ptr [esp]
        jnz @@YLoop0
        add esp, 4

        fUnload 8

      ; ————————————————————————————————————————————————————

      .else
        ;Solid colors
        fUnload 1

        ;Load LftColor components
        mov edx, dTopColor
        movzx eax, dl
        mov dBuffer, eax
        fild dBuffer
        movzx eax, dh
        mov dBuffer, eax
        fild dBuffer
        shr edx, 16
        movzx eax, dl
        mov dBuffer, eax
        fild dBuffer

        mov ebx, dHeight
        mov eax, dTopColor
        RGB2BGR eax
        align ALIGN_CODE
@@YLoop1:
        mov edi, dWidth
@@XLoop1:
        mov [esi], eax
        add esi, 4
        dec edi
        jnz @@XLoop1

        ;Compute new color
        fxch st(2)
        fadd st(0), st(5)
        fist dRed
        fxch st(2)

        fxch st(1)
        fadd st(0), st(4)
        fist dGreen
        fxch st(1)

        fadd st(0), st(3)
        fist dBlue

        ;Compose it
        mov ah, 255
        mov al, byte ptr dRed
        shl eax, 16
        mov ah, byte ptr dGreen
        mov al, byte ptr dBlue

        add esi, dSkipY

        dec ebx
        jnz @@YLoop1

        fUnload 6

      .endif
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.FlipRectHor
; Purpose:   Flips horizontally a rectangular area of the Pixelmap.
; Arguments: RECT to flip.
; Return:    Nothing.

Method Pixelmap.FlipRectHor, uses ebx edi esi, DstRct:RECT
    local dSkipY:dword, dWidth:dword, dHeight:dword
    local DstSpace:RECT, OutRct:RECT

    SetObject esi
    .if [esi].hDIBSection != 0
      ;Flip DstRct if necessary
      Pxm_FlipInvertedRect DstRct

      ;Clip DstRct to SrcSpace
      and DstSpace.left, 0
      and DstSpace.top, 0
      m2m DstSpace.right, [esi].dWidth, eax
      m2m DstSpace.bottom, [esi].dHeight, eax
      Pxm_ClipRect OutRct, DstRct, DstSpace

      ;Calc width and height
      mov ecx, OutRct.right
      mov eax, OutRct.bottom
      sub ecx, OutRct.left
      jz @@EOM
      sub eax, OutRct.top
      jz @@EOM
      mov dWidth, ecx
      mov dHeight, eax

      ;Calculate skip value
      shr ecx, 1
      mov eax, [esi].dWidth
      sub eax, ecx
      shl eax, 2
      mov dSkipY, eax

      ;Calculate pointer to first pixel in OutRct
      mov esi, $Pxm_PixelAddr(OutRct.left, OutRct.top, esi)
      ReleaseObject

      ;Initialize flip routine
      mov ebx, dHeight

      ;Flip now
      align ALIGN_CODE
      @YLoop:
        mov edi, dWidth
        lea edx, [esi + 4*edi]
        shr edi, 1
        @XLoop:
;          mov eax, [esi]
;          xchg [edx], eax      ;Super slow... replaced by the next 4 lines
;          mov [esi], eax

          mov eax, [esi]
          mov ecx, [edx]
          mov [esi], ecx
          mov [edx], eax
              
          add esi, 4
          sub edx, 4
          dec edi
        jnz @XLoop
        add esi, dSkipY
        dec ebx
      jnz @YLoop
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.FlipRectVer
; Purpose:   Flips vertically a rectangular area of the Pixelmap.
; Arguments: Arg1: RECT to flip.
; Return:    Nothing.

Method Pixelmap.FlipRectVer, uses ebx edi esi, DstRct:RECT
    local dSkipYPos:dword, dSkipYNeg:dword, dWidth:dword, dHeight:dword
    local DstSpace:RECT, OutRct:RECT

    SetObject esi
    .if [esi].hDIBSection != 0
      ;Flip DstRct if necessary
      Pxm_FlipInvertedRect DstRct

      ;Clip DstRct to SrcSpace
      and DstSpace.left, 0
      and DstSpace.top, 0
      m2m DstSpace.right, [esi].dWidth, eax
      m2m DstSpace.bottom, [esi].dHeight, eax
      Pxm_ClipRect OutRct, DstRct, DstSpace

      ;Calc width and height
      mov eax, OutRct.right
      mov ecx, OutRct.bottom
      sub eax, OutRct.left
      jz @@EOM
      sub ecx, OutRct.top
      jz @@EOM
      mov dWidth, eax
      mov dHeight, ecx

      ;Calculate skip values
      mov ecx, [esi].dWidth
      sub ecx, eax
      shl ecx, 2
      mov dSkipYPos, ecx
      add eax, [esi].dWidth
      shl eax, 2
      mov dSkipYNeg, eax

      ;Calculate pointer to first pixel in OutRct
      mov esi, $Pxm_PixelAddr(OutRct.left, OutRct.top, esi)
      ReleaseObject

      ;Initialize flip routine
      mov ebx, dHeight
      mov eax, dWidth
      mul ebx
      shl eax, 2
      mov edx, esi
      add edx, eax
      shr ebx, 1

      ;Flip now
      align ALIGN_CODE
      @YLoop:
        mov edi, dWidth
        @XLoop:
;          mov eax, [esi]
;          xchg [edx], eax      ;Super slow!!!
;          mov [esi], eax
    
          mov eax, [esi]
          mov ecx, [edx]
          mov [esi], ecx
          mov [edx], eax

          add esi, 4
          add edx, 4
          dec edi
        jnz @XLoop
        add esi, dSkipYPos
        sub edx, dSkipYNeg
        dec ebx
      jnz @YLoop

    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.Gray
; Purpose:   Transforms the Pixelmap into a gray scale image.
; Arguments: Arg1: RECT to transform.
; Return:    Nothing.

Method Pixelmap.Gray, uses ebx edi esi, DstRct:RECT
    local dSkipY:dword, dWidth:dword, dHeight:dword
    local DstSpace:RECT, OutRct:RECT

    SetObject esi
    .if [esi].hDIBSection != 0
      ;Flip SrcRct if necessary
      Pxm_FlipInvertedRect DstRct

      ;Clip SrcRct to SrcSpace
      and DstSpace.left, 0
      and DstSpace.top, 0
      m2m DstSpace.right, [esi].dWidth, eax
      m2m DstSpace.bottom, [esi].dHeight, eax
      Pxm_ClipRect OutRct, DstRct, DstSpace

      mov eax, OutRct.right
      mov ecx, OutRct.bottom
      sub eax, OutRct.left
      jz @@EOM
      sub ecx, OutRct.top
      jz @@EOM
      mov dWidth, eax
      mov dHeight, ecx

      ;Calculate X/Y skip values
      mov dSkipY, $Pxm_Bytes2Skip(dWidth, esi)

      ;Calculate first pixel position
      mov esi, $Pxm_PixelAddr(OutRct.left, OutRct.top, esi)
      ReleaseObject

      mov ebx, dHeight
@@Yloop:
      mov edi, dWidth
@@Xloop:
      mov eax, [esi]
      movzx ecx, al                       ;Blue
      imul ecx, ecx, 25
      movzx edx, ah                       ;Green
      imul edx, edx, 129
      add edx, ecx
      shr eax, 16                         ;Red
      imul eax, eax, 66
      add eax, edx
      add eax, 128
      shr eax, 8
      add eax, 16
      and eax, 0FFh
      imul eax, eax, 010101h
      mov [esi], eax
      add esi, 4
      dec edi
      jnz @@Xloop
      add esi, dSkipY
      dec ebx
      jnz @@Yloop
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.Litho
; Purpose:   Transforms the Pixelmap into a lithographic image.
; Arguments: Arg1: Destination RECT.
;            Arg2: Threshhold value.
; Return:    Nothing.
; Notes:     - Original code by Edgar Hansen - Donkey -

Method Pixelmap.Litho, uses ebx edi esi, DstRct:RECT, dThreshhold:dword
    local DstSpace:RECT, OutRct:RECT
    local dSkipY:dword, dWidth:dword, dHeight:dword
    local dWhite:dword, dBlack:dword

    SetObject esi
    .if [esi].hDIBSection != 0
      ;Flip SrcRct if necessary
      Pxm_FlipInvertedRect DstRct

      ;Clip SrcRct to SrcSpace
      and DstSpace.left, 0
      and DstSpace.top, 0
      m2m DstSpace.right, [esi].dWidth, eax
      m2m DstSpace.bottom, [esi].dHeight, eax
      Pxm_ClipRect OutRct, DstRct, DstSpace

      mov eax, OutRct.right
      mov ecx, OutRct.bottom
      sub eax, OutRct.left
      jz @@EOM
      sub ecx, OutRct.top
      jz @@EOM
      mov dWidth, eax
      mov dHeight, ecx

      ;Calculate X/Y skip values
      mov dSkipY, $Pxm_Bytes2Skip(dWidth, esi)

      ;Calculate first pixel position
      mov esi, $Pxm_PixelAddr(OutRct.left, OutRct.top, esi)
      ReleaseObject

      ;Initialize start values
      and dBlack, 0
      mov dWhite, 00FFFFFFh
      mov eax, dThreshhold
      or eax, eax
      .if Sign?
        mov dBlack, 00FFFFFFh
        and dWhite, 0
        neg eax
        mov dThreshhold, eax
      .endif

      mov ebx, dHeight
@@Yloop:
      mov edi, dWidth
@@Xloop:
      mov eax, [esi]
      movzx ecx, al                       ;Blue
      imul ecx, ecx, 25
      movzx edx, ah                       ;Green
      imul edx, edx, 129
      add edx, ecx
      shr eax, 16                         ;Red
      imul eax, eax, 66
      add eax, edx
      add eax, 128
      shr eax, 8
      add eax, 16
      and eax, 0FFh

      .if eax <= dThreshhold
        mov eax, dBlack
      .else
        mov eax, dWhite
      .endif
      mov [esi], eax

      mov [esi], eax
      add esi, 4
      dec edi
      jnz @@Xloop
      add esi, dSkipY
      dec ebx
      jnz @@Yloop
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.Replace_ARGB_ARGB
; Purpose:   Replaces an ARGB color with another.
; Arguments: Arg1: Destination RECT.
;            Arg2: Final ARGB value.
;            Arg3: ARGB value to replace.
; Return:    Nothing.

Method Pixelmap.Replace_ARGB_ARGB, uses ebx edi esi, DstRct:RECT, dDstARGB:dword, dSrcARGB:dword
    local dSkipY:dword, dWidth:dword, dHeight:dword
    local DstSpace:RECT, OutRct:RECT

    SetObject esi
    .if [esi].hDIBSection != 0

      ;Flip SrcRct if necessary
      Pxm_FlipInvertedRect DstRct

      ;Clip SrcRct to SrcSpace
      and DstSpace.left, 0
      and DstSpace.top, 0
      m2m DstSpace.right, [esi].dWidth, eax
      m2m DstSpace.bottom, [esi].dHeight, eax
      Pxm_ClipRect OutRct, DstRct, DstSpace

      mov eax, OutRct.right
      mov ecx, OutRct.bottom
      sub eax, OutRct.left
      jz @@EOM
      sub ecx, OutRct.top
      jz @@EOM
      mov dWidth, eax
      mov dHeight, ecx

      mov dSkipY, $Pxm_Bytes2Skip(dWidth, esi)
      mov esi, $Pxm_PixelAddr(OutRct.left, OutRct.top, esi)
      ReleaseObject

      mov eax, dSrcARGB
      RGB2BGR eax
      mov ecx, dDstARGB
      RGB2BGR ecx

      mov ebx, dHeight
      align ALIGN_CODE
@YLoop:
      mov edi, dWidth
@XLoop:
      mov edx, [esi]
      sub edx, eax
      .if Zero?
         mov [esi], ecx
      .endif
      add esi, 4
      dec edi
      jnz @XLoop
      add esi, dSkipY
      dec ebx
      jnz @YLoop
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.Replace_RGB_ARGB
; Purpose:   Replaces a pixel that matchs the RGB colors with an ARGB color.
; Arguments: Arg1: RECT where to applay the change.
;            Arg2: Final ARGB value.
;            Arg3: RGB value to change.
; Return:    Nothing.

Method Pixelmap.Replace_RGB_ARGB, uses ebx edi esi, DstRct:RECT, dDstARGB:dword, dSrcRGB:dword
    local dSkipY:dword, dWidth:dword, dHeight:dword
    local DstSpace:RECT, OutRct:RECT

    SetObject esi
    .if [esi].hDIBSection != 0

      ;Flip SrcRct if necessary
      Pxm_FlipInvertedRect DstRct

      ;Clip SrcRct to SrcSpace
      and DstSpace.left, 0
      and DstSpace.top, 0
      m2m DstSpace.right, [esi].dWidth, eax
      m2m DstSpace.bottom, [esi].dHeight, eax
      Pxm_ClipRect OutRct, DstRct, DstSpace

      mov eax, OutRct.right
      mov ecx, OutRct.bottom
      sub eax, OutRct.left
      jz @@EOM
      sub ecx, OutRct.top
      jz @@EOM
      mov dWidth, eax
      mov dHeight, ecx

      mov dSkipY, $Pxm_Bytes2Skip(dWidth, esi)
      mov esi, $Pxm_PixelAddr(OutRct.left, OutRct.top, esi)
      ReleaseObject

      mov eax, dSrcRGB
      RGB2BGR eax
      and eax, PXM_MASK_REG_RGB
      mov ecx, dDstARGB
      RGB2BGR ecx

      mov ebx, dHeight
      align ALIGN_CODE
@YLoop:
      mov edi, dWidth
@XLoop:
      mov edx, [esi]
      and edx, PXM_MASK_REG_RGB
      sub edx, eax
      .if Zero?
         mov [esi], ecx
      .endif
      add esi, 4
      dec edi
      jnz @XLoop
      add esi, dSkipY
      dec ebx
      jnz @YLoop
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.Replace_RGB_RGB
; Purpose:   Replaces all pixels with a given RGB value.
; Arguments: Arg1: Destination RECT.
;            Arg2: Final RGB value.
;            Arg3: RGB value to replace.
; Return:    Nothing.

Method Pixelmap.Replace_RGB_RGB, uses ebx edi esi, DstRct:RECT, dDstRGB:dword, dSrcRGB:dword
    local dSkipY:dword, dWidth:dword, dHeight:dword
    local DstSpace:RECT, OutRct:RECT

    SetObject esi
    .if [esi].hDIBSection != 0

      ;Flip SrcRct if necessary
      Pxm_FlipInvertedRect DstRct

      ;Clip SrcRct to SrcSpace
      and DstSpace.left, 0
      and DstSpace.top, 0
      m2m DstSpace.right, [esi].dWidth, eax
      m2m DstSpace.bottom, [esi].dHeight, eax
      Pxm_ClipRect OutRct, DstRct, DstSpace

      mov eax, OutRct.right
      mov ecx, OutRct.bottom
      sub eax, OutRct.left
      jz @@EOM
      sub ecx, OutRct.top
      jz @@EOM
      mov dWidth, eax
      mov dHeight, ecx

      mov dSkipY, $Pxm_Bytes2Skip(dWidth, esi)
      mov esi, $Pxm_PixelAddr(OutRct.left, DstRct.top, esi)
      ReleaseObject

      mov eax, dSrcRGB
      RGB2BGR eax
      and eax, PXM_MASK_REG_RGB
      mov ecx, dDstRGB
      RGB2BGR ecx
      and ecx, PXM_MASK_REG_RGB

      mov ebx, dHeight
      align ALIGN_CODE
@YLoop:
      mov edi, dWidth
@XLoop:
      mov edx, [esi]
      and edx, PXM_MASK_REG_RGB
      sub edx, eax
      .if Zero?
        mov edx, [esi]
        and edx, PXM_MASK_REG_ALPHA    ;Preserve alfa value
        or edx, ecx
        mov [esi], edx
      .endif
      add esi, 4
      dec edi
      jnz @XLoop
      add esi, dSkipY
      dec ebx
      jnz @YLoop
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.Reset
; Purpose:   Frees associated resources.
; Arguments: None.
; Return:    Nothing.

Method Pixelmap.Reset, NOFRAME
    push esi
    SetObject esi
    .if [esi].hDC != 0
      invoke SelectObject, [esi].hDC, [esi].hPrvBmp
      invoke DeleteDC, [esi].hDC
    .endif
    .if [esi].hDIBSection != 0
      invoke DeleteObject, [esi].hDIBSection
    .endif
    pop esi
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.SetAlphaMask
; Purpose:   Copies a channel of a given pixelmap to the alfa channel.
; Arguments: Arg1: -> Source Pixelmap.
;            Arg2: Source RECT.
;            Arg3: Destination starting POINT.
;            Arg4: Channel to use (PXM_CHANNEL_XXX).
; Return:    Nothing.

Method Pixelmap.SetAlphaMask, uses ebx edi esi, pSrcPxm:Pointer, SrcRct:RECT, DstPnt:POINT, dChannel:dword
    local dDstSkipY:dword, dSrcSkipY:dword, dSrcMask:dword
    local dWidth:dword, dHeight:dword
    local SrcSpace:RECT, DstSpace:RECT, OutRct:RECT

    SetObject edi
    SetObject esi,,pSrcPxm

    .if [esi].hDIBSection != 0 && [edi].hDIBSection != 0
      ;Flip SrcRct if necessary
      Pxm_FlipInvertedRect SrcRct

      ;Clip SrcRct to SrcSpace
      and SrcSpace.left, 0
      and SrcSpace.top, 0
      m2m SrcSpace.right, [esi].dWidth, eax
      m2m SrcSpace.bottom, [esi].dHeight, eax
      Pxm_ClipRect OutRct, SrcRct, SrcSpace

      ;Clip to DstSpace
      mov eax, SrcRct.left
      sub eax, DstPnt.x
      mov DstSpace.left, eax
      add eax, [edi].dWidth
      mov DstSpace.right, eax
      mov eax, SrcRct.top
      sub eax, DstPnt.y
      mov DstSpace.top, eax
      add eax, [edi].dHeight
      mov DstSpace.bottom, eax
      Pxm_ClipRect OutRct, OutRct, DstSpace

      ;Calc transfer sizes
      sub eax, ecx
      jz @@EOM
      mov dHeight, eax
      mov eax, OutRct.right
      sub eax, OutRct.left
      jz @@EOM
      mov dWidth, eax

      ;Calc skip values
      mov dDstSkipY, $Pxm_Bytes2Skip(dWidth, edi)
      mov dSrcSkipY, $Pxm_Bytes2Skip(dWidth, esi)

      ;Calc position of first pixel to transfer in source space
      mov esi, $Pxm_PixelAddr(OutRct.left, OutRct.top, esi)
      ReleaseObject

      ;Calc position of first pixel to transfer in destination space
      mov eax, OutRct.top
      sub eax, SrcRct.top
      add eax, DstPnt.y
      mul [edi].dWidth
      add eax, OutRct.left
      sub eax, SrcRct.left
      add eax, DstPnt.x
      shl eax, 2
      add eax, [edi].pPixels
      mov edi, eax
      ReleaseObject

      ;Prepare source channel mask anf shift value
      mov edx, dChannel
      movzx eax, byte ptr [PXM_CHANNEL_REG_MASK_TABLE + edx]
      mov dSrcMask, eax
      movzx ecx, byte ptr [PXM_CHANNEL_OFFSET_TABLE + edx]
      mov ecx, 24
      shl edx, 3
      sub ecx, edx

;      mov eax, 0FFh
;      mov ecx, dChannel
;      and ecx, 011b
;      shl ecx, 3
;      shl eax, cl
;      mov dSrcMask, eax
;      neg ecx
;      add ecx, 24

      ;Do transfer of alfa channel
      push dHeight
      align ALIGN_CODE
@YLoop:
      mov ebx, dWidth
@XLoop:
      mov eax, [esi]
      mov edx, [edi]
      and eax, dSrcMask
      and edx, PXM_MASK_REG_RGB
      shl eax, cl
      or eax, edx
      mov [edi], eax
      add esi, 4
      add edi, 4
      dec ebx
      jnz @XLoop
      add esi, dSrcSkipY
      add edi, dDstSkipY
      dec dword ptr [esp]
      jnz @YLoop
      add esp, 4
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.UV2XY
; Purpose:   Convert pixel coordinate from floating UV to floating XY.
; Arguments: Arg1: -> output real4 coordinates, i.e. Vec2 (2 x real4).
;            Arg2: -> input real4 coordinates, i.e. Vec2 (2 x real4).
;            Arg3: (optional) addressing mode:
;                   - PXM_ADDR_LIN: (default) linear conversion, no limit is applied.
;                   - PXM_ADDR_WRAP: tiles if range [0..1] is exceeded.
;                   - PXM_ADDR_MIRROR: mirrors if range [0..1] is exceeded.
;                   - PXM_ADDR_CLAMP: returns 1 or 0 if range [0..1] is exceeded.
; Return:    eax = Error code.

CFloat4 PXM_HALF, 0.5
CFloat4 PXM_TWO,  2.0

Method Pixelmap.UV2XY, uses esi, pOutCoords:Pointer, pInCoords:Pointer
    local wCurFpuCtrl:word, wPrvFpuCtrl:word, dBuffer:dword

    SetObject esi
    mov eax, pInCoords
    or eax, pInCoords
    jz @@Err
    mov eax, pOutCoords
    or eax, pOutCoords
    jnz @F
@@Err:
    OCall esi.ErrorReport, NULL, PXM_INVALID_POINTER
    jmp @@EOM
@@:
    .if ecx == 1
      mov ecx, [pInCoords + 4]                ;ecx = Address mode
    .endif
    JumpOn ecx, @@ADDR_LIN, @@ADDR_WRAP, @@ADDR_MIRROR, @@ADDR_CLAMP
    OCall esi.ErrorReport, NULL, PXM_INVALID_ADDR_MODE
    jmp @@EOM

@@ADDR_LIN:
    fld real4 ptr [eax]
    fld real4 ptr [eax + 4]
    jmp @@Final

@@ADDR_WRAP:
    fstcw wPrvFpuCtrl                         ;Store FPU control word
    m2m wCurFpuCtrl, wPrvFpuCtrl, cx          ;Duplicate value

    BitSet wCurFpuCtrl, BIT10                 ;Modify the control word to
    BitClr wCurFpuCtrl, BIT11                 ;  round to -inf
    fldcw wCurFpuCtrl                         ;Restore modified FPU control word

    fld real4 ptr [eax]
    fld st(0)
    frndint                                   ;Round down
    fsubp st(1), st(0)                        ;Frac(x) = x - int(x)

    fld real4 ptr [eax + 4]
    fld st(0)
    frndint                                   ;Round down
    fsubp st(1), st(0)                        ;Frac(x) = x - int(x)

    fldcw wPrvFpuCtrl                         ;Restore previous FPU control word

    jmp @@Final

@@ADDR_MIRROR:
    fstcw wPrvFpuCtrl                         ;Store FPU control word
    m2m wCurFpuCtrl, wPrvFpuCtrl, cx          ;Duplicate value
    BitSet wCurFpuCtrl, BIT10                 ;Modify the control word to
    BitClr wCurFpuCtrl, BIT11                 ;  round to -inf
    fldcw wCurFpuCtrl                         ;Restore modified FPU control word

    fld real4 ptr [eax + 4]                   ;y = abs(x-z) and z = int((x+1)/2) * 2
    fld st(0)
    fld1
    faddp st(1), st(0)
    fmul PXM_HALF
    frndint                                   ;Round down
    fmul PXM_TWO
    fsubp st(1), st(0)
    fabs

    fld real4 ptr [eax]
    fld st(0)
    fld1
    faddp st(1), st(0)
    fmul PXM_HALF
    frndint                                   ;Round down
    fmul PXM_TWO
    fsubp st(1), st(0)
    fabs

    fldcw wPrvFpuCtrl                         ;Restore previous FPU control word

    jmp @@Final

@@ADDR_CLAMP:
    push ebx
    mov ebx, eax
    fcmp real4 ptr [ebx], 3F800000h           ;1.0
    jg @@XClampTo1
    fcmp real4 ptr [ebx], 00000000h           ;0.0
    jl @@XClampTo0
    fld real4 ptr [ebx]
    jmp @F
@@XClampTo1:
    fld1
    jmp @F
@@XClampTo0:
    fldz

@@:
    fcmp real4 ptr [ebx + 4], 3F800000h       ;1.0
    jg @@YClampTo1
    fcmp real4 ptr [ebx + 4], 00000000h       ;0.0
    jl @@YClampTo0
    fld real4 ptr [ebx + 4]
    jmp @F
@@YClampTo1:
    fld1
    jmp @F
@@YClampTo0:
    fldz
@@:
    pop ebx

@@Final:
    mov edx, pOutCoords                       ;Compute XY coordinates
    mov eax, [esi].dHeight
    mov ecx, [esi].dWidth
    dec eax
    dec ecx
    mov dBuffer, eax
    fimul dBuffer                             ;Multiply by Height - 1
    mov dBuffer, ecx
    fstp real4 ptr [edx + 4]
    fimul dBuffer                             ;Multiply by Width - 1
    xor eax, eax                              ;Return OBJ_OK
    fstp real4 ptr [edx]

MethodEnd


; ==================================================================================================
; Private methods
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.Gamma2Linear
; Purpose:   Converts from gamma to linear color space.
; Arguments: Arg1: ARGB color.
; Return:    eax = Converted ARGB color.

Method Pixelmap.Gamma2Linear, NOFRAME, dARGB:dword
    SetObject ecx,, [esp + 4]
    mov edx, [ecx].pGam2LinTable
    mov eax, [esp + 8]                        ;dARGB
    movzx ecx, byte ptr [esp + 08]            ;Blue channel
    and eax, PXM_MASK_REG_ALPHA               ;Alpha is unchanged
    or al, byte ptr [ecx + edx]               ;Get the blue corrected value
    movzx ecx, byte ptr [esp + 09]            ;Green Channel
    or ah, byte ptr [ecx + edx]               ;Get the green corrected value
    movzx ecx, byte ptr [esp + 10]            ;Red Channel
    movzx ecx, byte ptr [ecx + edx]           ;Get the red corrected value
    shl ecx, 16
    or eax, ecx
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    Pixelmap.Linear2Gamma
; Purpose:   Converts from linear to gamma color space.
; Arguments: Arg1: ARGB color.
; Return:    eax = Converted ARGB color.

Method Pixelmap.Linear2Gamma, NOFRAME, dARGB:dword
    SetObject ecx,, [esp + 4]
    mov edx, [ecx].pLin2GamTable
    mov eax, [esp + 8]                        ;dARGB
    movzx ecx, byte ptr [esp + 08]            ;Blue channel
    and eax, PXM_MASK_REG_ALPHA               ;Alpha is unchanged
    or al, byte ptr [ecx + edx]               ;Get the blue corrected value
    movzx ecx, byte ptr [esp + 09]            ;Green Channel
    or ah, byte ptr [ecx + edx]               ;Get the green corrected value
    movzx ecx, byte ptr [esp + 10]            ;Red Channel
    movzx ecx, byte ptr [ecx + edx]           ;Get the red corrected value
    shl ecx, 16
    or eax, ecx
MethodEnd


; ——————————————————————————————————————————————————————————————————————————————————————————————————

; Method:	Pixelmap.PrintAt
; Purpose:	Print text into the pixelmap
Method Pixelmap.PrintAt,uses esi, hFont, pText, pt:POINT
LOCAL hBmp
LOCAL rc:RECT
LOCAL hDc	

	SetObject esi

	;---
	;Obtain the dimensions of the printed text, in the context of the main device:		
	;-Temporarily select the font into the main DC (preserve old font)
	push $invoke  (SelectObject,[esi].hDC,hFont)
	;-Measure the rect size of the printed text, IF we printed it to the main DC
	invoke SetBkMode,[esi].hDC,TRANSPARENT
	invoke DrawTextEx,[esi].hDC,pText,-1,addr rc,DT_CALCRECT,0
	;-Restore the old font
	pop eax
	invoke  SelectObject,[esi].hDC,eax
	;---
	
	
	;Create a temp DC, and a bitmap of RECT size
	mov hDc,$invoke  (CreateCompatibleDC,[esi].hDC)
	mov hBmp,$invoke (CreateCompatibleBitmap,[esi].hDC,rc.right,rc.bottom)

	;Select the BMP into the temp DC
	invoke  SelectObject,hDc,hBmp
	
	;Fill the new bitmap with a background color
	invoke  FillRect,hDc,addr rc,$invoke  (GetSysColorBrush,COLOR_BTNFACE)
	
	;Select the font into the temp DC
	invoke  SelectObject,hDc,hFont	
	
	;Render the text to the DC (and thus into the BMP)
	invoke  SetBkMode,hDc,TRANSPARENT
	invoke  DrawText,hDc,pText,-1,addr rc,DT_WORDBREAK

	;Copy the BMP into this Pixelmap
	OCall esi.CopyBmp,hDc,hBmp, rc,  pt
	
	;Release temp DC and BMP
	invoke  DeleteDC,hDc
	invoke  DeleteObject,hBmp
	
	
MethodEnd

; Method:	Pixelmap.Line
Method Pixelmap.Line,uses esi,x0,y0,x1,y1,color
LOCAL _dy:SDWORD,_dx:SDWORD, stepx, stepy,fraction:SDWORD
	SetObject esi
	
	mov eax,y1
	sub eax,y0
	mov _dy,eax
	
	mov eax,x1
	sub eax,x0
	mov _dx,eax
	
	.if _dy < 0
	  xor eax,eax
	  sub eax,_dy
	  mov _dy,eax
	  mov stepy,-1 
	.else 
		mov stepy , 1
	.endif
 	.if _dx < 0
 	  xor eax,eax
 	  sub eax,_dx
 	  mov _dx,eax
 	  mov stepx, -1 
	.else 
	 	mov stepx, 1 
	.endif
	
	shl _dy,1
	shl _dx,1
 
	OCall esi.SetPixel, x0, y0, color 
	
	mov eax,_dx
	.if eax > _dy
		shr eax,1
		mov edx,_dy
		sub edx,eax
		mov fraction,edx
		
		mov eax,x0
		.while eax != x1

			.if fraction >= 0
				mov eax,stepy
				add y0,eax
				mov eax,_dx
				sub fraction,eax
			.endif
   			mov eax,stepx
   			add x0,eax 
   			mov eax,_dy
   			add fraction,eax
   		    OCall esi.SetPixel, x0, y0, color 
   		    mov eax,x0
		.endw
	.else 
		mov edx,_dy
		shr edx,1
		sub eax,edx
		mov fraction,eax

		mov eax,y0
		.while eax != y1

			.if fraction >= 0
				mov eax,stepx
				add x0,eax
				mov eax,_dy
				sub fraction,eax
			.endif
			mov eax,stepy
			add y0,eax
			mov eax,_dx
			add fraction,eax
			OCall esi.SetPixel, x0, y0, color 
			mov eax,y0
		.endw
	.endif
MethodEnd


; Method:		Pixelmap.Circle
; Purpose:		Plot a Circle on pixelmap
; Arguments:	PosX,PosY = coordinate of center of circle
;				Rad = Radius in Pixels
;				color = Color to draw with
Method Pixelmap.Circle,uses esi, posX, posY, rad, color
LOCAL x,y
LOCAL step:real4,angle:real4

	SetObject esi
	fld1			;Calculate the angle of a triangle whose
	mov eax,rad		;base is the radius, and whose side is 1.0
	fildReg eax		;This is the inverse tangent of 1/radius
	fdiv			;Note that this value is in radians
	fArcTan
	fstp step
	
	mov x,0
	mov y,0
	
	mov angle,0
	.repeat	
		fld angle	;X and Y position are calculated via trig
		fsincos
		fimul rad
		fRndDn
		fistp y
		fimul rad
		fRndDn
		fistp x

    	mov eax,posX
    	add eax,x
    	mov edx,posY
    	add edx,y
     	OCall esi.SetPixel, eax, edx, color 
		
		;Increment angle
		fld step
		fadd angle
		fst angle
		
		;Check if we have passed "2PI" (a full circle)
		fldpi
		fldpi
		fadd
		fsubr
		fstpReg eax
		.ifBitSet eax,BIT31
			.break
		.endif
	.until 0
MethodEnd

endif