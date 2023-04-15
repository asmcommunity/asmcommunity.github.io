ifndef Vec2
    Vec2 struct
        x real4 ?
        y real4 ?
    Vec2 ends
endif
ifndef Vec3
    Vec3 struct
        x real4 ?
        y real4 ?
        z real4 ?
    Vec3 ends
    D3DXVECTOR3 typedef Vec3
endif

.data
r4_SizeOfFood      real4 5.0f
r4_MaxPerturbation real4 0.3f
r4_MaxTurnRate     real4 0.3f
iNumElite       equ 1               ;#fittest critters to retain across generations
iNumCopiesElite equ 1               ;#copies of fittest critters 
.code

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; A couple of macros for verifying/recording method input params
MustBeProvided macro target,source
    .if source==NULL
        %DbgWarning "Error - &source& not provided"
        int 3
    .endif
    GrabParam target, source
endm

GrabParam macro target, source
    m2m target,source,edx
endm

;All the entities in this simulation derive from this template.
AIBOID equ 234323
Object AIBaseObject,AIBOID,Primer
    DefineVariable vPosition, Vec3, {<>}
ObjectEnd


; ——————————————————————————————————————————————————————————————————————————————————————————————————
;This class describes a simulated instance of a plant.
;Plants represent food for herbivorous creatures.

PlantID equ 2354230
Object Plant,PlantID,AIBaseObject
    RedefineMethod Init,real4,real4
ObjectEnd

Method Plant.Init,uses esi,fX:real4,fZ:real4
    SetObject esi
    GrabParam [esi].vPosition.x,fX
    GrabParam [esi].vPosition.z,fZ
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
;This class describes a simulated instance of a Creature.
;The AI is implemented as a simple Neural Network.
AICID equ 2354234
Object AICreature,AICID,AIBaseObject
    RedefineMethod Init,            Pointer,Pointer,Pointer
    RedefineMethod Done
    VirtualMethod GetClosestFood,   Pointer
    VirtualMethod Update,           Pointer
    DefineVariable vLookAt,         Vec3,{<0.0f,0.0f,1.0f>}
    DefineVariable dFitness,        dword,NULL
    DefineVariable r4_TurnLeft,     real4,0.16
    DefineVariable r4_TurnRight,    real4,0.16
    DefineVariable r4_Rotation,     real4,0.0
    DefineVariable r4_Speed,        real4,5.0
    DefineVariable Vec2_WorldSize,  Vec2,{<500.0f,500.0f>}
    Embed Brain,NNet
ObjectEnd

Method AICreature.Init,uses esi,pNetInfo,pRNG,pvWorldSize
    SetObject esi
    OCall [esi].Brain::NNet.Init,0,pNetInfo, pRNG
    mov eax,pvWorldSize
    GrabParam [esi].Vec2_WorldSize.x,[eax].Vec2.x
    GrabParam [esi].Vec2_WorldSize.y,[eax].Vec2.y
MethodEnd

Method AICreature.Done,uses esi
    SetObject esi
    DbgText "AICreature.Done"
    OCall [esi].Brain::NNet.Done    ;Destroy critter's brain (hehehe)
MethodEnd

;Find closest food to this creature.
;Returns eax=ptr to closest creature
;        st2 = distance
;        st1,st0 = Vec2 vector from creature to food
Method AICreature.GetClosestFood,uses esi edi ebx,pFood
LOCAL fDistance,fClosest
LOCAL tempx,tempy
LOCAL vClosest:Vec2, pClosest
local pFoodItem

    SetObject esi
    fmov fClosest,9999999.9
    mov edi,pFood
    xor ebx,ebx
    .while ebx<[edi].Collection.dCount
        ;Get ptr to next Food item
        mov pFoodItem,$OCall (edi::Collection.ItemAt,ebx)
        lea edx,[eax].Plant.vPosition
    ;    DbgVec3 edx,"FOOD POSITION"
    ;    DbgVec3 [esi].vPosition,"CRITTER POSITION"
        
        ;Calculate Direction and Distance
        fld  [esi].vPosition.x
        fsub [eax].Plant.vPosition.x
        fst tempx
