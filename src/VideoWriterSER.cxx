#include "../headers/VideoWriterSER.h"
#include "../headers/Common.h"

#include <stdexcept>
#include <iostream>

using namespace AstroPhotoStacker;
using namespace std;


VideoWriterSER::VideoWriterSER(const std::string &file_address, const Metadata &first_frame_metadata, unsigned int width, unsigned int height, float fps, unsigned int bit_depth) {
    m_file_address = file_address;
    m_width = width;
    m_height = height;
    m_fps = fps;
    m_bit_depth = bit_depth;
    m_first_frame_metadata = first_frame_metadata;

    if (m_bit_depth != 8 && m_bit_depth != 16) {
        throw runtime_error("Unsupported bit depth for SER writer: " + to_string(m_bit_depth));
    }

    m_file = make_unique<ofstream>(m_file_address, ios::binary | ios::out);
    if (!m_file->is_open()) {
        throw runtime_error("Unable to open file for writing: " + m_file_address);
    }

    write_metadata();
};

void VideoWriterSER::write_frame(const std::vector<unsigned short> &frame_data)     {
    if (frame_data.size() != m_width * m_height) {
        throw runtime_error("Frame data size does not match video resolution in SER writer");
    }

    if (m_bit_depth == 16) {
        m_file->write(reinterpret_cast<const char*>(frame_data.data()), frame_data.size() * sizeof(unsigned short));
    }
    else if (m_bit_depth == 8) {
        std::vector<unsigned char> frame_data_8bit(frame_data.size(), 0);
        for (size_t i = 0; i < frame_data.size(); ++i) {
            frame_data_8bit[i] = static_cast<unsigned char>(frame_data[i] / 256);
        }
        m_file->write(reinterpret_cast<const char*>(frame_data_8bit.data()), frame_data_8bit.size() * sizeof(unsigned char));
    }
    m_frame_count++;
};

void VideoWriterSER::write_frame(const std::vector<unsigned char> &frame_data)   {
    if (frame_data.size() != m_width * m_height) {
        throw runtime_error("Frame data size does not match video resolution in SER writer");
    }

    if (m_bit_depth == 8) {
        m_file->write(reinterpret_cast<const char*>(frame_data.data()), frame_data.size() * sizeof(unsigned char));
    }
    else if (m_bit_depth == 16) {
        std::vector<unsigned short> frame_data_16bit(frame_data.size(), 0);
        for (size_t i = 0; i < frame_data.size(); ++i) {
            frame_data_16bit[i] = static_cast<unsigned short>(frame_data[i] * 256);
        }
        m_file->write(reinterpret_cast<const char*>(frame_data_16bit.data()), frame_data_16bit.size() * sizeof(unsigned short));
    }
    m_frame_count++;
};

void VideoWriterSER::save_file()    {
    if (m_file != nullptr && m_file->is_open()) {
        update_frame_count_in_header();
        m_file->close();
    }
};

int VideoWriterSER::get_int_code_for_bayer_matrix(const std::array<char, 4> &bayer_matrix) {
    if (bayer_matrix == std::array<char,4>{'R','G','G','B'}) {
        return 8;
    }
    else if (bayer_matrix == std::array<char,4>{'G','R','B','G'}) {
        return 9;
    }
    else if (bayer_matrix == std::array<char,4>{'B','G','G','R'}) {
        return 11;
    }
    else if (bayer_matrix == std::array<char,4>{'G','B','R','G'}) {
        return 10;
    }
    else {
        return 0;
    }
};

int VideoWriterSER::get_int_code_for_bayer_matrix(const std::string &bayer_matrix)  {
    if (bayer_matrix.length() == 0) {
        return 0;
    }
    else if (bayer_matrix.length() != 4) {
        throw runtime_error("Invalid bayer matrix string: " + bayer_matrix);
    }
    std::array<char,4> bayer_array = {bayer_matrix[0], bayer_matrix[1], bayer_matrix[2], bayer_matrix[3]};
    return get_int_code_for_bayer_matrix(bayer_array);
};

