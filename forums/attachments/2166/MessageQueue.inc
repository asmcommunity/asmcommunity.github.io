;Homer's MessageQueue implementation
;16 November, 2006
;
;In the following document, the terms 'Message', 'Event' and 'Notification'
;are synonymous and can be freely exchanged.
;
;This object is designed for 'inter-object event notifications and servicing'...
;Arbitrary objects can 'register' for notifications of arbitrary events.
;Messages can be sent to a particular object, or to all objects
;which have registered their interest in receiving them.
;Messages can be marked with a future Time when they should be processed,
;and can be marked as PERMANENT (never to be dequeued).
;Notifications are served through user-supplied callback functions.
;These callback functions are Procedures with two parameters,
;or Methods with a single parameter (methods have a hidden parameter)
;
;Examples:
;PerformChore proc pThis, pMessage
;Method Bomb.Explode,,pMessage
;
;Note that Messages are only guaranteed to be valid for
;the duration of the associated callback function,
;that garbage collection is automatic,
;and that Messages are guaranteed to both be processed
;and for execution to complete in order.
;Finally, note that Permanent Messages can be considered
;as a kind of Virtual MachineCode, since they represent a series 
;of operations which will (always) be executed on your behalf
;via calls to code which YOU are providing... interesting, eh?

;This structure represents a Game Message
;(used to Notify of events, and/or to execute code asynchronous to Caller)
;NOTE : Bit 31 of the dType field is used to identify 'permanent messages'
;If this bit is set, the Message is NOT automatically removed from the Queue.
;NOTE : 
Message struct
	pFrom dd ?		;Message Sender				(Optional)
	pTo   dd ?		;Message Recipient			(NULL for broadcast)
	dDelay real4 ?	;Time to perform Delivery	(Optional)
	dType dd ?		;Message Type				(MANDATORY)
	dArg  dd ?		;Message Argument			(Optional)
Message ends

;This structure describes an Object's registration for event notifications..
;They are stored in a Collection on a per-MessageType basis, thus for
;a given Type of Message, we have a list of interested Objects and their callbacks.
Registration struct
	pWho 	dd ?	;ptr to an object which Registered for event notifications
	pWhat	dd ?	;ptr to the callback Handler for this event
Registration ends

;===============================================================
;The MessageQueue object represents a queue of pending Messages..
;Deriving from DataCollection, it uses the Inherited collection
;to store the actual Messages queue.
;A secondary collection is used to store a list of registered message types.
;This collection contains a number of DataCollection objects
;whose pOwner field indicates the MessageType, and which each
;contain a list of Registration structures.
MPID equ 88121
Object MessageQueue,MPID,DataCollection
	RedefineMethod Init,Pointer,dword,dword,dword
	RedefineMethod Done
	;Private methods
	Private
		StaticMethod FreeMessage,Pointer
		StaticMethod IsRegisteredType,dword	
		StaticMethod IsRegisteredObject,Pointer,Pointer
		StaticMethod ProcessMessageQueue
		StaticMethod ProcessMessage,Pointer
		StaticMethod MQWorker
	PrivateEnd
	;Public methods
	StaticMethod AllocMessage
	StaticMethod RegisterForMessageType,Pointer,dword,Pointer
	StaticMethod QueueMessage,Pointer
	StaticMethod DeQueueMessage,Pointer
	;Variables
	DefineVariable pMessyPool,Pointer,NULL
	DefineVariable pRegisteredMessageTypes,Pointer,NULL
	DefineVariable hWakeUpLazyWorker,HANDLE,NULL
	DefineVariable bKillWorker,BOOL,FALSE
	DefineVariable r8_AppTime,REAL8,0.0f
ObjectEnd

Method MessageQueue.Init,uses esi,pOwner,xx,yy,zz
LOCAL threadid
	SetObject esi
	;Create a Collection to hold Registered Message Types
	mov [esi].pRegisteredMessageTypes,$New(Collection,Init,pOwner,xx,yy,zz)	
;	DbgText "Created collection for MessageType groups"
	;Create a Pool of Message structs
	mov [esi].pMessyPool,$New(SyncDataPool,Init,esi,sizeof Message, sizeof Message*256)
;	DbgText "Created MessagePool"
	;Initialize the 'MessageQueue' collection
	ACall Init,pOwner,xx,yy,zz
