; =================================================================================================
; Title:   D3DX9Core.inc
; Author:  G. Friedrich
; Version: 1.0.1
; Purpose: D3DX9 core support for ObjAsm32.
; Notes:   Based on the work of EvilHomer2k.
;          Version 1.0.0, September 2004:
;            - First release.
;          Version 1.0.1, December 2005:
;            - Dependancies removed.
; =================================================================================================

; D3DX_SDK_VERSION:
; -----------------
; This identifier is passed to D3DXCheckVersion in order to ensure that an
; application was built against the correct header files and lib files.
; This number is incremented whenever a header (or other) change would
; require applications to be rebuilt. If the version doesn't match,
; D3DXCreateVersion will return FALSE. (The number itself has no meaning.)

D3DX_VERSION        equ 0903h
D3D_SDK_VERSION     equ 32            ;DirectX 9.0 c, october 2004

Direct3DCreate9  proto :dword
D3DXCheckVersion proto :dword, :dword


; D3DXGetDriverLevel:
;    Returns driver version information:
;
;    700 - DX7 level driver
;    800 - DX8 level driver
;    900 - DX9 level driver

D3DXGetDriverLevel proto :Pointer

_FACD3D  equ 0876h
MAKE_D3DXHRESULT macro val:req
	LOCAL n
	n = MAKE_HRESULT(1, _FACD3D, val)
	exitm <n>
endm

; ID3DXBuffer:
; ------------
; The buffer object is used by D3DX to return arbitrary size data.
; GetBufferPointer - Returns a pointer to the beginning of the buffer.
; GetBufferSize - Returns the size of the buffer, in bytes.

BEGIN_INTERFACE ID3DXBuffer, IUnknown, <932E6A7E-C68E-45dd-A7BF-53D19C86DB1F>
	STDMETHOD	GetBufferPointer
	STDMETHOD	GetBufferSize
END_INTERFACE


; D3DXSPRITE flags:
; -----------------
; D3DXSPRITE_DONOTSAVESTATE
;   Specifies device state is not to be saved and restored in Begin/End.
; D3DXSPRITE_DONOTMODIFY_RENDERSTATE
;   Specifies device render state is not to be changed in Begin.  The device
;   is assumed to be in a valid state to draw vertices containing POSITION0,
;   TEXCOORD0, and COLOR0 data.
; D3DXSPRITE_OBJECTSPACE
;   The WORLD, VIEW, and PROJECTION transforms are NOT modified.  The
;   transforms currently set to the device are used to transform the sprites
;   when the batch is drawn (at Flush or End).  If this is not specified,
;   WORLD, VIEW, and PROJECTION transforms are modified so that sprites are
;   drawn in screenspace coordinates.
; D3DXSPRITE_BILLBOARD
;   Rotates each sprite about its center so that it is facing the viewer.
; D3DXSPRITE_ALPHABLEND
;   Enables ALPHABLEND(SRCALPHA, INVSRCALPHA) and ALPHATEST(alpha > 0).
;   ID3DXFont expects this to be set when drawing text.
; D3DXSPRITE_SORT_TEXTURE
;   Sprites are sorted by texture prior to drawing.  This is recommended when
;   drawing non-overlapping sprites of uniform depth.  For example, drawing
;   screen-aligned text with ID3DXFont.
; D3DXSPRITE_SORT_DEPTH_FRONTTOBACK
;   Sprites are sorted by depth front-to-back prior to drawing.  This is
;   recommended when drawing opaque sprites of varying depths.
; D3DXSPRITE_SORT_DEPTH_BACKTOFRONT
;   Sprites are sorted by depth back-to-front prior to drawing.  This is
;   recommended when drawing transparent sprites of varying depths.

D3DXSPRITE_DONOTSAVESTATE             equ 1
D3DXSPRITE_DONOTMODIFY_RENDERSTATE    equ 2
D3DXSPRITE_OBJECTSPACE                equ 4
D3DXSPRITE_BILLBOARD                  equ 8
D3DXSPRITE_ALPHABLEND                 equ 16
D3DXSPRITE_SORT_TEXTURE               equ 32
D3DXSPRITE_SORT_DEPTH_FRONTTOBACK     equ 64
D3DXSPRITE_SORT_DEPTH_BACKTOFRONT     equ 128


