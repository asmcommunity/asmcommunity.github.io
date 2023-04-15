; =================================================================================================
; Title:   D3DX9MShader.inc
; Author:  G. Friedrich, Homer
; Version: 1.0.2
; Purpose: D3DX9 shader support for ObjAsm32.
; Notes:   Based on the work of EvilHomer2k.
;          Version 1.0.0, September 2004:
;            - First release.
;          Version 1.0.1, December 2005:
;            - Some equates corrected.
;		   Version 1.0.2, August 2008
;			 - Updated to August 2007 (lol)
; =================================================================================================

;---------------------------------------------------------------------------
; D3DXTX_VERSION:
; --------------
; Version token used to create a procedural texture filler in effects
; Used by D3DXFill[]TX functions
;---------------------------------------------------------------------------
;D3DXTX_VERSION(_Major,_Minor) (('T' << 24) | ('X' << 16) | ((_Major) << 8) | (_Minor))

;----------------------------------------------------------------------------
;//----------------------------------------------------------------------------
;// D3DXSHADER flags:
;// -----------------
;// D3DXSHADER_DEBUG
;//   Insert debug file/line/type/symbol information.
;//
;// D3DXSHADER_SKIPVALIDATION
;//   Do not validate the generated code against known capabilities and
;//   constraints.  This option is only recommended when compiling shaders
;//   you KNOW will work.  (ie. have compiled before without this option.)
;//   Shaders are always validated by D3D before they are set to the device.
;//
;// D3DXSHADER_SKIPOPTIMIZATION 
;//   Instructs the compiler to skip optimization steps during code generation.
;//   Unless you are trying to isolate a problem in your code using this option 
;//   is not recommended.
;//
;// D3DXSHADER_PACKMATRIX_ROWMAJOR
;//   Unless explicitly specified, matrices will be packed in row-major order
;//   on input and output from the shader.
;//
;// D3DXSHADER_PACKMATRIX_COLUMNMAJOR
;//   Unless explicitly specified, matrices will be packed in column-major 
;//   order on input and output from the shader.  This is generally more 
;//   efficient, since it allows vector-matrix multiplication to be performed
;//   using a series of dot-products.
;//
;// D3DXSHADER_PARTIALPRECISION
;//   Force all computations in resulting shader to occur at partial precision.
;//   This may result in faster evaluation of shaders on some hardware.
;//
;// D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT
;//   Force compiler to compile against the next highest available software
;//   target for vertex shaders.  This flag also turns optimizations off, 
;//   and debugging on.  
;//
;// D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT
;//   Force compiler to compile against the next highest available software
;//   target for pixel shaders.  This flag also turns optimizations off, 
;//   and debugging on.
;//
;// D3DXSHADER_NO_PRESHADER
;//   Disables Preshaders. Using this flag will cause the compiler to not 
;//   pull out static expression for evaluation on the host cpu
;//
;// D3DXSHADER_AVOID_FLOW_CONTROL
;//   Hint compiler to avoid flow-control constructs where possible.
;//
;// D3DXSHADER_PREFER_FLOW_CONTROL
;//   Hint compiler to prefer flow-control constructs where possible.
;//
;//----------------------------------------------------------------------------

;--------------------------------------------------------------------------
D3DXSHADER_OPTIMIZATION_LEVEL1		equ 0
D3DXSHADER_DEBUG                    equ 1
D3DXSHADER_SKIPVALIDATION           equ 2
D3DXSHADER_SKIPOPTIMIZATION         equ 4
D3DXSHADER_PACKMATRIX_ROWMAJOR      equ 8
D3DXSHADER_PACKMATRIX_COLUMNMAJOR   equ 16
D3DXSHADER_PARTIALPRECISION         equ 32
D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT  equ 64
D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT  equ 128
D3DXSHADER_NO_PRESHADER				equ 256
D3DXSHADER_AVOID_FLOW_CONTROL  		equ 512
D3DXSHADER_PREFER_FLOW_CONTROL		equ 1024
										;2048
