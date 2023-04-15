; ==================================================================================================
; Title:   OleDB.inc
; Author:  K. Usanov
; Version: 1.0.3
; Purpose: Support of OleDB objects for ObjAsm32.
; Notes:   Version 1.0.0, September 2004
;            - First release.
;          Version 1.0.1, May 2005
;            - Minor changes. (corrected Delta's size for collections)
;                             (moved UString.inc from OleDB.asm file).
;          Version 1.0.2, June 2005
;            - Code adapted for new SEH Exception Code macro by Biterider.
;          Version 1.0.3, August 2009
;            - "Extended Properties" added to the connect string by Biterider.
; ==================================================================================================

;http://media.datadirect.com/download/docs/slnk/devref/devado.html
;http://media.datadirect.com/download/docs/slnk/devref/useado.html#wp1002598
;http://msdn.microsoft.com/en-us/library/ms714361(VS.85).aspx
;http://msdn.microsoft.com/en-us/library/ms725405(VS.85).aspx

% include &COMPath&OleDB_TLB.inc
% include &MacPath&UStrings.inc

Object OleDB, OleDBID, Primer
  RedefineMethod    Done
  RedefineMethod    Init,             dword

  VirtualMethod     IniData,          Pointer
  VirtualMethod     GetData,          Pointer, Pointer
  VirtualMethod     SetData,          Pointer

  Private
  VirtualMethod     DescribeError,    dword, Pointer
  VirtualMethod     ParseArg,         Pointer, Pointer, Pointer

  DefineVariable    pOLEDB,           Pointer, NULL
  DefineVariable    dShowException,   dword,   0
ObjectEnd


; ==================================================================================================

if IMPLEMENT

MAXEXPECTPARAM  equ     6
MAXLENGTH       equ     2048
NUMROWS_CHUNK   equ     100

CStr szErrException,    "EXCEPTION trapped in %s, Error = [%X]"

CStr ExpParamProvider,  "Provider"
CStr ExpParam1,         "Data Source"
CStr ExpParam2,         "Initial Catalog"
CStr ExpParam3,         "User ID"
CStr ExpParam4,         "Password"
CStr ExpParam5,         "Extended Properties"


.const
DB_NULLID      DBID     <>

CUStr OleDbJET,         "Microsoft.Jet.OleDB.4.0"
DefGUID CLSID_OleDbJET, %sCLSID_OleDbJET

CUStr OleDbSQL,         "SQLOLEDB"
DefGUID CLSID_OleDbSQL, %sCLSID_OleDbSQL

CUStr OleDbORA,         "MSDAORA"
DefGUID CLSID_OleDbORA, %sCLSID_OleDbORA

CUStr OleDbDSN,         "MSDASQL"
DefGUID CLSID_OleDbDSN, %sCLSID_OleDbDSN

DefGUID CLSID_NULL,             %sGUID_NULL

DefGUID DBGUID_DEFAULT,         %sDBGUID_DEFAULT
DefGUID DBPROPSET_DBINIT,       %sDBPROPSET_DBINIT
DefGUID DBPROPSET_ROWSET,       %sDBPROPSET_ROWSET
DefGUID IID_IDBInitialize,      %sIID_IDBInitialize
DefGUID IID_IDBProperties,      %sIID_IDBProperties
DefGUID IID_IDBCreateSession,   %sIID_IDBCreateSession
DefGUID IID_IDBCreateCommand,   %sIID_IDBCreateCommand
DefGUID IID_ICommandText,       %sIID_ICommandText
DefGUID IID_ICommandProperties, %sIID_ICommandProperties
DefGUID IID_IColumnsInfo,       %sIID_IColumnsInfo
DefGUID IID_IAccessor,          %sIID_IAccessor
DefGUID IID_IRowset,            %sIID_IRowset
DefGUID IID_NULL,               %sGUID_NULL


mvb macro Dst:req, Src:req, mSize
    push ecx
    ifb <mSize>
      mov ecx, SIZEOF Src
    else
      mov ecx, mSize
    endif
    invoke MemClone, addr Dst, addr Src, ecx
    pop ecx
endm

.code

CreateExceptionHandler OleDBException

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OleDB.Done
; Purpose:   Close datasource and free instance of OleDB object.
; Arguments: None.
; Return:    Nothing.

Method OleDB.Done, uses esi
    SetObject esi

    .if [esi].pOLEDB
      ICall [esi].pOLEDB::IDBInitialize.Uninitialize
      ICall [esi].pOLEDB::IDBInitialize.Release
    .endif

    ACall esi.Done
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OleDB.DescribeError
; Purpose:   Show correspond error message.
;- Arguments: Arg1: Pointer to Exception Record.
; Arguments: Arg1: Exception Code.
;            Arg2: Pointer to string that represents the context.
; Return:    Nothing.

Method OleDB.DescribeError, uses esi, ExceptionCode:dword, pErrContext:Pointer
    local szErr[MAXLENGTH]:byte, szTmp[MAXLENGTH]:byte
    local pIErrorInfo:Pointer, pSQLState:Pointer

    SetObject esi

    .if [esi].dShowException
      invoke wsprintf, addr szErr, offset szErrException, pErrContext, ExceptionCode
      .if !$invoke(GetErrorInfo, 0, addr pIErrorInfo)
        ICall pIErrorInfo::IErrorInfo.GetDescription, addr pSQLState    ;-> BStr
        ICall pIErrorInfo::IErrorInfo.Release

        invoke WideCharToMultiByte, CP_ACP, 0, pSQLState, -1, addr szTmp, MAXLENGTH, NULL, NULL
        invoke SysFreeString, pSQLState
      .else
        invoke MemClone, addr szTmp, addr szErr, MAXLENGTH
      .endif

      invoke MessageBox, NULL, addr szTmp, addr szErr, MB_OK
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OleDB.ParseArg
; Purpose:   Find the pattern string and convert it to a BStr.
; Arguments: Arg1: -> Input string.
;            Arg2: -> Pattern string.
;            Arg3: -> Output string.
; Return:    Nothing.

Method OleDB.ParseArg, uses ebx, pInpStr:Pointer, Pattern:Pointer, pOutBStr:Pointer
    local tmpStr[MAXLENGTH]:byte

    invoke StrLength, Pattern
    push eax
    invoke StrPos, pInpStr, Pattern
    pop ecx

    .if eax
      add eax, ecx

      invoke StrScan, eax, '='
      inc eax
      push eax

      invoke StrScan, eax, ';'

      pop ebx
      sub eax, ebx

      invoke StrLCopy, addr tmpStr, ebx, eax
      invoke StrTrim, addr tmpStr, addr tmpStr
    .else
      and tmpStr, NULL
    .endif

    invoke Str2BStr, pOutBStr, addr tmpStr
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OleDB.Init
; Purpose:   Initialize instance of OleDB object.
; Arguments: Arg1: Flag to activate the exception display (1 - show, 0 - no show).
; Return:    Nothing.

Method OleDB.Init,, dShowException:dword
    SetObject ecx
    m2m [ecx].dShowException, dShowException, edx
    ACall ecx.Init, NULL
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OleDB.IniData
; Purpose:   Open datasource.
; Arguments: Arg1: -> Connection string (ADO format).
; Return:    Error Exception code, otherwise 0.

Method OleDB.IniData, uses ebx esi edi, pStrConnect:Pointer
    local pIDBProperties:Pointer, rgPropSet:DBPROPSET, pProperties:Pointer
    local tmpBStr[MAXLENGTH]:word, nPropsInit:dword

    and pProperties, NULL
    and pIDBProperties, NULL
    
    
  Try OleDBException

      SetObject esi

      lea ebx, ExpParamProvider
      lea edi, tmpBStr[sizeof dword]
      OCall esi.ParseArg, pStrConnect, ebx, edi

      .if !$invoke(UStrComp, edi, offset OleDbJET)
        lea ebx, CLSID_OleDbJET
      .else
        .if !$invoke(UStrComp, edi, offset OleDbSQL)
          lea ebx, CLSID_OleDbSQL
        .else
          .if !$invoke(UStrComp, edi, offset OleDbORA)
            lea ebx, CLSID_OleDbORA
          .else
            .if !$invoke(UStrComp, edi, offset OleDbDSN)
              lea ebx, CLSID_OleDbDSN
            .else
              lea ebx, CLSID_NULL
            .endif
          .endif
        .endif
      .endif

      TryInvoke CoCreateInstance, ebx, NULL, CLSCTX_INPROC_SERVER, addr IID_IDBInitialize, addr [esi].pOLEDB

      mov pProperties, $MemAlloc(sizeof DBPROP)

      mov edx, eax
      invoke VariantInit, addr (DBPROP ptr[edx]).vValue
      mov (DBPROP ptr[edx]).dwOptions,    DBPROPOPTIONS_REQUIRED
      mvb (DBPROP ptr[edx]).colid,        DB_NULLID
      mov (DBPROP ptr[edx]).dwPropertyID, DBPROP_INIT_PROMPT
      mov (DBPROP ptr[edx]).vValue.vt,    VT_I2
      mov (DBPROP ptr[edx]).vValue.iVal,  DBPROMPT_NOPROMPT
      mov nPropsInit, 1

      mov ecx, 2
      lea ebx, ExpParam1
      lea edi, tmpBStr[sizeof dword]
      .while ecx <= MAXEXPECTPARAM
        push ecx
;        DbgStr ebx
        OCall esi.ParseArg, pStrConnect, ebx, edi
        pop ecx

        .if word ptr[edi]
          mov eax, sizeof DBPROP
          inc nPropsInit
          mul nPropsInit
          push ecx
          mov pProperties, $MemReAlloc(pProperties, eax)
          pop ecx

          mov eax, sizeof DBPROP
          mov edx, nPropsInit
          dec edx
          mul edx

          mov edx, pProperties
          add edx, eax
          invoke VariantInit, addr (DBPROP ptr[edx]).vValue
          mov (DBPROP ptr[edx]).dwOptions,      DBPROPOPTIONS_REQUIRED
          mvb (DBPROP ptr[edx]).colid,          DB_NULLID
          mov (DBPROP ptr[edx]).dwPropertyID,   $Choose(ecx, NULL, NULL, DBPROP_INIT_DATASOURCE, DBPROP_INIT_CATALOG, DBPROP_AUTH_USERID, DBPROP_AUTH_PASSWORD, DBPROP_INIT_PROVIDERSTRING)
          mov (DBPROP ptr[edx]).vValue.vt,      VT_BSTR
          mov (DBPROP ptr[edx]).vValue.bstrVal, edi
;          DbgUStr edi
;        .else
;          DbgText "not found"
        .endif

        push ecx
        invoke BStrLength, edi
        shl eax, 1
        add edi, eax
        add edi, 2 + sizeof dword

        add ebx, $invoke(StrLength, ebx)
        inc ebx
        pop ecx

        inc ecx
      .endw

      m2m rgPropSet.rgProperties,    pProperties
      m2m rgPropSet.cProperties,     nPropsInit
      mvb rgPropSet.guidPropertySet, DBPROPSET_DBINIT

      TryICall [esi].pOLEDB::IDBInitialize.QueryInterface, addr IID_IDBProperties, addr pIDBProperties

      TryICall pIDBProperties::IDBProperties.SetProperties, 1, addr rgPropSet
      ICall pIDBProperties::IDBProperties.Release
      and pIDBProperties, NULL

      TryICall [esi].pOLEDB::IDBInitialize.Initialize

      MemFree pProperties

      xor eax, eax

  Catch OleDBException
      mov ebx, $GetExceptionCode()
      .if pProperties
        MemFree pProperties
      .endif
      .if pIDBProperties
        ICall pIDBProperties::IDBProperties.Release
      .endif

      SetObject esi

      .if [esi].pOLEDB
        ICall [esi].pOLEDB::IDBInitialize.Release
        and [esi].pOLEDB, NULL
      .endif

      OCall esi.DescribeError, ebx, "OleDB.IniData"

      mov eax, ebx
  endc OleDBException
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OleDB.GetData
; Purpose:   Return recordset, collection(records) of datacollection(fields) objects,
;            for input SQL string (SELECT).
; Arguments: Arg1: -> Input SQL string.
;            Arg2: -> Output recordset.
; Return:    Error Exception code, otherwise 0.

Method OleDB.GetData, uses ebx esi edi, pInSQL:Pointer, pSQLData:Pointer
    local szSQL[MAXLENGTH]:word
    local pField:Pointer, pRecord:Pointer, pIMalloc:Pointer, hAccessor:HACCESSOR
    local dRowsAffected:dword, dCols:dword, dRow:dword, dRowsObtained:dword
    local Properties:DBPROP, rgPropSet:DBPROPSET
    local rghRows[NUMROWS_CHUNK]:HROW, prghRows:LPHROW
    local pIDBCreateSession:Pointer, pIDBCreateCommand:Pointer, pICommandText:Pointer, \
          pICommandProperties:Pointer, pIRowset:Pointer, pIColumnsInfo:Pointer, pIAccessor:Pointer
    local pDBBindings:Pointer, pDBBindStatus:Pointer
    local pColumnsInfo:LPDBCOLUMNINFO, pColumnStrings:Pointer, pRowValues:Pointer

    and pIDBCreateSession, NULL
    and pIDBCreateCommand, NULL
    and pICommandProperties, NULL
    and pICommandText, NULL
    and pIColumnsInfo, NULL

    and pDBBindings, NULL
    and pDBBindStatus, NULL

    and pRowValues, NULL
    and pColumnsInfo, NULL
    and pColumnStrings, NULL

    and pIRowset, NULL
    and pIAccessor, NULL

    mov ebx, pSQLData
    and dword ptr [ebx], 0

  Try OleDBException
      SetObject esi
      TryICall [esi].pOLEDB::IDBInitialize.QueryInterface, addr IID_IDBCreateSession, addr pIDBCreateSession
      mov [ebx], $New(Collection, Init, esi, 0, 1, COLLECTIONMAXSIZE)
      ReleaseObject                   ;Probable use of esi

      TryICall pIDBCreateSession::IDBCreateSession.CreateSession, NULL, addr IID_IDBCreateCommand, addr pIDBCreateCommand
      ICall pIDBCreateSession::IDBCreateSession.Release
      and pIDBCreateSession, NULL

      TryICall pIDBCreateCommand::IDBCreateCommand.CreateCommand, NULL, addr IID_ICommandText, addr pICommandText
      ICall pIDBCreateCommand::IDBCreateCommand.Release
      and pIDBCreateCommand, NULL

      TryICall pICommandText::ICommandText.QueryInterface, addr IID_ICommandProperties, addr pICommandProperties

      invoke VariantInit, addr Properties.vValue
      mov Properties.dwOptions,      DBPROPOPTIONS_REQUIRED
      mvb Properties.colid,          DB_NULLID
      mov Properties.dwPropertyID,   DBPROP_CANHOLDROWS
      mov Properties.vValue.vt,      VT_BOOL
      mov Properties.vValue.boolVal, VARIANT_TRUE

      mov rgPropSet.rgProperties,    $addr(Properties)
      mov rgPropSet.cProperties,     1
      mvb rgPropSet.guidPropertySet, DBPROPSET_ROWSET

      TryICall pICommandProperties::ICommandProperties.SetProperties, 1, addr rgPropSet
      ICall pICommandProperties::ICommandProperties.Release
      and pICommandProperties, NULL

      invoke MultiByteToWideChar, CP_ACP, 0, pInSQL, -1, addr szSQL, MAXLENGTH
      TryICall pICommandText::ICommandText.SetCommandText, addr DBGUID_DEFAULT, addr szSQL

      TryICall pICommandText::ICommandText.Execute, NULL, addr IID_IRowset, NULL, addr dRowsAffected, addr pIRowset
      ICall pICommandText::ICommandText.Release
      and pICommandText, NULL

      TryICall pIRowset::IRowset.QueryInterface, addr IID_IColumnsInfo, addr pIColumnsInfo

      TryICall pIColumnsInfo::IColumnsInfo.GetColumnInfo, addr dCols, addr pColumnsInfo, addr pColumnStrings
      ICall pIColumnsInfo::IColumnsInfo.Release
      and pIColumnsInfo, NULL

      mov eax, sizeof DBBINDING
      mul dCols
      mov pDBBindings, $MemAlloc(eax)

      mov eax, sizeof DBBINDSTATUS
      mul dCols
      mov pDBBindStatus, $MemAlloc(eax)

      xor ebx, ebx
      mov esi, pColumnsInfo
      mov ecx, sizeof DBCOLUMNINFO
      mov edi, pDBBindings
      mov edx, sizeof DBBINDING
      and dRow, 0
      .while ebx < dCols
        test (DBCOLUMNINFO ptr[esi]).dwFlags, DBCOLUMNFLAGS_ISBOOKMARK
        je @F

        dec dCols
        add esi, ecx
        .continue

@@:     
        mov (DBBINDING ptr[edi]).iOrdinal,   ebx
        inc (DBBINDING ptr[edi]).iOrdinal

        m2m (DBBINDING ptr[edi]).obStatus,   dRow, eax
        add dRow,                            sizeof dword
        m2m (DBBINDING ptr[edi]).obLength,   dRow, eax
        add dRow,                            sizeof dword
        m2m (DBBINDING ptr[edi]).obValue,    dRow, eax
        and (DBBINDING ptr[edi]).pTypeInfo,  NULL
        and (DBBINDING ptr[edi]).pObject,    NULL
        and (DBBINDING ptr[edi]).pBindExt,   NULL
        and (DBBINDING ptr[edi]).dwFlags,    0
        mov (DBBINDING ptr[edi]).dwPart,     DBPART_VALUE + DBPART_LENGTH + DBPART_STATUS
        mov (DBBINDING ptr[edi]).dwMemOwner, DBMEMOWNER_CLIENTOWNED
        mov (DBBINDING ptr[edi]).eParamIO,   DBPARAMIO_NOTPARAM
        m2m (DBBINDING ptr[edi]).cbMaxLen,   (DBCOLUMNINFO ptr[esi]).ulColumnSize, eax
        mov eax, (DBBINDING ptr[edi]).cbMaxLen
        add dRow, eax
        m2m (DBBINDING ptr[edi]).wType,      (DBCOLUMNINFO ptr[esi]).wType, ax
        b2b (DBBINDING ptr[edi]).bPrecision, (DBCOLUMNINFO ptr[esi]).bPrecision
        b2b (DBBINDING ptr[edi]).bScale,     (DBCOLUMNINFO ptr[esi]).bScale

        inc ebx
        add esi, ecx
        add edi, edx
      .endw

      TryInvoke CoGetMalloc, 1, addr pIMalloc
      mov pRowValues, $TryICall(pIMalloc::IMalloc.Alloc_, dRow)

      TryICall pIRowset::IRowset.QueryInterface, addr IID_IAccessor, addr pIAccessor

      TryICall pIAccessor::IAccessor.CreateAccessor, DBACCESSOR_ROWDATA, dCols, pDBBindings, 0, addr hAccessor, pDBBindStatus

      mov prghRows, $addr(rghRows)
      .while TRUE
        TryICall pIRowset::IRowset.GetNextRows, DB_NULL_HCHAPTER, 0, NUMROWS_CHUNK, addr dRowsObtained, addr prghRows
        .if !dRowsObtained
          .break
        .endif

        xor ebx, ebx
        xor esi, esi
        mov ecx, TYPE rghRows
        .while ebx < dRowsObtained
          PushAll ebx, esi, ecx
          TryICall pIRowset::IRowset.GetData, rghRows[esi], hAccessor, pRowValues

          SetObject esi
          mov pRecord, $New(DataCollection, Init, esi, dCols, 1, dCols)
          ReleaseObject ; for probable use ESI below in code.

          xor ebx, ebx
          mov edi, pDBBindings
          mov edx, sizeof DBBINDING
          .while ebx < dCols
            mov ecx, pRowValues
            add ecx, (DBBINDING ptr[edi]).obStatus
            .if dword ptr [ecx] == DBSTATUS_S_ISNULL
              and pField, NULL
              push edx
              jmp @@InsertField
            .endif

            mov esi, pRowValues
            add esi, (DBBINDING ptr[edi]).obLength
            mov esi, [esi]

            cmp (DBBINDING ptr[edi]).wType, DBTYPE_WSTR
            jne @F
            shr esi, 1
            inc esi
@@:
            push edx
            mov pField, $MemAlloc(esi)
            pop edx

            mov ecx, pRowValues
            add ecx, (DBBINDING ptr[edi]).obValue

            cmp (DBBINDING ptr[edi]).wType, DBTYPE_WSTR
            jne @F
            push edx
            invoke WideCharToMultiByte, CP_ACP, 0, ecx, -1, pField, esi, NULL, NULL
            jmp @@InsertField
@@:
            push edx
            invoke MemClone, pField, ecx, esi

@@InsertField:
            OCall pRecord::DataCollection.Insert, pField
            pop edx

            inc ebx
            add edi, edx
          .endw

          mov ebx, pSQLData
          OCall dword ptr [ebx]::Collection.Insert, pRecord
          PopAll ecx, esi, ebx

          inc ebx
          add esi, ecx
        .endw

        TryICall pIRowset::IRowset.ReleaseRows, dRowsObtained, addr rghRows, NULL, NULL, NULL
      .endw

      MemFree pDBBindings
      and pDBBindings, NULL
      MemFree pDBBindStatus
      and pDBBindStatus, NULL

      TryICall pIMalloc::IMalloc.Free_, pRowValues
      and pRowValues, NULL

      TryICall pIMalloc::IMalloc.Free_, pColumnsInfo
      and pColumnsInfo, NULL
      TryICall pIMalloc::IMalloc.Free_, pColumnStrings
      and pColumnStrings, NULL

      TryICall pIAccessor::IAccessor.ReleaseAccessor, hAccessor, NULL
      ICall pIAccessor::IAccessor.Release
      and pIAccessor, NULL

      ICall pIRowset::IRowset.Release
      and pIRowset, NULL

      xor eax, eax

  Catch OleDBException
      mov ebx, $GetExceptionCode()
      .if pRowValues
        ICall pIMalloc::IMalloc.Free_, pRowValues
      .endif
      .if pColumnsInfo
        ICall pIMalloc::IMalloc.Free_, pColumnsInfo
      .endif
      .if pColumnStrings
        ICall pIMalloc::IMalloc.Free_, pColumnStrings
      .endif
      .if pDBBindStatus
        MemFree pDBBindStatus
      .endif
      .if pDBBindings
        MemFree pDBBindings
      .endif

      .if pIRowset
        .if pIAccessor
          ICall pIAccessor::IAccessor.Release
        .endif
        ICall pIRowset::IRowset.Release
      .endif

      .if pIColumnsInfo
        ICall pIColumnsInfo::IColumnsInfo.Release
      .elseif pICommandText
        ICall pICommandText::ICommandText.Release
      .elseif pICommandProperties
        ICall pICommandProperties::ICommandProperties.Release
      .elseif pIDBCreateCommand
        ICall pIDBCreateCommand::IDBCreateCommand.Release
      .elseif pIDBCreateSession
        ICall pIDBCreateSession::IDBCreateSession.Release
      .endif

      mov ebx, pSQLData
      Destroy dword ptr[ebx]
      SetObject esi
      OCall esi.DescribeError, ebx, "OleDB.GetData"
      mov eax, ebx
  endc OleDBException
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    OleDB.SetData
; Purpose:   Execute manipulation SQL statement (INSERT, UPDATE, DELETE).
; Arguments: Arg1: -> Input SQL string.
; Return:    Error Exception code, otherwise 0.

Method OleDB.SetData, uses ebx esi, pInSQL:Pointer
    local szSQL[MAXLENGTH]:dword, dRowsAffected:dword
    local pIDBCreateSession:Pointer, pIDBCreateCommand:Pointer, pICommandText:Pointer

    xor eax, eax
    mov pIDBCreateSession, eax
    mov pIDBCreateCommand, eax
    mov pICommandText,     eax

  Try OleDBException

      SetObject esi
      TryICall [esi].pOLEDB::IDBInitialize.QueryInterface, addr IID_IDBCreateSession, addr pIDBCreateSession

      TryICall pIDBCreateSession::IDBCreateSession.CreateSession, NULL, addr IID_IDBCreateCommand, addr pIDBCreateCommand
      ICall pIDBCreateSession::IDBCreateSession.Release
      and pIDBCreateSession, NULL

      TryICall pIDBCreateCommand::IDBCreateCommand.CreateCommand, NULL, addr IID_ICommandText, addr pICommandText
      ICall pIDBCreateCommand::IDBCreateCommand.Release
      and pIDBCreateCommand, NULL

      invoke MultiByteToWideChar, CP_ACP, 0, pInSQL, -1, addr szSQL, MAXLENGTH
      TryICall pICommandText::ICommandText.SetCommandText, addr DBGUID_DEFAULT, addr szSQL

      TryICall pICommandText::ICommandText.Execute, NULL, addr IID_NULL, NULL, addr dRowsAffected, NULL
      ICall pICommandText::ICommandText.Release
      and pICommandText, NULL

      xor eax,eax

  Catch OleDBException
      mov ebx, $GetExceptionCode()
      .if pICommandText
        ICall pICommandText::ICommandText.Release
      .elseif pIDBCreateCommand
        ICall pIDBCreateCommand::IDBCreateCommand.Release
      .elseif pIDBCreateSession
        ICall pIDBCreateSession::IDBCreateSession.Release
      .endif

      SetObject esi
      OCall esi.DescribeError, ebx, "OleDB.SetData"
      mov eax, ebx
  endc OleDBException
MethodEnd

endif