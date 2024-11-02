#include "../headers/HotPixelIdentifier.h"
#include "../headers/InputArgumentsParser.h"
#include "../headers/Common.h"

#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <thread>

using namespace std;
using namespace AstroPhotoStacker;


int main(int argc, const char **argv) {
    try {
        InputArgumentsParser input_arguments_parser(argc, argv);
        const string directory_with_raw_files   = input_arguments_parser.get_argument<string>("raw_files_dir");
        const string hot_pixels_file            = input_arguments_parser.get_optional_argument<string>("hot_pixels_file", directory_with_raw_files + "/hot_pixels.txt");

        const unsigned int number_of_available_CPUs = thread::hardware_concurrency()/2 != 0 ? thread::hardware_concurrency()/2 : 1;
        const unsigned int n_cpu                = input_arguments_parser.get_optional_argument<unsigned int>("n_cpu", number_of_available_CPUs);

        const vector<string> raw_files = get_frame_files_in_folder(directory_with_raw_files);
        vector<InputFrame> input_frames;
        for (const string &raw_file : raw_files) {
            input_frames.push_back(InputFrame(raw_file));
        }

        HotPixelIdentifier hot_pixel_identifier;
        hot_pixel_identifier.set_n_cpu(n_cpu);
        hot_pixel_identifier.add_photos(input_frames);
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
