#include "../headers/HotPixelIdentifier.h"
#include "../headers/InputArgumentsParser.h"

#include <string>
#include <iostream>
#include <memory>
#include <filesystem>
#include <vector>

using namespace std;
using namespace AstroPhotoStacker;


int main(int argc, const char **argv) {
    try {
        InputArgumentsParser input_arguments_parser(argc, argv);
        const string directory_with_raw_files   = input_arguments_parser.get_argument<string>("raw_files_dir");
        const string hot_pixels_file            = input_arguments_parser.get_optional_argument<string>("hot_pixels_file", directory_with_raw_files + "/hot_pixels.txt");
        const unsigned int n_cpu                = input_arguments_parser.get_optional_argument<unsigned int>("n_cpu", 8);

        vector<string> raw_files;
        for (const auto & entry : filesystem::directory_iterator(directory_with_raw_files)) {
            if (entry.path().extension() == ".txt") {
                continue;
            }
            raw_files.push_back(entry.path());
        }

        HotPixelIdentifier hot_pixel_identifier;
        hot_pixel_identifier.set_n_cpu(n_cpu);
        hot_pixel_identifier.add_photos(raw_files);
        hot_pixel_identifier.compute_hot_pixels();
        hot_pixel_identifier.save_hot_pixels_to_file(hot_pixels_file);

        return 0;

    } catch (const exception &e) {
        cout << endl << endl;
        cout << e.what() << endl;
        abort();
    }

    return 0;
}
