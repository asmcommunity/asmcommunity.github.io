.386
.model flat, stdcall
option casemap:none

include C:\MASM32\include\windows.inc
include C:\MASM32\include\user32.inc
include C:\MASM32\include\kernel32.inc
include C:\masm32\include\gdi32.inc 

includelib C:\MASM32\lib\user32.lib
includelib C:\MASM32\lib\kernel32.lib
includelib C:\MASM32\lib\Gdi32.lib

.data?
hClipboard dd ?
hFile dd ?
bmp BITMAP <?>
imgX dd ?
imgY dd ?
cClrBits dw ?
DataSize dd ?
dwNumColors dd ?
RGBQuadSize dd ?
pBFH dd ?
pBMI dd ?
pRGBQuad dd ?
pData dd ?
hDlg dd ?
hDC_DIB dd ?
hDC dd ?
hNewBmp dd ?
hDC_DDB dd ?
OldObj dd ?
cbWrite dd ?

.data
lpFileName db "C:\test.bmp",0
nNumberofBytes dd 20

.code
start:

invoke keybd_event, VK_SNAPSHOT, 0, 0, 0
invoke OpenClipboard,0
invoke GetClipboardData, CF_BITMAP
mov hClipboard,eax
invoke CreateFile, ADDR lpFileName, GENERIC_WRITE or GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
mov hFile, eax

invoke GetObject, hClipboard, 4096, ADDR bmp

mov eax, bmp.bmWidth
mov imgX, eax
mov eax, bmp.bmHeight
mov imgY, eax
mov ax, bmp.bmBitsPixel
mov cClrBits, ax

mov cx,cClrBits
mov eax,1
shl eax,cl
mov dwNumColors,eax
mov edx,SIZEOF RGBQUAD
imul edx
mov RGBQuadSize,eax

mov eax,imgX
imul cClrBits
add eax,31
and eax,-31
shr eax,3
imul imgY
mov DataSize,eax

; Create a memory buffer
mov eax,RGBQuadSize
.IF cClrBits == 24 ; There is no RGBQUAD array for 24 bit
	mov dwNumColors,0
	mov RGBQuadSize,0
	mov eax,0
.ENDIF

add eax,SIZEOF BITMAPINFOHEADER
add eax,SIZEOF BITMAPFILEHEADER
add eax,DataSize
invoke GlobalAlloc,GMEM_FIXED,eax
mov pBFH,eax
add eax,SIZEOF BITMAPFILEHEADER
mov pBMI,eax
add eax,SIZEOF BITMAPINFOHEADER
mov pRGBQuad,eax
add eax,RGBQuadSize
mov pData,eax

invoke GetDC,hDlg
mov hDC,eax
invoke CreateCompatibleDC,hDC
mov hDC_DIB,eax
invoke CreateCompatibleDC,hDC
mov hDC_DDB,eax
invoke CreateCompatibleBitmap,hDC,imgX,imgY
mov hNewBmp,eax
invoke ReleaseDC,hDlg,hDC

invoke SelectObject,hDC_DDB,hNewBmp
mov OldObj,eax
invoke SelectObject,hDC_DIB,hClipboard
invoke BitBlt,hDC_DDB,0,0,imgX,imgY,hDC_DIB,0,0,SRCCOPY
invoke SelectObject,hDC_DDB,OldObj
invoke SelectObject,hDC_DIB,OldObj
invoke DeleteDC,hDC_DIB

invoke GetObject,hNewBmp,SIZEOF BITMAP,ADDR bmp

mov edi,pBMI
mov [edi].BITMAPINFO.bmiHeader.biXPelsPerMeter,0
mov [edi].BITMAPINFO.bmiHeader.biYPelsPerMeter,0
mov eax,dwNumColors
mov [edi].BITMAPINFO.bmiHeader.biClrUsed,eax
mov [edi].BITMAPINFO.bmiHeader.biClrImportant,0

mov [edi].BITMAPINFO.bmiHeader.biSize,SIZEOF BITMAPINFOHEADER
mov eax,bmp.bmWidth
mov [edi].BITMAPINFO.bmiHeader.biWidth,eax
mov eax,bmp.bmHeight
mov [edi].BITMAPINFO.bmiHeader.biHeight,eax
mov [edi].BITMAPINFO.bmiHeader.biPlanes,1
mov [edi].BITMAPINFO.bmiHeader.biCompression,BI_RGB
mov ax,cClrBits
mov [edi].BITMAPINFO.bmiHeader.biBitCount,ax
mov eax,DataSize
mov [edi].BITMAPINFO.bmiHeader.biSizeImage,eax

invoke GetDIBits, hDC_DDB, hNewBmp, 0, imgY, pData, pBMI, DIB_RGB_COLORS

mov esi,pBFH
mov [esi].BITMAPFILEHEADER.bfType,"MB"
mov eax,RGBQuadSize
add eax,DataSize
add eax,SIZEOF BITMAPINFOHEADER + SIZEOF BITMAPFILEHEADER
mov [esi].BITMAPFILEHEADER.bfSize,eax
mov [esi].BITMAPFILEHEADER.bfReserved1,0
mov [esi].BITMAPFILEHEADER.bfReserved2,0
mov eax,RGBQuadSize
add eax,sizeof BITMAPFILEHEADER
add eax,sizeof BITMAPINFOHEADER	
mov [esi].BITMAPFILEHEADER.bfOffBits,eax

mov ecx,SIZEOF BITMAPFILEHEADER
add ecx,sizeof BITMAPINFOHEADER
add ecx,DataSize
add ecx,RGBQuadSize

invoke WriteFile,hFile,pBFH,ecx,ADDR cbWrite,NULL

invoke DeleteDC,hDC_DDB
invoke GlobalFree,pBFH
invoke DeleteObject,hClipboard
invoke DeleteObject,hNewBmp

invoke EmptyClipboard
invoke CloseClipboard

invoke ExitProcess, NULL
end start