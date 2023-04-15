; ==================================================================================================
; Title:   OcxContainer.inc
; Author:  G. Friedrich
; Version: 1.0.0
; Purpose: Active X Control Container object using ObjAsm32.
; Notes:   Version 1.0.0, June 2009
;            - First release.
; ==================================================================================================

; Links:
; ——————
;http://msdn.microsoft.com/en-us/library/ms686619(VS.85).aspx     Containers
;http://msdn.microsoft.com/en-us/library/ms691268(VS.85).aspx     Interfaces
;ATL71.DLL is distributed with the free .NET Framework 1.1.
;http://msdn.microsoft.com/en-us/library/ms679718(VS.85).aspx     Ambient properties
;http://www.mactech.com/articles/mactech/Vol.11/11.01/OLEIn-PlaceActivation/index.html


;                    OCX Container                                        OCX Control
;                                                                             1..n
;                                  ———                                                ———
;                                 |   |                                              |   |
;                                  ———                                                ———
;                                   |                                                  |
;      ————————————————————————————————————                        ———————————————————————————
;     |                             |      |                      |                 IUnknown  |
;     |       OCX Client Site 1..n  |      |                      |                           |
;     |       ———————————————————————————  |              ———     |                           |
;     |     —|                  IUnknown | |             |   |————| IOleObject                |
;     |   —| |                    Site   | |              ———     |                           |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | | IOleClientSite            |—|——|   |      |   |————| IOleInPlaceObject         |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | | IAdviseSink               |—|——|   |      |   |————| IOleInPlaceActiveObject   |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | | IOleInPlaceSite           |—|——|   |      |   |————| IOleControl               |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | | IOleControlSite           |—|——|   |      |   |————| IDataObject               |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | | IOleContainer             |—|——|   |      |   |————| IViewObject2              |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | | IDispatch                 |—|——|   |      |   |————| IDispatch                 |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | | ISimpleFrameSite          |—|——|   |      |   |————| IConnectionPointContainer |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | | IPropertyNotifySink       |—|——|   |      |   |————| IProvideClassInfo(2)      |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | | IErrorInfo                |—|——|   |      |   |————| ISpecifyPropertyPages     |
;     |  | | |                           | |   ———        ———     |                           |
;     |  | | |                           | |              ———     |                           |
;     |  | | |                           | |             |   |————| IPersist...  (*)          |
;     |  | | |                           | |              ———     |                           |
;     |  | | '———————————————————————————' |              ———     |                           |
;     |  | '————————————————————————————'  |             |   |————| IOleCache(2)              |
;     |  '————————————————————————————'    |              ———     |                           |
;     |                                    |              ———     |                           |
;     |                                    |             |   |————| IExternalConnection       |
;     '————————————————————————————————————'              ———     |                           |
;                                                         ———     |                           |
;                                                        |   |————| IRunnableObject           |
;                                                         ———     |                           |
;                                                         ———     |                           |
;                                                        |   |————| IClassFactory2            |
;                                                         ———     |                           |
;                                                                 |                           |
;                                                                 '———————————————————————————'
;
;
; (*) Persistence Interfaces: IPersistStream, IPersistStreamInit, IPersistMemory, IPersistStorage,
;                             IPersistMoniker, IPersistPropertyBag
;
;
;
;
; In-Place window model
; —————————————————————
;
;             Frame
;             ———————————————————————————————————————————————————————————
;            |    Document                                               |
;            |    —————————————————————————————————————————————————————  |
;            |   |    Parent                                           | |
;            |   |    ———————————————————————————————————————————————  | |
;            |   |   |    In-Place Objects                           | | |
;            |   |   |    —————————————————————————————————————————  | | |
;            |   |   |   |                                         | | | |
;            |   |   |   |                                         | | | |
;            |   |   |   |                                         | | | |
;            |   |   |   |                                         | | | |
;            |   |   |   |                                         | | | |
;            |   |   |   |                                         | | | |
;            |   |   |   |                                         | | | |
;            |   |   |   |                                         | | | |
;            |   |   |   |                                         | | | |
;            |   |   |   |                                         | | | |
;            |   |   |   |                                         | | | |
;            |   |   |   '—————————————————————————————————————————' | | |
;            |   |   '———————————————————————————————————————————————' | |
;            |   '—————————————————————————————————————————————————————' |
;            '———————————————————————————————————————————————————————————'
;


