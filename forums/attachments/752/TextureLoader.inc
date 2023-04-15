ifndef _TEXTURE_LOADER_H_
 _TEXTURE_LOADER_H_ equ 1
 
IncludeBoth oleaut32

; BEGIN_INTERFACE IPicture, IUnknown, <7BF80981-BF32-101A-8BBB-00AA00300CAB>
;  STDMETHOD   get_Handle,       Pointer
;  STDMETHOD   get_hPal,  		Pointer
;  STDMETHOD   get_Type,  		Pointer
;  STDMETHOD   get_Width,  		Pointer
;  STDMETHOD   get_Height,		Pointer
;  STDMETHOD	  Render, dword,dword,dword,dword,dword,dword,dword,dword,dword,Pointer
;  STDMETHOD   set_hPal, 		dword
;  STDMETHOD   get_CurDC,		Pointer
;  STDMETHOD	  SelectPicture,	dword,Pointer,Pointer
;  STDMETHOD	  get_KeepOriginalFormat, Pointer
;  STDMETHOD   put_KeepOriginalFormat, dword
;  STDMETHOD	  PictureChanged
;  STDMETHOD	  SaveAsFile,		Pointer,dword,Pointer
;  STDMETHOD	  get_Attributes,	Pointer
;END_INTERFACE

 
;/****************************************************************************
;*                                                                           *
;* Texture Loader                                                            *
;*                                                                           *
;* Originally Based on Jeff Molofee's IPicture Basecode                      *
;* Extensions By Chris Leathley (http://members.iinet.net.au/~cleathley/)    *
;* ObjAsm32 Conversion by Leith Ketchell aka Evil Homer 	                 *
;*****************************************************************************
;*                                                                           *
;*  Loads  : BMP, EMF, GIF, ICO, JPG, WMF and TGA                            *
;*  Source : Reads From Disk, Ram, Project Resource or the Internet          *
;*  Extras : Images Can Be Any Width Or Height                               *
;*           Low Quality Textures can be created                             *
;*           Different Filter Level Support (None, Bilinear and Trilinear    *
;*           Mipmapping Support                                              *
;*                                                                           *
;*****************************************************************************
;*                                                                           *
;*  Free To Use In Projects Of Your Own.  All I Ask For Is A Simple Greet    *
;*  Or Mention of my site in your readme or the project itself :)            *
;*                                                                           *
;*****************************************************************************
;*                                                                           *
;* Revision History                                                          *
;*                                                                           *
;* Version 1.0 Released                                                      *
;* Version 1.1 Added FreeTexture and LoadTextureFromResource                 *
;*			  Added TexType to the glTexture Struction                      *
;*             Optimisations to the Alpha Conversion Loop			        *
;* Version 1.2 Added Support PSP8 TGA files...                               *
;*             Single TGA file loader                                        *
;* Version 1.3 Added Support for low quality textures..                      *
;*             Added ScaleTGA function                                       *
;* Version 1.4 Added Support for gluScaleImage                               *
;*             Removed ScaleTGA (replacew with gluScaleImage)                *
;*             Added TextureFilter and MipMapping Support                    *
;*                                                                           *
;****************************************************************************/ 
 

;#include <olectl.h>			// Header File For The OLE Controls Library

; Define Interface Data Types / Structures
	txUnknown	equ 0
	txBmp		equ 1
	txJpg		equ 2
	txPng		equ 3
	txTga		equ 4
	txGif		equ 5
	txIco		equ 6
	txEmf		equ 7
	txWmf		equ 8

	txNoFilter	equ 0
	txBilinear	equ 1
	txTrilinear	equ 2
	
.data
	CStr szbmp,"bmp"
	CStr szgif,"gif"
	CStr szico,"ico"
	CStr szemf,"emf"
	CStr szwmf,"wmf"
	CStr szjpg,"jpg"
	CStr szjpeg,"jpeg"
.code			

glTexture struct
	TextureID 	dd ? 		; Texture ID Used To Select A Texture
	TexType		dd ?		; Texture Format
	dwWidth		dd ?		; Image Width
	dwHeight	dd ?		; Image Height
	dwType		dd ?		; Image Type (GL_RGB, GL_RGBA)
	Bpp			dd ?		; Image Color Depth In Bits Per Pixel
glTexture ends



; Define Private Structures
TGAHeader struct
	ImgIdent	db ?
	ignored		db ?
	ImgType		db ?
	ignored2	db 9 dup (?)
	WidthLo		db ?
	WidthHi		db ?
	HeightLo	db ?
	HeightHi	db ?
	Bpp			db ?
	ignored3	db ?
TGAHeader ends

$FirstLowerPowerOfTwo macro InputValue
	mov eax,InputValue
	bsr ecx,eax	;ecx <-- highest SET bit from eax
	mov eax,1
	shl eax,cl	;eax = 1 << ecx
	exitm <eax>
endm

