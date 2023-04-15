; ==================================================================================================
; Title:   SAPI.inc
; Author:  G. Friedrich
; Version: 1.0.0
; Purpose: Sound API 5.1 support for ObjAsm32.
; Notes:   Version 1.0.0, December 2005:
;            - First release. Generated using COMView (www.japheth.de), NaN's header files
;              translation and edited manually.
; ==================================================================================================


sTLBID_SpeechLib                  textequ <C866CA3A-32F7-11D2-9602-00C04F8EE628>
_MajorVer_SpeechLib equ 5
_MinorVer_SpeechLib equ 0

LPUNKNOWN equ <LPIUNKNOWN>

sCLSID_SpNotifyTranslator         textequ <E2AE5372-5D40-11D2-960E-00C04F8EE628>
sCLSID_SpObjectTokenCategory      textequ <A910187F-0C7A-45AC-92CC-59EDAFB77B53>
sCLSID_SpObjectToken              textequ <EF411752-3736-4CB4-9C8C-8EF4CCB58EFE>
sCLSID_SpResourceManager          textequ <96749373-3391-11D2-9EE3-00C04F797396>
sCLSID_SpStreamFormatConverter    textequ <7013943A-E2EC-11D2-A086-00C04F8EF9B5>
sCLSID_SpMMAudioEnum              textequ <AB1890A0-E91F-11D2-BB91-00C04F8EE6C0>
sCLSID_SpMMAudioIn                textequ <CF3D2E50-53F2-11D2-960C-00C04F8EE628>
sCLSID_SpMMAudioOut               textequ <A8C680EB-3D32-11D2-9EE7-00C04F797396>
sCLSID_SpRecPlayAudio             textequ <FEE225FC-7AFD-45E9-95D0-5A318079D911>
sCLSID_SpStream                   textequ <715D9C59-4442-11D2-9605-00C04F8EE628>
sCLSID_SpVoice                    textequ <96749377-3391-11D2-9EE3-00C04F797396>
sCLSID_SpSharedRecoContext        textequ <47206204-5ECA-11D2-960F-00C04F8EE628>
sCLSID_SpInprocRecognizer         textequ <41B89B6B-9399-11D2-9623-00C04F8EE628>
sCLSID_SpSharedRecognizer         textequ <3BEE4890-4FE9-4A37-8C1E-5E7E12791C1F>
sCLSID_SpLexicon                  textequ <0655E396-25D0-11D3-9C26-00C04F8EF87C>
sCLSID_SpUnCompressedLexicon      textequ <C9E37C15-DF92-4727-85D6-72E5EEB6995A>
sCLSID_SpCompressedLexicon        textequ <90903716-2F42-11D3-9C26-00C04F8EF87C>
sCLSID_SpPhoneConverter           textequ <9185F743-1143-4C28-86B5-BFF14F20E5C8>
sCLSID_SpNullPhoneConverter       textequ <455F24E9-7396-4A16-9715-7C0FDBE3EFE3>
sCLSID_SpTextSelectionInformation textequ <0F92030A-CBFD-4AB8-A164-FF5985547FF6>
sCLSID_SpPhraseInfoBuilder        textequ <C23FC28D-C55F-4720-8B32-91F73C2BD5D1>
sCLSID_SpAudioFormat              textequ <9EF96870-E160-4792-820D-48CF0649E4EC>
sCLSID_SpWaveFormatEx             textequ <C79A574C-63BE-44B9-801F-283F87F898BE>
sCLSID_SpInProcRecoContext        textequ <73AD6842-ACE0-45E8-A4DD-8795881A2C2A>
sCLSID_SpCustomStream             textequ <8DBEF13F-1948-4AA8-8CF0-048EEBED95D8>
sCLSID_SpFileStream               textequ <947812B3-2AE1-4644-BA86-9E90DED7EC91>
sCLSID_SpMemoryStream             textequ <5FB7EF7D-DFF4-468A-B6B7-2FCBD188F994>


SPF_DEFAULT                 equ 0
SPF_ASYNC                   equ 1
SPF_PURGEBEFORESPEAK        equ 2
SPF_IS_FILENAME             equ 4
SPF_IS_XML                  equ 8
SPF_IS_NOT_XML              equ 16
SPF_PERSIST_XML             equ 32
SPF_NLP_SPEAK_PUNC          equ 64
SPF_NLP_MASK                equ SPF_NLP_SPEAK_PUNC
SPF_VOICE_MASK              equ SPF_ASYNC or SPF_PURGEBEFORESPEAK or SPF_IS_FILENAME or  \
                                SPF_IS_XML or SPF_IS_NOT_XML or SPF_NLP_MASK or SPF_PERSIST_XML
SPF_UNUSED_FLAGS            equ NOT SPF_VOICE_MASK

;User Interface wide strings (UNICODE)
SPDUI_EngineProperties                textequ <"EngineProperties">
SPDUI_AddRemoveWord                   textequ <"AddRemoveWord">
SPDUI_UserTraining                    textequ <"UserTraining">
SPDUI_MicTraining                     textequ <"MicTraining">
SPDUI_RecoProfileProperties           textequ <"RecoProfileProperties">
SPDUI_AudioProperties                 textequ <"AudioProperties">
SPDUI_AudioVolume                     textequ <"AudioVolume">

;SpeechStringConstants Wide strings (UNICODE)
SpeechRegistryUserRoot                textequ <"HKEY_CURRENT_USER\SOFTWARE\Microsoft\Speech">
SpeechRegistryLocalMachineRoot        textequ <"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech">
SpeechCategoryAudioOut                textequ <"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\AudioOutput">
SpeechCategoryAudioIn                 textequ <"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\AudioInput">
SpeechCategoryVoices                  textequ <"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\Voices">
SpeechCategoryRecognizers             textequ <"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\Recognizers">
SpeechCategoryAppLexicons             textequ <"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\AppLexicons">
SpeechCategoryPhoneConverters         textequ <"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\PhoneConverters">
SpeechCategoryRecoProfiles            textequ <"HKEY_CURRENT_USER\SOFTWARE\Microsoft\Speech\RecoProfiles">
SpeechTokenIdUserLexicon              textequ <"HKEY_CURRENT_USER\SOFTWARE\Microsoft\Speech\CurrentUserLexicon">
SpeechTokenValueCLSID                 textequ <"CLSID">
SpeechTokenKeyFiles                   textequ <"Files">
SpeechTokenKeyUI                      textequ <"UI">
SpeechTokenKeyAttributes              textequ <"Attributes">
SpeechVoiceCategoryTTSRate            textequ <"DefaultTTSRate">
SpeechPropertyResourceUsage           textequ <"ResourceUsage">
SpeechPropertyHighConfidenceThreshold textequ <"HighConfidenceThreshold">
SpeechPropertyNormalConfidenceThreshold textequ <"NormalConfidenceThreshold">
SpeechPropertyLowConfidenceThreshold  textequ <"LowConfidenceThreshold">
SpeechPropertyResponseSpeed           textequ <"ResponseSpeed">
SpeechPropertyComplexResponseSpeed    textequ <"ComplexResponseSpeed">
SpeechPropertyAdaptationOn            textequ <"AdaptationOn">
SpeechDictationTopicSpelling          textequ <"Spelling">
SpeechGrammarTagWildcard              textequ <"...">
SpeechGrammarTagDictation             textequ <"*">
SpeechGrammarTagUnlimitedDictation    textequ <"*+">
SpeechEngineProperties                textequ <"EngineProperties">
SpeechAddRemoveWord                   textequ <"AddRemoveWord">
SpeechUserTraining                    textequ <"UserTraining">
SpeechMicTraining                     textequ <"MicTraining">
SpeechRecoProfileProperties           textequ <"RecoProfileProperties">
SpeechAudioProperties                 textequ <"AudioProperties">
SpeechAudioVolume                     textequ <"AudioVolume">
SpeechVoiceSkipTypeSentence           textequ <"Sentence">
SpeechAudioFormatGUIDWave             textequ <"{C31ADBAE-527F-4ff5-A230-F62BB61FF70C}">
SpeechAudioFormatGUIDText             textequ <"{7CEEF9F9-3D13-11d2-9EE7-00C04F797396}">

;SpeechDataKeyLocation enumeration
SpeechDataKeyLocation typedef sdword
SDKLDefaultLocation         equ  0
SDKLCurrentUser             equ  1
SDKLLocalMachine            equ  2
SDKLCurrentConfig           equ  5

;SpeechTokenContext enumeration
SpeechTokenContext typedef sdword
STCInprocServer             equ  1
STCInprocHandler            equ  2
STCLocalServer              equ  4
STCRemoteServer             equ  16      ;=010h
STCAll                      equ  23      ;=017h

;SpeechTokenShellFolder enumeration
SpeechTokenShellFolder typedef sdword
STSF_AppData                equ  26      ;=01Ah
STSF_LocalAppData           equ  28      ;=01Ch
STSF_CommonAppData          equ  35      ;=023h
STSF_FlagCreate             equ  32768   ;=08000h

;SpeechAudioState enumeration
SpeechAudioState typedef sdword
SASClosed                   equ  0
SASStop                     equ  1
SASPause                    equ  2
SASRun                      equ  3

;SpeechAudioFormatType enumeration
SpeechAudioFormatType typedef sdword
SAFTDefault                 equ  -1
SAFTNoAssignedFormat        equ  0
SAFTText                    equ  1
SAFTNonStandardFormat       equ  2
SAFTExtendedAudioFormat     equ  3
SAFT8kHz8BitMono            equ  4
SAFT8kHz8BitStereo          equ  5
SAFT8kHz16BitMono           equ  6
SAFT8kHz16BitStereo         equ  7
SAFT11kHz8BitMono           equ  8
SAFT11kHz8BitStereo         equ  9
SAFT11kHz16BitMono          equ  10
SAFT11kHz16BitStereo        equ  11
SAFT12kHz8BitMono           equ  12
SAFT12kHz8BitStereo         equ  13
SAFT12kHz16BitMono          equ  14
SAFT12kHz16BitStereo        equ  15
SAFT16kHz8BitMono           equ  16      ;=010h
SAFT16kHz8BitStereo         equ  17      ;=011h
SAFT16kHz16BitMono          equ  18      ;=012h
SAFT16kHz16BitStereo        equ  19      ;=013h
SAFT22kHz8BitMono           equ  20      ;=014h
SAFT22kHz8BitStereo         equ  21      ;=015h
SAFT22kHz16BitMono          equ  22      ;=016h
SAFT22kHz16BitStereo        equ  23      ;=017h
SAFT24kHz8BitMono           equ  24      ;=018h
SAFT24kHz8BitStereo         equ  25      ;=019h
SAFT24kHz16BitMono          equ  26      ;=01Ah
SAFT24kHz16BitStereo        equ  27      ;=01Bh
SAFT32kHz8BitMono           equ  28      ;=01Ch
SAFT32kHz8BitStereo         equ  29      ;=01Dh
SAFT32kHz16BitMono          equ  30      ;=01Eh
SAFT32kHz16BitStereo        equ  31      ;=01Fh
SAFT44kHz8BitMono           equ  32      ;=020h
SAFT44kHz8BitStereo         equ  33      ;=021h
SAFT44kHz16BitMono          equ  34      ;=022h
SAFT44kHz16BitStereo        equ  35      ;=023h
SAFT48kHz8BitMono           equ  36      ;=024h
SAFT48kHz8BitStereo         equ  37      ;=025h
SAFT48kHz16BitMono          equ  38      ;=026h
SAFT48kHz16BitStereo        equ  39      ;=027h
SAFTTrueSpeech_8kHz1BitMono equ  40      ;=028h
SAFTCCITT_ALaw_8kHzMono     equ  41      ;=029h
SAFTCCITT_ALaw_8kHzStereo   equ  42      ;=02Ah
SAFTCCITT_ALaw_11kHzMono    equ  43      ;=02Bh
SAFTCCITT_ALaw_11kHzStereo  equ  44      ;=02Ch
SAFTCCITT_ALaw_22kHzMono    equ  45      ;=02Dh
SAFTCCITT_ALaw_22kHzStereo  equ  46      ;=02Eh
SAFTCCITT_ALaw_44kHzMono    equ  47      ;=02Fh
SAFTCCITT_ALaw_44kHzStereo  equ  48      ;=030h
SAFTCCITT_uLaw_8kHzMono     equ  49      ;=031h
SAFTCCITT_uLaw_8kHzStereo   equ  50      ;=032h
SAFTCCITT_uLaw_11kHzMono    equ  51      ;=033h
SAFTCCITT_uLaw_11kHzStereo  equ  52      ;=034h
SAFTCCITT_uLaw_22kHzMono    equ  53      ;=035h
SAFTCCITT_uLaw_22kHzStereo  equ  54      ;=036h
SAFTCCITT_uLaw_44kHzMono    equ  55      ;=037h
SAFTCCITT_uLaw_44kHzStereo  equ  56      ;=038h
SAFTADPCM_8kHzMono          equ  57      ;=039h
SAFTADPCM_8kHzStereo        equ  58      ;=03Ah
SAFTADPCM_11kHzMono         equ  59      ;=03Bh
SAFTADPCM_11kHzStereo       equ  60      ;=03Ch
SAFTADPCM_22kHzMono         equ  61      ;=03Dh
SAFTADPCM_22kHzStereo       equ  62      ;=03Eh
SAFTADPCM_44kHzMono         equ  63      ;=03Fh
SAFTADPCM_44kHzStereo       equ  64      ;=040h
SAFTGSM610_8kHzMono         equ  65      ;=041h
SAFTGSM610_11kHzMono        equ  66      ;=042h
SAFTGSM610_22kHzMono        equ  67      ;=043h
SAFTGSM610_44kHzMono        equ  68      ;=044h

;SpeechStreamSeekPositionType enumeration
SpeechStreamSeekPositionType typedef sdword
SSSPTRelativeToStart            equ  0
SSSPTRelativeToCurrentPosition  equ  1
SSSPTRelativeToEnd              equ  2

;SpeechStreamFileMode enumeration
SpeechStreamFileMode typedef sdword
SSFMOpenForRead             equ  0
SSFMOpenReadWrite           equ  1
SSFMCreate                  equ  2
SSFMCreateForWrite          equ  3

;SpeechRunState enumeration
SpeechRunState typedef sdword
SRSEDone                     equ  1
SRSEIsSpeaking               equ  2

;SpeechVoiceEvents enumeration
SpeechVoiceEvents typedef sdword
SVEStartInputStream         equ  2
SVEEndInputStream           equ  4
SVEVoiceChange              equ  8
SVEBookmark                 equ  16      ;=010h
SVEWordBoundary             equ  32      ;=020h
SVEPhoneme                  equ  64      ;=040h
SVESentenceBoundary         equ  128     ;=080h
SVEViseme                   equ  256     ;=0100h
SVEAudioLevel               equ  512     ;=0200h
SVEPrivate                  equ  32768   ;=08000h
SVEAllEvents                equ  33790   ;=083FEh

;SpeechVoicePriority enumeration
SpeechVoicePriority typedef sdword
SVPNormal                   equ  0
SVPAlert                    equ  1
SVPOver                     equ  2

;SpeechVoiceSpeakFlags enumeartion
SpeechVoiceSpeakFlags typedef sdword
SVSFDefault                 equ  0
SVSFlagsAsync               equ  1
SVSFPurgeBeforeSpeak        equ  2
SVSFIsFilename              equ  4
SVSFIsXML                   equ  8
SVSFIsNotXML                equ  16      ;=010h
SVSFPersistXML              equ  32      ;=020h
SVSFNLPSpeakPunc            equ  64      ;=040h
SVSFNLPMask                 equ  64      ;=040h
SVSFVoiceMask               equ  127     ;=07Fh
SVSFUnusedFlags             equ  -128    ;=0FFFFFF80h

;SpeechVisemeFeature enumeration
SpeechVisemeFeature typedef sdword
SVF_None                    equ  0
SVF_Stressed                equ  1
SVF_Emphasis                equ  2

;SpeechVisemeType enumeration
SpeechVisemeType typedef sdword
SVP_0                       equ  0
SVP_1                       equ  1
SVP_2                       equ  2
SVP_3                       equ  3
SVP_4                       equ  4
SVP_5                       equ  5
SVP_6                       equ  6
SVP_7                       equ  7
SVP_8                       equ  8
SVP_9                       equ  9
SVP_10                      equ  10
SVP_11                      equ  11
SVP_12                      equ  12
SVP_13                      equ  13
SVP_14                      equ  14
SVP_15                      equ  15
SVP_16                      equ  16      ;=010h
SVP_17                      equ  17      ;=011h
SVP_18                      equ  18      ;=012h
SVP_19                      equ  19      ;=013h
SVP_20                      equ  20      ;=014h
SVP_21                      equ  21      ;=015h

