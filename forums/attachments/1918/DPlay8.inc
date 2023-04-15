;DirectPlay8 include file (April 2006 DX-SDK)
;Translated to MASM format by EvilHomer
;17 July 2006

ifndef __DIRECTPLAY8_H__
__DIRECTPLAY8_H__ equ 1

echo == DirectPlay8 ==

;include "dpaddr.h"

;DirectPlay8 CLSIDs
DefGUID CLSID_DirectPlay8Client,<743F1DC6-5ABA-429f-8BDF-C54D03253DC2>
DefGUID CLSID_DirectPlay8Server,<DA825E1B-6830-43d7-835D-0B5AD82956A2>
DefGUID CLSID_DirectPlay8Peer,  <286F484D-375E-4458-A272-B138E2F80A6A>

; CLSIDs added for DirectX 9
DefGUID CLSID_DirectPlay8ThreadPool,  <FC47060E-6153-4b34-B975-8E4121EB7F3C>
DefGUID CLSID_DirectPlay8NATResolver, <E4C1D9A2-CBF7-48bd-9A69-34A55E0D8941>

;DirectPlay8 Interface IIDs
DefGUID IID_IDirectPlay8Client,<5102DACD-241B-11d3-AEA7-006097B01411>
DefGUID IID_IDirectPlay8Server,<5102DACE-241B-11d3-AEA7-006097B01411>
DefGUID IID_IDirectPlay8Peer,  <5102DACF-241B-11d3-AEA7-006097B01411>

; IIDs added for DirectX 9
DefGUID IID_IDirectPlay8ThreadPool,<0D22EE73-4A46-4a0d-89B2-045B4D666425>
DefGUID IID_IDirectPlay8NATResolver, <A9E213F2-9A60-486f-BF3B-53408B6D1CBB>

;DirectPlay8 Service Provider GUIDs
DefGUID CLSID_DP8SP_IPX,   <53934290-628D-11D2-AE0F-006097B01411>
DefGUID CLSID_DP8SP_MODEM, <6D4A3650-628D-11D2-AE0F-006097B01411>
DefGUID CLSID_DP8SP_SERIAL,<743B5D60-628D-11D2-AE0F-006097B01411>
DefGUID CLSID_DP8SP_TCPIP, <EBFE7BA0-628D-11D2-AE0F-006097B01411>

;Service providers added for DirectX 9
DefGUID CLSID_DP8SP_BLUETOOTH, <995513AF-3027-4b9a-956E-C772B3F78006>

;DirectPlay8 Callback Functions
DirectPlay8CallBack proto :Pointer,:dword,:Pointer

;DirectPlay8 Datatypes (Non-Structure / Non-Message)
;Player IDs.  Used to uniquely identify a player in a session
DPNID typedef dword
;Used as identifiers for operations
DPNHANDLE typedef dword

;DirectPlay8 Message Identifiers
DPN_MSGID_OFFSET equ 0FFFF0000h
DPN_MSGID_ADD_PLAYER_TO_GROUP equ	( DPN_MSGID_OFFSET or 1 )
DPN_MSGID_APPLICATION_DESC		equ	( DPN_MSGID_OFFSET  or  2 )
DPN_MSGID_ASYNC_OP_COMPLETE		equ	( DPN_MSGID_OFFSET  or 3 )
DPN_MSGID_CLIENT_INFO		equ		( DPN_MSGID_OFFSET or 4 )
DPN_MSGID_CONNECT_COMPLETE	equ		( DPN_MSGID_OFFSET or 5 )
DPN_MSGID_CREATE_GROUP		equ		( DPN_MSGID_OFFSET or 6 )
DPN_MSGID_CREATE_PLAYER		equ		( DPN_MSGID_OFFSET or 7 )
DPN_MSGID_DESTROY_GROUP		equ		( DPN_MSGID_OFFSET or 8 )
DPN_MSGID_DESTROY_PLAYER	equ		( DPN_MSGID_OFFSET or 9 )
DPN_MSGID_ENUM_HOSTS_QUERY		equ	( DPN_MSGID_OFFSET or 10 )
DPN_MSGID_ENUM_HOSTS_RESPONSE	equ	( DPN_MSGID_OFFSET or 11 )
DPN_MSGID_GROUP_INFO		equ		( DPN_MSGID_OFFSET or 12 )
DPN_MSGID_HOST_MIGRATE		equ		( DPN_MSGID_OFFSET or 13 )
DPN_MSGID_INDICATE_CONNECT	equ		( DPN_MSGID_OFFSET or 14 )
DPN_MSGID_INDICATED_CONNECT_ABORTED	equ ( DPN_MSGID_OFFSET or 15 )
DPN_MSGID_PEER_INFO			equ		( DPN_MSGID_OFFSET or 16 )
DPN_MSGID_RECEIVE			equ		( DPN_MSGID_OFFSET or 17 )
DPN_MSGID_REMOVE_PLAYER_FROM_GROUP	equ ( DPN_MSGID_OFFSET or 18 )
DPN_MSGID_RETURN_BUFFER		equ		( DPN_MSGID_OFFSET or 19 )
DPN_MSGID_SEND_COMPLETE		equ		( DPN_MSGID_OFFSET or 20 )
DPN_MSGID_SERVER_INFO		equ		( DPN_MSGID_OFFSET or 21 )
DPN_MSGID_TERMINATE_SESSION		equ	( DPN_MSGID_OFFSET or 22 )

;Messages added for DirectX 9
DPN_MSGID_CREATE_THREAD		equ		( DPN_MSGID_OFFSET or 23 )
DPN_MSGID_DESTROY_THREAD		equ	( DPN_MSGID_OFFSET or 24 )
DPN_MSGID_NAT_RESOLVER_QUERY equ	( DPN_MSGID_OFFSET or 0101h )

;DirectPlay8 Constants
DPNID_ALL_PLAYERS_GROUP	equ 0

;DESTROY_GROUP reasons
DPNDESTROYGROUPREASON_NORMAL				equ 1
DPNDESTROYGROUPREASON_AUTODESTRUCTED		equ 2
DPNDESTROYGROUPREASON_SESSIONTERMINATED		equ 3

;DESTROY_PLAYER reasons
DPNDESTROYPLAYERREASON_NORMAL				equ 1
DPNDESTROYPLAYERREASON_CONNECTIONLOST		equ 2
DPNDESTROYPLAYERREASON_SESSIONTERMINATED	equ 3
DPNDESTROYPLAYERREASON_HOSTDESTROYEDPLAYER	equ 4

