#include "../headers/ZWOVideoTextFileInfo.h"
#include "../headers/Common.h"
#include "../headers/MetadataCommon.h"

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace AstroPhotoStacker;

ZWOVideoTextFileInfo::ZWOVideoTextFileInfo(const std::string &input_file) {
    std::ifstream file(input_file);
    set_timestamp_string(input_file);

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + input_file);
    }
    read_data(&file);
}

Metadata ZWOVideoTextFileInfo::get_metadata() const {
    Metadata metadata;
    metadata.exposure_time = m_exposure_time;
    metadata.iso = m_gain;
    metadata.timestamp = m_unix_time;
    metadata.date_time = m_timestamp_string;
    metadata.is_raw = true;

    if (m_bayer_matrix[0] >= 0) {
        metadata.bayer_matrix = convert_bayer_int_array_to_string(m_bayer_matrix);
    }

    return metadata;
};

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

void ZWOVideoTextFileInfo::set_timestamp_string(const std::string &input_file)   {
    const string file_name = split_string(input_file, "/").back();
    // name of file format: 2024-12-13-200104-something

    const vector<string> elements = split_string(file_name, "-");
    if (elements.size() < 4) {
        return;
    }

    const string &original_daytime_string = elements[3];
    if (original_daytime_string.length() != 6) {
        return;
    }

    const string daytime_string = original_daytime_string.substr(0, 2) + ":" + original_daytime_string.substr(2, 2) + ":" + original_daytime_string.substr(4, 2);

    m_timestamp_string = elements[0] + "-" + elements[1] + "-" + elements[2] + " " + daytime_string;

    m_unix_time = get_unix_timestamp(m_timestamp_string, "%Y-%m-%d %H:%M:%S");
};