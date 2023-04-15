ALCdevice  typedef ptr ALCdevice_struct
ALCcontext typedef ptr ALCcontext_struct


ALCboolean typedef byte
ALCchar    typedef byte
ALCbyte    typedef sbyte
ALCubyte   typedef byte
ALCshort   typedef sword
ALCushort  typedef word
ALCint	   typedef sdword
ALCuint    typedef dword
ALCsizei   typedef dword
ALCenum    typedef dword
ALCfloat   typedef real4
ALCdouble  typedef real8
ALCvoid    typedef Pointer


ALC_FALSE                      equ          0
ALC_TRUE                       equ          1
ALC_FREQUENCY                  equ          01007h
ALC_REFRESH                    equ          01008h
ALC_SYNC                       equ          01009h
ALC_MONO_SOURCES               equ          01010h
ALC_STEREO_SOURCES             equ          01011h

ALC_NO_ERROR                   equ          ALC_FALSE
ALC_INVALID_DEVICE             equ          0A001h
ALC_INVALID_CONTEXT            equ          0A002h
ALC_INVALID_ENUM               equ          0A003h
ALC_INVALID_VALUE              equ          0A004h
ALC_OUT_OF_MEMORY              equ          0A005h

ALC_DEFAULT_DEVICE_SPECIFIER   equ          01004h
ALC_DEVICE_SPECIFIER           equ          01005h
ALC_EXTENSIONS                 equ          01006h

ALC_MAJOR_VERSION             equ           01000h
ALC_MINOR_VERSION             equ           01001h

ALC_ATTRIBUTES_SIZE             equ         01002h
ALC_ALL_ATTRIBUTES              equ         01003h

ALC_DEFAULT_ALL_DEVICES_SPECIFIER   equ     01012h
ALC_ALL_DEVICES_SPECIFIER           equ     01013h

ALC_CAPTURE_DEVICE_SPECIFIER         equ    0310h
ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER equ    0311h
ALC_CAPTURE_SAMPLES                  equ    0312h

alcCreateContext proto C :Pointer, :Pointer
alcMakeContextCurrent proto C :Pointer
alcProcessContext proto C :Pointer
alcSuspendContext proto C :Pointer
alcDestroyContext proto C :Pointer
alcGetCurrentContext proto C
alcGetContextsDevice proto C :Pointer
alcOpenDevice proto C :Pointer
alcCloseDevice proto C :Pointer
alcGetError proto C :Pointer
alcIsExtensionPresent proto C :Pointer, :Pointer
alcGetProcAddress proto C :Pointer, :Pointer
alcGetEnumValue proto C :Pointer, :Pointer
alcGetString proto C :Pointer, :ALCenum
alcGetIntegerv proto C :Pointer, :ALCenum, :ALCsizei, :Pointer
alcCaptureOpenDevice proto C :Pointer, :ALCuint, :ALCenum, :ALCsizei
alcCaptureCloseDevice proto C :Pointer
alcCaptureStart proto C :Pointer
alcCaptureStop proto C :Pointer
alcCaptureSamples proto C :Pointer, :Pointer, :ALCsizei