D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY equ 4096
D3DXSHADER_IEEE_STRICTNESS 			equ 8192
D3DXSHADER_OPTIMIZATION_LEVEL0  	equ 16384
D3DXSHADER_OPTIMIZATION_LEVEL2		equ 49152
D3DXSHADER_OPTIMIZATION_LEVEL3		equ 32768
D3DXSHADER_USE_LEGACY_D3DX9_31_DLL  equ 65536
;----------------------------------------------------------------------------
; D3DXHANDLE:
; -----------
; Handle values used to efficiently reference shader and effect parameters.
; Strings can be used as handles.  However, handles are not always strings.
;----------------------------------------------------------------------------

D3DXHANDLE    typedef dword
LPD3DXHANDLE  typedef ptr D3DXHANDLE

;----------------------------------------------------------------------------
; D3DXMACRO:
; ----------
; Preprocessor macro definition.  The application pass in a NULL-terminated
; array of this structure to various D3DX APIs.  This enables the application
; to #define tokens at runtime, before the file is parsed.
;----------------------------------------------------------------------------
D3DXMACRO struct
  Name        dword ?
  Definition  dword ?
D3DXMACRO ends
LPD3DXMACRO typedef ptr D3DXMACRO

D3DXSEMANTIC struct
  Usage       dword ?
  UsageIndex  dword ?
D3DXSEMANTIC ends
LPD3DXSEMANTIC typedef ptr D3DXSEMANTIC

D3DXFRAGMENT_DESC struct
  Name        dword ?
  Target      dword ?
D3DXFRAGMENT_DESC ends
LPD3DXFRAGMENT_DESC typedef ptr D3DXFRAGMENT_DESC

D3DXRS_BOOL           equ 1
D3DXRS_INT4           equ 2
D3DXRS_FLOAT4         equ 3
D3DXRS_SAMPLER        equ 4

D3DXPC_SCALAR         equ 1
D3DXPC_VECTOR         equ 2
D3DXPC_MATRIX_ROWS    equ 3
D3DXPC_MATRIX_COLUMNS equ 4
D3DXPC_OBJECT         equ 5
D3DXPC_STRUCT         equ 6

D3DXPT_VOID           equ 1
D3DXPT_BOOL           equ 2
D3DXPT_INT            equ 3
D3DXPT_FLOAT          equ 4
D3DXPT_STRING         equ 5
D3DXPT_TEXTURE        equ 6
D3DXPT_TEXTURE1D      equ 7
D3DXPT_TEXTURE2D      equ 8
D3DXPT_TEXTURE3D      equ 9
D3DXPT_TEXTURECUBE    equ 10
D3DXPT_SAMPLER        equ 11
D3DXPT_SAMPLER1D      equ 12
D3DXPT_SAMPLER2D      equ 13
D3DXPT_SAMPLER3D      equ 13
D3DXPT_SAMPLERCUBE    equ 15
D3DXPT_PIXELSHADER    equ 16
D3DXPT_VERTEXSHADER   equ 17
D3DXPT_PIXELFRAGMENT  equ 18
D3DXPT_VERTEXFRAGMENT equ 19
D3DXPT_UNSUPPORTED	  equ 20

D3DXCONSTANTTABLE_DESC struct
  Creator   dword ?                 ;Creator string
  Version   dword ?                 ;Shader version
  Constants dword ?                 ;Number of constants
D3DXCONSTANTTABLE_DESC ends
LPD3DXCONSTANTTABLE_DESC typedef ptr D3DXCONSTANTTABLE_DESC

D3DXCONSTANT_DESC struct
  pName         Pointer ?           ;Constant name
  RegisterSet   dword   ?           ;Register set=D3DXREGISTER_SET member
  RegisterIndex dword   ?           ;Register index
  RegisterCount dword   ?           ;Number of registers occupied
                        
  Class         dword   ?           ;Class=D3DXPARAMETER_CLASS member
  dType         dword   ?           ;Component type=D3DXPARAMETER_TYPE member
                        
  Rows          dword   ?           ;Number of rows
  Columns       dword   ?           ;Number of columns
  Elements      dword   ?           ;Number of array elements
  StructMembers dword   ?           ;Number of structure member sub-parameters
                        
  Bytes         dword   ?           ;Data size, in bytes
  DefaultValue  dword   ?           ;Pointer to default value
