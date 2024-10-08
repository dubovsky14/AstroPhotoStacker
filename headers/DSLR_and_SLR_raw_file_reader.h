#pragma once

#include "../headers/Metadata.h"

#include <memory>
#include <string>
#include <libraw/libraw.h>
#include <iostream>
#include <vector>
#include <map>
#include <tuple>

namespace AstroPhotoStacker   {
    /**
     * @brief Read metadata from the raw file
     *
     * @param raw_file_address - path to the raw file
     * @return Metadata - struct containing the aperture, exposure time, ISO, and focal length
    */
    Metadata read_metadata_from_raw_file_dslr_slr(const std::string &raw_file_address);

    /**
     * @brief Check if the file is a raw file
     *
     * @param file_address - path to the file
     * @return true - if the file is a raw file
     * @return false - if the file is not a raw file
    */
    bool is_raw_file_dslr_slr(const std::string &file_address);

    /**
     * @brief Read raw file and return the brightness values
     *
     * @tparam ValueType - type of the output values
     * @param raw_file_address - path to the raw file
     * @param width - pointer to the variable where the width of the photo will be stored
     * @param height - pointer to the variable where the height of the photo will be stored
     * @param colors - pointer to the vector where the color information will be stored
     * @return std::vector<ValueType> - pointer to the array containing the brightness values
    */
    template<typename ValueType = unsigned short>
    std::vector<ValueType> read_raw_file_dslr_slr(const std::string &raw_file_address, int *width, int *height, std::vector<char> *colors = nullptr)   {
        // create a LibRaw object
        LibRaw raw_processor;

        // open the file
        if (raw_processor.open_file(raw_file_address.c_str()) != LIBRAW_SUCCESS) {
            throw std::runtime_error("Cannot open raw file " + raw_file_address);
        }

        // unpack the raw data
        if (raw_processor.unpack() != LIBRAW_SUCCESS) {
            throw std::runtime_error("Cannot unpack raw data from file " + raw_file_address);
        }

        raw_processor.raw2image();
        raw_processor.subtract_black();

        int col, bps;
        raw_processor.get_mem_image_format(width, height, &col, &bps);

        if (*width < *height)   {
            std::swap(*width, *height);
        }

        std::vector<ValueType> brightness(*width*(*height));
        if (colors != nullptr)   {
            colors->resize((*width)*(*height));
        }
        const unsigned short int (*image_data)[4] = raw_processor.imgdata.image;

        if (colors != nullptr)   {
            for (int row = 0; row < *height; ++row) {
                for (int col = 0; col < *width; ++col) {
                    const unsigned int index = row * (*width) + col;
                    brightness[index] = image_data[index][raw_processor.COLOR(row,col)];
                    (*colors)[index] = raw_processor.COLOR(row,col);
                    if ((*colors)[index] == 3)    {
                        (*colors)[index] = 1;
                    }
                }
            }
        }
        else    {
            for (int row = 0; row < *height; ++row) {
                for (int col = 0; col < *width; ++col) {
                    const unsigned int index = row * (*width) + col;
                    brightness[index] = image_data[index][raw_processor.COLOR(row,col)];
                }
            }
        }

        // close the file
        raw_processor.recycle();

        return brightness;
    };

    /**
     * @brief Get the color information as a vector of characters (R - red, G - green, B - blue)
     *
     * @param raw_file - path to the raw file
     * @return std::vector<char> - vector of characters containing the color information for given index of the color (R = red, G = green, B = blue)
    */
    std::vector<char> get_color_info_as_char_vector_dslr_slr(const std::string &raw_file);

    /**
     * @brief Get the color information as a vector of numbers (0 - red, 1 - green, 2 - blue)
     *
     * @param raw_file - path to the raw file
     * @return std::vector<char> - vector of numbers containing the color information for given index of the color (0 = red, 1 = green, 2 = blue)
    */
    std::vector<char> get_color_info_as_number_dslr_slr(const std::string &raw_file);

    /**
     * @brief Get the resolution of the photo
     *
     * @param raw_file - path to the raw file
     * @param width - pointer to the variable where the width of the photo will be stored
     * @param height - pointer to the variable where the height of the photo will be stored
    */
    bool get_photo_resolution_raw_file_dslr_slr(const std::string &raw_file, int *width, int *height);
}