DPN_MAX_APPDESC_RESERVEDDATA_SIZE			equ 64

;DirectPlay8 Flags
;Asynchronous operation flags (for Async Ops)
DPNOP_SYNC	equ 80000000h

;Add player to group flags (for AddPlayerToGroup)
DPNADDPLAYERTOGROUP_SYNC equ DPNOP_SYNC

;Cancel flags
DPNCANCEL_CONNECT	equ 1
DPNCANCEL_ENUM		equ 2
DPNCANCEL_SEND		equ 4
DPNCANCEL_ALL_OPERATIONS equ 8000h
;Flags added for DirectX 9
DPNCANCEL_PLAYER_SENDS	equ 80000000h
DPNCANCEL_PLAYER_SENDS_PRIORITY_HIGH	equ (DPNCANCEL_PLAYER_SENDS or 10000h)
DPNCANCEL_PLAYER_SENDS_PRIORITY_NORMAL	equ (DPNCANCEL_PLAYER_SENDS or 20000h)
DPNCANCEL_PLAYER_SENDS_PRIORITY_LOW		equ (DPNCANCEL_PLAYER_SENDS or 40000h)

;Close flags (for Close, added for DirectX 9)
DPNCLOSE_IMMEDIATE equ 1

;Connect flags (for Connect)
DPNCONNECT_SYNC	equ 	DPNOP_SYNC
DPNCONNECT_OKTOQUERYFORADDRESSING equ 1

;Create group flags (for CreateGroup)
DPNCREATEGROUP_SYNC	equ	DPNOP_SYNC

;Destroy group flags (for DestroyGroup)
DPNDESTROYGROUP_SYNC equ	DPNOP_SYNC

;Enumerate clients and groups flags (for EnumPlayersAndGroups)
DPNENUM_PLAYERS	equ 1
DPNENUM_GROUPS	equ 16

;Enum hosts flags (for EnumHosts)
DPNENUMHOSTS_SYNC				equ DPNOP_SYNC
DPNENUMHOSTS_OKTOQUERYFORADDRESSING	equ 1
DPNENUMHOSTS_NOBROADCASTFALLBACK	equ 2

;Enum service provider flags (for EnumSP)
DPNENUMSERVICEPROVIDERS_ALL	equ 1

;GetLocalHostAddresses flags (added for DirectX 9)
DPNGETLOCALHOSTADDRESSES_COMBINED equ 1 

;Get send queue info flags (for GetSendQueueInfo)
DPNGETSENDQUEUEINFO_PRIORITY_NORMAL	equ 1
DPNGETSENDQUEUEINFO_PRIORITY_HIGH	equ 2
DPNGETSENDQUEUEINFO_PRIORITY_LOW	equ 4

;Group information flags (for Group Info)
DPNGROUP_AUTODESTRUCT	equ 1

;Host flags (for Host)
DPNHOST_OKTOQUERYFORADDRESSING	equ 1

;Set info
DPNINFO_NAME	equ 1
DPNINFO_DATA	equ 2

;Initialize flags (for Initialize)
DPNINITIALIZE_DISABLEPARAMVAL 	 equ 1
;Flags added for DirectX 9
DPNINITIALIZE_HINT_LANSESSION	 equ 2
DPNINITIALIZE_DISABLELINKTUNING	 equ 4

;Register Lobby flags
DPNLOBBY_REGISTER		equ 1
DPNLOBBY_UNREGISTER		equ 2

;Player information flags (for Player Info / Player Messages)
DPNPLAYER_LOCAL			equ 2
DPNPLAYER_HOST			equ 4

;Receive indication flags (added for DirectX 9)
DPNRECEIVE_GUARANTEED	equ 1
DPNRECEIVE_COALESCED	equ 2

;Remove player from group flags (for RemovePlayerFromGroup)
DPNREMOVEPLAYERFROMGROUP_SYNC	equ		DPNOP_SYNC

;Send flags (for Send/SendTo)
DPNSEND_SYNC					equ DPNOP_SYNC
DPNSEND_NOCOPY					equ 1
DPNSEND_NOCOMPLETE				equ 2
DPNSEND_COMPLETEONPROCESS		equ 4
DPNSEND_GUARANTEED				equ 8
DPNSEND_NONSEQUENTIAL			equ 16
DPNSEND_NOLOOPBACK				equ 32
DPNSEND_PRIORITY_LOW			equ 64
DPNSEND_PRIORITY_HIGH			equ 128
;Flag added for DirectX 9
DPNSEND_COALESCE				equ 256

;Send complete indication flags (added for DirectX 9)
DPNSENDCOMPLETE_GUARANTEED		equ 1
DPNSENDCOMPLETE_COALESCED		equ 2

;Session Flags (for DPN_APPLICATION_DESC)
DPNSESSION_CLIENT_SERVER		equ 1
DPNSESSION_MIGRATE_HOST			equ 4
DPNSESSION_NODPNSVR				equ 64
DPNSESSION_REQUIREPASSWORD		equ 128
;Flag added for DirectX 9
DPNSESSION_NOENUMS				equ 256
DPNSESSION_FAST_SIGNED			equ 512
DPNSESSION_FULL_SIGNED			equ 1024

;Set client info flags (for SetClientInfo)
DPNSETCLIENTINFO_SYNC			equ DPNOP_SYNC

;Set group info flags (for SetGroupInfo)
DPNSETGROUPINFO_SYNC			equ DPNOP_SYNC

;Set peer info flags (for SetPeerInfo)
DPNSETPEERINFO_SYNC				equ	DPNOP_SYNC

;Set server info flags (for SetServerInfo)
DPNSETSERVERINFO_SYNC			equ DPNOP_SYNC

;SP capabilities flags
DPNSPCAPS_SUPPORTSDPNSVR		equ 1
DPNSPCAPS_SUPPORTSDPNSRV		equ DPNSPCAPS_SUPPORTSDPNSVR
DPNSPCAPS_SUPPORTSBROADCAST		equ 2
DPNSPCAPS_SUPPORTSALLADAPTERS	equ 4
;Flags added for DirectX 9
DPNSPCAPS_SUPPORTSTHREADPOOL	equ 8
DPNSPCAPS_NETWORKSIMULATOR		equ 16

