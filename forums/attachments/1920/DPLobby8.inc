;DirectPlayLobby8 include file (April 2006 DX-SDK)
;Translated to MASM format by EvilHomer
;17 July 2006

ifndef	__DPLOBBY_H__
__DPLOBBY_H__ equ 1

echo == DirectPlayLobby8 ==

;===========================================
; DirectPlay8Lobby CLSIDs
;===========================================
DefGUID CLSID_DirectPlay8LobbiedApplication,<667955AD-6B3B-43ca-B949-BC69B5BAFF7F>
DefGUID CLSID_DirectPlay8LobbyClient, <3B2B6775-70B6-45af-8DEA-A209C69559F3>

;===========================================
; DirectPlay8Lobby Interface IIDs
;===========================================
DefGUID IID_IDirectPlay8LobbiedApplication,<819074A3-016C-11d3-AE14-006097B01411>
DefGUID IID_IDirectPlay8LobbyClient,<819074A2-016C-11d3-AE14-006097B01411>

;===========================================
; DirectPlay8 Lobby Message IDs
;===========================================
DPL_MSGID_LOBBY		equ 8000h
DPL_MSGID_RECEIVE		equ			(1 or DPL_MSGID_LOBBY)
DPL_MSGID_CONNECT		equ			(2 or DPL_MSGID_LOBBY)
DPL_MSGID_DISCONNECT	equ			(3 or DPL_MSGID_LOBBY)
DPL_MSGID_SESSION_STATUS	equ		(4 or DPL_MSGID_LOBBY)
DPL_MSGID_CONNECTION_SETTINGS  equ  (5 or DPL_MSGID_LOBBY)

;===========================================
; DirectPlay8Lobby Constants
;===========================================
;Specifies that operation should be performed on all open connections
DPLHANDLE_ALLCONNECTIONS equ -1

;The associated game session has suceeded in connecting / hosting
DPLSESSION_CONNECTED equ 1

;The associated game session failed connecting / hosting
DPLSESSION_COULDNOTCONNECT equ 2

;The associated game session has disconnected
DPLSESSION_DISCONNECTED	equ 3

;The associated game session has terminated 
DPLSESSION_TERMINATED	equ 4

;The associated game session's host has migrated 
DPLSESSION_HOSTMIGRATED	equ 5

;The associated game session's host has migrated to the local client
DPLSESSION_HOSTMIGRATEDHERE	equ 6

;===========================================
; DirectPlay8 Lobby Flags
;===========================================
;Do not automatically make the lobby app unavailable when a connection is established
DPLAVAILABLE_ALLOWMULTIPLECONNECT equ 1

;Launch a new instance of the application to connect to
DPLCONNECT_LAUNCHNEW		 equ 1

;Launch a new instance of the application if one is not waiting
DPLCONNECT_LAUNCHNOTFOUND	 equ 2

;When starting the associated game session, start it as a host
DPLCONNECTSETTINGS_HOST      equ 1

;Disable parameter validation
DPLINITIALIZE_DISABLEPARAMVAL equ 1

;===========================================
; DirectPlay8Lobby Structures (Non-Message)
;===========================================
;Information on a registered game
DPL_APPLICATION_INFO struct
	guidApplication GUID <>;            // GUID of the application
	pwszApplicationName dd ?;        // Name of the application
	dwNumRunning dd ?;               // # of instances of this application running
	dwNumWaiting dd ?;               // # of instances of this application waiting 
	dwFlags dd ?;                    // Flags
DPL_APPLICATION_INFO ends
PDPL_APPLICATION_INFO typedef ptr DPL_APPLICATION_INFO

;Settings to be used for connecting / hosting a game session
DPL_CONNECTION_SETTINGS struct
    dwSize dd ?;                 // Size of this structure
    dwFlags dd ?;                // Connection settings flags (DPLCONNECTSETTINGS_...)
    dpnAppDesc DPN_APPLICATION_DESC <>;             // Application desc for the associated DirectPlay session
    pdp8HostAddress dd ?;       // Address of host to connect to
    ppdp8DeviceAddresses dd ?; // Address of device to connect from / host on
    cNumDeviceAddresses dd ?;    // # of addresses specified in ppdp8DeviceAddresses
	pwszPlayerName dd ?;         // Name to give the player
DPL_CONNECTION_SETTINGS ends
PDPL_CONNECTION_SETTINGS typedef ptr DPL_CONNECTION_SETTINGS

;Information for performing a lobby connect
;(ConnectApplication)
DPL_CONNECT_INFO struct
	dwSize dd ?;             // Size of this structure
	dwFlags dd ?;            // Flags (DPLCONNECT_...)
	guidApplication GUID <>;    // GUID of application to launch
    pdplConnectionSettings dd ?;
    ;Settings application should use
	pvLobbyConnectData dd ?; // User defined data block
	dwLobbyConnectDataSize dd ?;Size of user defined data block
DPL_CONNECT_INFO ends
PDPL_CONNECT_INFO typedef ptr DPL_CONNECT_INFO

