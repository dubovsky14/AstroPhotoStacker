#include "../headers/ZWOVideoTextFileInfo.h"
#include "../headers/Common.h"

#include <vector>

using namespace std;
using namespace AstroPhotoStacker;

ZWOVideoTextFileInfo::ZWOVideoTextFileInfo(const std::string &input_file) {
    std::ifstream file(input_file);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + input_file);
    }
    read_data(&file);
}

void ZWOVideoTextFileInfo::read_data(std::ifstream *file)   {
    std::string line;
    while (std::getline(*file, line)) {
        vector<string> elements = split_string(line, "=");
        if (elements.size() != 2) {
            continue;
        }

        strip_string(&elements[0]);
        strip_string(&elements[1]);

        const string key = elements[0];
        const string value = elements[1];

        if (compare_case_insensitive(key, "Gain")) {
            m_gain = convert_string_to<int>(value);
        }
        else if (compare_case_insensitive(key, "Exposure")) {
            m_exposure_time = convert_string_to<float>(value);
        }
        else if (compare_case_insensitive(key, "Debayer Type")) {
            read_bayer_matrix(value);
        }
    }
};

void ZWOVideoTextFileInfo::read_bayer_matrix(const std::string &matrix_string)  {
    if (matrix_string.size() != 4) {
        throw std::runtime_error("Invalid bayer matrix string");
    }

    for (int i = 0; i < 4; i++) {
        char color_code = -1;
        if (matrix_string[i] == 'R') {
            color_code = 0;
        }
        else if (matrix_string[i] == 'G') {
            color_code = 1;
        }
        else if (matrix_string[i] == 'B') {
            color_code = 2;
        }
        else {
            throw std::runtime_error("Invalid color code in bayer matrix string");
        }

        m_bayer_matrix[i] = color_code;
    }
};
