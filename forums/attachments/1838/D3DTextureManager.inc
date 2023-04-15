;============================================
;Texture and TextureManager objects
;
;The Texture object is a container for
;a loaded IDirect3DTexture9 object, and the
;name of the file from which it was loaded.
;
;The TextureManager object's job is to ensure
;that no texture is ever loaded more than once.
;It manages a Collection of shared Texture objects.
;=============================================

TextureID equ 25774
Object Texture,TextureID,Primer
	RedefineMethod	Done
	DefineVariable	pName,Pointer,NULL
	DefineVariable	pTexture,Pointer,NULL
ObjectEnd

Method Texture.Done,uses esi
	SetObject esi
	;Free namestring, if any
	.if [esi].pName!=0
		MemFree [esi].pName
	.endif
	;Release IDirect3DTexture9 object, if any
	.if [esi].pTexture!=0
		OCall [esi].pTexture::IUnknown.Release
	.endif	
MethodEnd

;=============================================================

TextureManagerID equ 25775
Object TextureManager,TextureManagerID,Primer
	RedefineMethod	Init,Pointer
	RedefineMethod	Done
	StaticMethod	FindTextureByName,Pointer
	StaticMethod	LoadFromFile,Pointer
	DefineVariable	pTextures,Pointer,NULL
ObjectEnd

Method TextureManager.Init,uses esi,pOwner
	SetObject esi
	mov [esi].pTextures,$New(Collection,Init,esi,16,256,-1)	
	ACall Init,pOwner
MethodEnd

Method TextureManager.Done,uses esi
	SetObject esi
	Destroy [esi].pTextures
MethodEnd

;Searches the Collection of Loaded Textures
;for a Texture whose name matches that given,
;returns ptr to IDirect3DTexture9 or NULL
Method TextureManager.FindTextureByName,uses esi,pfilename
LOCAL cnt,ptex
	SetObject esi
	mov eax,[esi].pTextures
	m2m cnt,[eax].Collection.dCount
	xor ecx,ecx
	.while ecx<cnt
		push ecx
		mov ptex,$OCall ([esi].pTextures::Collection.ItemAt,ecx)
		invoke lstrcmpi,pfilename,[eax].Texture.pName
		pop ecx
		.if eax==0
			return ptex
		.endif
		inc ecx
	.endw
	return NULL
MethodEnd

;This method checks whether the given imagefile has been loaded.
;If already loaded, or if subsequent loading succeeds,
;a pointer to IDirect3DTexture9 object is returned.
;If loading fails, NULL is returned.
Method TextureManager.LoadFromFile,uses esi,pfilename
LOCAL ptex,pTex
	SetObject esi
	;First thing we need to do is check whether
	;the given filename has already been Loaded..
	OCall FindTextureByName,pfilename
	.if eax==NULL
		;The filename is new, so load the texture from it..
		invoke D3DXCreateTextureFromFile, pD3DDevice, pfilename, ADDR ptex
		.if eax != S_OK
			DbgWarning "Failed to load Texture"
			DbgStr addr szTestTexture,"FAILED LOADING"
			return NULL
		.endif
		;Texture has loaded successfully.. 
		;Allocate memory for NameString
		invoke lstrlen,pfilename
		inc eax
		push $MemAlloc(eax,MEM_INIT_ZERO)
		;Copy filename into allocated memory
		invoke lstrcpy,eax,pfilename
		;create a Container for new texture
		mov pTex,$New(Texture)
		;Store the loaded IDirect3D9Texture object in container
		m2m [eax].Texture.pTexture,ptex
		;Store ptr to our namestring
		pop [eax].Texture.pName
		;Return the ID3D
		mov eax,ptex
	.endif
MethodEnd