;SP information flags (added for DirectX 9)
DPNSPINFO_NETWORKSIMULATORDEVICE	equ 1

;DirectPlay8 Structures (Non-Message)
DPN_APPLICATION_DESC struct
	dwSize dd ?;						Size of this structure
	dwFlags dd ?;						Flags (DPNSESSION_...)
	guidInstance GUID <>;				Instance GUID
	guidApplication GUID <>;			Application GUID
	dwMaxPlayers dd ?;					Maximum # of players allowed (0=no limit)
	dwCurrentPlayers dd ?;				Current # of players allowed
	pwszSessionName dd ?;				Name of the session
	pwszPassword dd ?;					Password for the session
	pvReservedData dd ?					
	dwReservedDataSize dd ?
	pvApplicationReservedData dd ?
	dwApplicationReservedDataSize dd ?
DPN_APPLICATION_DESC ends
PDPN_APPLICATION_DESC typedef ptr DPN_APPLICATION_DESC


BUFFERDESC struct
	DWORD	dwBufferSize;		
	BYTE * 	pBufferData;		
BUFFERDESC ends
PDPN_BUFFER_DESC typedef ptr BUFFERDESC
DPN_BUFFER_DESC typedef BUFFERDESC

;DirectPlay8 capabilities
DPN_CAPS struct
	dwSize dd ?;					Size of this structure
	dwFlags dd ?;					Flags
	dwConnectTimeout dd ?;			ms before a connect request times out
	dwConnectRetries dd ?;			# of times to attempt the connection
	dwTimeoutUntilKeepAlive dd ?;	ms of inactivity before a keep alive is sent
DPN_CAPS ends
PDPN_CAPS typedef ptr DPN_CAPS

;Extended capabilities structures (added for DirectX 9)
DPN_CAPS_EX struct
	dwSize dd ?;					Size of this structure
	dwFlags dd ?;					Flags
	dwConnectTimeout dd ?;			ms before a connect request times out
	dwConnectRetries dd ?;			# of times to attempt the connection
	dwTimeoutUntilKeepAlive dd ?;	ms of inactivity before a keep alive is sent
	dwMaxRecvMsgSize dd ?;			maximum size in bytes of message that can be received
	dwNumSendRetries dd ?;			maximum number of send retries before link is considered dead
	dwMaxSendRetryInterval dd ?;	maximum period in msec between send retries
	dwDropThresholdRate dd ?;		percentage of dropped packets before throttling
	dwThrottleRate dd ?;			percentage amount to reduce send window when throttling
	dwNumHardDisconnectSends dd ?;	number of hard disconnect frames to send when close immediate flag is specified
	dwMaxHardDisconnectPeriod dd ?;	maximum period between hard disconnect sends
DPN_CAPS_EX ends
PDPN_CAPS_EX typedef ptr DPN_CAPS_EX

;Connection Statistics information
DPN_CONNECTION_INFO struct
	dwSize dd ?;
	dwRoundTripLatencyMS dd ?;
	dwThroughputBPS dd ?;
	dwPeakThroughputBPS dd ?;

	dwBytesSentGuaranteed dd ?;
	dwPacketsSentGuaranteed dd ?;
	dwBytesSentNonGuaranteed dd ?;
	dwPacketsSentNonGuaranteed dd ?;

	dwBytesRetried dd ?;		 Guaranteed only
	dwPacketsRetried dd ?;	     Guaranteed only
	dwBytesDropped dd ?;		 Non Guaranteed only
	dwPacketsDropped dd ?;	     Non Guaranteed only

	dwMessagesTransmittedHighPriority dd ?;
	dwMessagesTimedOutHighPriority dd ?;
	dwMessagesTransmittedNormalPriority dd ?;
	dwMessagesTimedOutNormalPriority dd ?;
	dwMessagesTransmittedLowPriority dd ?;
	dwMessagesTimedOutLowPriority dd ?;

	dwBytesReceivedGuaranteed dd ?;
	dwPacketsReceivedGuaranteed dd ?;
	dwBytesReceivedNonGuaranteed dd ?;
	dwPacketsReceivedNonGuaranteed dd ?;
	dwMessagesReceived dd ?;

DPN_CONNECTION_INFO ends
PDPN_CONNECTION_INFO typedef ptr DPN_CONNECTION_INFO

;Group information structure
DPN_GROUP_INFO struct
	dwSize dd ?;				// size of this structure
	dwInfoFlags dd ?;		// information contained
	pwszName dd ?;			// Unicode Name
	pvData dd ?;				// data block
	dwDataSize dd ?;			// size in BYTES of data block
	dwGroupFlags dd ?;		// group flags (DPNGROUP_...)
DPN_GROUP_INFO ends
PDPN_GROUP_INFO typedef ptr DPN_GROUP_INFO

;Player information structure
DPN_PLAYER_INFO struct
	dwSize dd ?;				// size of this structure
	dwInfoFlags dd ?;		// information contained
	pwszName dd ?;			// Unicode Name
	pvData dd ?;				// data block
	dwDataSize dd ?;			// size in BYTES of data block
	dwPlayerFlags dd ?;		// player flags (DPNPLAYER_...)
DPN_PLAYER_INFO ends
PDPN_PLAYER_INFO typedef ptr DPN_PLAYER_INFO

;Service provider & adapter enumeration structure
DPN_SERVICE_PROVIDER_INFO struct
	dwFlags dd ?;
	guid GUID <>;		// SP Guid
	pwszName dd ?;	// Friendly Name
	pvReserved dd ?;	
	dwReserved dd ?;
DPN_SERVICE_PROVIDER_INFO ends
PDPN_SERVICE_PROVIDER_INFO typedef ptr DPN_SERVICE_PROVIDER_INFO

;Service provider caps structure
DPN_SP_CAPS struct
	dwSize dd ?;							// Size of this structure
	dwFlags dd ?;						// Flags (DPNSPCAPS_...)
	dwNumThreads dd ?;					// # of worker threads to use
	dwDefaultEnumCount dd ?;				// default # of enum requests
	dwDefaultEnumRetryInterval dd ?;		// default ms between enum requests
	dwDefaultEnumTimeout dd ?;			// default enum timeout
	dwMaxEnumPayloadSize dd ?;			// maximum size in bytes for enum payload data
	dwBuffersPerThread dd ?;				// number of receive buffers per thread
	dwSystemBufferSize dd ?;				// amount of buffering to do in addition to posted receive buffers
