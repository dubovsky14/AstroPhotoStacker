#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/InputFrameReader.h"
#include "../headers/StackerFactory.h"
#include "../headers/StackerKappaSigmaBase.h"
#include "../headers/StackerMeanValue.h"
#include "../headers/InputArgumentsParser.h"
#include "../headers/PhotoRanker.h"
#include "../headers/FlatFrameHandler.h"

#include <thread>
#include <string>
#include <iostream>
#include <memory>
#include <tuple>
#include <opencv2/opencv.hpp>

using namespace std;

using namespace cv;
using namespace AstroPhotoStacker;

tuple<int, float> get_nfiles_or_fraction_of_files(const InputArgumentsParser &input_parser);
void configure_stacker_with_optional_arguments(StackerBase *stacker, const InputArgumentsParser &input_parser, bool print_info = true);

int main(int argc, const char **argv) {
    try {
        InputArgumentsParser input_arguments_parser(argc, argv);

        const string alignment_file     = input_arguments_parser.get_argument<string>("alignment_file");
        const string output_file        = input_arguments_parser.get_argument<string>("output");
        const string stacker_type       = input_arguments_parser.get_optional_argument<string>("stacker_type", "kappa-sigma median");

        cout << "\n";
        cout << "Alignment file: " << alignment_file << "\n";
        cout << "Output file: " << output_file << "\n";
        cout << "Stacking algorithm: " << stacker_type << "\n";

        PhotoAlignmentHandler photo_alignment_handler;
        photo_alignment_handler.read_from_text_file(alignment_file);

        // limiting number of files to be stacked
        auto [n_files, fraction_of_files] = get_nfiles_or_fraction_of_files(input_arguments_parser);
        if (n_files > 0)                photo_alignment_handler.limit_number_of_files(n_files);
        if (fraction_of_files > 0.0)    photo_alignment_handler.limit_fraction_of_files(fraction_of_files);

        // loading input files
        const vector<string> input_files = photo_alignment_handler.get_file_addresses();
        if (input_files.size() == 0) {
            throw runtime_error("No input files found in the alignment file");
        }

        // photo resolution
        int width, height;
        InputFrame input_frame(input_files[0]);
        InputFrameReader input_frame_reader(input_frame);
        input_frame_reader.get_photo_resolution(&width, &height);
        cout << "Photo resolution: " << width << "x" << height << "\n";

        // getting correct stacker instance and configuring it
        unique_ptr<StackerBase> stacker = create_stacker(stacker_type, 3, width, height, false);
        stacker->add_alignment_text_file(alignment_file);
        configure_stacker_with_optional_arguments(stacker.get(), input_arguments_parser);

        // flat frame
        const string flat_frame_file    = input_arguments_parser.get_optional_argument<string>("flat_frame", "");
        vector<shared_ptr<const CalibrationFrameBase> > calibration_frame_handlers;
        if (flat_frame_file != "")  {
            std::shared_ptr<const CalibrationFrameBase> flat_frame_handler = make_shared<FlatFrameHandler>(InputFrame(flat_frame_file));
            calibration_frame_handlers.push_back(flat_frame_handler);
            cout << "Flat frame file: " << flat_frame_file << "\n";
        }

        // adding files to stacker and stacking them
        for (const string &file : input_files) {
            stacker->add_photo(InputFrame(file), calibration_frame_handlers, true);
        }
        stacker->calculate_stacked_photo();
        stacker->save_stacked_photo(output_file, CV_16UC3);

        return 0;
    }
    catch (const exception &e) {
        cout << endl << endl;
        cout << e.what() << endl;
        abort();
    }
    return 0;
}


void configure_stacker_with_optional_arguments(StackerBase *stacker, const InputArgumentsParser &input_parser, bool print_info)  {
    // hot pixels
    const string hot_pixels_file    = input_parser.get_optional_argument<string>("hot_pixels_file", "");
    if (hot_pixels_file != "")  {
        stacker->register_hot_pixels_file(hot_pixels_file);
        if (print_info) cout << "Hot pixels file: " << hot_pixels_file << "\n";
    }


    const unsigned int number_of_available_CPUs = max<int>(thread::hardware_concurrency()/2,1);
    const unsigned int n_cpu                    = input_parser.get_optional_argument<unsigned int>("n_cpu", number_of_available_CPUs);
    stacker->set_number_of_cpu_threads(n_cpu);
    if (print_info) cout << "Number of CPU threads: " << n_cpu << "\n";

    // Memory limit
    if (dynamic_cast<StackerMeanValue*>(stacker) == nullptr) {
        const unsigned int memory_limit             = input_parser.get_optional_argument<unsigned int>("memory_limit", 16000);
        stacker->set_memory_usage_limit(memory_limit);
        if (print_info) cout << "Memory limit: " << memory_limit << "\n";
    }


    const std::string algorithm_specifict_settings_string = input_parser.get_optional_argument<std::string>("algorithm_specific_settings", "");
    if (algorithm_specifict_settings_string != "")   {
        std::map<std::string, double> algorithm_specific_settings;
        vector<string> settings = split_string(algorithm_specifict_settings_string, ";");
        for (const string &setting : settings) {
            vector<string> key_value = split_string(setting, "=");
            if (key_value.size() == 2) {
                const string &key = key_value[0];
                const double value = stod(key_value[1]);
                algorithm_specific_settings[key] = value;
            }
        }
        stacker->get_configurable_algorithm_settings().configure_with_settings_numerical(algorithm_specific_settings);
        if (print_info) {
            cout << "Algorithm specific settings:\n";
            for (const auto &pair : algorithm_specific_settings) {
                cout << "  " << pair.first << " = " << pair.second << "\n";
            }
        }
    }

    cout << "\n";
}

tuple<int, float> get_nfiles_or_fraction_of_files(const InputArgumentsParser &input_parser) {
    const int n_files              = input_parser.get_optional_argument<int>  ("n_files", -1);
    const float fraction_of_files  = input_parser.get_optional_argument<float>("fraction_of_files", -1.0);
    if (n_files >= 0 && fraction_of_files >= 0.0) {
        throw runtime_error("Only one of n_files and fraction_of_files can be set");
    }
    return make_tuple(n_files, fraction_of_files);
}