#include "../headers/raw_file_reader.h"
#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/StackerFactory.h"
#include "../headers/InputArgumentsParser.h"

#include <string>
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

using namespace std;

using namespace cv;
using namespace AstroPhotoStacker;

void configure_stacker_with_optional_arguments(StackerBase *stacker, const InputArgumentsParser &input_parser, bool print_info = true)  {
    const string flat_frame_file    = input_parser.get_optional_argument<string>("flat_frame", "");
    if (flat_frame_file != "")  {
        stacker->add_flat_frame(flat_frame_file);
        if (print_info) cout << "Flat frame file: " << flat_frame_file << "\n";
    }

    // StackerMean does not support memory and n_cpu configuration
    if (dynamic_cast<StackerMeanValue*>(stacker) == nullptr) {
        const unsigned int memory_limit = input_parser.get_optional_argument<unsigned int>("memory_limit", 16000);
        const unsigned int n_cpu        = input_parser.get_optional_argument<unsigned int>("n_cpu", 8);

        stacker->set_memory_usage_limit(memory_limit);
        stacker->set_number_of_cpu_threads(n_cpu);

        if (print_info) cout << "Memory limit: " << memory_limit << "\n";
        if (print_info) cout << "Number of CPU threads: " << n_cpu << "\n";
    }

    StackerKappaSigmaBase *stacker_kappa_sigma = dynamic_cast<StackerKappaSigmaBase*>(stacker);
    if (stacker_kappa_sigma != nullptr) {
        const float kappa = input_parser.get_optional_argument<float>("kappa", 3.0);
        const int n_iterations = input_parser.get_optional_argument<int>("n_iterations", 3);

        stacker_kappa_sigma->set_kappa(kappa);
        stacker_kappa_sigma->set_number_of_iterations(n_iterations);

        if (print_info) cout << "Kappa: " << kappa << "\n";
        if (print_info) cout << "Number of iterations: " << n_iterations << "\n";
    }
    cout << "\n";
}

int main(int argc, const char **argv) {
    try {
        InputArgumentsParser input_arguments_parser(argc, argv);

        const string alignment_file     = input_arguments_parser.get_argument<string>("alignment_file");
        const string output_file        = input_arguments_parser.get_argument<string>("output");
        const string stacker_type       = input_arguments_parser.get_optional_argument<string>("stacker_type", "kappa_sigma_clipping");

        cout << "\n";
        cout << "Alignment file: " << alignment_file << "\n";
        cout << "Output file: " << output_file << "\n";
        cout << "Stacking algorithm: " << stacker_type << "\n";

        PhotoAlignmentHandler photo_alignment_handler;
        photo_alignment_handler.read_from_text_file(alignment_file);
        vector<string> input_files = photo_alignment_handler.get_file_addresses();
        if (input_files.size() == 0) {
            throw runtime_error("No input files found in the alignment file");
        }

        int width, height;
        get_photo_resolution(input_files[0], &width, &height);
        cout << "Photo resolution: " << width << "x" << height << "\n";

        unique_ptr<StackerBase> stacker = create_stacker(stacker_type, 3, width, height);
        stacker->add_alignment_text_file(alignment_file);
        configure_stacker_with_optional_arguments(stacker.get(), input_arguments_parser);

        for (const string &file : input_files) {
            stacker->add_photo(file);
        }
        stacker->calculate_stacked_photo();
        stacker->stretch_stacked_photo(StretchingType::logarithmic);
        stacker->stretch_stacked_photo(StretchingType::quadratic);
        stacker->save_stacked_photo(output_file, CV_16UC3);

        return 0;

    } catch (const exception &e) {
        cout << e.what() << endl;
        abort();
    }

    return 0;
}