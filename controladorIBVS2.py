import pika, sys, os, ast
import cv2, numpy as np
import base64
import math
import numpy as np

#Yellow
b_yellow=38
g_yellow=224
r_yellow=232

#Green
b_green=67
g_green=120
r_green=77

#Blue
b_blue=206
g_blue=148
r_blue=94

def consumirImagenActual():
    #Iniciar connection sincrona (bloqueante) con RabbitMQ en localhost. 
    connection = pika.BlockingConnection(pika.ConnectionParameters(host='localhost'))
    channel = connection.channel()

    #Nombre de la cola a cual conectarnos. El producer mandará aqui la imágen.
    channel.queue_declare(queue='imagenActual')

    # Funcion que se correrá cada vez que recibamos una imagen en la cola "body"
    def callback(ch, method, properties, body):
        try:
            if os.path.exists('img/imgRecuperada.png'):
                os.remove('img/imgRecuperada.png')
        except FileNotFoundError:
            print("No image to delete")
        except Exception as e:
            print(f"An error occurred while deleting the image: {e}")
        
        #Guardamos la imágen en formato binario en una variable temporal encode.bin
        try:
            with open('encode.bin', "wb") as file:
                file.write(body)

            # Creamos un archivo png vacio y hacemos un decoding base64 para guardar la imagen decodeada
            with open('encode.bin', 'rb') as file:
                byte = file.read()
            
            with open('img/imgRecuperada.png', 'wb') as decodedImg:
                decodedImg.write(base64.b64decode((byte)))

            print(" [x] Received ")
            values()
            #cv2.imwrite('img/imgOptenida.png',frame)
            ch.basic_ack(delivery_tag=method.delivery_tag)
        except Exception as e:
            print(f"An error occurred while processing the image: {e}")
            ch.basic_nack(delivery_tag=method.delivery_tag, requeue=False)

    # Comenzamos a escuchar la cola de Rabbit MQ para procesar la imágen.
    channel.basic_consume(queue='imagenActual', on_message_callback=callback, auto_ack=False)

    print(' [*] Waiting for messages. To exit press CTRL+C')
    try:
        channel.start_consuming()
    except KeyboardInterrupt:
        channel.stop_consuming()
    finally:
        connection.close()

def cleanValues(coordinatesArray):
    if len(coordinatesArray)==0:
        final_list=coordinatesArray
    else:
        x=coordinatesArray[:,0]
        y=coordinatesArray[:,1]
        
        mean_x = np.mean(x, axis=0)
        sd_x = np.std(x, axis=0)
        mean_y = np.mean(y, axis=0)
        sd_y = np.std(y, axis=0)

        des = 0.6

        final_list = [a for a in coordinatesArray if ((a[0] > (mean_x - des * sd_x)) and (a[1] > (mean_y - des * sd_y)))]
        final_list = [a for a in final_list if ((a[0] < (mean_x + des * sd_x)) and (a[1] < (mean_y + des * sd_y)))]
        final_list = np.array(final_list)

    return final_list

def centroid(coordinatesArray):
    if len(coordinatesArray)==0:
        return coordinatesArray
    else:
        n = len(coordinatesArray)  
        return (np.sum(coordinatesArray,axis=0)/n).astype(int)

def getMarkersGroup(image, blue, green, red):
    overhead=30
    height, width, channels = image.shape
    print(image.shape)
    coordinates = np.empty((0, 2), int)
    for x in range(0, height-1) :
     for y in range(0, width-1) :
        
        b=image[x,y,0] #B Channel Value
        g=image[x,y,1] #G Channel Value
        r=image[x,y,2] #R Channel Value
        #Yellow
        if (blue-overhead)<=b<=(blue+overhead) and (green-overhead)<=g<=(green+overhead) and (red-overhead)<=r<=(red+overhead):
            #paintWhite('img/imgActual.png',x,y)
            coordinates = np.append(coordinates,np.array([[x,y]]), axis=0)
    return coordinates

