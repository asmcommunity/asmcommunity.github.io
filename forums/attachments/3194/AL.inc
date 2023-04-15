

ALboolean typedef byte
ALchar typedef byte
ALbyte typedef sbyte
ALubyte typedef byte
ALshort typedef sword
ALushort typedef word
ALint typedef sdword
ALuint typedef dword
ALsizei typedef sdword
ALenum typedef dword
ALfloat typedef real4
ALdouble typedef real8
ALvoid typedef Pointer

AL_INVALID                  equ              -1
AL_NONE                     equ              0
AL_FALSE                    equ              0
AL_TRUE                     equ              1
AL_SOURCE_RELATIVE          equ              0x202
AL_CONE_INNER_ANGLE                    equ   01001h
AL_CONE_OUTER_ANGLE                     equ  01002h
AL_PITCH                            equ      01003h
  
; Specify the current location in three dimensional space.
; OpenAL, like OpenGL, uses a right handed coordinate system,
;  where in a frontal default view X (thumb) points right, 
;  Y points up (index finger), and Z points towards the
;  viewer/camera (middle finger). 
; To switch from a left handed coordinate system, flip the
;  sign on the Z coordinate.
; Listener position is always in the world coordinate system.
AL_POSITION                           equ    01004h
AL_DIRECTION                          equ    01005h
AL_VELOCITY                           equ    01006h
AL_LOOPING                            equ    01007h
AL_BUFFER                             equ    01009h
AL_GAIN                               equ    0100Ah
AL_MIN_GAIN                           equ    0100Dh
AL_MAX_GAIN                           equ    0100Eh
AL_ORIENTATION                        equ    0100Fh
AL_CHANNEL_MASK                       equ    03000h
AL_SOURCE_STATE                       equ    01010h
AL_INITIAL                            equ    01011h
AL_PLAYING                            equ    01012h
AL_PAUSED                             equ    01013h
AL_STOPPED                            equ    01014h
AL_BUFFERS_QUEUED                     equ    01015h
AL_BUFFERS_PROCESSED                  equ    01016h
AL_SEC_OFFSET                         equ    01024h
AL_SAMPLE_OFFSET                      equ    01025h
AL_BYTE_OFFSET                        equ    01026h
AL_SOURCE_TYPE                        equ    01027h
AL_STATIC                             equ    01028h
AL_STREAMING                          equ    01029h
AL_UNDETERMINED                       equ    01030h

AL_FORMAT_MONO8                       equ    01100h
AL_FORMAT_MONO16                      equ    01101h
AL_FORMAT_STEREO8                     equ    01102h
AL_FORMAT_STEREO16                    equ    01103h

AL_REFERENCE_DISTANCE                 equ    01020h
AL_ROLLOFF_FACTOR                     equ    01021h
AL_CONE_OUTER_GAIN                    equ    01022h
AL_MAX_DISTANCE                       equ    01023h
AL_FREQUENCY                          equ    02001h
AL_BITS                               equ    02002h
AL_CHANNELS                           equ    02003h
AL_SIZE                               equ    02004h
AL_UNUSED                             equ    02010h
AL_PENDING                            equ    02011h
AL_PROCESSED                          equ    02012h

AL_NO_ERROR                    equ           AL_FALSE
AL_INVALID_NAME                equ           0A001h
AL_ILLEGAL_ENUM                equ           0A002h
AL_INVALID_ENUM                equ           0A002h
AL_INVALID_VALUE               equ           0A003h
AL_ILLEGAL_COMMAND             equ           0A004h
AL_INVALID_OPERATION           equ           0A004h
AL_OUT_OF_MEMORY               equ           0A005h

AL_VENDOR                        equ         0B001h
AL_VERSION                       equ         0B002h
AL_RENDERER                      equ         0B003h
AL_EXTENSIONS                    equ         0B004h

AL_DOPPLER_FACTOR                 equ        0C000h
AL_DOPPLER_VELOCITY               equ        0C001h
AL_SPEED_OF_SOUND                 equ        0C003h

AL_DISTANCE_MODEL                 equ        0D000h
AL_INVERSE_DISTANCE               equ        0D001h
AL_INVERSE_DISTANCE_CLAMPED       equ        0D002h
AL_LINEAR_DISTANCE                equ        0D003h
AL_LINEAR_DISTANCE_CLAMPED        equ        0D004h
AL_EXPONENT_DISTANCE              equ        0D005h
AL_EXPONENT_DISTANCE_CLAMPED      equ        0D006h

alEnable proto C :ALenum
alDisable  proto C :ALenum 
alIsEnabled  proto C :ALenum
alGetString  proto C :ALenum
alGetBooleanv proto C :ALenum, :Pointer
alGetIntegerv proto C :ALenum, :Pointer
alGetFloatv  proto C :ALenum, :Pointer
alGetDoublev proto C :ALenum, :Pointer
alGetBoolean proto C :ALenum
alGetInteger proto C :ALenum
alGetFloat  proto C :ALenum
alGetDouble  proto C :ALenum


alGetError proto C
alIsExtensionPresent proto C :LPSTR
alGetProcAddress proto C :LPSTR
alGetEnumValue proto C :LPSTR


alListenerf proto C :ALenum ,:ALfloat
alListener3f proto C :ALenum, :ALfloat, :ALfloat, :ALfloat
alListenerfv proto C :ALenum, :Pointer 
alListeneri proto C :ALenum, :ALint 
alListener3i proto C :ALenum, :ALint, :ALint, :ALint 
alListeneriv proto C :ALenum, :Pointer

