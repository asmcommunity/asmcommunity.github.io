;DirectPlay Lobby include file (April 2006 DX-SDK)
;Translated to MASM format by EvilHomer
;17 July 2006

ifndef __DPLOBBY_INCLUDED__
__DPLOBBY_INCLUDED__ equ 1

echo == DirectPlayLobby Base ==

DefGUID IID_IDirectPlayLobby, <AF465C71-9588-11cf-A020-00AA006157AC>
DefGUID IID_IDirectPlayLobbyA,<26C66A70-B367-11cf-A024-00AA006157AC>
DefGUID IID_IDirectPlayLobby2, <0194c220-a303-11d0-9c4f-00a0c905425e>
DefGUID IID_IDirectPlayLobby2A, <1bb4af80-a303-11d0-9c4f-00a0c905425e>
DefGUID IID_IDirectPlayLobby3,<2DB72490-652C-11d1-A7A8-0000F803ABFC>
DefGUID IID_IDirectPlayLobby3A,<2DB72491-652C-11d1-A7A8-0000F803ABFC>
DefGUID CLSID_DirectPlayLobby,<2FE8F810-B2A5-11d0-A787-0000F803ABFC>
;===================================================================

;Used to hold information about a registered DirectPlay application
DPLAPPINFO struct
dwSize dd ?		;Size of this structure
guidApplication GUID <> ;GUID of the Application
union
  lpszAppNameA LPSTR ?  ;Pointer to the Application Name
  lpszAppName LPWSTR ?
ends
DPLAPPINFO ends
LPDPLAPPINFO typedef ptr DPLAPPINFO

;A constant pointer to DPLAPPINFO
LPCDPLAPPINFO typedef ptr DPLAPPINFO

;An array of these is passed to CreateCompoundAddresses
DPCOMPOUNDADDRESSELEMENT struct
guidDataType GUID <>
dwDataSize dd ?
lpData dd ?
DPCOMPOUNDADDRESSELEMENT ends
LPDPCOMPOUNDADDRESSELEMENT typedef ptr DPCOMPOUNDADDRESSELEMENT

;A constant pointer to DPCOMPOUNDADDRESSELEMENT
LPCDPCOMPOUNDADDRESSELEMENT typedef ptr DPCOMPOUNDADDRESSELEMENT

DPAPPLICATIONDESC struct 
dwSize dd ?
dwFlags dd ?
union
  lpszApplicationNameA LPSTR ?
  lpszApplicationName LPWSTR ?
ends
guidApplication GUID <>
union
  lpszFilenameA LPSTR ?
  lpszFilename LPWSTR ?
ends
union
  lpszCommandLineA LPSTR ?
  lpszCommandLine LPWSTR ?
ends
union
  lpszPathA LPSTR ?
  lpszPath LPWSTR ?
ends
union
  lpszCurrentDirectoryA LPSTR ?
  lpszCurrentDirectory LPWSTR ?
ends
lpszDescriptionA LPSTR ?
lpszDescriptionW LPWSTR ?
DPAPPLICATIONDESC ends
LPDPAPPLICATIONDESC typedef ptr DPAPPLICATIONDESC 

DPAPPLICATIONDESC2 struct
dwSize dd ?
dwFlags dd ?
union
  lpszApplicationNameA LPSTR ?
  lpszApplicationName LPWSTR ?
ends
guidApplication GUID <>
union
  lpszFilenameA LPSTR ?
  lpszFilename LPWSTR ?
ends
union
  lpszCommandLineA LPSTR ?
  lpszCommandLine LPWSTR ?
ends
union
  lpszPathA LPSTR ?
  lpszPath LPWSTR ?
ends
union
  lpszCurrentDirectoryA LPSTR ?
  lpszCurrentDirectory LPWSTR ?
ends
lpszDescriptionA LPSTR ?
lpszDescriptionW LPWSTR ?
union
  lpszAppLauncherNameA LPSTR ?
  lpszAppLauncherName LPWSTR ?