; ==================================================================================================
;     Object definitions
; ==================================================================================================

externdef IID_IOleObject:GUID
externdef IID_IOleInPlaceObject:GUID
externdef IID_IOleInPlaceActiveObject:GUID
externdef IID_IOleControl:GUID
externdef IID_IDataObject:GUID
externdef IID_IViewObject2:GUID
externdef IID_IDispatch:GUID
externdef IID_IProvideClassInfo:GUID
externdef IID_ISpecifyPropertyPages:GUID
externdef IID_IPersistStorage:GUID
externdef IID_IConnectionPoint:GUID
externdef IID_IStorage:GUID
externdef IID_IClassFactory2:GUID
externdef IID_IFontDisp:GUID

.const
DefGUID CLSID_OCXCLIENTSITE, <F85F32F6-8658-4074-8B3D-743FDA45EFE8>

SHAREFLAGS  equ   STGM_DIRECT or STGM_READWRITE or STGM_SHARE_EXCLUSIVE


;An OcxClientSite instance is created for each OcxControl and stored in a collection owned
;by OcxContainer .

Object OcxClientSite, OcxClientSiteID, Component
  VirtualMethod     Connect
  VirtualMethod     Disconnect
  VirtualMethod     DoVerb,                 dword
  VirtualMethod     GetName
  RedefineMethod    Init,                   Pointer

  ;Control provided interfaces
  DefineVariable    pIUnknown,              Pointer,        NULL
  DefineVariable    pIOleObject,            Pointer,        NULL
  DefineVariable    pIOleInPlaceObject,     Pointer,        NULL
  DefineVariable    pIOleInPlaceActiveObject,  Pointer,     NULL
  DefineVariable    pIOleControl,           Pointer,        NULL
  DefineVariable    pIDataObject,           Pointer,        NULL
  DefineVariable    pIViewObject2,          Pointer,        NULL
  DefineVariable    pIDispatch,             Pointer,        NULL
  DefineVariable    pIProvideClassInfo,     Pointer,        NULL
  DefineVariable    pISpecifyPropertyPages, Pointer,        NULL
  DefineVariable    pIPersistStorage,       Pointer,        NULL
  DefineVariable    pIPNS_ConnectionPoint,  Pointer,        NULL      ;PropertyNotifySink CP

  ;Client Site provided interfaces
  DefineVariable    pIUnknownSite,          Pointer,        NULL
  DefineVariable    pIOleClientSite,        Pointer,        NULL
  DefineVariable    pIOleInPlaceSite,       Pointer,        NULL
  DefineVariable    pIOleInPlaceSiteEx,     Pointer,        NULL
  DefineVariable    pIOleInPlaceSiteWindowless, Pointer,    NULL
  DefineVariable    pIOleControlSite,       Pointer,        NULL
  DefineVariable    pISimpleFrameSite,      Pointer,        NULL
  DefineVariable    pIOleContainer,         Pointer,        NULL
  DefineVariable    pIErrorInfo,            Pointer,        NULL
  DefineVariable    pIOleInPlaceFrame,      Pointer,        NULL
  DefineVariable    pIServiceProvider,      Pointer,        NULL
  DefineVariable    pIStorage,              Pointer,        NULL

  ;Client Site Sinks
  DefineVariable    pIAdviseSink,           Pointer,        NULL
  DefineVariable    dAS_Cookie,             dword,          0
  DefineVariable    pIPropertyNotifySink,   Pointer,        NULL
  DefineVariable    dPNS_Cookie,            dword,          0

  ;Data members
  DefineVariable    pCtrlClsID,             REFGUID,        NULL      ;CLSID of control
  DefineVariable    pName,                  Pointer,        NULL
  DefineVariable    dMiscStatus,            dword,          0         ;Control miscellaneous status
  DefineVariable    hCtrlWnd,               Handle,         0
  DefineVariable    CtrlRect,               RECT,           {0,0,0,0}
  DefineVariable    ClipRect,               RECT,           {0,0,0,0}
  DefineVariable    BorderSpace,            RECT,           {0,0,0,0}
  DefineVariable    dShow,                  BOOL,           FALSE
  DefineVariable    dInPlaceActive,         BOOL,           FALSE
  DefineVariable    dLockCount,             dword,          0

  RedefineVariable  pCLSID,   offset CLSID_OCXCLIENTSITE
  
