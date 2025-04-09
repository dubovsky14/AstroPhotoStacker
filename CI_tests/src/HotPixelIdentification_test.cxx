#include "../headers/HotPixelIdentification_test.h"

#include "../../headers/HotPixelIdentifier.h"

#include <filesystem>
#include <fstream>
#include <iostream>

using namespace AstroPhotoStacker;
using namespace std;

void AstroPhotoStacker::produce_hot_pixel_file(const std::string &raw_files_folder, const std::string &output_file)    {
    vector<string> raw_files;
    for (const auto & entry : filesystem::directory_iterator(raw_files_folder)) {
        if (entry.path().extension() == ".txt") {
            continue;
        }
        raw_files.push_back(entry.path());
    }

    vector<InputFrame> input_frames;
    for (const string &raw_file : raw_files) {
        input_frames.push_back(InputFrame(raw_file));
    }

    HotPixelIdentifier hot_pixel_identifier;
    hot_pixel_identifier.set_n_cpu(1);
    hot_pixel_identifier.add_photos(input_frames);
    hot_pixel_identifier.compute_hot_pixels();
    hot_pixel_identifier.save_hot_pixels_to_file(output_file);
};

bool AstroPhotoStacker::compare_files(const std::string &file1, const std::string &file2)    {
    ifstream file1_stream(file1);
    ifstream file2_stream(file2);

    string line1;
    string line2;

    while (getline(file1_stream, line1) && getline(file2_stream, line2)) {
        if (line1 != line2) {
            return false;
        }
    }

    return true;
};

TestResult AstroPhotoStacker::hot_pixel_identification_test(const std::string &raw_files_folder, const std::string &output_file, const std::string &reference_output_file)   {
    produce_hot_pixel_file(raw_files_folder, output_file);
    if (!compare_files(output_file, reference_output_file)) {
        return TestResult(false, "Hot pixel identification test failed. Produced hot pixel file does not match reference output file.");
    }
    return TestResult(true, "Hot pixel identification test passed.");
};

//void AstroPhotoStacker::hot_pixel_identification_test1()   {
//    const string raw_files_folder       = "AstroPhotoStacker_test_files/data/CanonEOS6DMarkII_Andromeda/";
//    const string output_file            = "temp_hot_pixel_file.txt";
//    const string reference_output_file  = "AstroPhotoStacker_test_files/data/CanonEOS6DMarkII_Andromeda/reference_files/hot_pixels.txt";
//
//    hot_pixel_identification_test(raw_files_folder, output_file, reference_output_file);
//};