DPN_SP_CAPS ends
PDPN_SP_CAPS typedef ptr DPN_SP_CAPS

;IDirectPlay8 message handler call back structures
;Add player to group structure for message handler
;(DPN_MSGID_ADD_PLAYER_TO_GROUP)
DPNMSG_ADD_PLAYER_TO_GROUP struct
	dwSize dd ?;				// Size of this structure
	dpnidGroup dd ?;			// DPNID of group
	pvGroupContext dd ?;		// Group context value
	dpnidPlayer dd ?;		// DPNID of added player
	pvPlayerContext dd ?;	// Player context value
DPNMSG_ADD_PLAYER_TO_GROUP ends
PDPNMSG_ADD_PLAYER_TO_GROUP typedef ptr DPNMSG_ADD_PLAYER_TO_GROUP

;Async operation completion structure for message handler
;(DPN_MSGID_ASYNC_OP_COMPLETE)
DPNMSG_ASYNC_OP_COMPLETE struct
	dwSize dd ?;			// Size of this structure
	hAsyncOp dd ?;		// DirectPlay8 async operation handle
	pvUserContext dd ?;	// User context supplied
	hResultCode dd ?;	// HRESULT of operation
DPNMSG_ASYNC_OP_COMPLETE ends
PDPNMSG_ASYNC_OP_COMPLETE typedef ptr DPNMSG_ASYNC_OP_COMPLETE

;Client info structure for message handler
;DPN_MSGID_CLIENT_INFO)
DPNMSG_CLIENT_INFO struct
	dwSize dd ?;				// Size of this structure
	dpnidClient dd ?;		// DPNID of client
	pvPlayerContext dd ?;	// Player context value
DPNMSG_CLIENT_INFO ends
PDPNMSG_CLIENT_INFO typedef ptr DPNMSG_CLIENT_INFO

;Connect complete structure for message handler
;(DPN_MSGID_CONNECT_COMPLETE)
DPNMSG_CONNECT_COMPLETE struct
	dwSize dd ?;						// Size of this structure
	hAsyncOp dd ?;					// DirectPlay8 Async operation handle
	pvUserContext dd ?;				// User context supplied at Connect
	hResultCode dd ?;				// HRESULT of connection attempt
	pvApplicationReplyData dd ?;		// Connection reply data from Host/Server
	dwApplicationReplyDataSize dd ?;	// Size (in bytes) of pvApplicationReplyData
	;Fields added for DirectX 9
	dpnidLocal dd ?;					// DPNID of local player
DPNMSG_CONNECT_COMPLETE ends
PDPNMSG_CONNECT_COMPLETE typedef ptr DPNMSG_CONNECT_COMPLETE

;Create group structure for message handler
;(DPN_MSGID_CREATE_GROUP)
DPNMSG_CREATE_GROUP struct
	dwSize dd ?;				// Size of this structure
	dpnidGroup dd ?;			// DPNID of new group
	dpnidOwner dd ?;			// Owner of newgroup
	pvGroupContext dd ?;		// Group context value
	;Fields added for DirectX 9
	pvOwnerContext dd ?;		// Owner context value
DPNMSG_CREATE_GROUP ends
PDPNMSG_CREATE_GROUP typedef ptr DPNMSG_CREATE_GROUP

;Create player structure for message handler
;(DPN_MSGID_CREATE_PLAYER)
DPNMSG_CREATE_PLAYER struct
	dwSize dd ?;				// Size of this structure
	dpnidPlayer dd ?;		// DPNID of new player
	pvPlayerContext dd ?;	// Player context value
DPNMSG_CREATE_PLAYER ends
PDPNMSG_CREATE_PLAYER typedef ptr DPNMSG_CREATE_PLAYER

;Destroy group structure for message handler
;(DPN_MSGID_DESTROY_GROUP)
DPNMSG_DESTROY_GROUP struct
	dwSize dd ?;				// Size of this structure
	dpnidGroup dd ?;			// DPNID of destroyed group
	pvGroupContext dd ?;		// Group context value
	dwReason dd ?;			// Information only
DPNMSG_DESTROY_GROUP ends
PDPNMSG_DESTROY_GROUP typedef ptr DPNMSG_DESTROY_GROUP

;Destroy player structure for message handler
;DPN_MSGID_DESTROY_PLAYER)
DPNMSG_DESTROY_PLAYER struct
	dwSize dd ?;				// Size of this structure
	dpnidPlayer dd ?;		// DPNID of leaving player
	pvPlayerContext dd ?;	// Player context value
	dwReason dd ?;			// Information only
DPNMSG_DESTROY_PLAYER ends
PDPNMSG_DESTROY_PLAYER typedef ptr DPNMSG_DESTROY_PLAYER

;Enumeration request received structure for message handler
;(DPN_MSGID_ENUM_HOSTS_QUERY)
DPNMSG_ENUM_HOSTS_QUERY struct
	dwSize dd ?;				 // Size of this structure.
	pAddressSender dd ?;		// Address of client who sent the request
	pAddressDevice dd ?;		// Address of device request was received on
	pvReceivedData dd ?;		 // Request data (set on client)
	dwReceivedDataSize dd ?;	 // Request data size (set on client)
	dwMaxResponseDataSize dd ?;	// Max allowable size of enum response
	pvResponseData dd ?;			// Optional query repsonse (user set)
	dwResponseDataSize dd ?;		// Optional query response size (user set)
	pvResponseContext dd ?;		// Optional query response context (user set)
DPNMSG_ENUM_HOSTS_QUERY ends
PDPNMSG_ENUM_HOSTS_QUERY typedef ptr DPNMSG_ENUM_HOSTS_QUERY

;Enumeration response received structure for message handler
;(DPN_MSGID_ENUM_HOSTS_RESPONSE)
DPNMSG_ENUM_HOSTS_RESPONSE struct
	dwSize dd ?;					 // Size of this structure
	pAddressSender dd ?;			// Address of host who responded
	pAddressDevice dd ?;			// Device response was received on
	pApplicationDescription dd ?;	// Application description for the session
	pvResponseData dd ?;			 // Optional response data (set on host)
	dwResponseDataSize dd ?;		 // Optional response data size (set on host)
	pvUserContext dd ?;				// Context value supplied for enumeration
	dwRoundTripLatencyMS dd ?;		// Round trip latency in MS