;Information for registering an application
;(RegisterApplication)
DPL_PROGRAM_DESC struct
	dwSize dd ?;
	dwFlags dd ?;
	guidApplication GUID <>;		                // Application GUID
	pwszApplicationName dd ?;	                // Unicode application name
	pwszCommandLine dd ?;		                // Unicode command line arguments
	pwszCurrentDirectory dd ?;               	// Unicode current directory
	pwszDescription dd ?;		                // Unicode application description
	pwszExecutableFilename dd ?;	                // Unicode filename of application executable
	pwszExecutablePath dd ?;		                // Unicode path of application executable
	pwszLauncherFilename dd ?;	                // Unicode filename of launcher executable
	pwszLauncherPath dd ?;		                // Unicode path of launcher executable
DPL_PROGRAM_DESC ends
PDPL_PROGRAM_DESC typedef ptr DPL_PROGRAM_DESC

;===========================================
; DirectPlay8 Lobby Message Structures
;===========================================
;A connection was established 
DPL_MESSAGE_CONNECT struct
	dwSize dd ?;                     // Size of this structure
	hConnectId dd ?;                 // Handle of new connection
    pdplConnectionSettings dd ?;	    // Connection settings for this connection
	pvLobbyConnectData dd ?;         // User defined lobby data block
	dwLobbyConnectDataSize dd ?;     // Size of user defined lobby data block
	pvConnectionContext dd ?;        // Context value for this connection (user set)
DPL_MESSAGE_CONNECT ends
PDPL_MESSAGE_CONNECT typedef ptr DPL_MESSAGE_CONNECT

;Connection settings have been updated
DPL_MESSAGE_CONNECTION_SETTINGS struct
    dwSize dd ?;                     // Size of this structure
    hSender dd ?;                    // Handle of the connection for these settings
    pdplConnectionSettings dd ?;     // Connection settings
	pvConnectionContext dd ?;        // Context value for this connection
DPL_MESSAGE_CONNECTION_SETTINGS ends
PDPL_MESSAGE_CONNECTION_SETTINGS typedef ptr DPL_MESSAGE_CONNECTION_SETTINGS

;A connection has been disconnected
DPL_MESSAGE_DISCONNECT struct
	dwSize dd ?;                                     // Size of this structure
	hDisconnectId dd ?;                              // Handle of the connection that was terminated
	hrReason dd ?;                                   // Reason the connection was broken
	pvConnectionContext dd ?;                        // Context value for this connection
DPL_MESSAGE_DISCONNECT ends
PDPL_MESSAGE_DISCONNECT typedef ptr DPL_MESSAGE_DISCONNECT

;Data was received through a connection
DPL_MESSAGE_RECEIVE struct
	dwSize dd ?;                                     // Size of this structure
	hSender dd ?;                                    // Handle of the connection that is from
	pBuffer dd ?;                                   // Contents of the message
	dwBufferSize dd ?;                               // Size of the message context 
	pvConnectionContext dd ?;                        // Context value for this connection
DPL_MESSAGE_RECEIVE ends
PDPL_MESSAGE_RECEIVE typedef ptr DPL_MESSAGE_RECEIVE

;Current status of the associated connection
DPL_MESSAGE_SESSION_STATUS struct
	dwSize dd ?;                                     // Size of this structure
	hSender dd ?;                                    // Handle of the connection that this is from
	dwStatus dd ?;                                   // Status (DPLSESSION_...)
	pvConnectionContext dd ?;                        // Context value for this connection
DPL_MESSAGE_SESSION_STATUS ends
PDPL_MESSAGE_SESSION_STATUS typedef ptr DPL_MESSAGE_SESSION_STATUS

 ; This function is no longer supported.
 ;  It is recommended that CoCreateInstance be used to create DirectPlay8 lobby objects. 
; * extern HRESULT WINAPI DirectPlay8LobbyCreate( const GUID * pcIID, void **ppvInterface, IUnknown *pUnknown);

;DirectPlay8 Functions
BEGIN_INTERFACE IDirectPlay8LobbyClient,IUnknown,<819074A2-016C-11d3-AE14-006097B01411>
	STDMETHOD Initialize,Pointer,Pointer,DWORD
	STDMETHOD EnumLocalPrograms,Pointer,Pointer,Pointer,Pointer,DWORD
	STDMETHOD ConnectApplication,Pointer,Pointer,Pointer,DWORD,DWORD
	STDMETHOD Send,HANDLE,Pointer,Pointer,DWORD
	STDMETHOD ReleaseApplication,HANDLE,DWORD
	STDMETHOD Close,DWORD
	STDMETHOD GetConnectionSettings,HANDLE,Pointer,Pointer,DWORD	
	STDMETHOD SetConnectionSettings,HANDLE,Pointer,DWORD
END_INTERFACE

BEGIN_INTERFACE IDirectPlay8LobbiedApplication,IUnknown,<819074A3-016C-11d3-AE14-006097B01411>
	STDMETHOD Initialize,Pointer,Pointer,Pointer,DWORD
	STDMETHOD RegisterProgram,Pointer,DWORD
	STDMETHOD UnRegisterProgram,Pointer,DWORD
	STDMETHOD Send,HANDLE,Pointer,Pointer,DWORD
	STDMETHOD SetAppAvailable,BOOL,DWORD
	STDMETHOD UpdateStatus,HANDLE,DWORD,DWORD 
	STDMETHOD Close,DWORD
	STDMETHOD GetConnectionSettings,HANDLE, Pointer, Pointer,DWORD
	STDMETHOD SetConnectionSettings,HANDLE,Pointer,DWORD
END_INTERFACE

endif

