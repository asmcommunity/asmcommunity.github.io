ifndef __POOL__
	__POOL equ 1
;——————————————————————————————————————————————————————————————————————————————————————————————————
;This class manages a Pool of arbitrary objects.

;Pooled objects are stored in 'Used' and 'Free' collections,
;which makes it possible to "recycle" objects within the Pool
;without destroying and recreating them and their resources.

;This was written as a BASE CLASS for the NetEngine.
;Two derived classes are built on top of this one.
;Their names are IOJobPool and ClientPool.

;PLEASE NOTE:
;The CreatePooledObject method should be redefined or overridden
;by anything which implements Pooled objects.
;——————————————————————————————————————————————————————————————————————————————————————————————————
PoolID equ 19192
Object Pool, PoolID, Primer
	RedefineMethod	Init,				Pointer,dword		;-> PoolOwner, maxFreeItems
	RedefineMethod	Done
	VirtualMethod	Allocate
	VirtualMethod	Free,				Pointer   
	VirtualMethod	IsActive,			Pointer
	DynamicAbstract	CreatePooledObject
	Embed Used,		Collection
	Embed Available,Collection
	DefineVariable maxFreeItems, dword, 256

ObjectEnd



;——————————————————————————————————————————————————————————————————————————————————————————————————
if IMPLEMENT
Method Pool.Init, uses esi, pOwner, maxFreeItems
    SetObject esi
    ACall Init,pOwner
    m2m [esi].maxFreeItems,maxFreeItems
    OCall [esi].Used::Collection.Init,		pOwner,10,100,maxFreeItems
    OCall [esi].Available::Collection.Init,	pOwner,10,100,maxFreeItems    
MethodEnd

Method Pool.Done, uses esi
    SetObject esi
    OCall [esi].Used::Collection.Done
    OCall [esi].Available::Collection.Done
MethodEnd

Method Pool.IsActive,uses esi,pObject
	SetObject esi
	OCall [esi].Used::Collection.IndexOf,pObject
	.if eax==-1
		mov eax,FALSE
	.else
		mov eax,TRUE
	.endif
MethodEnd

;This method acquires a resource object, preferably recycled
;BUT DOES NOT INITIALIZE IT
Method Pool.Allocate,uses esi
LOCAL pNewItem

   SetObject esi

	;Is there room in the Used Collection?
	mov eax,[esi].Used.dCount
	.if eax==[esi].maxFreeItems
		xor eax,eax
		ExitMethod
	.endif
	
   ;Are there any Free items?
   .if [esi].Available.dCount>0   
      ;Fetch the last item from the Free Collection
      mov eax,[esi].Available.dCount
      dec eax  ;<-- last item's index = (#items - 1)     
      mov pNewItem,$OCall ([esi].Available::Collection.ItemAt, eax)
      ;Merely Remove reference from list, don't destroy item
      OCall [esi].Available::Collection.Delete,eax
   .else
      ;Create a new item      
      mov pNewItem,$OCall (CreatePooledObject)
   .endif
	;Add new item to Used
    SetObject esi
	OCall [esi].Used::Collection.Insert,pNewItem
	
	mov eax,pNewItem
   
MethodEnd

;Try to move an IOJob onto the Free collection.
;If theres too many Free we'll destroy it.
Method Pool.Free, uses esi, pPooledObject
   .if pPooledObject!=0
     ;Remove IOJob from Used Collection without Destroying it
     SetObject esi
     OCall [esi].Used::Collection.Delete,pPooledObject
     mov eax,[esi].Available.dCount
     .if eax<[esi].maxFreeItems
	    OCall [esi].Available::Collection.Insert, pPooledObject
     .else   
     	Destroy pPooledObject
     .endif

  .endif
MethodEnd

endif
endif