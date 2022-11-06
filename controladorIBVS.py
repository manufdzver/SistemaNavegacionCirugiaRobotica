import pika, sys, os, ast
import cv2, numpy as np
import base64

def consumirImagenActual():
    connection = pika.BlockingConnection(pika.ConnectionParameters(host='localhost'))
    channel = connection.channel()

    channel.queue_declare(queue='imagenActual')

    def callback(ch, method, properties, body):
        try:
            os.remove('img/imgRecuperada.png')
        except:
            print("No image to delete")
        with open('encode.bin', "wb") as file:
            file.write(body)

        file = open('encode.bin', 'rb')
        byte = file.read()
        file.close()
        
        decodeit = open('img/imgRecuperada.png', 'wb')
        decodeit.write(base64.b64decode((byte)))
        decodeit.close()

        print(" [x] Received ")
        #cv2.imwrite('img/imgOptenida.png',frame)


    channel.basic_consume(queue='imagenActual', on_message_callback=callback, auto_ack=True)

    print(' [*] Waiting for messages. To exit press CTRL+C')
    channel.start_consuming()

def diferencia():
    frame=cv2.imread('img/imgDeseada.png')
    height, width, channels = frame.shape
    print('height')
    print(height)
    print('width')
    print(width)
    print('channels')
    print(channels)
    for x in range(0, height-1) :
     for y in range(0, width-1) :
          b=frame[x,y,0] #B Channel Value
          g=frame[x,y,1] #G Channel Value
          r=frame[x,y,2] #R Channel Value
          #print(str(x)+" "+str(y))
          if b==166 and g==144 and r==73:
              print("Lugar deseado:")
              print(x)
              print(y)

    res=0

def main():
    try:
        consumirImagenActual()
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)
    

if __name__ == '__main__':
    main()