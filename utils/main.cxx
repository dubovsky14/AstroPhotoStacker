#include "../headers/raw_file_reader.h"
#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/StackerFactory.h"
#include "../headers/InputArgumentsParser.h"
#include "../headers/PhotoRanker.h"
#include "../headers/FlatFrameHandler.h"

#include <thread>
#include <string>
#include <iostream>
#include <memory>
#include <tuple>
#include <opencv2/opencv.hpp>

#include "../headers/FitFileReader.h"
#include "../headers/ImageFilesInputOutput.h"

using namespace std;

using namespace cv;
using namespace AstroPhotoStacker;

tuple<int, float> get_nfiles_or_fraction_of_files(const InputArgumentsParser &input_parser);
void configure_stacker_with_optional_arguments(StackerBase *stacker, const InputArgumentsParser &input_parser, bool print_info = true);


void scale_to_8_bits(unsigned short *image, int width, int height)   {

    int max_value = 0;
    for (int i_pixel = 0; i_pixel < width*height; i_pixel++) {
        if (image[i_pixel] > max_value) {
            max_value = image[i_pixel];
        }
    }

    max_value = pow(2,16)-1;

    // scale
    const double scale_factor = 255.0/max_value;
    for (int i_pixel = 0; i_pixel < width*height; i_pixel++) {
        image[i_pixel] *= scale_factor;
    }
}

int main(int argc, const char **argv) {

    // picture file
    FitFileReader fit_file_reader(argv[1]);
    const map<string,string> &metadata = fit_file_reader.get_metadata();
    for (const auto & [key, value] : metadata) {
        cout << key << " : " << value << "\n";
    }

    cout << endl << endl;

    cout << "Width: " << fit_file_reader.get_width() << "\n";
    cout << "Height: " << fit_file_reader.get_height() << "\n";
    cout << "Bit depth: " << fit_file_reader.get_bit_depth() << "\n";
    cout << "Exposure time: " << fit_file_reader.get_exposure_time() << "\n";


    vector<unsigned short int> data = fit_file_reader.get_data();

    //for (int x = 0; x < fit_file_reader.get_width(); x++) {
    //    for (int y = 0; y < fit_file_reader.get_height(); y++) {
    //        cout << hex << data[y*fit_file_reader.get_width() + x] << " ";
    //    }
    //    cout << "\n";
    //}

    vector<int> x(3);
    decrease_image_bit_depth(x.data(), x.size(), 8);

    //scale_to_8_bits(data.data(), fit_file_reader.get_width(), fit_file_reader.get_height());
    decrease_image_bit_depth(data.data(), fit_file_reader.get_width()*fit_file_reader.get_height(), 8);
    create_gray_scale_image(data.data(), fit_file_reader.get_width(), fit_file_reader.get_height(), "test.png");

    return 0;


    try {
        InputArgumentsParser input_arguments_parser(argc, argv);

        const string alignment_file     = input_arguments_parser.get_argument<string>("alignment_file");
        const string output_file        = input_arguments_parser.get_argument<string>("output");
        const string stacker_type       = input_arguments_parser.get_optional_argument<string>("stacker_type", "kappa_sigma_median");

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
        get_photo_resolution(input_files[0], &width, &height);
        cout << "Photo resolution: " << width << "x" << height << "\n";

        // getting correct stacker instance and configuring it
        unique_ptr<StackerBase> stacker = create_stacker(stacker_type, 3, width, height, false);
        stacker->add_alignment_text_file(alignment_file);
        configure_stacker_with_optional_arguments(stacker.get(), input_arguments_parser);

        // adding files to stacker and stacking them
        for (const string &file : input_files) {
            stacker->add_photo(file);
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
    // flat frame
    const string flat_frame_file    = input_parser.get_optional_argument<string>("flat_frame", "");
    if (flat_frame_file != "")  {
        std::shared_ptr<const CalibrationFrameBase> flat_frame_handler = make_shared<FlatFrameHandler>(flat_frame_file);
        stacker->add_calibration_frame_handler(flat_frame_handler);
        if (print_info) cout << "Flat frame file: " << flat_frame_file << "\n";
    }

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

    // Setting kappa and number of iterations for kappa-sigma algorithms
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

tuple<int, float> get_nfiles_or_fraction_of_files(const InputArgumentsParser &input_parser) {
    const int n_files              = input_parser.get_optional_argument<int>  ("n_files", -1);
    const float fraction_of_files  = input_parser.get_optional_argument<float>("fraction_of_files", -1.0);
    if (n_files >= 0 && fraction_of_files >= 0.0) {
        throw runtime_error("Only one of n_files and fraction_of_files can be set");
    }
    return make_tuple(n_files, fraction_of_files);
}