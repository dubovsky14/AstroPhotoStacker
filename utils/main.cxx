#include "../headers/raw_file_reader.h"
#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/StackerMeanValue.h"
#include "../headers/StackerMedian.h"
#include "../headers/StackerKappaSigmaClipping.h"
#include "../headers/StackerKappaSigmaMedian.h"
#include "../headers/InputArgumentsParser.h"

#include <string>
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

using namespace std;

using namespace cv;
using namespace AstroPhotoStacker;

int main(int argc, const char **argv) {
    try {
        InputArgumentsParser input_arguments_parser(argc, argv);

        const string alignment_file     = input_arguments_parser.get_argument<string>("alignment_file");
        const string output_file        = input_arguments_parser.get_argument<string>("output");
        const string flat_frame_file    = input_arguments_parser.get_optional_argument<string>("flat_frame", "");

        const unsigned int memory_limit = input_arguments_parser.get_optional_argument<unsigned int>("memory_limit", 16000);
        const unsigned int n_cpu        = input_arguments_parser.get_optional_argument<unsigned int>("n_cpu", 8);

        cout << "\n";
        cout << "Alignment file: " << alignment_file << "\n";
        cout << "Output file: " << output_file << "\n";
        cout << "Flat frame file: " << flat_frame_file << "\n";
        cout << "Memory limit: " << memory_limit << "\n";
        cout << "Number of CPU threads: " << n_cpu << "\n";
        cout << "\n";

        PhotoAlignmentHandler photo_alignment_handler;
        photo_alignment_handler.read_from_text_file(alignment_file);
        vector<string> input_files = photo_alignment_handler.get_file_addresses();
        if (input_files.size() == 0) {
            throw runtime_error("No input files found in the alignment file");
        }

        int width, height;
        get_photo_resolution(input_files[0], &width, &height);

        StackerKappaSigmaMedian stacker(3, width, height);
        stacker.set_memory_usage_limit(memory_limit);
        stacker.set_number_of_cpu_threads(n_cpu);
        stacker.add_alignment_text_file(alignment_file);
        if (flat_frame_file != "")  {
            stacker.add_flat_frame(flat_frame_file);
        }

        for (const string &file : input_files) {
            stacker.add_photo(file);
        }
        stacker.calculate_stacked_photo();
        stacker.save_stacked_photo(output_file, CV_16UC3);

        return 0;

    } catch (const exception &e) {
        cout << e.what() << endl;
        abort();
    }

    return 0;
}