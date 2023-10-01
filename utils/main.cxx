#include "../headers/raw_file_reader.h"


#include <string>
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

using namespace std;

using namespace cv;

void createImage(int* arr, int width, int height, const char* filename) {
    Mat image(height, width, CV_8UC1);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            image.at<uchar>(y, x) = arr[y*width + x]/64;
        }
    }
    imwrite(filename, image);
}

int main(int argc, const char **argv) {

    int width, height;
    unique_ptr<int[]> brightness = read_raw_file<int>(argv[1], &width, &height);

    std::cout << "Image width: " << width << std::endl;
    std::cout << "Image height: " << height << std::endl;

    createImage(&brightness[0], width, height, "brightness.png");


    return 0;
}