;SpeechRecognizerState enumeration
SpeechRecognizerState typedef sdword
SRSInactive                 equ  0
SRSActive                   equ  1
SRSActiveAlways             equ  2
SRSInactiveWithPurge        equ  3

;SpeechInterference enumeration
SpeechInterference typedef sdword
SINone                      equ  0
SINoise                     equ  1
SINoSignal                  equ  2
SITooLoud                   equ  3
SITooQuiet                  equ  4
SITooFast                   equ  5
SITooSlow                   equ  6

;SpeechRecoEvents enumeration
SpeechRecoEvents typedef sdword
SREStreamEnd                equ  1
SRESoundStart               equ  2
SRESoundEnd                 equ  4
SREPhraseStart              equ  8
SRERecognition              equ  16      ;=010h
SREHypothesis               equ  32      ;=020h
SREBookmark                 equ  64      ;=040h
SREPropertyNumChange        equ  128     ;=080h
SREPropertyStringChange     equ  256     ;=0100h
SREFalseRecognition         equ  512     ;=0200h
SREInterference             equ  1024    ;=0400h
SRERequestUI                equ  2048    ;=0800h
SREStateChange              equ  4096    ;=01000h
SREAdaptation               equ  8192    ;=02000h
SREStreamStart              equ  16384   ;=04000h
SRERecoOtherContext         equ  32768   ;=08000h
SREAudioLevel               equ  65536   ;=010000h
SREPrivate                  equ  262144  ;=040000h
SREAllEvents                equ  393215  ;=05FFFFh

;SpeechRecoContextState enumeration
SpeechRecoContextState typedef sdword
SRCS_Disabled               equ  0
SRCS_Enabled                equ  1

;SpeechRetainedAudioOptions enumeration
SpeechRetainedAudioOptions typedef sdword
SRAONone                    equ  0
SRAORetainAudio             equ  1

;SpeechGrammarState enumeration
SpeechGrammarState typedef sdword
SGSEnabled                  equ  1
SGSDisabled                 equ  0
SGSExclusive                equ  3

;SpeechRuleAttributes enumeration

SpeechRuleAttributes typedef sdword
SRATopLevel                 equ  1
SRADefaultToActive          equ  2
SRAExport                   equ  4
SRAImport                   equ  8
SRAInterpreter              equ  16      ;=010h
SRADynamic                  equ  32      ;=020h

;SpeechGrammarRuleStateTransitionType enumeration
SpeechGrammarRuleStateTransitionType typedef sdword
SGRSTTEpsilon               equ  0
SGRSTTWord                  equ  1
SGRSTTRule                  equ  2
SGRSTTDictation             equ  3
SGRSTTWildcard              equ  4
SGRSTTTextBuffer            equ  5

;SpeechGrammarWordType enumeration
SpeechGrammarWordType typedef sdword
SGDisplay                   equ  0
SGLexical                   equ  1
SGPronounciation            equ  2

;SpeechSpecialTransitionType enumeration
SpeechSpecialTransitionType typedef sdword
SSTTWildcard                equ  1
SSTTDictation               equ  2
SSTTTextBuffer              equ  3

;SpeechLoadOption enumeration
SpeechLoadOption typedef sdword
SLOStatic                   equ  0
SLODynamic                  equ  1

;SpeechRuleState enumeration
SpeechRuleState typedef sdword
SGDSInactive                equ  0
SGDSActive                  equ  1
SGDSActiveWithAutoPause     equ  3

;SpeechWordPronounceable enumeration
SpeechWordPronounceable typedef sdword
SWPUnknownWordUnpronounceable equ  0
SWPUnknownWordPronounceable   equ  1
SWPKnownWordPronounceable     equ  2

;SpeechEngineConfidence enumeration
SpeechEngineConfidence typedef sdword
SECLowConfidence            equ  -1
SECNormalConfidence         equ  0
SECHighConfidence           equ  1

;SpeechDisplayAttributes enumeration
SpeechDisplayAttributes typedef sdword
SDA_No_Trailing_Space       equ  0
SDA_One_Trailing_Space      equ  2
SDA_Two_Trailing_Spaces     equ  4
SDA_Consume_Leading_Spaces  equ  8

;SpeechDiscardType enumeration
SpeechDiscardType typedef sdword
SDTProperty                 equ  1
SDTReplacement              equ  2
SDTRule                     equ  4
SDTDisplayText              equ  8
SDTLexicalForm              equ  16      ;=010h
SDTPronunciation            equ  32      ;=020h
SDTAudio                    equ  64      ;=040h
SDTAlternates               equ  128     ;=080h
SDTAll                      equ  255      ;=0FFh

;SpeechBookmarkOptions enumeration
SpeechBookmarkOptions typedef sdword
SBONone                     equ  0
SBOPause                    equ  1

;SpeechFormatType enumeration
SpeechFormatType typedef sdword
SFTInput                    equ  0
SFTSREngine                 equ  1

;SpeechRecognitionType enumeration
SpeechRecognitionType typedef sdword
SRTStandard                 equ  0
SRTAutopause                equ  1
SRTEmulated                 equ  2

;SpeechLexiconType enumeration
SpeechLexiconType typedef sdword
SLTUser                     equ  1
SLTApp                      equ  2

;SpeechWordType enumeration
SpeechWordType typedef sdword
SWTAdded                    equ  1
SWTDeleted                  equ  2

;SpeechPartOfSpeech enumeration
SpeechPartOfSpeech typedef sdword
SPSNotOverriden             equ  -1
SPSUnknown                  equ  0
SPSNoun                     equ  4096       ;=01000h
SPSVerb                     equ  8192       ;=02000h
SPSModifier                 equ  12288      ;=03000h
SPSFunction                 equ  16384      ;=04000h
SPSInterjection             equ  20480      ;=05000h


;SpeechConstants enumeration, GUID={F3E092B2-6BDC-410F-BCB2-4C5ED4424180}
Speech_Default_Weight       equ  1.065353E+09
Speech_Max_Word_Length      equ  128      ;=080h
Speech_Max_Pron_Length      equ  384      ;=0180h
Speech_StreamPos_Asap       equ  0
Speech_StreamPos_RealTime   equ  -1
SpeechAllElements           equ  -1

;DISPID_SpeechDataKey enumeration
DISPID_SpeechDataKey typedef sdword
DISPID_SDKSetBinaryValue    equ  1
DISPID_SDKGetBinaryValue    equ  2
DISPID_SDKSetStringValue    equ  3
DISPID_SDKGetStringValue    equ  4
DISPID_SDKSetLongValue      equ  5
DISPID_SDKGetlongValue      equ  6
DISPID_SDKOpenKey           equ  7
DISPID_SDKCreateKey         equ  8
DISPID_SDKDeleteKey         equ  9
DISPID_SDKDeleteValue       equ  10
DISPID_SDKEnumKeys          equ  11
DISPID_SDKEnumValues        equ  12

;DISPID_SpeechObjectToken enumeration
DISPID_SpeechObjectToken typedef sdword
DISPID_SOTId                equ  1
DISPID_SOTDataKey           equ  2
DISPID_SOTCategory          equ  3
DISPID_SOTGetDescription    equ  4
DISPID_SOTSetId             equ  5
DISPID_SOTGetAttribute      equ  6
DISPID_SOTCreateInstance    equ  7
DISPID_SOTRemove            equ  8
DISPID_SOTGetStorageFileName    equ  9
DISPID_SOTRemoveStorageFileName equ  10
DISPID_SOTIsUISupported     equ  11
DISPID_SOTDisplayUI         equ  12
DISPID_SOTMatchesAttributes equ  13

;DISPID_SpeechObjectTokens enumeration
DISPID_SpeechObjectTokens typedef sdword
DISPID_SOTsCount            equ  1
DISPID_SOTsItem             equ  0
DISPID_SOTs_NewEnum         equ  -4      ;=0FFFFFFFCh

;DISPID_SpeechObjectTokenCategory enumeration
DISPID_SpeechObjectTokenCategory typedef sdword
DISPID_SOTCId               equ  1
DISPID_SOTCDefault          equ  2
DISPID_SOTCSetId            equ  3
DISPID_SOTCGetDataKey       equ  4
DISPID_SOTCEnumerateTokens  equ  5

;DISPID_SpeechAudioFormat enumeration
DISPID_SpeechAudioFormat typedef sdword
DISPID_SAFType              equ  1
DISPID_SAFGuid              equ  2
DISPID_SAFGetWaveFormatEx   equ  3
DISPID_SAFSetWaveFormatEx   equ  4

;DISPID_SpeechBaseStream enumeration
DISPID_SpeechBaseStream typedef sdword
DISPID_SBSFormat            equ  1
DISPID_SBSRead              equ  2
DISPID_SBSWrite             equ  3
DISPID_SBSSeek              equ  4

;DISPID_SpeechAudio enumeration
DISPID_SpeechAudio typedef sdword
DISPID_SAStatus             equ  200      ;=0C8h
DISPID_SABufferInfo         equ  201      ;=0C9h
DISPID_SADefaultFormat      equ  202      ;=0CAh
DISPID_SAVolume             equ  203      ;=0CBh
DISPID_SABufferNotifySize   equ  204      ;=0CCh
DISPID_SAEventHandle        equ  205      ;=0CDh
DISPID_SASetState           equ  206      ;=0CEh

;DISPID_SpeechMMSysAudio enumeration
DISPID_SpeechMMSysAudio typedef sdword
DISPID_SMSADeviceId         equ  300      ;=012Ch
DISPID_SMSALineId           equ  301      ;=012Dh
DISPID_SMSAMMHandle         equ  302      ;=012Eh

;DISPID_SpeechFileStream enumeration
DISPID_SpeechFileStream typedef sdword
DISPID_SFSOpen              equ  100      ;=064h
DISPID_SFSClose             equ  101      ;=065h

;DISPID_SpeechCustomStream enumeration
DISPID_SpeechCustomStream typedef sdword
DISPID_SCSBaseStream        equ  100      ;=064h

;DISPID_SpeechMemoryStream enumeration
DISPID_SpeechMemoryStream typedef sdword
DISPID_SMSSetData           equ  100      ;=064h
DISPID_SMSGetData           equ  101      ;=065h

;DISPID_SpeechAudioStatus enumeration
DISPID_SpeechAudioStatus typedef sdword
DISPID_SASFreeBufferSpace   equ  1
DISPID_SASNonBlockingIO     equ  2
DISPID_SASState             equ  3
DISPID_SASCurrentSeekPosition   equ  4
DISPID_SASCurrentDevicePosition equ  5

;DISPID_SpeechAudioBufferInfo enumeration
DISPID_SpeechAudioBufferInfo typedef sdword
DISPID_SABIMinNotification  equ  1
DISPID_SABIBufferSize       equ  2
DISPID_SABIEventBias        equ  3

;DISPID_SpeechWaveFormatEx enumeration
DISPID_SpeechWaveFormatEx typedef sdword
DISPID_SWFEFormatTag        equ  1
DISPID_SWFEChannels         equ  2
DISPID_SWFESamplesPerSec    equ  3
DISPID_SWFEAvgBytesPerSec   equ  4
DISPID_SWFEBlockAlign       equ  5
DISPID_SWFEBitsPerSample    equ  6
DISPID_SWFEExtraData        equ  7

;DISPID_SpeechVoice enumeration
DISPID_SpeechVoice typedef sdword
DISPID_SVStatus             equ  1
DISPID_SVVoice              equ  2
DISPID_SVAudioOutput        equ  3
DISPID_SVAudioOutputStream  equ  4
DISPID_SVRate               equ  5
DISPID_SVVolume             equ  6
DISPID_SVAllowAudioOuputFormatChangesOnNextSet  equ  7
DISPID_SVEventInterests     equ  8
DISPID_SVPriority           equ  9
DISPID_SVAlertBoundary      equ  10
DISPID_SVSyncronousSpeakTimeout  equ  11
DISPID_SVSpeak              equ  12
DISPID_SVSpeakStream        equ  13
DISPID_SVPause              equ  14
DISPID_SVResume             equ  15
DISPID_SVSkip               equ  16      ;=010h
DISPID_SVGetVoices          equ  17      ;=011h
DISPID_SVGetAudioOutputs    equ  18      ;=012h
DISPID_SVWaitUntilDone      equ  19      ;=013h
DISPID_SVSpeakCompleteEvent equ  20      ;=014h
DISPID_SVIsUISupported      equ  21      ;=015h
DISPID_SVDisplayUI          equ  22      ;=016h

;DISPID_SpeechVoiceStatus enumeration
DISPID_SpeechVoiceStatus typedef sdword
DISPID_SVSCurrentStreamNumber     equ  1
DISPID_SVSLastStreamNumberQueued  equ  2
DISPID_SVSLastResult              equ  3
DISPID_SVSRunningState            equ  4
DISPID_SVSInputWordPosition       equ  5
DISPID_SVSInputWordLength         equ  6
DISPID_SVSInputSentencePosition   equ  7
DISPID_SVSInputSentenceLength     equ  8
DISPID_SVSLastBookmark            equ  9
DISPID_SVSLastBookmarkId          equ  10
DISPID_SVSPhonemeId               equ  11
DISPID_SVSVisemeId                equ  12

;DISPID_SpeechVoiceEvent enumeration
DISPID_SpeechVoiceEvent typedef sdword
DISPID_SVEStreamStart       equ  1
DISPID_SVEStreamEnd         equ  2
DISPID_SVEVoiceChange       equ  3
DISPID_SVEBookmark          equ  4
DISPID_SVEWord              equ  5
DISPID_SVEPhoneme           equ  6
DISPID_SVESentenceBoundary  equ  7
DISPID_SVEViseme            equ  8
DISPID_SVEAudioLevel        equ  9
DISPID_SVEEnginePrivate     equ  10

;DISPID_SpeechRecognizer enumeration
DISPID_SpeechRecognizer typedef sdword
DISPID_SRRecognizer         equ  1
DISPID_SRAllowAudioInputFormatChangesOnNextSet  equ  2
DISPID_SRAudioInput         equ  3
DISPID_SRAudioInputStream   equ  4
DISPID_SRIsShared           equ  5
DISPID_SRState              equ  6
DISPID_SRStatus             equ  7
DISPID_SRProfile            equ  8
DISPID_SREmulateRecognition equ  9
DISPID_SRCreateRecoContext  equ  10
DISPID_SRGetFormat          equ  11
DISPID_SRSetPropertyNumber  equ  12
DISPID_SRGetPropertyNumber  equ  13
DISPID_SRSetPropertyString  equ  14
DISPID_SRGetPropertyString  equ  15
DISPID_SRIsUISupported      equ  16      ;=010h
DISPID_SRDisplayUI          equ  17      ;=011h
DISPID_SRGetRecognizers     equ  18      ;=012h
DISPID_SVGetAudioInputs     equ  19      ;=013h
DISPID_SVGetProfiles        equ  20      ;=014h

;DISPID_SpeechRecognizerStatus enumeration
DISPID_SpeechRecognizerStatus typedef sdword
DISPID_SRSAudioStatus           equ  1
DISPID_SRSCurrentStreamPosition equ  2
DISPID_SRSCurrentStreamNumber   equ  3
DISPID_SRSNumberOfActiveRules   equ  4
DISPID_SRSClsidEngine           equ  5
DISPID_SRSSupportedLanguages    equ  6

;DISPID_SpeechRecoContext enumeration
DISPID_SpeechRecoContext typedef sdword
DISPID_SRCRecognizer          equ  1
DISPID_SRCAudioInInterferenceStatus  equ  2
DISPID_SRCRequestedUIType     equ  3
DISPID_SRCVoice               equ  4
DISPID_SRAllowVoiceFormatMatchingOnNextSet  equ  5
DISPID_SRCVoicePurgeEvent     equ  6
DISPID_SRCEventInterests      equ  7
DISPID_SRCCmdMaxAlternates    equ  8
DISPID_SRCState               equ  9
DISPID_SRCRetainedAudio       equ  10
DISPID_SRCRetainedAudioFormat equ  11
DISPID_SRCPause               equ  12
DISPID_SRCResume              equ  13
DISPID_SRCCreateGrammar       equ  14
DISPID_SRCCreateResultFromMemory  equ  15
DISPID_SRCBookmark            equ  16      ;=010h
DISPID_SRCSetAdaptationData   equ  17      ;=011h

