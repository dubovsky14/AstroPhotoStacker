#include "../headers/VideoReaderSer.h"

#include "../headers/Common.h"

using namespace AstroPhotoStacker;
using namespace std;

unsigned int AstroPhotoStacker::read_uint_from_file(std::ifstream *file, size_t position_in_file)  {
    file->seekg(position_in_file, std::ios::beg);
    unsigned int value;
    file->read(reinterpret_cast<char*>(&value), sizeof(unsigned int));
    return value;
};

unsigned long long int AstroPhotoStacker::read_ulonglong_from_file(std::ifstream *file, size_t position_in_file, bool little_endian)    {
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

unsigned int AstroPhotoStacker::microsoft_to_unix_time(unsigned long long int microsoft_time) {
    return static_cast<unsigned int>((microsoft_time / 10'000'000ULL) - 62'135'596'800ULL); // 62,135,596,800 = number of seconds between 0001-01-01 and 1970-01-01
}

Metadata AstroPhotoStacker::read_ser_video_metadata(const std::string &video_address) {
    std::ifstream file(video_address, std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open video file: " + video_address);
    }
    return read_ser_video_metadata(&file);

};

float AstroPhotoStacker::get_fps_of_ser_video(const std::string &video_address)    {
    std::ifstream file(video_address, std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open video file: " + video_address);
    }

    char string_buffer[40];
    // metadata in format "fps=36.23gain=300exp=5.00             "
    file.seekg(82, std::ios::beg);
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
            reading_value = false;
        }
    }
    float fps = 0;
    for (const pair<string, float> &item : metadata_items) {
        if (item.first == "fps") {
            fps = item.second;
        }
    }
    return fps;
};

Metadata AstroPhotoStacker::read_ser_video_metadata(std::ifstream *file) {
    Metadata metadata;
    metadata.is_raw = true;

    const unsigned long long int microsoft_time = read_ulonglong_from_file(file, 162);
    metadata.timestamp = microsoft_to_unix_time(microsoft_time);

    char string_buffer[40];

    // get camera model, the string is in format "  ASI=ZWO ASI678MCtemp=36.0"
    file->seekg(82, std::ios::beg);
    file->read(string_buffer, 40);
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
    file->seekg(122, std::ios::beg);
    file->read(string_buffer, 40);
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
    float fps = -1;
    for (const pair<string, float> &item : metadata_items) {
        if (item.first == "exp") {
            metadata.exposure_time = item.second/1000;
        }
        else if (item.first == "gain") {
            metadata.iso = static_cast<int>(item.second);
        }
        else if (item.first == "fps") {
            fps = item.second;
        }
    }

    // decode bayer matrix
    const unsigned int bayer_pattern_code = read_uint_from_file(file, 18);
    if (bayer_pattern_code == 0) {
        metadata.bayer_matrix = "";
    }
    else if (bayer_pattern_code == 8) {
        metadata.bayer_matrix = "RGGB";
    }
    else if (bayer_pattern_code == 9) {
        metadata.bayer_matrix = "GRBG";
    }
    else if (bayer_pattern_code == 10) {
        metadata.bayer_matrix = "GBRG";
    }
    else if (bayer_pattern_code == 11) {
        metadata.bayer_matrix = "BGGR";
    }
    else {
        cout << "Warning: Unknown bayer pattern code in SER file: " << bayer_pattern_code << endl;
    }

    if (!metadata.bayer_matrix.empty()) {
        metadata.monochrome = false;
    }
    else {
        metadata.monochrome = true;
    }
    return metadata;
}

bool AstroPhotoStacker::is_ser_file(const std::string &file_address)   {
    return ends_with(to_upper_copy(file_address), ".SER");
};

void AstroPhotoStacker::get_ser_file_width_and_height(const std::string &video_address, int *width, int *height) {
    std::ifstream file(video_address, std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open video file: " + video_address);
    }

    *width = read_uint_from_file(&file, 26);
    *height = read_uint_from_file(&file, 30);
};