# Import modules
import subprocess
from threading import Thread
import logging
from time import sleep


logger = logging.getLogger(__name__)
# Prompt the user to input a network address
net_addr = "192.168.1."

class pingWorker(Thread):

    def __init__(self, ip):
        Thread.__init__(self)
        self.ip = ip

    def run(self):
        info = subprocess.STARTUPINFO()
        info.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        info.wShowWindow = subprocess.SW_HIDE
        output = subprocess.Popen(['ping', '-n', '1', '-w', '500', self.ip], stdout=subprocess.PIPE, startupinfo=info).communicate()[0]

        if "Destination host unreachable" in output.decode('utf-8'):
            pass
            #print(self.ip + " is Offline")
        elif "Request timed out" in output.decode('utf-8'):
            pass
            #print(self.ip + " is Offline")
        else:
            print(self.ip + " is Online######################")


# For each IP address in the subnet, 
# run the ping command with subprocess.popen interface
for i in range(0,255):
    worker = pingWorker(net_addr + str(i))
    worker.start()