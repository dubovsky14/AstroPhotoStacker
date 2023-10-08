#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/InputArgumentsParser.h"

#include <string>
#include <iostream>

using namespace std;
using namespace AstroPhotoStacker;

int main(int argc, const char **argv) {

    try {
        InputArgumentsParser input_arguments_parser(argc, argv);

        const string reference_file_address     = input_arguments_parser.get_argument<string>("reference_file");
        const string directory_with_raw_files   = input_arguments_parser.get_argument<string>("raw_files_dir");
        const unsigned int n_cpu                = input_arguments_parser.get_optional_argument<unsigned int>("n_cpu", 8);
        string output_alignment_file            = input_arguments_parser.get_optional_argument<string>("alignment_file","");
        if (output_alignment_file == "")        {
            output_alignment_file = directory_with_raw_files + "/alignment.txt";
        }


        cout << "\n";
        cout << "Reference file: " << reference_file_address << "\n";
        cout << "Directory with raw files: " << directory_with_raw_files << "\n";
        cout << "Output alignment file: " << output_alignment_file << "\n";
        cout << "Number of CPU threads: " << n_cpu << "\n";
        cout << "\n";

        PhotoAlignmentHandler photo_alignment_handler;
        photo_alignment_handler.set_number_of_cpu_threads(n_cpu);
        photo_alignment_handler.align_all_files_in_folder(reference_file_address, directory_with_raw_files);
        photo_alignment_handler.save_to_text_file(output_alignment_file);
    }
    catch (const exception &e) {
        cout << e.what() << endl;
        abort();
    }
}