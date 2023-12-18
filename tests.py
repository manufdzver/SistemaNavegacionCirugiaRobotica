import lecturaImagenActual 
import controladorIBVS 
import os, cv2
import numpy as np
import matplotlib.pyplot as plt
from scipy.stats import norm
import scipy.stats as stats

#Yellow
b_yellow=39
g_yellow=224
r_yellow=230

#Green
b_green=63
g_green=123
r_green=75

#Blue
b_blue=209
g_blue=148
r_blue=92

def paintWhite(path, height, width):
    path='img/imgActual.png'
    frame=cv2.imread(path)
    frame[height,width,0]=255
    frame[height,width,1]=255
    frame[height,width,2]=255
    cv2.imwrite(path,frame)

def paintBlack(path, height, width):
    frame=cv2.imread(path)
    frame[height,width,0]=0
    frame[height,width,1]=0
    frame[height,width,2]=0
    cv2.imwrite(path,frame)

def cleanValues(coordinatesArray):
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

def diferencia():
    imgActual=cv2.imread('img/imgActual.png')
    imgDeseada=cv2.imread('img/imgDeseada.png')

    #Coordinates blue green yellow
    coordinatesImgActual = locateMarkers(imgActual)

    #Coordinates blue green yellow
    coordinatesImgDeseada = locateMarkers(imgDeseada)

    error = np.subtract(coordinatesImgDeseada, coordinatesImgActual)
    return error

def testing(image):
    blueCoordinates=getMarkersGroup(image, b_blue, g_blue, r_blue)
    greenCoordinates=getMarkersGroup(image,  b_green, g_green, r_green)
    yellowCoordinates=getMarkersGroup(image, b_yellow, g_yellow, r_yellow)

    blueCoordinates=cleanValues(blueCoordinates)
    greenCoordinates=cleanValues(greenCoordinates)
    yellowCoordinates=cleanValues(yellowCoordinates)

    centerBlue = centroid(blueCoordinates)
    centerGreen = centroid(greenCoordinates)
    centerYellow = centroid(yellowCoordinates)

    coordinates = np.empty((0, 2), int)
    coordinates = np.append(coordinates,np.array([centerBlue]), axis=0)
    coordinates = np.append(coordinates,np.array([centerGreen]), axis=0)
    coordinates = np.append(coordinates,np.array([centerYellow]), axis=0)

    return coordinates

def main():
    #lecturaImagenActual.main()
    #controladorIBVS.main()
    #error = diferencia()
    #print("Error:")
    #print(error)
    #print("Done!")
    #imgDeseada=cv2.imread('img/imgDeseada.png')
    #locImgDes=locateMarkers(imgDeseada)
    #imgActual=cv2.imread('img/imgRecuperada.png')
    #locImgAct =locateMarkers(imgActual)
    #print("Deseada: ")
    #print(locImgDes)
    #print("Actual: ")
    #print(locImgAct)
    a=np.array([0, 1])
    #a=np.array([])
    
    if len(a)==0:
        print("empty")
    else:
        print("not empty")
    


if __name__ == "__main__":
    main()