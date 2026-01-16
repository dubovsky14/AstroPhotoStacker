#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/InputArgumentsParser.h"
#include "../headers/InputFrame.h"
#include "../headers/Common.h"
#include "../headers/VideoReader.h"

#include <string>
#include <iostream>
#include <thread>

using namespace std;
using namespace AstroPhotoStacker;

int main(int argc, const char **argv) {

    try {
        InputArgumentsParser input_arguments_parser(argc, argv);

        const string reference_file_address     = input_arguments_parser.get_argument<string>("reference_file");
        const string method                     = input_arguments_parser.get_optional_argument<string>("method", "stars");
        const string directory_with_raw_files   = input_arguments_parser.get_optional_argument<string>("raw_files_dir", "");
        const string file_to_align              = input_arguments_parser.get_optional_argument<string>("file_to_align", "");
        if (directory_with_raw_files == "" && file_to_align == "") {
            throw runtime_error("Either 'raw_files_dir' or 'file_to_align' argument must be provided.");
        }

        const unsigned int number_of_available_CPUs = thread::hardware_concurrency()/2 != 0 ? thread::hardware_concurrency()/2 : 1;
        const unsigned int n_cpu                = input_arguments_parser.get_optional_argument<unsigned int>("n_cpu", number_of_available_CPUs);

        string output_alignment_file            = input_arguments_parser.get_optional_argument<string>("alignment_file","");
        if (output_alignment_file == "")        {
            output_alignment_file = directory_with_raw_files + "/alignment.txt";
        }


        cout << "\n";
        cout << "Reference file: " << reference_file_address << "\n";
        cout << "Alignment method: " << method << "\n";
        cout << "Directory with raw files: " << directory_with_raw_files << "\n";
        cout << "Output alignment file: " << output_alignment_file << "\n";
        cout << "Number of CPU threads: " << n_cpu << "\n";
        cout << "\n";

        const vector<string> reference_frame_parts = split_string(reference_file_address, "|");
        const InputFrame reference_frame(reference_frame_parts[0], reference_frame_parts.size() > 1 ? stoi(reference_frame_parts[1]) : -1);

        PhotoAlignmentHandler photo_alignment_handler;
        photo_alignment_handler.set_alignment_method(method);
        photo_alignment_handler.set_number_of_cpu_threads(n_cpu);

        if (directory_with_raw_files != "") {
            photo_alignment_handler.align_all_files_in_folder(reference_frame, directory_with_raw_files);
        }
        else {
            vector<InputFrame> frames_to_align;
            if (is_valid_video_file(file_to_align))   {
                frames_to_align = get_video_frames(file_to_align);
            }
            else    {
                frames_to_align.push_back(InputFrame(file_to_align));
            }
            photo_alignment_handler.align_files(reference_frame, frames_to_align);
        }
        photo_alignment_handler.save_to_text_file(output_alignment_file);
    }
    catch (const exception &e) {
        cout << e.what() << endl;
        abort();
    }
}