;DirectPlay BASE include file (April 2006 DX-SDK)
;Translated to MASM format by EvilHomer
;18 July 2006

echo == DirectPlay Base ==
_FACDP equ 877h
MAKE_DPHRESULT macro ccode
    exitm <80000000h or (_FACDP shl 16) or (ccode)>
endm

ifndef __DPLAY_INCLUDED__
__DPLAY_INCLUDED__ equ 1

;GUIDS used by DirectPlay objects
DefGUID CLSID_DirectPlay,<D1EB6D20-8923-11d0-9D97-00A0C90A43CB>
DefGUID IID_IDirectPlay2, <2b74f7c0-9154-11cf-a9cd-00aa006886e3>
DefGUID IID_IDirectPlay2A,<9d460580-a822-11cf-960c-0080c7534e82>
DefGUID IID_IDirectPlay3, <133efe40-32dc-11d0-9cfb-00a0c90a43cb>
DefGUID IID_IDirectPlay3A,<133efe41-32dc-11d0-9cfb-00a0c90a43cb>
DefGUID IID_IDirectPlay4, <0ab1c530-4745-11d1-a7a1-0000f803abfc>
DefGUID IID_IDirectPlay4A,<0ab1c531-4745-11d1-a7a1-0000f803abfc>

;GUIDS used by Service Providers shipped with DirectPlay
;Use these to identify Service Provider returned by EnumConnections
DefGUID DPSPGUID_IPX,	<685BC400-9D2C-11cf-A9CD-00AA006886E3>; GUID for IPX service provider
DefGUID DPSPGUID_TCPIP,	<36E95EE0-8577-11cf-960C-0080C7534E82> ; GUID for TCP/IP service provider
DefGUID DPSPGUID_SERIAL,<0F1D6860-88D9-11cf-9C4E-00A0C905425E> ; GUID for Serial service provider
DefGUID DPSPGUID_MODEM, <44EAA760-CB68-11cf-9C4E-00A0C905425E>; GUID for Modem service provider

;==============================================
;Various structures used to invoke DirectPlay.
;==============================================
DPID typedef dword
LPDPID typedef ptr LPID

DPID_SYSMSG			equ 0
DPID_ALLPLAYERS		equ 0
DPID_SERVERPLAYER	equ 1
DPID_RESERVEDRANGE	equ 100
DPID_UNKNOWN		equ -1

;DPCAPS - Used to obtain the capabilities of a DirectPlay object
DPCAPS struct
    dwSize dd ?;                 // Size of structure, in bytes
    dwFlags dd ?;                // DPCAPS_xxx flags
    dwMaxBufferSize dd ?;        // Maximum message size, in bytes,  for this service provider
    dwMaxQueueSize dd ?;         // Obsolete. 
    dwMaxPlayers dd ?;           // Maximum players/groups (local + remote)
    dwHundredBaud dd ?;          // Bandwidth in 100 bits per second units; 
                     ;           // i.e. 24 is 2400, 96 is 9600, etc.
    dwLatency dd ?;              // Estimated latency; 0 = unknown
    dwMaxLocalPlayers dd ?;      // Maximum # of locally created players allowed
    dwHeaderLength dd ?;         // Maximum header length, in bytes, on messages
                 ;               // added by the service provider
    dwTimeout dd ?;              // Service provider's suggested timeout value
                  ;              // This is how long DirectPlay will wait for 
                  ;              // responses to system messages
DPCAPS ends
LPDPCAPS typedef ptr DPCAPS

;This DirectPlay object is the session host.  If the host exits the
;session, another application will become the host and receive a
;DPSYS_HOST system message.
DPCAPS_ISHOST equ 2

;The service provider bound to this DirectPlay object can optimize group messaging.
DPCAPS_GROUPOPTIMIZED equ 8

;The service provider bound to this DirectPlay object can optimize
;keep alives (see DPSESSION_KEEPALIVE)
DPCAPS_KEEPALIVEOPTIMIZED equ 10h

;The service provider bound to this DirectPlay object can optimize guaranteed message delivery.
DPCAPS_GUARANTEEDOPTIMIZED equ 20h

;This DirectPlay object supports guaranteed message delivery.
DPCAPS_GUARANTEEDSUPPORTED  equ 40h

;This DirectPlay object supports digital signing of messages.
DPCAPS_SIGNINGSUPPORTED equ 80h

;This DirectPlay object supports encryption of messages.
DPCAPS_ENCRYPTIONSUPPORTED equ 100h

;This DirectPlay player was created on this machine
DPPLAYERCAPS_LOCAL equ 800h

;Current Open settings supports all forms of Cancel
DPCAPS_ASYNCCANCELSUPPORTED equ 1000h

;Current Open settings supports CancelAll, but not Cancel
DPCAPS_ASYNCCANCELALLSUPPORTED equ 2000h

;Current Open settings supports Send Timeouts for sends
DPCAPS_SENDTIMEOUTSUPPORTED equ 4000h

;Current Open settings supports send priority
DPCAPS_SENDPRIORITYSUPPORTED equ 8000h

;Current Open settings supports DPSEND_ASYNC flag
DPCAPS_ASYNCSUPPORTED 		 equ 10000h

;DPSESSIONDESC2 - Used to describe the properties of a DirectPlay session instance
DPSESSIONDESC2 struct
    dwSize dd ?;             // Size of structure
    dwFlags dd ?;            // DPSESSION_xxx flags
    guidInstance GUID <>;       // ID for the session instance
    guidApplication GUID <>;    // GUID of the DirectPlay application.
                              ;  // GUID_NULL for all applications.
    dwMaxPlayers dd ?;       // Maximum # players allowed in session
    dwCurrentPlayers dd ?;   // Current # players in session (read only)
    union
        lpszSessionName dd ?;    // Unicode
        lpszSessionNameA dd ?;   // ANSI
    ends
    union
        lpszPassword dd ?;       // Unicode
        lpszPasswordA dd ?;      // ANSI
    ends
    dwReserved1 dd ?;        // Reserved for future MS use.
    dwReserved2 dd ?;
    dwUser1 dd ?;            // For use by the application
    dwUser2 dd ?;
    dwUser3 dd ?;
    dwUser4 dd ?;
DPSESSIONDESC2 ends
LPDPSESSIONDESC2 typedef ptr DPSESSIONDESC2

;LPCDPSESSIONDESC2 - A constant pointer to DPSESSIONDESC2
 
;Applications cannot create new players in this session.
DPSESSION_NEWPLAYERSDISABLED equ 1

;If the DirectPlay object that created the session, the host,
;quits, then the host will attempt to migrate to another
;DirectPlay object so that new players can continue to be created
;and new applications can join the session.
DPSESSION_MIGRATEHOST equ 4

;This flag tells DirectPlay not to set the idPlayerTo and idPlayerFrom 
;fields in player messages.  This cuts two DWORD's off the message overhead.
DPSESSION_NOMESSAGEID equ 8

;This flag tells DirectPlay to not allow any new applications to
;join the session.  Applications already in the session can still
;create new players.
DPSESSION_JOINDISABLED equ 20h

;This flag tells DirectPlay to detect when remote players 
;exit abnormally (e.g. their computer or modem gets unplugged)
DPSESSION_KEEPALIVE equ 40h

;This flag tells DirectPlay not to send a message to all players
;when a players remote data changes
DPSESSION_NODATAMESSAGES equ 80h

;This flag indicates that the session belongs to a secure server
;and needs user authentication
DPSESSION_SECURESERVER equ 100h

;This flag indicates that the session is private and requires a password
;for EnumSessions as well as Open.
DPSESSION_PRIVATE equ 200h

;This flag indicates that the session requires a password for joining.
DPSESSION_PASSWORDREQUIRED equ 400h

;This flag tells DirectPlay to route all messages through the server
DPSESSION_MULTICASTSERVER equ 800h

;This flag tells DirectPlay to only download information about the DPPLAYER_SERVERPLAYER.  
DPSESSION_CLIENTSERVER equ 1000h