;DISPIDSPRG enumeration
DISPIDSPRG typedef sdword
DISPID_SRGId                  equ  1
DISPID_SRGRecoContext         equ  2
DISPID_SRGState               equ  3
DISPID_SRGRules               equ  4
DISPID_SRGReset               equ  5
DISPID_SRGCommit              equ  6
DISPID_SRGCmdLoadFromFile     equ  7
DISPID_SRGCmdLoadFromObject   equ  8
DISPID_SRGCmdLoadFromResource equ  9
DISPID_SRGCmdLoadFromMemory   equ  10
DISPID_SRGCmdLoadFromProprietaryGrammar  equ  11
DISPID_SRGCmdSetRuleState     equ  12
DISPID_SRGCmdSetRuleIdState   equ  13
DISPID_SRGDictationLoad       equ  14
DISPID_SRGDictationUnload     equ  15
DISPID_SRGDictationSetState   equ  16      ;=010h
DISPID_SRGSetWordSequenceData equ  17      ;=011h
DISPID_SRGSetTextSelection    equ  18      ;=012h
DISPID_SRGIsPronounceable     equ  19      ;=013h

;DISPID_SpeechRecoContextEvents enumeration
DISPID_SpeechRecoContextEvents typedef sdword
DISPID_SRCEStartStream        equ  1
DISPID_SRCEEndStream          equ  2
DISPID_SRCEBookmark           equ  3
DISPID_SRCESoundStart         equ  4
DISPID_SRCESoundEnd           equ  5
DISPID_SRCEPhraseStart        equ  6
DISPID_SRCERecognition        equ  7
DISPID_SRCEHypothesis         equ  8
DISPID_SRCEPropertyNumberChange   equ  9
DISPID_SRCEPropertyStringChange   equ  10
DISPID_SRCEFalseRecognition   equ  11
DISPID_SRCEInterference       equ  12
DISPID_SRCERequestUI          equ  13
DISPID_SRCERecognizerStateChange  equ  14
DISPID_SRCEAdaptation         equ  15
DISPID_SRCERecognitionForOtherContext equ  16      ;=010h
DISPID_SRCEAudioLevel         equ  17      ;=011h
DISPID_SRCEEnginePrivate      equ  18      ;=012h

;DISPID_SpeechGrammarRule enumeration
DISPID_SpeechGrammarRule typedef sdword
DISPID_SGRAttributes          equ  1
DISPID_SGRInitialState        equ  2
DISPID_SGRName                equ  3
DISPID_SGRId                  equ  4
DISPID_SGRClear               equ  5
DISPID_SGRAddResource         equ  6
DISPID_SGRAddState            equ  7

;DISPID_SpeechGrammarRules enumeration
DISPID_SpeechGrammarRules typedef sdword
DISPID_SGRsCount              equ  1
DISPID_SGRsDynamic            equ  2
DISPID_SGRsAdd                equ  3
DISPID_SGRsCommit             equ  4
DISPID_SGRsCommitAndSave      equ  5
DISPID_SGRsFindRule           equ  6
DISPID_SGRsItem               equ  0
DISPID_SGRs_NewEnum           equ  -4      ;=0FFFFFFFCh

;DISPID_SpeechGrammarRuleState enumeration
DISPID_SpeechGrammarRuleState typedef sdword
DISPID_SGRSRule               equ  1
DISPID_SGRSTransitions        equ  2
DISPID_SGRSAddWordTransition  equ  3
DISPID_SGRSAddRuleTransition  equ  4
DISPID_SGRSAddSpecialTransition  equ  5

;DISPID_SpeechGrammarRuleStateTransitions enumeration
DISPID_SpeechGrammarRuleStateTransitions typedef sdword
DISPID_SGRSTsCount            equ  1
DISPID_SGRSTsItem             equ  0
DISPID_SGRSTs_NewEnum         equ  -4      ;=0FFFFFFFCh

;DISPID_SpeechGrammarRuleStateTransition enumeration
DISPID_SpeechGrammarRuleStateTransition typedef sdword
DISPID_SGRSTType              equ  1
DISPID_SGRSTText              equ  2
DISPID_SGRSTRule              equ  3
DISPID_SGRSTWeight            equ  4
DISPID_SGRSTPropertyName      equ  5
DISPID_SGRSTPropertyId        equ  6
DISPID_SGRSTPropertyValue     equ  7
DISPID_SGRSTNextState         equ  8

;DISPIDSPTSI enumeration
DISPIDSPTSI typedef sdword
DISPIDSPTSI_ActiveOffset      equ  1
DISPIDSPTSI_ActiveLength      equ  2
DISPIDSPTSI_SelectionOffset   equ  3
DISPIDSPTSI_SelectionLength   equ  4

;DISPID_SpeechRecoResult enumeration
DISPID_SpeechRecoResult typedef sdword
DISPID_SRRRecoContext         equ  1
DISPID_SRRTimes               equ  2
DISPID_SRRAudioFormat         equ  3
DISPID_SRRPhraseInfo          equ  4
DISPID_SRRAlternates          equ  5
DISPID_SRRAudio               equ  6
DISPID_SRRSpeakAudio          equ  7
DISPID_SRRSaveToMemory        equ  8
DISPID_SRRDiscardResultInfo   equ  9

;DISPID_SpeechPhraseBuilder enumeration
DISPID_SpeechPhraseBuilder typedef sdword
DISPID_SPPBRestorePhraseFromMemory  equ  1

;DISPID_SpeechRecoResultTimes enumeration
DISPID_SpeechRecoResultTimes typedef sdword
DISPID_SRRTStreamTime         equ  1
DISPID_SRRTLength             equ  2
DISPID_SRRTTickCount          equ  3
DISPID_SRRTOffsetFromStart    equ  4

;DISPID_SpeechPhraseAlternate enumeration
DISPID_SpeechPhraseAlternate typedef sdword
DISPID_SPARecoResult                equ  1
DISPID_SPAStartElementInResult      equ  2
DISPID_SPANumberOfElementsInResult  equ  3
DISPID_SPAPhraseInfo                equ  4
DISPID_SPACommit                    equ  5

;DISPID_SpeechPhraseAlternates enumeration
DISPID_SpeechPhraseAlternates typedef sdword
DISPID_SPAsCount              equ  1
DISPID_SPAsItem               equ  0
DISPID_SPAs_NewEnum           equ  -4      ;=0FFFFFFFCh

;DISPID_SpeechPhraseInfo enumeration
DISPID_SpeechPhraseInfo typedef sdword
DISPID_SPILanguageId          equ  1
DISPID_SPIGrammarId           equ  2
DISPID_SPIStartTime           equ  3
DISPID_SPIAudioStreamPosition equ  4
DISPID_SPIAudioSizeBytes      equ  5
DISPID_SPIRetainedSizeBytes   equ  6
DISPID_SPIAudioSizeTime       equ  7
DISPID_SPIRule                equ  8
DISPID_SPIProperties          equ  9
DISPID_SPIElements            equ  10
DISPID_SPIReplacements        equ  11
DISPID_SPIEngineId            equ  12
DISPID_SPIEnginePrivateData   equ  13
DISPID_SPISaveToMemory        equ  14
DISPID_SPIGetText             equ  15
DISPID_SPIGetDisplayAttributes  equ  16      ;=010h

;DISPID_SpeechPhraseElement enumeration
DISPID_SpeechPhraseElement typedef sdword
DISPID_SPEAudioTimeOffset     equ  1
DISPID_SPEAudioSizeTime       equ  2
DISPID_SPEAudioStreamOffset   equ  3
DISPID_SPEAudioSizeBytes      equ  4
DISPID_SPERetainedStreamOffset  equ  5
DISPID_SPERetainedSizeBytes   equ  6
DISPID_SPEDisplayText         equ  7
DISPID_SPELexicalForm         equ  8
DISPID_SPEPronunciation       equ  9
DISPID_SPEDisplayAttributes   equ  10
DISPID_SPERequiredConfidence  equ  11
DISPID_SPEActualConfidence    equ  12
DISPID_SPEEngineConfidence    equ  13

;DISPID_SpeechPhraseElements enumeration
DISPID_SpeechPhraseElements typedef sdword
DISPID_SPEsCount              equ  1
DISPID_SPEsItem               equ  0
DISPID_SPEs_NewEnum           equ  -4      ;=0FFFFFFFCh

;DISPID_SpeechPhraseReplacement enumeration
DISPID_SpeechPhraseReplacement typedef sdword
DISPID_SPRDisplayAttributes   equ  1
DISPID_SPRText                equ  2
DISPID_SPRFirstElement        equ  3
DISPID_SPRNumberOfElements    equ  4

;DISPID_SpeechPhraseReplacements enumeration
DISPID_SpeechPhraseReplacements typedef sdword
DISPID_SPRsCount              equ  1
DISPID_SPRsItem               equ  0
DISPID_SPRs_NewEnum           equ  -4      ;=0FFFFFFFCh

;DISPID_SpeechPhraseProperty enumeration
DISPID_SpeechPhraseProperty typedef sdword
DISPID_SPPName                equ  1
DISPID_SPPId                  equ  2
DISPID_SPPValue               equ  3
DISPID_SPPFirstElement        equ  4
DISPID_SPPNumberOfElements    equ  5
DISPID_SPPEngineConfidence    equ  6
DISPID_SPPConfidence          equ  7
DISPID_SPPParent              equ  8
DISPID_SPPChildren            equ  9

;DISPID_SpeechPhraseProperties enumeration
DISPID_SpeechPhraseProperties typedef sdword
DISPID_SPPsCount              equ  1
DISPID_SPPsItem               equ  0
DISPID_SPPs_NewEnum           equ  -4      ;=0FFFFFFFCh

;DISPID_SpeechPhraseRule enumeration
DISPID_SpeechPhraseRule typedef sdword
DISPID_SPRuleName             equ  1
DISPID_SPRuleId               equ  2
DISPID_SPRuleFirstElement     equ  3
DISPID_SPRuleNumberOfElements equ  4
DISPID_SPRuleParent           equ  5
DISPID_SPRuleChildren         equ  6
DISPID_SPRuleConfidence       equ  7
DISPID_SPRuleEngineConfidence equ  8

;DISPID_SpeechPhraseRules enumeration
DISPID_SpeechPhraseRules typedef sdword
DISPID_SPRulesCount           equ  1
DISPID_SPRulesItem            equ  0
DISPID_SPRules_NewEnum        equ  -4      ;=0FFFFFFFCh

;DISPID_SpeechLexicon enumeration
DISPID_SpeechLexicon typedef sdword
DISPID_SLGenerationId         equ  1
DISPID_SLGetWords             equ  2
DISPID_SLAddPronunciation     equ  3
DISPID_SLAddPronunciationByPhoneIds  equ  4
DISPID_SLRemovePronunciation  equ  5
DISPID_SLRemovePronunciationByPhoneIds  equ  6
DISPID_SLGetPronunciations    equ  7
DISPID_SLGetGenerationChange  equ  8

;DISPID_SpeechLexiconWords enumeration
DISPID_SpeechLexiconWords typedef sdword
DISPID_SLWsCount              equ  1
DISPID_SLWsItem               equ  0
DISPID_SLWs_NewEnum           equ  -4      ;=0FFFFFFFCh

;DISPID_SpeechLexiconWord enumeration
DISPID_SpeechLexiconWord typedef sdword
DISPID_SLWLangId              equ  1
DISPID_SLWType                equ  2
DISPID_SLWWord                equ  3
DISPID_SLWPronunciations      equ  4

;DISPID_SpeechLexiconProns enumeration

DISPID_SpeechLexiconProns typedef sdword
DISPID_SLPsCount              equ  1
DISPID_SLPsItem               equ  0
DISPID_SLPs_NewEnum           equ  -4      ;=0FFFFFFFCh

;DISPID_SpeechLexiconPronunciation enumeration
DISPID_SpeechLexiconPronunciation typedef sdword
DISPID_SLPType                equ  1
DISPID_SLPLangId              equ  2
DISPID_SLPPartOfSpeech        equ  3
DISPID_SLPPhoneIds            equ  4
DISPID_SLPSymbolic            equ  5

;DISPID_SpeechPhoneConverter enumeration
DISPID_SpeechPhoneConverter typedef sdword
DISPID_SPCLangId              equ  1
DISPID_SPCPhoneToId           equ  2
DISPID_SPCIdToPhone           equ  3

;SPDATAKEYLOCATION enumeration
SPDATAKEYLOCATION typedef sdword
SPDKL_DefaultLocation         equ  0
SPDKL_CurrentUser             equ  1
SPDKL_LocalMachine            equ  2
SPDKL_CurrentConfig           equ  5

;SPAUDIOSTATE enumeration
SPAUDIOSTATE typedef sdword
SPAS_CLOSED                   equ  0
SPAS_STOP                     equ  1
SPAS_PAUSE                    equ  2
SPAS_RUN                      equ  3

;SPFILEMODE enumeration
SPFILEMODE typedef sdword
SPFM_OPEN_READONLY            equ  0
SPFM_OPEN_READWRITE           equ  1
SPFM_CREATE                   equ  2
SPFM_CREATE_ALWAYS            equ  3
SPFM_NUM_MODES                equ  4

;END_SR_STREAM event (in WPARAM) enumeration
SPESF_NONE                    equ 0
SPESF_STREAM_RELEASED         equ 1

;Viseme features
SPVFEATURE_STRESSED           equ 1
SPVFEATURE_EMPHASIS           equ 2

;SPVISEMES enumeration
SPVISEMES typedef sdword
SP_VISEME_0                   equ  0
SP_VISEME_1                   equ  1
SP_VISEME_2                   equ  2
SP_VISEME_3                   equ  3
SP_VISEME_4                   equ  4
SP_VISEME_5                   equ  5
SP_VISEME_6                   equ  6
SP_VISEME_7                   equ  7
SP_VISEME_8                   equ  8
SP_VISEME_9                   equ  9
SP_VISEME_10                  equ  10
SP_VISEME_11                  equ  11
SP_VISEME_12                  equ  12
SP_VISEME_13                  equ  13
SP_VISEME_14                  equ  14
SP_VISEME_15                  equ  15
SP_VISEME_16                  equ  16      ;=010h
SP_VISEME_17                  equ  17      ;=011h
SP_VISEME_18                  equ  18      ;=012h
SP_VISEME_19                  equ  19      ;=013h
SP_VISEME_20                  equ  20      ;=014h
SP_VISEME_21                  equ  21      ;=015h

;SPVPRIORITY enumeration
SPVPRIORITY typedef sdword
SPVPRI_NORMAL                 equ  0
SPVPRI_ALERT                  equ  1
SPVPRI_OVER                   equ  2

;SPEVENTLPARAMTYPE enumeration
SPET_LPARAM_IS_UNDEFINED      equ  0
SPET_LPARAM_IS_TOKEN          equ  1
SPET_LPARAM_IS_OBJECT         equ  2
SPET_LPARAM_IS_POINTER        equ  3
SPET_LPARAM_IS_STRING         equ  4

;SPEVENTENUM enumeration
SPEVENTENUM typedef sdword
SPEI_UNDEFINED                equ  0
;TTS engine
SPEI_START_INPUT_STREAM       equ  1
SPEI_END_INPUT_STREAM         equ  2
SPEI_VOICE_CHANGE             equ  3
SPEI_TTS_BOOKMARK             equ  4
SPEI_WORD_BOUNDARY            equ  5
SPEI_PHONEME                  equ  6
SPEI_SENTENCE_BOUNDARY        equ  7
SPEI_VISEME                   equ  8
SPEI_TTS_AUDIO_LEVEL          equ  9
;Engine vendors use these reserved bits
SPEI_TTS_PRIVATE              equ  15
SPEI_MIN_TTS                  equ  1
SPEI_MAX_TTS                  equ  15
;Speech Recognition
SPEI_END_SR_STREAM            equ  34      ;=022h
SPEI_SOUND_START              equ  35      ;=023h
SPEI_SOUND_END                equ  36      ;=024h
SPEI_PHRASE_START             equ  37      ;=025h
SPEI_RECOGNITION              equ  38      ;=026h
SPEI_HYPOTHESIS               equ  39      ;=027h
SPEI_SR_BOOKMARK              equ  40      ;=028h
SPEI_PROPERTY_NUM_CHANGE      equ  41      ;=029h
SPEI_PROPERTY_STRING_CHANGE   equ  42      ;=02Ah
SPEI_FALSE_RECOGNITION        equ  43      ;=02Bh
SPEI_INTERFERENCE             equ  44      ;=02Ch
SPEI_REQUEST_UI               equ  45      ;=02Dh
SPEI_RECO_STATE_CHANGE        equ  46      ;=02Eh
SPEI_ADAPTATION               equ  47      ;=02Fh
SPEI_START_SR_STREAM          equ  48      ;=030h
SPEI_RECO_OTHER_CONTEXT       equ  49      ;=031h
SPEI_SR_AUDIO_LEVEL           equ  50      ;=032h
SPEI_SR_PRIVATE               equ  52      ;=034h
SPEI_MIN_SR                   equ  34      ;=022h
SPEI_MAX_SR                   equ  52      ;=034h
SPEI_RESERVED1                equ  30      ;=01Eh
SPEI_RESERVED2                equ  33      ;=021h
SPEI_RESERVED3                equ  63      ;=03Fh

