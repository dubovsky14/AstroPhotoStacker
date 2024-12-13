#pragma once

#include "../headers/Metadata.h"

#include <string>


namespace AstroPhotoStacker {
    int get_unix_timestamp(const std::string &time_string);

    Metadata get_file_creation_timestamp(const std::string &file_address, const Metadata &other_metadata = Metadata());
}