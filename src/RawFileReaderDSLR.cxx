#include "../headers/RawFileReaderDSLR.h"


#include <libraw/libraw.h>
#include <iomanip>
#include <sstream>
#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;



std::vector<short int> RawFileReaderDSLR::read_raw_file(int *width, int *height, std::array<char, 4> *bayer_pattern) {
    LibRaw raw_processor = get_libraw_processor();

    raw_processor.imgdata.params.use_camera_wb = 1;
    raw_processor.subtract_black();
    raw_processor.dcraw_process();

    int num_colors, bps;
    raw_processor.get_mem_image_format(width, height, &num_colors, &bps);

    if (*width < *height)   {
        std::swap(*width, *height);
    }

    auto process_color = [](int color_code) -> char {
        if (color_code == 3)    {
            return 1;
        }
        return color_code;
    };

    if (bayer_pattern != nullptr)   {
        bayer_pattern->at(0) = process_color(raw_processor.COLOR(0,0));
        bayer_pattern->at(1) = process_color(raw_processor.COLOR(0,1));
        bayer_pattern->at(2) = process_color(raw_processor.COLOR(1,0));
        bayer_pattern->at(3) = process_color(raw_processor.COLOR(1,1));
    }

    std::vector<short int> result((*width)*(*height));
    const unsigned short int (*image_data)[4] = raw_processor.imgdata.image;

    for (int row = 0; row < *height; ++row) {
        for (int col = 0; col < *width; ++col) {
            const unsigned int index = row * (*width) + col;
            const int color = raw_processor.COLOR(row, col);
            result[index] = image_data[index][color]/2; // divide by 2 to get 15-bit values
        }
    }

    raw_processor.recycle();
    return result;
};

void RawFileReaderDSLR::get_photo_resolution(int *width, int *height) {
    LibRaw raw_processor = get_libraw_processor();
    raw_processor.imgdata.params.use_camera_wb = 1;
    raw_processor.subtract_black();
    raw_processor.dcraw_process();

    int num_colors, bps;
    raw_processor.get_mem_image_format(width, height, &num_colors, &bps);

};

LibRaw RawFileReaderDSLR::get_libraw_processor() {
    LibRaw raw_processor;
    if (raw_processor.open_file(m_input_frame.get_file_address().c_str()) != LIBRAW_SUCCESS) {
        throw std::runtime_error("Cannot open the raw file");
    }
    if (raw_processor.unpack() != LIBRAW_SUCCESS) {
        throw std::runtime_error("Cannot unpack the raw file");
    }
    return raw_processor;
};

Metadata RawFileReaderDSLR::read_metadata() {
    // create a LibRaw object
    LibRaw raw_processor;

    // open the file
    if (raw_processor.open_file(m_input_frame.get_file_address().c_str()) != LIBRAW_SUCCESS) {
        throw std::runtime_error("Cannot open raw file " + m_input_frame.get_file_address());
    }

    Metadata result;

    // get the exif data
    result.aperture      = raw_processor.imgdata.other.aperture;
    result.exposure_time = raw_processor.imgdata.other.shutter;
    result.iso           = raw_processor.imgdata.other.iso_speed;
    result.focal_length  = raw_processor.imgdata.other.focal_len;
    result.max_value     = raw_processor.imgdata.color.maximum;
    result.timestamp     = raw_processor.imgdata.other.timestamp;
    result.is_raw        = true;

    // 0th element ir 'R', 1st is 'G', 2nd is 'B', 3rd is 'G'
    const char* color_description = raw_processor.imgdata.idata.cdesc;
    const int bayer_matrix_numbers[4] = {raw_processor.COLOR(0,0), raw_processor.COLOR(0,1), raw_processor.COLOR(1,0), raw_processor.COLOR(1,1)};
    char bayer_matrix_names[4];
    std::transform(bayer_matrix_numbers, bayer_matrix_numbers + 4, bayer_matrix_names, [color_description](int c) {
        return color_description[c];
    });
    std::string bayer_matrix(bayer_matrix_names, 4);
    result.bayer_matrix = bayer_matrix;


    // it's a bit tricky with timestamp
    std::tm* t = std::gmtime(&raw_processor.imgdata.other.timestamp);
    std::stringstream ss; // or if you're going to print, just input directly into the output stream
    ss << std::put_time(t, "%Y-%m-%d %I:%M:%S %p");
    result.date_time     = ss.str();

    result.camera_model   = raw_processor.imgdata.idata.model;

    // close the file
    raw_processor.recycle();

    return result;
}