ObjectEnd


Object OcxContainer, OcxContainerID, Primer
  VirtualMethod     CreateControl,          REFGUID, dword
  VirtualMethod     CreateControlEx,        REFGUID, dword, BSTR
  VirtualMethod     CreateControlLic,       REFGUID, dword, BSTR
  VirtualMethod     DeleteControl,          Pointer
  RedefineMethod    Done
  RedefineMethod    Init,                   Pointer, Handle

  DefineVariable    hWndSite,               Handle,         0         ;Parent window of hCtrlWnd
  DefineVariable    pIStorageRoot,          Pointer,        NULL

  ;Supported ambient properties
  DefineVariable    dUserMode,              BOOL,           TRUE      ;Run/Design mode = TRUE/FALSE
  DefineVariable    pIFontDisp,             Pointer,        NULL
  DefineVariable    dOleForeColor,          OLE_COLOR,      $RGB(0,0,255)
  DefineVariable    dOleBackColor,          OLE_COLOR,      $RGB(192,192,192)
  DefineVariable    dUserLocaleID,          LCID,           LOCALE_SYSTEM_DEFAULT
  DefineVariable    dMessageReflect,        BOOL,           FALSE
  DefineVariable    dShowGrabhandles,       BOOL,           TRUE      ;Show handles on the corners
  DefineVariable    dShowHatching,          BOOL,           TRUE      ;Show hatched border
  DefineVariable    dDisplayAsDefault,      BOOL,           TRUE
  DefineVariable    dRightToLeft,           BOOL,           FALSE
  DefineVariable    dTopToBottom,           BOOL,           TRUE
  DefineVariable    dUIDead,                BOOL,           FALSE     ;Ctrl response to the user

  Embed    OcxClientSiteColl,    Collection
ObjectEnd


include I_OcxCont_OleWindow.inc
include I_OcxCont_OleClientSite.inc
include I_OcxCont_AdviseSink.inc
include I_OcxCont_OleInPlaceSite.inc
include I_OcxCont_OleInPlaceSiteEx.inc
include I_OcxCont_OleControlSite.inc
include I_OcxCont_OleContainer.inc
include I_OcxCont_Dispatch.inc
include I_OcxCont_SimpleFrameSite.inc
include I_OcxCont_OleInPlaceFrame.inc
include I_OcxCont_PropertyNotifySink.inc
include I_OcxCont_ErrorInfo.inc
include I_OcxCont_OleInPlaceSiteWindowless.inc

.code

if IMPLEMENT

; ==================================================================================================
;     OcxClientSite implementation
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OcxClientSite.Connect
; Purpose:   Connects the control object to the container using the Site object. Called internally.
; Arguments: None.
; Return:    Nothing.

GatterCtrlInterface macro IntName:req
    lea ecx, [esi].p&IntName&
    and Pointer ptr [ecx], NULL
    ICall [esi].pIUnknown::IUnknown.QueryInterface, offset IID_&IntName&, ecx
    if DEBUGGING
      .if FAILED(eax)
        @CatStr(<DbgText !"Failed to get I>, <IntName>, <!">)
      .endif
    endif
endm