D3DXCONSTANT_DESC ends
LPD3DXCONSTANT_DESC typedef ptr D3DXCONSTANT_DESC


BEGIN_INTERFACE ID3DXConstantTable, ID3DXBuffer, <AB3C758F-093E-4356-B762-4DB18F1B3A01>
  STDMETHOD GetDesc,                  Pointer
  STDMETHOD GetConstantDesc,          Handle, Pointer, Pointer
  STDMETHOD GetSamplerIndex,		  Handle
; Handle operations
  STDMETHOD GetConstant,              Handle, dword
  STDMETHOD GetConstantByName,        Handle, Pointer
  STDMETHOD GetConstantElement,       Handle, dword
; Set Constants
  STDMETHOD SetDefaults,              Pointer
  STDMETHOD SetValue,                 Pointer, Handle, dword
  STDMETHOD SetBool,                  Pointer, Handle, dword
  STDMETHOD SetBoolArray,             Pointer, Handle, Pointer, dword
  STDMETHOD SetInt,                   Pointer, Handle, dword
  STDMETHOD SetIntArray,              Pointer, Handle, Pointer, dword
  STDMETHOD SetFloat,                 Pointer, Handle, FLOAT
  STDMETHOD SetFloatArray,            Pointer, Handle, Pointer, dword
  STDMETHOD SetVector,                Pointer, Handle, Pointer
  STDMETHOD SetVectorArray,           Pointer, Handle, Pointer, dword
  STDMETHOD SetMatrix,                Pointer, Handle, Pointer
  STDMETHOD SetMatrixArray,           Pointer, Handle, Pointer, dword
  STDMETHOD SetMatrixPointerArray,    Pointer, Handle, Pointer, dword
  STDMETHOD SetMatrixTranspose,       Pointer, Handle, Pointer
  STDMETHOD SetMatrixTransposeArray,  Pointer, Handle, Pointer, dword
  STDMETHOD SetMatrixTransposePointerArray, Pointer, Handle, Pointer, dword
END_INTERFACE

BEGIN_INTERFACE ID3DXTextureShader, IUnknown, <3E3D67F8-AA7A-405d-A857-BA01D4758426>
  STDMETHOD GetFunction,Pointer
  STDMETHOD GetConstantBuffer,Pointer
  
  STDMETHOD GetDesc,Pointer
  STDMETHOD GetConstantDesc,HANDLE,Pointer,Pointer
  STDMETHOD GetConstant,HANDLE,dword
  STDMETHOD GetConstantByName,HANDLE,Pointer
  STDMETHOD GetConstantElement,HANDLE,dword
  STDMETHOD SetDefault
  STDMETHOD SetValue,HANDLE,Pointer,dword
  STDMETHOD SetBool,HANDLE,BOOL
  STDMETHOD SetBoolArray,HANDLE,Pointer,dword
  STDMETHOD SetInt,HANDLE,dword
  STDMETHOD SetIntArray,HANDLE,Pointer,dword
  STDMETHOD SetFloat,HANDLE,real4
  STDMETHOD SetFloatArray,HANDLE,Pointer,dword
  STDMETHOD SetVector,HANDLE,Pointer
  STDMETHOD SetVectorArray,HANDLE,Pointer,dword
  STDMETHOD SetMatrix,HANDLE,Pointer
  STDMETHOD SetMatrixArray,HANDLE,Pointer,dword
  STDMETHOD SetMatrixPointerArray,HANDLE,Pointer,dword
  STDMETHOD SetMatrixTranspose,HANDLE,Pointer
  STDMETHOD SetMatrixTransposeArray,HANDLE,Pointer,dword
  STDMETHOD SetMatrixTransposePointerArray,HANDLE,Pointer,dword
END_INTERFACE

