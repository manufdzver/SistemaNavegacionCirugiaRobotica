import cv2
cap = cv2.VideoCapture('rstp://192.168.1.89:8000/user=&password=123456&channel=1&stream=1.sdp')
ret, frame = cap.read()
gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
cv2.imwrite('img.png',gray)
cap.release()