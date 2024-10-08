#include "../headers/raw_file_reader.h"

#include "../headers/FitFileMetadataReader.h"

#include <libraw/libraw.h>
#include <iomanip>
#include <sstream>

using namespace std;
using namespace AstroPhotoStacker;

bool AstroPhotoStacker::is_fit_file(const std::string &file_address)   {
    return ends_with(to_upper_copy(file_address), ".FIT");
};

Metadata AstroPhotoStacker::read_metadata_from_raw_file(const std::string &raw_file_address) {
    if (is_fit_file(raw_file_address)) {
        FitFileMetadataReader fit_file_metadata_reader(raw_file_address);
        return fit_file_metadata_reader.get_metadata();
    }
    return read_metadata_from_raw_file_dslr_slr(raw_file_address);
};

bool AstroPhotoStacker::is_raw_file(const std::string &file_address)   {
    if (is_fit_file(file_address)) {
        return true;
    }
    return is_raw_file_dslr_slr(file_address);
};

std::vector<char> AstroPhotoStacker::get_color_info_as_number(const std::string &raw_file)   {
    if (is_fit_file(raw_file)) {
        return {0,1,2,3};
    }
    return get_color_info_as_number_dslr_slr(raw_file);
};


bool AstroPhotoStacker::get_photo_resolution_raw_file(const std::string &raw_file, int *width, int *height) {
    if (is_fit_file(raw_file)) {
        FitFileMetadataReader fit_file_metadata_reader(raw_file);
        *width = fit_file_metadata_reader.get_width();
        *height = fit_file_metadata_reader.get_height();
        return true;
    }
    return get_photo_resolution_raw_file_dslr_slr(raw_file, width, height);
};