; ID3DXSprite:
; ------------
; This object intends to provide an easy way to drawing sprites using D3D.
;
; Begin -
;    Prepares device for drawing sprites.
;
; Draw -
;    Draws a sprite.  Before transformation, the sprite is the size of
;    SrcRect, with its top-left corner specified by Position.  The color
;    and alpha channels are modulated by Color.
;
; Flush -
;    Forces all batched sprites to submitted to the device.
;
; End -
;    Restores device state to how it was when Begin was called.
;
; OnLostDevice, OnResetDevice -
;    Call OnLostDevice() on this object before calling Reset() on the
;    device, so that this object can release any stateblocks and video
;    memory resources.  After Reset(), the call OnResetDevice().
;

LPD3DXSPRITE typedef ptr ID3DXSprite

BEGIN_INTERFACE ID3DXSprite, IUnknown, <D4715B38-6C44-472a-9024-6E2B0321CAC6>
  STDMETHOD   GetDevice,      Pointer
  STDMETHOD   GetTransform,   Pointer
  STDMETHOD   SetTransform,   Pointer
  STDMETHOD   SetWorldViewRH, Pointer, Pointer
  STDMETHOD   SetWorldViewLH, Pointer, Pointer
  STDMETHOD   Begin,          dword
  STDMETHOD   Draw,           Pointer, Pointer, Pointer, Pointer, dword
  STDMETHOD   Flush
  STDMETHOD   _End
  STDMETHOD   OnLostDevice
  STDMETHOD   OnResetDevice
END_INTERFACE

D3DXCreateSprite proto :dword,:dword


; ID3DXFont:
; ----------
; Font objects contain the textures and resources needed to render a specific
; font on a specific device.
;
; GetGlyphData -
;    Returns glyph cache data, for a given glyph.
;
; PreloadCharacters/PreloadGlyphs/PreloadText -
;    Preloads glyphs into the glyph cache textures.
;
; DrawText -
;    Draws formatted text on a D3D device.  Some parameters are
;    surprisingly similar to those of GDI's DrawText function.  See GDI
;    documentation for a detailed description of these parameters.
;    If pSprite is NULL, an internal sprite object will be used.
;
; OnLostDevice, OnResetDevice -
;    Call OnLostDevice() on this object before calling Reset() on the
;    device, so that this object can release any stateblocks and video
;    memory resources.  After Reset(), the call OnResetDevice().

D3DXFONT_DESCA struct
  dHeight         dword  ?
  dWidth          dword  ?
  Weight          dword  ?
  MipLevels       dword  ?
  Italic          dword  ?
  CharSet         byte   ?
  OutputPrecision byte   ?
  Quality         byte   ?
  PitchAndFamily  byte   ?
  FaceName byte  LF_FACESIZE dup (?)
D3DXFONT_DESCA ends
LPD3DXFONT_DESCA typedef ptr D3DXFONT_DESCA

D3DXFONT_DESCW struct
  dHeight         dword  ?
  dWidth          dword  ?
  Weight          dword  ?
  MipLevels       dword  ?
  Italic          dword  ?
  CharSet         byte   ?
  OutputPrecision byte   ?
  Quality         byte   ?
  PitchAndFamily  byte   ?
  FaceName        dw LF_FACESIZE dup (?)
D3DXFONT_DESCW ends
LPD3DXFONT_DESCW typedef ptr D3DXFONT_DESCW

D3DXFONT_DESC   typedef D3DXFONT_DESCA
LPD3DXFONT_DESC typedef LPD3DXFONT_DESCA

;** EDIT BY HOMER **
;This interface was extended at some point between April 2006 and August 2007
;Two methods were added just before 'GetDC'.

;BEGIN_INTERFACE ID3DXFont, IUnknown, <0B8D1536-9EEC-49b0-A5AD-93CF63AFB7C6>
;  STDMETHOD GetDevice,      Pointer
;  STDMETHOD GetDescA,       Pointer
;  STDMETHOD GetDescW,       Pointer
;  STDMETHOD GetDC
;  STDMETHOD GetGlyphData,   dword, Pointer, Pointer, Pointer
;  STDMETHOD PreloadCharacters,  dword, dword
;  STDMETHOD PreloadGlyphs,  dword, dword
;  STDMETHOD PreloadTextA,   Pointer, dword
;  STDMETHOD PreloadTextW,   Pointer, dword
;  STDMETHOD DrawTextA,      Pointer, Pointer, dword, Pointer, dword, dword
;  STDMETHOD DrawTextW,      Pointer, Pointer, dword, Pointer, dword, dword
;  STDMETHOD OnLostDevice
;  STDMETHOD OnResetDevice
;  STDMETHOD GetDesc,        Pointer             ;Return GetDescW(pDesc)
;  STDMETHOD PreloadText,    Pointer, dword      ;Return PreloadTextW(pString, Count)
;END_INTERFACE