;This flag tells DirectPlay to use the protocol built into dplay
;for reliability and statistics all the time.  When this bit is set, 
;only other sessions with this bit set can join or be joined.
DPSESSION_DIRECTPLAYPROTOCOL	equ 2000h

;This flag tells DirectPlay that preserving order of received
;packets is not important, when using reliable delivery.  This
;will allow messages to be indicated out of order if preceding
;messages have not yet arrived.  Otherwise DPLAY will wait for
;earlier messages before delivering later reliable messages.
DPSESSION_NOPRESERVEORDER equ 4000h

;This flag tells DirectPlay to optimize communication for latency
DPSESSION_OPTIMIZELATENCY equ 8000h

;This flag allows lobby launched games that aren't voice enabled to get voice capabilities.
DPSESSION_ALLOWVOICERETRO equ 10000h

;This flag supresses transmission of session desc changes.
;DPSESSION_NODATAMESSAGES was supposed to do that, but SetSessionDesc
;was ignoring the flag and some apps depended on the broken behavior, this
;flag allows applications to get the right behaviour without breaking apps depending
;on old broken behavior.
DPSESSION_NOSESSIONDESCMESSAGES	equ 20000h
 
;DPNAME - Used to hold the name of a DirectPlay entity like a player or a group

DPNAME struct 
    dwSize dd ?;             // Size of structure
    dwFlags dd ?;;            // Not used. Must be zero.
    union ; The short or friendly name
        lpszShortName dd ?;;  // Unicode
        lpszShortNameA dd ?;; // ANSI
    ends
    union ; The long or formal name
        lpszLongName dd ?;;   // Unicode
        lpszLongNameA dd ?;;  // ANSI
    ends
DPNAME ends
LPDPNAME typedef ptr DPNAME

;LPCDPNAME - A constant pointer to DPNAME

;DPCREDENTIALS - Used to hold the user name and password of a DirectPlay user

DPCREDENTIALS struct 
    dwSize dd ?;               // Size of structure
    dwFlags dd ?;              // Not used. Must be zero.
    union
        lpszUsername dd ?;   // Unicode
        lpszUsernameA dd ?;  // ANSI
    ends  
    union
        lpszPassword dd ?;   // Unicode
        lpszPasswordA dd ?;  // ANSI
    ends    
    union
        lpszDomain dd ?;     // Unicode
        lpszDomainA dd ?;    // ANSI
    ends
DPCREDENTIALS ends
LPDPCREDENTIALS typedef ptr DPCREDENTIALS


;DPSECURITYDESC - Used to describe the security properties of a DirectPlay session instance
DPSECURITYDESC struct 
    dwSize dd ?;                   // Size of structure
    dwFlags dd ?;                  // Not used. Must be zero.
    union ;SSPI provider name
        lpszSSPIProvider dd ?;   // Unicode
        lpszSSPIProviderA dd ?;  // ANSI
    ends
    union ;CAPI provider name
        lpszCAPIProvider dd ?;    // Unicode
        lpszCAPIProviderA dd ?;   // ANSI
    ends
    dwCAPIProviderType dd ?;       // Crypto Service Provider type
    dwEncryptionAlgorithm dd ?;    // Encryption Algorithm type
DPSECURITYDESC ends
LPDPSECURITYDESC typedef ptr DPSECURITYDESC

;DPACCOUNTDESC - Used to describe a user membership account
DPACCOUNTDESC struct 
    dwSize dd ?;                   // Size of structure
    dwFlags dd ?;                  // Not used. Must be zero.
    union ;Account identifier
        lpszAccountID  dd ?;      // Unicode
        lpszAccountIDA dd ?;     // ANSI
    ends
DPACCOUNTDESC ends
LPDPACCOUNTDESC typedef ptr DPACCOUNTDESC

;DPLCONNECTION
;Used to hold all in the informaion needed to connect
;an application to a session or create a session
DPLCONNECTION struct
    dwSize dd ?;             // Size of this structure
    dwFlags dd ?;            // Flags specific to this structure
    lpSessionDesc LPDPSESSIONDESC2 ?;      // Pointer to session desc to use on connect
    lpPlayerName LPDPNAME ?;       // Pointer to Player name structure
    guidSP GUID <>           ;             // GUID of the DPlay SP to use
    lpAddress dd ?;          // Address for service provider
    dwAddressSize dd ?;      // Size of address data
DPLCONNECTION ends
LPDPLCONNECTION typedef ptr DPLCONNECTION

;DPCHAT - Used to hold the a DirectPlay chat message
DPCHAT struct
    dwSize dd ?;
    dwFlags dd ?;
    union
        lpszMessage dd ?;   // Unicode
        lpszMessageA dd ?;  // ANSI
    ends
DPCHAT ends
LPDPCHAT typedef ptr DPCHAT

;SGBUFFER - Scatter Gather Buffer used for SendEx
SGBUFFER struct 
	len dd ?;       // length of buffer data
	pData dd ?;		// pointer to buffer data
SGBUFFER ends

;This flag is set on the EnumSessions callback dwFlags parameter when
;the time out has occurred. There will be no session data for this 
;callback. If *lpdwTimeOut is set to a non-zero value and the 
;EnumSessionsCallback function returns TRUE then EnumSessions will 
;continue waiting until the next timeout occurs. Timeouts are in 
;milliseconds.
DPESC_TIMEDOUT equ 1

; * Callback for IDirectPlay2::EnumPlayers
; *              IDirectPlay2::EnumGroups
; *              IDirectPlay2::EnumGroupPlayers
; */
;typedef BOOL (FAR PASCAL *LPDPENUMPLAYERSCALLBACK2)(
;    DPID            dpId,
;    DWORD           dwPlayerType,
;    LPCDPNAME       lpName,
;    DWORD           dwFlags,
;    LPVOID          lpContext );
;/*
; * Unicode callback for DirectPlayEnumerate
; * This callback prototype will be used if compiling
; * for Unicode strings
; */
;typedef BOOL (FAR PASCAL * LPDPENUMDPCALLBACK)(
;    LPGUID      lpguidSP,
;    LPWSTR      lpSPName,
;    DWORD       dwMajorVersion,
;    DWORD       dwMinorVersion,
;    LPVOID      lpContext);
;
;/*
; * ANSI callback for DirectPlayEnumerate
; * This callback prototype will be used if compiling
; * for ANSI strings
; */
;typedef BOOL (FAR PASCAL * LPDPENUMDPCALLBACKA)(
;    LPGUID      lpguidSP,
;    LPSTR       lpSPName,
;    DWORD       dwMajorVersion,     
;    DWORD       dwMinorVersion,
;    LPVOID      lpContext);
;
;/*
; * Callback for IDirectPlay3(A)::EnumConnections
; */
;typedef BOOL (FAR PASCAL * LPDPENUMCONNECTIONSCALLBACK)(
;    LPCGUID     lpguidSP,
;	LPVOID		lpConnection,
;	DWORD		dwConnectionSize,
;    LPCDPNAME   lpName,
;	DWORD 		dwFlags,
;	LPVOID 		lpContext);
;

DirectPlayEnumerateA proto :dword,:dword
DirectPlayEnumerateW proto :dword,:dword

;Don't use this API, it's no longer supported.
;extern HRESULT WINAPI DirectPlayCreate( LPGUID lpGUID, LPDIRECTPLAY *lplpDP, IUnknown *pUnk);

