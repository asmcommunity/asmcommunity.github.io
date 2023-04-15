.586
.model flat, stdcall
option casemap :none

include windows.inc
include kernel32.inc
include masm32.inc

includelib kernel32.lib
includelib masm32.lib

Factorial		proto	:real8
DoubleFactorial	proto	:real8
ustr2dw			proto	:DWORD

.data
	RunsDone	real8	0.0
	Two			real8	2.0
.data?
	InBuffer	dword	?
	InBuffLen	dword	?
	NumRuns		real8	?
	PI			real8	?
	Fact		real8	?
	DblFact		real8	?
	PIOut		CHAR	?
	Status		word	?
	DBLFACTIN	real8	?
	TEMPRUNS	real8	?
	CommandLine	dword	?
.code
start:
	;TODO, make command args
	invoke	GetCL,1,addr CommandLine
	mov		CommandLine,eax
	invoke	atodw,addr CommandLine
	fild	CommandLine
	fst		NumRuns	
	
	;Pi = 2 * SUM_LOOP(factorial(n) / doublefactorial(2 * k + 1)
	;BEGIN GENERATING PI NOW
	PI_LOOP:
	invoke	Factorial,RunsDone	;Factorial n where n = number of runs completed
								;now ST = Factorial(RunsDone)
	fst		Fact
	
	fld		RunsDone		;push
	fst		TEMPRUNS		;pop
	
	fld		RunsDone			;ST(2)
	fld1						;ST(1)
	fld		Two					;ST
	fmul	ST,ST(2)			;2*RunsDone	= ST
	fadd	ST,ST(1)			;+1			= ST
	fst		RunsDone			;RunsDone is now effectively 2k+1 tor calling the doublefactorial
	invoke	DoubleFactorial,RunsDone	;DoubleFactorial(2*n+1)
	fst		DblFact						;ST = result
	
	fld		TEMPRUNS		;push
	fst		RunsDone		;pop
	
	;Adding running total to new calculated version
	fld		PI		;ST(2)
	fld		DblFact	;ST(1)
	fld		Fact	;ST
	fdiv	ST,ST(1);Fact/DblFact result in ST
	fadd	ST,ST(2);add previous answer to running total of PI
					;now ST = Answer
	fst		PI
	;finished adding current run to pi
	
	;If we havent finished, jump to beginning
	fld		RunsDone;ST(1)
	fld		NumRuns	;ST
	fcom	ST(1)
	fstsw	Status
	mov		ax, Status
	sahf	
	jc		PI_LOOP		;if carry flag = 1, jump to loop
	
	;coprocessor needed again, to mltiply PI by 2 to get the proper aproximation
	fld		PI	;ST
	fmul	Two
	fst		PI
	invoke	FloatToStr,PI,addr PIOut
	invoke	StdOut,PIOut
end start

;modifies ST as output
Factorial	proc	input:REAL8
	LOCAL	STATUS:WORD
	LOCAL	TEMP:REAL8
	fld		input	;ST(2)
	fldz			;ST(1)
	fcom	ST(1)
	fstsw	STATUS
	mov		ax,STATUS
	jz		ZERO
	fld1			;ST(1)
	fld		input	;ST
	fsub	ST(1)	;subtract 1 from ST
	mov		TEMP,input		;push
	fst		input			;use decreased input
	invoke	Factorial,input	;ST is now the result of Factorial
	mov		input,TEMP		;pop
	fld		input			;ST is input, ST(1) is the result
	fmul	ST(1)			;ST = result of previous call * input
							;this si the return value
	LOCAL ZERO:
	fld1		;return ST = 1
Factorial endp
;modifies ST as output
DoubleFactorial	proc	input:REAL8
	LOCAL	return:REAL8
	LOCAL	STATUS:WORD
	mov		DBLFACTIN,input
	fld		input	;ST(1)
	fldz			;ST
	fcom	ST(1)	;comp input with 0
	fstsw	STATUS	;copy status flags
	mov		ax, STATUS
	jz		ZERO	;if input = 0
	
	fld		Two		;ST(1) last 3 are ST(2)and ST(3) respectively
	fld		input	;ST
	fsub	ST,ST(1)
	fst		input	;set up recursive call
	invoke	DoubleFactorial,input	;now ST is the return value
	mov		input,DBLFACTIN
	fld		input					;ST = input ST(1) = return
	fmul	ST(1)					;ST = input return * n
	;now ST is the return value
	LOCAL ZERO:	;when the recursive call finally hits 0
		mov		input,DBLFACTIN
		fldz			;ST(2)
		fld		Two		;ST(1)
		fld		input	;ST
		fprem			;ST = remainder
		fcom	ST(2)	;ST with ST(2)  [remainder with 0]
		fstsw	STATUS	;copy status flags
		mov		ax, STATUS
		jc		ZERO	;if remainder != 0
		;return 2
		fld		Two		;ST is now 2.0
		RET1:
		fld1			;ST now is 1.0
DoubleFactorial endp