#include "../headers/DSLR_and_SLR_raw_file_reader.h"

#include <libraw/libraw.h>
#include <iomanip>
#include <sstream>
#include <algorithm>

using namespace std;
using namespace AstroPhotoStacker;

Metadata AstroPhotoStacker::read_metadata_from_raw_file_dslr_slr(const std::string &raw_file_address) {
    // create a LibRaw object
    LibRaw raw_processor;

    // open the file
    if (raw_processor.open_file(raw_file_address.c_str()) != LIBRAW_SUCCESS) {
        throw std::runtime_error("Cannot open raw file " + raw_file_address);
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

    // close the file
    raw_processor.recycle();

    return result;
};

bool AstroPhotoStacker::is_raw_file_dslr_slr(const std::string &file_address)   {
    LibRaw raw_processor;
    return raw_processor.open_file(file_address.c_str()) == LIBRAW_SUCCESS;
};

bool AstroPhotoStacker::get_photo_resolution_raw_file_dslr_slr(const std::string &raw_file, int *width, int *height) {
    LibRaw raw_processor;
    raw_processor.open_file(raw_file.c_str());
    raw_processor.unpack();
    raw_processor.raw2image();
    int n_colors, bps;
    raw_processor.get_mem_image_format(width, height, &n_colors, &bps);
    raw_processor.recycle();

    if (*width < *height)   {
        std::swap(*width, *height);
    }

    return true;
};
