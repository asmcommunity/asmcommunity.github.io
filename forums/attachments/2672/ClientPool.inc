
if IMPLEMENT
;——————————————————————————————————————————————————————————————————————————————————————————————————

Method ClientPool.Done,uses esi
	SetObject esi
	DbgDec [esi].Used.dCount, "ClientPool is Done"
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
	OCall pClientToFree::Client.Goodbye		;Release client resources
	ACall Free,pClientToFree                ;Recycle the Client object
MethodEnd
endif