;        ;Let AI see food across World Bounds
;        .if $IsNegative(tempx)==TRUE
;            fadd [esi].Vec2_WorldSize.x
;            fst tempx 
;        .elseif $IsGreater(tempx,[esi].Vec2_WorldSize.x)==TRUE
;            fsub [esi].Vec2_WorldSize.x
;            fst tempx
;        .endif
        fmul st(0),st(0)
        ;
        mov eax,pFoodItem
        fld [esi].vPosition.z
        fsub [eax].Plant.vPosition.z
        fst tempy
;        .if $IsNegative(tempy)==TRUE
;            fadd [esi].Vec2_WorldSize.y
;            fst tempy
;        .elseif $IsGreater(tempy,[esi].Vec2_WorldSize.y)==TRUE
;            fsub [esi].Vec2_WorldSize.y
;            fst tempy
;        .endif
        fmul st(0),st(0)
        ;
		fadd
		fsqrt
		fstp fDistance
	;	DbgFloat fDistance

        ;Is this the shortest distance so far?
		.if $IsLess(fDistance , fClosest)==TRUE
	;    DbgFloat fDistance,"Is closer than"
	;    DbgFloat fClosest
			m2m fClosest, fDistance,edx
			m2m vClosest.x,tempx,edx
			m2m vClosest.y,tempy,edx
			m2m pClosest,pFoodItem,edx
	;.else
	;    DbgFloat fDistance,"is bigger than"
	;    DbgFloat fClosest
		.endif

        inc ebx
	.endw

	.if $IsLess(fDistance,r4_SizeOfFood)==TRUE
	    DbgText "HIT","HitWindow"
	    mov eax,pClosest

	.else
        fld fDistance
        fld vClosest.x
        fdiv st(0),st(1)    ;normalize direction vector
        fld vClosest.y
        fdiv st(0),st(2)
        xor eax,eax
	.endif


MethodEnd



Method AICreature.Reset,uses esi
    SetObject esi
    
	;Reset the  positions
	OCall [esi].Brain.random::RNG.Real_Unsigned_Normal
	fmul [esi].Vec2_WorldSize.x
	fstp [esi].vPosition.x
	OCall [esi].Brain.random::RNG.Real_Unsigned_Normal
	fmul [esi].Vec2_WorldSize.y
	fstp [esi].vPosition.z
	
	;and the fitness
	mov [esi].dFitness , 0

    ;and the rotation
    OCall [esi].Brain.random::RNG.Real_Unsigned_Normal
    fldpi
    fadd st(0),st(0)
    fmul
    fstp [esi].r4_Rotation 

MethodEnd