;==========================================
;IDirectPlay2 (and IDirectPlay2A) Interface
;==========================================
;DEFINE_GUID(IID_IDirectPlay2, 0x2b74f7c0, 0x9154, 0x11cf, 0xa9, 0xcd, 0x0, 0xaa, 0x0, 0x68, 0x86, 0xe3);
BEGIN_INTERFACE IDirectPlay2, IUnknown, <2b74f7c0-9154-11cf-a9cd-00aa006886e3>
    STDMETHOD AddPlayerToGroup,dword,dword
    STDMETHOD Close
    STDMETHOD CreateGroup,Pointer,Pointer,dword,dword,dword
    STDMETHOD CreatePlayer,Pointer,Pointer,HANDLE,Pointer,DWORD,DWORD
    STDMETHOD DeletePlayerFromGroup,dword,dword
    STDMETHOD DestroyGroup,dword
    STDMETHOD DestroyPlayer,dword
    STDMETHOD EnumGroupPlayers,dword,Pointer,Pointer,Pointer,DWORD
    STDMETHOD EnumGroups,Pointer,Pointer,Pointer,DWORD
    STDMETHOD EnumPlayers,Pointer,Pointer,Pointer,DWORD
    STDMETHOD EnumSessions,Pointer,DWORD,Pointer,Pointer,DWORD
    STDMETHOD GetCaps,Pointer,DWORD
    STDMETHOD GetGroupData,dword,Pointer,Pointer,DWORD
    STDMETHOD GetGroupName,DWORD,Pointer,Pointer
    STDMETHOD GetMessageCount,DWORD, Pointer
    STDMETHOD GetPlayerAddress,DWORD,Pointer,Pointer
    STDMETHOD GetPlayerCaps,DWORD,Pointer,DWORD
    STDMETHOD GetPlayerData,DWORD,Pointer,Pointer,DWORD
    STDMETHOD GetPlayerName,DWORD,Pointer,Pointer
    STDMETHOD GetSessionDesc,Pointer,Pointer
    STDMETHOD Initialize,Pointer
    STDMETHOD Open,LPDPSESSIONDESC2,DWORD
    STDMETHOD Receive,Pointer,Pointer,DWORD,Pointer,Pointer
    STDMETHOD Send,DWORD, DPID, DWORD, Pointer, DWORD
    STDMETHOD SetGroupData,DWORD,Pointer,DWORD,DWORD
    STDMETHOD SetGroupName,DWORD,Pointer,DWORD
    STDMETHOD SetPlayerData,DWORD,Pointer,DWORD,DWORD
    STDMETHOD SetPlayerName,DWORD,Pointer,DWORD
    STDMETHOD SetSessionDesc,LPDPSESSIONDESC2,DWORD
END_INTERFACE

;DEFINE_GUID(IID_IDirectPlay2A,0x9d460580, 0xa822, 0x11cf, 0x96, 0xc, 0x0, 0x80, 0xc7, 0x53, 0x4e, 0x82);
BEGIN_INTERFACE IDirectPlay2A, IUnknown, <9d460580-a822-11cf-960c-0080c7534e82>
    STDMETHOD AddPlayerToGroup,dword,dword
    STDMETHOD Close
    STDMETHOD CreateGroup,Pointer,Pointer,dword,dword,dword
    STDMETHOD CreatePlayer,Pointer,Pointer,HANDLE,Pointer,DWORD,DWORD
    STDMETHOD DeletePlayerFromGroup,dword,dword
    STDMETHOD DestroyGroup,dword
    STDMETHOD DestroyPlayer,dword
    STDMETHOD EnumGroupPlayers,dword,Pointer,Pointer,Pointer,DWORD
    STDMETHOD EnumGroups,Pointer,Pointer,Pointer,DWORD
    STDMETHOD EnumPlayers,Pointer,Pointer,Pointer,DWORD
    STDMETHOD EnumSessions,Pointer,DWORD,Pointer,Pointer,DWORD
    STDMETHOD GetCaps,Pointer,DWORD
    STDMETHOD GetGroupData,dword,Pointer,Pointer,DWORD
    STDMETHOD GetGroupName,DWORD,Pointer,Pointer
    STDMETHOD GetMessageCount,DWORD, Pointer
    STDMETHOD GetPlayerAddress,DWORD,Pointer,Pointer
    STDMETHOD GetPlayerCaps,DWORD,Pointer,DWORD
    STDMETHOD GetPlayerData,DWORD,Pointer,Pointer,DWORD
    STDMETHOD GetPlayerName,DWORD,Pointer,Pointer
    STDMETHOD GetSessionDesc,Pointer,Pointer
    STDMETHOD Initialize,Pointer
    STDMETHOD Open,LPDPSESSIONDESC2,DWORD
    STDMETHOD Receive,Pointer,Pointer,DWORD,Pointer,Pointer
    STDMETHOD Send,DWORD, DPID, DWORD, Pointer, DWORD
    STDMETHOD SetGroupData,DWORD,Pointer,DWORD,DWORD
    STDMETHOD SetGroupName,DWORD,Pointer,DWORD
    STDMETHOD SetPlayerData,DWORD,Pointer,DWORD,DWORD
    STDMETHOD SetPlayerName,DWORD,Pointer,DWORD
    STDMETHOD SetSessionDesc,LPDPSESSIONDESC2,DWORD
END_INTERFACE

;==========================================
;IDirectPlay3 (and IDirectPlay3A) Interface
;==========================================
;DEFINE_GUID(IID_IDirectPlay3, 0x133efe40, 0x32dc, 0x11d0, 0x9c, 0xfb, 0x0, 0xa0, 0xc9, 0xa, 0x43, 0xcb);
BEGIN_INTERFACE IDirectPlay3, IDirectPlay2, <133efe40-32dc-11d0-9cfb-00a0c90a43cb>
    STDMETHOD AddGroupToGroup,dword, dword
    STDMETHOD CreateGroupInGroup,dword,Pointer,Pointer,Pointer,DWORD,DWORD
    STDMETHOD DeleteGroupFromGroup,dword,dword	
    STDMETHOD EnumConnections,Pointer,Pointer,Pointer,DWORD
    STDMETHOD EnumGroupsInGroup,dword,Pointer,Pointer,Pointer,DWORD
	STDMETHOD GetGroupConnectionSettings,DWORD, dword, Pointer, Pointer
	STDMETHOD InitializeConnection,Pointer,DWORD
    STDMETHOD SecureOpen,Pointer,DWORD,Pointer,Pointer
	STDMETHOD SendChatMessage,dword,DPID,DWORD,LPDPCHAT
	STDMETHOD SetGroupConnectionSettings,DWORD,DPID,LPDPLCONNECTION
	STDMETHOD StartSession,DWORD,DWORD
	STDMETHOD GetGroupFlags,dword,Pointer
	STDMETHOD GetGroupParent,dword,Pointer
	STDMETHOD GetPlayerAccount,dword, DWORD, Pointer, Pointer
	STDMETHOD GetPlayerFlags,dword,Pointer
END_INTERFACE

;DEFINE_GUID(IID_IDirectPlay3A,0x133efe41, 0x32dc, 0x11d0, 0x9c, 0xfb, 0x0, 0xa0, 0xc9, 0xa, 0x43, 0xcb);
BEGIN_INTERFACE IDirectPlay3A, IDirectPlay2A, <133efe41-32dc-11d0-9cfb-00a0c90a43cb>
    STDMETHOD AddGroupToGroup,dword, dword
    STDMETHOD CreateGroupInGroup,dword,Pointer,Pointer,Pointer,DWORD,DWORD
    STDMETHOD DeleteGroupFromGroup,dword,dword	
    STDMETHOD EnumConnections,Pointer,Pointer,Pointer,DWORD
    STDMETHOD EnumGroupsInGroup,dword,Pointer,Pointer,Pointer,DWORD
	STDMETHOD GetGroupConnectionSettings,DWORD, dword, Pointer, Pointer
	STDMETHOD InitializeConnection,Pointer,DWORD
    STDMETHOD SecureOpen,Pointer,DWORD,Pointer,Pointer
	STDMETHOD SendChatMessage,dword,DPID,DWORD,LPDPCHAT
	STDMETHOD SetGroupConnectionSettings,DWORD,DPID,LPDPLCONNECTION
	STDMETHOD StartSession,DWORD,DWORD
	STDMETHOD GetGroupFlags,dword,Pointer
	STDMETHOD GetGroupParent,dword,Pointer
	STDMETHOD GetPlayerAccount,dword, DWORD, Pointer, Pointer
	STDMETHOD GetPlayerFlags,dword,Pointer
