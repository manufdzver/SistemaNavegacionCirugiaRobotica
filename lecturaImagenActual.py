
import cv2, os, pika, base64


# Método para codificar la imagen, para poder mandarla a RabbitMQ como un String
def encodeImage(imgPath):
    with open(imgPath, "rb") as image2string:
        converted_string = base64.b64encode(image2string.read())
    return converted_string

# Método para leer la imagen actual que observa la cámara
def lecturaImagenActual():
    cap = cv2.VideoCapture(2, cv2.CAP_DSHOW)
    # ret es un booleano si hay imagen o no
    # fram es un array con la imagen
    ret, frame = cap.read()
    try:
        os.remove('img/imgActual.png')
    except:
        print("No image to delete")
    cv2.imwrite('img/imgActual.png',frame)
    cap.release()
    return frame

# Método para publicar la imagen actual que observa la cámara en RabbitMQ. Queue=imagenActual
def publicarImagenActual(imgPath):
    connection = pika.BlockingConnection(pika.ConnectionParameters(host='localhost'))
    channel = connection.channel()
    channel.queue_declare(queue='imagenActual')
    converted_string = encodeImage(imgPath)
    channel.basic_publish(exchange='', routing_key='imagenActual', body=converted_string)
    print("Sent imagenActual to Queue")
    connection.close()

def imageSize(imgPath):
    frame=cv2.imread(imgPath)
    height, width, channels = frame.shape
    print('height')
    print(height)
    print('width')
    print(width)
    print('channels')
    print(channels)
    #frame[201,200,0] = 255
    #frame[201,200,1] = 255 
    #frame[201,200,2] = 255

def imagenDeseada():
    cap = cv2.VideoCapture(2, cv2.CAP_DSHOW)
    ret, frame = cap.read()
    try:
        os.remove('img/imgDeseada.png')
    except:
        print("No image to delete")
    cv2.imwrite('img/imgDeseada.png',frame)
    cap.release()

def main():
    #imagenDeseada()
    #lecturaImagenActual()
    publicarImagenActual("img/imgActual.png")
    #print("Done")

if __name__ == '__main__':
    main()