BEGIN_INTERFACE ID3DXFragmentLinker, IUnknown, <1A2C0CC2-E5B6-4ebc-9E8D-390E057811B6>
  STDMETHOD GetDevice,                Pointer
  STDMETHOD GetNumberOfFragments
  STDMETHOD GetFragmentHandleByIndex, dword
  STDMETHOD GetFragmentHandleByName,  LPSTR
  STDMETHOD GetFragmentDesc,          HANDLE, Pointer
; Add the fragments in the buffer to the linker
  STDMETHOD AddFragments,             Pointer
; Create a buffer containing the fragments.  Suitable for saving to disk
  STDMETHOD GetAllFragments,          Pointer
  STDMETHOD GetFragment,              LPSTR, Pointer
  STDMETHOD LinkShader,               Pointer, dword, Pointer, dword, Pointer, Pointer
  STDMETHOD LinkVertexShader,         Pointer, dword, Pointer, dword, Pointer, Pointer
  STDMETHOD LinkPixelShader,          Pointer, dword, Pointer, dword, Pointer, Pointer
  STDMETHOD ClearCache
END_INTERFACE

;----------------------------------------------------------------------------
; D3DXINCLUDE_TYPE:
;----------------------------------------------------------------------------
D3DXINC_LOCAL       equ 0
D3DXINC_SYSTEM      equ 1
D3DXINC_FORCE_DWORD equ 07fffffffh

;----------------------------------------------------------------------------
; ID3DXInclude:
; -------------
; This interface is intended to be implemented by the application, and can
; be used by various D3DX APIs.  This enables application-specific handling
; of #include directives in source files.
;
; Open()
;    Opens an include file.  If successful, it should fill in ppData and
;    pBytes.  The data pointer returned must remain valid until Close is
;    subsequently called.
; Close()
;    Closes an include file.  If Open was successful, Close is guaranteed
;    to be called before the API using this interface returns.
;----------------------------------------------------------------------------
;LPD3DXINCLUDE typedef ptr ID3DXInclude
;
BEGIN_INTERFACE ID3DXInclude
  STDMETHOD Open, dword, LPSTR, Pointer, Pointer, Pointer
  STDMETHOD Close,Pointer
END_INTERFACE


;
; APIs
;

;//----------------------------------------------------------------------------
;// D3DXAssembleShader:
;// -------------------
;// Assembles a shader.
;//
;// Parameters:
;//  pSrcFile
;//      Source file name
;//  hSrcModule
;//      Module handle. if NULL, current module will be used
;//  pSrcResource
;//      Resource name in module
;//  pSrcData
;//      Pointer to source code
;//  SrcDataLen
;//      Size of source code, in bytes
;//  pDefines
;//      Optional NULL-terminated array of preprocessor macro definitions.
;//  pInclude
;//      Optional interface pointer to use for handling #include directives.
;//      If this parameter is NULL, #includes will be honored when assembling
;//      from file, and will error when assembling from resource or memory.
;//  Flags
;//      See D3DXSHADER_xxx flags
;//  ppShader
;//      Returns a buffer containing the created shader.  This buffer contains
;//      the assembled shader code, as well as any embedded debug info.
;//  ppErrorMsgs
;//      Returns a buffer containing a listing of errors and warnings that were
;//      encountered during assembly.  If you are running in a debugger,
;//      these are the same messages you will see in your debug output.
;//----------------------------------------------------------------------------
;----------------------------------------------------------------------------
D3DXAssembleShaderFromFileA     proto :Pointer,:Pointer,:Pointer,:dword,:Pointer,:Pointer
D3DXAssembleShaderFromFileW     proto :Pointer,:Pointer,:Pointer,:dword,:Pointer,:Pointer
D3DXAssembleShaderFromFile      equ <D3DXAssembleShaderFromFileA>
D3DXAssembleShaderFromResourceA proto :HANDLE,:Pointer,:Pointer,:Pointer,:dword,:Pointer,:Pointer
D3DXAssembleShaderFromResourceW proto :HANDLE,:Pointer,:Pointer,:Pointer,:dword,:Pointer,:Pointer
D3DXAssembleShaderFromResource  equ <D3DXAssembleShaderFromResourceA>
D3DXAssembleShader              proto :Pointer,:dword,:Pointer,:Pointer,:dword,:Pointer,:Pointer