;	DbgText "Initialized MessageQueue"
	;Create 'WakeUp' event
	mov [esi].hWakeUpLazyWorker,$invoke (CreateEvent,0,FALSE,FALSE,0)
	;Create Worker Thread
	lea ebx,threadid
	invoke CreateThread,0,0,$MethodAddr(MessageQueue.MQWorker),esi,0,ebx
	invoke CloseHandle,eax
MethodEnd

Method MessageQueue.Done,uses esi
	SetObject esi
	;Tell the Worker that it's time to quit
	mov [esi].bKillWorker,TRUE
	;Kick the Worker just in case he's sleeping
	invoke SetEvent,[esi].hWakeUpLazyWorker
	;Wait until the Worker responds to our quit notification
	.repeat
		invoke Sleep,200
	.until [esi].bKillWorker==FALSE
	;Release the sleepy worker event object
	invoke CloseHandle,[esi].hWakeUpLazyWorker
	;Release registrations
	Destroy [esi].pRegisteredMessageTypes
	;Release messagepool
	Destroy [esi].pMessyPool
	;Release messagequeue
	ACall Done
MethodEnd

;USER method for allocating new Message structs
;(usually recycled from the internal Pool)
Method MessageQueue.AllocMessage,uses esi
	SetObject esi
	OCall [esi].pMessyPool::SyncDataPool.NewItem	
MethodEnd

;PRIVATE method, sends old Message structs to the 'graveyard'
Method MessageQueue.FreeMessage,uses esi,pMessage
	SetObject esi
	OCall [esi].pMessyPool::SyncDataPool.FreeItem,pMessage
MethodEnd

;USER method for posting Messages to the Queue
Method MessageQueue.QueueMessage,uses esi,pMessage
	SetObject esi
	OCall Insert,pMessage
	;Trigger the 'WakeUp' event
	invoke SetEvent,[esi].hWakeUpLazyWorker
MethodEnd

;USER method, can be used to dequeue Permanent messages
Method MessageQueue.DeQueueMessage,uses esi,pMessage
	SetObject esi
	OCall Delete,pMessage		;remove from Queue
	OCall FreeMessage,pMessage	;return to Pool	
MethodEnd

;This method verifies that a given Message Type has been Registered..
;Returns ptr to new or existing DataCollection of Registration structs
Method MessageQueue.IsRegisteredType,uses esi edi,dMessageType
LOCAL cnt
	SetObject esi
;	DbgWarning "Checking for Registered MessageType"
	mov eax,[esi].pRegisteredMessageTypes
	m2m cnt,[eax].Collection.dCount
;	DbgDec cnt,"#Registered MessageTypes"
	mov edi,dMessageType
	;Search for Registered Type
	xor ecx,ecx
	.while ecx<cnt
		push ecx
;		DbgDec ecx,"current index"
		OCall [esi].pRegisteredMessageTypes::DataCollection.ItemAt,ecx
		.break .if eax==0
		.if [eax].DataCollection.pOwner==edi
			ExitMethod
		.else
			DbgDec [eax].DataCollection.pOwner,"this aint the right MessageType"
		.endif
		pop ecx
		inc ecx
	.endw
;	DbgWarning "Registering NEW MessageType"
	;MessageType is NOT Registered yet - do so now
	push $New(DataCollection)
	OCall eax::DataCollection.Init,dMessageType,16,256,-1
	pop eax
	OCall [esi].pRegisteredMessageTypes::Collection.Insert,eax	
MethodEnd

;This method checks whether an object has already Registered for an event
;Returns ptr to existing Registration, or NULL='not registered'
Method MessageQueue.IsRegisteredObject,uses esi,pRegistrations,pWho
LOCAL cnt
	SetObject esi
	mov eax,pRegistrations
	m2m cnt,[eax].DataCollection.dCount
	xor ecx,ecx
	.while ecx<cnt
		push ecx
		OCall pRegistrations::DataCollection.ItemAt,ecx
		mov ebx,pWho
		.if [eax].Registration.pWho==ebx
			pop ecx
			ExitMethod
		.endif
	.endw
	;Object has not yet been Registered
	xor eax,eax
MethodEnd