END_INTERFACE

;==========================================
;IDirectPlay4 (and IDirectPlay4A) Interface
;==========================================
;DEFINE_GUID(IID_IDirectPlay4, 0xab1c530, 0x4745, 0x11d1, 0xa7, 0xa1, 0x0, 0x0, 0xf8, 0x3, 0xab, 0xfc);
BEGIN_INTERFACE IDirectPlay4, IDirectPlay3,<ab1c530-4745-11d1-a7a1-0000f803abfc>
    STDMETHOD GetGroupOwner,dword, Pointer
    STDMETHOD SetGroupOwner,dword, DWORD
    STDMETHOD SendEx,dword, DWORD, DWORD, Pointer, DWORD, DWORD, DWORD, Pointer,Pointer
    STDMETHOD GetMessageQueue,dword, DWORD, DWORD, Pointer, Pointer
    STDMETHOD CancelMessage,DWORD, DWORD
    STDMETHOD CancelPriority,DWORD, DWORD, DWORD
END_INTERFACE

;DEFINE_GUID(IID_IDirectPlay4A,0xab1c531, 0x4745, 0x11d1, 0xa7, 0xa1, 0x0, 0x0, 0xf8, 0x3, 0xab, 0xfc);
BEGIN_INTERFACE IDirectPlay4A, IDirectPlay3A,<ab1c531-4745-11d1-a7a1-0000f803abfc>
    STDMETHOD GetGroupOwner,dword, Pointer
    STDMETHOD SetGroupOwner,dword, DWORD
    STDMETHOD SendEx,dword, DWORD, DWORD, Pointer, DWORD, DWORD, DWORD, Pointer,Pointer
    STDMETHOD GetMessageQueue,dword, DWORD, DWORD, Pointer, Pointer
    STDMETHOD CancelMessage,DWORD, DWORD
    STDMETHOD CancelPriority,DWORD, DWORD, DWORD
END_INTERFACE

;==========================================
;EnumConnections API flags...
;==========================================
;Enumerate Service Providers
DPCONNECTION_DIRECTPLAY equ 1
;Enumerate Lobby Providers
DPCONNECTION_DIRECTPLAYLOBBY equ 2


;==========================================
;EnumPlayers API flags...
;==========================================
;Enumerate all players in the current session
DPENUMPLAYERS_ALL  equ 0
DPENUMGROUPS_ALL   equ DPENUMPLAYERS_ALL

;Enumerate only local (created by this application) players or groups
DPENUMPLAYERS_LOCAL  equ 8
DPENUMGROUPS_LOCAL	 equ DPENUMPLAYERS_LOCAL

;Enumerate only remote (non-local) players or groups
DPENUMPLAYERS_REMOTE equ 10h
DPENUMGROUPS_REMOTE	 equ DPENUMPLAYERS_REMOTE

;Enumerate groups along with the players
DPENUMPLAYERS_GROUP  equ 20h

;Enumerate players or groups in another session (must supply lpguidInstance)
DPENUMPLAYERS_SESSION equ 80h
DPENUMGROUPS_SESSION  equ DPENUMPLAYERS_SESSION

;Enumerate server players
DPENUMPLAYERS_SERVERPLAYER  equ 100h

;Enumerate spectator players
DPENUMPLAYERS_SPECTATOR  equ 200h

;Enumerate shortcut groups
DPENUMGROUPS_SHORTCUT equ 400h

;Enumerate staging area groups
DPENUMGROUPS_STAGINGAREA equ 800h

;Enumerate hidden groups
DPENUMGROUPS_HIDDEN  equ 1000h

;Enumerate the group's owner
DPENUMPLAYERS_OWNER	equ 2000h

;==========================================
;CreatePlayer API flags...
;==========================================
;This flag indicates that this player should be designated
;the server player. The app should specify this at CreatePlayer.
DPPLAYER_SERVERPLAYER  equ DPENUMPLAYERS_SERVERPLAYER

;This flag indicates that this player should be designated
;a spectator. The app should specify this at CreatePlayer.
DPPLAYER_SPECTATOR  equ DPENUMPLAYERS_SPECTATOR

;This flag indicates that this player was created locally.
;(returned from GetPlayerFlags)
DPPLAYER_LOCAL equ DPENUMPLAYERS_LOCAL

;This flag indicates that this player is the group's owner
;(Only returned in EnumGroupPlayers)
DPPLAYER_OWNER equ DPENUMPLAYERS_OWNER

;==========================================
;CreateGroup API flags
;==========================================
;This flag indicates that the StartSession can be called on the group.
;The app should specify this at CreateGroup, or CreateGroupInGroup.
DPGROUP_STAGINGAREA     equ        DPENUMGROUPS_STAGINGAREA

;This flag indicates that this group was created locally.
;(returned from GetGroupFlags)
DPGROUP_LOCAL   equ    DPENUMGROUPS_LOCAL

;This flag indicates that this group was created hidden.
DPGROUP_HIDDEN   equ DPENUMGROUPS_HIDDEN

;==========================================
;EnumSessions API flags...
;==========================================
;Enumerate sessions which can be joined
DPENUMSESSIONS_AVAILABLE  equ 1

;Enumerate all sessions even if they can't be joined.
DPENUMSESSIONS_ALL        equ 2

;Start an asynchronous enum sessions
DPENUMSESSIONS_ASYNC	equ 10h

;Stop an asynchronous enum sessions
DPENUMSESSIONS_STOPASYNC equ 20h
 
;Enumerate sessions even if they require a password
DPENUMSESSIONS_PASSWORDREQUIRED	equ 40h

;Return status about progress of enumeration instead of showing any status dialogs.
DPENUMSESSIONS_RETURNSTATUS equ 80h

;==========================================
;GetCaps and GetPlayerCaps API flags...
;==========================================
;The latency returned should be for guaranteed message sending.
;Default is non-guaranteed messaging.
DPGETCAPS_GUARANTEED  equ 1
 
;GetGroupData, GetPlayerData API flags
;Remote and local Group/Player data is maintained separately. 
;Default is DPGET_REMOTE.

;Get the remote data (set by any DirectPlay object in the session using DPSET_REMOTE)
DPGET_REMOTE equ 0
	
;Get the local data (set by this DirectPlay object using DPSET_LOCAL)
DPGET_LOCAL equ 1


;==========================================
;Open API flags...
;==========================================
;Join the session that is described by the DPSESSIONDESC2 structure
DPOPEN_JOIN equ 1

;Create a new session as described by the DPSESSIONDESC2 structure
DPOPEN_CREATE  equ 2

;Return status about progress of open instead of showing any status dialogs.
DPOPEN_RETURNSTATUS	equ	DPENUMSESSIONS_RETURNSTATUS


;==========================================
;DPLCONNECTION flags...
;==========================================
;This application should create a new session as
;described by the DPSESIONDESC structure
DPLCONNECTION_CREATESESSION	equ DPOPEN_CREATE

;This application should join the session described by 
;the DPSESIONDESC structure with the lpAddress data
DPLCONNECTION_JOINSESSION	equ	DPOPEN_JOIN

;==========================================
;Receive API flags
;Default is DPRECEIVE_ALL
;==========================================
;Get the first message in the queue
DPRECEIVE_ALL  equ 1

;Get the first message in the queue directed to a specific player 
DPRECEIVE_TOPLAYER  equ 2

;Get the first message in the queue from a specific player
DPRECEIVE_FROMPLAYER equ 4

;Get the message but don't remove it from the queue
DPRECEIVE_PEEK equ 8

;==========================================
;Send API flags
;==========================================
;Send the message using a guaranteed send method.
;Default is non-guaranteed.
DPSEND_GUARANTEED  equ 1

;
;/*
; * This flag is obsolete. It is ignored by DirectPlay
; */
;#define DPSEND_HIGHPRIORITY         0x00000002
;
;/*
; * This flag is obsolete. It is ignored by DirectPlay
; */
;#define DPSEND_OPENSTREAM           0x00000008
;
;/*
; * This flag is obsolete. It is ignored by DirectPlay
; */
;#define DPSEND_CLOSESTREAM          0x00000010

