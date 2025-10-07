#include "../headers/RawFileReaderVideoSer.h"
#include "../headers/MetadataCommon.h"
#include "../headers/Common.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;



std::vector<short int> RawFileReaderVideoSer::read_raw_file(int *width, int *height, std::array<char, 4> *bayer_pattern) {
   std::ifstream file(m_input_frame.get_file_address(), std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open video file: " + m_input_frame.get_file_address());
    }


    const int frame_id = m_input_frame.get_frame_number();
    *width = read_uint_from_file(&file, 26);
    *height = read_uint_from_file(&file, 30);
    const unsigned int bit_depth = read_uint_from_file(&file, 34);

    const size_t bytes_per_pixel = bit_depth / 8;
    const size_t frame_size = bytes_per_pixel * (*width) * (*height);
    const size_t header_size = 178;


    file.seekg(header_size + frame_id*frame_size, std::ios::beg);

    std::vector<short unsigned int> result_unsigned_short(*width*(*height),0);

    file.read(reinterpret_cast<char*>(result_unsigned_short.data()), frame_size);

    if (bit_depth == 16) {
        std::transform(result_unsigned_short.begin(), result_unsigned_short.end(), result_unsigned_short.begin(), [](unsigned short int x) -> unsigned short {return x/2;});
    }
    else if (bit_depth == 8) {
        for (int i_pixel = (*width)*(*height)-1; i_pixel >= 0; --i_pixel) {
            result_unsigned_short[i_pixel] = reinterpret_cast<unsigned char*>(result_unsigned_short.data())[i_pixel];
            reinterpret_cast<unsigned char*>(result_unsigned_short.data())[i_pixel] = 0;
        }
    }
    else {
        throw std::runtime_error("Unsupported bit depth in SER file: " + std::to_string(bit_depth));
    }

    if (bayer_pattern != nullptr) {
        const unsigned int bayer_pattern_code = read_uint_from_file(&file, 18);
        const std::string bayer_pattern_string = RawFileReaderVideoSer::int_code_to_bayer_matrix(bayer_pattern_code);
        *bayer_pattern = convert_bayer_string_to_int_array(bayer_pattern_string);

    }

    std::vector<short int> result(*width*(*height),0);
    std::transform(result_unsigned_short.begin(), result_unsigned_short.end(), result.begin(), [](unsigned short int x) -> short int { return static_cast<short int>(x); } );
    return result;
};

void RawFileReaderVideoSer::get_photo_resolution(int *width, int *height) {
   std::ifstream file(m_input_frame.get_file_address(), std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open video file: " + m_input_frame.get_file_address());
    }

    *width = read_uint_from_file(&file, 26);
    *height = read_uint_from_file(&file, 30);
};

Metadata RawFileReaderVideoSer::read_metadata_without_cache() {
    std::ifstream file(m_input_frame.get_file_address(), std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open video file: " + m_input_frame.get_file_address());
    }

    Metadata metadata;
    metadata.is_raw = true;

    const unsigned long long int microsoft_time = read_ulonglong_from_file(&file, 162);
    metadata.timestamp = microsoft_to_unix_time(microsoft_time);

    char string_buffer[40];

    // get camera model, the string is in format "  ASI=ZWO ASI678MCtemp=36.0"
    file.seekg(82, std::ios::beg);
    file.read(string_buffer, 40);
    metadata.camera_model = std::string(string_buffer);
    size_t temp_position = metadata.camera_model.find("temp=");
    if (temp_position != std::string::npos) {
        metadata.camera_model = metadata.camera_model.substr(0, temp_position);
    }
    // drop first "<something>="
    size_t equal_sign_position = metadata.camera_model.find('=');
    if (equal_sign_position != std::string::npos) {
        metadata.camera_model = metadata.camera_model.substr(equal_sign_position + 1);
    }

    // metadata in format "fps=36.23gain=300exp=5.00             "
    file.seekg(122, std::ios::beg);
    file.read(string_buffer, 40);
    std::vector<std::pair<std::string, float>> metadata_items;
    bool reading_value = false;
    std::string value, key;
    for (char &c : string_buffer) {
        if (c == '=')   {
            reading_value = true;
            continue;
        }
        if (!reading_value) {
            key += c;
        }
        else if (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9' || c == '.') {
            value += c;
        }
        else {
            if (!key.empty() && !value.empty()) {
                metadata_items.emplace_back(std::make_pair(key, std::stof(value)));
            }
            key.clear();
            value.clear();
            key += c;
            reading_value = false;
        }
    }
    for (const pair<string, float> &item : metadata_items) {
        if (item.first == "exp") {
            metadata.exposure_time = item.second/1000;
        }
        else if (item.first == "gain") {
            metadata.iso = static_cast<int>(item.second);
        }
        else if (item.first == "fps") {
            metadata.video_fps = item.second;
        }
    }

    // decode bayer matrix
    const unsigned int bayer_pattern_code = read_uint_from_file(&file, 18);
    metadata.bayer_matrix = RawFileReaderVideoSer::int_code_to_bayer_matrix(bayer_pattern_code);

    if (!metadata.bayer_matrix.empty()) {
        metadata.monochrome = false;
    }
    else {
        metadata.monochrome = true;
    }
    return metadata;
}


unsigned int RawFileReaderVideoSer::microsoft_to_unix_time(unsigned long long int microsoft_time) {
    return static_cast<unsigned int>((microsoft_time / 10'000'000ULL) - 62'135'596'800ULL); // 62,135,596,800 = number of seconds between 0001-01-01 and 1970-01-01
}

unsigned int RawFileReaderVideoSer::read_uint_from_file(std::ifstream *file, size_t position_in_file)   const  {
    file->seekg(position_in_file, std::ios::beg);
    unsigned int value;
    file->read(reinterpret_cast<char*>(&value), sizeof(unsigned int));
    return value;
};

unsigned long long int RawFileReaderVideoSer::read_ulonglong_from_file(std::ifstream *file, size_t position_in_file, bool little_endian)    const {
    file->seekg(position_in_file, std::ios::beg);
    unsigned long long int value = 0;
    file->read(reinterpret_cast<char*>(&value), sizeof(unsigned long long int));
    if (!little_endian) {
        unsigned char *x = reinterpret_cast<unsigned char*>(&value);
        std::swap(x[0], x[7]);
        std::swap(x[1], x[6]);
        std::swap(x[2], x[5]);
        std::swap(x[3], x[4]);
    }
    return value;
};

std::string RawFileReaderVideoSer::int_code_to_bayer_matrix(unsigned int code) {
    switch (code) {
        case 0:
            return "";
        case 8:
            return "RGGB";
        case 9:
            return "GRBG";
        case 10:
            return "GBRG";
        case 11:
            return "BGGR";
        default:
            cout << "Warning: Unknown bayer pattern code in SER file: " << code << endl;
            return "";
    }
}

bool AstroPhotoStacker::is_ser_file(const std::string &file_address)   {
    return ends_with(to_upper_copy(file_address), ".SER");
};