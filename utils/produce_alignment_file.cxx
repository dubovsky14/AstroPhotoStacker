#include "../headers/PhotoAlignmentHandler.h"

#include <string>
#include <iostream>

using namespace std;
using namespace AstroPhotoStacker;

int main(int argc, const char **argv) {
        if (argc < 3) {
                cout << "Usage: \n" << argv[0] << " <reference_file_address> <directory_with_raw_files> <output_alignment_file (optional)>" << endl;
                return 1;
        }

        const string reference_file_address = argv[1];
        const string directory_with_raw_files = argv[2];
        const string output_alignment_file = argc > 3 ? argv[3] : directory_with_raw_files + "/alignment.txt";

        PhotoAlignmentHandler photo_alignment_handler;
        photo_alignment_handler.align_all_files_in_folder(reference_file_address, directory_with_raw_files);
        photo_alignment_handler.save_to_text_file(output_alignment_file);
}