;Send the message digitally signed to ensure authenticity.
DPSEND_SIGNED equ 20h

;Send the message with encryption to ensure privacy.
DPSEND_ENCRYPTED equ 40h

;The message is a lobby system message
DPSEND_LOBBYSYSTEMMESSAGE equ 80h

;Send message asynchronously, must check caps
;before using this flag.  It is always provided
;if the protocol flag is set.
DPSEND_ASYNC equ 200h

;When an message is completed, don't tell me. 
;by default the application is notified with a system message.
DPSEND_NOSENDCOMPLETEMSG  equ 400h

;Maximum priority for sends available to applications
DPSEND_MAX_PRI  equ 0FFFFh
DPSEND_MAX_PRIORITY      equ   DPSEND_MAX_PRI


;====================================================================================
; SetGroupData, SetGroupName, SetPlayerData, SetPlayerName, SetSessionDesc API flags.
; Default is DPSET_REMOTE.
;====================================================================================
;Propagate the data to all players in the session
DPSET_REMOTE equ 0

;Do not propagate the data to other players
DPSET_LOCAL equ 1

;Used with DPSET_REMOTE, use guaranteed message send to propagate the data
DPSET_GUARANTEED equ 2

;=============================================
;GetMessageQueue API flags.
;Default is DPMESSAGEQUEUE_SEND
;=============================================
;Get Send Queue - requires Service Provider Support
DPMESSAGEQUEUE_SEND equ 1

;Get Receive Queue
DPMESSAGEQUEUE_RECEIVE equ 2

;=============================================
;Connect API flags
;=============================================
;Start an asynchronous connect which returns status codes
DPCONNECT_RETURNSTATUS  equ  DPENUMSESSIONS_RETURNSTATUS


;============================================================================
; DirectPlay system messages and message data structures
; All system message come 'From' player DPID_SYSMSG.  To determine what type 
; of message it is, cast the lpData from Receive to DPMSG_GENERIC and check
; the dwType member against one of the following DPSYS_xxx constants. Once
; a match is found, cast the lpData to the corresponding of the DPMSG_xxx
; structures to access the data of the message.
;============================================================================
;A new player or group has been created in the session
;Use DPMSG_CREATEPLAYERORGROUP.  Check dwPlayerType to see if its a player or a group.
DPSYS_CREATEPLAYERORGROUP equ 3 

;A player has been deleted from the session
;Use DPMSG_DESTROYPLAYERORGROUP
DPSYS_DESTROYPLAYERORGROUP equ 5 

;A player has been added to a group
;Use DPMSG_ADDPLAYERTOGROUP
DPSYS_ADDPLAYERTOGROUP    equ 7

;A player has been removed from a group
;Use DPMSG_DELETEPLAYERFROMGROUP
DPSYS_DELETEPLAYERFROMGROUP equ 21h  

;This DirectPlay object lost its connection with all the other players in the session.
;Use DPMSG_SESSIONLOST.
DPSYS_SESSIONLOST equ 31h

;The current host has left the session.
;This DirectPlay object is now the host.
;Use DPMSG_HOST.
DPSYS_HOST  equ 101h

;The remote data associated with a player or
;group has changed. Check dwPlayerType to see
;if it is a player or a group
;Use DPMSG_SETPLAYERORGROUPDATA
DPSYS_SETPLAYERORGROUPDATA  equ 102h

;The name of a player or group has changed.
;Check dwPlayerType to see if it is a player or a group.
;Use DPMSG_SETPLAYERORGROUPNAME
DPSYS_SETPLAYERORGROUPNAME  equ 103h

;The session description has changed.
;Use DPMSG_SETSESSIONDESC
DPSYS_SETSESSIONDESC equ 104h

;A group has been added to a group
;Use DPMSG_ADDGROUPTOGROUP
DPSYS_ADDGROUPTOGROUP  equ 105h

;A group has been removed from a group
;Use DPMSG_DELETEGROUPFROMGROUP
DPSYS_DELETEGROUPFROMGROUP 	equ 106h

;A secure player-player message has arrived.
;Use DPMSG_SECUREMESSAGE
DPSYS_SECUREMESSAGE   equ 107h

;Start a new session.
;Use DPMSG_STARTSESSION
DPSYS_STARTSESSION   equ 108h

;A chat message has arrived
;Use DPMSG_CHAT
DPSYS_CHAT       equ 109h

;The owner of a group has changed
;Use DPMSG_SETGROUPOWNER
DPSYS_SETGROUPOWNER   equ 10Ah

;An async send has finished, failed or been cancelled
;Use DPMSG_SENDCOMPLETE
DPSYS_SENDCOMPLETE  equ 10Dh

;Used in the dwPlayerType field to indicate if it applies to a group or a player
DPPLAYERTYPE_GROUP  equ 0
DPPLAYERTYPE_PLAYER equ 1

;DPMSG_GENERIC
;Generic message structure used to identify the message type.
DPMSG_GENERIC struct
   dwType dd ?;         // Message type
DPMSG_GENERIC ends
LPDPMSG_GENERIC typedef ptr DPMSG_GENERIC

;DPMSG_CREATEPLAYERORGROUP
;System message generated when a new player or group
;created in the session with information about it.
DPMSG_CREATEPLAYERORGROUP struct
    dwType dd ?;         // Message type
    wPlayerType dd ?;   // Is it a player or group
    dpId dd ?;           // ID of the player or group
    dwCurrentPlayers dd ?;   // current # players & groups in session
    lpData dd ?;         // pointer to remote data
    dwDataSize dd ?;     // size of remote data
    dpnName DPNAME <>;        // structure with name info
	; the following fields are only available when using
	; the IDirectPlay3 interface or greater
    dpIdParent dd ?;     // id of parent group
	dwFlags dd ?;		// player or group flags
DPMSG_CREATEPLAYERORGROUP ends
LPDPMSG_CREATEPLAYERORGROUP typedef ptr DPMSG_CREATEPLAYERORGROUP

;DPMSG_DESTROYPLAYERORGROUP
;System message generated when a player or group is being
;destroyed in the session with information about it.
DPMSG_DESTROYPLAYERORGROUP struct
    dwType dd ?;         // Message type
    dwPlayerType dd ?;   // Is it a player or group
    dpId dd ?;           // player ID being deleted
    lpLocalData dd ?;    // copy of players local data
    dwLocalDataSize dd ?; // sizeof local data
    lpRemoteData dd ?;   // copy of players remote data
    dwRemoteDataSize dd ?; // sizeof remote data
	; the following fields are only available when using
	; the IDirectPlay3 interface or greater
    dpnName DPNAME <>;        // structure with name info
    dpIdParent dd ?;     // id of parent group	
	dwFlags dd ?;		// player or group flags
DPMSG_DESTROYPLAYERORGROUP ends
LPDPMSG_DESTROYPLAYERORGROUP typedef ptr DPMSG_DESTROYPLAYERORGROUP

;DPMSG_ADDPLAYERTOGROUP
;System message generated when a player is being added to a group.
DPMSG_ADDPLAYERTOGROUP struct
    dwType dd ?;         // Message type
    dpIdGroup dd ?;      // group ID being added to
    dpIdPlayer dd ?;     // player ID being added
DPMSG_ADDPLAYERTOGROUP ends
LPDPMSG_ADDPLAYERTOGROUP typedef ptr DPMSG_ADDPLAYERTOGROUP

;DPMSG_DELETEPLAYERFROMGROUP
;System message generated when a player is being removed from a group
DPMSG_DELETEPLAYERFROMGROUP typedef DPMSG_ADDPLAYERTOGROUP
LPDPMSG_DELETEPLAYERFROMGROUP typedef ptr DPMSG_DELETEPLAYERFROMGROUP

