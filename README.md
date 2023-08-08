# DeepBeetle
This repository was copied from the original repository hosted on [GitLab](https://gitlab.engr.illinois.edu/akhot2/group-01-phys371-sp2023) to preserve this work after we graduate from University of Illinois at Urbana-Champaign. Our student GitLab accounts are [akhot2](https://gitlab.engr.illinois.edu/akhot2) (Ayush Khot), [jl190](https://gitlab.engr.illinois.edu/jl190) (Jeremiah Lin), [smp6](https://gitlab.engr.illinois.edu/smp6) (Samuel Pasquesi), and [pavanp2](https://gitlab.engr.illinois.edu/pavanp2) (Pavan Pavithran).

This was a group project for PHYS371 at the University of Illinois at Urbana-Champaign. This repository contains the necessary code and documents the results of implementing the YOLOv5 model to identify western corn rootworm beetles on yellow sticky traps. The architecture of YOLOv5 is summarized in the [GitHub repository](https://github.com/ultralytics/yolov5).

# Preparing the repository
To train/retrain models and run the notebooks provided with this repository, one needs to create the right environment and install certain dependencies. First `cd` to the project's top directory and do

`export PROJPATH=$PWD`

### Arduino Code and Drone Attachment

The code and files for the data acquisition from the arduino, the design of the PCB, and the enclosure files, are stored in the [`Assemblies and Hardware`](https://github.com/fresleven/DeepBeetle/tree/main/Assemblies%20and%20Hardware) directory. More information about these files is explained in the `README` file provided in [`Assemblies and Hardware`](https://github.com/fresleven/DeepBeetle/tree/main/Assemblies%20and%20Hardware) directory.

### Download Data
The dataset used by these studies has been made available to use by Professor Spencer of the University of Illinois at Urbana-Champaign It is available at https://universe.roboflow.com/wcr-beetle-detection/wcr-beetle-traps. Please use the steps provided on the Roboflow website to download the latest version of the dataset. We used the validation part of this dataset as the test images for our paper. Some of these images were incorrectly cropped, and so we discarded them from our results.

### Make necessary directories
Run the following commands to make the necessary directories
```
cd $PROJPATH
mkdir -p crop/data/train/images
mkdir -p crop/data/train/labels
mkdir -p crop/data/val/images
mkdir -p crop/data/val/labels
mkdir -p crop/data/test/images
mkdir -p crop/data/test/labels
```

### Setup necessary environment! 
The dependencies for the cropping and simulated data are encompassed in the YOLOv5 model requirements. Run the following commands to set up the environment:
```
cd $PROJPATH
cd yolov5_model
pip install -r requirements.txt
```

### Data pre-processing
To preprocess the data for the `YOLOv5` models, please look at the `README` file in [`crop`](https://github.com/fresleven/DeepBeetle/tree/main/crop) folder. 

# Training your own models
For each model architecture, we have trained a number of alternate variants and they are hosted in the [`yolov5_model/runs/train`](https://github.com/fresleven/DeepBeetle/tree/main/yolov5_model/runs/train) directory. If you are interested in training your own models, please follow the instructions in the `README` file given in the [`yolov5_model`](https://github.com/fresleven/DeepBeetle/tree/main/yolov5_model) directory. Please use the `beetles.yaml` as the `.yaml` file to train your own model. An example of training the model is:
```
cd $PROJPATH
cd yolov5_model
python train.py --img 1280 --batch 16 --epochs 100 --data beetles.yaml --weights yolov5x.pt --name NAME --workers 30
```
The model would be then stored in the `yolov5_model/runs/train/NAME` folder.

# Reproducing image classification results
**They rely on avaliability of the pretrained models and the datasets in the way they have been setup in the previous section**. The tested images in the paper are stored in [`yolov5_model/runs/detect`](https://github.com/fresleven/DeepBeetle/tree/main/yolov5_model/runs/detect) directory. You can test unlabeled images by
```
cd $PROJPATH
cd yolov5_model
python test.py --img 1280 --source path/to/images/ --weights path/to/weights/best.pt --name NAME --workers 30
```
For full data acquisition, you can use the [`transfer.py`](https://github.com/fresleven/DeepBeetle/blob/main/transfer/transfer.py) file to output the number of predicted beetles as well as the confidence level into an [`output.csv`](https://github.com/fresleven/DeepBeetle/blob/main/transfer/output.csv) file. To run this, run the following commands:
```
cd $PROJPATH
cd transfer
python transfer.py
```
# Reference
The model and drone attachment in this repository are compiled and explained in this paper: [Building an Arduino Drone Attachment to Detect Western Corn Rootworm Beetles Using YOLOv5](https://drive.google.com/file/d/167-oGDc3cTERrExamo15tIQeFQ6fNDwl/view?usp=sharing)

To cite this work, please use the following `bibtex` entry:
```
@misc{khot_lin_pasq_pavan_2023,
  title={Building an Arduino Drone Attachment to Detect Western Corn Rootworm Beetles Using YOLOv5},
  author={Khot, Ayush and  Lin, Jeremiah and Pasquesi, Samuel and Pavithran, Pavan},
  year={2023},
  url={https://gitlab.engr.illinois.edu/akhot2/group-01-phys371-sp2023/}
}
```

Please use the contact information given in  https://universe.roboflow.com/wcr-beetle-detection/wcr-beetle-traps to gain permission to use the dataset in research. To cite this dataset, please use the `bibtex` entry given on the website.


# Source Code
Existing resources from publicly available repositories have been adapted to implement the image cropping and YOLOv5 model. We are greatly thankful to the authors of the following works for making these repositories and resources publicly available. 

- Part of the image cropping code is obtained from: https://stackoverflow.com/questions/60941012/how-do-i-find-corners-of-a-paper-when-there-are-printed-corners-lines-on-paper-i
- Implementation of YOLOv5 has been largely adopted from: https://github.com/ultralytics/yolov5
- Part of the arduino code has been obtained from the examples for the Adafruit libaries

# Contact:
For comments, feedback, and suggestions: Ayush Khot (akhot2@illinois.edu), Jeremiah Lin (jl190@illinois.edu), Samuel Pasquesi (smp6@illinois.edu), and Pavan Pavithran (pavanp2@illinois.edu)