;First we take sensor readings and feed these into the sweepers brain.
;The inputs are:
;-A vector to the closest food (x,y)
;-The creature's 'look at' vector (x,y)
;We receive two outputs from the brain.. fTurnLeft and fTurnRight.
;So given a force for each track we calculate the resultant rotation 
;and acceleration and apply to current velocity vector.
Method AICreature.Update,uses esi edi,pFood
LOCAL inputs [2] : Vec2
LOCAL pClosestFood,fDistance,fRotForce,temp
    SetObject esi

	;get info about closest food (direction, distance etc)
	mov pClosestFood,$OCall (esi.GetClosestFood,pFood)
	
	;store info.. NNet input #1 of 2 = vector from creature to food
	lea edi,inputs
	fstp [edi].Vec2.y  
	fstp [edi].Vec2.x
	fstp fDistance
	
    ;NNet input #2 of 2 = Creature's LookAt vector
    add edi,sizeof Vec2
    m2m [edi].Vec2.x, [esi].vLookAt.x, edx
    m2m [edi].Vec2.y, [esi].vLookAt.z, edx


	;update the brain and get feedback
	OCall [esi].Brain::NNet.run,addr inputs

	;assign the outputs to the left & right tracks
	fld real8 ptr[eax]
	fst [esi].r4_TurnLeft
	fld real8 ptr[eax+sizeof real8]
	fst [esi].r4_TurnRight

	;calculate steering forces
	fsub
	fstp fRotForce

	;clamp rotation between "+/- r4_MaxTurnRate"
	fld r4_MaxTurnRate
	fchs
	fstp temp
	.if $IsLess(fRotForce,temp)
	    m2m fRotForce,temp,edx
	.elseif $IsGreater(fRotForce,r4_MaxTurnRate)
	    m2m fRotForce,r4_MaxTurnRate,edx
	.endif
	
	;DbgFloat fRotForce

    ;update rotation
    fld fRotForce
    fadd [esi].r4_Rotation
    fstp [esi].r4_Rotation
    fldpi
    fadd st(0),st(0)
    fstp temp
	
	fld [esi].r4_TurnLeft
	fadd [esi].r4_TurnRight
	fstp [esi].r4_Speed	
	;DbgFloat [esi].r4_Speed

	;update LookAt vector
	fld [esi].r4_Rotation
	fsincos
	fstp [esi].vLookAt.x
	fstp [esi].vLookAt.z


	;update position, allow creature to cross world boundaries
	fld  [esi].vLookAt.x 
	fmul [esi].r4_Speed
	fadd [esi].vPosition.x
	fst temp
    .if $IsNegative(temp)==TRUE
        fadd [esi].Vec2_WorldSize.x
    .elseif $IsGreater(temp,[esi].Vec2_WorldSize.x)
        fsub [esi].Vec2_WorldSize.x
    .endif	
	fstp [esi].vPosition.x
	
	fld  [esi].vLookAt.z
	fmul [esi].r4_Speed
	fadd [esi].vPosition.z
	fst temp
    .if $IsNegative(temp)==TRUE
        fadd [esi].Vec2_WorldSize.y
    .elseif $IsGreater(temp,[esi].Vec2_WorldSize.y)
        fsub [esi].Vec2_WorldSize.y
    .endif	
	fstp [esi].vPosition.z

	mov eax,pClosestFood

MethodEnd eax

; ——————————————————————————————————————————————————————————————————————————————————————————————————

;This class applies a genetic algorithm to evolve
;a population of 'critters' (NNet-based AI instances).
;The AI 'learn' through being rewarded for achieving
;their 'Prime Directive', and punished for failing.
;Unscripted behaviors naturally evolve as a result
;of this 'machine learning' mechanism.
GAICID equ 345332
Object GAIController,GAICID,Primer

	RedefineMethod Init,Pointer,dword,dword,dword,dword,real4,real4,real4,real4 
	RedefineMethod Done
	VirtualMethod  Update,Pointer,Pointer

	VirtualMethod Crossover,Pointer,Pointer,Pointer,Pointer
	VirtualMethod Mutate,Pointer,dword
	VirtualMethod GetChromoRoulette,Pointer

	;use to introduce elitism
	VirtualMethod GrabFittest,Pointer
    VirtualMethod GrabNFittest,dword,dword,Pointer,Pointer 

	VirtualMethod Reset


	;this runs the GA for one generation.
	VirtualMethod Epoch,Pointer ;pOldPopulation

    DefineVariable Vec2_WorldSize,Vec2,{<500.0f, 500.0f>}   ;world dimensions
    DefineVariable dTicks,dword,0                           ;#cycles so far this generation
    DefineVariable dCyclesPerGeneration,dword,2000          ;cycles per generation
	DefineVariable dGenerations,dword,0                     ;Generation counter
	DefineVariable dTotalFitness,dword,0                    ;Total fitness of population this generation

	DefineVariable r4_MutationRate, real4,0.15	            ;probability that a chromosones bits will mutate... Try figures around 0.05 to 0.3 ish
	DefineVariable r4_CrossoverRate,real4,0.7f              ;probability of chromosones crossing over bits... 0.7 works pretty good
	
	DefineVariable NNetInfo,NNINFO,  {<4,2,1,6>} ;#inputs,#outputs,#hiddenlayers,#neurons per hidden layer
    DefineVariable pRNG,Pointer,NULL             ;Random Generator
	
	DefineVariable pHerbivores,Pointer,NULL   ;Vegetable-eating creatures
	Embed Plants,    Collection   ;Food for the vegetarians
ObjectEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————

Method GAIController.Done,uses esi
    SetObject esi
    DbgText "GAIController.Done is destroying population"
    Destroy [esi].pHerbivores
    DbgText "GAIController.Done is destroying food"
    OCall [esi].Plants::Collection.Done
    ACall Done
MethodEnd

;Constructor method..
;Initializes everything.
Method GAIController.Init,uses esi edi ebx, pRNG,dNumHerbivores,dNumPlants,\ 
                                    dCyclesPerGeneration,dGenerations,\
                                    fWorldWidth:real4, fWorldHeight:real4,\
                                    MutationRate:real4,CrossRate:real4
    SetObject esi
    
    MustBeProvided [esi].pRNG, pRNG
    GrabParam [esi].Vec2_WorldSize.x,fWorldWidth
    GrabParam [esi].Vec2_WorldSize.y,fWorldHeight
    GrabParam [esi].dCyclesPerGeneration, dCyclesPerGeneration
    GrabParam [esi].dGenerations,dGenerations
    GrabParam [esi].r4_MutationRate,  MutationRate
    GrabParam [esi].r4_CrossoverRate, CrossRate
    
    mov [esi].pHerbivores,$New(Collection,Init,0,16,256,-1)
    OCall [esi].Plants    ::Collection.Init,0,16,256,-1

    ;let's create some random critters
	xor ebx,ebx
	.while ebx<dNumHerbivores
	    push ebx
	    OCall [esi].pHerbivores::Collection.Insert,$New(AICreature,Init,addr [esi].NNetInfo,[esi].pRNG,addr [esi].Vec2_WorldSize)
	    mov edi,eax
	    OCall [edi].AICreature.Brain::NNet.initRandom ;Brainwash the new critter
	    pop ebx
	    inc ebx
	.endw
	
	;create some random plants (for the critters to eat)
	xor ebx,ebx
	.while ebx<dNumPlants
	    OCall pRNG::RNG.Real_Ranged,0,fWorldWidth
	    OCall pRNG::RNG.Real_Ranged,0,fWorldHeight
	    fstpReg edx
	    fstpReg eax
	    OCall [esi].Plants::Collection.Insert,$New(Plant,Init,eax,edx)
	    inc ebx
	.endw

MethodEnd