Method OcxClientSite.Connect, uses edi esi
    local pIStorage:Pointer
    local wszStorageName[MAX_PATH]:word

    SetObject esi

    ;Try to gatter all these control interfaces
    GatterCtrlInterface IOleObject
    GatterCtrlInterface IOleInPlaceObject
    GatterCtrlInterface IOleInPlaceActiveObject
    GatterCtrlInterface IOleControl
    GatterCtrlInterface IDataObject
    GatterCtrlInterface IViewObject2
    GatterCtrlInterface IDispatch
    GatterCtrlInterface IProvideClassInfo 
    GatterCtrlInterface ISpecifyPropertyPages 
    GatterCtrlInterface IPersistStorage 

    SetOwner edi, OcxContainer, esi

    .if [esi].pIOleObject != NULL
      ;Get the MiscStatus flags
      ICall [esi].pIOleObject::IOleObject.GetMiscStatus, DVASPECT_CONTENT, addr [esi].dMiscStatus

      .ifBitSet [esi].dMiscStatus, OLEMISC_SETCLIENTSITEFIRST
        ;Set the Control's clientsite
        ICall [esi].pIOleObject::IOleObject.SetClientSite, [esi].pIOleClientSite
        ;If all went right, the control should have querried at least for IDispatch
        .if FAILED(eax)
          DbgWarning "IOleObject.SetClientSite failed"
          DbgComError(eax)
        .endif
      .endif

      ;Set Advise Sink
      ICall [esi].pIOleObject::IOleObject.Advise, [esi].pIAdviseSink, addr [esi].dAS_Cookie
      .if FAILED(eax)
        DbgWarning "IOleObject.Advise failed"
        DbgComError eax
      .endif
    .endif

    ;Storage initialization
;    invoke MultiByteToWideChar, CP_ACP, 0, [esi].pName, -1, addr wszStorageName, MAX_PATH
    invoke dword2hex, addr wszStorageName, esi
    lea eax, wszStorageName
    mov byte ptr [eax+8], 0
    invoke Str2UStr, eax, eax
    DbgUStr wszStorageName
    and [esi].pIStorage, NULL
    ICall [edi].pIStorageRoot::IStorage.OpenStorage, addr wszStorageName, NULL, SHAREFLAGS, 0, 0, addr [esi].pIStorage
    .if eax == STG_E_FILENOTFOUND
      ICall [edi].pIStorageRoot::IStorage.CreateStorage, addr wszStorageName, SHAREFLAGS, 0, 0, addr [esi].pIStorage
      .if eax != S_OK
        DbgComError eax
      .else
        ICall [esi].pIStorage::IStorage.SetClass, [esi].pCtrlClsID
        ;Call IPersistStorage::Load or ::InitNew
        .if [esi].pIPersistStorage != NULL
          ICall [esi].pIPersistStorage::IPersistStorage.Load, [esi].pIStorage
          .if FAILED(eax)
            ICall [esi].pIPersistStorage::IPersistStorage.InitNew, [esi].pIStorage
          .endif
        .endif
      .endif
    .endif

    .if [esi].pIOleObject != NULL
      ICall [esi].pIOleObject::IOleObject.SetHostNames, \
                              $OfsCBStr("OCX Control Container"), NULL
    .endif

    ;Put the Control from loaded to running state (rarely needed for activex controls)
    .if [esi].pIUnknown != NULL
      invoke OleRun, [esi].pIUnknown
    .endif

    .if [esi].pIOleObject != NULL
      ;Activate the Control
      OCall DoVerb, OLEIVERB_INPLACEACTIVATE
      .if FAILED(eax)
        DbgWarning "OLEIVERB_INPLACEACTIVATE failed, trying OLEIVERB_SHOW..."
        OCall DoVerb, OLEIVERB_SHOW
      .endif

      ;If we did it not before, the we set the client site now
      .ifBitClr [esi].dMiscStatus, OLEMISC_SETCLIENTSITEFIRST
        ICall [esi].pIOleObject::IOleObject.SetClientSite, [esi].pIOleClientSite
      .endif
    .endif

    .if [esi].pIOleInPlaceObject != NULL
      ICall [esi].pIOleInPlaceObject::IOleInPlaceObject.GetWindow_, addr [esi].hCtrlWnd
    .endif

    ;Establish IPropertyNotifySink Connection Point
    invoke Events_Advice, [esi].pIUnknown, \
                          [esi].pIPropertyNotifySink, offset IID_IPropertyNotifySink, \
                          addr [esi].pIPNS_ConnectionPoint, addr [esi].dPNS_Cookie
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OcxClientSite.Disconnect
; Purpose:   Disconnects the control object from the container.
; Arguments: None.
; Return:    Nothing.

