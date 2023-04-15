; made by Ultrano @ultranos.com


;=====<< Classes >>===\
EnterHostedClass

class IDirect3D9,,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual RegisterSoftwareDevice
	virtual GetAdapterCount
	virtual GetAdapterIdentifier
	virtual GetAdapterModeCount
	virtual EnumAdapterModes
	virtual GetAdapterDisplayMode
	virtual CheckDeviceType
	virtual CheckDeviceFormat
	virtual CheckDeviceMultiSampleType
	virtual CheckDepthStencilMatch
	virtual CheckDeviceFormatConversion
	virtual GetDeviceCaps
	virtual GetAdapterMonitor
	virtual CreateDevice
endclass
	
	
class IDirect3DDevice9,,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual TestCooperativeLevel
	virtual GetAvailableTextureMem
	virtual EvictManagedResources
	virtual GetDirect3D
	virtual GetDeviceCaps
	virtual GetDisplayMode
	virtual GetCreationParameters
	virtual SetCursorProperties
	virtual SetCursorPosition
	virtual ShowCursor
	virtual CreateAdditionalSwapChain
	virtual GetSwapChain
	virtual GetNumberOfSwapChains
	virtual Reset
	virtual Present
	virtual GetBackBuffer
	virtual GetRasterStatus
	virtual SetDialogBoxMode
	virtual SetGammaRamp
	virtual GetGammaRamp
	virtual CreateTexture
	virtual CreateVolumeTexture
	virtual CreateCubeTexture
	virtual CreateVertexBuffer
	virtual CreateIndexBuffer
	virtual CreateRenderTarget
	virtual CreateDepthStencilSurface
	virtual UpdateSurface
	virtual UpdateTexture
	virtual GetRenderTargetData
	virtual GetFrontBufferData
	virtual StretchRect
	virtual ColorFill
	virtual CreateOffscreenPlainSurface
	virtual SetRenderTarget
	virtual GetRenderTarget
	virtual SetDepthStencilSurface
	virtual GetDepthStencilSurface
	virtual BeginScene
	virtual EndScene
	virtual Clear
	virtual SetTransform
	virtual GetTransform
	virtual MultiplyTransform
	virtual SetViewport
	virtual GetViewport
	virtual SetMaterial
	virtual GetMaterial
	virtual SetLight
	virtual GetLight
	virtual LightEnable
	virtual GetLightEnable
	virtual SetClipPlane
	virtual GetClipPlane
	virtual SetRenderState
	virtual GetRenderState
	virtual CreateStateBlock
	virtual BeginStateBlock
	virtual EndStateBlock
	virtual SetClipStatus
	virtual GetClipStatus
	virtual GetTexture
	virtual SetTexture
	virtual GetTextureStageState
	virtual SetTextureStageState
	virtual GetSamplerState
	virtual SetSamplerState
	virtual ValidateDevice
	virtual SetPaletteEntries
	virtual GetPaletteEntries
	virtual SetCurrentTexturePalette
	virtual GetCurrentTexturePalette
	virtual SetScissorRect
	virtual GetScissorRect
	virtual SetSoftwareVertexProcessing
	virtual GetSoftwareVertexProcessing
	virtual SetNPatchMode
	virtual GetNPatchMode
	virtual DrawPrimitive
	virtual DrawIndexedPrimitive
	virtual DrawPrimitiveUP
	virtual DrawIndexedPrimitiveUP
	virtual ProcessVertices
	virtual CreateVertexDeclaration
	virtual SetVertexDeclaration
	virtual GetVertexDeclaration
	virtual SetFVF
	virtual GetFVF
	virtual CreateVertexShader
	virtual SetVertexShader
	virtual GetVertexShader
	virtual SetVertexShaderConstantF
	virtual GetVertexShaderConstantF
	virtual SetVertexShaderConstantI
	virtual GetVertexShaderConstantI
	virtual SetVertexShaderConstantB
	virtual GetVertexShaderConstantB
	virtual SetStreamSource
	virtual GetStreamSource
	virtual SetStreamSourceFreq
	virtual GetStreamSourceFreq
	virtual SetIndices
	virtual GetIndices
	virtual CreatePixelShader
	virtual SetPixelShader
	virtual GetPixelShader
	virtual SetPixelShaderConstantF
	virtual GetPixelShaderConstantF
	virtual SetPixelShaderConstantI
	virtual GetPixelShaderConstantI
	virtual SetPixelShaderConstantB
	virtual GetPixelShaderConstantB
	virtual DrawRectPatch
	virtual DrawTriPatch
	virtual DeletePatch
	virtual CreateQuery