;DPMSG_ADDGROUPTOGROUP
;System message generated when a group is being added to a group.
DPMSG_ADDGROUPTOGROUP struct
    dwType dd ?;         // Message type
    dpIdParentGroup dd ?; // group ID being added to
    dpIdGroup dd ?;     // group ID being added
DPMSG_ADDGROUPTOGROUP ends
LPDPMSG_ADDGROUPTOGROUP typedef ptr DPMSG_ADDGROUPTOGROUP

;DPMSG_DELETEGROUPFROMGROUP
;System message generated when a GROUP is being removed from a group

DPMSG_DELETEGROUPFROMGROUP typedef DPMSG_ADDGROUPTOGROUP
LPDPMSG_DELETEGROUPFROMGROUP typedef ptr DPMSG_DELETEGROUPFROMGROUP  

;DPMSG_SETPLAYERORGROUPDATA
;System message generated when remote data for a player or group has changed.
DPMSG_SETPLAYERORGROUPDATA struct
    dwType dd ?;         // Message type
    dwPlayerType dd ?;   // Is it a player or group
    dpId dd ?;           // ID of player or group
    lpData dd ?;         // pointer to remote data
    dwDataSize dd ?;     // size of remote data
DPMSG_SETPLAYERORGROUPDATA ends
LPDPMSG_SETPLAYERORGROUPDATA typedef ptr DPMSG_SETPLAYERORGROUPDATA

;DPMSG_SETPLAYERORGROUPNAME
;System message generated when the name of a player or group has changed.
DPMSG_SETPLAYERORGROUPNAME struct
    dwType dd ?;         // Message type
    dwPlayerType dd ?;   // Is it a player or group
    dpId dd ?;           // ID of player or group
    dpnName DPNAME <>;        // structure with new name info
DPMSG_SETPLAYERORGROUPNAME ends
LPDPMSG_SETPLAYERORGROUPNAME typedef ptr DPMSG_SETPLAYERORGROUPNAME

;DPMSG_SETSESSIONDESC
;System message generated when session desc has changed
DPMSG_SETSESSIONDESC struct
	dwType dd ?;     // Message type
    dpDesc DPSESSIONDESC2 <>;     // Session desc
DPMSG_SETSESSIONDESC ends
LPDPMSG_SETSESSIONDESC typedef ptr DPMSG_SETSESSIONDESC

;DPMSG_HOST
;System message generated when the host has migrated to this DirectPlay object.
DPMSG_HOST typedef DPMSG_GENERIC
LPDPMSG_HOST typedef ptr DPMSG_HOST

;DPMSG_SESSIONLOST
;System message generated when the connection to the session is lost.
DPMSG_SESSIONLOST typedef DPMSG_GENERIC       
LPDPMSG_SESSIONLOST typedef ptr DPMSG_SESSIONLOST

;DPMSG_SECUREMESSAGE
;System message generated when a player requests a secure send
DPMSG_SECUREMESSAGE struct 
    dwType dd ?;         // Message Type
    dwFlags dd ?;        // Signed/Encrypted
    dpIdFrom dd ?;       // ID of Sending Player
    lpData dd ?;         // Player message
    dwDataSize dd ?;     // Size of player message
DPMSG_SECUREMESSAGE ends
LPDPMSG_SECUREMESSAGE typedef ptr DPMSG_SECUREMESSAGE

;DPMSG_STARTSESSION
;System message containing all information required to start a new session
DPMSG_STARTSESSION struct
	dwType dd ?;     // Message type
    lpConn LPDPLCONNECTION ?;     // DPLCONNECTION structure
DPMSG_STARTSESSION ends
LPDPMSG_STARTSESSION typedef ptr DPMSG_STARTSESSION

;DPMSG_CHAT
;System message containing a chat message
DPMSG_CHAT struct
    dwType dd ?;       	// Message type
    dwFlags dd ?;      	// Message flags
    idFromPlayer dd ?; 	// ID of the Sending Player
    idToPlayer dd ?;   	// ID of the To Player
    idToGroup dd ?;    	// ID of the To Group
	lpChat dd ?;			// Pointer to a structure containing the chat message
DPMSG_CHAT ends
LPDPMSG_CHAT typedef ptr DPMSG_CHAT

;DPMSG_SETGROUPOWNER
;System message generated when the owner of a group has changed
DPMSG_SETGROUPOWNER struct
    dwType dd ?;         // Message type
    idGroup dd ?;        // ID of the group
    idNewOwner dd ?;     // ID of the player that is the new owner
    idOldOwner dd ?;     // ID of the player that used to be the owner
DPMSG_SETGROUPOWNER ends
LPDPMSG_SETGROUPOWNER typedef ptr DPMSG_SETGROUPOWNER

;DPMSG_SENDCOMPLETE
;System message generated when finished with an Async Send message
;NOTE SENDPARMS has an overlay for DPMSG_SENDCOMPLETE, don't
;change this message w/o changing SENDPARMS.
DPMSG_SENDCOMPLETE struct
	dwType dd ?;
	idFrom dd ?;
	idTo dd ?;
	dwFlags dd ?;
	dwPriority dd ?;
	dwTimeout dd ?;
	lpvContext dd ?;
	dwMsgID dd ?;
	hr dd ?;
	dwSendTime dd ?;
DPMSG_SENDCOMPLETE ends
LPDPMSG_SENDCOMPLETE typedef ptr DPMSG_SENDCOMPLETE

;====================================================================
;DIRECTPLAY ERRORS
;Errors are represented by negative values and cannot be combined.
;====================================================================
DP_OK                       equ    S_OK
DPERR_ALREADYINITIALIZED   equ     MAKE_DPHRESULT(   5 )
DPERR_ACCESSDENIED       equ       MAKE_DPHRESULT(  10 )
DPERR_ACTIVEPLAYERS     equ        MAKE_DPHRESULT(  20 )
DPERR_BUFFERTOOSMALL     equ       MAKE_DPHRESULT(  30 )
DPERR_CANTADDPLAYER       equ      MAKE_DPHRESULT(  40 )
DPERR_CANTCREATEGROUP       equ    MAKE_DPHRESULT(  50 )
DPERR_CANTCREATEPLAYER    equ      MAKE_DPHRESULT(  60 )
DPERR_CANTCREATESESSION    equ     MAKE_DPHRESULT(  70 )
DPERR_CAPSNOTAVAILABLEYET  equ     MAKE_DPHRESULT(  80 )
DPERR_EXCEPTION           equ      MAKE_DPHRESULT(  90 )
DPERR_GENERIC             equ      E_FAIL
DPERR_INVALIDFLAGS         equ     MAKE_DPHRESULT( 120 )
DPERR_INVALIDOBJECT          equ   MAKE_DPHRESULT( 130 )
DPERR_INVALIDPARAM         equ     E_INVALIDARG
DPERR_INVALIDPARAMS      equ       DPERR_INVALIDPARAM
DPERR_INVALIDPLAYER     equ        MAKE_DPHRESULT( 150 )
DPERR_INVALIDGROUP    equ         	MAKE_DPHRESULT( 155 )
DPERR_NOCAPS           equ         MAKE_DPHRESULT( 160 )
DPERR_NOCONNECTION      equ        MAKE_DPHRESULT( 170 )
DPERR_NOMEMORY         equ         E_OUTOFMEMORY
DPERR_OUTOFMEMORY       equ        DPERR_NOMEMORY
DPERR_NOMESSAGES          equ      MAKE_DPHRESULT( 190 )
DPERR_NONAMESERVERFOUND  equ       MAKE_DPHRESULT( 200 )
DPERR_NOPLAYERS         equ        MAKE_DPHRESULT( 210 )
DPERR_NOSESSIONS      equ          MAKE_DPHRESULT( 220 )
DPERR_PENDING		equ			   E_PENDING
DPERR_SENDTOOBIG	equ			   MAKE_DPHRESULT( 230 )
DPERR_TIMEOUT         equ          MAKE_DPHRESULT( 240 )
DPERR_UNAVAILABLE      equ         MAKE_DPHRESULT(250)
DPERR_UNSUPPORTED        equ       E_NOTIMPL
DPERR_BUSY              equ        MAKE_DPHRESULT( 270 )
DPERR_USERCANCEL       equ         MAKE_DPHRESULT( 280 ) 
DPERR_NOINTERFACE        equ       E_NOINTERFACE
DPERR_CANNOTCREATESERVER  equ      MAKE_DPHRESULT( 290 )
DPERR_PLAYERLOST         equ       MAKE_DPHRESULT( 300 )
DPERR_SESSIONLOST      equ         MAKE_DPHRESULT( 310 )
DPERR_UNINITIALIZED      equ       MAKE_DPHRESULT( 320 )
DPERR_NONEWPLAYERS       equ       MAKE_DPHRESULT( 330 )
DPERR_INVALIDPASSWORD    equ       MAKE_DPHRESULT( 340 )
DPERR_CONNECTING        equ        MAKE_DPHRESULT( 350 )
DPERR_CONNECTIONLOST   equ         MAKE_DPHRESULT( 360 )
DPERR_UNKNOWNMESSAGE  equ          MAKE_DPHRESULT( 370 )
DPERR_CANCELFAILED     equ         MAKE_DPHRESULT( 380 )
DPERR_INVALIDPRIORITY   equ        MAKE_DPHRESULT( 390 )
DPERR_NOTHANDLED         equ       MAKE_DPHRESULT( 400 )
DPERR_CANCELLED            equ     MAKE_DPHRESULT( 410 )
DPERR_ABORTED             equ      MAKE_DPHRESULT( 420 )

