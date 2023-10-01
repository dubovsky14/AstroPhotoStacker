#include "../headers/raw_file_reader.h"
#include "../headers/StarFinder.h"

#include <string>
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

using namespace std;

using namespace cv;

void createImage(unsigned short* arr, int width, int height, const char* filename) {
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
    unique_ptr<unsigned short[]> brightness = read_raw_file(argv[1], &width, &height);

    const unsigned short threshold = get_threshold_value<unsigned short>(&brightness[0], width*height, 0.0005);
    cout << "Threshold: " << threshold << endl;
    vector<tuple<float,float,int> > stars = get_stars(&brightness[0], width, height, threshold);
    keep_only_stars_above_size(&stars, 9);
    sort_stars_by_size(&stars);

    cout << "Number of stars: " << stars.size() << endl;
    cout << "Star coordinates and brightness: " << endl;
    for (unsigned int i = 0; i < stars.size(); i++) {
        cout << get<0>(stars[i]) << " " << get<1>(stars[i]) << " " << get<2>(stars[i]) << endl;
    }

    std::cout << "Image width: " << width << std::endl;
    std::cout << "Image height: " << height << std::endl;

    createImage(&brightness[0], width, height, "brightness.png");


    return 0;
}