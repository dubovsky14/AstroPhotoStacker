#pragma once

#include "../headers/Metadata.h"

#include "../headers/DSLR_and_SLR_raw_file_reader.h"
#include "../headers/FitFileReader.h"
#include "../headers/ZWOVideoTextFileInfo.h"
#include "../headers/VideoReader.h"
#include "../headers/InputFrame.h"
#include "../headers/ZWOVideoTextFileInfo.h"

#include "../headers/Common.h"

#include <memory>
#include <string>
#include <libraw/libraw.h>
#include <iostream>
#include <vector>
#include <map>
#include <tuple>

namespace AstroPhotoStacker   {
    bool is_fit_file(const std::string &file_address);

    /**
     * @brief Read metadata from the raw file
     *
     * @param raw_file_address - path to the raw file
     * @return Metadata - struct containing the aperture, exposure time, ISO, and focal length
    */
    Metadata read_metadata_from_raw_file(const std::string &raw_file_address);

    /**
     * @brief Check if the file is a raw file
     *
     * @param file_address - path to the file
     * @return true - if the file is a raw file
     * @return false - if the file is not a raw file
    */
    bool is_raw_file(const std::string &file_address);

    /**
     * @brief Read raw file and return the brightness values
     *
     * @tparam ValueType - type of the output values
     * @param input_frame - input frame - either a still image or a video frame
     * @param width - pointer to the variable where the width of the photo will be stored
     * @param height - pointer to the variable where the height of the photo will be stored
     * @param colors - pointer to the vector where the color information will be stored
     * @return std::vector<ValueType> - pointer to the array containing the brightness values
    */
    template<typename ValueType = unsigned short>
    std::vector<ValueType> read_raw_file(const InputFrame &input_frame, int *width, int *height, std::vector<char> *colors = nullptr)   {
        const bool is_still_image = input_frame.is_still_image();
        if (is_still_image) {
            const std::string raw_file_address = input_frame.get_file_address();
            if (is_fit_file(raw_file_address))    {
                FitFileReader fit_files_reader(raw_file_address);
                if (colors != nullptr)   {
                    *colors = fit_files_reader.get_colors();
                }
                *width = fit_files_reader.get_width();
                *height = fit_files_reader.get_height();
                return fit_files_reader.get_data_templated<ValueType>();
            }
            else   {
                return read_raw_file_dslr_slr<ValueType>(raw_file_address, width, height, colors);
            }
        }
        else {
            std::vector<ValueType> result = read_one_channel_from_video_frame<ValueType>(input_frame.get_file_address(), input_frame.get_frame_number(), width, height, 0);

            const ZWOVideoTextFileInfo info(input_frame.get_file_address() + ".txt");
            const std::array<char, 4> &bayer_matrix = info.get_bayer_matrix();

            if (colors != nullptr)   {
                colors->resize(result.size());
                for (int i_x = 0; i_x < *width; i_x++) {
                    for (int i_y = 0; i_y < *height; i_y++) {
                        const int index = i_y*(*width) + i_x;
                        (*colors)[index] = bayer_matrix[(i_y%2)*2 + i_x%2];
                    }
                }
            }
            return result;
        }
    };

    /**
     * @brief This is a vector with the same number of elemetns as possible color codes (most ofthen 3 or 4)
     *
     * @param raw_file - path to the raw file
     * @return std::vector<char> - vector of numbers containing the color information for given index of the color (0 = red, 1 = green, 2 = blue)
    */
    std::vector<char> get_color_info_as_number(const InputFrame &input_frame);

    /**
     * @brief Get the resolution of the photo
     *
     * @param raw_file - path to the raw file
     * @param width - pointer to the variable where the width of the photo will be stored
     * @param height - pointer to the variable where the height of the photo will be stored
    */
    bool get_photo_resolution_raw_file(const std::string &raw_file, int *width, int *height);

}