;Define TextureLoader Class
Object TextureLoader,TextureLoaderID,Primer
	RedefineMethod Init
	RedefineMethod Done
	
	StaticMethod SetAlphaMatch,byte,byte,byte,byte ;(bEnabled,  RedAlphaMatch,  GreenAlphaMatch,  BlueAlphaMatch)
	StaticMethod SetHighQualityTextures,byte ;bEnabled
	StaticMethod SetMipMapping,byte 		 ;bEnabled
	StaticMethod SetTextureFilter,dword		 ;dwtype

	StaticMethod LoadTextureFromDisk,Pointer,Pointer
	StaticMethod LoadTextureFromRam,Pointer,dword,Pointer,dword   ;pData, Length, glTexture, TexType
	StaticMethod LoadTextureFromResource,dword,dword,Pointer,dword;pResourceName, pResourceType, pglTexture, TexType
	StaticMethod FreeTexture,Pointer						 	  ;pglTexture

	PrivateMethod BuildTexture,Pointer,Pointer					  ;pszPathName, pglTexture
	PrivateMethod LoadTGAFromDisk,Pointer,Pointer			 	  ;pszFileName, pglTexture

	PrivateMethod LoadJPG_GIFResource,Pointer,Pointer,Pointer	  ;pResourceName, pResourceType, pglTexture
	PrivateMethod LoadTGAResource,Pointer,Pointer,Pointer	 	  ;pResourceName, pResourceType, pglTexture

	PrivateMethod GenerateTexture,Pointer,Pointer			 	  ;pglTexture, pImgData
	PrivateMethod ExtensionFromFilename,Pointer,Pointer		 	  ;pszFileName, pszExtension

	;Booleans
	DefineVariable m_fAlphaConversion,byte,0
	DefineVariable m_fHighQualityTextures,byte,0
	DefineVariable m_fMipMapping,byte,0
	DefineVariable m_TextureFilterType,dword,0
	;Bytes
	DefineVariable	m_RedAlphaMatch,byte,0
	DefineVariable	m_GreenAlphaMatch,byte,0
	DefineVariable 	m_BlueAlphaMatch,byte,0
ObjectEnd

if IMPLEMENT
Method TextureLoader.Init
	OCall SetAlphaMatch,TRUE, 0, 0, 0 ; default to alpha matching BLACK
	OCall SetHighQualityTextures,TRUE ; default to full sized textures
	OCall SetMipMapping,FALSE 		  ; no mipmap textures
	OCall SetTextureFilter,txNoFilter ; no texture filtering
MethodEnd

; Set Alpha Matching State and Match Colour
Method TextureLoader.SetAlphaMatch,uses esi,fEnabled:byte, RedAlphaMatch:byte, GreenAlphaMatch:byte, BlueAlphaMatch:byte
	SetObject esi
	b2b [esi].m_fAlphaConversion, fEnabled
	; only set the colour match if the conversion is enabled
	.if fEnabled == TRUE
		b2b [esi].m_RedAlphaMatch, 	 RedAlphaMatch
		b2b [esi].m_GreenAlphaMatch, GreenAlphaMatch
		b2b [esi].m_BlueAlphaMatch,  BlueAlphaMatch
	.endif
MethodEnd


; set the high quality texture flag
Method TextureLoader.SetHighQualityTextures,uses esi, bEnabled:byte
	SetObject esi
	b2b [esi].m_fHighQualityTextures , bEnabled
MethodEnd

; set the mipmapping flag
Method TextureLoader.SetMipMapping,uses esi, bEnabled:byte
	SetObject esi
	b2b [esi].m_fMipMapping, bEnabled
MethodEnd

; set the texture filtering flag
Method TextureLoader.SetTextureFilter,uses esi, dwtype
	SetObject esi
	m2m [esi].m_TextureFilterType , dwtype
MethodEnd