DPERR_BUFFERTOOLARGE        equ    MAKE_DPHRESULT( 1000 )
DPERR_CANTCREATEPROCESS     equ    MAKE_DPHRESULT( 1010 )
DPERR_APPNOTSTARTED        equ     MAKE_DPHRESULT( 1020 )
DPERR_INVALIDINTERFACE     equ     MAKE_DPHRESULT( 1030 )
DPERR_NOSERVICEPROVIDER     equ    MAKE_DPHRESULT( 1040 )
DPERR_UNKNOWNAPPLICATION    equ    MAKE_DPHRESULT( 1050 )
DPERR_NOTLOBBIED           equ     MAKE_DPHRESULT( 1070 )
DPERR_SERVICEPROVIDERLOADED	equ	MAKE_DPHRESULT( 1080 )
DPERR_ALREADYREGISTERED		equ	MAKE_DPHRESULT( 1090 )
DPERR_NOTREGISTERED			equ	MAKE_DPHRESULT( 1100 )

;Security related errors
DPERR_AUTHENTICATIONFAILED  equ    MAKE_DPHRESULT(  2000 )
DPERR_CANTLOADSSPI         equ     MAKE_DPHRESULT(  2010 )
DPERR_ENCRYPTIONFAILED   equ       MAKE_DPHRESULT(  2020 )
DPERR_SIGNFAILED            equ    MAKE_DPHRESULT(  2030 )
DPERR_CANTLOADSECURITYPACKAGE equ  MAKE_DPHRESULT(  2040 )
DPERR_ENCRYPTIONNOTSUPPORTED  equ  MAKE_DPHRESULT(  2050 )
DPERR_CANTLOADCAPI          equ    MAKE_DPHRESULT(  2060 )
DPERR_NOTLOGGEDIN         equ      MAKE_DPHRESULT(  2070 )
DPERR_LOGONDENIED        equ       MAKE_DPHRESULT(  2080 )


