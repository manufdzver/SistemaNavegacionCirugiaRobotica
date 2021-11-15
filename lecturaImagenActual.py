import cv2
import pika

def publicarImagenActual(imagen):
    connection = pika.BlockingConnection(pika.ConnectionParameters(host='localhost'))
    channel = connection.channel()

    channel.queue_declare(queue='imagenActual')

    channel.basic_publish(exchange='', routing_key='imagenActual', body=imagen)
    print(" [x] Sent 'Hello World!'")
    connection.close()


def lecturaImagenActual():
    cap = cv2.VideoCapture('rstp://192.168.1.89:8000/user=&password=123456&channel=1&stream=1.sdp')
    ret, frame = cap.read()
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    cv2.imwrite('img.png',gray)
    cap.release()

def main():
    imagen = "Prueba 3"
    publicarImagenActual(imagen)
    print("Done")
    # Do amazing things

if __name__ == '__main__':
    main()