unsigned long long int VideoWriterSER::unix_time_to_microsoft_time(unsigned int unix_time)   {
    return (static_cast<unsigned long long int>(unix_time) + 62'135'596'800ULL) * 10'000'000ULL; // 62,135,596,800 = number of seconds between 0001-01-01 and 1970-01-01
};

VideoWriterSER::~VideoWriterSER()   {
    save_file();
};

void VideoWriterSER::write_metadata() {
    m_file->seekp(0, ios::beg);
    const unsigned int zero = 0;

    // FILE ID (bytes 0-13)
    const char file_id[14] = {'S','E','R',' ','V','i','d','e','o',' ','F','i','l','e'};
    m_file->write(reinterpret_cast<const char*>(file_id), 14);

    // LuID (bytes 14-17)
    m_file->write(reinterpret_cast<const char*>(&zero), 4);

    // Bayer pattern (bytes 18-21)
    const int bayer_code = get_int_code_for_bayer_matrix(m_first_frame_metadata.bayer_matrix);
    m_file->write(reinterpret_cast<const char*>(&bayer_code), 4);

    // Little endian flag (bytes 22-25)
    const unsigned int little_endian_flag = 0;
    m_file->write(reinterpret_cast<const char*>(&little_endian_flag), 4);

    // Width (bytes 26-29)
    m_file->write(reinterpret_cast<const char*>(&m_width), 4);

    // Height (bytes 30-33)
    m_file->write(reinterpret_cast<const char*>(&m_height), 4);

    // Bit depth (bytes 34-37)
    m_file->write(reinterpret_cast<const char*>(&m_bit_depth), 4);

    // Frame count (bytes 38-41) - initially zero, will be updated later
    m_file->write(reinterpret_cast<const char*>(&zero), 4);

    // OBSERVER - bytes 42-81
    const char observer[40] = {' '};
    m_file->write(reinterpret_cast<const char*>(observer), 40);

    // INSTRUMENT - bytes 82-121
    const string instrument_string = get_instrument_string();
    m_file->write(instrument_string.c_str(), 40);

    // TELESCOPE - bytes 122-161
    const string telescope_string = get_telescope_string();
    m_file->write(telescope_string.c_str(), 40);

    // TIMESTAMP - bytes 162-169
    const unsigned long long int microsoft_time = unix_time_to_microsoft_time(m_first_frame_metadata.timestamp);
    m_file->write(reinterpret_cast<const char*>(&microsoft_time), 8);

    // TIMESTAMP UTC - bytes 162-169
    m_file->write(reinterpret_cast<const char*>(&microsoft_time), 8);
};

void VideoWriterSER::update_frame_count_in_header() {
    m_file->seekp(38, ios::beg);
    m_file->write(reinterpret_cast<const char*>(&m_frame_count), 4);
};

std::string VideoWriterSER::get_instrument_string() {
    // we need something like this: "ASI=ZWO ASI678MCtemp=36.0"
    string result = "ASI=" + m_first_frame_metadata.camera_model;
    if (m_first_frame_metadata.temperature > -273) {
        result += "temp=" + round_and_convert_to_string(m_first_frame_metadata.temperature);
    };

    int max_length = 40;
    if (result.length() > static_cast<size_t>(max_length)) {
        result = result.substr(0, max_length);
    }
    else {
        result.append(max_length - result.length(), ' ');
    }
    return result;

};

std::string VideoWriterSER::get_telescope_string() {
    // we need something like this: "fps=36.23gain=300exp=5.00"
    string result = "fps=" + round_and_convert_to_string(m_fps, 2);
    if (m_first_frame_metadata.iso > 0) {
        result += "gain=" + to_string(m_first_frame_metadata.iso);
    }
    if (m_first_frame_metadata.exposure_time > 0) {
        result += "exp=" + round_and_convert_to_string(m_first_frame_metadata.exposure_time * 1000, 2); // in milliseconds
    }

    int max_length = 40;
    if (result.length() > static_cast<size_t>(max_length)) {
        result = result.substr(0, max_length);
    }
    else {
        result.append(max_length - result.length(), ' ');
    }
    return result;
};