ends
DPAPPLICATIONDESC2 ends
LPDPAPPLICATIONDESC2 typedef ptr DPAPPLICATIONDESC2 

;=======================================
;Enumeration Method Callback Prototypes
;=======================================

;Callback for EnumAddress()
;    REFGUID         guidDataType,
;    DWORD           dwDataSize,
;    LPCVOID         lpData,
;    LPVOID          lpContext
EnumAddressCallBack proto :dword,:dword,:dword,:dword

;Callback for EnumAddressTypes()
;    REFGUID         guidDataType,
;    LPVOID          lpContext,
;    DWORD           dwFlags
EnumAddressTypesCallBack proto :dword,:dword,:dword

;Callback for EnumLocalApplications()
;    LPCDPLAPPINFO   lpAppInfo,
;    LPVOID          lpContext,
;    DWORD           dwFlags
EnumLocalApplicationsCallBack proto :dword,:dword,:dword

;=======================================
;DirectPlayLobby API Prototypes
;=======================================
 ;(LPGUID, LPDIRECTPLAYLOBBY *, IUnknown *, LPVOID, DWORD );
DirectPlayLobbyCreateA proto :dword,:dword,:dword,:dword,:dword
DirectPlayLobbyCreateW proto :dword,:dword,:dword,:dword,:dword

;===================================================
;IDirectPlayLobby (and IDirectPlayLobbyA) Interfaces
;(all that differs is the GUID)
;===================================================
BEGIN_INTERFACE IDirectPlayLobby, IUnknown, <AF465C71-9588-11cf-A020-00AA006157AC>
    STDMETHOD Connect,dword,Pointer,Pointer
    STDMETHOD CreateAddress,Pointer,Pointer, Pointer,dword,Pointer,Pointer
    STDMETHOD EnumAddress,Pointer,Pointer,dword,Pointer
    STDMETHOD EnumAddressTypes,Pointer,Pointer,Pointer,dword
    STDMETHOD EnumLocalApplications,Pointer, Pointer,dword
    STDMETHOD GetConnectionSettings,dword,Pointer,Pointer
    STDMETHOD ReceiveLobbyMessage,dword, dword, Pointer, Pointer, Pointer
    STDMETHOD RunApplication,dword, Pointer, Pointer, HANDLE
    STDMETHOD SendLobbyMessage,dword, dword, Pointer, dword
    STDMETHOD SetConnectionSettings, dword, dword, Pointer
    STDMETHOD SetLobbyMessageEvent,dword, dword, HANDLE
END_INTERFACE

BEGIN_INTERFACE IDirectPlayLobbyA, IUnknown, <26C66A70-B367-11cf-A024-00AA006157AC>
    STDMETHOD Connect,dword,Pointer,Pointer
    STDMETHOD CreateAddress,Pointer,Pointer, Pointer,dword,Pointer,Pointer
    STDMETHOD EnumAddress,Pointer,Pointer,dword,Pointer
    STDMETHOD EnumAddressTypes,Pointer,Pointer,Pointer,dword
    STDMETHOD EnumLocalApplications,Pointer, Pointer,dword
    STDMETHOD GetConnectionSettings,dword,Pointer,Pointer
    STDMETHOD ReceiveLobbyMessage,dword, dword, Pointer, Pointer, Pointer
    STDMETHOD RunApplication,dword, Pointer, Pointer, HANDLE
    STDMETHOD SendLobbyMessage,dword, dword, Pointer, dword
    STDMETHOD SetConnectionSettings, dword, dword, Pointer
    STDMETHOD SetLobbyMessageEvent,dword, dword, HANDLE
END_INTERFACE

;==================================================
;IDirectPlayLobby2 (and IDirectPlayLobby2A) Interfaces
;(all that differs is the GUID)
;==================================================
BEGIN_INTERFACE IDirectPlayLobby2, IDirectPlayLobby, <0194C220-A303-11d0-9C4F-00A0C905425E>
    STDMETHOD CreateCompoundAddress,Pointer,dword,Pointer,Pointer