; Load A Texture from Disk (based on the current location of the executable)
Method TextureLoader.LoadTextureFromDisk,uses esi,pszFileName, pglTexture
local szFullPath[256]:BYTE ; Full Path To Picture
local szExtension[16]:BYTE ; Extenstion of Picture
	;Check if the pathname begins with HTTP://
	CStr szhttp,"http"
	mov esi,pszFileName
	m2m dword ptr [szFullPath,dword ptr[esi]
	mov szFullPath[4],0
	invoke lstrcmpi,addr szFullPath,addr szhttp
	;If the FileName is a URL... We Are Loading From The Net
	.if eax==0 && word ptr[esi+4]=="/:" && byte ptr[esi+6]=="/"
		invoke lstrcpy,addr szFullPath, pszFileName
	;Otherwise... We Are Loading From A File
	.else ;Format the local file pathname
		CStr szFileSlash,"\"
		invoke GetCurrentDirectory,255, addr szFullPath
		invoke lstrcat,addr szFullPath, addr szFileSlash
		invoke lstrcat,addr szFullPath, pszFileName
	.endif

	OCall ExtensionFromFilename,pszFileName, addr szExtension

	; if the file is a TGA then use the TGA file loader
	CStr sztga,"tga"
	.if ($invoke (lstrcmpi,addr szExtension, addr sztga)) == 0
		OCall LoadTGAFromDisk,addr szFullPath, pglTexture
		ret	
	.else ; Load BMP, JPG, GIF
		mov esi,pglTexture
		mov [esi].glTexture.TexType, txUnknown
		
		.if ($invoke (lstrcmpi,addr szExtension,addr szbmp)) == 0
			mov esi,pglTexture	
			mov [esi].glTexture.TexType, txtxBmp
		
		.elseif ($invoke (lstrcmpi,addr szExtension, addr szgif)) == 0
			mov esi,pglTexture	
			mov [esi].glTexture.TexType, txGif

		.elseif ($invoke (lstrcmpi,addr szExtension, addr szico)) == 0
			mov esi,pglTexture	
			mov [esi].glTexture.TexType, txIco

		.elseif ($invoke (lstrcmpi,addr szExtension, addr szemf)) == 0
			mov esi,pglTexture	
			mov [esi].glTexture.TexType, txEmf

		.elseif ($invoke (lstrcmpi,addr szExtension, addr szwmf)) == 0
			mov esi,pglTexture	
			mov [esi].glTexture.TexType, txWmf

		
		.else
			invoke lstrcmpi,addr szExtension, addr szjpg
			.if  eax == 0 || ($invoke (lstrcmpi,addr szExtension, addr szjpeg)) == 0
				mov esi,pglTexture	
				mov [esi].glTexture.TexType, txJpg
			.else
				DbgStr pszFileName, "UNKNOWN FILE EXTENSION"
				return NULL
			.endif
		.endif		
		; Load BMP, JPG, GIF etc..
		OCall BuildTexture,addr szFullPath, pglTexture
	.endif
MethodEnd

; Load A Texture from RAM
Method TextureLoader.LoadTextureFromRam,,pData, dwLength, pglTexture, TexType
	; it would be much easier to allocate some global memory and make a stream out of it but that
	; would require changing the TGA loading code to use RAM and not Disk access commands

local szTempFileName[256]:BYTE
local hFile
local rc
local BytesWritten
	CStr sztempfile,"glTEMP$$.tmp"
	invoke GetCurrentDirectory,255, addr szTempFileName
	invoke lstrcat,addr szTempFileName, addr sztempfile
	invoke CreateFile,addr szTempFileName,GENERIC_WRITE,0,NULL,
			 CREATE_ALWAYS,FILE_ATTRIBUTE_TEMPORARY,NULL
	mov hFile,eax
	.if eax != INVALID_HANDLE_VALUE
		; Write The Data To disk (To A Temp File)
		invoke WriteFile,hFile, pData, dwLength, addr BytesWritten, NULL					
		invoke CloseHandle,hFile
		mov eax,pglTexture
		m2m [eax].glTexture.TexType,TexType

		;if the file is a TGA then use the TGA file loader
		.if TexType == txTga			
			mov rc ,$OCall (LoadTGAFromDisk,addr szTempFileName, pglTexture)		
		.else  ; Load BMP, JPG, GIF
			mov rc ,$OCall (BuildTexture,addr szTempFileName, pglTexture)
		.endif
		invoke DeleteFile,addr szTempFileName
		mov eax,rc
	.else
		mov eax,FALSE
	.endif
MethodEnd


; Load A Texture from the project Resource
Method TextureLoader.LoadTextureFromResource,,dwResourceName, pResourceType, pglTexture, TexType
	; set the texture type
	mov eax,pglTexture
	m2m [eax].glTexture.TexType , TexType	
	.if TexType == txJpg || TexType == txGif
		OCall LoadJPG_GIFResource,dwResourceName, pResourceType, pglTexture
	.elseif TexType == txBmp
		; some bug with the bitmap resource loader..
		OCall LoadBMPResource,dwResourceName, pResourceType, pglTexture
	.elseif TexType == txTga
		OCall LoadTGAResource,dwResourceName, pResourceType, pglTexture
	.else
		mov eax,FALSE
	.endif
MethodEnd

;Free a Texture from openGL
Method TextureLoader.FreeTexture,,pglTexture
	mov eax,pglTexture
	invoke glDeleteTextures,1,addr [eax].glTexture.TextureID 
MethodEnd

;Load BMP, GIF or JPG and Convert To A Texture
Method TextureLoader.BuildTexture,,pszPathName, pglTexture
local hdcTemp,hbmpTemp,pPicture
local wszPath[256]:WORD
local lWidth,lHeight
local lWidthPixels,lHeightPixels
local glMaxTexDim,pBits
local bi:BITMAPINFO

	;Convert From ASCII To Unicode
	invoke MultiByteToWideChar,CP_ACP, 0, pszPathName, -1, addr wszPath, 256
	invoke OleLoadPicturePath,addr wszPath, 			;Path
									NULL,				;punkCaller
									0,					;Reserved
									0,					;Reserved Transparent Colour
									addr IID_IPicture,	;riid of Inertface to return
									addr pPicture	    ;pointer to hold returned pInterface

	.if eax==FAILED
		return FALSE
	.endif

	mov hdcTemp ,$invoke (CreateCompatibleDC,($invoke (GetDC,0)))								// Create The Windows Compatible Device Context
	.if !eax	; Did Creation Fail?	
		OCall pPicture::IUnknown.Release	; Decrements IPicture Reference Count
		return FALSE
	.endif

	;Get Maximum Texture Size Supported
	invoke glGetIntegerv,GL_MAX_TEXTURE_SIZE, addr glMaxTexDim
	
	; Get IPicture Width (Convert To Pixels)
	OCall pPicture::IPicture.get_Width,addr lWidth	
	mov lWidthPixels, $invoke(MulDiv,lWidth, ($invoke(GetDeviceCaps,hdcTemp, LOGPIXELSX)), 2540)

	; Get IPicture Height (Convert To Pixels)
	OCall pPicture::IPicture.get_Height,addr lHeight
	mov lHeightPixels,$invoke(MulDiv,lHeight,($invoke(GetDeviceCaps,hdcTemp, LOGPIXELSY)), 2540)

	; Is Image Width Less Than Cards Limit?
	.if lWidthPixels < glMaxTexDim
		; Resize Image To Closest Power Of Two
		mov lWidthPixels,$FirstLowerPowerOfTwo(lWidthPixels)
	.else  ; Otherwise, Set Width To "Max Power Of Two" That The Card Can Handle
		m2m lWidthPixels , glMaxTexDim
	.endif

 	; Is Image Height Less Than Than Cards Limit
	.if lHeightPixels < glMaxTexDim
		mov lHeightPixels,$FirstLowerPowerOfTwo(lHeightPixels)
	.else  ; Otherwise  Set Height To "Max Power Of Two" That The Card Can Handle
		m2m lHeightPixels , glMaxTexDim
	.endif

	; if low quality textures then make them halve the size which saved 4 times the texture space
	SetObject esi
	.if [esi].m_fHighQualityTextures == FALSE && lWidthPixels > 64
		;Divide by 2
		shr lWidthPixels,1   
		shr lHeightPixels,1
	.endif

	;Create A Temporary Bitmap
	mov bi.bmiHeader.biSize,sizeof(BITMAPINFOHEADER)
	mov bi.bmiHeader.biBitCount,32	; 32 Bit
	m2m bi.bmiHeader.biWidth, lWidthPixels		; Power Of Two Width
	m2m bi.bmiHeader.biHeight,lHeightPixels		; Make Image Top Up (Positive Y-Axis)
	mov bi.bmiHeader.biCompression, BI_RGB		; RGB Encoding
	mov bi.bmiHeader.biPlanes, 1				; #Bitplanes = ONE

	;Creating A Bitmap This Way Allows Us To Specify Color Depth And Gives Us Imediate Access To The Bits
	mov hbmpTemp ,$invoke(CreateDIBSection,hdcTemp,addr bi,DIB_RGB_COLORS,addr pBits,0,0)
	.if !eax
		invoke DeleteDC,hdcTemp			 ; Delete The Device Context
		OCall pPicture::IPicture,Release ; Decrements IPicture Reference Count
		return FALSE
	.endif

	;Select Handle To Our Temp DC And Our Temp Bitmap Object
	invoke SelectObject,hdcTemp, hbmpTemp
	

	;Render The IPicture On To The Bitmap
	xor eax,eax
	sub eax,lHeight
	OCall pPicture::IPicture,Render,hdcTemp,0,0,lWidthPixels,lHeightPixels,0,lHeight,lWidth,eax,0

	; Convert From BGR To RGB Format And Add An Alpha Value Of 255
	SetObject esi
	.if [esi].m_fAlphaConversion == TRUE
		; it is important to remember that the colour order is different as we are reading in a DWORD
		; so the intel processor will read it in as little endian so the order is reversed (ABGR)
		movzx eax,[esi].m_BlueAlphaMatch
		shl eax,8
		mov al,[esi].m_GreenAlphaMatch
		shl eax,8
		mov al,[esi].m_RedAlphaMatch
		
		mov eax,lWidthPixels
		mul lHeightPixels
		mov NumPixels,eax
		ReleaseObject
		mov esi,pBits		
		.while NumPixels
			; swap red and blue over
			mov eax,dword ptr[esi]
			mov ebx,eax
			mov ecx,eax
			shr eax,16
			and ebx,0000FF00h
			or eax,ebx
			and ecx,000000FFh
			shl ecx,16
			or eax,ecx
			.if eax!=AlphaMatch
				or eax,0FF000000h
			.endif
			mov dword ptr[esi],eax
			add esi,4
			dec NumPixels
		.endw

	.else
		mov eax,lWidthPixels
		mul lHeightPixels
		mov NumPixels,eax
		mov esi,pBits
		.while NumPixels
			;Switch Red and Blue bytes
			mov al,byte ptr[esi]
			b2b [esi], [esi+2]
			mov byte ptr[esi+2],al
			;Set Alpha
			mov byte ptr[esi+3],255
			add esi,4
			dec NumPixels
		.endw
	.endif

	mov eax,pglTexture
	m2m [eax].glTexture.dwWidth, bi.bmiHeader.biWidth
	m2m [eax].glTexture.dwHeight , bi.bmiHeader.biHeight
	m2m [eax].glTexture.Bpp , bi.bmiHeader.biBitCount
	mov [eax].glTexture.dwType , GL_RGBA

	;Create The Texture
	invoke glGenTextures,1, addr [eax].glTexture.TextureID

	;generate the texture using the filtering model selected
	OCall GenerateTexture,pglTexture, pBits

	;Delete The Object and Device Context
	invoke DeleteObject,hbmpTemp	
	invoke DeleteDC,hdcTemp

	OCall pPicture::IPicture.Release	;Decrements IPicture Reference Count
	mov eax,TRUE
MethodEnd

; Load a TGA file
Method TextureLoader.LoadTGAFromDisk,,pszFileName,pglTexture
local fTGA,pImgData,glMaxTexDim,colorbuffer,chunkheader
local header:TGAHeader
local imageType,imageHeight,imageWidth,imageBytesPerPel,imageSize
local MyQ:QWORD

	mov fTGA ,$New(DiskStream,Init,NULL, pszFileName)

	.if eax == NULL
		Destroy fTGA
		return FALSE
	.endif

	OCall fTGA::DiskStream.BinRead,addr header,sizeof header
	.if eax == 0
		Destroy fTGA
		return FALSE
	.endif
	
	; Precalc some values from the header
	
	b2d imageType, header.ImgType

	movzx eax,header.WidthHi
	shl eax,8
	movzx ebx,header.WidthLo
	add eax,ebx
	mov imageWidth,eax
	
	movzx eax,header.HeightHi
	shl eax,8
	movzx ebx,header.HeightLo
	add eax,ebx
	mov imageHeight,eax
	
	movzx eax,header.Bpp
	shr eax,3
	mov imageBytesPerPel,eax	
	mul imageHeight
	mul imageWidth
	mov imageSize,eax

	; load up our texture information
	mov esi,pglTexture
	m2m [esi].glTexture.dwWidth  , imageWidth
	m2m [esi].glTexture.dwHeight , imageHeight
	b2d [esi].glTexture.Bpp, header.Bpp

	.if header.Bpp == 24
		mov [esi].glTexture.dwType , GL_RGB
	.else
		mov [esi].glTexture.dwType , GL_RGBA
	.endif
	
	; Validate header info
	.if( ( imageType != 2 && imageType != 10 ) ||\
	    ( imageWidth == 0 ) || ( imageHeight == 0 ) ||\
		( imageBytesPerPel != 3 && imageBytesPerPel != 4 ) )

		; invalid header, bomb out
		Destroy fTGA
		return FALSE
	.endif

	; Allocate the memory for the image size
	mov pImgData , $MemAlloc(imageSize)
	.if eax==0
		Destroy fTGA
		return FALSE
	.endif

	; Skip image ident field
	.if header.ImgIdent > 0 
		movzx eax, header.ImgIdent
		mov MyQ.HiDWord,0
		mov MyQ.LoDWord,eax
		OCall fTGA::DiskStream.Seek,MyQ
	.endif

	; un-compressed image
	.if imageType == 2
		OCall fTGA::DiskStream.BinRead,pImgData,imageSize
		.if eax!=imageSize
			MemFree pImgData
			Destroy fTGA
			return FALSE
		.endif

		xor ecx,ecx
		mov esi,pImgData
		.while ecx<imageSize
			;Switch Red and Blue bytes
			mov al,byte ptr[esi]
			b2b [esi],[esi+2]
			mov byte ptr[esi+2],al
			inc ecx			
			add esi,imageBytesPerPel
		.endw

	.else
		; compressed image
		mov eax,imageHeight
		mul imageWidth
		mov pixelcount,eax
		mov currentpixel,0
		mov currentbyte,0
		
		mov colorbuffer,$MemAlloc(imageBytesPerPel)
		.repeat	
			mov chunkheader , 0
			OCall fTGA::DiskStream.BinRead,addr chunkheader,1
			.if eax==0
				MemFree colorbuffer
				MemFree pImgData
				Destroy fTGA
				return FALSE
			.endif
			
			;If the header is < 128, it means the that is the number
			;of RAW color packets minus 1 that follow the header..
			.if chunkheader < 128 
				;add 1 to get number of following color values
				inc chunkheader
				

				;Read RAW color values
				mov counter,0
				xor eax,eax
				.while eax < chunkheader	
					OCall fTGA::DiskStream.BinRead,colorbuffer,imageBytesPerPel
					.if eax!=imageBytesPerPel
						MemFree pImgData
						MemFree colorbuffer
						Destroy fTGA
						return FALSE
					.endif
					; write to memory
					mov esi,pImgData
					mov edi,colorbuffer
					mov eax,currentbyte
					;Flipping color bytes (sigh)
					b2b [esi+eax+0] , [edi+2]
					b2b [esi+eax+1] , [edi+1]
					b2b [esi+eax+2] , [edi+0]
					;if its a 32 bpp image, copy Alpha byte
					.if imageBytesPerPel == 4
						b2b [esi+eax+3] , [edi+3]
					.endif
					
					; Increase the current byte by the number of bytes per pixel
					mov eax,imageBytesPerPel
					add currentbyte , eax
					; Increment current pixel 
					inc currentpixel
					mov eax,currentpixel
					.if eax > pixelcount ; Make sure we havent read too many pixels
						MemFree pImgData
						MemFree colorbuffer
						Destroy fTGA
						return FALSE
					.endif
					inc counter
					mov eax,counter
				.endw

			.else ;chunkheader > 128 RLE data, next color reapeated chunkheader - 127 times
				;Subtract 127 to get rid of the ID bit
				sub chunkheader,127 
				OCall fTGA::DiskStream.BinRead,colorbuffer,imageBytesPerPel
				.if eax!=imageBytesPerPel ;	 Attempt to read following color values
					MemFree colorbuffer
					MemFree pImgData
					Destroy fTGA
					return FALSE
				.endif

				;copy the color into the image data as many times as dictated by the header
				mov counter , 0
				xor eax,eax
				.while eax < chunkheader
					mov esi,pImgData
					mov edi,colorbuffer
					mov eax,currentbyte
					;Flipping color bytes (sigh)
					b2b [esi+eax+0] ,[edi+2]
					b2b [esi+eax+1] ,[edi+1]
					b2b [esi+eax+2] ,[edi+0]
					;if its a 32 bpp image, copy Alpha byte
					.if imageBytesPerPel == 4
						b2b [esi+eax+3] ,[edi+3]
					.endif
					
					
					; Increase current byte by the number of bytes per pixel
					mov eax,imageBytesPerPel
					add currentbyte,eax
					inc currentpixel
					mov eax,currentpixel
					.if eax > pixelcount
						MemFree colorbuffer
						MemFree pImgData
						Destroy fTGA
						return FALSE
					.endif
					inc counter
					mov eax,counter
				.endw
			.endif
			mov eax,currentpixel
		.until eax==pixelcount   ; Loop while there are still pixels left
	.endif
	Destroy fTGA

;
; Scale Image to be a power of 2
;
	; Get Maximum Texture Size Supported
	invoke glGetIntegerv,GL_MAX_TEXTURE_SIZE, addr glMaxTexDim
						
	; Resize Image To Closest Power Of Two
	mov eax,imageWidth
	.if eax <= glMaxTexDim ; Is Image Width Less Than Or Equal To Cards Limit
		mov lWidthPixels , $FirstLowerPowerOfTwo(imageWidth)
	.else  ; Otherwise  Set Width To "Max Power Of Two" That The Card Can Handle
		m2m lWidthPixels , glMaxTexDim
	.endif
	
	mov eax,imageHeight
	.if eax <= glMaxTexDim ; Is Image Height Greater Than Cards Limit
		mov lHeightPixels, $FirstLowerPowerOfTwo(imageHeight)
	.else  ; Otherwise  Set Height To "Max Power Of Two" That The Card Can Handle
		m2m lHeightPixels , glMaxTexDim
	.endif
	
	; if low quality textures then make them halve the size which saved 4 times the texture space
	SetObject esi
	.if [esi].m_fHighQualityTextures == FALSE && lWidthPixels > 64
		shr lWidthPixels,1		;/2
		shr lHeightPixels,1
	.endif

	; if the size needs to change, the rescale the raw image data
	mov eax,lWidthPixels
	mov ebx,lHeightPixels
	.if (eax != imageWidth && ebx != imageHeight)	
		mov eax,lWidthPixels
		mul lHeightPixels
		mul imageBytesPerPel
		mov pNewImgData,$MemAlloc (eax)

		.if imageBytesPerPel == 4
			mov dwformat , GL_RGBA
		else
			mov dwformat , GL_RGB
		.endif

		invoke gluScaleImage,dwformat, imageWidth, imageHeight, GL_UNSIGNED_BYTE, pImgData,
							  lWidthPixels, lHeightPixels, GL_UNSIGNED_BYTE, pNewImgData
		MemFree pImgData ;free the original image data

		; ..old becomes new..
		m2m pImgData , pNewImgData

		; update our texture structure
		mov eax,pglTexture
		m2m [eax].glTexture.dwWidth  , lWidthPixels
		m2m [eax].glTexture.dwHeight , lHeightPixels
	.endif

	; Typical Texture Generation Using Data From The TGA loader
	; (Create The Texture)
	mov eax,pglTexture
	invoke glGenTextures,1,addr [eax].glTexture.TextureID

	; generate the texture using the filtering model selected
	OCall GenerateTexture,pglTexture, pImgData

	; free the memory allocated
	MemFree pImgData	
	MemFree colorbuffer ;<-- HOMER FOUND THIS LEAK

	mov eax, TRUE
MethodEnd


;Load JPG or GIF
Method TextureLoader.LoadJPG_GIFResource,,pResourceName, pResourceType, pglTexture
local hdcTemp,hbmpTemp,pPicture,lWidth,lHeight,lWidthPixels,lHeightPixels,glMaxTexDim,hRes
local dwDataSize,hGlob,pData,hGlobal,pData2,pStream,pBits
local bi:BITMAPINFO
	mov hRes,$invoke (FindResource,NULL, pResourceName, pResourceType)
	mov dwDataSize,$invoke (SizeofResource,NULL,hRes)
	mov	hGlob,$invoke (LoadResource,NULL,hRes)
	mov	pData,$invoke (LockResource,hGlob)
	mov hGlobal,$invoke (GlobalAlloc,GMEM_MOVEABLE, dwDataSize)
	mov pData2,$invoke (GlobalLock,hGlobal)
	invoke RtlMoveMemory,pData2, pData, dwDataSize
	invoke GlobalUnlock,hGlobal
	invoke UnlockResource,hGlob
	invoke FreeResource,hGlob

	mov pStream , NULL
	pPicture = NULL
	HRESULT hr;

	.if ($invoke(CreateStreamOnHGlobal,hGlobal, TRUE, addr pStream)) != S_OK
		return FALSE
	.endif
	invoke OleLoadPicture,pStream, 0, 0, addr IID_IPicture, addr pPicture
	push eax
	OCall pStream::IUnknown.Release
	pop eax
	.if eax==FAILED
		return FALSE
	.endif
	mov hdcTemp , $invoke (CreateCompatibleDC,$invoke (GetDC,0))
	.if !eax
		OCall pPicture::IUnknown.Release
		return FALSE
	.endif
	invoke glGetIntegerv,GL_MAX_TEXTURE_SIZE, addr glMaxTexDim
	OCall pPicture::IPicture.get_Width,addr lWidth
	mov lWidthPixels,$invoke (MulDiv,lWidth, ($invoke (GetDeviceCaps,hdcTemp, LOGPIXELSX)), 2540)
	OCall pPicture::IPicture.get_Height,addr lHeight
	mov lHeightPixels,$invoke (MulDiv,lHeight, ($invoke (GetDeviceCaps,hdcTemp, LOGPIXELSY)), 2540)

	; Resize Image To Closest Power Of Two
	mov eax,lWidthPixels
	.if eax <= glMaxTexDim ; Is Image Width Less Than Or Equal To Cards Limit
		mov lWidthPixels, $FirstLowerPowerOfTwo(lWidthPixels)
	.else  ; Otherwise  Set Width To "Max Power Of Two" That The Card Can Handle
		m2mlWidthPixels , glMaxTexDim
	.endif
	
	mov eax,lHeightPixels
	.if eax <= glMaxTexDim ; Is Image Height Greater Than Cards Limit
		mov lHeightPixels , $FirstLowerPowerOfTwo(lHeightPixels)
	.else  ; Otherwise  Set Height To "Max Power Of Two" That The Card Can Handle
		m2m lHeightPixels , glMaxTexDim
	.endif
		
	; if low quality textures then make them halve the size which saved 4 times the texture space
	SetObject esi
	.if [esi].m_fHighQualityTextures == FALSE && lWidthPixels > 64
		shr lWidthPixels,1 ;/2
		shr lHeightPixels,1
	.endif

	;	Create A Temporary Bitmap
	mov bi.bmiHeader.biSize, sizeof(BITMAPINFOHEADER)	
	mov bi.bmiHeader.biBitCount,32
	m2m bi.bmiHeader.biWidth, lWidthPixels
	m2m bi.bmiHeader.biHeight,lHeightPixels	
	mov bi.bmiHeader.biCompression	, BI_RGB
	mov bi.bmiHeader.biPlanes,1

	;	Creating A Bitmap This Way Allows Us To Specify Color Depth And Gives Us Imediate Access To The Bits
	mov hbmpTemp ,$invoke (CreateDIBSection,hdcTemp, addr bi, DIB_RGB_COLORS,addr pBits, 0, 0)
	.if !eax
		invoke DeleteDC(hdcTemp)
		OCall pPicture::IPicture.Release
		return FALSE
	.endif
	invoke SelectObject,hdcTemp, hbmpTemp)

	; Render The IPicture On To The Bitmap
	xor eax,eax
	sub eax,lHeight
	OCall pPicture::IPicture.Render,hdcTemp, 0, 0, lWidthPixels, lHeightPixels, 0, lHeight, lWidth, eax, 0)

	; Convert From BGR To RGB Format And Add An Alpha Value Of 255
	SetObject esi
	mov eax,pglTexture
	.if [esi].m_fAlphaConversion == TRUE && [eax].glTexture.TexType != txJpg
		; it is important to remember that the colour order is different as we are reading in a DWORD
		; so the intel processor will read it in as little endian so the order is reversed (ABGR)
		movzx eax,[esi].m_BlueAlphaMatch
		shl eax,8
		or eax,[esi].m_GreenAlphaMatch
		shl eax,8
		or eax,[esi].m_RedAlphaMatch
		mov AlphaMatch ,eax
		
		ReleaseObject
		mov esi,pBits

		mov eax,lWidthPixels
		mul lHeightPixels
		mov NumPixels,eax
		.while NumPixels					
			DWORD	tempRGB;

			mov eax,dword ptr[esi]
			; swap red and blue over
			mov ebx,eax
			mov ecx,eax
			and eax,0FF00h
			and ebx,0FF0000h
			and ecx,0FFh
			shr ebx,16
			shl ecx,16
			or eax,ebx
			or eax,ecx
			.if eax!=AlphaMatch
				or eax,0FF000000h
			.endif
			mov dword ptr[esi],eax
			add esi,4
			dec NumPixels
		.endw

	.else
		mov esi,pBits
		mov eax,lWidthPixels
		mul lHeightPixels
		mov NumPixels,eax
		.while NumPixels
			mov al,byte ptr[esi]
			m2m byte ptr [esi],byte ptr [esi+2]
			mov byte ptr [esi+2],al
			mov byte ptr [esi+3],255
			add esi,4
			dec NumPixels
		.endw
	.endif

	mov eax,pglTexture
	m2m [eax].glTexture.dwWidth , bi.bmiHeader.biWidth
	m2m [eax].glTexture.Height , bi.bmiHeader.biHeight
	m2m [eax].glTexture.Bpp , bi.bmiHeader.biBitCount
	mov [eax].glTexture.dwType , GL_RGBA

	invoke glGenTextures,1, addr [eax].glTexture.TextureID
	; generate the texture using the filtering model selected
	OCall GenerateTexture,pglTexture,pBits

	invoke DeleteObject,hbmpTemp
	invoke DeleteDC,hdcTemp
	OCall pPicture::IPicture.Release

	mov eax, TRUE
