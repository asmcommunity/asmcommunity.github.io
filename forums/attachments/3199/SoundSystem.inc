;3D SoundSystem class, powered by Open Audio Library (OpenAL)
;Written June 2010 by Leith Ketchell

;Classes:
;SoundSystem  = main class, also represents the 3D Listener (ie camera)
;SoundSource  = 3D audio emitter

ifndef DwordCollection
	LoadObjects DwordCollection
endif

;Constants for SoundSource.Controls method
SOUND_PLAY   equ 0	
SOUND_PAUSE  equ 1
SOUND_REWIND equ 2
SOUND_STOP   equ 3

include SoundSystem_MoreStructs.inc

; ——————————————————————————————————————————————————————————————————————————————————————————————————

;Orientation of the Audio Listener (your 3D camera, whatever)
Orient struct
	vFacing Vec3 <>	;Direction we are facing
	vUp Vec3 <>		;Direction of UP
Orient ends

;The SoundSystem is the main engine component, although theres really not much going on here.
;It also represents the local listener in 3D space, which is worth knowing :)
Object SoundSystem, 534534, Primer
	RedefineMethod	Init
	RedefineMethod	Done	
	StaticMethod	Update		;<-- call whenever 3D camera moves/rotates

	Private
		StaticMethod Monitor
	PrivateEnd
	
	;You can change these, and then call Update
	DefineVariable vPosition, Vec3, {<0.0f, 0.0f, 0.0f>} ; Position of the Listener.
	DefineVariable vVelocity, Vec3, {<0.0f, 0.0f, 0.0f>} ; Velocity of the Listener.
	DefineVariable vvOrientation, Orient, {<< 0.0f, 0.0f, -1.0f>, <0.0f, 1.0f, 0.0f>>} ;Note: Normalized vectors		


	Embed StreamSources_Playing,DwordCollection	;Monitor uses this to refill streaming buffers

	;Internal Use Only
	DefineVariable Device,Pointer ;-> ALCcontext
	DefineVariable Context,Pointer;-> ALCdevice	
	DefineVariable bShutDown, BOOL, FALSE
ObjectEnd


; ——————————————————————————————————————————————————————————————————————————————————————————————————

;This class represents a 3D audio emitter, complete with position, velocity etc.
;It supports Static WAV, Streaming WAV, and Streaming MP3
Object SoundSource, 435345, Primer
	RedefineMethod	Init, 			Pointer, SDWORD, BOOL
	RedefineMethod Done

	StaticMethod Controls, dword
	StaticMethod Update
	StaticMethod GetState
	
	StaticMethod 	Init_MP3_Streaming_Support,Pointer
	StaticMethod	Load_WAV_File_Static,  LPSTR
	StaticMethod	Load_WAV_File_Stream,  LPSTR
	StaticMethod	Load_MP3_File,	LPSTR
	StaticMethod    Refill			;Call this no less than once per second


	Private
		StaticMethod Bind_SoundBuffer,BOOL	
		StaticMethod Init_Decompression
		StaticMethod MapFile,LPSTR,Pointer,Pointer,Pointer
		StaticMethod Map_WAV_File,LPSTR
		StaticMethod Map_MP3_File,LPSTR
		StaticMethod Parse_WAV_Header,Pointer, Pointer,  Pointer, Pointer
		StaticMethod Parse_MP3_Header,Pointer, Pointer, Pointer
		StaticMethod Parse_MP3_FrameHeader,Pointer, Pointer,Pointer
		StaticMethod stream, 		Pointer
		StaticMethod UnMapFile
	PrivateEnd

	DefineVariable bStreaming, 		   BOOL,FALSE	;Flag - use multibuffer queue ('streaming') ??
	DefineVariable bDataExhausted,	   BOOL,FALSE
	DefineVariable bCompressed,		   BOOL,FALSE	;Flag - Is the stream compressed ('AUDIO CODEC') ??
	DefineVariable bHave_Desired_Codec,BOOL,FALSE	;Flag - Is the required CODEC installed on this system?
	DefineVariable Desired_Codec_Tag				;FOURCC Identifier of the required Audio Codec
	DefineVariable ACMStreamHandle,dword,-1			;Handle of ACM conversion stream	
	DefineVariable Source, ALuint					;Handle to oal source object (sound emitter)
	
	DefineVariable vPosition, Vec3, {<0.0f, 0.0f, 0.0f>}	; Position of the source sound.
	DefineVariable vVelocity, Vec3, {<0.0f, 0.0f, 0.0f>}  ; Velocity of the source sound.

	;FileMapping stuff
	DefineVariable pFileBase		;ptr to base of WAV file data (RIFF Header)
	DefineVariable hFileMapping		;Handle to Mapped View of File
	DefineVariable hFile			;Handle to File

	;Static stuff
	DefineVariable pPCMData			;ptr to start of PCM/WAV audio data
	DefineVariable dFileData_Size	;#bytes of PCM data

	;Streaming stuff
	DefineVariable  dCompressed_FrameSize	;Size of compressed frame
	DefineVariable  dPCM_FrameSize			;Size of decompressed frame
	DefineVariable  pFrame_PCM, Pointer, NULL
	DefineVariable  pFrame_Compressed, Pointer, NULL
	DefineVariable  ACMStreamHeader, ACMSTREAMHEADER, {<>}
	DefineVariable	dStreamOffset
	
	;WAV format stuff
	DefineVariable dBytesPerSecond
	DefineVariable format			;Format (channels/bits)
	DefineVariable freq				;Sample Rate

	DefineVariable  Buffer, ALuint	;Handle to oal buffer object (sound buffer)
	Embed 			Buffers, 		DwordCollection ;Streaming buffers
ObjectEnd



; ——————————————————————————————————————————————————————————————————————————————————————————————————

; ——————————————————————————————————————————————————————————————————————————————————————————————————

