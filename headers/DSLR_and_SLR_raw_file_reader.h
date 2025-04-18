#pragma once

#include "../headers/Metadata.h"

#include <memory>
#include <string>
#include <libraw/libraw.h>
#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include <stdexcept>

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
    std::vector<ValueType> read_raw_file_dslr_slr(const std::string &raw_file_address, int *width, int *height, std::vector<char> *colors)   {
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


        raw_processor.imgdata.params.use_camera_wb = 1;
        raw_processor.subtract_black();
        raw_processor.dcraw_process();

        int num_colors, bps;
        raw_processor.get_mem_image_format(width, height, &num_colors, &bps);

        if (*width < *height)   {
            std::swap(*width, *height);
        }

        std::vector<ValueType> brightness(*width*(*height));
        if (colors != nullptr)   {
            colors->resize((*width)*(*height));
        }
        const unsigned short int (*image_data)[4] = raw_processor.imgdata.image;


        for (int row = 0; row < *height; ++row) {
            for (int col = 0; col < *width; ++col) {
                const unsigned int index = row * (*width) + col;
                const int color = raw_processor.COLOR(row, col);
                brightness[index] = image_data[index][color]/2; // divide by 2 to get 15-bit values
                if (colors != nullptr)  {
                    (*colors)[index] = color;
                    if ((*colors)[index] == 3)    {
                        (*colors)[index] = 1;
                    }
                }
            }
        }

        // close the file
        raw_processor.recycle();

        return brightness;
    };

    /**
     * @brief Get the resolution of the photo
     *
     * @param raw_file - path to the raw file
     * @param width - pointer to the variable where the width of the photo will be stored
     * @param height - pointer to the variable where the height of the photo will be stored
    */
    bool get_photo_resolution_raw_file_dslr_slr(const std::string &raw_file, int *width, int *height);
}