END_INTERFACE

BEGIN_INTERFACE IDirectPlayLobby2A, IDirectPlayLobbyA, <1BB4AF80-A303-11d0-9C4F-00A0C905425E>
    STDMETHOD CreateCompoundAddress,Pointer,dword,Pointer,Pointer
END_INTERFACE

;==================================================
;IDirectPlayLobby3 (and IDirectPlayLobby3A) Interfaces
;(all that differs is the GUID)
;==================================================
BEGIN_INTERFACE IDirectPlayLobby3, IDirectPlayLobby2, <2DB72490-652C-11d1-A7A8-0000F803ABFC>
    STDMETHOD ConnectEx,dword, Pointer, Pointer, Pointer
    STDMETHOD RegisterApplication,dword, Pointer
    STDMETHOD UnregisterApplication,dword, Pointer
    STDMETHOD WaitForConnectionSettings,dword
END_INTERFACE

BEGIN_INTERFACE IDirectPlayLobby3A, IDirectPlayLobby2A, <2DB72491-652C-11d1-A7A8-0000F803ABFC>
    STDMETHOD ConnectEx,dword, Pointer, Pointer, Pointer
    STDMETHOD RegisterApplication,dword, Pointer
    STDMETHOD UnregisterApplication,dword, Pointer
    STDMETHOD WaitForConnectionSettings,dword
END_INTERFACE

;==================================================
;DirectPlayLobby Flags
;==================================================
;This flag is used by IDirectPlayLobby->WaitForConnectionSettings to
;cancel a current wait that is in progress.
DPLWAIT_CANCEL equ 1

;This is a message flag used by ReceiveLobbyMessage.  It can be
;returned in the dwMessageFlags parameter to indicate a message from
;the system.
DPLMSG_SYSTEM equ 1

;This is a message flag used by ReceiveLobbyMessage and SendLobbyMessage.
;It is used to indicate that the message is a standard lobby message.
;DPLMSG_SETPROPERTY, DPLMSG_SETPROPERTYRESPONSE, DPLMSG_GETPROPERTY,
;DPLMSG_GETPROPERTYRESPONSE
DPLMSG_STANDARD equ 2

;Lobbyable Application registration flags:
;Applications registered with this flag will not show up when
;applications are enumerated in the lobby.  This application
;will only be able to be launched by a lobby client that already
;knows about the application.
DPLAPP_NOENUM equ 80000000h

;Applications registered with this flag want voice to automatically
;be enabled for their application.  All players will be launched into
;an 'n'-way voice conference when the application is started.  The 
;user will be able to enable this flag for existing non-voice 
;directplay applications.
DPLAPP_AUTOVOICE equ 1

;Applications that do their own voice conferencing should register with
;this flag to avoid allowing the user to enable other voice chat 
;capabilites during the same session.  This is to avoid users forcing
;the DPLAPP_AUTOVOICE flag for the application.
DPLAPP_SELFVOICE equ 2

;==================================================
;DirectPlayLobby messages and message data structures:
;All system messages have a dwMessageFlags value of DPLMSG_SYSTEM returned
;from a call to ReceiveLobbyMessage.
;All standard messages have a dwMessageFlags value of DPLMSG_STANDARD returned
;from a call to ReceiveLobbyMessage.
;==================================================

;Generic message structure used to identify the message type.
DPLMSG_GENERIC struct
	dwType dd ?
DPLMSG_GENERIC ends
LPDPLMSG_GENERIC typedef ptr DPLMSG_GENERIC

;Generic message format for all system messages --
;DPLSYS_CONNECTIONSETTINGSREAD, DPLSYS_DPLYCONNECTSUCCEEDED,
;DPLSYS_DPLAYCONNECTFAILED, DPLSYS_APPTERMINATED, DPLSYS_NEWCONNECTIONSETTINGS
DPLMSG_SYSTEMMESSAGE struct
	dwType dd ?  		 ; Message type
	guidInstance GUID <> ; Instance GUID of the dplay session the message corresponds to
