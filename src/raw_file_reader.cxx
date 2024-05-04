#include "../headers/raw_file_reader.h"

#include <libraw/libraw.h>

using namespace std;
using namespace AstroPhotoStacker;

std::tuple<float, float, int, float> AstroPhotoStacker::read_metadata(const std::string &raw_file_address) {
    // create a LibRaw object
    LibRaw raw_processor;

    // open the file
    if (raw_processor.open_file(raw_file_address.c_str()) != LIBRAW_SUCCESS) {
        throw std::runtime_error("Cannot open raw file " + raw_file_address);
    }

    // get the exif data
    float aperture      = raw_processor.imgdata.other.aperture;
    float exposure_time = raw_processor.imgdata.other.shutter;
    int iso             = raw_processor.imgdata.other.iso_speed;
    float focal_length  = raw_processor.imgdata.other.focal_len;

    // close the file
    raw_processor.recycle();

    return std::make_tuple(aperture, exposure_time, iso, focal_length);
};

std::vector<char> AstroPhotoStacker::get_color_info_as_char_vector(const std::string &raw_file)   {
    vector<char> result;
    LibRaw raw_processor;
    raw_processor.open_file(raw_file.c_str());
    raw_processor.unpack();
    raw_processor.raw2image();
    int width, height, n_colors, bps;
    raw_processor.get_mem_image_format(&width, &height, &n_colors, &bps);

    for (unsigned int i_color = 0; i_color < 4; i_color++) {
        result.push_back(raw_processor.imgdata.idata.cdesc[i_color]);
    }

    raw_processor.recycle();
    return result;
};

std::vector<char> AstroPhotoStacker::get_color_info_as_number(const std::string &raw_file)   {
    vector<char> colors_letters = get_color_info_as_char_vector(raw_file);
    vector<char> result;
    for (char color_letter : colors_letters) {
        if (color_letter == 'R')    {
                result.push_back(0);
        }
        else if (color_letter == 'G')   {
                result.push_back(1);
        }
        else if (color_letter == 'B')   {
                result.push_back(2);
        }
        else {
            throw runtime_error("Unknown color letter " + color_letter);
        }
    }
    return result;
};


bool AstroPhotoStacker::get_photo_resolution(const std::string &raw_file, int *width, int *height) {
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
