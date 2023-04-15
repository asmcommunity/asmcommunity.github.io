include \masm32\ultrano\bank\base.inc
include \masm32\include\msacm32.inc
includelib \masm32\lib\msacm32.lib
.data
	decodersFound dd 0
	g_mp3stream   dd 0
.code
	
ACMDRIVERDETAILS struct
	cbStruct	dd ?
	fccType		FOURCC <> ; compressor type 'audc'
	fccComp		FOURCC <> ; sub-type (not used; reserved)
	wMid		dw ? ; manufacturer id
	wPid		dw ? ; product id
	
	vdwACM		dd ? ; version of the ACM *compiled* for
	vdwDriver	dd ? ; version of the driver
	
	fdwSupport	dd ? ; misc. support flags
	cFormatTags	dd ? ; total unique format tags supported
	cFilterTags	dd ? ; total unique filter tags supported
	
	hicon		dd ? ; handle to custom icon
	
	szShortName	db 32 dup (?)
	szLongName	db 128 dup (?)
	szCopyright	db 80 dup (?)
	szLicensing	db 128 dup (?)
	szFeatures	db 512 dup (?)
ACMDRIVERDETAILS ends
	
ACMFORMATTAGDETAILS struct
	cbStruct 		dd ?
	dwFormatTagIndex	dd ?
	dwFormatTag		dd ?
	cbFormatSize		dd ?
	fdwSupport		dd ?
	cStandardFormats	dd ?
	szFormatTag		db 48 dup (?)
	
ACMFORMATTAGDETAILS ends

MPEGLAYER3_WFX_EXTRA_BYTES  equ 12

MPEGLAYER3WAVEFORMAT struct
	wfx		WAVEFORMATEX <>
	wID		dw ?
	fdwFlags	dd ?
	nBlockSize	dw ?
	nFramesPerBlock	dw ?
	nCodecDelay	dw ?
MPEGLAYER3WAVEFORMAT ends

WAVE_FORMAT_MPEGLAYER3 equ 55h
WAVE_FORMAT_OGG equ 'gO'

ACMSTREAMHEADER struct
	cbStruct		dd ?
	fdwStatus		dd ?
	dwUser			dd ?
	pbSrc			dd ?
	cbSrcLength		dd ?
	cbSrcLengthUsed		dd ?
	dwSrcUser		dd ?
	pbDst			dd ?
	cbDstLength		dd ?
	cbDstLengthUsed		dd ?
	dwDstUser		dd ?
	dwReservedDriver	dd 10 dup (?)
ACMSTREAMHEADER ends


		
acmDriverEnumCallback proc hadid,dwInstance,fdwSupport
	local details:ACMDRIVERDETAILS,driver
	local fmtDetails:ACMFORMATTAGDETAILS
	test fdwSupport,1
	jz _ret
	Clear details
	mov details.cbStruct,sizeof ACMDRIVERDETAILS
	invoke acmDriverDetails,hadid,addr details,0
	invoke acmDriverOpen,addr driver,hadid,0
	xor ecx,ecx
	.while ecx<details.cFormatTags && ecx<300
		push ecx
		Clear fmtDetails
		mov fmtDetails.cbStruct,sizeof ACMFORMATTAGDETAILS
		mov fmtDetails.dwFormatTagIndex,ecx
		invoke acmFormatTagDetails,driver,addr fmtDetails,0
		.if fmtDetails.dwFormatTag == WAVE_FORMAT_MPEGLAYER3
			inc decodersFound
			;trace "Found decoder: %s",addr details.szLongName
		.endif
		pop ecx
		inc ecx
	.endw
	invoke acmDriverClose,driver,0
	
 _ret:
	mov eax,1
	ret
acmDriverEnumCallback endp

