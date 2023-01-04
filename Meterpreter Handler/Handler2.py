import sys,socket,threading,struct
import Handler_implement

Client_List = [] #[(socket,connection info)]
Sock = None

port = 5555

def Thread_Proc():
    while(1):
        try:
            cl = Sock.accept()
            cl[0].settimeout(60)
            #cl[0].setsockopt(socket.SOL_SOCKET,socket.SO_RCVBUF,0x40000)
            Type = struct.unpack_from("=L",cl[0].recv(4))[0]
            if(Type == 1 ):
                Client_List.append(cl)
                print("[+]New Connection : ",cl[1])

            elif (Type == 2): #This will be implemented Later
                cl[0].close()

            elif (Type == 3): #This will be implemented Later
                cl[0].close() 

            else:
                cl[0].close()

        except socket.timeout as Err:
            print(str(Err));cl[0].close()

        except struct.error as Err:
            print(str(Err));cl[0].close()

        except socket.error as Err:
            print(str(Err));cl[0].close()                                 


         

if(len(sys.argv)!=2):
    print("Usage %s <port>"%sys.argv[0])
    print("Default Port : 5555")
else:
    try:
        port = int(sys.argv[1])
    except ValueError as Err:
        print(str(Err))
        sys.exit(0)


try:
    Sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM,socket.IPPROTO_TCP) 
    Sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
    Sock.bind(("0.0.0.0",port))
    Sock.listen(5)
except:
    print("Error Occured")
    sys.exit(0)

thread = threading.Thread(target = Thread_Proc)
thread.daemon = True
thread.start()

Main_menu = Handler_implement.Menu(Client_List)
Main_menu.cmdloop()
