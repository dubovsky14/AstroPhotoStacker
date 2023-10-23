#pragma once

#include <string>

namespace AstroPhotoStacker {
    void produce_hot_pixel_file(const std::string &raw_files_folder, const std::string &output_file);

    bool compare_files(const std::string &file1, const std::string &file2);

    void hot_pixel_identification_test(const std::string &raw_files_folder, const std::string &output_file, const std::string &reference_output_file);

    void hot_pixel_identification_test(int argc, const char **argv);
}