convertMP3 proc MP3File,OutputFile
	local maxFormatSize,waveFormat,mp3format,baka
	local f1,f2,rawbufsize,rawbuf
	local mp3buf[522]:byte
	local mp3streamHead:ACMSTREAMHEADER
	
	invoke acmDriverEnum,acmDriverEnumCallback,0,0
	.if !decodersFound
		msgbox "No decoders found"
		return 0
	.endif
	mov maxFormatSize,0
	invoke acmMetrics,0,50,addr maxFormatSize
	mov waveFormat,malloc(maxFormatSize)
	assume eax:ptr WAVEFORMATEX
	mov [eax].wFormatTag,WAVE_FORMAT_PCM
	mov [eax].nChannels,1
	mov [eax].nSamplesPerSec,44100
	mov [eax].wBitsPerSample,16
	mov [eax].nBlockAlign,2
	mov [eax].nAvgBytesPerSec,2*44100
	mov [eax].cbSize,0
	mov mp3format,malloc(maxFormatSize)
	assume eax:ptr MPEGLAYER3WAVEFORMAT
	mov [eax].wfx.cbSize,MPEGLAYER3_WFX_EXTRA_BYTES
	mov [eax].wfx.wFormatTag,WAVE_FORMAT_MPEGLAYER3
	mov [eax].wfx.nAvgBytesPerSec ,128 * (1024 / 8)
	mov [eax].wfx.nChannels,1
	mov [eax].wfx.wBitsPerSample,0
	mov [eax].wfx.nBlockAlign,1
	mov [eax].wfx.nSamplesPerSec,44100
	mov [eax].fdwFlags,2 ; MPEGLAYER3_FLAG_PADDING_OFF
	mov [eax].nBlockSize,522
	mov [eax].nFramesPerBlock,1
	mov [eax].nCodecDelay,1393
	mov [eax].wID,1 ; MPEGLAYER3_ID_MPEG
	assume eax:nothing
	
	invoke acmStreamOpen,addr g_mp3stream,0,mp3format,waveFormat,0,0,0,4
	push eax
	free waveFormat
	free mp3format
	pop eax
	.if eax!=MMSYSERR_NOERROR
		print eax
		msgbox "Error converting"
		return 0
	.endif
	mov f1,fopen(MP3File,"rb")
	print f1
	.if f1==-1
		invoke acmStreamClose,g_mp3stream,0
		msgbox "can't open the mp3"
		return 0
	.endif
	
	
	invoke acmStreamSize,g_mp3stream,522,addr rawbufsize,0
	mov rawbuf,malloc(rawbufsize)
	print rawbufsize
	
	
	ZeroMemory &mp3streamHead,sizeof ACMSTREAMHEADER
	mov mp3streamHead.cbStruct,sizeof ACMSTREAMHEADER
	lea eax,mp3buf
	mov mp3streamHead.pbSrc,eax
	mov mp3streamHead.cbSrcLength,522
	m2m mp3streamHead.pbDst,rawbuf
	m2m mp3streamHead.cbDstLength,rawbufsize
	invoke acmStreamPrepareHeader,g_mp3stream,addr mp3streamHead,0
	mov f2,fopen(OutputFile,"wb")
	
	print OutputFile
	print "Starting decoding..."
   _again:
	invoke ReadFile,f1,addr mp3buf,522,addr baka,0
	cmp baka,0
	je _done
	invoke acmStreamConvert,g_mp3stream,addr mp3streamHead,4
	invoke WriteFile,f2,rawbuf,mp3streamHead.cbDstLengthUsed,addr baka,0
	
	jmp _again
	_done:
	print "Decoding complete!"
	invoke CloseHandle,f1
	invoke CloseHandle,f2
	invoke acmStreamUnprepareHeader,g_mp3stream,addr mp3streamHead,0
	free rawbuf
		
	invoke acmStreamClose,g_mp3stream,0
	
	return 1
convertMP3 endp




start:
	
	msgbox "Start"
	invoke convertMP3,T("input3.mp3"),T("output.raw")
	
	;;invoke convertMP3,T("input.mp3"),T("output.raw")
	
	invoke ExitProcess,0
end start