def locateMarkers(image):
    blueCoordinates=getMarkersGroup(image, b_blue, g_blue, r_blue)
    
    #greenCoordinates=getMarkersGroup(image, b_green, g_green, r_green)
    #yellowCoordinates=getMarkersGroup(image, b_yellow, g_yellow, r_yellow)

    blueCoordinates=cleanValues(blueCoordinates)
    #greenCoordinates=cleanValues(greenCoordinates)
    #yellowCoordinates=cleanValues(yellowCoordinates)
    centerBlue = centroid(blueCoordinates)
    #centerGreen = centroid(greenCoordinates)
    #centerYellow = centroid(yellowCoordinates)
    
    #coordinates = np.empty((0, 2), int)
    
    #coordinates = np.append(coordinates,np.array([centerBlue]), axis=0)
    #print(coordinates)
    #coordinates = np.append(coordinates,np.array([centerGreen]), axis=0)
    #coordinates = np.append(coordinates,np.array([centerYellow]), axis=0)

    return centerBlue

def locateBlueMarker(image):
    blueCoordinates=getMarkersGroup(image, b_blue, g_blue, r_blue)
    blueCoordinates=cleanValues(blueCoordinates)
    centerBlue = centroid(blueCoordinates)

    return centerBlue

def locateGreenMarker(image):
    greenCoordinates=getMarkersGroup(image, b_green, g_green, r_green)
    greenCoordinates=cleanValues(greenCoordinates)
    centerGreen = centroid(greenCoordinates)

    return centerGreen

def locateYellowMarker(image):
    yellowCoordinates=getMarkersGroup(image, b_yellow, g_yellow, r_yellow)
    yellowCoordinates=cleanValues(yellowCoordinates)
    centerYellow = centroid(yellowCoordinates)

    return centerYellow

def intrinsicCamaraMatrix():
    diagFOV = 59
    hFOV = 51.3
    vFOV = 35.5
    resH = 640
    resV = 480

    u0 = resH/2
    v0= resV/2
    fx = u0/math.tan(math.radians(hFOV/2))
    fy = v0/math.tan(math.radians(vFOV/2))

    matrix = np.matrix([
    [fx, 0, u0], 
    [0, fy, v0],
    [0, 0, 1]
    ] )

    return matrix
"""
def controlIBVS(pos,pos_d,z,Fu,Fv,u0,v0, lamda):
    n = size(pos,2)

    s = pixel2coord(pos, Fu, Fv, u0, v0)
    s_final = pixel2coord(pos, Fu, Fv, u0, v0)

    J = zeros(2*n, 6)

    for i in range(1, n) :
        x = s(i*2-1)
        y = (i*2)
        J(i*2-1,:) = [-1/z, 0, x/z, x*y, -(1+(x*x)), y]
        J(i*2,:) = [0, -1/z, y/z, 1+(y*y), -x*y, -x]

    Ji = pinv(J)

    cIBVS = -lamda* Ji * (s-s_final)

    return cIBVS
"""

def pixel2meters(u,v):
    matrix=intrinsicCamaraMatrix()
    n = 2
    fx= matrix[0,0]
    fy= matrix[1,1]
    u0= matrix[0,2]
    v0= matrix[1,2]
    x = (u-u0)/fx
    y = (v-v0)/fy

    return [x,y]




def values():
    imgDeseada=cv2.imread('img/imgDeseada.png')
    locImgDes=locateMarkers(imgDeseada)
    imgActual=cv2.imread('img/imgRecuperada.png')
    locImgAct =locateMarkers(imgActual)
    print("Deseada: ")
    print(locImgDes)
    print("Actual: ")
    print(locImgAct)