DPNMSG_ENUM_HOSTS_RESPONSE ends
PDPNMSG_ENUM_HOSTS_RESPONSE typedef ptr DPNMSG_ENUM_HOSTS_RESPONSE

;Group info structure for message handler
;(DPN_MSGID_GROUP_INFO)
DPNMSG_GROUP_INFO struct
	dwSize dd ?;					// Size of this structure
	dpnidGroup dd ?;				// DPNID of group
	pvGroupContext dd ?;			// Group context value
DPNMSG_GROUP_INFO ends
PDPNMSG_GROUP_INFO typedef ptr DPNMSG_GROUP_INFO

;Migrate host structure for message handler
;(DPN_MSGID_HOST_MIGRATE)
DPNMSG_HOST_MIGRATE struct
	dwSize dd ?					// Size of this structure
	dpnidNewHost dd ?;			// DPNID of new Host player
	pvPlayerContext dd ?;		// Player context value
DPNMSG_HOST_MIGRATE ends
PDPNMSG_HOST_MIGRATE typedef ptr DPNMSG_HOST_MIGRATE

;Indicate connect structure for message handler
;(DPN_MSGID_INDICATE_CONNECT)
DPNMSG_INDICATE_CONNECT struct
	dwSize dd ?;					// Size of this structure
	pvUserConnectData dd ?;		// Connecting player data
	dwUserConnectDataSize dd ?;	// Size (in bytes) of pvUserConnectData
	pvReplyData dd ?;			// Connection reply data
	dwReplyDataSize dd ?;		// Size (in bytes) of pvReplyData
	pvReplyContext dd ?;			// Buffer context for pvReplyData
	pvPlayerContext dd ?;		// Player context preset
	pAddressPlayer dd ?;		// Address of connecting player
	pAddressDevice dd ?;		// Address of device receiving connect attempt
DPNMSG_INDICATE_CONNECT ends
PDPNMSG_INDICATE_CONNECT typedef ptr DPNMSG_INDICATE_CONNECT

;Indicated connect aborted structure for message handler
;DPN_MSGID_INDICATED_CONNECT_ABORTED)
DPNMSG_INDICATED_CONNECT_ABORTED struct
	dwSize dd ?;				// Size of this structure
	pvPlayerContext dd ?;	// Player context preset from DPNMSG_INDICATE_CONNECT
DPNMSG_INDICATED_CONNECT_ABORTED ends
PDPNMSG_INDICATED_CONNECT_ABORTED typedef ptr DPNMSG_INDICATED_CONNECT_ABORTED

;Peer info structure for message handler
;(DPN_MSGID_PEER_INFO)
DPNMSG_PEER_INFO struct
	dwSize dd ?;					// Size of this structure
	dpnidPeer dd ?;				// DPNID of peer
	pvPlayerContext dd ?;		// Player context value
DPNMSG_PEER_INFO ends
PDPNMSG_PEER_INFO typedef ptr DPNMSG_PEER_INFO

;Receive structure for message handler
;(DPN_MSGID_RECEIVE)
DPNMSG_RECEIVE struct
	dwSize dd ?;				// Size of this structure
	dpnidSender dd ?;		// DPNID of sending player
	pvPlayerContext dd ?;	// Player context value of sending player
	pReceiveData dd ?;		// Received data
	dwReceiveDataSize dd ?;	// Size (in bytes) of pReceiveData
	hBufferHandle dd ?;		// Buffer handle for pReceiveData
	;Fields added for DirectX 9
	dwReceiveFlags dd ?;		// Flags describing how message was received
DPNMSG_RECEIVE ends
PDPNMSG_RECEIVE typedef ptr DPNMSG_RECEIVE

;Remove player from group structure for message handler
;(DPN_MSGID_REMOVE_PLAYER_FROM_GROUP)
DPNMSG_REMOVE_PLAYER_FROM_GROUP struct
	dwSize dd ?;					// Size of this structure
	dpnidGroup dd ?;				// DPNID of group
	pvGroupContext dd ?;			// Group context value
	dpnidPlayer dd ?;			// DPNID of deleted player
	pvPlayerContext dd ?;		// Player context value
DPNMSG_REMOVE_PLAYER_FROM_GROUP ends
PDPNMSG_REMOVE_PLAYER_FROM_GROUP typedef ptr DPNMSG_REMOVE_PLAYER_FROM_GROUP

;Returned buffer structure for message handler
;(DPN_MSGID_RETURN_BUFFER)
DPNMSG_RETURN_BUFFER struct
	dwSize dd ?;				// Size of this structure
	hResultCode dd ?;		// Return value of operation
	pvBuffer dd ?;			// Buffer being returned
	pvUserContext dd ?;		// Context associated with buffer
DPNMSG_RETURN_BUFFER ends 
PDPNMSG_RETURN_BUFFER typedef ptr DPNMSG_RETURN_BUFFER

;Send complete structure for message handler
;(DPN_MSGID_SEND_COMPLETE)
DPNMSG_SEND_COMPLETE struct
	dwSize dd ?;					// Size of this structure
	hAsyncOp dd ?;				// DirectPlay8 Async operation handle
	pvUserContext dd ?;			// User context supplied at Send/SendTo
	hResultCode dd ?;			// HRESULT of send
	dwSendTime dd ?;				// Send time in ms
	;Fields added for DirectX 9
	dwFirstFrameRTT dd ?;		// RTT of the first frame in the message
	dwFirstFrameRetryCount dd ?;	// Retry count of the first frame
	dwSendCompleteFlags dd ?;	// Flags describing how message was sent
	pBuffers dd ?;				// Pointer to array of buffers sent, if DirectPlay did not make a copy
	dwNumBuffers dd ?;			// Number of buffers in previous array
DPNMSG_SEND_COMPLETE ends
PDPNMSG_SEND_COMPLETE typedef ptr DPNMSG_SEND_COMPLETE

;Server info structure for message handler
;(DPN_MSGID_SERVER_INFO)
DPNMSG_SERVER_INFO struct
	dwSize dd ?;					// Size of this structure
	dpnidServer dd ?;			// DPNID of server
	pvPlayerContext dd ?;		// Player context value
DPNMSG_SERVER_INFO ends
PDPNMSG_SERVER_INFO typedef ptr DPNMSG_SERVER_INFO

