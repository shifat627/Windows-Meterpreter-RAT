

#ifndef _SYSTEM_FUNC_H_
#define _SYSTEM_FUNC_H_
#include<winsock2.h>
#include<windows.h>

typedef struct DriveInfo
{
    char name[4];
    DWORD failled;
    DWORD SectorsPerCluster;
    DWORD BytesPerSector;
    DWORD NumberOfFreeClusters;
    DWORD TotalNumberOfClusters;
}DRIVE_INFO ;
typedef struct shutdown_options
{
    DWORD timeout;
    WORD forced;
    WORD Restart;
}SHUTDOWN_OP;

typedef struct _sys_error
{
    DWORD code;
    DWORD reason;
}SYS_ERROR , * LPSYS_ERROR;


extern int Shell(SOCKET sock);
extern int List_Process(SOCKET sock);
extern int Change_Wallpaper(SOCKET sock);
extern void Lock_Window(SOCKET sock);
extern void Reboot(SOCKET sock);
extern void List_Installed(SOCKET sock);
extern void List_Disk(SOCKET sock);
#endif // _SYSTEM_FUNC_H_
