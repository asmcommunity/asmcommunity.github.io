;The ClientGroup object affords the Server a way to organize groups of Clients.
;Methods exist for things like broadcasting to all members of a group.
;How your Server uses Groups depends on your implementation,
;for example a Group could represent a ChatRoom / communication channel,
;or it could represent a team of aligned players in a game.
;NOTE: This object supports HIERARCHIES of Groups (SubGroups)
;		In order to Add and Remove Clients from a Group,
;		just call the regular Insert and Delete methods...
;		Insert() can return NULL meaning the Client was already in the group.


if IMPLEMENT
Method ClientGroup.Init,uses esi,pOwner,pszGroupName:LPSTR,pszGroupDesc:LPSTR
	SetObject esi
	ACall Init,pOwner,64,64,640
	OCall [esi].SubGroups::Collection.Init,pOwner,64,64,640	
	mov [esi].pszGroupName, $invoke(StrNew,pszGroupName)
	.if pszGroupDesc!=0
		mov [esi].pszGroupDesc,$invoke(StrNew,pszGroupDesc)
	.endif
MethodEnd

Method ClientGroup.Done,uses esi
	SetObject esi
	;Clean up any SubGroups first
	.while [esi].SubGroups.dCount!=0
		OCall [esi].SubGroups::Collection.DisposeAt,0
	.endw
	OCall [esi].SubGroups::Collection.Done
	;Then clean up Myself	
	invoke StrDispose, [esi].pszGroupName
	.if [esi].pszGroupDesc!=0
		invoke StrDispose,[esi].pszGroupDesc
	.endif
	ACall Done ;need this?
MethodEnd

;Add a new SubGroup to this ClientGroup.
;DOES NOT CHECK FOR NAME DUPLICATES
;pszGroupName = ptr to name of new group
;pszDesc      = ptr to descriptive string
;Returns ptr to new ClientGroup, or NULL (already exists)
Method ClientGroup.AddSubGroup,uses esi,pszGroupName:LPSTR,pszDesc:LPSTR
	SetObject esi
	OCall FindGroupByName,pszGroupName
	.if eax==0
		OCall [esi].SubGroups::Collection.Insert,$New(ClientGroup,Init,esi,pszGroupName,pszDesc)
	.else
		xor eax,eax
	.endif
MethodEnd

;Destroys a particular ClientGroup and removes from this Parent.
;This method *SHOULD* be checking whether there was Clients in that Group
;and deciding what to do with them *BEFORE* destroying the Group.
Method ClientGroup.RemoveSubGroup,uses esi,pGroup
	SetObject esi
	OCall [esi].SubGroups::Collection.Dispose,pGroup
MethodEnd

;Recursive search for a particular GroupName
;Returns ptr to ClientGroup, or NULL
Method ClientGroup.FindGroupByName,uses esi edi,pszSubGroup:LPSTR
	SetObject esi	
	invoke lstrcmp, [esi].pszGroupName,pszSubGroup
	.if eax==0
		return esi
	.endif
	xor ecx,ecx
	mov edi,[esi].SubGroups.pItems
	.while ecx<[esi].SubGroups.dCount
		push ecx
		mov eax,[edi+ecx*4]
		OCall dword ptr[edi+ecx*4]::ClientGroup.FindGroupByName,pszSubGroup
		pop ecx
		.if eax!=0
			ExitMethod
		.endif
		inc ecx
	.endw
	xor eax,eax
MethodEnd

;Search only in *this* group for a username
;Returns ptr to Client, or NULL
Method ClientGroup.FindUserByName,uses esi,pUser:LPSTR
    SetObject esi
    xor ebx,ebx
    .while ebx<[esi].dCount
        OCall esi.ItemAt,ebx
        push eax
        invoke lstrcmpi,pUser,addr [eax].Client.UserID.namestring
        pop edx
        .if eax==0
            return edx
        .endif
        inc ebx
    .endw
    xor eax,eax
MethodEnd

;Search only in *this* group for a Client
;Returns ptr to Client, or NULL
Method ClientGroup.FindClient,uses esi,pClient
    SetObject esi
    OCall esi.IndexOf,pClient
    .if eax==-1
        xor eax,eax
    .else
        mov eax,pClient
    .endif
MethodEnd

;Delete a Client from this Group and all its SubGroups
;We are missing an opportunity to notify other Clients (ie, in this group)
Method ClientGroup.GoodbyeClient,uses esi,pClient
	SetObject esi	
	;Remove Client from this Group
	DbgWarning "GROUP BYEBYE"
	OCall IndexOf,pClient
	.if eax!=-1
		OCall DeleteAt,eax
	.endif
	;Remove Client from all subgroups
	.if [esi].SubGroups.dCount!=0
		OCall [esi].SubGroups::Collection.ForEach,$MethodAddr(ClientGroup.GoodbyeClient),pClient
	.endif
MethodEnd

;Send some data to all members of this group,
;with the exception of the client who is doing the sending.
Method ClientGroup.Send,uses esi,pClient,pData,dLength
    SetObject esi
    ;Send some data to the Clients in this group 
    xor ecx,ecx
    .while ecx<[esi].dCount
        push ecx
        OCall esi.ItemAt,ecx
        .if eax!=pClient
            OCall eax::Client.QueueWrite,pData,dLength
        .endif
        pop ecx
        inc ecx
    .endw
MethodEnd

;Broadcast some data to all members of this group and its subgroups,
;with the exception of the client who is doing the sending.
Method ClientGroup.Broadcast,uses esi,pClient,pData,dLength
    SetObject esi
    ;Send some data to the Clients in this group 
    OCall esi.Send, pClient,pData,dLength
    ;We should attempt to send the data to members of subgroups, if any
    xor ecx,ecx
    .while ecx<[esi].SubGroups.dCount
        push ecx
        OCall [esi].SubGroups::Collection.ItemAt,ecx
        OCall eax::ClientGroup.Broadcast,pClient,pData,dLength
        pop ecx
        inc ecx
    .endw
MethodEnd

endif