#define WIN32_LEAN_AND_MEAN
#define WIN32_WINNT 0x0600 

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#include "winsock2.h"
#pragma comment(lib, "Ws2_32.lib")

#define WM_SOCKET WM_USER+1
#define IDC_START 1000
#define IDC_STOP  1001

#define GetFont( Size, Face, Weight ) CreateFont( Size,0,0,0,Weight,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH+FF_DONTCARE,Face )
#define AddComponent( ExStyle, Class, Text, Style, x, y, width, height, id ) CreateWindowEx( ExStyle, Class, Text, Style, x, y, width, height, hWnd, (HMENU)id, hInstance, NULL )
#define AddLabel( Text, Style, x, y, width )              AddComponent( 0, "STATIC", Text, WS_CHILD|Style, x, y, width, 21, NULL )
#define AddButton( Text, Style, x, y, width, height, id ) AddComponent( 0, "BUTTON", Text, WS_CHILD|BS_PUSHBUTTON|Style, x, y, width, height, id )
#define AddTextField( Text, Style, x, y, width, id )      AddComponent( WS_EX_CLIENTEDGE, "EDIT", Text, WS_CHILD|ES_AUTOHSCROLL|Style, x, y, width, 20, id )
#define AddTextArea( Text, Style, x, y, width, height )   AddComponent( WS_EX_CLIENTEDGE, "Edit", Text, WS_CHILD|ES_AUTOHSCROLL|ES_AUTOVSCROLL|ES_MULTILINE|Style, x, y, width, height, 0 )

SOCKET listenSocket = INVALID_SOCKET;
HINSTANCE hInstance = NULL;
HANDLE hResults, hPort;

char output[10240];

int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
  LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

  HWND hWnd; MSG msg;
  WNDCLASSEX wndClassEx;

	WORD wVersionRequested = MAKEWORD( 1, 1 ); WSADATA wsaData;
	if( WSAStartup( wVersionRequested, &wsaData ) )
	{
		MessageBox( NULL, "Initialize WinSock Failed", "Test Application - WinSock", MB_OK );
		return( 1 );
	}

  wndClassEx.cbSize			    = sizeof( WNDCLASSEX );
  wndClassEx.style			    = CS_HREDRAW | CS_VREDRAW;
  wndClassEx.lpfnWndProc		= WndProc;
  wndClassEx.cbClsExtra		  = wndClassEx.cbWndExtra = 0;
  wndClassEx.hInstance		  = hInstance = hInst;
  wndClassEx.hIcon	 		    = NULL;
  wndClassEx.hCursor			  = NULL;
  wndClassEx.hbrBackground	= (struct HBRUSH__ *)COLOR_WINDOW;
  wndClassEx.lpszMenuName	  = NULL;
  wndClassEx.lpszClassName	= "WinClass";
  wndClassEx.hIconSm			  = NULL;

  InitCommonControls();
	if( RegisterClassEx( &wndClassEx ) )
  {
    hWnd = CreateWindowEx( 0, "WinClass", "Test Application - WinSock",
                           WS_OVERLAPPEDWINDOW | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT,
                           450, 300, NULL, NULL, hInst, NULL );
    ShowWindow( hWnd, nShowCmd );
    UpdateWindow( hWnd );

    while( GetMessage( &msg, NULL, 0, 0 ) )
    {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
    }
  }

	WSACleanup();	
  return( 0 );
}

BOOL OnStart( u_short port, HWND hWnd )
{
  int nRet;
  WSADATA wsaData;
  SOCKADDR_IN saServer;			

  nRet = WSAStartup( 0x0101, &wsaData );
  if( nRet != 0 )
  {
    MessageBox( hWnd, "WSAStartup()", "Error", MB_OK );
    return( FALSE );
  }

  // Create a TCP/IP stream socket
  listenSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  if( listenSocket == INVALID_SOCKET )
  {
    MessageBox( hWnd, "socket()", "Error", MB_OK );
    return( FALSE );
  }

  // Request async notification
  nRet = WSAAsyncSelect( listenSocket, hWnd, WM_SOCKET, FD_ACCEPT|FD_READ|FD_WRITE|FD_CLOSE );
  if( nRet == SOCKET_ERROR )
  {
    MessageBox( hWnd, "WSAAsyncSelect()", "Error", MB_OK );
    closesocket( listenSocket );
    return( FALSE );
  }

  // Fill in the address structure
  saServer.sin_port = htons( port );
  saServer.sin_family = AF_INET;
  saServer.sin_addr.s_addr = INADDR_ANY;

  // bind our name to the socket
  nRet = bind( listenSocket, (LPSOCKADDR)(&saServer), sizeof( struct sockaddr ) );
  if( nRet == SOCKET_ERROR )
  {
    MessageBox( hWnd, "bind()", "Error", MB_OK );
    closesocket( listenSocket );
    return( FALSE );
  }

  // Set the socket to listen
  nRet = listen( listenSocket, SOMAXCONN );
  if( nRet == SOCKET_ERROR )
  {
    MessageBox( hWnd, "listen()", "Error", MB_OK );
    closesocket( listenSocket );
    return( FALSE );
  }

  return( TRUE );
}

