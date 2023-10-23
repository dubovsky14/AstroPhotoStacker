#include "../headers/HotPixelIdentification_test.h"

#include "../../headers/HotPixelIdentifier.h"

#include <filesystem>
#include <fstream>

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

    HotPixelIdentifier hot_pixel_identifier;
    hot_pixel_identifier.set_n_cpu(1);
    hot_pixel_identifier.add_photos(raw_files);
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

void AstroPhotoStacker::hot_pixel_identification_test(const std::string &raw_files_folder, const std::string &output_file, const std::string &reference_output_file)   {
    produce_hot_pixel_file(raw_files_folder, output_file);
    if (!compare_files(output_file, reference_output_file)) {
        throw runtime_error("Hot pixel identification test failed. Produced hot pixel file does not match reference output file.");
    }
};

void AstroPhotoStacker::hot_pixel_identification_test(int argc, const char **argv)  {
    if (argc < 4) {
        throw runtime_error("Invalid input! Four input arguments are required: test type, raw files folder, output file, reference output file");
    }

    const string raw_files_folder = argv[2];
    const string output_file = argv[3];
    const string reference_output_file = argv[4];

    hot_pixel_identification_test(raw_files_folder, output_file, reference_output_file);
};