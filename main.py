import lecturaImagenActual 
import controladorIBVS 
import os, cv2
import threading
from threading import Thread
import time

stop_thread = threading.Event()

def run_thread():
    while True:
        #print(f'{x} of 25, thread running')
        time.sleep(5) #do some work...
        lecturaImagenActual.main()

        if stop_thread.is_set():
            break

    print('Finished  thread 1.')

def run_thread2():
    while True:
        #print(f'{x} of 25, thread running')
        #time.sleep(0.3) #do some work...
        controladorIBVS.main()

        if stop_thread.is_set():
            break

    print('Finished.')

def stop():
    stop_thread.set()
    
def main():
    thread = threading.Thread(target=run_thread)

    thread.start()
    #time.sleep(3)
    #stop_thread.set()
    #thread.join()

    thread2 = threading.Thread(target=run_thread2)

    thread2.start()
    #time.sleep(3)
    #stop_thread.set()
    #thread2.join()
    
if __name__ == "__main__":
    main()