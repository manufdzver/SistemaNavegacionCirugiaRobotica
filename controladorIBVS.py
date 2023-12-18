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
        values()
        #cv2.imwrite('img/imgOptenida.png',frame)


    channel.basic_consume(queue='imagenActual', on_message_callback=callback, auto_ack=True)

    print(' [*] Waiting for messages. To exit press CTRL+C')
    channel.start_consuming()

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

\
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
    print("BLue")
    b = locateBlueMarker(imgDeseada)
    print(b)
    print(pixel2meters(284,273))
    print("Green")
    g = locateGreenMarker(imgDeseada)
    print(g)
    print(pixel2meters(263,101))
    print("Yellow")
    y = locateYellowMarker(imgDeseada)
    print(y)
    print(pixel2meters(193,285))
    print("BlueGreen")
    print(np.linalg.norm(b-g))
    print("BlueYellow")
    print(np.linalg.norm(b-y))
    print("GreenYellow")
    print(np.linalg.norm(g-y))

    

if __name__ == '__main__':
    main()