;Terminated session structure for message handler
;(DPN_MSGID_TERMINATE_SESSION)
DPNMSG_TERMINATE_SESSION struct
	dwSize dd ?;				// Size of this structure
	hResultCode dd ?;		// Reason
	pvTerminateData dd ?;	// Data passed from Host/Server
	dwTerminateDataSize dd ?;// Size (in bytes) of pvTerminateData
DPNMSG_TERMINATE_SESSION ends
PDPNMSG_TERMINATE_SESSION typedef ptr DPNMSG_TERMINATE_SESSION

;Message structures added for DirectX 9

;Create thread info structure for message handler
;(DPN_MSGID_CREATE_THREAD)
DPNMSG_CREATE_THREAD struct
	dwSize dd ?;				// Size of this structure
	dwFlags dd ?;			// Flags describing this thread
	dwProcessorNum dd ?;		// Index of processor to which thread is bound
	pvUserContext dd ?;		// Thread context value
DPNMSG_CREATE_THREAD ends
PDPNMSG_CREATE_THREAD typedef ptr DPNMSG_CREATE_THREAD

;Destroy thread info structure for message handler
;(DPN_MSGID_DESTROY_THREAD)
DPNMSG_DESTROY_THREAD struct
	dwSize dd ?;				// Size of this structure
	dwProcessorNum dd ?;		// Index of processor to which thread was bound
	pvUserContext dd ?;		// Thread context value
DPNMSG_DESTROY_THREAD ends
PDPNMSG_DESTROY_THREAD typedef ptr DPNMSG_DESTROY_THREAD

;Query-to-resolve-NAT-address structure for message handler
;(DPN_MSGID_NAT_RESOLVER_QUERY)
DPNMSG_NAT_RESOLVER_QUERY struct
	dwSize dd ?;				// Size of this structure.
	pAddressSender dd ?;	// Address of client that sent the query
	pAddressDevice dd ?;	// Address of device on which query was received
	pwszUserString dd ?;	// User specified string, or NULL if none
DPNMSG_NAT_RESOLVER_QUERY ends
PDPNMSG_NAT_RESOLVER_QUERY typedef ptr DPNMSG_NAT_RESOLVER_QUERY

;========================
;DirectPlay8 Functions
;========================


;This function is no longer supported.  It is recommended that CoCreateInstance be used to create DirectPlay8 objects.
;DirectPlay8Create( const CLSID * pcIID, void **ppvInterface, IUnknown *pUnknown );

;DirectPlay8 Application Interfaces
;COM definition for DirectPlay8 Client interface

BEGIN_INTERFACE IDirectPlay8Client,IUnknown,<5102DACD-241B-11d3-AEA7-006097B01411>
	STDMETHOD Initialize,Pointer,Pointer,dword
	STDMETHOD EnumServiceProviders,Pointer,Pointer,Pointer,Pointer,Pointer,dword
	STDMETHOD EnumHosts,Pointer,Pointer,Pointer,Pointer,dword,dword,dword,dword,Pointer,Pointer,dword
	STDMETHOD CancelAsyncOperation,dword,dword
	STDMETHOD Connect,Pointer,Pointer,Pointer,Pointer,Pointer,Pointer,dword,Pointer,Pointer,dword
	STDMETHOD Send,Pointer,dword,dword,Pointer,Pointer,dword
	STDMETHOD GetSendQueueInfo,Pointer,Pointer,dword
	STDMETHOD GetApplicationDesc,Pointer,Pointer,dword
	STDMETHOD SetClientInfo,Pointer,Pointer,Pointer,dword
	STDMETHOD GetServerInfo,Pointer,Pointer,dword
	STDMETHOD GetServerAddress,Pointer,dword
	STDMETHOD Close,dword
	STDMETHOD ReturnBuffer,dword,dword
	STDMETHOD GetCaps,Pointer,dword
	STDMETHOD SetCaps,Pointer,dword
	STDMETHOD SetSPCaps,Pointer,Pointer,dword
	STDMETHOD GetSPCaps,Pointer,Pointer,dword
	STDMETHOD GetConnectionInfo,Pointer,dword
	STDMETHOD RegisterLobby,dword,Pointer,dword
END_INTERFACE

;COM definition for DirectPlay8 Server interface
BEGIN_INTERFACE IDirectPlay8Server,IUnknown,<5102DACE-241B-11d3-AEA7-006097B01411>
	STDMETHOD Initialize,Pointer,Pointer,dword
	STDMETHOD EnumServiceProviders,Pointer,Pointer,Pointer,Pointer,Pointer,dword
	STDMETHOD CancelAsyncOperation,dword,dword
	STDMETHOD GetSendQueueInfo,dword,Pointer,Pointer,dword
	STDMETHOD GetApplicationDesc,Pointer,Pointer,dword
	STDMETHOD SetServerInfo,Pointer,Pointer,Pointer,dword
	STDMETHOD GetClientInfo,dword,Pointer,Pointer,dword
	STDMETHOD GetClientAddress,dword,Pointer,dword
	STDMETHOD GetLocalHostAddresses,Pointer,Pointer,dword
	STDMETHOD SetApplicationDesc,Pointer,dword
	STDMETHOD Host,Pointer,Pointer,dword,Pointer,Pointer,Pointer,dword
	STDMETHOD SendTo,dword,Pointer,dword,dword,Pointer,Pointer,dword
	STDMETHOD CreateGroup,Pointer,Pointer,Pointer,Pointer,dword
	STDMETHOD DestroyGroup,dword,Pointer,Pointer,dword
	STDMETHOD AddPlayerToGroup,dword,dword,Pointer,Pointer,dword
	STDMETHOD RemovePlayerFromGroup,dword,dword,Pointer,Pointer,dword
	STDMETHOD SetGroupInfo,dword,Pointer,Pointer,Pointer,dword
	STDMETHOD GetGroupInfo,dword,Pointer,Pointer,dword
	STDMETHOD EnumPlayersAndGroups,Pointer,Pointer,dword
	STDMETHOD EnumGroupMembers,dword,Pointer,Pointer,dword
	STDMETHOD Close,dword
	STDMETHOD DestroyClient,dword,Pointer,dword,dword
	STDMETHOD ReturnBuffer,dword,dword
	STDMETHOD GetPlayerContext,dword,Pointer,dword
	STDMETHOD GetGroupContext,dword,Pointer,dword
	STDMETHOD GetCaps,Pointer,dword
	STDMETHOD SetCaps,Pointer,dword
	STDMETHOD SetSPCaps,Pointer,Pointer,dword
	STDMETHOD GetSPCaps,Pointer,Pointer,dword
	STDMETHOD GetConnectionInfo,dword,Pointer,dword
	STDMETHOD RegisterLobby,dword,Pointer,dword
