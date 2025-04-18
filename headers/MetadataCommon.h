#pragma once

#include "../headers/Metadata.h"

#include <string>
#include <array>

namespace AstroPhotoStacker {
    int get_unix_timestamp(const std::string &time_string, const std::string timestamp_format = "%Y:%m:%d %H:%M:%S");

    std::string get_string_timestamp_from_unix_time(int unix_time, const std::string timestamp_format = "%Y:%m:%d %H:%M:%S");

    Metadata get_file_creation_timestamp(const std::string &file_address, const Metadata &other_metadata = Metadata());

    /**
     * * Convert a Bayer pattern represented as an array of 4 numbers (0,1,2,3) to a string like "RGGB".
     */
    std::string convert_bayer_int_array_to_string(const std::array<char, 4> &bayer_array);
}