Method OcxClientSite.Disconnect, uses esi
    SetObject esi

    ;Unadvise and cleanup
    .if [esi].dAS_Cookie != 0                           ;This also ensures that pIOleObject != NULL
      ICall [esi].pIOleObject::IOleObject.Unadvise, [esi].dAS_Cookie
      and [esi].dAS_Cookie, 0
    .endif
    Destroy [esi].pIAdviseSink

    .if [esi].dPNS_Cookie != 0
      invoke Events_Unadvice, [esi].pIPNS_ConnectionPoint, [esi].dPNS_Cookie
      and [esi].dPNS_Cookie, 0
    .endif
    Destroy [esi].pIPropertyNotifySink


    ;Release all aquired interfaces
    SAFE_RELEASE_AND_NULLIFY [esi].pISpecifyPropertyPages
    SAFE_RELEASE_AND_NULLIFY [esi].pIProvideClassInfo
    SAFE_RELEASE_AND_NULLIFY [esi].pIDispatch
    SAFE_RELEASE_AND_NULLIFY [esi].pIViewObject2
    SAFE_RELEASE_AND_NULLIFY [esi].pIDataObject
    SAFE_RELEASE_AND_NULLIFY [esi].pIOleControl
    SAFE_RELEASE_AND_NULLIFY [esi].pIOleInPlaceActiveObject
    SAFE_RELEASE_AND_NULLIFY [esi].pIOleInPlaceObject

    .if [esi].pIOleObject != NULL
      ICall [esi].pIOleObject::IOleObject.Close, OLECLOSE_SAVEIFDIRTY
      ICall [esi].pIOleObject::IOleObject.SetClientSite, NULL
      ICall [esi].pIOleObject::IOleObject.Release
    .endif

    SAFE_RELEASE_AND_NULLIFY [esi].pIPersistStorage
    SAFE_RELEASE_AND_NULLIFY [esi].pIUnknown

    ;Final cleanup
    invoke StrDispose, [esi].pName                      ;It is safe to call this API

    ;Now we trigger the component destruction. This MUST be the last thing we do.
    ICall [esi].pIUnknownSite::IUnknown.Release
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OcxClientSite.DoVerb
; Purpose:   Performs a verb specific command.
; Arguments: Arg1: verb command.
; Return:    HRESULT.

Method OcxClientSite.DoVerb,, dVerb:dword
    SetObject ecx
    .if [ecx].pIOleObject != NULL
      SetOwner edx, OcxContainer, ecx
      ICall [ecx].pIOleObject::IOleObject.DoVerb, dVerb, 0, [ecx].pIOleClientSite, 0, \
                                                      [edx].hWndSite, addr [ecx].CtrlRect
      if DEBUGGING
        .if FAILED(eax)
          DbgWarning "IOleObject.DoVerb failed"
          DbgDec dVerb
          DbgComError eax
        .endif
      endif
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OcxClientSite.Init
; Purpose:   Initializes the Control object.
; Arguments: Arg1: -> Owner object (OcxContainer).
; Return:    OBJ_OK if succeeded.

