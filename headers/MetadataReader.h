#pragma once

#include "../headers/Metadata.h"
#include "../headers/raw_file_reader.h"

#include <string>

namespace AstroPhotoStacker {
    Metadata read_metadata_rgb_image(const std::string &input_file);

    Metadata read_metadata(const std::string &input_file);
}