END_INTERFACE

;COM definition for DirectPlay8 Peer interface
BEGIN_INTERFACE IDirectPlay8Peer,IUnknown,<5102DACF-241B-11d3-AEA7-006097B01411>
	STDMETHOD Initialize,Pointer,Pointer,dword
	STDMETHOD EnumServiceProviders,Pointer,Pointer,Pointer,Pointer,Pointer,dword
	STDMETHOD CancelAsyncOperation,dword,dword
	STDMETHOD Connect,Pointer,Pointer,Pointer,Pointer,Pointer,Pointer,dword,Pointer,Pointer,Pointer,dword
	STDMETHOD SendTo,dword,Pointer,dword,dword,Pointer,Pointer,dword
	STDMETHOD GetSendQueueInfo,dword,Pointer,Pointer,dword
	STDMETHOD Host,Pointer,Pointer,dword,Pointer,Pointer,Pointer,dword
	STDMETHOD GetApplicationDesc,Pointer,Pointer,dword
	STDMETHOD SetApplicationDesc,Pointer,dword
	STDMETHOD CreateGroup,Pointer,Pointer,Pointer,Pointer,dword
	STDMETHOD DestroyGroup,dword,Pointer,Pointer,dword
	STDMETHOD AddPlayerToGroup,dword,dword,Pointer,Pointer,dword
	STDMETHOD RemovePlayerFromGroup,dword,dword,Pointer,Pointer,dword
	STDMETHOD SetGroupInfo,dword,Pointer,Pointer,Pointer,dword
	STDMETHOD GetGroupInfo,dword,Pointer,Pointer,dword
	STDMETHOD EnumPlayersAndGroups,Pointer,Pointer,dword
	STDMETHOD EnumGroupMembers,dword,Pointer,Pointer,dword
	STDMETHOD SetPeerInfo,Pointer,Pointer,Pointer,dword
	STDMETHOD GetPeerInfo,dword,Pointer,Pointer,dword
	STDMETHOD GetPeerAddress,dword,Pointer,dword
	STDMETHOD GetLocalHostAddresses,Pointer,Pointer,dword
	STDMETHOD Close,dword
	STDMETHOD EnumHosts,Pointer,Pointer,Pointer,Pointer,dword,dword,dword,dword,Pointer,Pointer,dword
	STDMETHOD DestroyPeer,dword,Pointer,dword,dword
	STDMETHOD ReturnBuffer,dword,dword
	STDMETHOD GetPlayerContext,dword,Pointer,dword
	STDMETHOD GetGroupContext,dword,Pointer,dword
	STDMETHOD GetCaps,Pointer,dword
	STDMETHOD SetCaps,Pointer,dword
	STDMETHOD SetSPCaps,Pointer,Pointer,dword
	STDMETHOD GetSPCaps,Pointer,Pointer,dword
	STDMETHOD GetConnectionInfo,dword,Pointer,dword
	STDMETHOD RegisterLobby,dword,Pointer,dword
	STDMETHOD TerminateSession,Pointer,dword,dword
END_INTERFACE

;COM definition for DirectPlay8 Thread Pool interface
BEGIN_INTERFACE IDirectPlay8ThreadPool,IUnknown,<0D22EE73-4A46-4a0d-89B2-045B4D666425>
	STDMETHOD Initialize,Pointer,Pointer,dword
	STDMETHOD Close,dword
	STDMETHOD GetThreadCount,Pointer,Pointer,dword
	STDMETHOD SetThreadCount,dword,dword,dword
	STDMETHOD DoWork,dword,dword
END_INTERFACE

;COM definition for DirectPlay8 NAT Resolver interface
BEGIN_INTERFACE IDirectPlay8NATResolver,IUnknown,<A9E213F2-9A60-486f-BF3B-53408B6D1CBB>
	STDMETHOD Initialize,Pointer,Pointer,dword
	STDMETHOD Start,Pointer,dword,dword
	STDMETHOD Close,dword
	STDMETHOD EnumDevices,Pointer,Pointer,Pointer,dword
	STDMETHOD GetAddresses,Pointer,Pointer,dword
END_INTERFACE


;DIRECTPLAY8 ERRORS
;Errors are represented by negative values and cannot be combined.

_DPN_FACILITY_CODE				equ 15h
_DPNHRESULT_BASE				equ 8000h
MAKE_DPNHRESULT macro ccode
	MAKE_HRESULT( 1, _DPN_FACILITY_CODE, ( ccode + _DPNHRESULT_BASE ) )
endm

DPN_OK	equ S_OK