;----------------------------------------------------------------------------
; D3DXCompileShader:
; ------------------
; Compiles a shader.
;
; Parameters:
;  pSrcFile
;      Source file name.
;  hSrcModule
;      Module handle. if NULL, current module will be used.
;  pSrcResource
;      Resource name in module.
;  pSrcData
;      Pointer to source code.
;  SrcDataLen
;      Size of source code, in bytes.
;  pDefines
;      Optional NULL-terminated array of preprocessor macro definitions.
;  pInclude
;      Optional interface pointer to use for handling #include directives.
;      If this parameter is NULL, #includes will be honored when compiling
;      from file, and will error when compiling from resource or memory.
;  pFunctionName
;      Name of the entrypoint function where execution should begin.
;  pProfile
;      Instruction set to be used when generating code.  Currently supported
;      profiles are "vs_1_1", "vs_2_0", "vs_2_a", "vs_2_sw", "ps_1_1",
;      "ps_1_2", "ps_1_3", "ps_1_4", "ps_2_0", "ps_2_a", "ps_2_sw", "tx_1_0"
;  Flags
;      See D3DXSHADER_xxx flags.
;  ppShader
;      Returns a buffer containing the created shader.  This buffer contains
;      the compiled shader code, as well as any embedded debug and symbol
;      table info.  (See D3DXGetShaderConstantTable)
;  ppErrorMsgs
;      Returns a buffer containing a listing of errors and warnings that were
;      encountered during the compile.  If you are running in a debugger,
;      these are the same messages you will see in your debug output.
;  ppConstantTable
;      Returns a ID3DXConstantTable object which can be used to set
;      shader constants to the device.  Alternatively, an application can
;      parse the D3DXSHADER_CONSTANTTABLE block embedded as a comment within
;      the shader.
;----------------------------------------------------------------------------
D3DXCompileShaderFromFileA      proto :Pointer,:Pointer,:Pointer,:Pointer,:Pointer,:dword,:Pointer,:Pointer,:Pointer
D3DXCompileShaderFromFileW      proto :Pointer,:Pointer,:Pointer,:Pointer,:Pointer,:dword,:Pointer,:Pointer,:Pointer
D3DXCompileShaderFromFile       equ <D3DXCompileShaderFromFileA>
D3DXCompileShaderFromResourceA  proto :HANDLE,:Pointer,:Pointer,:Pointer,:Pointer,:Pointer,:dword,:Pointer,:Pointer,:Pointer
D3DXCompileShaderFromResourceW  proto :HANDLE,:Pointer,:Pointer,:Pointer,:Pointer,:Pointer,:dword,:Pointer,:Pointer,:Pointer
D3DXCompileShaderFromResource   equ D3DXCompileShaderFromResourceA
D3DXCompileShader               proto :Pointer,:dword,:Pointer,:Pointer,:Pointer,:Pointer,:dword,:Pointer,:Pointer,:Pointer
D3DXDisassembleShader			proto :Pointer,:BOOL,:LPSTR,:Pointer

;----------------------------------------------------------------------------
; D3DXGetPixelShaderProfile/D3DXGetVertexShaderProfile:
; -----------------------------------------------------
; Returns the name of the HLSL profile best suited to a given device.
;
; Parameters:
;  pDevice
;      Pointer to the device in question
;----------------------------------------------------------------------------
D3DXGetPixelShaderProfile   proto :Pointer
D3DXGetVertexShaderProfile  proto :Pointer

;----------------------------------------------------------------------------
; D3DXFindShaderComment:
; ----------------------
; Searches through a shader for a particular comment, denoted by a FourCC in
; the first dword of the comment.  If the comment is not found, and no other
; error has occurred, S_FALSE is returned.
;
; Parameters:
;  pFunction
;      Pointer to the function dword stream
;  FourCC
;      FourCC used to identify the desired comment block.
;  ppData
;      Returns a pointer to the comment data (not including comment token
;      and FourCC).  Can be NULL.
;  pSizeInBytes
;      Returns the size of the comment data in bytes.  Can be NULL.
;----------------------------------------------------------------------------
D3DXFindShaderComment proto :Pointer,:dword,:Pointer,:Pointer

