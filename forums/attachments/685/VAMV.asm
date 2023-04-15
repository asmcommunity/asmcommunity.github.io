include vamv.inc

FRAME_TO_RIP = 509


DumpMyDib proc pDib
	local f1,baka,pDibData
	mov eax,pDib
	or eax,eax
	jz _ret
	mov eax,[eax].BITMAPINFOHEADER.biSize
	add eax,pDib
	mov pDibData,eax
	
	
	invoke CreateFile,T("out.bmp"),GENERIC_READ or GENERIC_WRITE,0,0,OPEN_ALWAYS,0,0
	mov f1,eax
	invoke SetFilePointer,f1,54,0,FILE_BEGIN
	mov ecx,pDib
	mov eax,[ecx].BITMAPINFOHEADER.biWidth
	imul eax,[ecx].BITMAPINFOHEADER.biHeight
	lea edx,[eax+eax*2]
	
	invoke WriteFile,f1,pDibData,edx,addr baka,0
	invoke CloseHandle,f1
	
 _ret:	ret
DumpMyDib endp

	

InitAvi proc uses ebx ecx edx FileName
	local pfile,pavi
	local bih:BITMAPINFOHEADER
	local info:AVIFILEINFO
	local pgetf
	local lpdata,datasize
	
	invoke AVIStreamOpenFromFile,addr pavi,FileName,streamtypeVIDEO,0,0,0
	.if eax
		xor eax,eax
		ret
	.endif
	
	
	
	Clear bih
	mov bih.biSize,sizeof BITMAPINFOHEADER
	mov bih.biWidth,640
	mov bih.biHeight,360
	mov bih.biPlanes,1
	mov bih.biBitCount,24
	mov bih.biCompression,BI_RGB
	
	invoke AVIStreamGetFrameOpen,pavi,addr bih
	mov pgetf,eax
	
	invoke AVIStreamGetFrame,pgetf,FRAME_TO_RIP
	invoke DumpMyDib,eax
	
	
	;----[ success ]------\
	mov eax,1
	ret
	;---------------------/
	
	;----[ bad ]----------\
_ret:	xor eax,eax
	ret
	;---------------------/
InitAvi endp

main proc
	invoke InitAvi,T("test.avi")
	;print eax
	ret
main endp

start:
	invoke AVIFileInit
	invoke main
	invoke AVIFileExit
	invoke ExitProcess,0
end start