;This is the main workhorse for genetic simulation.
;The comments should explain what is going on adequately.
Method GAIController.Update,uses esi edi ebx,ppPopulation,pFood
LOCAL pCritter,GrabHit,pClosestFood
    SetObject esi
	;Run the Creature Simulator through a prescribed # of cycles. 
	;During this loop each critter's NN is constantly updated with the appropriate
    ;information from its surroundings. The output from the NN is obtained
    ;and the critter is moved. If it encounters food, its fitness is incremented.
    mov eax,[esi].dTicks
	.if eax < [esi].dCyclesPerGeneration
	    ;increment cycles counter
        inc [esi].dTicks
	    DbgDec [esi].dTicks
        ;For each Critter in the input Population:
		xor ebx,ebx
		mov edi,ppPopulation
		mov edi,[edi]
		.while ebx<[edi].Collection.dCount
		    ;Obtain ptr to critter
		    mov pCritter,$OCall (edi::Collection.ItemAt,ebx)

			;update the critter's Brain and Position
			mov pClosestFood,$OCall (eax::AICreature.Update,pFood)
			.if eax != 0
			    ;WE HAVE A HIT
			    ;Increase its fitness
			    mov eax,pCritter
			    inc [eax].AICreature.dFitness
			    ;Pick a new random position for the plant
			    push eax
			    OCall [eax].AICreature.Brain.random::RNG.Real_Signed_Normal
			    fmul [esi].Vec2_WorldSize.x
			    pop eax
			    OCall [eax].AICreature.Brain.random::RNG.Real_Signed_Normal
			    mov eax,pClosestFood
			    fstp [eax].AIBaseObject.vPosition.z
			    fstp [eax].AIBaseObject.vPosition.x
			.endif
            inc ebx
		.endw

	;Another generation has been completed.
	;Time to run the Genetic Algorithm
	.else
		DbgWarning "Epoch"
		inc [esi].dGenerations           ;increment the generation counter
		OCall esi.Epoch, ppPopulation ;run the GA to create a new population
		
		mov eax,[esi].pHerbivores
		DbgDec [eax].Collection.dCount

        ;If we did that correctly, the population should
        ;now all have their fitness set to zero		

    .endif
MethodEnd

;Resets all the relevant variables ready for a new generation
Method GAIController.Reset,uses esi
    SetObject esi
	mov [esi].dTotalFitness     , 0
	mov [esi].dTicks , 0
MethodEnd

;mutates a chromosome by perturbing its weights
Method GAIController.Mutate,uses esi edi ebx,pWeights,dWeights
local fRandom:real4
    SetObject esi
    mov edi,pWeights
    DbgHex pWeights
    DbgDec dWeights
	;traverse the chromosome and mutate each weight dependent on the mutation rate
	xor ebx,ebx
	.while ebx<dWeights
		;do we perturb this weight?
		OCall [esi].pRNG::RNG.Real_Unsigned_Normal
		fstp fRandom
		.if $IsLess(fRandom,[esi].r4_MutationRate)==TRUE
			;add or subtract a small value to the weight
			OCall [esi].pRNG::RNG.Real_Signed_Normal 
			fmul r4_MaxPerturbation
			fadd real8 ptr[edi]
			fstp real8 ptr[edi]
			add edi,sizeof real8
		.endif
		inc ebx
	.endw
MethodEnd

;Return a random critter via a fitness-based heuristic
Method GAIController.GetChromoRoulette,uses esi edi ebx, pPopulation
LOCAL Slice, FitnessSoFar
    SetObject esi
    
	;generate a random number between 0 & total fitness count
	mov Slice,$OCall ([esi].pRNG::RNG.Int_Ranged,0,[esi].dTotalFitness)

	;go through the chromosones adding up the fitness so far
	mov FitnessSoFar ,0
	
	mov edi,pPopulation
	xor ebx,ebx
	.while ebx<[edi].Collection.dCount
	    OCall edi::Collection.ItemAt,ebx
	    mov edx,[eax].AICreature.dFitness
	    add FitnessSoFar,edx
		
		;if the fitness so far > random number, return this critter
		mov edx,FitnessSoFar
		.if edx >= Slice
		    OCall edi::Collection.DeleteAt,ebx
            .break
        .endif
        inc ebx
		
	.endw

MethodEnd eax
	
