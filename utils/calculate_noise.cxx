#include "../headers/PixelType.h"
#include "../headers/InputFrameReader.h"
#include "../headers/InputFrame.h"

#include <vector>
#include <filesystem>
#include <iostream>
#include <cmath>

using namespace std;
using namespace AstroPhotoStacker;

int main(int argc, char **argv) {
    if (argc != 2)  {
        cerr << "Usage: " << argv[0] << " <raw_file>" << endl;
        return 1;
    }

    const std::string input_file = argv[1];
    int width, height;

    InputFrame input_frame(input_file);
    InputFrameReader reader(input_frame);
    reader.load_input_frame_data();
    reader.get_photo_resolution(&width, &height);
    vector<PixelType> brightness = reader.get_raw_data();

    double mean = 0;
    double mean2 = 0;
    for (PixelType value : brightness) {
        mean += value;
        mean2 += value*value;
    }

    mean /= brightness.size();
    mean2 /= brightness.size();

    const double variance = mean2 - mean*mean;
    const double std_dev = sqrt(variance);

    cout << "File: " << input_file << "Average noise level: " << mean << "\t\tRMS: " << std_dev << endl;


    return 0;
}