alGetListenerf proto C :ALenum, :Pointer
alGetListener3f proto C :ALenum, :Pointer, :Pointer, :Pointer
alGetListenerfv proto C :ALenum, :Pointer
alGetListeneri proto C :ALenum, :Pointer
alGetListener3i proto C :ALenum, :Pointer, :Pointer, :Pointer
alGetListeneriv proto C :ALenum, :Pointer

;
;/**
; * SOURCE
; * Sources represent individual sound objects in 3D-space.
; * Sources take the PCM data provided in the specified Buffer,
; * apply Source-specific modifications, and then
; * submit them to be mixed according to spatial arrangement etc.
; * 
; * Properties include: -
; *
; * Gain                              AL_GAIN                 ALfloat
; * Min Gain                          AL_MIN_GAIN             ALfloat
; * Max Gain                          AL_MAX_GAIN             ALfloat
; * Position                          AL_POSITION             ALfloat[3]
; * Velocity                          AL_VELOCITY             ALfloat[3]
; * Direction                         AL_DIRECTION            ALfloat[3]
; * Head Relative Mode                AL_SOURCE_RELATIVE      ALint (AL_TRUE or AL_FALSE)
; * Reference Distance                AL_REFERENCE_DISTANCE   ALfloat
; * Max Distance                      AL_MAX_DISTANCE         ALfloat
; * RollOff Factor                    AL_ROLLOFF_FACTOR       ALfloat
; * Inner Angle                       AL_CONE_INNER_ANGLE     ALint or ALfloat
; * Outer Angle                       AL_CONE_OUTER_ANGLE     ALint or ALfloat
; * Cone Outer Gain                   AL_CONE_OUTER_GAIN      ALint or ALfloat
; * Pitch                             AL_PITCH                ALfloat
; * Looping                           AL_LOOPING              ALint (AL_TRUE or AL_FALSE)
; * MS Offset                         AL_MSEC_OFFSET          ALint or ALfloat
; * Byte Offset                       AL_BYTE_OFFSET          ALint or ALfloat
; * Sample Offset                     AL_SAMPLE_OFFSET        ALint or ALfloat
; * Attached Buffer                   AL_BUFFER               ALint
; * State (Query only)                AL_SOURCE_STATE         ALint
; * Buffers Queued (Query only)       AL_BUFFERS_QUEUED       ALint
; * Buffers Processed (Query only)    AL_BUFFERS_PROCESSED    ALint
; */

alGenSources proto C :ALsizei, :Pointer
alDeleteSources proto C :ALsizei, :Pointer
alIsSource proto C :ALuint 
alSourcef proto C :ALuint, :ALenum, :ALfloat 
alSource3f proto C :ALuint, :ALenum, :ALfloat, :ALfloat, :ALfloat 
alSourcefv proto C :ALuint, :ALenum, :Pointer
alSourcei proto C :ALuint, :ALenum, :ALint 
alSource3i proto C :ALuint, :ALenum, :ALint, :ALint, :ALint 
alSourceiv proto C :ALuint, :ALenum, :Pointer

alGetSourcef proto C :ALuint, :ALenum, :Pointer
alGetSource3f proto C :ALuint, :ALenum, :Pointer, :Pointer, :Pointer
alGetSourcefv proto C :ALuint, :ALenum, :Pointer
alGetSourcei proto C :ALuint,  :ALenum, :Pointer
alGetSource3i proto C :ALuint, :ALenum, :Pointer,:Pointer, :Pointer
alGetSourceiv proto C :ALuint, :ALenum, :Pointer

alSourcePlayv proto C :ALsizei, :Pointer
alSourceStopv proto C :ALsizei, :Pointer
alSourceRewindv proto C :ALsizei, :Pointer
alSourcePausev proto C :ALsizei, :Pointer

alSourcePlay proto C :ALuint
alSourceStop proto C :ALuint
alSourceRewind proto C :ALuint
alSourcePause proto C :ALuint

alSourceQueueBuffers proto C :ALuint, :ALsizei, :Pointer
alSourceUnqueueBuffers proto C :ALuint, :ALsizei, :Pointer

alGenBuffers proto C :ALsizei, :Pointer
alDeleteBuffers proto C :ALsizei, :Pointer
alIsBuffer proto C :ALuint
alBufferData proto C :ALuint, :ALenum, :Pointer, :ALsizei, :ALsizei

alBufferf proto C :ALuint, :ALenum, :ALfloat
alBuffer3f proto C :ALuint, :ALenum, :ALfloat, :ALfloat, :ALfloat
alBufferfv proto C :ALuint, :ALenum, :Pointer
alBufferi proto C :ALuint, :ALenum, :ALint
alBuffer3i proto C :ALuint, :ALenum, :ALint, :ALint, :ALint
alBufferiv proto C :ALuint, :ALenum, :Pointer

alGetBufferf proto C :ALuint, :ALenum, :Pointer
alGetBuffer3f proto C :ALuint, :ALenum, :Pointer, :Pointer, :Pointer
alGetBufferfv proto C :ALuint, :ALenum, :Pointer
alGetBufferi proto C :ALuint, :ALenum, :Pointer
alGetBuffer3i proto C :ALuint, :ALenum, :Pointer, :Pointer, :Pointer
alGetBufferiv proto C :ALuint, :ALenum, :Pointer


alDopplerFactor proto C :ALfloat
alDopplerVelocity proto C :ALfloat
alSpeedOfSound proto C :ALfloat
alDistanceModel proto C :ALenum
