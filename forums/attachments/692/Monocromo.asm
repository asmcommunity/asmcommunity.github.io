; Monocromo: Para todos los colores RGB
; RGBij = (aRij + bGij + gBij) / d  donde a, b, g y d son parámetros de inicialización

format binary
use32
org 0

PIXEL	  = 3		; Tamaño en bytes de un pixel.
MASCARA_P = 00FFFFFFh	; Máscara de bits para el pixel.
MASCARA_R = 00FF0000h	; Máscara de bits para el canal Rojo.
MASCARA_G = 0000FF00h	; Máscara de bits para el canal Verde.
MASCARA_B = 000000FFh	; Máscara de bits para el canal Azul.
SHIFT_R   = 16		; Desplazamiento (shift) para el canal Rojo.
SHIFT_G   = 8		; Desplazamiento (shift) para el canal Verde.
SHIFT_B   = 0		; Desplazamiento (shift) para el canal Azul.


; Tabla con los puntos de entrada del API.
; Debe estar siempre al principio del archivo.

db 0E9h 	; offset 0
dd Iniciar
db 90h, 90h, 90h

db 0E9h 	; offset 8
dd Filtrar
db 90h, 90h, 90h


; Aquí guardamos los parámetros.
param_a    dd 0
param_b    dd 0
param_g    dd 0
param_d    dd 0
param_1sd  dq 0 	; Precalculamos 1 / d.


; Esta función auxiliar devuelve un puntero al principio del archivo.
; Esto es necesario para poder acceder a las variables globales.
; Parámetro: la dirección relativa del punto de retorno.
; Convención stdcall.
Delta:	pop	eax
	pop	edx
	push	eax
	sub	eax,	edx
	ret
	nop	; Alineamos la función siguiente a QWORD.
	nop


; API: void __cdecl Filtrar(int x, int y, void *p_arriba, void *p_medio, void *p_abajo, void *p_salida);
; Aplica el filtro en un punto del mapa de bits (no debe corresponder a un borde).
Filtrar:
	push	ebp				; Armamos el stack frame.
	mov	ebp,	esp
	push	ebx				; Preservamos algunos registros (convención C)

	; Stack frame:
	;       [ebp - 04h]     Valor original de EBX
	;       [ebp + 00h]     Valor original de EBP
	;       [ebp + 04h]     Dirección de retorno
	;       [ebp + 08h]     int x
	;       [ebp + 0Ch]     int y
	;       [ebp + 10h]     void *p_arriba
	;       [ebp + 14h]     void *p_medio
	;       [ebp + 18h]     void *p_abajo
	;       [ebp + 1Ch]     void *p_salida

	push	@f				; Cálculo de delta offset para ubicar los parámetros al filtro.
	call	Delta
@@:	mov	ebx,	eax			; EBX ahora apunta a la dirección real del principio del archivo.

	mov	eax,	[ebp + 14h]		; Leemos el pixel del medio.
	mov	eax,	[eax]
	mov	ecx,	eax			; Separamos las tres componentes RGB.
	mov	edx,	eax
	shr	eax,	SHIFT_R
	shr	ecx,	SHIFT_G
;        shr     edx,    SHIFT_B
	and	eax,	0FFh			; EAX == R
	and	ecx,	0FFh			; ECX == G
	and	edx,	0FFh			; EDX == B

	push	eax				; R: [esp + 08h]
	push	ecx				; G: [esp + 04h]
	push	edx				; B: [esp + 00h]

	fild	dword	[esp + 08h]		; Calcula aRij.
	fimul	dword	[ebx + param_a]
	fild	dword	[esp + 04h]		; Calcula bGij.
	fimul	dword	[ebx + param_b]
	fild	dword	[esp]			; Calcula gBij.
	fimul	dword	[ebx + param_g]
	faddp					; Suma los tres valores.
	faddp
	fmul	qword	[ebx + param_1sd]	; Multiplica por 1 / d.
	fistp	dword	[esp]			; Obtiene el resultado en EAX.
	fwait
	pop	eax
	pop	edx				; Restaura el balance del stack.
	pop	ecx

	cmp	eax,	0FFh			; Copia el resultado en las tres componentes R, G y B.
	jle	@f
	mov	eax,	0FFh
@@:	test	eax,	eax
	jns	@f
	xor	eax,	eax
@@:	mov	ecx,	eax
	mov	edx,	eax
	shl	eax,	SHIFT_R
	shl	ecx,	SHIFT_G
;        shl     edx,    SHIFT_B
	or	eax,	ecx
	or	eax,	edx

	mov	edx,	[ebp + 1Ch]		; Guardamos el pixel resultante.
	mov	[edx],	eax

	pop	ebx				; Recuperamos los registros preservados.
	leave					; Desarmamos el stack frame.
	ret					; El host quita los parámetros del stack (convención C).


; API: void __cdecl Iniciar(int ancho, int alto, int cantidad, ...);
; Inicialización del filtro.
; Los parámetros variables son todos enteros de 32 bits.
Iniciar:
	push	ebp				; Armamos el stack frame.
	mov	ebp,	esp

	; Stack frame:
	;       [ebp + 00h]     Valor original de EBP
	;       [ebp + 04h]     Dirección de retorno
	;       [ebp + 08h]     int ancho
	;       [ebp + 0Ch]     int alto
	;       [ebp + 10h]     int cantidad
	;       [ebp + 14h]     param_a
	;       [ebp + 18h]     param_b
	;       [ebp + 1Ch]     param_g
	;       [ebp + 20h]     param_d

	cmp	dword	[ebp + 10h], 4		; Verificamos que la cantidad de parámetros sea la correcta.
	jne	.salir

	push	@f				; Cálculo de delta offset.
	call	Delta
@@:
	push	dword	[ebp + 14h]		; Copiamos los parámetros al filtro.
	pop	dword	[eax + param_a]
	push	dword	[ebp + 18h]
	pop	dword	[eax + param_b]
	push	dword	[ebp + 1Ch]
	pop	dword	[eax + param_g]
	push	dword	[ebp + 20h]
	pop	dword	[eax + param_d]
	fld1					; Precalculamos 1 / d.
	fidiv	dword	[ebp + 20h]
	fstp	qword	[eax + param_1sd]

.salir:
	leave					; Desarmamos el stack frame.
	ret					; El host quita los parámetros del stack (convención C).

; Fin del archivo.