;given parents and storage for the offspring,
;this method performs 'genetic crossover'
;according to the 'crossover rate'.
;pmum,   pdad     = input AICreatures
;ppbaby1, ppbaby2 = placeholders for output AICreatures 
Method GAIController.Crossover,uses esi edi ebx,pmum,pdad,ppbaby1,ppbaby2
local fRand:real4,Rnd:dword
    SetObject esi
    
	OCall [esi].pRNG::RNG.Real_Unsigned_Normal
	fstp fRand
	
	;if parents are the same, or if crossover probability dictates,
	;then we will just return the parent(s)
	mov edx,pmum
	.if edx == pdad
	    mov edx,ppbaby1
	    m2m [edx],pmum,eax
	.elseif $IsGreater(fRand, [esi].r4_CrossoverRate)==TRUE 
	    mov edx,ppbaby1
	    m2m [edx],pmum,eax
	    mov edx,ppbaby2
	    m2m [edx],pdad,eax
		ret
	.endif

	;determine a crossover point
	mov eax,pmum
	mov eax,[eax].AICreature.Brain.nTotalWeights
	dec eax
	mov Rnd,$OCall ([esi].pRNG::RNG.Int_Ranged,0, eax)

	;create the offspring
	mov edi,ppbaby1
	mov [edi],$New(AICreature,Init,addr [esi].NNetInfo,[esi].pRNG,addr [esi].Vec2_WorldSize)
	mov edi,ppbaby2
	mov [edi],$New(AICreature,Init,addr [esi].NNetInfo,[esi].pRNG,addr [esi].Vec2_WorldSize)
	
	;Copy some Neuron Weights .. mum->baby1, dad->baby2
	xor ebx,ebx
	.while ebx<Rnd
        mov edx,pmum
        mov edx,[edx].AICreature.Brain.lpWeightArr
        fld real8 ptr [edx+ebx*sizeof real8]
        mov edx,ppbaby1
        mov edx,[edx]
        mov edx,[edx].AICreature.Brain.lpWeightArr
        fstp real8 ptr [edx+ebx*sizeof real8] 
        mov edx,pdad
        mov edx,[edx].AICreature.Brain.lpWeightArr
        fld real8 ptr [edx+ebx*sizeof real8]
        mov edx,ppbaby2
        mov edx,[edx]
        mov edx,[edx].AICreature.Brain.lpWeightArr
        fstp real8 ptr [edx+ebx*sizeof real8] 
	    inc ebx
	.endw

    ;Copy remaining Weights .. dad->baby1, mum->baby2
    mov edi,pmum
	.while ebx<[edi].AICreature.Brain.nTotalWeights
        mov edx,pdad
        mov edx,[edx].AICreature.Brain.lpWeightArr
        fld real8 ptr [edx+ebx*sizeof real8]
        mov edx,ppbaby1
        mov edx,[edx]
        mov edx,[edx].AICreature.Brain.lpWeightArr
        fstp real8 ptr [edx+ebx*sizeof real8] 
        mov edx,pmum
        mov edx,[edx].AICreature.Brain.lpWeightArr
        fld real8 ptr [edx+ebx*sizeof real8]
        mov edx,ppbaby2
        mov edx,[edx]
        mov edx,[edx].AICreature.Brain.lpWeightArr
        fstp real8 ptr [edx+ebx*sizeof real8] 
	    inc ebx
	.endw
MethodEnd

