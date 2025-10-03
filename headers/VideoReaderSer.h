// description if this format can be found here: https://grischa-hahn.hier-im-netz.de/astro/ser/SER%20Doc%20V3b.pdf

#pragma once

#include "../headers/InputFrame.h"
#include "../headers/Metadata.h"

#include <opencv2/opencv.hpp>

#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <iostream>
#include <fstream>

namespace AstroPhotoStacker {
    unsigned int read_uint_from_file(std::ifstream *file, size_t position_in_file);

    unsigned long long int read_ulonglong_from_file(std::ifstream *file, size_t position_in_file, bool little_endian = true);

    unsigned int microsoft_to_unix_time(unsigned long long int microsoft_time);

    Metadata read_ser_video_metadata(std::ifstream *file);

    Metadata read_ser_video_metadata(const std::string &video_address);

    float get_fps_of_ser_video(const std::string &video_address);

    void get_ser_file_width_and_height(const std::string &video_address, int *width, int *height);

    template<class ValueType>
    std::vector<ValueType> read_ser_video_frame_as_gray_scale(const std::string &video_address, int frame_id, int *width, int *height) {
        std::ifstream file(video_address, std::ios::binary | std::ios::in);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open video file: " + video_address);
        }

        *width = read_uint_from_file(&file, 26);
        *height = read_uint_from_file(&file, 30);
        const unsigned int bit_depth = read_uint_from_file(&file, 34);
        //const unsigned int n_frames = read_uint_from_file(&file, 38);

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

        std::vector<ValueType> result(*width*(*height),0);
        std::transform(result_unsigned_short.begin(), result_unsigned_short.end(), result.begin(), [](unsigned short int x) -> ValueType { return static_cast<ValueType>(x); } );
        return result;
    }

    bool is_ser_file(const std::string &file_address);

}