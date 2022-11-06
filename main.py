import lecturaImagenActual 
import controladorIBVS 
import os, cv2
import numpy as np
import matplotlib.pyplot as plt
from scipy.stats import norm
import scipy.stats as stats

def cleanValues(coordinatesArray):
    x=coordinatesArray[:,0]
    y=coordinatesArray[:,1]
    plt.xlim(0,480)
    plt.ylim(0,640)
    plt.scatter(x,y,color = "g", marker = ".") 
    plt.show()
    
    mean_x = np.mean(x, axis=0)
    sd_x = np.std(x, axis=0)
    mean_y = np.mean(y, axis=0)
    sd_y = np.std(y, axis=0)

    des = 0.6

    final_list = [a for a in coordinatesArray if ((a[0] > (mean_x - des * sd_x)) and (a[1] > (mean_y - des * sd_y)))]
    final_list = [a for a in final_list if ((a[0] < (mean_x + des * sd_x)) and (a[1] < (mean_y + des * sd_y)))]
    final_list = np.array(final_list)
    
    x=final_list[:,0]
    y=final_list[:,1]
    plt.xlim(0,480)
    plt.ylim(0,640)
    plt.scatter(x,y,color = "g", marker = ".") 
    plt.show()

    return final_list


def centroid(coordinatesArray):
    n = len(coordinatesArray)  
    return (np.sum(coordinatesArray,axis=0)/n).astype(int)


def diferencia():
    frame=cv2.imread('img/imgActual.png')
    height, width, channels = frame.shape
    #Yellow
    blue=39
    green=224
    red=230
    #Green
    #blue=63
    #green=123
    #red=75
    #Blue
    #blue=210
    #green=150
    #red=98
    overhead=30
    # 594 124 83
    pixelsPerPoint=0
    coordinates = np.empty((0, 2), int)
    for x in range(0, height-1) :
     for y in range(0, width-1) :
        
        b=frame[x,y,0] #B Channel Value
        g=frame[x,y,1] #G Channel Value
        r=frame[x,y,2] #R Channel Value
        #Yellow
        if (blue-overhead)<=b<=(blue+overhead) and (green-overhead)<=g<=(green+overhead) and (red-overhead)<=r<=(red+overhead):
            paintWhite(x,y)
            coordinates = np.append(coordinates,np.array([[x,y]]), axis=0)
            pixelsPerPoint = pixelsPerPoint +1
        
    print("Pixels:"+str(pixelsPerPoint))
    coordinates = cleanValues(coordinates)
    center=centroid(coordinates)
    #print(coordinates)
    print(center)
    paintBlack(center[0], center[1])

    

def paintWhite(height, width):
    frame=cv2.imread('img/imgActual.png')
    frame[height,width,0]=255
    frame[height,width,1]=255
    frame[height,width,2]=255
    cv2.imwrite('img/imgActual.png',frame)

def paintBlack(height, width):
    frame=cv2.imread('img/imgActual.png')
    frame[height,width,0]=0
    frame[height,width,1]=0
    frame[height,width,2]=0
    cv2.imwrite('img/imgActual.png',frame)

def main():
    #lecturaImagenActual.main()
    #controladorIBVS.main()
    diferencia()
    #print("Done!")

    #array = np.array([[1,2],[3,4],[5,6]])
    #print(array[:,0])
    #plt.scatter(x, y, color = "g", marker = "o") 

if __name__ == "__main__":
    main()