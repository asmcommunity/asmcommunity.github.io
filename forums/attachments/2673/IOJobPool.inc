

IOJobPoolID equ 91919
Object IOJobPool, IOJobPoolID, Pool
   RedefineMethod Init, Pointer, dword, dword	
   RedefineMethod Done	
   RedefineMethod CreatePooledObject
   RedefineMethod Allocate,Pointer					;->pNewOwner
   ;-------------
   DefineVariable bufferSize, dword
ObjectEnd

;——————————————————————————————————————————————————————————————————————————————————————————————————
if IMPLEMENT

Method IOJobPool.Done,uses esi
	SetObject esi
	DbgWarning "Draining Job Pool"
	ACall Done
MethodEnd

Method IOJobPool.Init, uses esi, pOwner, maxFreeJobs,bufferSize
    SetObject esi
    m2m [esi].bufferSize,bufferSize
    ACall esi.Init,pOwner,maxFreeJobs  
MethodEnd

Method IOJobPool.CreatePooledObject,uses esi
	;Create an IOJob
	DbgWarning "IOJObPool creates new IOJob"
	New IOJob
MethodEnd	

Method IOJobPool.Allocate,uses esi,pNewOwner
	SetObject esi
	ACall esi.Allocate
	mov [eax].IOJob.xovl.operation,OPERATION_DOACCEPT	;Set default operation
	push eax
	OCall eax::IOJob.Init,pNewOwner,[esi].bufferSize	;Set IOJob's Owner
	pop eax
MethodEnd
endif