;/****************************************************************************
; *
; * 	dplay 1.0 obsolete structures + interfaces
; *	Included for compatibility only. New apps should
; *	use IDirectPlay2
; *
; ****************************************************************************/
;
;// define this to ignore obsolete interfaces and constants
;#ifndef IDIRECTPLAY2_OR_GREATER
;
;#define DPOPEN_OPENSESSION          DPOPEN_JOIN
;#define DPOPEN_CREATESESSION        DPOPEN_CREATE
;
;#define DPENUMSESSIONS_PREVIOUS     0x00000004
;
;#define DPENUMPLAYERS_PREVIOUS      0x00000004
;
;#define DPSEND_GUARANTEE            DPSEND_GUARANTEED
;#define DPSEND_TRYONCE              0x00000004
;
;#define DPCAPS_NAMESERVICE          0x00000001
;#define DPCAPS_NAMESERVER           DPCAPS_ISHOST
;#define DPCAPS_GUARANTEED           0x00000004
;
;#define DPLONGNAMELEN               52
;#define DPSHORTNAMELEN              20
;#define DPSESSIONNAMELEN            32
;#define DPPASSWORDLEN               16
;#define DPUSERRESERVED              16
;
;#define DPSYS_ADDPLAYER             0x0003
;#define DPSYS_DELETEPLAYER          0x0005
;
;#define DPSYS_DELETEGROUP           0x0020
;#define DPSYS_DELETEPLAYERFROMGRP   0x0021
;#define DPSYS_CONNECT               0x484b
;
;typedef struct
;{
;    DWORD       dwType;
;    DWORD       dwPlayerType;
;    DPID        dpId;
;    char        szLongName[DPLONGNAMELEN];
;    char        szShortName[DPSHORTNAMELEN];
;    DWORD       dwCurrentPlayers;
;}   DPMSG_ADDPLAYER;
;
;typedef DPMSG_ADDPLAYER DPMSG_ADDGROUP;
;
;typedef struct
;{
;    DWORD       dwType;
;    DPID        dpIdGroup;
;    DPID        dpIdPlayer;
;} DPMSG_GROUPADD;
;
;typedef DPMSG_GROUPADD DPMSG_GROUPDELETE;
;typedef struct
;{
;    DWORD       dwType;
;    DPID        dpId;
;} DPMSG_DELETEPLAYER;
;
;typedef BOOL (PASCAL *LPDPENUMPLAYERSCALLBACK)(
;    DPID    dpId,
;    LPSTR   lpFriendlyName,
;    LPSTR   lpFormalName,
;    DWORD   dwFlags,
;    LPVOID  lpContext );
;
;typedef struct
;{
;    DWORD   dwSize;
;    GUID    guidSession;                  
;    DWORD_PTR dwSession;                    
;    DWORD   dwMaxPlayers;                 
;    DWORD   dwCurrentPlayers;             
;    DWORD   dwFlags;                      
;    char    szSessionName[DPSESSIONNAMELEN];
;    char    szUserField[DPUSERRESERVED];
;    DWORD_PTR   dwReserved1;                  
;    char    szPassword[DPPASSWORDLEN];    
;    DWORD_PTR   dwReserved2;                  
;    DWORD_PTR   dwUser1;
;    DWORD_PTR   dwUser2;
;    DWORD_PTR   dwUser3;
;    DWORD_PTR   dwUser4;
;} DPSESSIONDESC,*LPDPSESSIONDESC;
;
;typedef BOOL (PASCAL * LPDPENUMSESSIONSCALLBACK)(
;    LPDPSESSIONDESC lpDPSessionDesc,
;    LPVOID      lpContext,
;    LPDWORD     lpdwTimeOut,
;    DWORD       dwFlags);
;
;/*
; * IDirectPlay
; */
;#undef INTERFACE
;#define INTERFACE IDirectPlay
;DECLARE_INTERFACE_( IDirectPlay, IUnknown )
;{
;    /*** IUnknown methods ***/
;    STDMETHOD QueryInterface)       (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
;    STDMETHOD_(ULONG,AddRef)        (THIS)  PURE;
;    STDMETHOD_(ULONG,Release)       (THIS) PURE;
;    /*** IDirectPlay methods ***/
;    STDMETHOD AddPlayerToGroup)     ,dword, DPID) PURE;
;    STDMETHOD Close)                (THIS) PURE;
;    STDMETHOD CreatePlayer)         (THIS_ LPDPID,LPSTR,LPSTR,LPHANDLE) PURE;
;    STDMETHOD CreateGroup)          (THIS_ LPDPID,LPSTR,LPSTR) PURE;
;    STDMETHOD DeletePlayerFromGroup),dword,DPID) PURE;
;    STDMETHOD DestroyPlayer)        (THIS_ DPID) PURE;
;    STDMETHOD DestroyGroup)         (THIS_ DPID) PURE;
;    STDMETHOD EnableNewPlayers)     (THIS_ BOOL) PURE;
;    STDMETHOD EnumGroupPlayers)     ,dword, LPDPENUMPLAYERSCALLBACK,LPVOID,DWORD) PURE;
;    STDMETHOD EnumGroups)           (THIS_ DWORD_PTR, LPDPENUMPLAYERSCALLBACK,LPVOID,DWORD) PURE;
;    STDMETHOD EnumPlayers)          (THIS_ DWORD_PTR, LPDPENUMPLAYERSCALLBACK,LPVOID,DWORD) PURE;
;    STDMETHOD EnumSessions)         (THIS_ LPDPSESSIONDESC,DWORD,LPDPENUMSESSIONSCALLBACK,LPVOID,DWORD) PURE;
;    STDMETHOD GetCaps)              (THIS_ LPDPCAPS) PURE;
;    STDMETHOD GetMessageCount)      ,dword, LPDWORD) PURE;
;    STDMETHOD GetPlayerCaps)        ,dword, LPDPCAPS) PURE;
;    STDMETHOD GetPlayerName)        ,dword,LPSTR,LPDWORD,LPSTR,LPDWORD) PURE;
;    STDMETHOD Initialize)           (THIS_ LPGUID) PURE;
;    STDMETHOD Open)                 (THIS_ LPDPSESSIONDESC) PURE;
;    STDMETHOD Receive)              (THIS_ LPDPID,LPDPID,DWORD,LPVOID,LPDWORD) PURE;
;    STDMETHOD SaveSession)          (THIS_ LPSTR) PURE;
;    STDMETHOD Send)                 ,dword, DPID, DWORD, LPVOID, DWORD) PURE;
;    STDMETHOD SetPlayerName)        ,dword,LPSTR,LPSTR) PURE;
;};
;
;/****************************************************************************
; *
; * IDirectPlay interface macros
; *
; ****************************************************************************/
;
;#if !defined(__cplusplus) || defined(CINTERFACE)
;
;#define IDirectPlay_AddPlayerToGroup(p,a,b)         (p)->lpVtbl->AddPlayerToGroup(p,a,b)
;#define IDirectPlay_Close(p)                        (p)->lpVtbl->Close(p)
;#define IDirectPlay_CreateGroup(p,a,b,c)            (p)->lpVtbl->CreateGroup(p,a,b,c)
;#define IDirectPlay_CreatePlayer(p,a,b,c,d)         (p)->lpVtbl->CreatePlayer(p,a,b,c,d)
;#define IDirectPlay_DeletePlayerFromGroup(p,a,b)    (p)->lpVtbl->DeletePlayerFromGroup(p,a,b)
;#define IDirectPlay_DestroyGroup(p,a)               (p)->lpVtbl->DestroyGroup(p,a)
;#define IDirectPlay_DestroyPlayer(p,a)              (p)->lpVtbl->DestroyPlayer(p,a)
;#define IDirectPlay_EnableNewPlayers(p,a)           (p)->lpVtbl->EnableNewPlayers(p,a)
;#define IDirectPlay_EnumGroupPlayers(p,a,b,c,d)     (p)->lpVtbl->EnumGroupPlayers(p,a,b,c,d)
;#define IDirectPlay_EnumGroups(p,a,b,c,d)           (p)->lpVtbl->EnumGroups(p,a,b,c,d)
;#define IDirectPlay_EnumPlayers(p,a,b,c,d)          (p)->lpVtbl->EnumPlayers(p,a,b,c,d)
;#define IDirectPlay_EnumSessions(p,a,b,c,d,e)       (p)->lpVtbl->EnumSessions(p,a,b,c,d,e)
;#define IDirectPlay_GetCaps(p,a)                    (p)->lpVtbl->GetCaps(p,a)
;#define IDirectPlay_GetMessageCount(p,a,b)          (p)->lpVtbl->GetMessageCount(p,a,b)
;#define IDirectPlay_GetPlayerCaps(p,a,b)            (p)->lpVtbl->GetPlayerCaps(p,a,b)
;#define IDirectPlay_GetPlayerName(p,a,b,c,d,e)      (p)->lpVtbl->GetPlayerName(p,a,b,c,d,e)
;#define IDirectPlay_Initialize(p,a)                 (p)->lpVtbl->Initialize(p,a)
;#define IDirectPlay_Open(p,a)                       (p)->lpVtbl->Open(p,a)
;#define IDirectPlay_Receive(p,a,b,c,d,e)            (p)->lpVtbl->Receive(p,a,b,c,d,e)
;#define IDirectPlay_SaveSession(p,a)                (p)->lpVtbl->SaveSession(p,a)
;#define IDirectPlay_Send(p,a,b,c,d,e)               (p)->lpVtbl->Send(p,a,b,c,d,e)
;#define IDirectPlay_SetPlayerName(p,a,b,c)          (p)->lpVtbl->SetPlayerName(p,a,b,c)
;
;#else /* C++ */
;
;#define IDirectPlay_AddPlayerToGroup(p,a,b)         (p)->AddPlayerToGroup(a,b)
;#define IDirectPlay_Close(p)                        (p)->Close()
;#define IDirectPlay_CreateGroup(p,a,b,c)            (p)->CreateGroup(a,b,c)
;#define IDirectPlay_CreatePlayer(p,a,b,c,d)         (p)->CreatePlayer(a,b,c,d)
;#define IDirectPlay_DeletePlayerFromGroup(p,a,b)    (p)->DeletePlayerFromGroup(a,b)
;#define IDirectPlay_DestroyGroup(p,a)               (p)->DestroyGroup(a)
;#define IDirectPlay_DestroyPlayer(p,a)              (p)->DestroyPlayer(a)
;#define IDirectPlay_EnableNewPlayers(p,a)           (p)->EnableNewPlayers(a)
;#define IDirectPlay_EnumGroupPlayers(p,a,b,c,d)     (p)->EnumGroupPlayers(a,b,c,d)
;#define IDirectPlay_EnumGroups(p,a,b,c,d)           (p)->EnumGroups(a,b,c,d)
;#define IDirectPlay_EnumPlayers(p,a,b,c,d)          (p)->EnumPlayers(a,b,c,d)
;#define IDirectPlay_EnumSessions(p,a,b,c,d,e)       (p)->EnumSessions(a,b,c,d,e)
;#define IDirectPlay_GetCaps(p,a)                    (p)->GetCaps(a)
;#define IDirectPlay_GetMessageCount(p,a,b)          (p)->GetMessageCount(a,b)
;#define IDirectPlay_GetPlayerCaps(p,a,b)            (p)->GetPlayerCaps(a,b)
;#define IDirectPlay_GetPlayerName(p,a,b,c,d,e)      (p)->GetPlayerName(a,b,c,d,e)
;#define IDirectPlay_Initialize(p,a)                 (p)->Initialize(a)
;#define IDirectPlay_Open(p,a)                       (p)->Open(a)
;#define IDirectPlay_Receive(p,a,b,c,d,e)            (p)->Receive(a,b,c,d,e)
;#define IDirectPlay_SaveSession(p,a)                (p)->SaveSession(a)
;#define IDirectPlay_Send(p,a,b,c,d,e)               (p)->Send(a,b,c,d,e)
;#define IDirectPlay_SetPlayerName(p,a,b,c)          (p)->SetPlayerName(a,b,c)
;
;#endif
;
;DEFINE_GUID(IID_IDirectPlay, 0x5454e9a0, 0xdb65, 0x11ce, 0x92, 0x1c, 0x00, 0xaa, 0x00, 0x6c, 0x49, 0x72);
;
;#endif // IDIRECTPLAY2_OR_GREATER




else
  echo (DPlay Base already included)
endif

