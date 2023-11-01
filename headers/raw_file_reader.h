#pragma once

#include <memory>
#include <string>
#include <libraw/libraw.h>
#include <iostream>
#include <vector>
#include <map>

namespace AstroPhotoStacker   {

    template<typename output_type = unsigned short>
    std::unique_ptr<output_type[]> read_raw_file(const std::string &raw_file_address, int *width, int *height, std::vector<char> *colors = nullptr)   {
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

        std::unique_ptr<output_type[]> brightness = std::unique_ptr<output_type[]>(new output_type[(*width)*(*height)]);
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
    std::vector<char> get_color_info_as_char_vector(const std::string &raw_file);

    // Standard indexing of colors (from RGB): 0 = red, 1 = green, 2 = blue
    std::vector<char> get_color_info_as_number(const std::string &raw_file);

    bool get_photo_resolution(const std::string &raw_file, int *width, int *height);
}