;This method registers an Object instance to receive
;messages of the given Type, and will also register the MessageType
;if it has not yet been registered.
;pWho = ptr to Object who wishes to register for event callbacks
;dWhat = id of notification
;pWhat = ptr to callback function
;Returns ptr to new or existing Registration struct
Method MessageQueue.RegisterForMessageType,uses esi,pWho,dWhat,pWhat
LOCAL pRegistrations,pRegistration
	SetObject esi
	;Ensure the MessageType has been registered with the MessageQueue
	mov pRegistrations,$OCall (IsRegisteredType,dWhat)
	;Ensure the object hasn't already registered for this event
	mov pRegistration,$OCall (IsRegisteredObject,pRegistrations,pWho)
	.if eax==NULL ;if object not already registered for this event..
		;Register the given object for receiving given MessageType
		push $MemAlloc(sizeof Registration)
		m2m [eax].Registration.pWho,pWho
		m2m [eax].Registration.pWhat,pWhat
		pop eax
		OCall pRegistrations::DataCollection.Insert,eax
	.endif
MethodEnd

;This method performs Message notification by calling
;the Handler associated with the target object or objects
Method MessageQueue.ProcessMessage,uses esi ecx, pMessage
LOCAL pRegistrations,cnt
	SetObject esi
	mov ebx,pMessage
	mov pRegistrations,$OCall (IsRegisteredType,[ebx].Message.dType)
;	DbgHex pRegistrations
	m2m cnt,[eax].DataCollection.dCount

	mov ebx,pMessage
	;Is there a Target object for this Message?
	.if [ebx].Message.pTo!=0
		;This Message is intended for a specific Target object,
		;but is the Target object Registered to receive it?
		OCall pRegistrations::DataCollection.IndexOf,[ebx].Message.pTo
		.if eax!=0
			;Yep - send to Target object
			push ebx						;pMessage
			push [ebx].Message.pTo			;pthis
			call [eax].Registration.pWhat	;call handler method or proc
		.endif
		
	.else

		;This is a 'Broadcast' Message - send to ALL associated objects				
		xor ecx,ecx
		.while ecx<cnt
			push ecx
			push ebx						;pMessage
			OCall pRegistrations::DataCollection.ItemAt,ecx
			push [eax].Registration.pWho
			call [eax].Registration.pWhat						
			pop ecx
			inc ecx
		.endw
	.endif
MethodEnd

;This method processes all pending Messages,
;servicing and dequeuing them appropriately.
Method MessageQueue.ProcessMessageQueue,uses esi
LOCAL pMessage
	SetObject esi
	xor ecx,ecx
	.while ecx<[esi].dCount
		push ecx
		mov pMessage,$OCall (ItemAt,ecx)
		.if [eax].Message.dDelay==0
			;Message is not 'Delayed', so let's 'do it' now!
;			DbgWarning "FIRE EVENT"
			jmp @F
		.else
			;Message is 'Delayed', so check if its time to 'do it'
			fld [esi].r8_AppTime
			fsub  [eax].Message.dDelay
			fstpReg ebx
			and ebx,80000000h
			.if ebx!=0
				;This message is officially OVERDUE for processing.
				;We're Late - do it NOW, or never!!
@@:				OCall ProcessMessage,pMessage
				;Now that the Message has been Dispatched,
				;we should remove it from the Queue..
				;but before we do that, let's check for permanence
				mov eax,pMessage
				mov eax,[eax].Message.dType
				and eax,80000000h
				.if eax==0
					;Message is not marked as Permanent
					;so we can discard it now :)
					OCall DeQueueMessage,pMessage
				.endif
			.endif
		.endif
		pop ecx
		inc ecx
	.endw
MethodEnd

;This is our WORKER THREAD which services the MessageQueue..
Method MessageQueue.MQWorker,uses esi
	SetObject esi	
	invoke SuperTimer,TIMER_START
	fstp [esi].r8_AppTime
;	DbgText "MessageQueue Worker thread created"
;	DbgHex esi
	.while [esi].bKillWorker==FALSE		
		;If theres any Messages, process them immediately
		.if [esi].dCount!=0
			invoke SuperTimer,TIMER_GETAPPTIME
			fstp [esi].r8_AppTime
			OCall ProcessMessageQueue
		.else
			;No messages are queued, so Sleep until there is work to do
			invoke WaitForSingleObject,[esi].hWakeUpLazyWorker,INFINITE
		.endif
	.endw
	mov [esi].bKillWorker,FALSE
;	DbgText "MessageQueue Worker thread destroyed"
	invoke ExitThread,0
MethodEnd