;Method:	SoundSystem.Init
;Purpose:	Initialize OpenAL Audio Library
Method SoundSystem.Init, uses esi
LOCAL tid
	SetObject esi
	OCall [esi].StreamSources_Playing::DwordCollection.Init,esi,16,256,-1
	mov [esi].Device, $invoke (alcOpenDevice,"DirectSound3D")
	.if eax == NULL
		return FALSE
	.endif
	mov [esi].Context,$invoke(alcCreateContext,eax,NULL)
	.if eax == NULL
		return FALSE
	.endif
	invoke alcMakeContextCurrent,eax
	.if $invoke(alGetError) != AL_NO_ERROR
		return AL_FALSE
	.endif
	invoke CreateThread,0,0,$MethodAddr(SoundSystem.Monitor),esi,0,addr tid
	invoke CloseHandle,eax
	OCall esi.Update	
MethodEnd

;Method:	SoundSystem.Done
;Purpose:	Release OpenAL Library
Method SoundSystem.Done, uses esi
	SetObject esi
	mov [esi].bShutDown,TRUE
	.repeat 
		DbgWarning "Waiting for Worker Death"
		invoke Sleep,500	
	.until [esi].bShutDown==FALSE
	OCall [esi].StreamSources_Playing::DwordCollection.Done
	invoke alcDestroyContext, [esi].Context
	invoke alcCloseDevice, [esi].Device
MethodEnd

;Method:	SoundSystem.Update
;Purpose:	Periodically update the Listener for change in Position, Velocity, etc
Method SoundSystem.Update,uses esi
	SetObject esi
	invoke alListenerfv,AL_POSITION,    addr [esi].vPosition
	invoke alListenerfv,AL_VELOCITY,    addr [esi].vVelocity
	invoke alListenerfv,AL_ORIENTATION, addr [esi].vvOrientation
	invoke alGetError
MethodEnd


; ——————————————————————————————————————————————————————————————————————————————————————————————————
Method SoundSource.Init, uses esi, pOwner, dBufferCount:SDWORD, bStreaming:BOOL
LOCAL buffer
	SetObject esi
	ACall Init, pOwner
	m2m [esi].bStreaming,bStreaming,edx
	invoke alGenSources,1, addr [esi].Source
	.if $invoke(alGetError) != AL_NO_ERROR
		return AL_FALSE
	.endif
	
	invoke alSourcef ,[esi].Source, AL_PITCH,    r4_1
	invoke alSourcef ,[esi].Source, AL_GAIN,     r4_1

	.if bStreaming
		OCall [esi].Buffers::DwordCollection.Init,esi,16,256,-1
		.if dBufferCount<2			;Sanity Check:
			mov dBufferCount,2		;Need min. 2 buffers for streaming (ie double buffering)
		.endif
		xor ebx,ebx
		.while ebx<dBufferCount
			invoke alGenBuffers,1, addr buffer
			.if $invoke(alGetError) != AL_NO_ERROR
				return AL_FALSE
			.endif
			OCall [esi].Buffers::DwordCollection.Insert,buffer
			inc ebx
		.endw
	.else
		invoke alGenBuffers,1, addr [esi].Buffer
		.if $invoke(alGetError) != AL_NO_ERROR
			return AL_FALSE
		.endif
	.endif
	OCall esi.Update

MethodEnd

Method SoundSource.Done, uses esi
	SetObject esi
	invoke alDeleteSources,1, addr [esi].Source
	.if [esi].bStreaming==FALSE
		invoke alDeleteBuffers,1, addr [esi].Buffer
	.else
		invoke alDeleteBuffers,[esi].Buffers.dCount, addr [esi].Buffers.pItems
		OCall [esi].Buffers::DwordCollection.Done		
	.endif
	OCall esi.UnMapFile	
MethodEnd

;Tell this SoundSource which Buffer it should be using
;This method is only required for 'static' (non-streaming) playback
Method SoundSource.Bind_SoundBuffer, uses esi, bLooping:BOOL
	SetObject esi
	;Set source attributes
	invoke alSourcei ,[esi].Source, AL_BUFFER,   [esi].Buffer   
	invoke alSourcei ,[esi].Source, AL_LOOPING,  bLooping     
	;Set source position/velocity
	OCall esi.Update

	; Do another error check and return.
	.if $invoke(alGetError) == AL_NO_ERROR
		mov eax, AL_TRUE
	.else
		mov eax, AL_FALSE
	.endif
MethodEnd

;We can use this to check if a sound has finished playing (lazy method)
;returns something like AL_PLAYING
Method SoundSource.GetState
LOCAL res
	SetObject EDX
	invoke alGetSourcei,[edx].Source, AL_SOURCE_STATE, addr res
	mov eax,res
MethodEnd

;Method:	SoundSource.Update
;Purpose:	Update the audio emitter periodically for changes in its position/velocity
Method SoundSource.Update, uses esi
	SetObject esi
	invoke alSourcefv,[esi].Source, AL_POSITION, addr [esi].vPosition
	invoke alSourcefv,[esi].Source, AL_VELOCITY, addr [esi].vVelocity
	invoke alGetError
MethodEnd