endclass
	
	
class IDirect3DStateBlock9,,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual GetDevice
	virtual Capture
	virtual Apply
endclass
	
	
class IDirect3DSwapChain9,,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual Present
	virtual GetFrontBufferData
	virtual GetBackBuffer
	virtual GetRasterStatus
	virtual GetDisplayMode
	virtual GetDevice
	virtual GetPresentParameters
endclass
	
	
class IDirect3DResource9,,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual GetDevice
	virtual SetPrivateData
	virtual GetPrivateData
	virtual FreePrivateData
	virtual SetPriority
	virtual GetPriority
	virtual PreLoad
	virtual GetType
endclass
	
	
class IDirect3DVertexDeclaration9,,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual GetDevice
	virtual GetDeclaration
endclass
	
	
class IDirect3DVertexShader9,,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual GetDevice
	virtual GetFunction
endclass
	
	
class IDirect3DPixelShader9,,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual GetDevice
	virtual GetFunction
endclass
	
	
class IDirect3DBaseTexture9,IDirect3DResource9,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual GetDevice
	virtual SetPrivateData
	virtual GetPrivateData
	virtual FreePrivateData
	virtual SetPriority
	virtual GetPriority
	virtual PreLoad
	virtual GetType
	virtual SetLOD
	virtual GetLOD
	virtual GetLevelCount
	virtual SetAutoGenFilterType
	virtual GetAutoGenFilterType
	virtual GenerateMipSubLevels
endclass
	
	
class IDirect3DTexture9,IDirect3DBaseTexture9,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual GetDevice
	virtual SetPrivateData
	virtual GetPrivateData
	virtual FreePrivateData
	virtual SetPriority
	virtual GetPriority
	virtual PreLoad
	virtual GetType
	virtual SetLOD
	virtual GetLOD
	virtual GetLevelCount
	virtual SetAutoGenFilterType
	virtual GetAutoGenFilterType
	virtual GenerateMipSubLevels
	virtual GetLevelDesc
	virtual GetSurfaceLevel
	virtual LockRect
	virtual UnlockRect
	virtual AddDirtyRect
endclass
	
	
class IDirect3DVolumeTexture9,IDirect3DBaseTexture9,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual GetDevice
	virtual SetPrivateData
	virtual GetPrivateData
	virtual FreePrivateData
	virtual SetPriority
	virtual GetPriority
	virtual PreLoad
	virtual GetType
	virtual SetLOD
	virtual GetLOD
	virtual GetLevelCount
	virtual SetAutoGenFilterType
	virtual GetAutoGenFilterType
	virtual GenerateMipSubLevels
	virtual GetLevelDesc
	virtual GetVolumeLevel
	virtual LockBox
	virtual UnlockBox
	virtual AddDirtyBox
endclass
	
	
class IDirect3DCubeTexture9,IDirect3DBaseTexture9,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual GetDevice
	virtual SetPrivateData
	virtual GetPrivateData
	virtual FreePrivateData
	virtual SetPriority
	virtual GetPriority
	virtual PreLoad
	virtual GetType
	virtual SetLOD
	virtual GetLOD
	virtual GetLevelCount
	virtual SetAutoGenFilterType
	virtual GetAutoGenFilterType
	virtual GenerateMipSubLevels
	virtual GetLevelDesc
	virtual GetCubeMapSurface
	virtual LockRect
	virtual UnlockRect
	virtual AddDirtyRect
