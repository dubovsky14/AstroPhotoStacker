#include "../headers/raw_file_reader.h"
#include "../headers/StarFinder.h"
#include "../headers/ReferencePhotoHandler.h"

#include <string>
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

using namespace std;

using namespace cv;
using namespace AstroPhotoStacker;

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
    ReferencePhotoHandler referencePhotoHandler(argv[1]);
    cout << "Width: " << referencePhotoHandler.get_width() << endl;
    cout << "Height: " << referencePhotoHandler.get_height() << endl;
    cout << "Number of hashes: " << referencePhotoHandler.get_number_of_hashes() << endl;
    for (unsigned int i_hash = 0; i_hash < referencePhotoHandler.get_number_of_hashes(); i_hash++) {
        tuple<tuple<float,float,float,float>,unsigned int, unsigned int, unsigned int, unsigned int> hash = referencePhotoHandler.get_hash(i_hash);
        tuple <float,float,float,float> coordinates = get<0>(hash);
        cout << "Hash " << i_hash << ": " << endl;
        cout << "\t" << get<0>(coordinates);
        cout << "\t" << get<1>(coordinates);
        cout << "\t" << get<2>(coordinates);
        cout << "\t" << get<3>(coordinates);
        cout << "\tindices: ";
        cout << "\t" << get<1>(hash);
        cout << "\t" << get<2>(hash);
        cout << "\t" << get<3>(hash);
        cout << "\t" << get<4>(hash) << endl;
    }

/*
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
*/

    return 0;
}