BEGIN_INTERFACE ID3DXFont, IUnknown,<d79dbb70-5f21-4d36-bbc2-ff525c213cdc>
  STDMETHOD GetDevice,     		 Pointer
  STDMETHOD GetDescA,      		 Pointer
  STDMETHOD GetDescW,      		 Pointer
  STDMETHOD GetTextMetricsA,     Pointer
  STDMETHOD GetTextMetricsW,     Pointer
  STDMETHOD GetDC
  STDMETHOD GetGlyphData,   dword, Pointer, Pointer, Pointer
  STDMETHOD PreloadCharacters,  dword, dword
  STDMETHOD PreloadGlyphs,  dword, dword
  STDMETHOD PreloadTextA,   Pointer, dword
  STDMETHOD PreloadTextW,   Pointer, dword
  STDMETHOD DrawTextA,      Pointer, Pointer, dword, Pointer, dword, dword
  STDMETHOD DrawTextW,      Pointer, Pointer, dword, Pointer, dword, dword
  STDMETHOD OnLostDevice
  STDMETHOD OnResetDevice
  STDMETHOD GetDesc,        Pointer             ;Return GetDescW(pDesc)
  STDMETHOD PreloadText,    Pointer, dword      ;Return PreloadTextW(pString, Count)
END_INTERFACE

D3DXCreateFontA proto :dword,:dword,:dword,:dword,:dword,:dword,:dword,:dword,:dword,:dword,:dword,:dword
D3DXCreateFontW proto :dword,:dword,:dword,:dword,:dword,:dword,:dword,:dword,:dword,:dword,:dword,:dword
D3DXCreateFont  equ D3DXCreateFontA

D3DXCreateFontIndirectA proto :dword,:dword,:dword
D3DXCreateFontIndirectW proto :dword,:dword,:dword
D3DXCreateFontIndirect  equ D3DXCreateFontIndirectA


; ID3DXRenderToSurface:
; ---------------------
; This object abstracts rendering to surfaces.  These surfaces do not
; necessarily need to be render targets.  If they are not, a compatible
; render target is used, and the result copied into surface at end scene.
;
; BeginScene, EndScene -
;    Call BeginScene() and EndScene() at the beginning and ending of your
;    scene.  These calls will setup and restore render targets, viewports,
;    etc..
;
; OnLostDevice, OnResetDevice -
;    Call OnLostDevice() on this object before calling Reset() on the
;    device, so that this object can release any stateblocks and video
;    memory resources.  After Reset(), the call OnResetDevice().

D3DXRTS_DESC struct
    dWidth        dword ?
    dHeight       dword ?
    Format        dword ?
    DepthStencil  dword ?
    DepthStencilFormat dword ?
D3DXRTS_DESC ends

LPD3DXRTS_DESC typedef ptr D3DXRTS_DESC

BEGIN_INTERFACE ID3DXRenderToSurface, IUnknown, <0D014791-8863-4c2c-A1C0-02F3E0C0B653>
  STDMETHOD GetDevice,      Pointer
  STDMETHOD GetDesc,        Pointer
  STDMETHOD BeginScene,     Pointer, Pointer
  STDMETHOD EndScene,       Pointer
  STDMETHOD OnLostDevice
  STDMETHOD OnResetDevice
END_INTERFACE

D3DXCreateRenderToSurface proto :dword,:dword,:dword,:dword,:dword,:dword,:dword


; ID3DXRenderToEnvMap:
; --------------------
; This object abstracts rendering to environment maps.  These surfaces
; do not necessarily need to be render targets.  If they are not, a
; compatible render target is used, and the result copied into the
; environment map at end scene.
;
; BeginCube, BeginSphere, BeginHemisphere, BeginParabolic -
;    This function initiates the rendering of the environment map.  As
;    parameters, you pass the textures in which will get filled in with
;    the resulting environment map.
;
; Face -
;    Call this function to initiate the drawing of each face.  For each
;    environment map, you will call this six times.. once for each face
;    in D3DCUBEMAP_FACES.
;
; End -
;    This will restore all render targets, and if needed compose all the
;    rendered faces into the environment map surfaces.
;
; OnLostDevice, OnResetDevice -
;    Call OnLostDevice() on this object before calling Reset() on the
;    device, so that this object can release any stateblocks and video
;    memory resources.  After Reset(), the call OnResetDevice().