Method OcxClientSite.Init, uses ebx esi, pOwner:Pointer
    SetObject esi
    ACall esi.Init, pOwner, NULL
    SetOwner ebx, OcxContainer, esi
    mov [esi].pIAdviseSink, $New(I_OcxCont_AdviseSink, Init, esi)
    mov [esi].pIPropertyNotifySink, $New(I_OcxCont_PropertyNotifySink, Init, esi)

    mov [esi].pIOleClientSite, $New(I_OcxCont_OleClientSite, Init, esi)
    OCall [esi].pInterfaces::Collection.Insert, eax
    mov [esi].pIOleInPlaceSite, $New(I_OcxCont_OleInPlaceSite, Init, esi, [ebx].hWndSite)
    OCall [esi].pInterfaces::Collection.Insert, eax
    mov [esi].pIOleInPlaceSiteEx, $New(I_OcxCont_OleInPlaceSiteEx, Init, esi, [ebx].hWndSite)
    OCall [esi].pInterfaces::Collection.Insert, eax
    mov [esi].pIOleInPlaceSiteWindowless, $New(I_OcxCont_OleInPlaceSiteWindowless, Init, esi, [ebx].hWndSite)
    OCall [esi].pInterfaces::Collection.Insert, eax
    mov [esi].pIOleInPlaceFrame, $New(I_OcxCont_OleInPlaceFrame, Init, esi, [ebx].hWndSite)
    OCall [esi].pInterfaces::Collection.Insert, eax
    mov [esi].pIOleControlSite, $New(I_OcxCont_OleControlSite, Init, esi)
    OCall [esi].pInterfaces::Collection.Insert, eax
    mov [esi].pIDispatch, $New(I_OcxCont_Dispatch, Init, esi, NULL, NULL)
    OCall [esi].pInterfaces::Collection.Insert, eax
    mov [esi].pISimpleFrameSite, $New(I_OcxCont_SimpleFrameSite, Init, esi)
    OCall [esi].pInterfaces::Collection.Insert, eax
    mov [esi].pIErrorInfo, $New(I_OcxCont_ErrorInfo, Init, esi)
    OCall [esi].pInterfaces::Collection.Insert, eax
    mov [esi].pIOleContainer, $New(I_OcxCont_OleContainer, Init, esi)
    OCall [esi].pInterfaces::Collection.Insert, eax

    ;Increment the ref count to take control over the component destruction
    OCall esi.GetInterface, offset IID_IUnknown, addr [esi].pIUnknownSite
    xor eax, eax                                        ;eax = OBJ_OK
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OcxClientSite.GetName
; Purpose:   Retrievs the name of the control (ANSI).
; Arguments: None.
; Return:    Nothing.