;Method:	SoundSource.Controls
;Purpose:	Control the SoundSource
;Args:		dAction = 0 (Play), 1 (Pause), 2 (Rewind), 3(Stop)
Method SoundSource.Controls, uses esi, dAction
LOCAL buffer
	SetObject esi
	mov eax,dAction
	.if eax==SOUND_PLAY
		;If the source is Playing, do nothing
		.if $OCall(esi.GetState)==AL_PLAYING
	        return TRUE
    	.endif
     	.if [esi].bStreaming==TRUE
		    invoke alSourceQueueBuffers,[esi].Source, [esi].Buffers.dCount, [esi].Buffers.pItems 
			.if $invoke(alGetError) != AL_NO_ERROR
				DbgWarning "Error during QueueBuffers"
				return AL_FALSE
			.else
				DbgWarning "Buffers Queued"
			.endif
		.else
			OCall esi.Bind_SoundBuffer,FALSE ;Bind the SoundBuffer to the Source
		.endif

    	invoke alSourcePlay,[esi].Source
    	.if [esi].bStreaming==TRUE
			mov edx,[esi].pOwner
			OCall [edx].SoundSystem.StreamSources_Playing::DwordCollection.Insert,esi
    	.endif	
    	mov [esi].bDataExhausted,FALSE

	.elseif eax==SOUND_PAUSE
		invoke alSourcePause,[esi].Source
		
	.elseif eax==SOUND_REWIND
		;If the source is Playing, stop it
		.if $OCall(esi.GetState)==AL_PLAYING
			OCall esi.Controls,SOUND_STOP
		.endif
		invoke alSourceRewind,[esi].Source
		
	.else ;eax==SOUND_STOP
		;If the source is Playing, stop it
		.if $OCall(esi.GetState)==AL_PLAYING
			DbgWarning "STOPPING"
			invoke alSourceStop,[esi].Source
			mov [esi].dStreamOffset,0
			mov edx,[esi].pOwner
			OCall [edx].SoundSystem.StreamSources_Playing::DwordCollection.Delete,esi
		.endif
		.repeat	;Dequeue any queued buffers
			invoke alSourceUnqueueBuffers,[esi].Source,1,addr buffer
			invoke alGetError
			.break .if eax!=AL_NO_ERROR				
		.until 0
	.endif
	.if $invoke(alGetError) == AL_NO_ERROR
		mov eax, AL_TRUE
	.else
		mov eax, AL_FALSE
	.endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
;Method:    SoundSource.Map_WAV_File
;Purpose:   Load and Interrogate a WAV file for all it is worth - we need to know !!
Method SoundSource.Map_WAV_File,uses esi,pszWAVFile
local hFile,hFileMapping,pRIFFBytes,lpWaveData, pwfxFormat
local dwNumberOfBytesToRead
    SetObject esi

    ; Attempt to open the .wav file using memory mapped functions
    OCall esi.MapFile, pszWAVFile, addr hFile,addr hFileMapping,addr pRIFFBytes
    .if eax==FALSE
      DbgWarning "Error - failed to Map File (wav)"
      xor eax,eax
      ret
    .endif

    ;Parse the WAV Header
    OCall esi.Parse_WAV_Header, pRIFFBytes, addr pwfxFormat,  addr lpWaveData, addr dwNumberOfBytesToRead
	mov edx,pwfxFormat
	.if [edx].WAVEFORMATEX.wBitsPerSample==8
        .if [edx].WAVEFORMATEX.nChannels== 1
            mov eax,AL_FORMAT_MONO8
        .else ;if channels == 2
            mov eax,AL_FORMAT_STEREO8
    	.endif
    .else;if [edx].WAVEFORMATEX.wBitsPerSample==16
        .if [edx].WAVEFORMATEX.nChannels == 1
            mov eax,AL_FORMAT_MONO16
        .else;if [edx].WAVEFORMATEX.nChannels==2
            mov eax,AL_FORMAT_STEREO16
        .endif
    .endif
    mov [esi].format,eax
	m2m [esi].freq,	[edx].WAVEFORMATEX.nSamplesPerSec, eax    
	m2m [esi].pFileBase,	 pRIFFBytes, 			edx
	m2m [esi].pPCMData, 	lpWaveData, 			edx
	m2m [esi].dFileData_Size,dwNumberOfBytesToRead,	edx
	m2m [esi].hFileMapping,  	hFileMapping, 			edx
	m2m [esi].hFile, 	hFile, 					edx
	DbgDec [esi].freq
	DbgDec [esi].dFileData_Size

    mov eax,dwNumberOfBytesToRead
    mov edx, pwfxFormat
    .if [edx].WAVEFORMATEX.nAvgBytesPerSec <= eax
     ; mov [esi].bStreaming,TRUE
      m2m [esi].dBytesPerSecond, [edx].WAVEFORMATEX.nAvgBytesPerSec, eax
   ; .else
      ;mov [esi].bStreaming,FALSE
    .endif

    mov eax,edx
MethodEnd


