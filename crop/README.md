The different files and directories contain the images and code necessary to construct the western corn rootworm (WCR) simulated dataset using the images given in the [Roboflow dataset](https://universe.roboflow.com/wcr-beetle-detection/wcr-beetle-traps). The content of each file is summarized below.

- `beetles`: Directory of cropped WCR beetles taken from the cropped traps
- `cropped_imgs`: Directory of cropped traps, containing both WCR beetles and insects
- `imgs`: Directory of uncropped traps, containing both WCR beetles and insects
- `non_beetles`: Directory of cropped various insects, not including WCR beetles, taken from the cropped traps
- `SimData.ipynb`: Constructs the simulated dataset using the cropped traps, beetles, and insects in the various folders
- `autocrop.py`: Performs cropping of an image using Otsu's Method, cropping only one image. To crop an image, setup the required environment and run the following command:
```
python3 autocrop.py imgpath destpath [sensitivity=110] [debug=False]
```

The available options are:
```
  imgpath               path to image file
  destpath		path to destination file, including new filename
  sensitivity		sensitivity of foreground vs background
  debug		        saves additional images to folder (polygon and mask of foreground pixels)
```
*Note*: To perform autocropping, delete lines 111 and 133. These were implemented to be able to be used in `transfer.py`. 
- `autocrop_folder.py`: Performs cropping of an image using Otsu's Method, cropping a whole folder. To crop a folder, setup the required environment and run the following command:
```
python3 autocrop_folder.py imgfolder destfolder [sensitivity=110] [debug=False]
```
The available options are:
```
  imgfolder             path to the folder of images
  destfolder		path to destination folder
  sensitivity		sensitivity of foreground vs background
  debug		        saves additional images to folder (polygon and mask of foreground pixels)  
```
*Note*: When one file fails to autcrop due to needing a higher or lower sensitivity, the whole program stops. In this case, it is better to move it out of the folder and correctly crop it with a new sensitivity using `autocrop.py` and `debug=True`. Then, you can use `autocrop_folder.py` again.
