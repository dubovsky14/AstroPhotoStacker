#include "../headers/raw_file_reader.h"
#include "../headers/StarFinder.h"
#include "../headers/ReferencePhotoHandler.h"
#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/ShiftedPhotoHandler.h"

#include <string>
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

#include <filesystem>

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
    try {
    const string &alignment_file = argv[1];
    const string &input_raw_file = argv[2];
    const string &output_png_file = argv[3];

    PhotoAlignmentHandler photo_alignment_handler;
    photo_alignment_handler.ReadFromTextFile(alignment_file);
    float shift_x, shift_y, rot_center_x, rot_center_y, rotation;
    photo_alignment_handler.get_alignment_parameters(input_raw_file, &shift_x, &shift_y, &rot_center_x, &rot_center_y, &rotation);

    ShiftedPhotoHandler shifted_photo_handler(shift_x, shift_y, rot_center_x, rot_center_y, rotation);
    shifted_photo_handler.add_raw_data(input_raw_file);

    int width, height;
    unique_ptr<unsigned short[]> brightness = read_raw_file(input_raw_file, &width, &height);
    memset(&brightness[0], 0, width*height*sizeof(unsigned short));

    for (int y = 0; y < height; y++)  {
        if (y % 100 == 0)   {
            cout << "y = " << y << endl;
        }
        for (int x = 0; x < width; x++)   {
            unsigned int value;
            char color;
            shifted_photo_handler.get_value_by_reference_frame_coordinates(x, y, &value, &color);
            if (color == 0) {
                brightness[y*width + x] = value;
            }
        }
    }

    createImage(&brightness[0], width, height, output_png_file.c_str());


/*
    const string reference_file_address = argv[1];
    const string directory_with_raw_files = argv[2];

    PhotoAlignmentHandler photo_alignment_handler;
    photo_alignment_handler.AlignAllFilesInFolder(reference_file_address, directory_with_raw_files);
    photo_alignment_handler.SaveToTextFile(directory_with_raw_files + "/alignment.txt");

    // list files in the directory
    vector<string> files;
    for (const auto & entry : filesystem::directory_iterator(directory_with_raw_files)) {
        if (entry.path().extension() == ".txt") {
            continue;
        }
        files.push_back(entry.path());
    }
    sort(files.begin(), files.end());


    // read reference file
    ReferencePhotoHandler referencePhotoHandler(reference_file_address);
    for (const std::string &file : files)   {
        float shift_x, shift_y, rot_center_x, rot_center_y, rotation;
        if (referencePhotoHandler.plate_solve(file, &shift_x, &shift_y, &rot_center_x, &rot_center_y, &rotation)) {
            cout << file << " " << shift_x << " " << shift_y << " " << rot_center_x << " " << rot_center_y << " " << rotation << endl;
        }
    }

    ReferencePhotoHandler referencePhotoHandler(argv[1]);
    float shift_x, shift_y, rot_center_x, rot_center_y, rotation;
    referencePhotoHandler.plate_solve(argv[2], &shift_x, &shift_y, &rot_center_x, &rot_center_y, &rotation);
    cout << "Shift: " << shift_x << " " << shift_y << endl;
    cout << "Rotation: " << rotation << endl;
    cout << "Rotation center: " << rot_center_x << " " << rot_center_y << endl;

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
    } catch (const exception &e) {
        cout << e.what() << endl;
    }

    return 0;
}