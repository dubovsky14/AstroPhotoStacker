/**
 * Short program to convert a raw file or a video frame to a FIT file.
 * Usage: convert_to_fit_file <input_file> <output_fits> <frame_number> <bit depth>
 */

#include "../headers/ConvertToFitFile.h"

#include <string>
#include <iostream>

using namespace std;
using namespace AstroPhotoStacker;

int main(int argc, const char **argv)   {
    if (argc < 5)   {
        cerr << "Usage: convert_to_fit_file <input_file> <output_fits> <frame_number> <bit depth>" << endl;
        return 1;
    }

    const string input_file = argv[1];
    const string output_file = argv[2];
    const int frame_number = stoi(argv[3]);
    const int bit_depth = stoi(argv[4]);

    const InputFrame input_frame = InputFrame(input_file, frame_number);

    convert_to_fit_file(input_frame, output_file, bit_depth);

    return 0;
}