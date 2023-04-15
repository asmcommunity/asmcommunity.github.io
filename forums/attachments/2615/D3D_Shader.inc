externdef matIdentity:D3DXMATRIX

;This object represents a PixelShader or a VertexShader.
D3D_ShaderID equ 436633
Object D3D_Shader,D3D_ShaderID,Primer
	RedefineMethod	Init,Pointer,Pointer
	RedefineMethod	Done
	VirtualMethod	On
	VirtualMethod	Off
	VirtualMethod	from_File,LPSTR,LPSTR,LPSTR,BOOL
	VirtualMethod	SetMatrix, LPSTR, Pointer
	;
	DefineVariable	pDirect3D,Pointer,NULL
	DefineVariable	dType,dword,-1
	DefineVariable	IShader,Pointer,NULL
	DefineVariable	IConstants,Pointer,NULL	;See ID3DXConstantTable for more information
ObjectEnd

TYPE_VERTEX_SHADER	equ 0	;these are just used internally, dont worry
TYPE_PIXEL_SHADER	equ 1

Method D3D_Shader.Done,uses esi
	SetObject esi
	.if [esi].IShader!=0
		SAFE_RELEASE [esi].IConstants
		SAFE_RELEASE [esi].IShader
	.endif
MethodEnd

Method D3D_Shader.Init,uses esi,pOwner,pDirect3D
	SetObject esi
	m2m [esi].pDirect3D,pDirect3D,edx
	ACall esi.Init,pOwner
MethodEnd


;This method compiles a HLSL or ASM Shader sourcecode, 
;and then creates a shader from it.
;pszShaderFile = file containing shader sourcecode
;pszMainFunc = name of main function in shader
;pszShaderProfile = shader profile string .. examples: 'vs_1_1' , 'ps_1_1'
Method D3D_Shader.from_File, uses esi, pszShaderFile,pszMainFunc,pszShaderProfile, bFileIsHLSL:BOOL
LOCAL code:Pointer ;LPD3DXBUFFER
LOCAL pdata
	SetObject esi
	.if bFileIsHLSL==TRUE
		;HLSL is pretty cool, you can embed inline ASM in HLSL also
		;DbgText "Compiling HLSL shader"
		mov edx,pszShaderProfile
		.if byte ptr[edx+3]=="1"
			;Compile 1.x shader to use 2.0
			mov edx,D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY
		.else
			xor edx,edx
		.endif
		invoke D3DXCompileShaderFromFileA,pszShaderFile,NULL,NULL,pszMainFunc,pszShaderProfile,\
										edx,addr code, NULL,addr [esi].IConstants 
	.else
		;DbgText "Assembling ASM Shader"
		invoke D3DXAssembleShaderFromFileA,pszShaderFile,NULL,NULL,0, addr code,NULL 
	.endif

	.if FAILED(eax)
	    DbgWarning "Failed to compiled Shader - maybe bad code"
	    mov eax,FALSE
	.else
		;DbgWarning "Shader was compiled or assembled"
		mov pdata,$ICall (code::ID3DXBuffer.GetBufferPointer)
		mov edx,pszShaderProfile
		.if word ptr[edx]=="sv"
			;DbgText "Creating VertexShader"
			mov [esi].dType,TYPE_VERTEX_SHADER
			mov edx, [esi].pDirect3D
			ICall [edx].Direct3D.pID3DDevice::IDirect3DDevice9.CreateVertexShader,pdata,addr [esi].IShader
		.elseif word ptr[edx]=="sp"	
			;DbgText "Creating PixelShader"
			mov [esi].dType,TYPE_PIXEL_SHADER
			mov edx, [esi].pDirect3D
			ICall [edx].Direct3D.pID3DDevice::IDirect3DDevice9.CreatePixelShader,pdata,addr [esi].IShader
		.else
			DbgWarning "Error - unexpected Shader Profile string"
			ICall code::IUnknown.Release
			mov eax,FALSE
			ExitMethod
		.endif	
		.if FAILED(eax)
			DbgHex eax
			DbgWarning "Failed to create Shader interface"
		.endif
		
	    ICall code::IUnknown.Release
	    mov eax,TRUE
	.endif
MethodEnd

;This method 'Selects' this Shader in the Render device,
;so that all subsequent Drawing is affected by this Shader.
;It enables a VertexShader OR a PixelShader.
;They can be used together, or separately, but note that
;you can only have ONE of each active at any given time.
Method D3D_Shader.On,uses esi
	SetObject esi
	mov edx,[esi].pDirect3D
	mov eax,[esi].dType
	.if eax==TYPE_VERTEX_SHADER
		ICall [edx].Direct3D.pID3DDevice::IDirect3DDevice9.SetVertexShader,[esi].IShader
		.if FAILED(eax)
			DbgWarning "Failed to set VertexShader"
		.endif
	.elseif eax==TYPE_PIXEL_SHADER
		ICall [edx].Direct3D.pID3DDevice::IDirect3DDevice9.SetPixelShader,[esi].IShader
		.if FAILED(eax)
			DbgWarning "Failed to set PixelShader"
		.endif
	.else
		DbgWarning "Error - Shader not ready"
	.endif
MethodEnd

;This method disables BOTH Pixel and Vertex shaders,
;so that all subsequent Draw functions are handled by the FixedFunction Pipeline.
Method D3D_Shader.Off,uses esi
	SetObject esi
	mov edx,[esi].pDirect3D
	ICall [edx].Direct3D.pID3DDevice::IDirect3DDevice9.SetVertexShader,NULL
	mov edx,[esi].pDirect3D
	ICall [edx].Direct3D.pID3DDevice::IDirect3DDevice9.SetPixelShader,NULL
MethodEnd

;This method sets (by Name) a Matrix Constant defined within the Shader
;I think it should only be used on HLSL shaders, not ASM ones
Method D3D_Shader.SetMatrix,uses esi,pszName:LPSTR,pMatrix
	SetObject esi
	.if [esi].IConstants!=0 && [esi].pDirect3D!=0
		mov edx,[esi].pDirect3D
		ICall [esi].IConstants::ID3DXConstantTable.SetMatrix,[edx].Direct3D.pID3DDevice,pszName,pMatrix
	.endif
MethodEnd
