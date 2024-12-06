#include <iostream>
#include <vector>
#include <string>

#include "../headers/RGBAlignmentTool.h"
#include "../headers/InputArgumentsParser.h"

using namespace std;
using namespace AstroPhotoStacker;

std::pair<int,int> convert_string_to_pair(string s) {
    for (char &c : s) {
        if (c == 'm') {
            c = '-';
        }
    }

    cout << "s: " << s << endl;
    const size_t pos = s.find(",");
    if (pos == string::npos) {
        throw runtime_error("Invalid pair format: " + s);
    }

    const string first = s.substr(0, pos);
    const string second = s.substr(pos+1);

    return {stoi(first), stoi(second)};
}

int main(int argc, const char **argv) {
    try {
        InputArgumentsParser input_arguments_parser(argc, argv);

        const string input_file     = input_arguments_parser.get_argument<string>("i");
        const string output_file    = input_arguments_parser.get_argument<string>("o");

        const string shift_red_str  = input_arguments_parser.get_argument<string>("r");
        const string shift_blue_str = input_arguments_parser.get_optional_argument<string>("b", "");

        const pair<int,int> shift_red = convert_string_to_pair(shift_red_str);
        const pair<int,int> shift_blue = shift_blue_str == "" ? pair<int,int>({-shift_red.first, -shift_red.second}) : convert_string_to_pair(shift_blue_str);

        RGBAlignmentTool<unsigned int> rgb_alignment_tool(input_file);
        rgb_alignment_tool.calculate_shifted_image(shift_red, shift_blue);
        rgb_alignment_tool.save_shifted_image(output_file);
    }
    catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}