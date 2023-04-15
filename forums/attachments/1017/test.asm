.386
.model flat,stdcall
option casemap:none

include windows.inc
include kernel32.inc
include wininet.inc
includelib kernel32.lib
includelib wininet.lib

cstr macro arg:vararg
	local szAutomaticString
	.data
		szAutomaticString db arg,0
		align 4
	.code
		exitm <offset szAutomaticString>
endm

.code
start proc
	local hInternet	:DWORD
	local hConnect	:DWORD
	local hRequest	:DWORD
	
	int 3
	
	invoke InternetOpen,
		cstr("Mozilla/5.0"),
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL,
		NULL,
		0
	mov hInternet, eax
	
	invoke InternetConnect,
		hInternet,
		cstr("127.0.0.1"),
		cstr("80"),
		NULL,
		NULL,
		INTERNET_SERVICE_HTTP,
		0,
		NULL
	mov hConnect, eax
	
	invoke HttpOpenRequest,
		hConnect,
		NULL,
		cstr('/'),
		NULL,
		NULL,
		cstr("*/*"),
		INTERNET_FLAG_IGNORE_CERT_CN_INVALID	or \
		INTERNET_FLAG_IGNORE_CERT_DATE_INVALID	or \
		INTERNET_FLAG_KEEP_CONNECTION			or \
		INTERNET_FLAG_NO_CACHE_WRITE			or \
		INTERNET_FLAG_NO_UI						or \
		INTERNET_FLAG_RELOAD,
		NULL
	mov hRequest, eax
	
	invoke HttpSendRequest,
		hRequest,
		NULL,
		0,
		NULL,
		0
	
	invoke GetLastError
	
	invoke HttpEndRequest,
		hRequest,
		NULL,
		0,
		NULL
	invoke InternetCloseHandle,
		hRequest
	invoke InternetCloseHandle,
		hConnect
	invoke InternetCloseHandle,
		hInternet
	invoke ExitProcess,
		0
	
start endp
end start
