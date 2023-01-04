



#ifndef _REFLECTIVE_PE_H_
#define _REFLECTIBE_PE_H_
#include <winsock2.h>
#include <windows.h>


typedef struct _pe_error
{
    DWORD code;
    DWORD reason;
}PE_ERROR , *LPPE_ERROR;


typedef LPVOID __stdcall (*GetProc)(LPVOID ,LPSTR);
typedef LPVOID __stdcall (*LoadLib)(LPSTR);

typedef struct _pe_info
{
    LPVOID base;
    BOOL is_required_rolocation;
    GetProc GetFuncAddr;
    LoadLib LoadDll;
}PE_INFO , *LPE_INFO;

typedef struct _relocation_entry_
{
    WORD offset:12;WORD type:4;
}TYPE_ENTRY , *LPTYPE_ENTRY;

#ifdef _WIN64
#define BASE_REL_TYPE 10
#else
#define BASE_REL_TYPE 3
#endif // _WIN64


extern int Remote_Refelctive_Pe(SOCKET sock);
extern int Early_Bird(SOCKET sock);
#endif // _REFLECTIVE_PE_H_
