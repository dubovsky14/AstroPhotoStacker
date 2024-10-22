import cv2
from sys import argv
from os import listdir, path

def make_timelapse_video(folder : str, output_name : str, fps : int = 25, repeat : int = 1):
    image_files = [folder + '/' + f for f in listdir(folder) if f.upper().endswith('.JPG') and path.isfile(folder + '/' + f)]
    image_files.sort()
    images = [cv2.imread(file) for file in image_files]
    height, width, layers = images[0].shape
    video = cv2.VideoWriter(folder + "/" + output_name, cv2.VideoWriter_fourcc(*'XVID'), fps, (width, height))
    if (video.isOpened() == False):
        print("Error opening video stream or file")
    for i in range(repeat):
        for image in images:
            video.write(image)
    video.release()

if __name__ == '__main__':
    make_timelapse_video(argv[1], argv[2], 20, 3)