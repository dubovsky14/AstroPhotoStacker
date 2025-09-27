// description if this format can be found here: https://grischa-hahn.hier-im-netz.de/astro/ser/SER%20Doc%20V3b.pdf

#pragma once

#include "../headers/InputFrame.h"

#include <opencv2/opencv.hpp>

#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <iostream>
#include <fstream>

namespace AstroPhotoStacker {
    void read_uint_from_file(std::ifstream *file, unsigned int *value, size_t position_in_file);

    template<class ValueType>
    std::vector<ValueType> read_ser_video_frame_as_gray_scale(const std::string &video_address, int frame_id, int *width, int *height) {
        //const size_t file_size = std::filesystem::file_size(std::filesystem::path(video_address));

        std::ifstream file(video_address, std::ios::binary | std::ios::in);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open video file: " + video_address);
        }

        unsigned int width_uint, height_uint;
        read_uint_from_file(&file, &width_uint, 26);
        read_uint_from_file(&file, &height_uint, 30);
        *width = static_cast<int>(width_uint);
        *height = static_cast<int>(height_uint);
        unsigned int n_frames = 0;
        read_uint_from_file(&file, &n_frames, 38);

        const size_t frame_size = 2*(*width)*(*height);
        const size_t header_size = 178;

        file.seekg(header_size + frame_id*frame_size, std::ios::beg);

        std::vector<short unsigned int> result_unsigned_short(*width*(*height),0);

        file.read(reinterpret_cast<char*>(result_unsigned_short.data()), frame_size);

        std::transform(result_unsigned_short.begin(), result_unsigned_short.end(), result_unsigned_short.begin(), [](unsigned short int x) -> unsigned short {return x/2;});

        std::vector<ValueType> result(*width*(*height),0);
        std::transform(result_unsigned_short.begin(), result_unsigned_short.end(), result.begin(), [](unsigned short int x) -> ValueType { return static_cast<ValueType>(x); } );
        return result;
    }

}