MethodEnd


Method TextureLoader.LoadTGAResource,,pResourceName, pResourceType, pglTexture
local hRes,dwDataSize,hGlob,pData,hGlobal,pData2
local 
	mov hRes,$invoke (FindResource,NULL, pResourceName, pResourceType)
	mov dwDataSize,$invoke (SizeofResource,NULL,hRes)
	mov	hGlob,$invoke(LoadResource,NULL,hRes)
	mov	pData,$invoke(LockResource,hGlob)

	; allocate some global memory and copy the resource data into it..
	mov hGlobal ,$invoke (GlobalAlloc,GMEM_MOVEABLE, dwDataSize);
	mov pData2 ,$invoke (GlobalLock,hGlobal)
	invoke RtlMoveMemory,pData2, pData, dwDataSize

	; unlock and free the resource
	invoke GlobalUnlock,hGlobal
	invoke UnlockResource,hGlob
	invoke FreeResource,hGlob

	; load it
	push $OCall (LoadTextureFromRam,pData2, dwDataSize, pglTexture, txTga)
	; free our global memory (16 bit os's only)
	invoke GlobalFree,hGlobal
	pop eax
MethodEnd

; Set the Texture parameters to match the type of filtering we want.
Method TextureLoader.GenerateTexture,uses esi,pglTexture, pImgData
local components,Mipping
local Mipping
	mov eax,pglTexture
	.if [eax].glTexture.dwType == GL_RGBA
		mov components , 4
	.else
		mov components , 3
	.endif

	; Typical Texture Generation Using Data From The Bitmap
	; Bind To The Texture ID
	invoke glBindTexture,GL_TEXTURE_2D, [eax].glTexture.TextureID

	invoke glTexParameteri,GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP
	invoke glTexParameteri,GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP

	SetObject esi
	mov eax,[esi].m_TextureFilterType
	.if eax== txNoFilter
		invoke glPixelStorei,GL_UNPACK_ALIGNMENT, 1 
		invoke glTexParameteri,GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST
		invoke glTexParameteri,GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST
		mov Mipping , FALSE
	.elseif eax==txBilinear
		.if [esi].m_fMipMapping == FALSE
			invoke glTexParameteri,GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR
			invoke glTexParameteri,GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR
			mov Mipping , FALSE
		.else
			invoke glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR
			invoke glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST
			mov Mipping , TRUE
		.endif

	.elseif eax== txTrilinear
		invoke glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR
		invoke glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR
		mov Mipping , TRUE	;always mip mapping for trilinear
	.endif

	; crank out the texture
	mov eax,pglTexture
	.if Mipping == FALSE
		invoke glTexImage2D,GL_TEXTURE_2D,0,components,
			 [eax].glTexture.dwWidth,[eax].glTexture.dwHeight,0,
			 [eax].glTexture.dwType,GL_UNSIGNED_BYTE,pImgData
	.else
		; Build Mipmaps (builds different versions of the picture for distances - looks better)
		invoke gluBuild2DMipmaps,GL_TEXTURE_2D,components,
			[eax].glTexture.dwWidth,[eax].glTexture.dwHeight,
			[eax].glTexture.dwType,GL_UNSIGNED_BYTE,pImgData
	.endif
	xor eax,eax
MethodEnd

;extract the extension from the specified filename
Method TextureLoader.ExtensionFromFilename,uses ecx,pszFileName, pszExtension
	invoke lstrlen,pszFileName
	mov ecx,pszFileName
	.while eax>0
		.if byte ptr[eax+ecx] == '.'
			dec eax
			.break
		.endif
	.endw
	.if eax <= 0
		mov ecx,pszExtension
		mov byte ptr[ecx],0
	.else
		add eax,ecx
		invoke lstrcpy,pszExtension, eax
	.endif
MethodEnd


endif
endif