DPLMSG_SYSTEMMESSAGE ends
LPDPLMSG_SYSTEMMESSAGE typedef ptr DPLMSG_SYSTEMMESSAGE

;Standard message sent by an application to a lobby to set a property
DPLMSG_SETPROPERTY struct
	dwType dd ? 			; Message type
	dwRequestID dd ?		; Request ID (DPL_NOCONFIRMATION if no confirmation desired)
	guidPlayer GUID <>		; Player GUID
	guidPropertyTag GUID <> ; Property GUID
	dwDataSize dd ?			; Size of data
	dwPropertyData dd ?		; Buffer containing data
DPLMSG_SETPROPERTY ends
LPDPLMSG_SETPROPERTY typedef ptr DPLMSG_SETPROPERTY

DPL_NOCONFIRMATION equ 0

;Standard message returned by a lobby to confirm a DPLMSG_SETPROPERTY message.
DPLMSG_SETPROPERTYRESPONSE struct
	dwType dd ?;				 Message type
	dwRequestID dd ?;			 Request ID
	guidPlayer GUID <>;			Player GUID
	guidPropertyTag GUID <>;	Property GUID
	hr dd ?;					Return Code
DPLMSG_SETPROPERTYRESPONSE ends
LPDPLMSG_SETPROPERTYRESPONSE typedef ptr DPLMSG_SETPROPERTYRESPONSE

;Standard message sent by an application to a lobby to request
;the current value of a property
DPLMSG_GETPROPERTY struct
	dwType dd ?;				 Message type
	dwRequestID dd ?;			 Request ID
	guidPlayer GUID <>;			 Player GUID
	guidPropertyTag GUID <>;	 Property GUID
DPLMSG_GETPROPERTY ends
LPDPLMSG_GETPROPERTY typedef ptr DPLMSG_GETPROPERTY


; Standard message returned by a lobby in response to a DPLMSG_GETPROPERTY message.
DPLMSG_GETPROPERTYRESPONSE struct
	dwType dd ?;				 Message type
	dwRequestID dd ?;			 Request ID
	guidPlayer GUID <>;			 Player GUID
	guidPropertyTag GUID <>;	 Property GUID
	hr dd ?;					 Return Code
	dwDataSize dd ?;			 Size of data
	dwPropertyData dd ?;		 Buffer containing data
DPLMSG_GETPROPERTYRESPONSE ends
LPDPLMSG_GETPROPERTYRESPONSE typedef ptr DPLMSG_GETPROPERTYRESPONSE

;Standard message returned by a lobby in response to a
;the session host migrating to a new client
DPLMSG_NEWSESSIONHOST struct
    dwType dd ?;              Message type
    guidInstance GUID <>;     GUID Instance of the session
DPLMSG_NEWSESSIONHOST ends
LPDPLMSG_NEWSESSIONHOST typedef ptr DPLMSG_NEWSESSIONHOST


;==================================================
;DirectPlay Lobby message dwType values
;==================================================
;The application has read the connection settings...
;It is now O.K. for the lobby client to release
;its IDirectPlayLobby interface.
DPLSYS_CONNECTIONSETTINGSREAD equ 1 ;See above ^
DPLSYS_DPLAYCONNECTFAILED	  equ 2	;The application's call to DirectPlayConnect failed
DPLSYS_DPLAYCONNECTSUCCEEDED  equ 3	;The application has created a DirectPlay session.
DPLSYS_APPTERMINATED 		  equ 4	;The application has terminated.
DPLSYS_SETPROPERTY 			  equ 5	;The message is a DPLMSG_SETPROPERTY message.
DPLSYS_SETPROPERTYRESPONSE	  equ 6 ;The message is a DPLMSG_SETPROPERTYRESPONSE message
DPLSYS_GETPROPERTY	 		  equ 7 ;The message is a DPLMSG_GETPROPERTY message
DPLSYS_GETPROPERTYRESPONSE	  equ 8 ;The message is a DPLMSG_GETPROPERTYRESPONSE message.
DPLSYS_NEWSESSIONHOST		  equ 9 ;The message is a DPLMSG_NEWSESSIONHOST message.
DPLSYS_NEWCONNECTIONSETTINGS  equ 10;New connection settings are available.
DPLSYS_LOBBYCLIENTRELEASE 	  equ 11 ;The Lobby Client has released the DirectPlayLobby Interface