SPFEI_FLAGCHECK               equ (1 SHL SPEI_RESERVED1) or (1 SHL SPEI_RESERVED2)
SPFEI_ALL_TTS_EVENTS          equ 000000000000FFFEh or SPFEI_FLAGCHECK
SPFEI_ALL_SR_EVENTS           equ 001FFFFC00000000h or SPFEI_FLAGCHECK
SPFEI_ALL_EVENTS              equ 0EFFFFFFFFFFFFFFFh


; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     SPFEI
; Purpose:   Created a qword in edx::eax that identifies the event interest.
; Arguments: Interest flag.

SPFEI macro SPEI_Flag:req
  if SPEI_Flag gt 31
    mov eax, 40000000h
    if SPEI_Flag gt 32
      mov edx, (1 shl (SPEI_Flag - 32)) or 00000002h
    else
      mov edx, 00000002h
    endif
  else
    mov eax, (1 shl SPEI_Flag) or 40000000h
    mov edx, 00000002h
  endif
endm


;SPRECOSTATE enumeration
SPRECOSTATE typedef sdword
SPRST_INACTIVE                equ  0
SPRST_ACTIVE                  equ  1
SPRST_ACTIVE_ALWAYS           equ  2
SPRST_INACTIVE_WITH_PURGE     equ  3
SPRST_NUM_STATES              equ  4

;SPWAVEFORMATTYPE enumeration
SPWAVEFORMATTYPE typedef sdword
SPWF_INPUT                    equ  0
SPWF_SRENGINE                 equ  1

;SPGRAMMARWORDTYPE enumeration
SPGRAMMARWORDTYPE typedef sdword
SPWT_DISPLAY                  equ  0
SPWT_LEXICAL                  equ  1
SPWT_PRONUNCIATION            equ  2

;SPLOADOPTIONS enumeration
SPLOADOPTIONS typedef sdword
SPLO_STATIC                   equ  0
SPLO_DYNAMIC                  equ  1

;SPRULESTATE enumeration
SPRULESTATE typedef sdword
SPRS_INACTIVE                 equ  0
SPRS_ACTIVE                   equ  1
SPRS_ACTIVE_WITH_AUTO_PAUSE   equ  3

;SPWORDPRONOUNCEABLE enumeration
SPWORDPRONOUNCEABLE typedef sdword
SPWP_UNKNOWN_WORD_UNPRONOUNCEABLE equ  0
SPWP_UNKNOWN_WORD_PRONOUNCEABLE   equ  1
SPWP_KNOWN_WORD_PRONOUNCEABLE     equ  2

;SPGRAMMARSTATE enumeration
SPGRAMMARSTATE typedef sdword
SPGS_DISABLED                 equ  0
SPGS_ENABLED                  equ  1
SPGS_EXCLUSIVE                equ  3

;SPINTERFERENCE enumeration
SPINTERFERENCE typedef sdword
SPINTERFERENCE_NONE           equ  0
SPINTERFERENCE_NOISE          equ  1
SPINTERFERENCE_NOSIGNAL       equ  2
SPINTERFERENCE_TOOLOUD        equ  3
SPINTERFERENCE_TOOQUIET       equ  4
SPINTERFERENCE_TOOFAST        equ  5
SPINTERFERENCE_TOOSLOW        equ  6

;SPAUDIOOPTIONS enumeration
SPAUDIOOPTIONS typedef sdword
SPAO_NONE                     equ  0
SPAO_RETAIN_AUDIO             equ  1

;SPBOOKMARKOPTIONS enumeration
SPBOOKMARKOPTIONS typedef sdword
SPBO_NONE                     equ  0
SPBO_PAUSE                    equ  1

;SPCONTEXTSTATE enumeration
SPCONTEXTSTATE typedef sdword
SPCS_DISABLED                 equ  0
SPCS_ENABLED                  equ  1

;SPLEXICONTYPE enumeration
SPLEXICONTYPE typedef sdword
eLEXTYPE_USER                 equ  1
eLEXTYPE_APP                  equ  2
eLEXTYPE_RESERVED1            equ  4
eLEXTYPE_RESERVED2            equ  8
eLEXTYPE_RESERVED3            equ  16             ;=010h
eLEXTYPE_RESERVED4            equ  32             ;=020h
eLEXTYPE_RESERVED5            equ  64             ;=040h
eLEXTYPE_RESERVED6            equ  128            ;=080h
eLEXTYPE_RESERVED7            equ  256            ;=0100h
eLEXTYPE_RESERVED8            equ  512            ;=0200h
eLEXTYPE_RESERVED9            equ  1024           ;=0400h
eLEXTYPE_RESERVED10           equ  2048           ;=0800h
eLEXTYPE_PRIVATE1             equ  4096           ;=01000h
eLEXTYPE_PRIVATE2             equ  8192           ;=02000h
eLEXTYPE_PRIVATE3             equ  16384          ;=04000h
eLEXTYPE_PRIVATE4             equ  32768          ;=08000h
eLEXTYPE_PRIVATE5             equ  65536          ;=010000h
eLEXTYPE_PRIVATE6             equ  131072         ;=020000h
eLEXTYPE_PRIVATE7             equ  262144         ;=040000h
eLEXTYPE_PRIVATE8             equ  524288         ;=080000h
eLEXTYPE_PRIVATE9             equ  1048576        ;=0100000h
eLEXTYPE_PRIVATE10            equ  2097152        ;=0200000h
eLEXTYPE_PRIVATE11            equ  4194304        ;=0400000h
eLEXTYPE_PRIVATE12            equ  8388608        ;=0800000h
eLEXTYPE_PRIVATE13            equ  16777216       ;=01000000h
eLEXTYPE_PRIVATE14            equ  33554432       ;=02000000h
eLEXTYPE_PRIVATE15            equ  67108864       ;=04000000h
eLEXTYPE_PRIVATE16            equ  134217728      ;=08000000h
eLEXTYPE_PRIVATE17            equ  268435456      ;=010000000h
eLEXTYPE_PRIVATE18            equ  536870912      ;=020000000h
eLEXTYPE_PRIVATE19            equ  1073741824     ;=040000000h
eLEXTYPE_PRIVATE20            equ  -2147483648    ;=080000000h

;SPPARTOFSPEECH enumeration
SPPARTOFSPEECH typedef sdword
SPPS_NotOverriden             equ  -1
SPPS_Unknown                  equ  0
SPPS_Noun                     equ  4096           ;=01000h
SPPS_Verb                     equ  8192           ;=02000h
SPPS_Modifier                 equ  12288          ;=03000h
SPPS_Function                 equ  16384          ;=04000h
SPPS_Interjection             equ  20480          ;=05000h

;SPWORDTYPE enumeration
SPWORDTYPE typedef sdword
eWORDTYPE_ADDED               equ  1
eWORDTYPE_DELETED             equ  2


SPPR_ALL_ELEMENTS             equ -1

SPREF_AutoPause               equ 1
SPREF_Emulated                equ 2

SPPS_NotOverriden             equ -1
SPPS_Unknown                  equ  0
SPPS_Noun                     equ 1000h
SPPS_Verb                     equ 2000h
SPPS_Modifier                 equ 3000h
SPPS_Function                 equ 4000h
SPPS_Interjection             equ 5000h

eLEXTYPE_USER                 equ 00000001h
eLEXTYPE_APP                  equ 00000002h
eLEXTYPE_RESERVED1            equ 00000004h
eLEXTYPE_RESERVED2            equ 00000008h
eLEXTYPE_RESERVED3            equ 00000010h
eLEXTYPE_RESERVED4            equ 00000020h
eLEXTYPE_RESERVED5            equ 00000040h
eLEXTYPE_RESERVED6            equ 00000080h
eLEXTYPE_RESERVED7            equ 00000100h
eLEXTYPE_RESERVED8            equ 00000200h
eLEXTYPE_RESERVED9            equ 00000400h
eLEXTYPE_RESERVED10           equ 00000800h
eLEXTYPE_PRIVATE1             equ 00001000h
eLEXTYPE_PRIVATE2             equ 00002000h
eLEXTYPE_PRIVATE3             equ 00004000h
eLEXTYPE_PRIVATE4             equ 00008000h
eLEXTYPE_PRIVATE5             equ 00010000h
eLEXTYPE_PRIVATE6             equ 00020000h
eLEXTYPE_PRIVATE7             equ 00040000h
eLEXTYPE_PRIVATE8             equ 00080000h
eLEXTYPE_PRIVATE9             equ 00100000h
eLEXTYPE_PRIVATE10            equ 00200000h
eLEXTYPE_PRIVATE11            equ 00400000h
eLEXTYPE_PRIVATE12            equ 00800000h
eLEXTYPE_PRIVATE13            equ 01000000h
eLEXTYPE_PRIVATE14            equ 02000000h
eLEXTYPE_PRIVATE15            equ 04000000h
eLEXTYPE_PRIVATE16            equ 08000000h
eLEXTYPE_PRIVATE17            equ 10000000h
eLEXTYPE_PRIVATE18            equ 20000000h
eLEXTYPE_PRIVATE19            equ 40000000h
eLEXTYPE_PRIVATE20            equ 80000000h

WORDTYPE_ADDED                equ 1
eWORDTYPE_DELETED             equ 2

SP_GETWHOLEPHRASE             equ SPPR_ALL_ELEMENTS
SPRR_ALL_ELEMENTS             equ SPPR_ALL_ELEMENTS

GID_DICTATION                 equ 0    ;ID for the dictation grammar
GID_DICTATIONCC               equ 1    ;ID for the C&C grammar that's active during dictation
GID_CC                        equ 2    ;ID for the C&C grammar that's active when dictation is no

SPWF_INPUT                    equ 0
SPWF_SRENGINE                 equ 1
SPRST_INACTIVE                equ 0
SPRST_ACTIVE                  equ 1
SPRST_ACTIVE_ALWAYS           equ 2
SPRST_INACTIVE_WITH_PURGE     equ 3
SPRST_NUM_STATES              equ 4

SPRAF_TopLevel                equ 1
SPRAF_Active                  equ 2
SPRAF_Export                  equ 4
SPRAF_Import                  equ 8
SPRAF_Interpreter             equ 16
SPRAF_Dynamic                 equ 32
SPRAF_AutoPause               equ 65536

SPWT_DISPLAY                  equ 0
SPWT_LEXICAL                  equ 1
SPWT_PRONUNCIATION            equ 2

SPRS_DONE                     equ 1
SPRS_IS_SPEAKING              equ 2

SPMIN_VOLUME                  equ 0
SPMAX_VOLUME                  equ 100
SPMIN_RATE                    equ -10
SPMAX_RATE                    equ 10

SPVPRI_NORMAL                 equ  0
SPVPRI_ALERT                  equ  1
SPVPRI_OVER                   equ  2

SPVA_Speak                    equ 0
SPVA_Silence                  equ 1
SPVA_Pronounce                equ 2
SPVA_Bookmark                 equ 3
SPVA_SpellOut                 equ 4
SPVA_Section                  equ 5
SPVA_ParseUnknownTag          equ 6

SPAF_ONE_TRAILING_SPACE       equ 02h
SPAF_TWO_TRAILING_SPACES      equ 04h
SPAF_CONSUME_LEADING_SPACES   equ 08h
SPAF_ALL                      equ 0fh

SPDF_PROPERTY                 equ 1
SPDF_REPLACEMENT              equ 2
SPDF_RULE                     equ 4
SPDF_DISPLAYTEXT              equ 8
SPDF_LEXICALFORM              equ 10h
SPDF_PRONUNCIATION            equ 20h
SPDF_AUDIO                    equ 40h
SPDF_ALTERNATES               equ 80h
SPDF_ALL                      equ 0ffh

SP_STREAMPOS_ASAP             equ 0
SP_STREAMPOS_REALTIME         equ -1

SP_MAX_LANGIDS                equ 20


SPPHRASEELEMENT struct 4
  ulAudioTimeOffset       dword     ?
  ulAudioSizeTime         dword     ?
  ulAudioStreamOffset     dword     ?
  ulAudioSizeBytes        dword     ?
  ulRetainedStreamOffset  dword     ?
  ulRetainedSizeBytes     dword     ?
  pszDisplayText          Pointer   ?
  pszLexicalForm          Pointer   ?
  pszPronunciation        Pointer   ?
  bDisplayAttributes      byte      ?
  RequiredConfidence      sbyte     ?
  ActualConfidence        sbyte     ?
  reserved                byte      ?
  SREngineConfidence      real4     ?
SPPHRASEELEMENT ends


SPPHRASEREPLACEMENT struct 4
  bDisplayAttributes      byte      ?
  pszReplacementText      Pointer   ?
  ulFirstElement          dword     ?
  ulCountOfElements       dword     ?
SPPHRASEREPLACEMENT ends


SPSERIALIZEDPHRASE struct 4
  ulSerializedSize        dword     ?
SPSERIALIZEDPHRASE ends


_RemotableHandle struct 4
  fContext                sdword    ?
  u                       dword     ?
_RemotableHandle ends


SPPROPERTYINFO struct 8
  pszName                 Pointer   ?
  ulId                    dword     ?
  pszValue                Pointer   ?
  vValue                  VARIANT   <?>
SPPROPERTYINFO ends


SPAUDIOSTATUS struct 8
  cbFreeBuffSpace         sdword    ?
  cbNonBlockingIO         dword     ?
  State                   SPAUDIOSTATE  ?
  CurSeekPos              qword     ?
  CurDevicePos            qword     ?
  dwReserved1             dword     ?
  dwReserved2             dword     ?
SPAUDIOSTATUS ends


SPBINARYGRAMMAR struct 4
  ulTotalSerializedSize   dword     ?
SPBINARYGRAMMAR ends


SPAUDIOBUFFERINFO struct 4
  ulMsMinNotification     dword     ?
  ulMsBufferSize          dword     ?
  ulMsEventBias           dword     ?
SPAUDIOBUFFERINFO ends


SPTEXTSELECTIONINFO struct 4
  ulStartActiveOffset     dword     ?
  cchActiveChars          dword     ?
  ulStartSelection        dword     ?
  cchSelection            dword     ?
SPTEXTSELECTIONINFO ends


SPVOICESTATUS struct 4
  ulCurrentStream         dword     ?
  ulLastStreamQueued      dword     ?
  hrLastResult            HRESULT   ?
  dwRunningState          dword     ?
  ulInputWordPos          dword     ?
  ulInputWordLen          dword     ?
  ulInputSentPos          dword     ?
  ulInputSentLen          dword     ?
  lBookmarkId             sdword    ?
  PhonemeId               word      ?
  VisemeId                SPVISEMES ?
  dwReserved1             dword     ?
  dwReserved2             dword     ?
SPVOICESTATUS ends


SPRECOCONTEXTSTATUS struct 4
  eInterference           SPINTERFERENCE  ?
  szRequestTypeOfUI       word 255 dup (?)
  dwReserved1             dword     ?
  dwReserved2             dword     ?
SPRECOCONTEXTSTATUS ends


SPSERIALIZEDRESULT struct 4
  ulSerializedSize        dword     ?
SPSERIALIZEDRESULT ends


WaveFormatEx struct 4
  wFormatTag              word      ?
  nChannels               word      ?
  nSamplesPerSec          dword     ?
  nAvgBytesPerSec         dword     ?
  nBlockAlign             word      ?
  wBitsPerSample          word      ?
  cbSize                  word      ?
WaveFormatEx ends


SPEVENT struct 8
  eEventId                word      ?
  elParamType             word      ?
  ulStreamNum             dword     ?
  ullAudioStreamOffset    qword     ?
  wParam                  Pointer   ?
  lParam                  Pointer   ?
SPEVENT ends

SPSERIALIZEDEVENT textequ <SPEVENT>


SPSERIALIZEDEVENT64 struc
  eEventId                word      ?
  elParamType             word      ?
  ulStreamNum             dword     ?
  ullAudioStreamOffset    qword     ?
  SerializedwParam        qword     ?
  SerializedlParam        qword     ?