endclass
	
	
class IDirect3DVertexBuffer9,IDirect3DResource9,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual GetDevice
	virtual SetPrivateData
	virtual GetPrivateData
	virtual FreePrivateData
	virtual SetPriority
	virtual GetPriority
	virtual PreLoad
	virtual GetType
	virtual Lock
	virtual Unlock
	virtual GetDesc
endclass
	
	
class IDirect3DIndexBuffer9,IDirect3DResource9,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual GetDevice
	virtual SetPrivateData
	virtual GetPrivateData
	virtual FreePrivateData
	virtual SetPriority
	virtual GetPriority
	virtual PreLoad
	virtual GetType
	virtual Lock
	virtual Unlock
	virtual GetDesc
endclass
	
	
class IDirect3DSurface9,IDirect3DResource9,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual GetDevice
	virtual SetPrivateData
	virtual GetPrivateData
	virtual FreePrivateData
	virtual SetPriority
	virtual GetPriority
	virtual PreLoad
	virtual GetType
	virtual GetContainer
	virtual GetDesc
	virtual LockRect
	virtual UnlockRect
	virtual GetDC
	virtual ReleaseDC
endclass
	
	
class IDirect3DVolume9,,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual GetDevice
	virtual SetPrivateData
	virtual GetPrivateData
	virtual FreePrivateData
	virtual GetContainer
	virtual GetDesc
	virtual LockBox
	virtual UnlockBox
endclass
	
	
class IDirect3DQuery9,,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	virtual GetDevice
	virtual GetType
	virtual GetDataSize
	virtual Issue
	virtual GetData
endclass
	
LeaveHostedClass
;=======/


;=====<< D3DX Classes >>===\
EnterHostedClass
class ID3DXSprite,,COM compatible
	virtual QueryInterface
	virtual AddRef
	virtual Release
	
	virtual GetDevice
	virtual Begin
	virtual Draw
	virtual DrawTransform
	virtual End
	virtual OnLostDevice
	virtual OnResetDevice
endclass
LeaveHostedClass
;=======/