Method OcxClientSite.GetName, uses esi
    local wszCLSID[40]:word, szCLSID[40]:byte, szText[MAX_PATH]:byte
    local hKey:Handle, dSize:dword

    SetObject esi
    invoke StringFromGUID2, [esi].pCtrlClsID, addr wszCLSID, 40
    invoke StrECopy, addr szText, $OfsCStr("CLSID\")
    invoke WideCharToMultiByte, CP_ACP, 0, addr wszCLSID, -1, eax, 40, NULL, NULL
    invoke RegOpenKeyEx, HKEY_CLASSES_ROOT, addr szText, 0, KEY_READ, addr hKey

    .if (eax == ERROR_SUCCESS)
      mov dSize, sizeof szText
      invoke RegQueryValueEx, hKey, offset szNULL, NULL, NULL, addr szText, addr dSize
      invoke RegCloseKey, hKey
      invoke StrDispose, [esi].pName                    ;It is safe to call this API
      mov [esi].pName, $invoke(StrNew, addr szText)
    .endif
MethodEnd


; ==================================================================================================
;     OcxContainer implementation
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OcxContainer.CreateControl
; Purpose:   Creates a new hosted control using CoCreateInstance.
; Arguments: Arg1: -> Control class ID (GUID).
;            Arg2: Class context.
; Return:    On success, eax -> new created OcxControlSite, otherwise NULL.

Method OcxContainer.CreateControl, uses ebx esi, pCtrlClsID:REFGUID, dClsCtx:dword
    SetObject esi
    New OcxClientSite
    .if eax != NULL
      mov ebx, eax
      OCall ebx::OcxClientSite.Init, esi
      .if eax == OBJ_OK
        assume ebx:ptr OcxClientSite
        invoke CoCreateInstance, pCtrlClsID, NULL, dClsCtx, offset IID_IUnknown, addr [ebx].pIUnknown
        .if SUCCEEDED(eax)
          ;Insert new OcxClientSite
          OCall [esi].OcxClientSiteColl::Collection.Insert, ebx
          m2m [ebx].pCtrlClsID, pCtrlClsID, edx
          OCall ebx::OcxClientSite.GetName
          OCall ebx::OcxClientSite.Connect
          mov eax, ebx
        .else
          Kill ebx
          xor eax, eax
        .endif
        assume ebx:nothing
      .else
        Kill ebx
        xor eax, eax
      .endif
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OcxContainer.CreateControlEx
; Purpose:   Creates a new hosted control using CoCreateInstanceEx.
; Arguments: Arg1: -> Control class ID (GUID).
;            Arg2: Class context.
;            Arg3: BSTR identifiying the remote host machine.
; Return:    On success, eax -> new created OcxControlSite, otherwise NULL.

Method OcxContainer.CreateControlEx, uses ebx esi, pCtrlClsID:REFGUID, dClsCtx:dword, pMachine:BSTR
    local csi:COSERVERINFO
    local mqi[1]:MULTI_QI

    SetObject esi
    New OcxClientSite
    .if eax != NULL
      mov ebx, eax
      OCall ebx::OcxClientSite.Init, esi
      .if eax == OBJ_OK
        assume ebx:ptr OcxClientSite

        mov mqi[0*sizeof MULTI_QI].pIID, offset IID_IUnknown
        and mqi[0*sizeof MULTI_QI].pItf, NULL
        and mqi[0*sizeof MULTI_QI].hr, 0

        .if pMachine == NULL
          xor edx, edx
          mov ecx, dClsCtx
        .else
          and csi.dwReserved1, 0
          m2m csi.pwszName, pMachine, edx
          mov csi.pAuthInfo, 0
          and csi.dwReserved2, 0
          lea edx, csi
          mov ecx, CLSCTX_REMOTE_SERVER
        .endif

        invoke CoCreateInstanceEx, pCtrlClsID, NULL, ecx, edx, 3, addr mqi
        .if SUCCEEDED(eax)
          OCall [esi].OcxClientSiteColl::Collection.Insert, ebx
          .if mqi[0*sizeof MULTI_QI].hr == S_OK
            m2m [ebx].pIUnknown, mqi[0*sizeof MULTI_QI].pItf, ecx
          .endif
          m2m [ebx].pCtrlClsID, pCtrlClsID, edx
          OCall ebx::OcxClientSite.GetName
          OCall ebx::OcxClientSite.Connect
          mov eax, ebx
        .else
          Kill ebx
          xor eax, eax
        .endif
        assume ebx:nothing
      .else
        Kill ebx
        xor eax, eax
      .endif
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OcxContainer.CreateControlLic
; Purpose:   Creates a new hosted control using IClassFactory2.
; Arguments: Arg1: -> Control class ID (GUID).
;            Arg2: Class context.
;            Arg3: -> License Key (BSTR).
; Return:    On success, eax -> new created OcxControlSite, otherwise NULL.

Method OcxContainer.CreateControlLic, uses ebx esi, pCtrlClsID:REFGUID, dClsCtx:dword, pLicKey:BSTR
    local pClassFactory2:Pointer, LicInfo:LICINFO, bInternalLicKey:BOOL

    SetObject esi
    and bInternalLicKey, FALSE

    New OcxClientSite
    .if eax != NULL
      mov ebx, eax
      OCall ebx::OcxClientSite.Init, esi
      .if eax == OBJ_OK
        assume ebx:ptr OcxClientSite
        invoke CoGetClassObject, pCtrlClsID, dClsCtx, NULL, addr IID_IClassFactory2, addr pClassFactory2
        .if SUCCEEDED(eax)
          .if pLicKey == NULL
            ICall pClassFactory2::IClassFactory2.GetLicInfo, addr LicInfo
            DbgHex LicInfo.fRuntimeKeyAvail, "fRuntimeKeyAvail"
            DbgHex LicInfo.fLicVerified, "fLicVerified"
            .if LicInfo.fRuntimeKeyAvail == TRUE
              ICall pClassFactory2::IClassFactory2.RequestLicKey, NULL, addr pLicKey
              .if SUCCEEDED(eax)
                mov bInternalLicKey, TRUE
              .endif
            .endif
          .endif

          ICall pClassFactory2::IClassFactory2.CreateInstanceLic, NULL, NULL, offset IID_IUnknown, \
                                                                  pLicKey, addr [ebx].pIUnknown
          .if SUCCEEDED(eax)
            ;Cleanup
            .if (pLicKey != NULL) && (bInternalLicKey == TRUE)
              invoke SysFreeString, pLicKey
            .endif
            ICall pClassFactory2::IClassFactory2.Release

            ;Insert new OcxClientSite
            OCall [esi].OcxClientSiteColl::Collection.Insert, ebx
            m2m [ebx].pCtrlClsID, pCtrlClsID, edx
            OCall ebx::OcxClientSite.GetName
            OCall ebx::OcxClientSite.Connect
            mov eax, ebx
          .else
            DbgComError eax
            Kill ebx
            xor eax, eax
          .endif
        .else
          Kill ebx
          xor eax, eax
        .endif
        assume ebx:nothing
      .else
        Kill ebx
        xor eax, eax
      .endif
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OcxContainer.DeleteControl
; Purpose:   Deletes the currently hosted control.
; Arguments: Arg1: -> OcxClientSite.
; Return:    Nothing.

Method OcxContainer.DeleteControl,, pClientSite:Pointer
    .if pClientSite != NULL
      OCall pClientSite::OcxClientSite.Disconnect
      SetObject ecx
      OCall [ecx].OcxClientSiteColl::Collection.Delete, pClientSite
    .endif 
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OcxContainer.Done
; Purpose:   Finalizes the Host object.
; Arguments: None.
; Return:    Nothing.

Method OcxContainer.Done, uses ebx edi esi
    SetObject esi
    xor ebx, ebx
    mov edi, [esi].OcxClientSiteColl.pItems
    .while ebx != [esi].OcxClientSiteColl.dCount
      OCall Pointer ptr [edi]::OcxClientSite.Disconnect
      add edi, 4
      inc ebx
    .endw
    OCall [esi].OcxClientSiteColl::Collection.DeleteAll
    
    OCall [esi].OcxClientSiteColl::Collection.Done

    SAFE_RELEASE_AND_NULLIFY [esi].pIFontDisp
    SAFE_RELEASE_AND_NULLIFY [esi].pIStorageRoot
    ACall esi.Done
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OcxContainer.Init
; Purpose:   Initializes the OcxContainer object.
; Arguments: Arg1: -> Owner.
;            Arg2: Container Window handle. It acts as parent for all control windows.
; Return:    OBJ_OK if succeeded.

Method OcxContainer.Init, uses esi, pOwner:Pointer, hWndSite:Handle
    local FD:FONTDESC

    SetObject esi
    ACall esi.Init, pOwner
    m2m [esi].hWndSite, hWndSite, edx
    OCall [esi].OcxClientSiteColl::Collection.Init, esi, 10, 10, COLLECTIONMAXSIZE

    mov FD.FONTDESC.cbSizeofstruct, sizeof FONTDESC
    mov FD.FONTDESC.lpstrName, $OfsCUStr("MS Sans Serif")
    mov FD.FONTDESC.cySize.Lo, 100000                                     ;Points * 10000
    and FD.FONTDESC.cySize.Hi, 0
    mov FD.FONTDESC.sWeight, FW_NORMAL
    mov FD.FONTDESC.sCharset, ANSI_CHARSET
    mov FD.FONTDESC.fItalic, -1
    mov FD.FONTDESC.fUnderline, FALSE
    mov FD.FONTDESC.fStrikethrough, FALSE
    invoke OleCreateFontIndirect, addr FD, offset IID_IFontDisp, addr [esi].pIFontDisp

    mov [esi].dUserLocaleID, $invoke(GetUserDefaultLCID)

    invoke StgCreateDocfile, NULL, SHAREFLAGS, 0, addr [esi].pIStorageRoot
    if DEBUGGING
      .if FAILED(eax)
        DbgComError eax
      .endif
    endif

MethodEnd


endif