			.model small
 			.386
			.data	               ;start of data segment
	saddr		dd	?	       ;old stack area
	sarea		dw	1000h dup (?)  ;new stack area

	stop		equ	this word      ;define to of new stack

			.code		       ;start of code segment
			.startup	       ;start of program

			cli		       ;disable interrupts

			mov	ax,sp          ;save old sp
			mov	word ptr saddr,ax	
			mov 	ax,ss          ;save old ss
			mov 	word ptr saddr+2,ax

			mov	ax,ds          ;load new ss
			mov	ss,ax
			mov 	ax,offset stop ;load new sp
			mov	sp,ax

			sti		       ;enable interrupts

			mov	ax,ax          ;do dummy instructions
			mov	ax,ax
			lss	sp,saddr       ;load old ss and sp

			.exit		       ;exit to dos
			end		       ;end of file