SPSERIALIZEDEVENT64 ends



SPWORDPRONUNCIATIONLIST struct 4
  ulSize                  dword     ?
  pvBuffer                Pointer   ?
  pFirstWordPronunciation Pointer   ?
SPWORDPRONUNCIATIONLIST ends


SPWORDPRONUNCIATION struct 4
  pNextWordPronunciation  Pointer   ?
  eLexiconType            SPLEXICONTYPE   ?
  LangId                  word      ?
  wReserved               word      ?
  ePartOfSpeech           SPPARTOFSPEECH  ?
  szPronunciation         word 1 dup (?)
SPWORDPRONUNCIATION ends


SPEVENTSOURCEINFO struct 8
  ullEventInterest        qword     ?
  ullQueuedInterest       qword     ?
  ulCount                 dword     ?
SPEVENTSOURCEINFO ends


SPPHRASERULE struct 4
  pszName                 Pointer   ?
  ulId                    dword     ?
  ulFirstElement          dword     ?
  ulCountOfElements       dword     ?
  pNextSibling            Pointer   ?
  pFirstChild             Pointer   ?
  SREngineConfidence      real4     ?
  Confidence              sbyte     ?
SPPHRASERULE ends


SPWORDLIST struct 4
  ulSize                  dword     ?
  pvBuffer                Pointer   ?
  pFirstWord              Pointer   ?
SPWORDLIST ends


SPWORD struct 4
  pNextWord               Pointer   ?
  LangId                  word      ?
  wReserved               word      ?
  eWordType               SPWORDTYPE ?
  pszWord                 Pointer   ?
  pFirstWordPronunciation Pointer   ?
SPWORD ends


SPPHRASEPROPERTY struct 8
  pszName                 Pointer   ?
  ulId                    dword     ?
  pszValue                Pointer   ?
  vValue                  VARIANT   <?>
  ulFirstElement          dword     ?
  ulCountOfElements       dword     ?
  pNextSibling            Pointer   ?
  pFirstChild             Pointer   ?
  SREngineConfidence      real4     ?
  Confidence              sbyte     ?
SPPHRASEPROPERTY ends


SPRECOGNIZERSTATUS struct 8
  AudioStatus             SPAUDIOSTATUS  <>
  ullRecognitionStreamPos qword     ?
  ulStreamNumber          dword     ?
  ulNumActive             dword     ?
  ClsidEngine             GUID      <>
  cLangIDs                dword     ?
  aLangID                 word 20 dup  (?)
  dwReserved1             dword     ?
  dwReserved2             dword     ?
SPRECOGNIZERSTATUS ends


tagSTATSTG struct 8
  pwcsName                LPWSTR    ?
  dType                   dword     ?
  cbSize                  qword     ?
  mtime                   FILETIME  <>
  ctime                   FILETIME  <>
  atime                   FILETIME  <>
  grfMode                 dword     ?
  grfLocksSupported       dword     ?
  clsid                   GUID      <>
  grfStateBits            dword     ?
  reserved                dword     ?
tagSTATSTG ends


SPRECORESULTTIMES struct 8
  ftStreamTime            FILETIME  <>
  ullLength               qword     ?
  dwTickCount             dword     ?
  ullStart                qword     ?
SPRECORESULTTIMES ends


SPPHRASE struct 8
  cbSize                  dword     ?
  LangId                  word      ?
  wReserved               word      ?
  ullGrammarID            qword     ?
  ftStartTime             qword     ?
  ullAudioStreamPosition  qword     ?
  ulAudioSizeBytes        dword     ?
  ulRetainedSizeBytes     dword     ?
  ulAudioSizeTime         dword     ?
  Rule                    SPPHRASERULE  <>
  pProperties             Pointer   ?
  pElements               Pointer   ?
  cReplacements           dword     ?
  pReplacements           Pointer   ?
  SREngineID              GUID      <>
  ulSREnginePrivateDataSize dword   ?
  pSREnginePrivateData    Pointer   ?
SPPHRASE ends


__MIDL_IWinTypes_0009 union 4
  hInproc                 sdword    ?
  hRemote                 sdword    ?
__MIDL_IWinTypes_0009 ends


SPVPITCH struc
  MiddleAdj               sdword    ?
  RangeAdj                sdword    ?
SPVPITCH ends


SPVCONTEXT struc
  pCategory               Pointer   ?
  pBefore                 Pointer   ?
  pAfter                  Pointer   ?
SPVCONTEXT  ends


SPVSTATE struc
  eAction                 dword     ?
  LangID                  word      ?
  wReserved               word      ?
  EmphAdj                 sdword    ?
  RateAdj                 sdword    ?
  Volume                  dword     ?
  PitchAdj                SPVPITCH  <>
  SilenceMSecs            dword     ?
  pPhoneIds               Pointer   ?
  ePartOfSpeech           dword     ?
  Context                 SPVCONTEXT <>
SPVSTATE ends


SPSTREAMFORMATTYPE  typedef SPWAVEFORMATTYPE

BEGIN_INTERFACE IEnumSpObjectTokens, IUnknown, <06B64F9E-7FDA-11D2-B4F2-00C04F797396>
  STDMETHOD  Next,                        :dword, :ptr ptr ISpObjectToken, :ptr dword
  STDMETHOD  Skip,                        :dword
  STDMETHOD  Reset
  STDMETHOD  Clone,                       :ptr ptr IEnumSpObjectTokens
  STDMETHOD  Item,                        :dword, :ptr ptr ISpObjectToken
  STDMETHOD  GetCount,                    :ptr dword
END_INTERFACE

BEGIN_INTERFACE ISpDataKey, IUnknown, <14056581-E16C-11D2-BB90-00C04F8EE6C0>
  STDMETHOD  SetData,                     :ptr word, :dword, :ptr byte
  STDMETHOD  GetData,                     :ptr word, :ptr dword, :ptr byte
  STDMETHOD  SetStringValue,              :ptr word, :ptr word
  STDMETHOD  GetStringValue,              :ptr word, :ptr ptr word
  STDMETHOD  SetDWORD,                    :ptr word, :dword
  STDMETHOD  GetDWORD,                    :ptr word, :ptr dword
  STDMETHOD  OpenKey,                     :ptr word, :ptr ptr ISpDataKey
  STDMETHOD  CreateKey,                   :ptr word, :ptr ptr ISpDataKey
  STDMETHOD  DeleteKey,                   :ptr word
  STDMETHOD  DeleteValue,                 :ptr word
  STDMETHOD  EnumKeys,                    :dword, :ptr ptr word
  STDMETHOD  EnumValues,                  :dword, :ptr ptr word
END_INTERFACE


;BEGIN_INTERFACE IServiceProvider, IUnknown, <6D5140C1-7436-11CE-8034-00AA006009FA>
;  STDMETHOD  RemoteQueryService,          :ptr GUID, :ptr GUID, :ptr LPUNKNOWN
;END_INTERFACE


BEGIN_INTERFACE ISpResourceManager, IServiceProvider, <93384E18-5014-43D5-ADBB-A78E055926BD>
  STDMETHOD  SetObj,                      :ptr GUID, :LPUNKNOWN
  STDMETHOD  GetObj,                      :ptr GUID, :ptr GUID, :ptr GUID, :sdword, :ptr ptr
END_INTERFACE


BEGIN_INTERFACE ISpStreamFormat, IStream, <BED530BE-2606-4F4D-A1C0-54C5CDA5566F>
  STDMETHOD  GetFormat,                   :ptr GUID, :ptr ptr WaveFormatEx
END_INTERFACE


BEGIN_INTERFACE ISpStreamFormatConverter, ISpStreamFormat, <678A932C-EA71-4446-9B41-78FDA6280A29>
  STDMETHOD  SetBaseStream,               :ptr ISpStreamFormat, :sdword, :sdword
  STDMETHOD  GetBaseStream,               :ptr ptr ISpStreamFormat
  STDMETHOD  SetFormat,                   :ptr GUID, :ptr WaveFormatEx
  STDMETHOD  ResetSeekPosition
  STDMETHOD  ScaleConvertedToBaseOffset,  :qword, :ptr qword
  STDMETHOD  ScaleBaseToConvertedOffset,  :qword, :ptr qword
END_INTERFACE


BEGIN_INTERFACE ISpeechVoiceStatus, IDispatch, <8BE47B07-57F6-11D2-9EEE-00C04F797396>
  STDMETHOD  get_CurrentStreamNumber,     :ptr sdword
  STDMETHOD  get_LastStreamNumberQueued,  :ptr sdword
  STDMETHOD  get_LastHResult,             :ptr sdword
  STDMETHOD  get_RunningState,            :ptr SpeechRunState
  STDMETHOD  get_InputWordPosition,       :ptr sdword
  STDMETHOD  get_InputWordLength,         :ptr sdword
  STDMETHOD  get_InputSentencePosition,   :ptr sdword
  STDMETHOD  get_InputSentenceLength,     :ptr sdword
  STDMETHOD  get_LastBookmark,            :ptr BSTR
  STDMETHOD  get_LastBookmarkId,          :ptr sdword
  STDMETHOD  get_PhonemeId,               :ptr sword
  STDMETHOD  get_VisemeId,                :ptr sword
END_INTERFACE


BEGIN_INTERFACE ISpeechAudioBufferInfo, IDispatch, <11B103D8-1142-4EDF-A093-82FB3915F8CC>
  STDMETHOD  get_MinNotification,         :ptr sdword
  STDMETHOD  put_MinNotification,         :sdword
  STDMETHOD  get_BufferSize,              :ptr sdword
  STDMETHOD  put_BufferSize,              :sdword
  STDMETHOD  get_EventBias,               :ptr sdword
  STDMETHOD  put_EventBias,               :sdword
END_INTERFACE


BEGIN_INTERFACE ISpeechAudioStatus, IDispatch, <C62D9C91-7458-47F6-862D-1EF86FB0B278>
  STDMETHOD  get_FreeBufferSpace,         :ptr sdword
  STDMETHOD  get_NonBlockingIO,           :ptr sdword
  STDMETHOD  get_State,                   :ptr SpeechAudioState
  STDMETHOD  get_CurrentSeekPosition,     :ptr VARIANT
  STDMETHOD  get_CurrentDevicePosition,   :ptr VARIANT
END_INTERFACE


BEGIN_INTERFACE ISpeechTextSelectionInformation, IDispatch, <3B9C7E7A-6EEE-4DED-9092-11657279ADBE>
  STDMETHOD  put_ActiveOffset,            :sdword
  STDMETHOD  get_ActiveOffset,            :ptr sdword
  STDMETHOD  put_ActiveLength,            :sdword
  STDMETHOD  get_ActiveLength,            :ptr sdword
  STDMETHOD  put_SelectionOffset,         :sdword
  STDMETHOD  get_SelectionOffset,         :ptr sdword
  STDMETHOD  put_SelectionLength,         :sdword
  STDMETHOD  get_SelectionLength,         :ptr sdword
END_INTERFACE


BEGIN_INTERFACE ISpeechDataKey, IDispatch, <CE17C09B-4EFA-44D5-A4C9-59D9585AB0CD>
  STDMETHOD  SetBinaryValue,              :BSTR, :VARIANT
  STDMETHOD  GetBinaryValue,              :BSTR, :ptr VARIANT
  STDMETHOD  SetStringValue,              :BSTR, :BSTR
  STDMETHOD  GetStringValue,              :BSTR, :ptr BSTR
  STDMETHOD  SetLongValue,                :BSTR, :sdword
  STDMETHOD  GetLongValue,                :BSTR, :ptr sdword
  STDMETHOD  OpenKey,                     :BSTR, :ptr ptr ISpeechDataKey
  STDMETHOD  CreateKey,                   :BSTR, :ptr ptr ISpeechDataKey
  STDMETHOD  DeleteKey,                   :BSTR
  STDMETHOD  DeleteValue,                 :BSTR
  STDMETHOD  EnumKeys,                    :sdword, :ptr BSTR
  STDMETHOD  EnumValues,                  :sdword, :ptr BSTR
END_INTERFACE


BEGIN_INTERFACE ISpNotifySource, IUnknown, <5EFF4AEF-8487-11D2-961C-00C04F8EE628>
  STDMETHOD  SetNotifySink,               :ptr ISpNotifySink
  STDMETHOD  SetNotifyWindowMessage,      :HWND, :dword, :dword, :sdword
  STDMETHOD  SetNotifyCallbackFunction,   :ptr ptr , :dword, :sdword
  STDMETHOD  SetNotifyCallbackInterface,  :ptr ptr , :dword, :sdword
  STDMETHOD  SetNotifyWin32Event
  STDMETHOD  WaitForNotifyEvent,          :dword
  STDMETHOD  GetNotifyEventHandle
END_INTERFACE


BEGIN_INTERFACE ISpEventSource, ISpNotifySource, <BE7A9CCE-5F9E-11D2-960F-00C04F8EE628>
  STDMETHOD  SetInterest,                 :qword, :qword
  STDMETHOD  GetEvents,                   :dword, :ptr SPEVENT, :ptr dword
  STDMETHOD  GetInfo,                     :ptr SPEVENTSOURCEINFO
END_INTERFACE


BEGIN_INTERFACE ISpeechRecoResult, IDispatch, <ED2879CF-CED9-4EE6-A534-DE0191D5468D>
  STDMETHOD  get_RecoContext,             :ptr ptr ISpeechRecoContext
  STDMETHOD  get_Times,                   :ptr ptr ISpeechRecoResultTimes
  STDMETHOD  putref_AudioFormat,          :ptr ISpeechAudioFormat
  STDMETHOD  get_AudioFormat,             :ptr ptr ISpeechAudioFormat
  STDMETHOD  get_PhraseInfo,              :ptr ptr ISpeechPhraseInfo
  STDMETHOD  Alternates,                  :sdword, :sdword, :sdword, :ptr ptr ISpeechPhraseAlternates
  STDMETHOD  Audio,                       :sdword, :sdword, :ptr ptr ISpeechMemoryStream
  STDMETHOD  SpeakAudio,                  :sdword, :sdword, :SpeechVoiceSpeakFlags, :ptr sdword
  STDMETHOD  SaveToMemory,                :ptr VARIANT
  STDMETHOD  DiscardResultInfo,           :SpeechDiscardType
END_INTERFACE


BEGIN_INTERFACE ISpeechRecoResultTimes, IDispatch, <62B3B8FB-F6E7-41BE-BDCB-056B1C29EFC0>
  STDMETHOD  get_StreamTime,              :ptr VARIANT
  STDMETHOD  get_Length,                  :ptr VARIANT
  STDMETHOD  get_TickCount,               :ptr sdword
  STDMETHOD  get_OffsetFromStart,         :ptr VARIANT
END_INTERFACE


BEGIN_INTERFACE ISpEventSink, IUnknown, <BE7A9CC9-5F9E-11D2-960F-00C04F8EE628>
  STDMETHOD  AddEvents,                   :ptr SPEVENT, :dword
  STDMETHOD  GetEventInterest,            :ptr qword
END_INTERFACE


BEGIN_INTERFACE ISpObjectWithToken, IUnknown, <5B559F40-E952-11D2-BB91-00C04F8EE6C0>
  STDMETHOD  SetObjectToken,              :ptr ISpObjectToken
  STDMETHOD  GetObjectToken,              :ptr ptr ISpObjectToken
END_INTERFACE


BEGIN_INTERFACE ISpAudio, ISpStreamFormat, <C05C768F-FAE8-4EC2-8E07-338321C12452>
  STDMETHOD  SetState,                    :SPAUDIOSTATE, :qword
  STDMETHOD  SetFormat,                   :ptr GUID, :ptr WaveFormatEx
  STDMETHOD  GetStatus,                   :ptr SPAUDIOSTATUS
  STDMETHOD  SetBufferInfo,               :ptr SPAUDIOBUFFERINFO
  STDMETHOD  GetBufferInfo,               :ptr SPAUDIOBUFFERINFO
  STDMETHOD  GetDefaultFormat,            :ptr GUID, :ptr ptr WaveFormatEx
  STDMETHOD  EventHandle
  STDMETHOD  GetVolumeLevel,              :ptr dword
  STDMETHOD  SetVolumeLevel,              :dword
  STDMETHOD  GetBufferNotifySize,         :ptr dword
  STDMETHOD  SetBufferNotifySize,         :dword
END_INTERFACE