;================================================================
;DirectPlay defined property GUIDs and associated data structures
;================================================================
;DPLPROPERTY_MessagesSupported:
;Request whether the lobby supports standard.  Lobby with respond with either
;TRUE or FALSE or may not respond at all. 
;Property data is a single BOOL with TRUE or FALSE
;{762CCDA1-D916-11d0-BA39-00C04FD7ED67}
DefGUID IID_DPLPROPERTY_MessagesSupported, <762CCDA1-D916-11d0-BA39-00C04FD7ED67>

;DPLPROPERTY_LobbyGuid:
;Request the GUID that identifies the lobby software that the application is communicating with.
;Property data is a single GUID.
;{F56920A0-D218-11d0-BA39-00C04FD7ED67}
DefGUID IID_DPLPROPERTY_LobbyGuid, <F56920A0-D218-11d0-BA39-00C04FD7ED67>

;DPLPROPERTY_PlayerGuid:
;Request the GUID that identifies the player on this machine for sending
;property data back to the lobby.
;Property data is the DPLDATA_PLAYERDATA structure
;{B4319322-D20D-11d0-BA39-00C04FD7ED67}
DefGUID IID_DPLPROPERTY_PlayerGuid, <B4319322-D20D-11d0-BA39-00C04FD7ED67>

;Data structure to hold the GUID of the player and player creation flags from the lobby.
DPLDATA_PLAYERGUID struct
	guidPlayer GUID <>
	dwPlayerFlags dd ?
DPLDATA_PLAYERGUID ends
LPDPLDATA_PLAYERGUID typedef ptr DPLDATA_PLAYERGUID

;DPLPROPERTY_PlayerScore:
;Used to send an array of long integers to the lobby indicating the score of a player.
;Property data is the DPLDATA_PLAYERSCORE structure.
;{48784000-D219-11d0-BA39-00C04FD7ED67}
DefGUID IID_DPLPROPERTY_PlayerScore, <48784000-D219-11d0-BA39-00C04FD7ED67>

;Data structure to hold an array of long integers representing a player score.
;Application must allocate enough memory to hold all the scores.
DPLDATA_PLAYERSCORE struct
	dwScoreCount dd ?
	Score 		 dd ?
DPLDATA_PLAYERSCORE ends
LPDPLDATA_PLAYERSCORE typedef ptr DPLDATA_PLAYERSCORE

;=============================
;DirectPlay Address IDs
;=============================
;DirectPlay Address:
;A DirectPlay address consists of multiple chunks of data, each tagged
;with a GUID signifying the type of data in the chunk. The chunk also
;has a length so that unknown chunk types can be skipped.
;The EnumAddress() function is used to parse these address data chunks.



;Header for block of address data elements
DPADDRESS struct
	guidDataType GUID <>
    dwDataSize dd ?
DPADDRESS ends
LPDPADDRESS typedef ptr DPADDRESS


;Chunk is a DWORD containing size of entire DPADDRESS structure
;{1318F560-912C-11d0-9DAA-00A0C90A43CB}
DefGUID IID_DPAID_TotalSize, <1318F560-912C-11d0-9DAA-00A0C90A43CB>

