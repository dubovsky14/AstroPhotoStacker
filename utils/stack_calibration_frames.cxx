#include "../headers/raw_file_reader.h"
#include "../headers/StackerFactory.h"
#include "../headers/StackerMeanValue.h"
#include "../headers/StackerKappaSigmaBase.h"
#include "../headers/InputArgumentsParser.h"
#include "../headers/Common.h"
#include "../headers/FlatFrameHandler.h"
#include "../headers/ImageFilesInputOutput.h"
#include "../headers/InputFrame.h"

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

        const string input_folder       = input_arguments_parser.get_argument<string>("raw_files_dir");
        const string output_file        = input_arguments_parser.get_argument<string>("output");
        const string stacker_type       = input_arguments_parser.get_optional_argument<string>("stacker_type", "kappa-sigma median");

        cout << "\n";
        cout << "Raw files directory: " << input_folder << "\n";
        cout << "Output file: " << output_file << "\n";
        cout << "Stacking algorithm: " << stacker_type << "\n";

        const vector<string> input_files = get_frame_files_in_folder(input_folder);

        // photo resolution
        int width, height;
        get_photo_resolution(InputFrame(input_files[0]), &width, &height);
        cout << "Photo resolution: " << width << "x" << height << "\n";

        // getting correct stacker instance and configuring it
        unique_ptr<StackerBase> stacker = create_stacker(stacker_type, 3, width, height, false);
        configure_stacker_with_optional_arguments(stacker.get(), input_arguments_parser);

        // adding files to stacker and stacking them
        for (const string &file : input_files) {
            stacker->add_photo(InputFrame(file), {}, false);
        }
        stacker->calculate_stacked_photo();
        stacker->save_stacked_photo(output_file, CV_16UC1);

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

    // Number of CPUs and memory limit
    if (dynamic_cast<StackerMeanValue*>(stacker) == nullptr) {
        const unsigned int memory_limit             = input_parser.get_optional_argument<unsigned int>("memory_limit", 16000);
        const unsigned int number_of_available_CPUs = max<int>(thread::hardware_concurrency()/2,1);
        const unsigned int n_cpu                    = input_parser.get_optional_argument<unsigned int>("n_cpu", number_of_available_CPUs);

        stacker->set_memory_usage_limit(memory_limit);
        stacker->set_number_of_cpu_threads(n_cpu);

        if (print_info) cout << "Memory limit: " << memory_limit << "\n";
        if (print_info) cout << "Number of CPU threads: " << n_cpu << "\n";
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
        stacker->configure_stacker(algorithm_specific_settings);
        if (print_info) {
            cout << "Algorithm specific settings:\n";
            for (const auto &pair : algorithm_specific_settings) {
                cout << "  " << pair.first << " = " << pair.second << "\n";
            }
        }
    }
    cout << "\n";
}