BEGIN_INTERFACE ISpMMSysAudio, ISpAudio, <15806F6E-1D70-4B48-98E6-3B1A007509AB>
  STDMETHOD  GetDeviceId,                 :ptr dword
  STDMETHOD  SetDeviceId,                 :dword
  STDMETHOD  GetMMHandle,                 :ptr ptr
  STDMETHOD  GetLineId,                   :ptr dword
  STDMETHOD  SetLineId,                   :dword
END_INTERFACE


BEGIN_INTERFACE ISpeechPhraseInfo, IDispatch, <961559CF-4E67-4662-8BF0-D93F1FCD61B3>
  STDMETHOD  get_LanguageId,              :ptr sdword
  STDMETHOD  get_GrammarId,               :ptr VARIANT
  STDMETHOD  get_StartTime,               :ptr VARIANT
  STDMETHOD  get_AudioStreamPosition,     :ptr VARIANT
  STDMETHOD  get_AudioSizeBytes,          :ptr sdword
  STDMETHOD  get_RetainedSizeBytes,       :ptr sdword
  STDMETHOD  get_AudioSizeTime,           :ptr sdword
  STDMETHOD  get_Rule,                    :ptr ptr ISpeechPhraseRule
  STDMETHOD  get_Properties,              :ptr ptr ISpeechPhraseProperties
  STDMETHOD  get_Elements,                :ptr ptr ISpeechPhraseElements
  STDMETHOD  get_Replacements,            :ptr ptr ISpeechPhraseReplacements
  STDMETHOD  get_EngineId,                :ptr BSTR
  STDMETHOD  get_EnginePrivateData,       :ptr VARIANT
  STDMETHOD  SaveToMemory,                :ptr VARIANT
  STDMETHOD  GetText,                     :sdword, :sdword, :BOOL, :ptr BSTR
  STDMETHOD  GetDisplayAttributes,        :sdword, :sdword, :BOOL, :ptr SpeechDisplayAttributes
END_INTERFACE


BEGIN_INTERFACE ISpeechPhraseRule, IDispatch, <A7BFE112-A4A0-48D9-B602-C313843F6964>
  STDMETHOD  get_Name,                    :ptr BSTR
  STDMETHOD  get_Id,                      :ptr sdword
  STDMETHOD  get_FirstElement,            :ptr sdword
  STDMETHOD  get_NumberOfElements,        :ptr sdword
  STDMETHOD  get_Parent,                  :ptr ptr ISpeechPhraseRule
  STDMETHOD  get_Children,                :ptr ptr ISpeechPhraseRules
  STDMETHOD  get_Confidence,              :ptr SpeechEngineConfidence
  STDMETHOD  get_EngineConfidence,        :ptr real4
END_INTERFACE


BEGIN_INTERFACE ISpeechPhraseRules, IDispatch, <9047D593-01DD-4B72-81A3-E4A0CA69F407>
  STDMETHOD  get_Count,                   :ptr sdword
  STDMETHOD  Item,                        :sdword, :ptr ptr ISpeechPhraseRule
  STDMETHOD  get__NewEnum,                :ptr LPUNKNOWN
END_INTERFACE


BEGIN_INTERFACE ISpeechAudioFormat, IDispatch, <E6E9C590-3E18-40E3-8299-061F98BDE7C7>
  STDMETHOD  get_Type,                    :ptr SpeechAudioFormatType
  STDMETHOD  put_Type,                    :SpeechAudioFormatType
  STDMETHOD  get_Guid,                    :ptr BSTR
  STDMETHOD  put_Guid,                    :BSTR
  STDMETHOD  GetWaveFormatEx,             :ptr ptr ISpeechWaveFormatEx
  STDMETHOD  SetWaveFormatEx,             :ptr ISpeechWaveFormatEx
END_INTERFACE


BEGIN_INTERFACE ISpStream, ISpStreamFormat, <12E3CCA9-7518-44C5-A5E7-BA5A79CB929E>
  STDMETHOD  SetBaseStream,               :ptr IStream, :ptr GUID, :ptr WaveFormatEx
  STDMETHOD  GetBaseStream,               :ptr ptr IStream
  STDMETHOD  BindToFile,                  :ptr word, :SPFILEMODE, :ptr GUID, :ptr WaveFormatEx, :qword
  STDMETHOD  Close
END_INTERFACE


BEGIN_INTERFACE ISpeechPhraseProperties, IDispatch, <08166B47-102E-4B23-A599-BDB98DBFD1F4>
  STDMETHOD  get_Count,                   :ptr sdword
  STDMETHOD  Item,                        :sdword, :ptr ptr ISpeechPhraseProperty
  STDMETHOD  get__NewEnum,                :ptr LPUNKNOWN
END_INTERFACE


BEGIN_INTERFACE ISpVoice, ISpEventSource, <6C44DF74-72B9-4992-A1EC-EF996E0422D4>
  STDMETHOD  SetOutput,                   :LPUNKNOWN, :sdword
  STDMETHOD  GetOutputObjectToken,        :ptr ptr ISpObjectToken
  STDMETHOD  GetOutputStream,             :ptr ptr ISpStreamFormat
  STDMETHOD  Pause_
  STDMETHOD  Resume
  STDMETHOD  SetVoice,                    :ptr ISpObjectToken
  STDMETHOD  GetVoice,                    :ptr ptr ISpObjectToken
  STDMETHOD  Speak,                       :LPWSTR, :dword, :ptr dword
  STDMETHOD  SpeakStream,                 :ptr IStream, :dword, :ptr dword
  STDMETHOD  GetStatus,                   :ptr SPVOICESTATUS, :ptr LPWSTR
  STDMETHOD  Skip,                        :LPWSTR, :sdword, :ptr dword
  STDMETHOD  SetPriority,                 :SPVPRIORITY
  STDMETHOD  GetPriority,                 :ptr SPVPRIORITY
  STDMETHOD  SetAlertBoundary,            :SPEVENTENUM
  STDMETHOD  GetAlertBoundary,            :ptr SPEVENTENUM
  STDMETHOD  SetRate,                     :sdword
  STDMETHOD  GetRate,                     :ptr sdword
  STDMETHOD  SetVolume,                   :word
  STDMETHOD  GetVolume,                   :ptr word
  STDMETHOD  WaitUntilDone,               :dword
  STDMETHOD  SetSyncSpeakTimeout,         :dword
  STDMETHOD  GetSyncSpeakTimeout,         :ptr dword
  STDMETHOD  SpeakCompleteEvent
  STDMETHOD  IsUISupported,               :ptr word, :ptr , :dword, :ptr sdword
  STDMETHOD  DisplayUI,                   :Handle, :ptr word, :ptr word, :ptr , :dword
END_INTERFACE


BEGIN_INTERFACE ISpeechPhraseProperty, IDispatch, <CE563D48-961E-4732-A2E1-378A42B430BE>
  STDMETHOD  get_Name,                    :ptr BSTR
  STDMETHOD  get_Id,                      :ptr sdword
  STDMETHOD  get_Value,                   :ptr VARIANT
  STDMETHOD  get_FirstElement,            :ptr sdword
  STDMETHOD  get_NumberOfElements,        :ptr sdword
  STDMETHOD  get_EngineConfidence,        :ptr real4
  STDMETHOD  get_Confidence,              :ptr SpeechEngineConfidence
  STDMETHOD  get_Parent,                  :ptr ptr ISpeechPhraseProperty
  STDMETHOD  get_Children,                :ptr ptr ISpeechPhraseProperties
END_INTERFACE


BEGIN_INTERFACE ISpeechPhraseElements, IDispatch, <0626B328-3478-467D-A0B3-D0853B93DDA3>
  STDMETHOD  get_Count,                   :ptr sdword
  STDMETHOD  Item,                        :sdword, :ptr ptr ISpeechPhraseElement
  STDMETHOD  get__NewEnum,                :ptr LPUNKNOWN
END_INTERFACE


BEGIN_INTERFACE ISpeechPhraseElement, IDispatch, <E6176F96-E373-4801-B223-3B62C068C0B4>
  STDMETHOD  get_AudioTimeOffset,         :ptr sdword
  STDMETHOD  get_AudioSizeTime,           :ptr sdword
  STDMETHOD  get_AudioStreamOffset,       :ptr sdword
  STDMETHOD  get_AudioSizeBytes,          :ptr sdword
  STDMETHOD  get_RetainedStreamOffset,    :ptr sdword
  STDMETHOD  get_RetainedSizeBytes,       :ptr sdword
  STDMETHOD  get_DisplayText,             :ptr BSTR
  STDMETHOD  get_LexicalForm,             :ptr BSTR
  STDMETHOD  get_Pronunciation,           :ptr VARIANT
  STDMETHOD  get_DisplayAttributes,       :ptr SpeechDisplayAttributes
  STDMETHOD  get_RequiredConfidence,      :ptr SpeechEngineConfidence
  STDMETHOD  get_ActualConfidence,        :ptr SpeechEngineConfidence
  STDMETHOD  get_EngineConfidence,        :ptr real4
END_INTERFACE


BEGIN_INTERFACE ISpeechObjectTokens, IDispatch, <9285B776-2E7B-4BC0-B53E-580EB6FA967F>
  STDMETHOD  get_Count,                   :ptr sdword
  STDMETHOD  Item,                        :sdword, :ptr ptr ISpeechObjectToken
  STDMETHOD  get__NewEnum,                :ptr LPUNKNOWN
END_INTERFACE


BEGIN_INTERFACE ISpRecoContext, ISpEventSource, <F740A62F-7C15-489E-8234-940A33D9272D>
  STDMETHOD  GetRecognizer,               :ptr ptr ISpRecognizer
  STDMETHOD  CreateGrammar,               :qword, :ptr ptr ISpRecoGrammar
  STDMETHOD  GetStatus,                   :ptr SPRECOCONTEXTSTATUS
  STDMETHOD  GetMaxAlternates,            :ptr dword
  STDMETHOD  SetMaxAlternates,            :dword
  STDMETHOD  SetAudioOptions,             :SPAUDIOOPTIONS, :ptr GUID, :ptr WaveFormatEx
  STDMETHOD  GetAudioOptions,             :ptr SPAUDIOOPTIONS, :ptr GUID, :ptr ptr WaveFormatEx
  STDMETHOD  DeserializeResult,           :ptr SPSERIALIZEDRESULT, :ptr ptr ISpRecoResult
  STDMETHOD  Bookmark,                    :SPBOOKMARKOPTIONS, :qword, :sdword
  STDMETHOD  SetAdaptationData,           :LPWSTR, :dword
  STDMETHOD  Pause_,                       :dword
  STDMETHOD  Resume,                      :dword
  STDMETHOD  SetVoice,                    :ptr ISpVoice, :sdword
  STDMETHOD  GetVoice,                    :ptr ptr ISpVoice
  STDMETHOD  SetVoicePurgeEvent,          :qword
  STDMETHOD  GetVoicePurgeEvent,          :ptr qword
  STDMETHOD  SetContextState,             :SPCONTEXTSTATE
  STDMETHOD  GetContextState,             :ptr SPCONTEXTSTATE
END_INTERFACE


BEGIN_INTERFACE ISpProperties, IUnknown, <5B4FB971-B115-4DE1-AD97-E482E3BF6EE4>
  STDMETHOD  SetPropertyNum,              :ptr word, :sdword
  STDMETHOD  GetPropertyNum,              :ptr word, :ptr sdword
  STDMETHOD  SetPropertyString,           :ptr word, :ptr word
  STDMETHOD  GetPropertyString,           :ptr word, :ptr ptr word
END_INTERFACE


BEGIN_INTERFACE ISpRecognizer, ISpProperties, <C2B5F241-DAA0-4507-9E16-5A1EAA2B7A5C>
  STDMETHOD  SetRecognizer,               :ptr ISpObjectToken
  STDMETHOD  GetRecognizer,               :ptr ptr ISpObjectToken
  STDMETHOD  SetInput,                    :LPUNKNOWN, :sdword
  STDMETHOD  GetInputObjectToken,         :ptr ptr ISpObjectToken
  STDMETHOD  GetInputStream,              :ptr ptr ISpStreamFormat
  STDMETHOD  CreateRecoContext,           :ptr ptr ISpRecoContext
  STDMETHOD  GetRecoProfile,              :ptr ptr ISpObjectToken
  STDMETHOD  SetRecoProfile,              :ptr ISpObjectToken
  STDMETHOD  IsSharedInstance
  STDMETHOD  GetRecoState,                :ptr SPRECOSTATE
  STDMETHOD  SetRecoState,                :SPRECOSTATE
  STDMETHOD  GetStatus,                   :ptr SPRECOGNIZERSTATUS
  STDMETHOD  GetFormat,                   :SPSTREAMFORMATTYPE, :ptr GUID, :ptr ptr WaveFormatEx
  STDMETHOD  IsUISupported,               :ptr word, :ptr , :dword, :ptr sdword
  STDMETHOD  DisplayUI,                   :Handle, :ptr word, :ptr word, :ptr , :dword
  STDMETHOD  EmulateRecognition,          :ptr ISpPhrase
END_INTERFACE


BEGIN_INTERFACE ISpeechPhraseReplacements, IDispatch, <38BC662F-2257-4525-959E-2069D2596C05>
  STDMETHOD  get_Count,                   :ptr sdword
  STDMETHOD  Item,                        :sdword, :ptr ptr ISpeechPhraseReplacement
  STDMETHOD  get__NewEnum,                :ptr LPUNKNOWN
END_INTERFACE


BEGIN_INTERFACE ISpeechPhraseReplacement, IDispatch, <2890A410-53A7-4FB5-94EC-06D4998E3D02>
  STDMETHOD  get_DisplayAttributes,       :ptr SpeechDisplayAttributes
  STDMETHOD  get_Text,                    :ptr BSTR
  STDMETHOD  get_FirstElement,            :ptr sdword
  STDMETHOD  get_NumberOfElements,        :ptr sdword
END_INTERFACE


BEGIN_INTERFACE ISpeechPhraseAlternates, IDispatch, <B238B6D5-F276-4C3D-A6C1-2974801C3CC2>
  STDMETHOD  get_Count,                   :ptr sdword
  STDMETHOD  Item,                        :sdword, :ptr ptr ISpeechPhraseAlternate
  STDMETHOD  get__NewEnum,                :ptr LPUNKNOWN
END_INTERFACE


BEGIN_INTERFACE ISpeechPhraseAlternate, IDispatch, <27864A2A-2B9F-4CB8-92D3-0D2722FD1E73>
  STDMETHOD  get_RecoResult,              :ptr ptr ISpeechRecoResult
  STDMETHOD  get_StartElementInResult,    :ptr sdword
  STDMETHOD  get_NumberOfElementsInResult, :ptr sdword
  STDMETHOD  get_PhraseInfo,              :ptr ptr ISpeechPhraseInfo
  STDMETHOD  Commit
END_INTERFACE


BEGIN_INTERFACE ISpPhrase, IUnknown, <1A5C0354-B621-4B5A-8791-D306ED379E53>
  STDMETHOD  GetPhrase,                   :ptr ptr SPPHRASE
  STDMETHOD  GetSerializedPhrase,         :ptr ptr SPSERIALIZEDPHRASE
  STDMETHOD  GetText,                     :dword, :dword, :sdword, :ptr ptr word, :ptr byte
  STDMETHOD  Discard,                     :dword
END_INTERFACE


BEGIN_INTERFACE ISpeechPhraseInfoBuilder, IDispatch, <3B151836-DF3A-4E0A-846C-D2ADC9334333>
  STDMETHOD  RestorePhraseFromMemory,     :ptr VARIANT, :ptr ptr ISpeechPhraseInfo
END_INTERFACE


BEGIN_INTERFACE ISpeechWaveFormatEx, IDispatch, <7A1EF0D5-1581-4741-88E4-209A49F11A10>
  STDMETHOD  get_FormatTag,               :ptr sword
  STDMETHOD  put_FormatTag,               :sword
  STDMETHOD  get_Channels,                :ptr sword
  STDMETHOD  put_Channels,                :sword
  STDMETHOD  get_SamplesPerSec,           :ptr sdword
  STDMETHOD  put_SamplesPerSec,           :sdword
  STDMETHOD  get_AvgBytesPerSec,          :ptr sdword
  STDMETHOD  put_AvgBytesPerSec,          :sdword
  STDMETHOD  get_BlockAlign,              :ptr sword
  STDMETHOD  put_BlockAlign,              :sword
  STDMETHOD  get_BitsPerSample,           :ptr sword
  STDMETHOD  put_BitsPerSample,           :sword
  STDMETHOD  get_ExtraData,               :ptr VARIANT
  STDMETHOD  put_ExtraData,               :VARIANT