Method SoundSource.Map_MP3_File,uses edi esi,pszMP3File
    local hFile, hFileMapping, pAudioData
    local dwNumberOfBytesToBuffer, dwNumberOfBytesToRead
    local pDSBuffer

    SetObject esi
    DbgWarning "Map_MP3"
    DbgHex esi
    ;Has MP3 Streaming Support been initialized yet?
    .if [esi].ACMStreamHandle==-1
      DbgWarning "StreamBuffer.Load_MP3_File is initializing MP3 Support (you forgot to?)"
      OCall esi.Init_MP3_Streaming_Support,addr Preferred_Wave_Format_For_MP3_PlayBack
    .endif
    ;Did we find an audio codec that we can use?
    .if [esi].ACMStreamHandle==0
      DbgWarning "Error - failed to open MP3 Stream - no codec"
      xor eax,eax
      ret
    .endif
    ; Attempt to open the .mp3 file using memory mapped functions
    OCall esi.MapFile, pszMP3File, addr hFile,addr hFileMapping,addr pAudioData
    .if eax==FALSE
      DbgWarning "Error - failed to Map File (mp3)"
      DbgStr pszMP3File
      xor eax,eax
      ret
    .endif
    
    ; find out how big the source mp3 file is - IMPORTANT, DO NOT OMIT
    mov dwNumberOfBytesToRead,$invoke (GetFileSize,hFile,0)

    ;Parse the data for Header/Footer and adjust FileSize
    ;We should end up pointing at the first MP3 Frame
    OCall esi.Parse_MP3_Header,pAudioData,addr pAudioData,addr dwNumberOfBytesToRead
   ; shl dwNumberOfBytesToRead,1 ;HACK HACK HACK
    DbgDec dwNumberOfBytesToRead,"amount of compressed data"
    
    m2m [esi].freq,	Preferred_Wave_Format_For_MP3_PlayBack.nSamplesPerSec  
    
    ;Probe the first MP3 Frame for useful information (#channels, bitrate etc)    
    OCall esi.Parse_MP3_FrameHeader,pAudioData,addr [esi].dCompressed_FrameSize,addr Preferred_Wave_Format_For_MP3_PlayBack.nAvgBytesPerSec
    DbgDec edx,"channels"
    .if edx==1
    	mov [esi].format,AL_FORMAT_MONO16
    	
    .else
    	mov [esi].format,AL_FORMAT_STEREO16	
	;shr [esi].freq,1
    .endif




    
    ; find out how much WAV buffer we need for EACH BLOCK of decompressed MP3 data
    ;Given a suggested size of MP3 buffer, determine size of WAV buffer
    invoke acmStreamSize, [esi].ACMStreamHandle,2*MP3_BLOCK_SIZE, addr [esi].dPCM_FrameSize, ACM_STREAMSIZEF_SOURCE
    ;Given a known good size for WAV buffer, determine best size of MP3 buffer - IT MATTERS
    invoke acmStreamSize, [esi].ACMStreamHandle, [esi].dPCM_FrameSize, addr [esi].dCompressed_FrameSize, ACM_STREAMSIZEF_DESTINATION	

	m2m [esi].pPCMData,	 pAudioData, 			edx
	m2m [esi].hFileMapping,  	hFileMapping, 			edx
	m2m [esi].hFile, 	hFile, 					edx
    m2m [esi].dFileData_Size,dwNumberOfBytesToRead,edx
    
	mov eax,edx
MethodEnd

;Method:    SoundSource.Parse_MP3_Header
;Purpose:   Parses the header of an MPEG file
;           It removes ID3 top and botton tags (if any) from our sight,
;           by adjusting the file data pointer and file size.
Method SoundSource.Parse_MP3_Header,uses esi edi ebx,pFileData,ppAudioData,pSize
    SetObject esi
    
    ;Check for and strip Footer if any
    mov eax,pFileData
    mov edx,pSize
    DbgDec dword ptr[edx],"before adjust"
    add eax,dword ptr[edx]
    sub eax,128
    .if word ptr [eax]=='AT' && byte ptr[eax+2]=='G'
      DbgWarning "MP3 Footer Stripped"
      mov edx,pSize
      sub dword ptr[edx],128
    .endif

    mov edi,pFileData
    .if byte ptr[edi]!=0FFh
      ;There may be an ID3 Tag to skip past
      .if word ptr[edi]=='DI' && byte ptr[edi+2]=='3'   ;ID3 tag?
        DbgWarning "ID3v2 Tag Detected"
        
        movzx eax,byte ptr[edi+3]
        DbgDec eax,"ID3v2 Major Version"
        movzx eax,byte ptr[edi+4]
        DbgDec eax,"ID3v2 Minor Version"
        
        movzx eax,byte ptr[edi+5]
        .ifBitSet eax,BIT07
          DbgWarning "UNSYNC bit set"
        .endif
        .ifBitSet eax,BIT06
          DbgWarning "ID3V2 Header Extension present"

        .endif
       ; .ifBitSet eax,BIT04                    Not defined, bits 0-4 should be Clear
       ;   DbgWarning "Footer is present"
       ; .endif

        ;Get Size of ID3 Tag - its encoded in a dword
        mov eax,dword ptr[edi+6]
        bswap eax

        ;The ID3v2 tag size is encoded with four bytes where the most 
        ;significant bit (bit 7) is set to zero in every byte, making a total of 28 bits. 
        ;The zeroed bits are ignored, so a 257 bytes long tag is represented as $00 00 02 01.
    	movzx edx,ax    ;0xxxxxxx 0xxxxxxx 0xxxxxxx 0xxxxxxx
    	shr eax,15      ;00000000 00000000 xxxxxxx0 xxxxxxx0
    	shl dx,1        ;00000000 00000000 xxxxxxx0 xxxxxxx0
    	shr ah,1        ;00000000 00000000 0xxxxxxx xxxxxxx0
    	shr dh,1        ;00000000 00000000 0xxxxxxx xxxxxxx0
    	shr ax,1        ;00000000 00000000 00xxxxxx xxxxxxxx
    	shr dx,1        ;00000000 00000000 00xxxxxx xxxxxxxx
    	;cwde
    	shl eax,14      ;xxxxxxxx xxxxxx00 00000000 00000000
    	or eax,edx     
    	DbgBin eax   
    	DbgHex eax
        
 ;       DbgBin edx

        add edi,eax
        DbgDec eax,"size of ID3 header"

        ;Although we SHOULD be pointing at an MP3 Frame Header,
        ;Sometimes we'll find that an MP3 File is 'BROKEN'...
        ;Scan for the first MP3 Sync Marker.
        ;Its a 255 byte, followed by a byte with its three (or four) MSB's set
        xor ebx,ebx
        .repeat
        .if byte ptr [edi]==255
            mov dl,byte ptr[edi+1]
            and dl,11110000b
            .break .if dl==11110000b
        .endif
        inc eax
        inc edi
        inc ebx
        .until 0
        DbgDec ebx,"Bytes Skipped to find Sync Marker"
        

        mov edx,ppAudioData
        mov dword ptr[edx],edi
        mov edx,pSize
        sub dword ptr[edx],eax
      .endif
    .endif
MethodEnd

;Method:    SoundSource.Parse_MP3_FrameHeader
;Purpose:   Parses the Header of an MPEG Audio Frame
;           to make sure that nothing FUNKY is going on, 
;           eg variable quality, or padding weirdness.
;Arguments: pFileData = pointer to mapped file data
;           pSize = ptr to dword to receive size of Frame
;Returns:   TRUE/FALSE (success/failed), EDX = mono/stereo (1 or 2)
Method SoundSource.Parse_MP3_FrameHeader,uses edi ebx, pFileData,pSize, pBitRate
  local version,layer,bitrate,samplerate
  local header,crc:BOOL
  mov edi,pFileData

  ;Grab the MPEG Audio Frame Header , just a dword
  mov eax,[edi]
  bswap eax
  mov header,eax


  mov edx,eax
  and edx, 11111111111000000000000000000000b
  .if edx!=11111111111000000000000000000000b
    DbgWarning "Error - Resynchronizing"
    ;Although we SHOULD be pointing at an MP3 Frame Header,
    ;Sometimes we'll find that an MP3 File is 'BROKEN'...
    ;Scan for the first MP3 Sync Marker.
    ;Its a 255 byte, followed by a byte with its three (or four) MSB's set
    xor ebx,ebx
    .repeat
    .if byte ptr [edi]==255
        mov dl,byte ptr[edi+1]
        and dl,11110000b
        .break .if dl==11110000b
    .endif
    inc eax
    inc edi
    inc ebx
    .until 0
    DbgDec ebx,"Bytes Skipped to Rediscover Sync Marker" 
    
  .endif
    shr eax,19
    and al,3
    .if al==0
  ;    DbgWarning "MPEG v2.5"
      mov version,25
    .elseif al==1
  ;    DbgWarning "Error - reserved MPEG Version"
      return FALSE
    .elseif al==2
  ;    DbgWarning "MPEG v2"
      mov version,2
    .else
  ;    DbgWarning "MPEG v1"
      mov version,1
    .endif

    mov eax,header
    shr eax,17
    and al,3
    .if al==0
      DbgWarning "Error - reserved MPEG Layer descriptor"
      return FALSE
    .elseif al==1
      mov layer,3
    .elseif al==2
      mov layer,2
    .else
      mov layer,1
    .endif
  ;  DbgDec layer,"MPEG LAYER"

    mov eax,header
    .ifBitSet eax,BIT16
  ;    DbgWarning "CRC Protected"
      mov crc,TRUE
    .else
  ;    DbgWarning "NO CRC Protection"
      mov crc,FALSE
    .endif

    ;find the bitrate, in Hertz
    mov edx,header
    shr edx,12
    and edx,0Fh
    shl edx,1
    .if version==1
      .if layer==1
        lea eax,v1L1
      .elseif layer==2
        lea eax,v1L2
      .else
        lea eax,v1L3
      .endif
    .elseif version==2
      .if layer==1
        lea eax,v2L1
      .else
        lea eax,v2L2L3
      .endif
    .endif
    add edx,eax
    
    mov eax,edx
    shl eax,7
    push eax
    mov eax,pBitRate
    pop [eax]
    
    mov eax,1000
    movzx edx,word ptr[edx]
    mul edx
    mov bitrate,eax
    DbgDec bitrate,"Hz"
  
  

    ;Find the sample rate
    mov eax,header
    shr eax,10
    and eax,3
    .if al==0
      .if version==1
        mov eax,44100
      .elseif version==2
        mov eax,22050
      .else
        mov eax,11025
      .endif
    .elseif al==1
      .if version==1
        mov eax,48000
      .elseif version==2
        mov eax,24000
      .else
        mov eax,12000
      .endif
    .elseif al==2
      .if version==1
        mov eax,32000
      .elseif version==2
        mov eax,16000
      .else
        mov eax,8000
      .endif
    .else
      DbgWarning "Error - reserved samplerate"
    .endif
    mov samplerate,eax
  ;  DbgDec samplerate

    .if layer==1
      mov eax,12
      mul bitrate
      xor edx,edx
      div samplerate
      .ifBitSet header,BIT09
  ;      DbgWarning "padded"
        add eax,4
      .endif
      shl eax,2

    .else
      mov eax,144
      mul bitrate
      xor edx,edx
      div samplerate
      .ifBitSet header,BIT09
  ;      DbgWarning "padded"
        inc eax
      .endif

    .endif

    mov edx,pSize
    mov [edx],eax
  ;  DbgDec eax,"Calculated FrameSize"
  
  
	;Return edx = channel mode
	;0 - Stereo
	;1 - Joint stereo (Stereo)
	;2 - Dual channel (Stereo)
	;3 - Single channel (Mono)  	

  mov edx,header
  shr edx,6
  and edx,3
  .if edx==3
  	mov edx,1	;mono
  .else
  	mov edx,2	;stereo
  .endif

    mov eax,TRUE


MethodEnd

;Method:    SoundSource.Parse_WAV_Header
;Purpose:   Find start of PCM WAV data, its length, and other good stuff
Method SoundSource.Parse_WAV_Header,uses esi, pRIFFBytes, ppwfmx,  ppChunkData, pCkSize
    local pdwChunkTailPtr;     Points to the end of the chunk
    local dwChunkID;       4 byte chunk ID
    local dwType;         Form type
    local dwLength;         Size of data in the chunk

    mov edx,ppwfmx
    mov dword ptr[edx],0    ; Initialize the WAVEFORMATEX structure
    mov edx,ppChunkData
    mov dword ptr[edx],0    ; Initialize the chunk data pointer
    mov edx,pCkSize
    mov dword ptr[edx],0    ; Initialize the chunk size pointer


    mov esi,  pRIFFBytes        ; Reference the WAVE resource buffer
    m2m dwChunkID , dword ptr[esi]    ; Unpack the chunk ID
    add esi,4
    m2m dwLength ,dword ptr[esi]    ; Unpack the size field
    add esi,4
    m2m dwType ,dword ptr[esi]      ; Unpack the form field
    add esi,4

    .if dwChunkID != 'FFIR'        ; Check the Chunk ID
      DbgWarning "Error - Not a RIFF"
      return FALSE
    .endif
    DbgWarning "RIFF Header located"

    ; Is this a wave file?
    .if dwType != 'EVAW'
      DbgWarning "Error - Not a WAV"
      return FALSE
    .endif
    DbgWarning "File appears to be WAV"

    ; Set the tail to point to the end of the wav file
    mov eax,esi
    add eax,dwLength
    sub eax,4
    mov pdwChunkTailPtr,eax

    .while TRUE
      ; Unpack the form type
      m2m dwType ,dword ptr[esi]
      add esi,4
      ; Unpack the size
      m2m dwLength ,dword ptr [esi]
      add esi,4
      mov eax,dwType
      .if eax == " tmf"
        DbgWarning "WAV FORMAT located"
        mov edx,ppwfmx
        .if dword ptr[edx]==0
          .if dwLength < sizeof WAVEFORMAT
            DbgWarning "Error - Still not a WAV"
            return FALSE
          .endif
          mov edx,ppwfmx
          mov [edx],esi
          mov edx,ppChunkData
          .if dword ptr[edx]!=0
            mov edx,pCkSize
            .if  dword ptr[edx]==0
              return TRUE
            .endif
          .endif
        .endif

      .elseif eax=="atad"
        DbgWarning "WAV DATA located"

          .if ppChunkData !=0
            mov edx,ppChunkData
            mov dword ptr [edx] , esi
            add esi,4
          .endif
          .if pCkSize!=0
            mov edx,pCkSize
            m2m [edx], dwLength, eax
            DbgDec dwLength,"of WAV data chunk"
          .endif
          mov edx,ppwfmx
          .if dword ptr[edx]!=0
            return TRUE
          .endif

      .endif
      mov eax,dwLength
      inc eax
      and eax,-2
      add esi,eax

      .break .if esi >= pdwChunkTailPtr

    .endw
    DbgWarning "Unable to parse wav file"
    mov eax,FALSE
MethodEnd

;Method:    SoundSource.MapFile
;Purpose:   Opens a File using FileMapping for 'bufferless' read access to the file's data
Method SoundSource.MapFile,uses esi, pszFile:LPSTR,phFile,phFM,phFV

    .if $invoke (CreateFile, pszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, 0)==0
      DbgWarning "Error - failed to open File"
      DbgStr pszFile
      return FALSE
    .endif
    mov edx,phFile
    mov [edx],eax

    .if $invoke (CreateFileMapping, eax, NULL, PAGE_READONLY, 0, 0, NULL) ==0
      mov edx,phFile
      invoke CloseHandle, dword ptr[edx]
      DbgWarning "Error - failed to create FileMapping"
      DbgStr pszFile
      return FALSE
    .endif
    mov edx,phFM
    mov [edx],eax

    .if $invoke (MapViewOfFile, eax, FILE_MAP_READ, 0, 0, 0)==0
      mov edx,phFM
      invoke CloseHandle, dword ptr[edx]
      mov edx,phFile
      invoke CloseHandle, dword ptr[edx]
      DbgWarning "Error - failed to Map File"
      DbgStr pszFile
      return FALSE
    .endif
    mov edx,phFV
    mov [edx],eax
    mov eax,TRUE
MethodEnd

;Method:	SoundSource.UnMapFile
;Purpose:	Release FileMapping objects
Method SoundSource.UnMapFile, uses esi
	SetObject esi
    invoke UnmapViewOfFile, [esi].pFileBase
    invoke CloseHandle, 	[esi].hFileMapping
    invoke CloseHandle, 	[esi].hFile	
MethodEnd

;Method:	SoundSource.Load_WAV_File
;Purpose:	Load WAV into this SoundSource's (only) buffer
Method SoundSource.Load_WAV_File_Static, uses esi, pszName:LPSTR
	SetObject esi	
	OCall esi.Map_WAV_File, pszName
	.if eax!=0
		invoke alBufferData,[esi].Buffer, [esi].format, [esi].pPCMData, [esi].dFileData_Size, [esi].freq
		.if $invoke (alGetError)==AL_NO_ERROR
			mov eax,TRUE
		.else
			xor eax,eax
		.endif
	.else
		DbgWarning "Failed to Map WAV"
	.endif
MethodEnd

;Method:	SoundSource.Load_WAV_File
;Purpose:	Load WAV into this SoundSource's (only) buffer
Method SoundSource.Load_WAV_File_Stream, uses esi, pszName:LPSTR
	SetObject esi	
	OCall esi.Map_WAV_File, pszName
	.if eax!=0
		.if ![esi].bStreaming
			DbgWarning "StreamBuffer Warning: WAV is less than one second long - should use a SoundBuffer"
			int 3
		.endif
		
		xor ebx,ebx
		.while ebx<[esi].Buffers.dCount
			OCall esi.stream, $OCall([esi].Buffers::DwordCollection.ItemAt,ebx)			
			inc ebx
			DbgDec ebx,"Buffers Loaded"
		.endw
		
		.if $invoke (alGetError)==AL_NO_ERROR
			mov eax,TRUE
		.else
			xor eax,eax
		.endif
	.else
		DbgWarning "Failed to Map WAV"
	.endif
MethodEnd

;Method:	SoundSource.Load_WAV_File
;Purpose:	Load WAV into this SoundSource's (only) buffer
Method SoundSource.Load_MP3_File, uses esi, pszName:LPSTR
	SetObject esi	
	DbgWarning "Load MP3"
	DbgHex esi
	OCall esi.Map_MP3_File, pszName
	.if eax!=0
		mov [esi].bCompressed,TRUE
		OCall esi.Init_Decompression

		xor ebx,ebx
		.while ebx<[esi].Buffers.dCount
			OCall esi.stream, $OCall([esi].Buffers::DwordCollection.ItemAt,ebx)			
			inc ebx
			DbgDec ebx,"Buffers Loaded"
		.endw
		
		.if $invoke (alGetError)==AL_NO_ERROR
			mov eax,TRUE
		.else
			xor eax,eax
		.endif
		
	.else
	
		DbgWarning "Failed to Map MP3"
		
	.endif
MethodEnd

;Method:   SoundSource.Init_MP3_Streaming_Support 
;Purpose:  Attempt to initialize an "MP3 to PCM/WAV" streaming object
;          which will be shared by any/all Sounds which require it.
;Returns:  TRUE/FALSE
Method SoundSource.Init_MP3_Streaming_Support,uses esi,pwfx
    local pwfxIN:ptr MPEGLAYER3WAVEFORMAT
    local maxFormatSize

    SetObject esi
    
    DbgWarning "Attempting to initialize MP3 Support"
    .if [esi].ACMStreamHandle!=-1
        DbgWarning "MP3 Streaming Support already initialized"
        return FALSE
    .endif

    ; try to find an MP3 codec
    mov [esi].Desired_Codec_Tag,WAVE_FORMAT_MPEGLAYER3  
    mov [esi].bHave_Desired_Codec,FALSE    
    invoke acmDriverEnum, addr Find_Desired_Codec, esi, 0
    .if [esi].bHave_Desired_Codec == FALSE
      DbgWarning "ERROR: No MP3 decoders found"
      return FALSE
    .endif

    ; find the biggest format size
    mov maxFormatSize , 0
    invoke acmMetrics, NULL, ACM_METRIC_MAX_SIZE_FORMAT, addr maxFormatSize

    invoke acmStreamOpen,addr [esi].ACMStreamHandle,      ; open an ACM conversion stream
    NULL,                             ; querying all ACM drivers
    addr Default_Wave_Format_For_MP3_Input,         ; converting from MP3
    addr Preferred_Wave_Format_For_MP3_PlayBack,    ; to WAV
    NULL,                        ; with no filter
    0,                          ; or async callbacks
    0,                          ; (and no data for the callback)
    0                          ; and no flags

    push eax
    MemFree pwfxIN
    pop eax

    .if eax== MMSYSERR_NOERROR
     DbgWarning "MP3 to WAV stream ready"



     return TRUE
    .elseif eax== MMSYSERR_INVALPARAM
       DbgWarning "Invalid parameters passed to acmStreamOpen"
    .elseif eax== ACMERR_NOTPOSSIBLE
       DbgWarning "No ACM filter found capable of decoding MP3"
    .else
       DbgWarning "Some error opening ACM decoding stream"
    .endif
    mov eax, FALSE
MethodEnd

; Prepare decompression stream header with values obtained from parsing mp3 first frame
Method SoundSource.Init_Decompression, uses esi
	SetObject esi	
    mov [esi].pFrame_PCM,		$MemAlloc([esi].dPCM_FrameSize)
    mov [esi].pFrame_Compressed,$MemAlloc([esi].dCompressed_FrameSize)    
    invoke RtlZeroMemory,addr [esi].ACMStreamHeader, sizeof ACMSTREAMHEADER

    mov [esi].ACMStreamHeader.cbStruct, sizeof ACMSTREAMHEADER
    m2m [esi].ACMStreamHeader.pbSrc, [esi].pFrame_Compressed, edx
    m2m [esi].ACMStreamHeader.cbSrcLength , [esi].dCompressed_FrameSize,edx
    m2m [esi].ACMStreamHeader.pbDst ,[esi].pFrame_PCM, edx
    m2m [esi].ACMStreamHeader.cbDstLength , [esi].dPCM_FrameSize, edx
    invoke acmStreamPrepareHeader, [esi].ACMStreamHandle, addr [esi].ACMStreamHeader, 0
    .if eax==0
        DbgWarning "MP3 Decompression ConversionStream Header prepared"
    .else
        DbgWarning "FAILED to prepare MP3 Decompression ConversionStream Header"
        .if eax==MMSYSERR_INVALFLAG
            DbgWarning "Invalid FLAG"
        .elseif eax==MMSYSERR_INVALHANDLE
            DbgWarning "Invalid Handle"
        .elseif eax==MMSYSERR_INVALPARAM
            DbgWarning "Invalid Param"
        .else
            DbgWarning "(I dont know why)"
        .endif
    .endif
MethodEnd

;Method:	SoundSource.stream
;Purpose:	Fill given buffer with PCM audio data
Method SoundSource.stream,uses esi, buffer
	SetObject esi
	.if [esi].bCompressed==FALSE
		mov edx,[esi].dStreamOffset			;Have we got enough data left for a full buffer?
		add edx,[esi].dBytesPerSecond
		.if edx>[esi].dFileData_Size
			mov edx,[esi].dFileData_Size		;No - calculate whats left
			sub edx,[esi].dStreamOffset
		.else
			mov edx,[esi].dBytesPerSecond	;Yes - use default buffer size
		.endif	
		mov eax,[esi].pPCMData				;Obtain ptr to source data = base + cursor
		add eax,[esi].dStreamOffset
		add [esi].dStreamOffset,edx			;<-- Update Cursor

	.else
		mov edx,[esi].dStreamOffset			;Have we got enough data left for a full buffer?
		add edx,[esi].dCompressed_FrameSize
		.if edx>[esi].dFileData_Size
			mov edx,[esi].dFileData_Size		;No - calculate whats left
			sub edx,[esi].dStreamOffset
			mov [esi].ACMStreamHeader.cbSrcLength,edx
			.if eax==0
				;We have run out of MP3 data - theres nothing to decompress
				DbgWarning "MP3 Data Exhausted"
				mov [esi].bDataExhausted,TRUE				
				return FALSE	;This will tell the stream method not to requeue the buffer
			.endif

			;Grab chunk of Compressed audio
			mov eax,[esi].pPCMData				;Obtain ptr to source data = base + cursor
			add eax,[esi].dStreamOffset
			invoke RtlMoveMemory,[esi].pFrame_Compressed,eax,[esi].ACMStreamHeader.cbSrcLength
		
			;Decompress audio into PCM format
			invoke acmStreamConvert,[esi].SoundSource.ACMStreamHandle,addr [esi].ACMStreamHeader,ACM_STREAMCONVERTF_END
			
		.else
			;Grab chunk of Compressed audio
			mov eax,[esi].pPCMData				;Obtain ptr to source data = base + cursor
			add eax,[esi].dStreamOffset
			invoke RtlMoveMemory,[esi].pFrame_Compressed,eax,[esi].ACMStreamHeader.cbSrcLength
			;Decompress audio into PCM format
			invoke acmStreamConvert,[esi].ACMStreamHandle,addr [esi].ACMStreamHeader,ACM_STREAMCONVERTF_BLOCKALIGN
		.endif	
		mov eax,[esi].ACMStreamHeader.cbSrcLengthUsed
		add [esi].dStreamOffset,eax
		
		mov edx,[esi].ACMStreamHeader.cbDstLengthUsed
		mov eax,[esi].pFrame_PCM
	.endif
	
	;Copy PCM audio data into Buffer
	invoke alBufferData,buffer,[esi].format, eax,edx, [esi].freq	 ;Copy data to buffer
	.if $invoke (alGetError)==AL_NO_ERROR	;Check for error
		mov eax,TRUE
	.else
		mov eax,FALSE
	.endif	

MethodEnd

;Method:	SoundSource.Refill
;Purpose:	Refill and re-queue any buffers which have finished playing
Method SoundSource.Refill, uses esi
LOCAL buffer, processed,active:BOOL
LOCAL src
	SetObject esi
    mov active,TRUE
    m2m src,[esi].Source,eax
    invoke alGetSourcei,src, AL_BUFFERS_PROCESSED, addr processed 
    .if processed
        DbgDec processed	
	    .repeat     
	        invoke alSourceUnqueueBuffers,src, 1, addr buffer
			.if $invoke(alGetError) != AL_NO_ERROR
				return FALSE
			.endif
	        or active, $OCall(esi.stream,buffer) 
	        .if eax ;Only requeue the buffer if we succeeded in filling it with PCM data
		        invoke alSourceQueueBuffers,src, 1, addr buffer
				.if $invoke(alGetError) != AL_NO_ERROR
					int 3
					return FALSE
				.endif
			.else
				DbgText "A buffer was dequeued"
			.endif
	        dec processed
	    .until processed==0
 	.endif
 	.if !active
 		OCall esi.Controls,SOUND_STOP
 	.endif
    return active
MethodEnd


; ——————————————————————————————————————————————————————————————————————————————————————————————————
; ——————————————————————————————————————————————————————————————————————————————————————————————————
; ——————————————————————————————————————————————————————————————————————————————————————————————————
; ——————————————————————————————————————————————————————————————————————————————————————————————————
; ——————————————————————————————————————————————————————————————————————————————————————————————————

; Thread to poll refill of streaming buffers
Method SoundSystem.Monitor,uses esi ebx
LOCAL crit:CRITICAL_SECTION
	SetObject esi
	invoke InitializeCriticalSection,addr crit
	DbgWarning "Worker Active","Worker"
	.repeat
		invoke Sleep,200
		invoke EnterCriticalSection,addr crit
		xor ebx,ebx
		.while ebx<[esi].StreamSources_Playing.dCount
			OCall [esi].StreamSources_Playing::DwordCollection.ItemAt,ebx			
			.if eax!=-1
				OCall eax::SoundSource.Refill
			.endif
			inc ebx
		.endw
		invoke LeaveCriticalSection,addr crit
		.if [esi].bShutDown==TRUE
			DbgWarning "Worker Death","Worker"
			mov [esi].bShutDown,FALSE
			invoke DeleteCriticalSection,addr crit
			invoke ExitThread,0
		.endif
	.until 0
	
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; ——————————————————————————————————————————————————————————————————————————————————————————————————
; ——————————————————————————————————————————————————————————————————————————————————————————————————
; ——————————————————————————————————————————————————————————————————————————————————————————————————
; ——————————————————————————————————————————————————————————————————————————————————————————————————

;Procedure: Find_Desired_Codec
;Purpose:   Enumerate codecs to find one capable of decoding desired compression
Find_Desired_Codec proc hadid, dwInstance, fdwSupport
  local details:ACMDRIVERDETAILS
  local driver:Handle
  local fmtDetails:ACMFORMATTAGDETAILS
  .ifBitSet fdwSupport, ACMDRIVERDETAILS_SUPPORTF_CODEC
    mov details.cbStruct , sizeof details
    invoke acmDriverDetails, hadid, addr details, 0
    invoke acmDriverOpen, addr driver, hadid, 0
      xor ecx,ecx
    .while ecx < details.cFormatTags
      push ecx
      invoke RtlZeroMemory, addr fmtDetails, sizeof fmtDetails
      pop ecx
      push ecx
      mov fmtDetails.cbStruct , sizeof fmtDetails
      mov fmtDetails.dwFormatTagIndex , ecx
      invoke acmFormatTagDetails, driver, addr fmtDetails, ACM_FORMATTAGDETAILSF_INDEX
      pop ecx
      mov edx,dwInstance
      mov edx,[edx].SoundSource.Desired_Codec_Tag
      .if fmtDetails.dwFormatTag == edx
        DbgWarning "Found a useable codec: "
        lea eax,details.szLongName
        DbgStr eax
        mov edx,dwInstance
        mov [edx].SoundSource.bHave_Desired_Codec,TRUE
        .break
      .endif
      inc ecx
      .endw
      invoke acmDriverClose,addr driver, 0
  .endif
  ret
Find_Desired_Codec endp

