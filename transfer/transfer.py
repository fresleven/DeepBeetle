import os, io
import shutil
from tkinter.filedialog import askdirectory
from tkinter.messagebox import askyesno
from crop.autocrop import crop
import cv2
from PIL import Image
import torch


def readQR(image : cv2.Mat):
    qrCodeDetector = cv2.QRCodeDetector()
    decoded_text, points, _ = qrCodeDetector.detectAndDecode(image)
    if points is None:
        return ''
    return decoded_text

# find sd path
sd_path = askdirectory(message='Select Path To SD')
assert sd_path
save_dir = askdirectory(message='Select Directory To Save To')
assert save_dir

# confirmation
assert askyesno(message=f'Confirm Image Transfer:\n{sd_path}\n\u2192\n{save_dir}')

# move imgs from sd to pc
save_paths = []
dir = os.fsencode(sd_path)
for file in os.listdir(dir):
    filename = os.fsdecode(file)
    if filename.endswith(".png") or filename.endswith(".jpg"):
        # save original image to computer
        src_path = os.path.join(sd_path, filename)
        dest_path = os.path.join(save_dir, filename)
        shutil.move(src_path, dest_path)
        save_paths.append(dest_path)

#load model
model_path = '../yolov5_model/runs/train/20beetle_40-non_20dirt_bkg_overlap/weights/best.pt'
model = torch.hub.load('ultralytics/yolov5', 'custom', model_path)
model.eval()

# create output csv
csv_title = 'Image Path, Location, Number of Beetles, Confidence\n'
csv = ''
for path in save_paths:
    image = cv2.imread(path)
    
    # read QR
    trap_label = readQR(image)

    # crop + save cropped
    warped_rgb = crop(image)
    Image.fromarray(warped_rgb).save(path+"cropped.jpg")

    # run model
    result = model(warped_rgb)
    num_beetles = result.pandas().xyxy[0].shape[0]
    confidence = result.pandas().xyxy[0]['confidence'].sum()
    # update csv
    csv += f'{path},{trap_label},{num_beetles},{confidence}\n'

# save to csv
csv_path = os.path.join(save_dir, 'output.csv')
with open(csv_path, 'a+') as csvfile:
    csvfile.seek(0)
    if csvfile.readline().strip() == csv_title.strip():
        csvfile.seek(0, io.SEEK_END)
    else:
        csv = csv_title + csv
    csvfile.write(csv)