import struct,socket,datetime,os,time,calendar

def Recv_All(sock,size):
    data = None
    while(size > 0):
        temp = sock.recv(size)
        l = len(temp)
        if(data == None):
            data = temp 
        else:
            data += temp
        size -= l        

    return data


def Sort_Name(name):
    try:
        s = name.split(b'\x00',1)[0]
        dm = s.decode('utf-8')
        return dm
    except:
        #print("Conversation Failed")
        return str(name)

def utc_to_time(utc):
    if(utc < 116444736000000000):
        return "Unknown Time"

    return datetime.datetime.utcfromtimestamp((utc-116444736000000000)/10000000).ctime()     

def List_dir(sock , path ):
    if(len(path) == 0):
        print('[!]Please Provide A Path')
        return

    try:
        sock.send(b'\x00\x00\x00\x00'+path.encode('utf-8')+ b'\\*')
    
    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

        
    while(1):
        try:
            cmd = sock.recv(4)
            n = int.from_bytes(cmd,byteorder='little')

            if( n== 2):
                stc = struct.unpack_from("=LL",sock.recv(8))

                if(stc[0] == 404):
                    print("Error Occured. Error Code :",stc[1])
                    return 0

            elif (n==1):
                dir_info = struct.unpack_from("=LQQQLLLL260s14s2x",Recv_All(sock,320)) #sock.recv(320)
                print(dir_info[0],'\t|\t',utc_to_time(dir_info[1]),'\t|\t',utc_to_time(dir_info[2]),'\t|\t',utc_to_time(dir_info[3]),'\t|\t',Sort_Name(dir_info[8]))
                #sock.send(b'\x01\x00\x00\x00')

            elif (n==0):
                return 0

            else:
                print("[-]Unknown Option")
                return 0

        except socket.timeout as Err:
            print(str(Err));return 404

        except ConnectionAbortedError as Err:
            print(str(Err));return 404

        except ConnectionError as Err:
            print(str(Err));return 404                   

        except struct.error as Err:
            print(str(Err));return 0    



def File_Upload(sock,src_path,des_path):

    
    if((len(src_path)==0) or (len(des_path)==0)):
        print('[!]Please Provide Source Path And Destination Path')
        return 0

    try:
        try:
            fd = open(src_path,'rb')
            file_info = os.stat(src_path)
        except OSError as Err:
            print(str(Err));return 0   

        sock.send(b'\x02\x00\x00\x00'+file_info.st_size.to_bytes(length=4,byteorder='little')+des_path.encode('utf-8'))

        status = struct.unpack_from('=LL',sock.recv(8))
        if(status[0] == 404):
            print("[-]Failed To Create File On Client Side.Error Code : ",status[1])
            return 0

        sock.sendfile(fd)
        fd.close()
        print("[+]File Uploaded SuccessFully")  
    

    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404  

    except struct.error as Err:
        print(str(Err));return 0

    except socket.timeout as Err:
        print(str(Err));return 404    



def File_Download(sock,src_path,des_path):
    
    
    if((len(src_path)==0) or (len(des_path)==0)):
        print('[!]Please Provide Source Path And Destination Path')
        return 0


    try:
        sock.send(b'\x03\x00\x00\x00'+src_path.encode('utf-8'))
        data = struct.unpack_from('=LLQ',sock.recv(16))

        if(data[0] == 404):
            print("[-]Failed To Download File. Error code: ",data[1])
            return 0
        i = 0

        try:
            file = open(des_path,'wb')
        except OSError as Err:
            print(str(Err));return 0


        while(i<data[2]):
            DATA = sock.recv(2048)
            i += len(DATA)
            print('\rFile Download Completed(percent): %d'% ((i/data[2])*100),end='' )
            file.write(DATA)
            file.flush()

        file.close();print('[+]File is Downloaded Successfully')

    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404 
    
    except socket.timeout as Err:
        print(str(Err));return 404

    except struct.error as Err:
        print(str(Err));return 0                       



def Change_Dir(sock,path):
    if(len(path) == 0):
        print("[!]Please Provide a path")
        return 0

    try:
        sock.send(b'\x04\x00\x00\x00'+path.encode('utf-8'))

        ret = struct.unpack_from('=LL',sock.recv(8))

        if(ret[0] == 404):
            print('[-]Failed To Change Directory.Error code: ',ret[1])
            return 0

    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404

    except struct.error as Err:
        print(str(Err));return 0    



def pwd(sock):

    try:
        sock.send(b'\x05\x00\x00\x00')
        path = sock.recv(260)
        print(Sort_Name(path))
    
    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404       


def SetFileTime(sock,file_name):
    try:
        fmt = input("[*]Please Enter Format Of Your Time: ")
        str_time = input('[*]Please Enter Time: ')
    except:
        fmt = '';str_time = ''

    if((len(fmt)==0) or (len(str_time)==0) or (len(file_name)==0)):
        print("[-]Something Is Missing . Please Check Your Input")
        return 0

    try:
        tup = time.strptime(str_time,fmt)
        time_t = calendar.timegm(tup)

        file_time = (time_t * 10000000) + 116444736000000000

        sock.send(b'\x06\x00\x00\x00'+ file_time.to_bytes(8,'little') + file_name.encode('utf-8'))

        ret = struct.unpack_from('=LL',sock.recv(8))

        if(ret[0] == 404):
            print("[-]Failed To Change FileStamp . Error Code : ",ret[1])
            return 0
        else:
            print('[+]FileStamp is Changed Sccessfully')

    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404

    except struct.error as Err:
        print(str(Err));return 0

    except ValueError as Err:
        print(str(Err));return 0


def Del_File(sock,path):

    if(len(path)==0):
        print("[-]Please Provide File name");return 0

    try:
        sock.send(b'\x07\x00\x00\x00' + path.encode('utf-8'))
        ret = struct.unpack_from('=LL',sock.recv(8))

        if(ret[0]==404):
            print('[-]Failed To Delete File. Error Code : ',ret[1]);return 0
        else:
            print('[+]File Is Deleted')
            return 0
    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404

    except struct.error as Err:
        print(str(Err));return 0              



def Create_Dir(sock,path):
    
    if(len(path)==0):
        print("[-]Please Provide File name");return 0

    try:
        sock.send(b'\x08\x00\x00\x00' + path.encode('utf-8'))
        ret = struct.unpack_from('=LL',sock.recv(8))

        if(ret[0]==404):
            print('[-]Failed To Create Directory. Error Code : ',ret[1]);return 0
        else:
            print('[+]Directory Is Created')
            return 0
    except ConnectionAbortedError as Err:
        print(str(Err));return 404

    except ConnectionError as Err:
        print(str(Err));return 404

    except socket.timeout as Err:
        print(str(Err));return 404

    except struct.error as Err:
        print(str(Err));return 0         