;----------------------------------------------------------------------------
; D3DXGetShaderVersion:
; -----------------------
; Returns the shader version of a given shader.  Returns zero if the shader
; function is NULL.
;
; Parameters:
;  pFunction
;      Pointer to the function dword stream
;----------------------------------------------------------------------------
D3DXGetShaderVersion proto :Pointer


;//----------------------------------------------------------------------------
;// D3DXFindShaderComment:
;// ----------------------
;// Searches through a shader for a particular comment, denoted by a FourCC in
;// the first DWORD of the comment.  If the comment is not found, and no other
;// error has occurred, S_FALSE is returned.
;//
;// Parameters:
;//  pFunction
;//      Pointer to the function DWORD stream
;//  FourCC
;//      FourCC used to identify the desired comment block.
;//  ppData
;//      Returns a pointer to the comment data (not including comment token
;//      and FourCC).  Can be NULL.
;//  pSizeInBytes
;//      Returns the size of the comment data in bytes.  Can be NULL.
;//----------------------------------------------------------------------------
D3DXFindShaderComment proto :Pointer,:dword,:Pointer,:Pointer

;//----------------------------------------------------------------------------
;// D3DXGetShaderSize:
;// ------------------
;// Returns the size of the shader byte-code, in bytes.
;//
;// Parameters:
;//  pFunction
;//      Pointer to the function DWORD stream
;//----------------------------------------------------------------------------
D3DXGetShaderSize proto :Pointer

;//----------------------------------------------------------------------------
;// D3DXGetShaderVersion:
;// -----------------------
;// Returns the shader version of a given shader.  Returns zero if the shader 
;// function is NULL.
;//
;// Parameters:
;//  pFunction
;//      Pointer to the function DWORD stream
;//----------------------------------------------------------------------------

D3DXGetShaderVersion proto :Pointer

;----------------------------------------------------------------------------
; D3DXGetShaderSemantics:
; -----------------------
; Gets semantics for all input elements referenced inside a given shader.
;
; Parameters:
;  pFunction
;      Pointer to the function dword stream
;  pSemantics
;      Pointer to an array of D3DXSEMANTIC structures.  The function will
;      fill this array with the semantics for each input element referenced
;      inside the shader.  This array is assumed to contain at least
;      MAXD3DDECLLENGTH elements.
;  pCount
;      Returns the number of elements referenced by the shader
;----------------------------------------------------------------------------
D3DXGetShaderInputSemantics   proto :Pointer,:Pointer,:Pointer
D3DXGetShaderOutputSemantics  proto :Pointer,:Pointer,:Pointer

;----------------------------------------------------------------------------
; D3DXGetShaderSamplers:
; ----------------------
; Gets semantics for all input elements referenced inside a given shader.
;
; pFunction
;      Pointer to the function dword stream
; pSamplers
;      Pointer to an array of LPCSTRs.  The function will fill this array
;      with pointers to the sampler names contained within pFunction, for
;      each sampler referenced inside the shader.  This array is assumed to
;      contain at least 16 elements.
; pCount
;      Returns the number of samplers referenced by the shader
;----------------------------------------------------------------------------
D3DXGetShaderSamplers proto :Pointer,:Pointer,:Pointer

;----------------------------------------------------------------------------
; D3DXGetShaderConstantTable:
; ---------------------------
; Gets shader constant table embedded inside shader.  A constant table is
; generated by D3DXAssembleShader and D3DXCompileShader, and is embedded in
; the body of the shader.
;
; Parameters:
;  pFunction
;      Pointer to the function dword stream
;  ppConstantTable
;      Returns a ID3DXConstantTable object which can be used to set
;      shader constants to the device.  Alternatively, an application can
;      parse the D3DXSHADER_CONSTANTTABLE block embedded as a comment within
;      the shader.
;----------------------------------------------------------------------------
D3DXGetShaderConstantTable proto :Pointer,:Pointer

