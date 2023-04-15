ClientPoolID equ 45219
Object ClientPool,ClientPoolID,Pool
   RedefineMethod Done
   RedefineMethod CreatePooledObject
   RedefineMethod Free,Pointer
   ;-------------
ObjectEnd
if IMPLEMENT
;——————————————————————————————————————————————————————————————————————————————————————————————————

Method ClientPool.Done,uses esi
	SetObject esi
	DbgWarning "clearing clientpool"
	ACall Done
MethodEnd

;Called by the Ancestor class (Pool),
;this method creates a Derived object (Client)
;BUT DOES NOT INITIALIZE IT
Method ClientPool.CreatePooledObject,uses esi
	 New Client
MethodEnd	

Method ClientPool.Free,uses esi,pClientToFree
	SetObject esi

	;Closing the Socket will cause any pending IOJobs to fail
	;but we'll handle that from our Worker thread.
	mov eax,pClientToFree
	invoke closesocket,[eax].Client.hSocket
	;Invalidate the Socket to force socket recreation
	mov eax,pClientToFree
	mov [eax].Client.hSocket,INVALID_SOCKET	
	;Recycle the Client object
	ACall Free,pClientToFree

MethodEnd
endif