;Applies the Genetic Algorithm to the given population.
;Replaces input population with new population.
Method GAIController.Epoch,uses esi edi ebx,ppOldPopulation
LOCAL cnt,pmum,pdad,pbaby1,pbaby2,pNewPop
    SetObject esi
    
    mov eax,ppOldPopulation
    mov eax,[eax]
    m2m cnt,[eax].Collection.dCount,edx


    ;reset the appropriate variables
    OCall esi.Reset
    
    ;If theres only one critter, it cant 'breed' in the conventional sense,
    ;we cant apply Crossover, but we CAN mutate the genes :)
	.if cnt==1
        mov eax,ppOldPopulation
        mov eax,[eax] 
        mov eax,[eax].Collection.pItems
        mov eax,[eax]
	    OCall esi.Mutate,[eax].AICreature.Brain.lpWeightArr,[eax].AICreature.Brain.nTotalWeights
	    ret
	.endif
	
    ;create an output population
	mov pNewPop,$New(Collection,Init,0,16,256,-1)
	
    ;ELITISM
	; Now to add a little elitism we shall add in some copies of the fittest genomes.
	mov eax,iNumCopiesElite
	mov edx,iNumElite
	mul edx
	.if eax<=cnt
	    sub cnt,eax
        mov eax,ppOldPopulation
        OCall esi.GrabNFittest,iNumElite,  iNumCopiesElite, dword ptr [eax], pNewPop
	.endif
	;Make sure the remaining quota of creatures we require is EVEN,
	;otherwise the Roulette Wheel sampling will crash.
    .if cnt!=0 
        mov eax,cnt
        and eax,1
        .if eax==1
            ;argh, its odd, we better grab one more
    	    sub cnt,eax
            mov eax,ppOldPopulation
            OCall esi.GrabNFittest,1,  1, dword ptr [eax], pNewPop
        .endif
    .endif

    ;GENETIC ALGORITHM
	;Select two members of the population which are 'reasonably fit'.
	;Apply the Crossover function to them,
	;and then Mutate the resulting pair of Children.
	;Insert the new Children into the output population.
	;Repeat until a new population is generated
	mov edi,pNewPop
	mov eax,cnt
	.while cnt!=0
		;grab two chromosomes
		mov ebx,ppOldPopulation
		mov pmum,$OCall (esi.GetChromoRoulette,dword ptr[ebx])
		mov pdad,$OCall (esi.GetChromoRoulette,dword ptr[ebx])

		;create some offspring via crossover
		OCall esi.Crossover,pmum, pdad, addr pbaby1, addr pbaby2

		;now we mutate
		mov eax,pbaby1
		OCall esi.Mutate,[eax].AICreature.Brain.lpWeightArr,[eax].AICreature.Brain.nTotalWeights
		mov eax,pbaby2
		OCall esi.Mutate,[eax].AICreature.Brain.lpWeightArr,[eax].AICreature.Brain.nTotalWeights

		;now copy into new population
		OCall pNewPop::Collection.Insert,pbaby1
		OCall pNewPop::Collection.Insert,pbaby2
		sub cnt,2
	.endw

	;Zap the old population, and replace with new population.
	mov edi,ppOldPopulation
	Destroy dword ptr [edi]
	m2m dword ptr[edi],pNewPop,edx

MethodEnd


;Find fittest member of given population,
;clone its Neural Network weights into a new critter,
;return new critter
Method GAIController.GrabFittest,uses esi edi ebx,pPopulation
LOCAL dBest,pBest
    DbgText "GrabFittest"
    mov dBest,0
    mov pBest,0
    SetObject esi
    mov edi,pPopulation
    xor ebx,ebx
    .while ebx<[edi].Collection.dCount
        OCall pPopulation::Collection.ItemAt,ebx
        mov edx,[eax].AICreature.dFitness
        .if edx>dBest
            mov pBest,eax
            mov dBest,edx
        .endif
        inc ebx
    .endw

	push $New (AICreature,Init,addr [esi].NNetInfo,[esi].pRNG,addr [esi].Vec2_WorldSize)
    mov edi,pBest
    invoke RtlMoveMemory,[eax].AICreature.Brain.lpWeightArr,[edi].AICreature.Brain.lpWeightArr,[edi].AICreature.Brain.lnWeightArr
    pop eax
    mov [eax].AICreature.dFitness,0 ;reset fitness 
MethodEnd eax


;This works like an advanced form of elitism by inserting
;X copies of the Y most fittest critters into the new population
Method GAIController.GrabNFittest,uses esi ebx, NBest,NumCopies,pOldPopulation,pNewPopulation
LOCAL pBest
    SetObject esi
    ;add the required amount of copies of the n most fittest 
    xor ebx,ebx
	.while ebx<NBest
	    push ebx
	    mov pBest,$OCall (esi.GrabFittest,pOldPopulation)
		xor ebx,ebx
		.while ebx<NumCopies
		    OCall pNewPopulation::Collection.Insert,pBest
		    inc ebx
	    .endw
	    pop ebx
	    inc ebx
	.endw
MethodEnd




