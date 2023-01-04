



#ifndef _FILE_IO_H_
#define _FILE_IO_H_

#include <WinSock2.h>
#include <MSWSock.h>
#include "Utils.h"
#include <stdio.h>

typedef struct FILE_RELATED_ERROR
{
	DWORD Code;
	DWORD Reason;
}FILE_ERROR , *LPFILE_ERROR;

extern int List_Directory(SOCKET sock);
extern int File_Upload(SOCKET sock);
extern void File_Download(SOCKET sock);
extern void Change_Dir(SOCKET sock);
extern void Get_Cur_Dir(SOCKET sock);
extern void ChangeFileTime(SOCKET sock);
extern void Del_File(SOCKET sock);
extern void Create_Dir(SOCKET sock);
#endif // !_FILE_IO_H_