;Chunk is a GUID describing the service provider that created the chunk.
;All addresses must contain this chunk.
;{07D916C0-E0AF-11cf-9C4E-00A0C905425E}
DefGUID IID_DPAID_ServiceProvider,<07D916C0-E0AF-11cf-9C4E-00A0C905425E>

;Chunk is a GUID describing the lobby provider that created the chunk.
;All addresses must contain this chunk.
;{59B95640-9667-11d0-A77D-0000F803ABFC}
DefGUID IID_DPAID_LobbyProvider, <59B95640-9667-11d0-A77D-0000F803ABFC>

;Chunk is a string containing a phone number (i.e. "1-800-555-1212")
;in ANSI or UNICODE format
;{78EC89A0-E0AF-11cf-9C4E-00A0C905425E}
DefGUID IID_DPAID_Phone, <78EC89A0-E0AF-11cf-9C4E-00A0C905425E>
;{BA5A7A70-9DBF-11d0-9CC1-00A0C905425E}
DefGUID IID_DPAID_PhoneW, <BA5A7A70-9DBF-11d0-9CC1-00A0C905425E>

;Chunk is a string containing a modem name registered with TAPI
;in ANSI or UNICODE format
;{F6DCC200-A2FE-11d0-9C4F-00A0C905425E}
DefGUID IID_DPAID_Modem, <F6DCC200-A2FE-11d0-9C4F-00A0C905425E>
; {01FD92E0-A2FF-11d0-9C4F-00A0C905425E}
DefGUID IID_DPAID_ModemW, <01FD92E0-A2FF-11d0-9C4F-00A0C905425E>

;Chunk is a string containing a TCP/IP host name or an IP address
;(i.e. "dplay.microsoft.com" or "137.55.100.173") in ANSI or UNICODE format
;{C4A54DA0-E0AF-11cf-9C4E-00A0C905425E}
DefGUID IID_DPAID_INet, <C4A54DA0-E0AF-11cf-9C4E-00A0C905425E>
;{E63232A0-9DBF-11d0-9CC1-00A0C905425E}
DefGUID IID_DPAID_INetW, <E63232A0-9DBF-11d0-9CC1-00A0C905425E>

;Chunk is the port number used for creating the apps TCP and UDP sockets.
;WORD value (i.e. 47624).
;{E4524541-8EA5-11d1-8A96-006097B01411}
DefGUID IID_DPAID_INetPort, <E4524541-8EA5-11d1-8A96-006097B01411>

;DPCOMPORTADDRESS:
;Used to specify com port settings. The constants that define baud rate,
;stop bits and parity are defined in WINBASE.H. The constants for flow
;control are given below.
 DPCPA_NOFLOW        equ 0           ; no flow control
 DPCPA_XONXOFFFLOW   equ 1           ; software flow control
 DPCPA_RTSFLOW       equ 2           ; hardware flow control with RTS
 DPCPA_DTRFLOW       equ 3           ; hardware flow control with DTR
 DPCPA_RTSDTRFLOW    equ 4           ; hardware flow control with RTS and DTR

DPCOMPORTADDRESS struct
    dwComPort dd ?                  ; COM port to use (1-4)
    dwBaudRate dd ?                 ; baud rate (100-256k)
    dwStopBits dd ?                 ; no. stop bits (1-2)
    dwParity dd ?                   ; parity (none, odd, even, mark)
    dwFlowControl dd ?              ; flow control (none, xon/xoff, rts, dtr)
DPCOMPORTADDRESS ends
LPDPCOMPORTADDRESS typedef ptr DPCOMPORTADDRESS

;Chunk contains a DPCOMPORTADDRESS structure defining the serial port.
;{F2F0CE00-E0AF-11cf-9C4E-00A0C905425E}
DefGUID IID_DPAID_ComPort, <F2F0CE00-E0AF-11cf-9C4E-00A0C905425E>

;dplobby 1.0 obsolete definitions
;Included for compatibility only.
DPLAD_SYSTEM equ DPLMSG_SYSTEM


endif
