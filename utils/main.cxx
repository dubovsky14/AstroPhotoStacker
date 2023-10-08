#include "../headers/raw_file_reader.h"
#include "../headers/StarFinder.h"
#include "../headers/ReferencePhotoHandler.h"
#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/FlatFrameHandler.h"
#include "../headers/ImageFilesInputOutput.h"
#include "../headers/StackerMeanValue.h"
#include "../headers/StackerMedian.h"

#include <string>
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

#include <filesystem>

using namespace std;

using namespace cv;
using namespace AstroPhotoStacker;

int main(int argc, const char **argv) {
    try {


        const string alignment_file  = argv[1];
        const string output_png_file = argv[2];
        const string flat_frame_file = argc > 3 ? argv[3] : "";

        PhotoAlignmentHandler photo_alignment_handler;
        photo_alignment_handler.read_from_text_file(alignment_file);
        vector<string> input_files = photo_alignment_handler.get_file_addresses();
        if (input_files.size() == 0) {
            throw runtime_error("No input files found in the alignment file");
        }

        int width, height;
        get_photo_resolution(input_files[0], &width, &height);

        StackerMedian stacker(3, width, height);
        stacker.set_memory_usage_limit(16000);
        stacker.set_number_of_cpu_threads(8);
        stacker.add_alignment_text_file(alignment_file);
        if (flat_frame_file != "")  {
            stacker.add_flat_frame(flat_frame_file);
        }

        for (const string &file : input_files) {
            stacker.add_photo(file);
        }
        stacker.calculate_stacked_photo();
        stacker.save_stacked_photo(output_png_file, CV_16UC3);

        return 0;

    } catch (const exception &e) {
        cout << e.what() << endl;
    }

    return 0;
}