#include "../headers/SyntheticFlatCreator.h"

#include <iostream>
#include <string>

using namespace std;
using namespace AstroPhotoStacker;


int main (int argc, const char **argv)  {
    if (argc != 3)  {
        cerr << "Usage: create_synthetic_flat <input_file> <output_file>\n";
        return 1;
    }

    const std::string input_file = argv[1];
    const std::string output_file = argv[2];

    SyntheticFlatCreator flat_creator(input_file);
    flat_creator.create_and_save_synthetic_flat(output_file);
}