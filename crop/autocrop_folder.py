# https://stackoverflow.com/questions/60941012/how-do-i-find-corners-of-a-paper-when-there-are-printed-corners-lines-on-paper-i

from PIL import Image
from pillow_heif import register_heif_opener
register_heif_opener()
import numpy as np
import cv2
import sys
import glob

# does not work with heif rn
def crop(imgpath, destpath, sensitivity=110, debug=False):
    img = cv2.imread(imgpath)
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)

    # Define lower and uppper limits of what we call "white-ish"
    lower_white = np.array([0, 0, 255 - sensitivity])
    upper_white = np.array([255, sensitivity, 255])

    # Create mask to only select white
    mask = cv2.inRange(hsv, lower_white, upper_white)

    # Change image to green where we found white
    nopole = img.copy()
    nopole[mask > 0] = (76, 52, 40)

    #  convert img to grayscale
    gray = cv2.cvtColor(nopole, cv2.COLOR_BGR2GRAY)

    # blur image
    blur = cv2.GaussianBlur(gray, (3,3), 0)

    # do otsu threshold on gray image
    thresh = cv2.threshold(blur, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)[1]

    # apply morphology
    kernel = np.ones((7,7), np.uint8)
    morph = cv2.morphologyEx(thresh, cv2.MORPH_CLOSE, kernel)
    morph = cv2.morphologyEx(morph, cv2.MORPH_OPEN, kernel)

    # get largest contour
    contours = cv2.findContours(morph, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    contours = contours[0] if len(contours) == 2 else contours[1]
    area_thresh = 0
    for c in contours:
        area = cv2.contourArea(c)
        if area > area_thresh:
            area_thresh = area
            big_contour = c

    # draw white filled largest contour on black just as a check to see it got the correct region
    page = np.zeros_like(img)
    cv2.drawContours(page, [big_contour], 0, (255,255,255), -1)

    # get perimeter and approximate a polygon
    peri = cv2.arcLength(big_contour, True)
    corners = cv2.approxPolyDP(big_contour, 0.04 * peri, True)

    # enforce CCW starting at top-left indexing
    avgx = np.mean(corners[:,0,0])
    avgy = np.mean(corners[:,0,1])

    while(corners[0,0,0] > avgx or corners[0,0,1] > avgy):
        corners = corners[[1,2,3,0], :, :]

    # draw polygon on input image from detected corners
    polygon = img.copy()
    cv2.polylines(polygon, [corners], True, (0,0,255), 1, cv2.LINE_AA)
    # Alternate: cv2.drawContours(page,[corners],0,(0,0,255),1)

    # for simplicity get average of top/bottom side widths and average of left/right side heights
    # note: probably better to get average of horizontal lengths and of vertical lengths
    width = 0.5*( (corners[3][0][0] - corners[0][0][0]) + (corners[2][0][0] - corners[1][0][0]) )
    height = 0.5*( (corners[1][0][1] - corners[0][0][1]) + (corners[2][0][1] - corners[3][0][1]) )
    width = np.int0(width)
    height = np.int0(height)

    # reformat input corners to x,y list
    icorners = []
    for corner in corners:
        pt = [ corner[0][0],corner[0][1] ]
        icorners.append(pt)
    icorners = np.float32(icorners)

    # get corresponding output corners from width and height
    ocorners = [ [0,0], [0,height], [width,height], [width,0] ]
    ocorners = np.float32(ocorners)

    # get perspective tranformation matrix
    M = cv2.getPerspectiveTransform(icorners, ocorners)

    # do perspective 
    warped = cv2.warpPerspective(img, M, (width, height))
    
    warped_rgb = cv2.cvtColor(warped, cv2.COLOR_BGR2RGB)
    polygon_rgb = cv2.cvtColor(polygon, cv2.COLOR_BGR2RGB)
    
    Image.fromarray(warped_rgb).save(destpath)
    if debug:
        Image.fromarray(page).save(destpath+"page.jpg")
        Image.fromarray(polygon_rgb).save(destpath+"polygon.jpg")

if len(sys.argv) < 3 or len(sys.argv) > 5 :
    sys.print("Format: python3 autocrop_folder.py imgfolder destfolder [sensitivity=110] [debug=False]")
    exit(1)
for folder_name in glob.glob(sys.argv[1] + "*"):
    if len(sys.argv) == 3:
        print(folder_name)
        print(sys.argv[2]  + folder_name.split('/')[-1])
        crop(folder_name, sys.argv[2]  + folder_name.split('/')[-1])
    elif len(sys.argv) == 5:
        crop(folder_name, sys.argv[2]  + folder_name.split('/')[-1], sys.argv[3], sys.argv[4])
    elif type(int(sys.argv[3])) is int:
        crop(folder_name, sys.argv[2]  + folder_name.split('/')[-1], sensitivity=sys.argv[3])
    else:
        crop(folder_name, sys.argv[2]  + folder_name.split('/')[-1], debug=sys.argv[3])