;=====[[ Definitions >>===\
D3D_SDK_VERSION equ 31

MAKEFOURCC macro ch0,ch1,ch2,ch3
	exitm <( ( ch0 )  or  ( ( ch1 )  shl  8 )  or  ( ( ch2 )  shl  16 )  or  ( ( ch3 )  shl  24 ) ) >
	endm

;--------------------[ D3DFORMAT ]-----------------------------[
D3DFORMAT typedef DWORD
D3DFMT_UNKNOWN = 0 
D3DFMT_R8G8B8 = 20 
D3DFMT_A8R8G8B8 = 21 
D3DFMT_X8R8G8B8 = 22 
D3DFMT_R5G6B5 = 23 
D3DFMT_X1R5G5B5 = 24 
D3DFMT_A1R5G5B5 = 25 
D3DFMT_A4R4G4B4 = 26 
D3DFMT_R3G3B2 = 27 
D3DFMT_A8 = 28 
D3DFMT_A8R3G3B2 = 29 
D3DFMT_X4R4G4B4 = 30 
D3DFMT_A2B10G10R10 = 31 
D3DFMT_A8B8G8R8 = 32 
D3DFMT_X8B8G8R8 = 33 
D3DFMT_G16R16 = 34 
D3DFMT_A2R10G10B10 = 35 
D3DFMT_A16B16G16R16 = 36 
D3DFMT_A8P8 = 40 
D3DFMT_P8 = 41 
D3DFMT_L8 = 50 
D3DFMT_A8L8 = 51 
D3DFMT_A4L4 = 52 
D3DFMT_V8U8 = 60 
D3DFMT_L6V5U5 = 61 
D3DFMT_X8L8V8U8 = 62 
D3DFMT_Q8W8V8U8 = 63 
D3DFMT_V16U16 = 64 
D3DFMT_A2W10V10U10 = 67 
D3DFMT_UYVY = MAKEFOURCC( 'U' , 'Y' , 'V' , 'Y')
D3DFMT_R8G8_B8G8 = MAKEFOURCC( 'R' , 'G' , 'B' , 'G')
D3DFMT_YUY2 = MAKEFOURCC( 'Y' , 'U' , 'Y' , '2')
D3DFMT_G8R8_G8B8 = MAKEFOURCC( 'G' , 'R' , 'G' , 'B')
D3DFMT_DXT1 = MAKEFOURCC( 'D' , 'X' , 'T' , '1')
D3DFMT_DXT2 = MAKEFOURCC( 'D' , 'X' , 'T' , '2')
D3DFMT_DXT3 = MAKEFOURCC( 'D' , 'X' , 'T' , '3')
D3DFMT_DXT4 = MAKEFOURCC( 'D' , 'X' , 'T' , '4')
D3DFMT_DXT5 = MAKEFOURCC( 'D' , 'X' , 'T' , '5')
D3DFMT_D16_LOCKABLE = 70 
D3DFMT_D32 = 71 
D3DFMT_D15S1 = 73 
D3DFMT_D24S8 = 75 
D3DFMT_D24X8 = 77 
D3DFMT_D24X4S4 = 79 
D3DFMT_D16 = 80 
D3DFMT_D32F_LOCKABLE = 82 
D3DFMT_D24FS8 = 83 
D3DFMT_L16 = 81 
D3DFMT_VERTEXDATA = 100 
D3DFMT_INDEX16 = 101 
D3DFMT_INDEX32 = 102 
D3DFMT_Q16W16V16U16 = 110 
D3DFMT_MULTI2_ARGB8 = MAKEFOURCC( 'M' , 'E' , 'T' , '1')
D3DFMT_R16F = 111 
D3DFMT_G16R16F = 112 
D3DFMT_A16B16G16R16F = 113
D3DFMT_R32F = 114
D3DFMT_G32R32F = 115 
D3DFMT_A32B32G32R32F = 116 
D3DFMT_CxV8U8 = 117
;--------------------------------------------------------------/

;----------------------[ D3DRENDERSTATETYPE ]---------------------------------[
D3DRS_ZENABLE			EQU		7t
D3DRS_FILLMODE			EQU		8t
D3DRS_SHADEMODE			EQU		9t
D3DRS_ZWRITEENABLE		EQU		14t
D3DRS_ALPHATESTENABLE		EQU		15t
D3DRS_LASTPIXEL			EQU		16t
D3DRS_SRCBLEND			EQU		19t
D3DRS_DESTBLEND			EQU		20t
D3DRS_CULLMODE			EQU		22t
D3DRS_ZFUNC			EQU		23t
D3DRS_ALPHAREF			EQU		24t
D3DRS_ALPHAFUNC			EQU		25t
D3DRS_DITHERENABLE		EQU		26t
D3DRS_ALPHABLENDENABLE		EQU		27t
D3DRS_FOGENABLE			EQU		28t
D3DRS_SPECULARENABLE		EQU		29t
D3DRS_FOGCOLOR			EQU		34t
D3DRS_FOGTABLEMODE		EQU		35t
D3DRS_FOGSTART			EQU		36t
D3DRS_FOGEND			EQU		37t
D3DRS_FOGDENSITY		EQU		38t
D3DRS_RANGEFOGENABLE		EQU		48t
D3DRS_STENCILENABLE		EQU		52t
D3DRS_STENCILFAIL		EQU		53t
D3DRS_STENCILZFAIL		EQU		54t
D3DRS_STENCILPASS		EQU		55t
D3DRS_STENCILFUNC		EQU		56t
D3DRS_STENCILREF		EQU		57t
D3DRS_STENCILMASK		EQU		58t
D3DRS_STENCILWRITEMASK		EQU		59t
D3DRS_TEXTUREFACTOR		EQU		60t
D3DRS_WRAP0			EQU		128t
D3DRS_WRAP1			EQU		129t
D3DRS_WRAP2			EQU		130t
D3DRS_WRAP3			EQU		131t
D3DRS_WRAP4			EQU		132t
D3DRS_WRAP5			EQU		133t
D3DRS_WRAP6			EQU		134t
D3DRS_WRAP7			EQU		135t
D3DRS_CLIPPING			EQU		136t
D3DRS_LIGHTING			EQU		137t
D3DRS_AMBIENT			EQU		139t
D3DRS_FOGVERTEXMODE		EQU		140t
D3DRS_COLORVERTEX		EQU		141t
D3DRS_LOCALVIEWER		EQU		142t
D3DRS_NORMALIZENORMALS		EQU		143t
D3DRS_DIFFUSEMATERIALSOURCE	EQU		145t
D3DRS_SPECULARMATERIALSOURCE	EQU		146t
D3DRS_AMBIENTMATERIALSOURCE	EQU		147t
D3DRS_EMISSIVEMATERIALSOURCE	EQU		148t
D3DRS_VERTEXBLEND		EQU		151t
D3DRS_CLIPPLANEENABLE		EQU		152t
D3DRS_POINTSIZE			EQU		154t
D3DRS_POINTSIZE_MIN		EQU		155t
D3DRS_POINTSPRITEENABLE		EQU		156t
D3DRS_POINTSCALEENABLE		EQU		157t
D3DRS_POINTSCALE_A		EQU		158t
D3DRS_POINTSCALE_B		EQU		159t
D3DRS_POINTSCALE_C		EQU		160t
D3DRS_MULTISAMPLEANTIALIAS	EQU		161t
D3DRS_MULTISAMPLEMASK		EQU		162t
D3DRS_PATCHEDGESTYLE		EQU		163t
D3DRS_DEBUGMONITORTOKEN		EQU		165t
D3DRS_POINTSIZE_MAX		EQU		166t
D3DRS_INDEXEDVERTEXBLENDENABLE	EQU		167t
D3DRS_COLORWRITEENABLE		EQU		168t
D3DRS_TWEENFACTOR		EQU		170t
D3DRS_BLENDOP			EQU		171t
D3DRS_POSITIONDEGREE		EQU		172t
D3DRS_NORMALDEGREE		EQU		173t
D3DRS_SCISSORTESTENABLE		EQU		174t
D3DRS_SLOPESCALEDEPTHBIAS	EQU		175t
D3DRS_ANTIALIASEDLINEENABLE	EQU		176t
D3DRS_MINTESSELLATIONLEVEL	EQU		178t
D3DRS_MAXTESSELLATIONLEVEL	EQU		179t
D3DRS_ADAPTIVETESS_X		EQU		180t
D3DRS_ADAPTIVETESS_Y		EQU		181t
D3DRS_ADAPTIVETESS_Z		EQU		182t
D3DRS_ADAPTIVETESS_W		EQU		183t
D3DRS_ENABLEADAPTIVETESSELLATION EQU		184t
D3DRS_TWOSIDEDSTENCILMODE	EQU		185t
D3DRS_CCW_STENCILFAIL		EQU		186t
D3DRS_CCW_STENCILZFAIL		EQU		187t
D3DRS_CCW_STENCILPASS		EQU		188t
D3DRS_CCW_STENCILFUNC		EQU		189t
D3DRS_COLORWRITEENABLE1		EQU		190t
D3DRS_COLORWRITEENABLE2		EQU		191t
D3DRS_COLORWRITEENABLE3		EQU		192t
D3DRS_BLENDFACTOR		EQU		193t
D3DRS_SRGBWRITEENABLE		EQU		194t
D3DRS_DEPTHBIAS			EQU		195t
D3DRS_WRAP8			EQU		198t
D3DRS_WRAP9			EQU		199t
D3DRS_WRAP10			EQU		200t
D3DRS_WRAP11			EQU		201t
D3DRS_WRAP12			EQU		202t
D3DRS_WRAP13			EQU		203t
D3DRS_WRAP14			EQU		204t
D3DRS_WRAP15			EQU		205t
D3DRS_SEPARATEALPHABLENDENABLE	EQU		206t
D3DRS_SRCBLENDALPHA		EQU		207t
D3DRS_DESTBLENDALPHA		EQU		208t
D3DRS_BLENDOPALPHA		EQU		209t
;-----------------------------------------------------------------------------/


;------------------[ D3DBLEND ]---------------[
    D3DBLEND_ZERO               = 1
    D3DBLEND_ONE                = 2
    D3DBLEND_SRCCOLOR           = 3
    D3DBLEND_INVSRCCOLOR        = 4
    D3DBLEND_SRCALPHA           = 5
    D3DBLEND_INVSRCALPHA        = 6
    D3DBLEND_DESTALPHA          = 7
    D3DBLEND_INVDESTALPHA       = 8
    D3DBLEND_DESTCOLOR          = 9
    D3DBLEND_INVDESTCOLOR       = 10
    D3DBLEND_SRCALPHASAT        = 11
    D3DBLEND_BOTHSRCALPHA       = 12
    D3DBLEND_BOTHINVSRCALPHA    = 13
    D3DBLEND_BLENDFACTOR        = 14
    D3DBLEND_INVBLENDFACTOR     = 15
;---------------------------------------------/



D3DSWAPEFFECT_DISCARD	equ 1
D3DSWAPEFFECT_FLIP	equ 2
D3DSWAPEFFECT_COPY	equ 3


D3DADAPTER_DEFAULT	equ 0

D3DCREATE_FPU_PRESERVE			equ 2h
D3DCREATE_MULTITHREADED			equ 4h
D3DCREATE_PUREDEVICE			equ 10h
D3DCREATE_SOFTWARE_VERTEXPROCESSING	equ 20h
D3DCREATE_HARDWARE_VERTEXPROCESSING	equ 40h
D3DCREATE_MIXED_VERTEXPROCESSING	equ 80h
D3DCREATE_DISABLE_DRIVER_MANAGEMENT	equ 100h
D3DCREATE_ADAPTERGROUP_DEVICE		equ 200h


D3DDEVTYPE_HAL	equ 1
D3DDEVTYPE_REF	equ 2
D3DDEVTYPE_SW	equ 3

D3DCULL_NONE equ 1
D3DCULL_CW   equ 2
D3DCULL_CCW  equ 3


D3DCLEAR_TARGET  equ 1
D3DCLEAR_ZBUFFER equ 2
D3DCLEAR_STENCIL equ 4
;=======/


;=====[[ Procedures >>===\
Direct3DCreate9 proto Version:DWORD

D3DXCreateSprite proto pDevice:DWORD,ppSprite:DWORD
D3DXCreateTextureFromFile textequ <D3DXCreateTextureFromFileA>
D3DXCreateTextureFromFileA proto pDevice:DWORD,lpszFileName:DWORD,ppTexture:DWORD

;=======/


;=====<< Structures >>===\
D3DPRESENT_PARAMETERS struct
		BackBufferWidth		dd ?
		BackBufferHeight	dd ?
		BackBufferFormat	dd ?
		BackBufferCount		dd ?
		
		MultiSampleType		dd ?
		MultiSampleQuality	dd ?
		
		SwapEffect		dd ?
		hDeviceWindow		dd ?
		Windowed		dd ?
		EnableAutoDepthStencil	dd ?
		AutoDepthStencilFormat	dd ?
		dwFlags			dd ?
		
		FullScreen_RefreshRateInHz dd ?
		PresentationInterval	dd ?
D3DPRESENT_PARAMETERS ends

D3DSURFACE_DESC struct
	Format		dd ?
	dwType		dd ?
	Usage		dd ?
	Pool		dd ?
	MultiSampleType	dd ?
	MultiSampleQuality dd ?
	dwWidth		dd ?
	dwHeight	dd ?
D3DSURFACE_DESC ends

;=======/