;//----------------------------------------------------------------------------
;// D3DXCreateTextureShader:
;// ------------------------
;// Creates a texture shader object, given the compiled shader.
;//
;// Parameters
;//  pFunction
;//      Pointer to the function DWORD stream
;//  ppTextureShader
;//      Returns a ID3DXTextureShader object which can be used to procedurally 
;//      fill the contents of a texture using the D3DXFillTextureTX functions.
;//----------------------------------------------------------------------------
D3DXCreateTextureShader proto :Pointer,:Pointer

;----------------------------------------------------------------------------
; D3DXGatherFragments:
; -------------------
; Assembles shader fragments into a buffer to be passed to a fragment linker.
;   will generate shader fragments for all fragments in the file
;
; Parameters:
;  pSrcFile
;      Source file name
;  hSrcModule
;      Module handle. if NULL, current module will be used
;  pSrcResource
;      Resource name in module
;  pSrcData
;      Pointer to source code
;  SrcDataLen
;      Size of source code, in bytes
;  pDefines
;      Optional NULL-terminated array of preprocessor macro definitions.
;  pInclude
;      Optional interface pointer to use for handling #include directives.
;      If this parameter is NULL, #includes will be honored when assembling
;      from file, and will error when assembling from resource or memory.
;  Flags
;      See D3DXSHADER_xxx flags
;  ppShader
;      Returns a buffer containing the created shader fragments.  This buffer contains
;      the assembled shader code, as well as any embedded debug info.
;  ppErrorMsgs
;      Returns a buffer containing a listing of errors and warnings that were
;      encountered during assembly.  If you are running in a debugger,
;      these are the same messages you will see in your debug output.
;----------------------------------------------------------------------------
D3DXGatherFragmentsFromFileA      proto :Pointer,:Pointer,:Pointer,:dword,:Pointer,:Pointer
D3DXGatherFragmentsFromFileW      proto :Pointer,:Pointer,:Pointer,:dword,:Pointer,:Pointer
D3DXGatherFragmentsFromFile       equ D3DXGatherFragmentsFromFileA
D3DXGatherFragmentsFromResourceA  proto :HANDLE,:Pointer,:Pointer,:Pointer,:dword,:Pointer,:Pointer
D3DXGatherFragmentsFromResourceW  proto :HANDLE,:Pointer,:Pointer,:Pointer,:dword,:Pointer,:Pointer
D3DXGatherFragmentsFromResource   equ D3DXGatherFragmentsFromResourceA
D3DXGatherFragments               proto :Pointer,:dword,:Pointer,:Pointer,:dword,:Pointer,:Pointer
LPD3DXFRAGMENTLINKER typedef ptr ID3DXFragmentLinker

;----------------------------------------------------------------------------
; D3DXCreateFragmentLinker:
; -------------------------
; Creates a fragment linker with a given cache size.  The interface returned
; can be used to link together shader fragments.  (both HLSL & ASM fragements)
;
; Parameters:
;  pDevice
;      Pointer to the device on which to create the shaders
;  ShaderCacheSize
;      Size of the shader cache
;  ppFragmentLinker
;      pointer to a memory location to put the created interface pointer
;
;----------------------------------------------------------------------------
D3DXCreateFragmentLinker proto :Pointer,:dword,:Pointer

;//----------------------------------------------------------------------------
;// D3DXPreprocessShader:
;// ---------------------
;// Runs the preprocessor on the specified shader or effect, but does
;// not actually compile it.  This is useful for evaluating the #includes
;// and #defines in a shader and then emitting a reformatted token stream
;// for debugging purposes or for generating a self-contained shader.
;//
;// Parameters:
;//  pSrcFile
;//      Source file name
;//  hSrcModule
;//      Module handle. if NULL, current module will be used
;//  pSrcResource
;//      Resource name in module
;//  pSrcData
;//      Pointer to source code
;//  SrcDataLen
;//      Size of source code, in bytes
;//  pDefines
;//      Optional NULL-terminated array of preprocessor macro definitions.
;//  pInclude
;//      Optional interface pointer to use for handling #include directives.
;//      If this parameter is NULL, #includes will be honored when assembling
;//      from file, and will error when assembling from resource or memory.
;//  ppShaderText
;//      Returns a buffer containing a single large string that represents
;//      the resulting formatted token stream
;//  ppErrorMsgs
;//      Returns a buffer containing a listing of errors and warnings that were
;//      encountered during assembly.  If you are running in a debugger,
;//      these are the same messages you will see in your debug output.
;//----------------------------------------------------------------------------

