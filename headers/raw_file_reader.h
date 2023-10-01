#pragma once

#include <memory>
#include <string>
#include <libraw/libraw.h>
#include <iostream>

template<typename output_type = unsigned short>
std::unique_ptr<output_type[]> read_raw_file(const std::string &raw_file_address, int *width, int *height)   {
    // create a LibRaw object
    LibRaw rawProcessor;

    // open the file
    if (rawProcessor.open_file(raw_file_address.c_str()) != LIBRAW_SUCCESS) {
        throw std::runtime_error("Cannot open file " + raw_file_address);
    }

    // unpack the raw data
    if (rawProcessor.unpack() != LIBRAW_SUCCESS) {
        throw std::runtime_error("Cannot unpack raw data from file " + raw_file_address);
    }

    rawProcessor.raw2image();

    int colors, bps;
    rawProcessor.get_mem_image_format(width, height, &colors, &bps);

    std::unique_ptr<output_type[]> brightness = std::unique_ptr<output_type[]>(new output_type[(*width)*(*height)]);
    const unsigned short int (*image_data)[4] = rawProcessor.imgdata.image;
    for (int row = 0; row < *height; ++row) {
        for (int col = 0; col < *width; ++col) {
            const unsigned int index = row * (*width) + col;
            brightness[index] = image_data[index][rawProcessor.COLOR(row,col)];
        }
    }
    std::cout << "bps: " << bps << std::endl;

    // close the file
    rawProcessor.recycle();

    return brightness;
};