def find_centroid(image_path, rgbColor):
    """Finds the centroid of a yellow marker in an image."""

    img = cv2.imread(image_path)
    if img is None:
        return None, "Error: Could not read image."
    
    # Convert to HSV
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    
    # Define yellow color range (adjust these values as needed)

    hue, saturation, value = rgb_to_hsv(rgbColor)
    #print(hue, saturation, value)

    # Create an image filled with the HSV color
    hsv_image = np.zeros((200, 200, 3), dtype=np.uint8)
    hsv_image[:] = (hue, saturation, value)

    # Convert the HSV image back to BGR for display
    bgr_image = cv2.cvtColor(hsv_image, cv2.COLOR_HSV2BGR)
    # Display the image
    cv2.imshow("HSV Color Display", bgr_image)
    cv2.imshow("HSV Image", hsv)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

    hueVar=2
    satVar=50
    valueVar=50

    lower_yellow = np.array([hue-hueVar, saturation-satVar, value-valueVar], dtype=np.uint8)
    upper_yellow = np.array([hue+hueVar, 255, 255], dtype=np.uint8)
    
    # Create mask
    mask = cv2.inRange(hsv, lower_yellow, upper_yellow)
    cv2.imshow("Mask", mask)
    
    # Connected component analysis
    num_labels, labels, stats, centroids = cv2.connectedComponentsWithStats(mask, 8, cv2.CV_32S)
    
    # Filter by size (find the largest component, excluding background)
    largest_label = 0
    largest_area = 0
    for i in range(1, num_labels):  # Start from 1 to skip background
        area = stats[i, cv2.CC_STAT_AREA]
        if area > largest_area:
            largest_area = area
            largest_label = i
    #print(largest_label)
    if largest_label == 0:  # No marker found
        return None, "Error: Yellow marker not found."

    # Get centroid of the largest component
    cx, cy = int(centroids[largest_label][0]), int(centroids[largest_label][1])

    # Calculate center coordinates
    # Get image dimensions
    height, width = img.shape[:2]  #640x480
    center_x = width // 2
    center_y = height // 2

    # Transform centroid coordinates to centered origin
    centered_cx = cx - (width / 2)
    centered_cy = (cy - (height / 2))*(-1)
    print(f"Centroid: ({cx}, {cy})")
    print(f"Centroid (centered): ({centered_cx}, {centered_cy})")

    # Paint a white circle at the centroid
    cv2.circle(img, (cx, cy), 5, (255, 255, 255), -1)  # -1 fills the circle
    # Paint x-axis
    cv2.line(img, (0, center_y), (width, center_y), (255, 255, 255), 1)
    # Paint y-axis
    cv2.line(img, (center_x, 0), (center_x, height), (255, 255, 255), 1)

    cv2.imshow("Image with Centroid", img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
    
    return (cx, cy), None

def rgb_to_hsv(rgb_color):
    """Converts an RGB color tuple to HSV."""
    
    # Convert RGB to a NumPy array (OpenCV expects a NumPy array)
    rgb = np.uint8([[list(rgb_color)]]) # Note: OpenCV uses BGR, not RGB!

    # Convert to HSV
    hsv = cv2.cvtColor(rgb, cv2.COLOR_RGB2HSV)  # Or cv2.COLOR_BGR2HSV if you got BGR from paint

    # Extract the HSV values
    hue = hsv[0][0][0]
    saturation = hsv[0][0][1]
    value = hsv[0][0][2]
    
    return hue, saturation, value

def main():
    '''
    try:
        consumirImagenActual()
        
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)
    '''
    imgDeseada=cv2.imread('img/imgDeseada.png')
    print("Blue")
    b = locateBlueMarker(imgDeseada)
    print(b)
    #print(pixel2meters(284,273))
    print("Green")
    g = locateGreenMarker(imgDeseada)
    print(g)
    #print(pixel2meters(263,101))
    print("Yellow")
    y = locateYellowMarker(imgDeseada)
    print(y)
    #print(pixel2meters(193,285))
    print("BlueGreen")
    print(np.linalg.norm(b-g))
    print("BlueYellow")
    print(np.linalg.norm(b-y))
    print("GreenYellow")
    print(np.linalg.norm(g-y))

def main2():
    blue=(r_blue, g_blue, b_blue)
    yellow=(r_yellow, g_yellow, b_yellow)
    green=(r_green, g_green, b_green)
    #find_centroid('img/imgActual.png', blue)
    find_centroid('img/imgActual.png', yellow)
    #find_centroid('img/imgActual.png', green)

    

if __name__ == '__main__':
    main2()