D3DXPreprocessShaderFromFileA proto :Pointer,:Pointer,:Pointer,:Pointer,:Pointer
D3DXPreprocessShaderFromFileW proto :Pointer,:Pointer,:Pointer,:Pointer,:Pointer
D3DXPreprocessShaderFromFile equ  D3DXPreprocessShaderFromFileA                                     
D3DXPreprocessShaderFromResourceA proto :HANDLE,:Pointer,:Pointer,:Pointer,:Pointer,:Pointer
D3DXPreprocessShaderFromResourceW proto :HANDLE,:Pointer,:Pointer,:Pointer,:Pointer,:Pointer
D3DXPreprocessShaderFromResource equ D3DXPreprocessShaderFromResourceA
D3DXPreprocessShader proto :Pointer,:dword,:Pointer,:Pointer,:Pointer,:Pointer
 
;
; Shader comment block layouts
;

;----------------------------------------------------------------------------
; D3DXSHADER_CONSTANTTABLE:
; -------------------------
; Shader constant information; included as an CTAB comment block inside
; shaders.  All offsets are BYTE offsets from start of CONSTANTTABLE struct.
; Entries in the table are sorted by Name in ascending order.
;----------------------------------------------------------------------------
D3DXSHADER_CONSTANTTABLE struct
  dSize           dword   ?     ;sizeof(D3DXSHADER_CONSTANTTABLE)
  Creator         dword   ?     ;LPCSTR offset
  Version         dword   ?     ;Shader version
  Constants       dword   ?     ;Number of constants
  ConstantInfo    dword   ?     ;D3DXSHADER_CONSTANTINFO[Constants] offset
  Flags           dword   ?     ;Flags shader was compiled with
  pTarget         Pointer ?     ;LPCSTR offset
D3DXSHADER_CONSTANTTABLE ends
LPD3DXSHADER_CONSTANTTABLE typedef ptr D3DXSHADER_CONSTANTTABLE

D3DXSHADER_CONSTANTINFO struct
  pName           Pointer ?     ;LPCSTR offset
  RegisterSet     word    ?     ;D3DXREGISTER_SET
  RegisterIndex   word    ?     ;Register number
  RegisterCount   word    ?     ;Number of registers
  Reserved        word    ?     ;Reserved
  TypeInfo        dword   ?     ;D3DXSHADER_TYPEINFO offset
  DefaultValue    dword   ?     ;Offset of default value
D3DXSHADER_CONSTANTINFO ends
LPD3DXSHADER_CONSTANTINFO typedef ptr D3DXSHADER_CONSTANTINFO

D3DXSHADER_TYPEINFO struct
  Class           word    ?     ;D3DXPARAMETER_CLASS
  wType           word    ?     ;D3DXPARAMETER_TYPE
  Rows            word    ?     ;Number of rows (matrices)
  Columns         word    ?     ;Number of columns (vectors and matrices)
  Elements        word    ?     ;Array dimension
  StructMembers   word    ?     ;Number of struct members
  StructMemberInfo dword  ?     ;D3DXSHADER_STRUCTMEMBERINFO[Members] offset
D3DXSHADER_TYPEINFO ends
LPD3DXSHADER_TYPEINFO typedef ptr D3DXSHADER_TYPEINFO

D3DXSHADER_STRUCTMEMBERINFO struct
  pName           dword   ?     ;LPCSTR offset
  TypeInfo        dword   ?     ;D3DXSHADER_TYPEINFO offset
D3DXSHADER_STRUCTMEMBERINFO ends
LPD3DXSHADER_STRUCTMEMBERINFO typedef ptr D3DXSHADER_STRUCTMEMBERINFO