END_INTERFACE


BEGIN_INTERFACE ISpeechRecognizer, IDispatch, <2D5F1C0C-BD75-4B08-9478-3B11FEA2586C>
  STDMETHOD  putref_Recognizer,           :ptr ISpeechObjectToken
  STDMETHOD  get_Recognizer,              :ptr ptr ISpeechObjectToken
  STDMETHOD  put_AllowAudioInputFormatChangesOnNextSet, BOOL
  STDMETHOD  get_AllowAudioInputFormatChangesOnNextSet, :ptr BOOL
  STDMETHOD  putref_AudioInput,           :ptr ISpeechObjectToken
  STDMETHOD  get_AudioInput,              :ptr ptr ISpeechObjectToken
  STDMETHOD  putref_AudioInputStream,     :ptr ISpeechBaseStream
  STDMETHOD  get_AudioInputStream,        :ptr ptr ISpeechBaseStream
  STDMETHOD  get_IsShared,                :ptr BOOL
  STDMETHOD  put_State,                   :SpeechRecognizerState
  STDMETHOD  get_State,                   :ptr SpeechRecognizerState
  STDMETHOD  get_Status,                  :ptr ptr ISpeechRecognizerStatus
  STDMETHOD  putref_Profile,              :ptr ISpeechObjectToken
  STDMETHOD  get_Profile,                 :ptr ptr ISpeechObjectToken
  STDMETHOD  EmulateRecognition,          :VARIANT, :ptr VARIANT, :sdword
  STDMETHOD  CreateRecoContext,           :ptr ptr ISpeechRecoContext
  STDMETHOD  GetFormat,                   :SpeechFormatType, :ptr ptr ISpeechAudioFormat
  STDMETHOD  SetPropertyNumber,           :BSTR, :sdword, :ptr BOOL
  STDMETHOD  GetPropertyNumber,           :BSTR, :ptr sdword, :ptr BOOL
  STDMETHOD  SetPropertyString,           :BSTR, :BSTR, :ptr BOOL
  STDMETHOD  GetPropertyString,           :BSTR, :ptr BSTR, :ptr BOOL
  STDMETHOD  IsUISupported,               :BSTR, :ptr VARIANT, :ptr BOOL
  STDMETHOD  DisplayUI,                   :sdword, :BSTR, :BSTR, :ptr VARIANT
  STDMETHOD  GetRecognizers,              :BSTR, :BSTR, :ptr ptr ISpeechObjectTokens
  STDMETHOD  GetAudioInputs,              :BSTR, :BSTR, :ptr ptr ISpeechObjectTokens
  STDMETHOD  GetProfiles,                 :BSTR, :BSTR, :ptr ptr ISpeechObjectTokens
END_INTERFACE


BEGIN_INTERFACE ISpeechBaseStream, IDispatch, <6450336F-7D49-4CED-8097-49D6DEE37294>
  STDMETHOD  get_Format,                  :ptr ptr ISpeechAudioFormat
  STDMETHOD  putref_Format,               :ptr ISpeechAudioFormat
  STDMETHOD  Read,                        :ptr VARIANT, :sdword, :ptr sdword
  STDMETHOD  Write,                       :VARIANT, :ptr sdword
  STDMETHOD  Seek,                        :VARIANT, :SpeechStreamSeekPositionType, :ptr VARIANT
END_INTERFACE


BEGIN_INTERFACE ISpeechRecognizerStatus, IDispatch, <BFF9E781-53EC-484E-BB8A-0E1B5551E35C>
  STDMETHOD  get_AudioStatus,             :ptr ptr ISpeechAudioStatus
  STDMETHOD  get_CurrentStreamPosition,   :ptr VARIANT
  STDMETHOD  get_CurrentStreamNumber,     :ptr sdword
  STDMETHOD  get_NumberOfActiveRules,     :ptr sdword
  STDMETHOD  get_ClsidEngine,             :ptr BSTR
  STDMETHOD  get_SupportedLanguages,      :ptr VARIANT
END_INTERFACE


BEGIN_INTERFACE ISpeechLexicon, IDispatch, <3DA7627A-C7AE-4B23-8708-638C50362C25>
  STDMETHOD  get_GenerationId,            :ptr sdword
  STDMETHOD  GetWords,                    :SpeechLexiconType, :ptr sdword, :ptr ptr ISpeechLexiconWords
  STDMETHOD  AddPronunciation,            :BSTR, :sdword, :SpeechPartOfSpeech, :BSTR
  STDMETHOD  AddPronunciationByPhoneIds,  :BSTR, :sdword, :SpeechPartOfSpeech, :ptr VARIANT
  STDMETHOD  RemovePronunciation,         :BSTR, :sdword, :SpeechPartOfSpeech, :BSTR
  STDMETHOD  RemovePronunciationByPhoneIds, :BSTR, :sdword, :SpeechPartOfSpeech, :ptr VARIANT
  STDMETHOD  GetPronunciations,           :BSTR, :sdword, :SpeechLexiconType, :ptr ptr ISpeechLexiconPronunciations
  STDMETHOD  GetGenerationChange,         :ptr sdword, :ptr ptr ISpeechLexiconWords
END_INTERFACE


BEGIN_INTERFACE ISpGrammarBuilder, IUnknown, <8137828F-591A-4A42-BE58-49EA7EBAAC68>
  STDMETHOD  ResetGrammar,                :word
  STDMETHOD  GetRule,                     :ptr word, :dword, :dword, :sdword, :ptr ptr
  STDMETHOD  ClearRule,                   :ptr
  STDMETHOD  CreateNewState,              :ptr , :ptr ptr
  STDMETHOD  AddWordTransition,           :ptr , :ptr , :ptr word, :ptr word, :SPGRAMMARWORDTYPE, :real4, :ptr SPPROPERTYINFO
  STDMETHOD  AddRuleTransition,           :ptr , :ptr , :ptr , :real4, :ptr SPPROPERTYINFO
  STDMETHOD  AddResource,                 :ptr , :ptr word, :ptr word
  STDMETHOD  Commit,                      :dword
END_INTERFACE


BEGIN_INTERFACE ISpRecoGrammar, ISpGrammarBuilder, <2177DB29-7F45-47D0-8554-067E91C80502>
  STDMETHOD  GetGrammarId,                :ptr qword
  STDMETHOD  GetRecoContext,              :ptr ptr ISpRecoContext
  STDMETHOD  LoadCmdFromFile,             :LPWSTR, :SPLOADOPTIONS
  STDMETHOD  LoadCmdFromObject,           :ptr GUID, :LPWSTR, :SPLOADOPTIONS
  STDMETHOD  LoadCmdFromResource,         :ptr , :LPWSTR, :LPWSTR, :word, :SPLOADOPTIONS
  STDMETHOD  LoadCmdFromMemory,           :ptr SPBINARYGRAMMAR, :SPLOADOPTIONS
  STDMETHOD  LoadCmdFromProprietaryGrammar, :ptr GUID, :LPWSTR, :ptr , :dword, :SPLOADOPTIONS
  STDMETHOD  SetRuleState,                :LPWSTR, :ptr , :SPRULESTATE
  STDMETHOD  SetRuleIdState,              :dword, :SPRULESTATE
  STDMETHOD  LoadDictation,               :LPWSTR, :SPLOADOPTIONS
  STDMETHOD  UnloadDictation
  STDMETHOD  SetDictationState,           :SPRULESTATE
  STDMETHOD  SetWordSequenceData,         :ptr word, :dword, :ptr SPTEXTSELECTIONINFO
  STDMETHOD  SetTextSelection,            :ptr SPTEXTSELECTIONINFO
  STDMETHOD  IsPronounceable,             :LPWSTR, :ptr SPWORDPRONOUNCEABLE
  STDMETHOD  SetGrammarState,             :SPGRAMMARSTATE
  STDMETHOD  SaveCmd,                     :ptr IStream, :ptr ptr word
  STDMETHOD  GetGrammarState,             :ptr SPGRAMMARSTATE
END_INTERFACE


BEGIN_INTERFACE ISpeechRecoContext, IDispatch, <580AA49D-7E1E-4809-B8E2-57DA806104B8>
  STDMETHOD  get_Recognizer,              :ptr ptr ISpeechRecognizer
  STDMETHOD  get_AudioInputInterferenceStatus,  :ptr SpeechInterference
  STDMETHOD  get_RequestedUIType,         :ptr BSTR
  STDMETHOD  putref_Voice,                :ptr ISpeechVoice
  STDMETHOD  get_Voice,                   :ptr ptr ISpeechVoice
  STDMETHOD  put_AllowVoiceFormatMatchingOnNextSet, :BOOL
  STDMETHOD  get_AllowVoiceFormatMatchingOnNextSet, :ptr BOOL
  STDMETHOD  put_VoicePurgeEvent,         :SpeechRecoEvents
  STDMETHOD  get_VoicePurgeEvent,         :ptr SpeechRecoEvents
  STDMETHOD  put_EventInterests,          :SpeechRecoEvents
  STDMETHOD  get_EventInterests,          :ptr SpeechRecoEvents
  STDMETHOD  put_CmdMaxAlternates,        :sdword
  STDMETHOD  get_CmdMaxAlternates,        :ptr sdword
  STDMETHOD  put_State,                   :SpeechRecoContextState
  STDMETHOD  get_State,                   :ptr SpeechRecoContextState
  STDMETHOD  put_RetainedAudio,           :SpeechRetainedAudioOptions
  STDMETHOD  get_RetainedAudio,           :ptr SpeechRetainedAudioOptions
  STDMETHOD  putref_RetainedAudioFormat,  :ptr ISpeechAudioFormat
  STDMETHOD  get_RetainedAudioFormat,     :ptr ptr ISpeechAudioFormat
  STDMETHOD  Pause_
  STDMETHOD  Resume
  STDMETHOD  CreateGrammar,               :VARIANT, :ptr ptr ISpeechRecoGrammar
  STDMETHOD  CreateResultFromMemory,      :ptr VARIANT, :ptr ptr ISpeechRecoResult
  STDMETHOD  Bookmark,                    :SpeechBookmarkOptions, :VARIANT, :VARIANT
  STDMETHOD  SetAdaptationData,           :BSTR
END_INTERFACE


BEGIN_INTERFACE ISpeechLexiconWords, IDispatch, <8D199862-415E-47D5-AC4F-FAA608B424E6>
  STDMETHOD  get_Count,                   :ptr sdword
  STDMETHOD  Item,                        :sdword, :ptr ptr ISpeechLexiconWord
  STDMETHOD  get__NewEnum,                :ptr LPUNKNOWN
END_INTERFACE


BEGIN_INTERFACE ISpeechLexiconWord, IDispatch, <4E5B933C-C9BE-48ED-8842-1EE51BB1D4FF>
  STDMETHOD  get_LangId,                  :ptr sdword
  STDMETHOD  get_Type,                    :ptr SpeechWordType
  STDMETHOD  get_Word,                    :ptr BSTR
  STDMETHOD  get_Pronunciations,          :ptr ptr ISpeechLexiconPronunciations
END_INTERFACE


BEGIN_INTERFACE ISpeechObjectToken, IDispatch, <C74A3ADC-B727-4500-A84A-B526721C8B8C>
  STDMETHOD  get_Id,                      :ptr BSTR
  STDMETHOD  get_DataKey,                 :ptr ptr ISpeechDataKey
  STDMETHOD  get_Category,                :ptr ptr ISpeechObjectTokenCategory
  STDMETHOD  GetDescription,              :sdword, :ptr BSTR
  STDMETHOD  SetId,                       :BSTR, :BSTR, :BOOL
  STDMETHOD  GetAttribute,                :BSTR, :ptr BSTR
  STDMETHOD  CreateInstance,              :LPUNKNOWN, :SpeechTokenContext, :ptr LPUNKNOWN
  STDMETHOD  Remove,                      :BSTR
  STDMETHOD  GetStorageFileName,          :BSTR, :BSTR, :BSTR, :SpeechTokenShellFolder, :ptr BSTR
  STDMETHOD  RemoveStorageFileName,       :BSTR, :BSTR, :BOOL
  STDMETHOD  IsUISupported,               :BSTR, :ptr VARIANT, :LPUNKNOWN, :ptr BOOL
  STDMETHOD  DisplayUI,                   :sdword, :BSTR, :BSTR, :ptr VARIANT, :LPUNKNOWN
  STDMETHOD  MatchesAttributes,           :BSTR, :ptr BOOL
END_INTERFACE


BEGIN_INTERFACE ISpeechLexiconPronunciations, IDispatch, <72829128-5682-4704-A0D4-3E2BB6F2EAD3>
  STDMETHOD  get_Count,                   :ptr sdword
  STDMETHOD  Item,                        :sdword, :ptr ptr ISpeechLexiconPronunciation
  STDMETHOD  get__NewEnum,                :ptr LPUNKNOWN
END_INTERFACE


BEGIN_INTERFACE ISpeechLexiconPronunciation, IDispatch, <95252C5D-9E43-4F4A-9899-48EE73352F9F>
  STDMETHOD  get_Type,                    :ptr SpeechLexiconType
  STDMETHOD  get_LangId,                  :ptr sdword
  STDMETHOD  get_PartOfSpeech,            :ptr SpeechPartOfSpeech
  STDMETHOD  get_PhoneIds,                :ptr VARIANT
  STDMETHOD  get_Symbolic,                :ptr BSTR
END_INTERFACE


BEGIN_INTERFACE ISpeechAudio, ISpeechBaseStream, <CFF8E175-019E-11D3-A08E-00C04F8EF9B5>
  STDMETHOD  get_Status,                  :ptr ptr ISpeechAudioStatus
  STDMETHOD  get_BufferInfo,              :ptr ptr ISpeechAudioBufferInfo
  STDMETHOD  get_DefaultFormat,           :ptr ptr ISpeechAudioFormat
  STDMETHOD  get_Volume,                  :ptr sdword
  STDMETHOD  put_Volume,                  :sdword
  STDMETHOD  get_BufferNotifySize,        :ptr sdword
  STDMETHOD  put_BufferNotifySize,        :sdword
  STDMETHOD  get_EventHandle,             :ptr sdword
  STDMETHOD  SetState,                    :SpeechAudioState
END_INTERFACE


BEGIN_INTERFACE ISpeechObjectTokenCategory, IDispatch, <CA7EAC50-2D01-4145-86D4-5AE7D70F4469>
  STDMETHOD  get_Id,                      :ptr BSTR
  STDMETHOD  put_Default,                 :BSTR
  STDMETHOD  get_Default,                 :ptr BSTR
  STDMETHOD  SetId,                       :BSTR, :BOOL
  STDMETHOD  GetDataKey,                  :SpeechDataKeyLocation, :ptr ptr ISpeechDataKey
  STDMETHOD  EnumerateTokens,             :BSTR, :BSTR, :ptr ptr ISpeechObjectTokens
END_INTERFACE


BEGIN_INTERFACE ISpeechFileStream, ISpeechBaseStream, <AF67F125-AB39-4E93-B4A2-CC2E66E182A7>
  STDMETHOD  Open,                        :BSTR, :SpeechStreamFileMode, :BOOL
  STDMETHOD  Close
END_INTERFACE


BEGIN_INTERFACE ISpeechRecoGrammar, IDispatch, <B6D6F79F-2158-4E50-B5BC-9A9CCD852A09>
  STDMETHOD  get_Id,                      :ptr VARIANT
  STDMETHOD  get_RecoContext,             :ptr ptr ISpeechRecoContext
  STDMETHOD  put_State,                   :SpeechGrammarState
  STDMETHOD  get_State,                   :ptr SpeechGrammarState
  STDMETHOD  get_Rules,                   :ptr ptr ISpeechGrammarRules
  STDMETHOD  Reset,                       :sdword
  STDMETHOD  CmdLoadFromFile,             :BSTR, :SpeechLoadOption
  STDMETHOD  CmdLoadFromObject,           :BSTR, :BSTR, :SpeechLoadOption
  STDMETHOD  CmdLoadFromResource,         :sdword, :VARIANT, :VARIANT, :sdword, :SpeechLoadOption
  STDMETHOD  CmdLoadFromMemory,           :VARIANT, :SpeechLoadOption
  STDMETHOD  CmdLoadFromProprietaryGrammar, :BSTR, :BSTR, :VARIANT, :SpeechLoadOption
  STDMETHOD  CmdSetRuleState,             :BSTR, :SpeechRuleState
  STDMETHOD  CmdSetRuleIdState,           :sdword, :SpeechRuleState
  STDMETHOD  DictationLoad,               :BSTR, :SpeechLoadOption
  STDMETHOD  DictationUnload
  STDMETHOD  DictationSetState,           :SpeechRuleState
  STDMETHOD  SetWordSequenceData,         :BSTR, :sdword, :ptr ISpeechTextSelectionInformation
  STDMETHOD  SetTextSelection,            :ptr ISpeechTextSelectionInformation
  STDMETHOD  IsPronounceable,             :BSTR, :ptr SpeechWordPronounceable