D3DXRTE_DESC struct
  _Size         dword ?
  MipLevels     dword ?
  Format        dword ?
  DepthStencil  dword ?
  DepthStencilFormat dword  ?
D3DXRTE_DESC ends

LPD3DXRTE_DESC        typedef ptr D3DXRTE_DESC
LPD3DXRenderToEnvMap  typedef ptr ID3DXRenderToEnvMap

BEGIN_INTERFACE ID3DXRenderToEnvMap, IUnknown, <1561135E-BC78-495b-8586-94EA537BD557>
  STDMETHOD GetDevice,        Pointer
  STDMETHOD GetDesc,          Pointer
  STDMETHOD BeginCube,        Pointer
  STDMETHOD BeginSphere,      Pointer
  STDMETHOD BeginHemisphere,  Pointer, Pointer
  STDMETHOD BeginParabolic,   Pointer, Pointer
  STDMETHOD Face,             dword, dword
  STDMETHOD _End,             dword
  STDMETHOD OnLostDevice
  STDMETHOD OnResetDevice
END_INTERFACE

D3DXCreateRenderToEnvMap proto :dword,:dword,:dword,:dword,:dword,:dword,:dword


; ID3DXLine:
; ------------
; This object intends to provide an easy way to draw lines using D3D.
;
; Begin -
;    Prepares device for drawing lines
;
; Draw -
;    Draws a line strip in screen-space.
;    Input is in the form of a array defining points on the line strip. of D3DXVECTOR2
;
; DrawTransform -
;    Draws a line in screen-space with a specified input transformation matrix.
;
; End -
;     Restores device state to how it was when Begin was called.
;
; SetPattern -
;     Applies a stipple pattern to the line.  Input is one 32-bit
;     dword which describes the stipple pattern. 1 is opaque, 0 is
;     transparent.
;
; SetPatternScale -
;     Stretches the stipple pattern in the u direction.  Input is one
;     floating-point value.  0.0f is no scaling, whereas 1.0f doubles
;     the length of the stipple pattern.
;
; SetWidth -
;     Specifies the thickness of the line in the v direction.  Input is
;     one floating-point value.
;
; SetAntialias -
;     Toggles line antialiasing.  Input is a BOOL.
;     TRUE  = Antialiasing on.
;     FALSE = Antialiasing off.
;
; SetGLLines -
;     Toggles non-antialiased OpenGL line emulation.  Input is a BOOL.
;     TRUE  = OpenGL line emulation on.
;     FALSE = OpenGL line emulation off.
;
; OpenGL line:     Regular line:
;   *\                *\
;   | \              /  \
;   |  \            *\   \
;   *\  \             \   \
;     \  \             \   \
;      \  *             \   *
;       \ |              \ /
;        \|               *
;         *
;
; OnLostDevice, OnResetDevice -
;    Call OnLostDevice() on this object before calling Reset() on the
;    device, so that this object can release any stateblocks and video
;    memory resources.  After Reset(), the call OnResetDevice().

LPD3DXLINE typedef ptr ID3DXLine

BEGIN_INTERFACE ID3DXLine, IUnknown, <72CE4D70-CC40-4143-A896-32E50AD2EF35>
  STDMETHOD GetDevice,        Pointer
  STDMETHOD Begin
  STDMETHOD Draw,             Pointer, dword, dword
  STDMETHOD DrawTransform,    Pointer, dword, Pointer, dword
  STDMETHOD SetPattern,       dword
  STDMETHOD GetPattern
  STDMETHOD SetPatternScale,  FLOAT
  STDMETHOD GetPatternScale
  STDMETHOD SetWidth,         FLOAT
  STDMETHOD GetWidth
  STDMETHOD SetAntialias,     dword
  STDMETHOD GetAntialias
  STDMETHOD SetGLLines,       dword
  STDMETHOD GetGLLines
  STDMETHOD _End
  STDMETHOD OnLostDevice
  STDMETHOD OnResetDevice
END_INTERFACE

D3DXCreateLine proto :dword,:dword