DPNSUCCESS_EQUAL equ MAKE_HRESULT( 0, _DPN_FACILITY_CODE, ( 5 + _DPNHRESULT_BASE ) )
DPNSUCCESS_NOPLAYERSINGROUP	equ	MAKE_HRESULT( 0, _DPN_FACILITY_CODE, ( 8 + _DPNHRESULT_BASE ) )	; added for DirectX 9
DPNSUCCESS_NOTEQUAL			equ	MAKE_HRESULT( 0, _DPN_FACILITY_CODE, (10 + _DPNHRESULT_BASE ) )
DPNSUCCESS_PENDING			equ	MAKE_HRESULT( 0, _DPN_FACILITY_CODE, (14 + _DPNHRESULT_BASE ) )

 DPNERR_ABORTED			equ		MAKE_DPNHRESULT( 30h )
 DPNERR_ADDRESSING		equ		MAKE_DPNHRESULT( 40h )
 DPNERR_ALREADYCLOSING	equ		MAKE_DPNHRESULT( 50h )
 DPNERR_ALREADYCONNECTED		equ	MAKE_DPNHRESULT( 60h )
 DPNERR_ALREADYDISCONNECTING	equ	MAKE_DPNHRESULT( 70h )
 DPNERR_ALREADYINITIALIZED	equ	MAKE_DPNHRESULT( 80h )
 DPNERR_ALREADYREGISTERED	equ	MAKE_DPNHRESULT( 90h )
 DPNERR_BUFFERTOOSMALL		equ	MAKE_DPNHRESULT( 100h )
 DPNERR_CANNOTCANCEL			equ	MAKE_DPNHRESULT( 110h )
 DPNERR_CANTCREATEGROUP		equ	MAKE_DPNHRESULT( 120h )
 DPNERR_CANTCREATEPLAYER		equ	MAKE_DPNHRESULT( 130h )
 DPNERR_CANTLAUNCHAPPLICATION equ	MAKE_DPNHRESULT( 140h )
 DPNERR_CONNECTING		equ		MAKE_DPNHRESULT( 150h )
 DPNERR_CONNECTIONLOST	equ		MAKE_DPNHRESULT( 160h )
 DPNERR_CONVERSION		equ		MAKE_DPNHRESULT( 170h )
 DPNERR_DATATOOLARGE		equ		MAKE_DPNHRESULT( 175h )
 DPNERR_DOESNOTEXIST		equ		MAKE_DPNHRESULT( 180h )
 DPNERR_DPNSVRNOTAVAILABLE	equ	MAKE_DPNHRESULT( 185h )
 DPNERR_DUPLICATECOMMAND	equ		MAKE_DPNHRESULT( 190h )
 DPNERR_ENDPOINTNOTRECEIVING	equ	MAKE_DPNHRESULT( 200h )
 DPNERR_ENUMQUERYTOOLARGE	equ	MAKE_DPNHRESULT( 210h )
 DPNERR_ENUMRESPONSETOOLARGE	equ	MAKE_DPNHRESULT( 220h )
 DPNERR_EXCEPTION		equ		MAKE_DPNHRESULT( 230h )
 DPNERR_GENERIC			equ		E_FAIL
 DPNERR_GROUPNOTEMPTY	equ		MAKE_DPNHRESULT( 240h )
 DPNERR_HOSTING			equ		MAKE_DPNHRESULT( 250h )
 DPNERR_HOSTREJECTEDCONNECTION equ	MAKE_DPNHRESULT( 260h )
 DPNERR_HOSTTERMINATEDSESSION equ	MAKE_DPNHRESULT( 270h )
 DPNERR_INCOMPLETEADDRESS	equ	MAKE_DPNHRESULT( 280h )
 DPNERR_INVALIDADDRESSFORMAT	equ	MAKE_DPNHRESULT( 290h )
 DPNERR_INVALIDAPPLICATION	equ	MAKE_DPNHRESULT( 300h )
 DPNERR_INVALIDCOMMAND		equ	MAKE_DPNHRESULT( 310h )
 DPNERR_INVALIDDEVICEADDRESS	equ	MAKE_DPNHRESULT( 320h )
 DPNERR_INVALIDENDPOINT		equ	MAKE_DPNHRESULT( 330h )
 DPNERR_INVALIDFLAGS			equ	MAKE_DPNHRESULT( 340h )
 DPNERR_INVALIDGROUP			equ MAKE_DPNHRESULT( 350h )
 DPNERR_INVALIDHANDLE		equ	MAKE_DPNHRESULT( 360h )
 DPNERR_INVALIDHOSTADDRESS	equ	MAKE_DPNHRESULT( 370h )
 DPNERR_INVALIDINSTANCE		equ	MAKE_DPNHRESULT( 380h )
 DPNERR_INVALIDINTERFACE		equ	MAKE_DPNHRESULT( 390h )
 DPNERR_INVALIDOBJECT		equ	MAKE_DPNHRESULT( 400h )
 DPNERR_INVALIDPARAM			equ	E_INVALIDARG
 DPNERR_INVALIDPASSWORD		equ	MAKE_DPNHRESULT( 410h )
 DPNERR_INVALIDPLAYER		equ	MAKE_DPNHRESULT( 420h )
 DPNERR_INVALIDPOINTER		equ	E_POINTER
 DPNERR_INVALIDPRIORITY		equ	MAKE_DPNHRESULT( 430h )
 DPNERR_INVALIDSTRING		equ	MAKE_DPNHRESULT( 440h )
 DPNERR_INVALIDURL			equ	MAKE_DPNHRESULT( 450h )
 DPNERR_INVALIDVERSION		equ	MAKE_DPNHRESULT( 460h )
 DPNERR_NOCAPS				equ	MAKE_DPNHRESULT( 470h )
 DPNERR_NOCONNECTION			equ	MAKE_DPNHRESULT( 480h )
 DPNERR_NOHOSTPLAYER			equ	MAKE_DPNHRESULT( 490h )
 DPNERR_NOINTERFACE			equ	E_NOINTERFACE
 DPNERR_NOMOREADDRESSCOMPONENTS equ	MAKE_DPNHRESULT( 500h )
 DPNERR_NORESPONSE			equ	MAKE_DPNHRESULT( 510h )
 DPNERR_NOTALLOWED			equ	MAKE_DPNHRESULT( 520h )
 DPNERR_NOTHOST				equ	MAKE_DPNHRESULT( 530h )
 DPNERR_NOTREADY				equ	MAKE_DPNHRESULT( 540h )
 DPNERR_NOTREGISTERED		equ	MAKE_DPNHRESULT( 550h )
 DPNERR_OUTOFMEMORY			equ	E_OUTOFMEMORY
 DPNERR_PENDING				equ	DPNSUCCESS_PENDING
 DPNERR_PLAYERALREADYINGROUP	equ	MAKE_DPNHRESULT( 560h )
 DPNERR_PLAYERLOST			equ	MAKE_DPNHRESULT( 570h )
 DPNERR_PLAYERNOTINGROUP		equ	MAKE_DPNHRESULT( 580h )
 DPNERR_PLAYERNOTREACHABLE	equ	MAKE_DPNHRESULT( 590h )
 DPNERR_SENDTOOLARGE			equ	MAKE_DPNHRESULT( 600h )
 DPNERR_SESSIONFULL			equ	MAKE_DPNHRESULT( 610h )
 DPNERR_TABLEFULL			equ	MAKE_DPNHRESULT( 620h )
 DPNERR_TIMEDOUT				equ	MAKE_DPNHRESULT( 630h )
 DPNERR_UNINITIALIZED		equ	MAKE_DPNHRESULT( 640h )
 DPNERR_UNSUPPORTED			equ	E_NOTIMPL
 DPNERR_USERCANCEL			equ	MAKE_DPNHRESULT( 650h )


endif