void OnStop( void )
{
	// Close the listening socket
	closesocket( listenSocket );
}

void OnAccept( HWND hWnd, int nErrorCode )
{
  SOCKADDR_IN SockAddr;
  SOCKET      peerSocket;
  int         nRet, nLen;

	// accept the new socket descriptor
	nLen = sizeof( SOCKADDR_IN );
	peerSocket = accept( listenSocket, (LPSOCKADDR)&SockAddr, &nLen );
	if( peerSocket == SOCKET_ERROR )
	{
		nRet = WSAGetLastError();
		if( nRet != WSAEWOULDBLOCK )
		{
			return;
		}
	}

	// Make sure we get async notices for this socket
  nRet = WSAAsyncSelect( peerSocket, hWnd, WM_SOCKET, FD_READ|FD_WRITE|FD_CLOSE );
	if( peerSocket == SOCKET_ERROR )
	{
		return;
	}
}

void OnRead( SOCKET socket, int nErrorCode )
{
  int nRet;
  static BYTE buf[2048];

  // Receive the data
  nRet = recv( socket, buf, sizeof( buf ) - 1, 0 );
  if( nRet == SOCKET_ERROR )
  {
    if( WSAGetLastError() != WSAEWOULDBLOCK ) closesocket( socket ); return;
  }
}

void OnWrite( SOCKET socket, int nErrorCode )
{
  static BYTE message[2048];
  char *content = "<html><body>Hello World!!</body></html>";
  
  wsprintf( message, "HTTP/1.1 200 OK\r\n"
                     "Connection: close\r\n"
                     "Content-Length: %d\r\n"
                     "\r\n%s", lstrlen( content ), content );
  send( socket, message, lstrlen( message ), 0 );
}

void OnClose( SOCKET socket, int nErrorCode )
{
  closesocket( socket );
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  char s[64]; HFONT mono14;

  switch( message )
  {
    case WM_SOCKET:
      switch( WSAGETSELECTEVENT( lParam ) )
      {
        case FD_ACCEPT:
          OnAccept( hWnd, WSAGETSELECTERROR( lParam ) );
          lstrcat( output, "  Connection accepted!\r\n" );
          SetWindowText( hResults, output );
          break;

        case FD_READ:
          OnRead( (SOCKET)wParam, WSAGETSELECTERROR( lParam ) );
          lstrcat( output, "    FD_READ message\r\n" );
          SetWindowText( hResults, output );
          break;

        case FD_WRITE:
          OnWrite( (SOCKET)wParam, WSAGETSELECTERROR( lParam ) );
          lstrcat( output, "    FD_WRITE message\r\n" );
          SetWindowText( hResults, output );
          break;

        case FD_CLOSE:
          OnClose( (SOCKET)wParam, WSAGETSELECTERROR( lParam ) );
          lstrcat( output, "  Connection closed!\r\n" );
          SetWindowText( hResults, output );
          break;
      }
      return( 0 );

    case WM_COMMAND:
      switch( LOWORD( wParam ) )
      {
        case IDC_START:
          GetWindowText( hPort, s, 64 );
          OnStart( (u_short)atoi( s ), hWnd );
          wsprintf( output, "Starting server on port %d...\r\n", atoi( s ) );
          SetWindowText( hResults, output );
          break;

        case IDC_STOP:
          OnStop();
          lstrcat( output, "Server stopped.\r\n" );
          SetWindowText( hResults, output );
          break;
      }
      return( 0 );

    case WM_SIZE:
      MoveWindow( hResults, 0, 45, LOWORD( lParam ), HIWORD( lParam ) - 45, TRUE );
      return( 0 );

    case WM_CREATE:
      mono14  = GetFont( 14, "Courier New", FW_DONTCARE );

      AddLabel( "Port Number:", WS_VISIBLE, 10, 13, 100 );
      hPort = AddTextField( "8080", WS_VISIBLE, 110, 10, 50, 0 );
      AddButton( "Start Server", WS_VISIBLE, 200, 10, 100, 25, IDC_START );
      AddButton( "Stop Server", WS_VISIBLE, 310, 10, 100, 25, IDC_STOP );
      hResults = AddTextArea( "", WS_VISIBLE|WS_VSCROLL, 0, 0, 0, 0 );
      SendMessage( hResults, WM_SETFONT, (WPARAM)mono14, TRUE );
      return( 0 );

    case WM_CLOSE:
    case WM_DESTROY:
      PostQuitMessage( 0 );
      return( 0 );
  }

  return( DefWindowProc( hWnd, message, wParam, lParam ) );
}