END_INTERFACE


BEGIN_INTERFACE ISpeechMemoryStream, ISpeechBaseStream, <EEB14B68-808B-4ABE-A5EA-B51DA7588008>
  STDMETHOD  SetData,                     :VARIANT
  STDMETHOD  GetData,                     :ptr VARIANT
END_INTERFACE


BEGIN_INTERFACE ISpeechGrammarRules, IDispatch, <6FFA3B44-FC2D-40D1-8AFC-32911C7F1AD1>
  STDMETHOD  get_Count,                   :ptr sdword
  STDMETHOD  FindRule,                    :VARIANT, :ptr ptr ISpeechGrammarRule
  STDMETHOD  Item,                        :sdword, :ptr ptr ISpeechGrammarRule
  STDMETHOD  get__NewEnum,                :ptr LPUNKNOWN
  STDMETHOD  get_Dynamic,                 :ptr BOOL
  STDMETHOD  Add_,                        :BSTR, :SpeechRuleAttributes, :sdword, :ptr ptr ISpeechGrammarRule
  STDMETHOD  Commit
  STDMETHOD  CommitAndSave,               :ptr BSTR, :ptr VARIANT
END_INTERFACE


BEGIN_INTERFACE ISpeechGrammarRule, IDispatch, <AFE719CF-5DD1-44F2-999C-7A399F1CFCCC>
  STDMETHOD  get_Attributes,              :ptr SpeechRuleAttributes
  STDMETHOD  get_InitialState,            :ptr ptr ISpeechGrammarRuleState
  STDMETHOD  get_Name,                    :ptr BSTR
  STDMETHOD  get_Id,                      :ptr sdword
  STDMETHOD  Clear
  STDMETHOD  AddResource,                 :BSTR, :BSTR
  STDMETHOD  AddState,                    :ptr ptr ISpeechGrammarRuleState
END_INTERFACE


BEGIN_INTERFACE ISpeechPhoneConverter, IDispatch, <C3E4F353-433F-43D6-89A1-6A62A7054C3D>
  STDMETHOD  get_LanguageId,              :ptr sdword
  STDMETHOD  put_LanguageId,              :sdword
  STDMETHOD  PhoneToId,                   :BSTR, :ptr VARIANT
  STDMETHOD  IdToPhone,                   :VARIANT, :ptr BSTR
END_INTERFACE


BEGIN_INTERFACE ISpRecoResult, ISpPhrase, <20B053BE-E235-43CD-9A2A-8D17A48B7842>
  STDMETHOD  GetResultTimes,              :ptr SPRECORESULTTIMES
  STDMETHOD  GetAlternates,               :dword, :dword, :dword, :ptr ptr ISpPhraseAlt, :ptr dword
  STDMETHOD  GetAudio,                    :dword, :dword, :ptr ptr ISpStreamFormat
  STDMETHOD  SpeakAudio,                  :dword, :dword, :dword, :ptr dword
  STDMETHOD  Serialize,                   :ptr ptr SPSERIALIZEDRESULT
  STDMETHOD  ScaleAudio,                  :ptr GUID, :ptr WaveFormatEx
  STDMETHOD  GetRecoContext,              :ptr ptr ISpRecoContext
END_INTERFACE


BEGIN_INTERFACE ISpNotifySink, IUnknown, <259684DC-37C3-11D2-9603-00C04F8EE628>
  STDMETHOD  Notify
END_INTERFACE


BEGIN_INTERFACE ISpPhraseAlt, ISpPhrase, <8FCEBC98-4E49-4067-9C6C-D86A0E092E3D>
  STDMETHOD  GetAltInfo,                  :ptr ptr ISpPhrase, :ptr dword, :ptr dword, :ptr dword
  STDMETHOD  Commit
END_INTERFACE


BEGIN_INTERFACE ISpNotifyTranslator, ISpNotifySink, <ACA16614-5D3D-11D2-960E-00C04F8EE628>
  STDMETHOD  InitWindowMessage,           :Handle, :dword, :dword, :sdword
  STDMETHOD  InitCallback,                :ptr ptr , :dword, :sdword
  STDMETHOD  InitSpNotifyCallback,        :ptr ptr , :dword, :sdword
  STDMETHOD  InitWin32Event,              :ptr , :sdword
  STDMETHOD  Wait_,                       :dword
  STDMETHOD  GetEventHandle
END_INTERFACE


BEGIN_INTERFACE ISpeechCustomStream, ISpeechBaseStream, <1A9E9F4F-104F-4DB8-A115-EFD7FD0C97AE>
  STDMETHOD  get_BaseStream,              :ptr LPUNKNOWN
  STDMETHOD  putref_BaseStream,           :LPUNKNOWN
END_INTERFACE


BEGIN_INTERFACE ISpLexicon, IUnknown, <DA41A7C2-5383-4DB2-916B-6C1719E3DB58>
  STDMETHOD  GetPronunciations,           :ptr word, :word, :dword, :ptr SPWORDPRONUNCIATIONLIST
  STDMETHOD  AddPronunciation,            :ptr word, :word, :SPPARTOFSPEECH, :ptr word
  STDMETHOD  RemovePronunciation,         :ptr word, :word, :SPPARTOFSPEECH, :ptr word
  STDMETHOD  GetGeneration,               :ptr dword
  STDMETHOD  GetGenerationChange,         :dword, :ptr dword, :ptr SPWORDLIST
  STDMETHOD  GetWords,                    :dword, :ptr dword, :ptr dword, :ptr SPWORDLIST
END_INTERFACE


BEGIN_INTERFACE ISpeechGrammarRuleState, IDispatch, <D4286F2C-EE67-45AE-B928-28D695362EDA>
  STDMETHOD  get_Rule,                    :ptr ptr ISpeechGrammarRule
  STDMETHOD  get_Transitions,             :ptr ptr ISpeechGrammarRuleStateTransitions
  STDMETHOD  AddWordTransition,           :ptr ISpeechGrammarRuleState, :BSTR, :BSTR, :SpeechGrammarWordType, :BSTR, :sdword, :ptr VARIANT, :real4
  STDMETHOD  AddRuleTransition,           :ptr ISpeechGrammarRuleState, :ptr ISpeechGrammarRule, :BSTR, :sdword, :ptr VARIANT, :real4
  STDMETHOD  AddSpecialTransition,        :ptr ISpeechGrammarRuleState, :SpeechSpecialTransitionType, :BSTR, :sdword, :ptr VARIANT, :real4
END_INTERFACE


BEGIN_INTERFACE ISpeechGrammarRuleStateTransitions, IDispatch, <EABCE657-75BC-44A2-AA7F-C56476742963>
  STDMETHOD  get_Count,                   :ptr sdword
  STDMETHOD  Item,                        :sdword, :ptr ptr ISpeechGrammarRuleStateTransition
  STDMETHOD  get__NewEnum,                :ptr LPUNKNOWN
END_INTERFACE


BEGIN_INTERFACE ISpeechGrammarRuleStateTransition, IDispatch, <CAFD1DB1-41D1-4A06-9863-E2E81DA17A9A>
  STDMETHOD  get_Type,                    :ptr SpeechGrammarRuleStateTransitionType
  STDMETHOD  get_Text,                    :ptr BSTR
  STDMETHOD  get_Rule,                    :ptr ptr ISpeechGrammarRule
  STDMETHOD  get_Weight,                  :ptr VARIANT
  STDMETHOD  get_PropertyName,            :ptr BSTR
  STDMETHOD  get_PropertyId,              :ptr sdword
  STDMETHOD  get_PropertyValue,           :ptr VARIANT
  STDMETHOD  get_NextState,               :ptr ptr ISpeechGrammarRuleState
END_INTERFACE


BEGIN_INTERFACE ISpeechMMSysAudio, ISpeechAudio, <3C76AF6D-1FD7-4831-81D1-3B71D5A13C44>
  STDMETHOD  get_DeviceId,                :ptr sdword
  STDMETHOD  put_DeviceId,                :sdword
  STDMETHOD  get_LineId,                  :ptr sdword
  STDMETHOD  put_LineId,                  :sdword
  STDMETHOD  get_MMHandle,                :ptr sdword
END_INTERFACE


BEGIN_INTERFACE ISpObjectTokenCategory, ISpDataKey, <2D3D3845-39AF-4850-BBF9-40B49780011D>
  STDMETHOD  SetId,                       :ptr word, :sdword
  STDMETHOD  GetId,                       :ptr ptr word
  STDMETHOD  GetDataKey,                  :SPDATAKEYLOCATION, :ptr ptr ISpDataKey
  STDMETHOD  EnumTokens,                  :LPWSTR, :LPWSTR, :ptr ptr IEnumSpObjectTokens
  STDMETHOD  SetDefaultTokenId,           :ptr word
  STDMETHOD  GetDefaultTokenId,           :ptr ptr word
END_INTERFACE


BEGIN_INTERFACE ISpObjectToken, ISpDataKey, <14056589-E16C-11D2-BB90-00C04F8EE6C0>
  STDMETHOD  SetId,                       :ptr word, :ptr word, :sdword
  STDMETHOD  GetId,                       :ptr ptr word
  STDMETHOD  GetCategory,                 :ptr ptr ISpObjectTokenCategory
  STDMETHOD  CreateInstance,              :LPUNKNOWN, :dword, :ptr GUID, :ptr ptr
  STDMETHOD  GetStorageFileName,          :ptr GUID, :ptr word, :ptr word, :dword, :ptr ptr word
  STDMETHOD  RemoveStorageFileName,       :ptr GUID, :ptr word, :sdword
  STDMETHOD  Remove,                      :ptr GUID
  STDMETHOD  IsUISupported,               :ptr word, :ptr , :dword, :LPUNKNOWN, :ptr sdword
  STDMETHOD  DisplayUI,                   :Handle, :ptr word, :ptr word, :ptr , :dword, :LPUNKNOWN
  STDMETHOD  MatchesAttributes,           :ptr word, :ptr sdword
END_INTERFACE


BEGIN_INTERFACE ISpeechVoice, IDispatch, <269316D8-57BD-11D2-9EEE-00C04F797396>
  STDMETHOD  get_Status,                  :ptr ptr ISpeechVoiceStatus
  STDMETHOD  get_Voice,                   :ptr ptr ISpeechObjectToken
  STDMETHOD  putref_Voice,                :ptr ISpeechObjectToken
  STDMETHOD  get_AudioOutput,             :ptr ptr ISpeechObjectToken
  STDMETHOD  putref_AudioOutput,          :ptr ISpeechObjectToken
  STDMETHOD  get_AudioOutputStream,       :ptr ptr ISpeechBaseStream
  STDMETHOD  putref_AudioOutputStream,    :ptr ISpeechBaseStream
  STDMETHOD  get_Rate,                    :ptr sdword
  STDMETHOD  put_Rate,                    :sdword
  STDMETHOD  get_Volume,                  :ptr sdword
  STDMETHOD  put_Volume,                  :sdword
  STDMETHOD  put_AllowAudioOutputFormatChangesOnNextSet,  :BOOL
  STDMETHOD  get_AllowAudioOutputFormatChangesOnNextSet,  :ptr BOOL
  STDMETHOD  get_EventInterests,          :ptr SpeechVoiceEvents
  STDMETHOD  put_EventInterests,          :SpeechVoiceEvents
  STDMETHOD  put_Priority,                :SpeechVoicePriority
  STDMETHOD  get_Priority,                :ptr SpeechVoicePriority
  STDMETHOD  put_AlertBoundary,           :SpeechVoiceEvents
  STDMETHOD  get_AlertBoundary,           :ptr SpeechVoiceEvents
  STDMETHOD  put_SynchronousSpeakTimeout, :sdword
  STDMETHOD  get_SynchronousSpeakTimeout, :ptr sdword
  STDMETHOD  Speak,                       :BSTR, :SpeechVoiceSpeakFlags, :ptr sdword
  STDMETHOD  SpeakStream,                 :ptr ISpeechBaseStream, :SpeechVoiceSpeakFlags, :ptr sdword
  STDMETHOD  Pause_
  STDMETHOD  Resume
  STDMETHOD  Skip,                        :BSTR, :sdword, :ptr sdword
  STDMETHOD  GetVoices,                   :BSTR, :BSTR, :ptr ptr ISpeechObjectTokens
  STDMETHOD  GetAudioOutputs,             :BSTR, :BSTR, :ptr ptr ISpeechObjectTokens
  STDMETHOD  WaitUntilDone,               :sdword, :ptr BOOL
  STDMETHOD  SpeakCompleteEvent,          :ptr sdword
  STDMETHOD  IsUISupported,               :BSTR, :ptr VARIANT, :ptr BOOL
  STDMETHOD  DisplayUI,                   :sdword, :BSTR, :BSTR, :ptr VARIANT
END_INTERFACE


BEGIN_INTERFACE ISpPhoneConverter, ISpObjectWithToken, <8445C581-0CAC-4A38-ABFE-9B2CE2826455>
  STDMETHOD  PhoneToId,                   :ptr word, :ptr word
  STDMETHOD  IdToPhone,                   :ptr word, :ptr word
END_INTERFACE


BEGIN_INTERFACE _ISpeechRecoContextEvents, IDispatch, <7B8FCB42-0E9D-4F00-A048-7B04D6179D3D>
  DISPMETHOD  StartStream,                :sdword, :VARIANT
  DISPMETHOD  EndStream,                  :sdword, :VARIANT, :BOOL
  DISPMETHOD  Bookmark,                   :sdword, :VARIANT, :VARIANT, :SpeechBookmarkOptions
  DISPMETHOD  SoundStart,                 :sdword, :VARIANT
  DISPMETHOD  SoundEnd,                   :sdword, :VARIANT
  DISPMETHOD  PhraseStart,                :sdword, :VARIANT
  DISPMETHOD  Recognition,                :sdword, :VARIANT, :SpeechRecognitionType, :ptr ISpeechRecoResult
  DISPMETHOD  Hypothesis,                 :sdword, :VARIANT, :ptr ISpeechRecoResult
  DISPMETHOD  PropertyNumberChange,       :sdword, :VARIANT, :BSTR, :sdword
  DISPMETHOD  PropertyStringChange,       :sdword, :VARIANT, :BSTR, :BSTR
  DISPMETHOD  FalseRecognition,           :sdword, :VARIANT, :ptr ISpeechRecoResult
  DISPMETHOD  Interference,               :sdword, :VARIANT, :SpeechInterference
  DISPMETHOD  RequestUI,                  :sdword, :VARIANT, :BSTR
  DISPMETHOD  RecognizerStateChange,      :sdword, :VARIANT, :SpeechRecognizerState
  DISPMETHOD  Adaptation,                 :sdword, :VARIANT
  DISPMETHOD  RecognitionForOtherContext, :sdword, :VARIANT
  DISPMETHOD  AudioLevel,                 :sdword, :VARIANT, :sdword
  DISPMETHOD  EnginePrivate,              :sdword, :VARIANT, :VARIANT
END_INTERFACE


BEGIN_INTERFACE _ISpeechVoiceEvents, IDispatch, <A372ACD1-3BEF-4BBD-8FFB-CB3E2B416AF8>
  DISPMETHOD  StartStream,                :sdword, :VARIANT
  DISPMETHOD  EndStream,                  :sdword, :VARIANT
  DISPMETHOD  VoiceChange,                :sdword, :VARIANT, :ptr ISpeechObjectToken
  DISPMETHOD  Bookmark,                   :sdword, :VARIANT, :BSTR, :sdword
  DISPMETHOD  _Word,                      :sdword, :VARIANT, :sdword, :sdword
  DISPMETHOD  Sentence,                   :sdword, :VARIANT, :sdword, :sdword
  DISPMETHOD  Phoneme,                    :sdword, :VARIANT, :sdword, :sword, :SpeechVisemeFeature, :sword
  DISPMETHOD  Viseme,                     :sdword, :VARIANT, :sdword, :SpeechVisemeType, :SpeechVisemeFeature, :SpeechVisemeType
  DISPMETHOD  AudioLevel,                 :sdword, :VARIANT, :sdword
  DISPMETHOD  EnginePrivate,              :sdword, :sdword, :VARIANT
END_INTERFACE
