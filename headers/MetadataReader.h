#pragma once

#include "../headers/Metadata.h"
#include "../headers/raw_file_reader.h"
#include "../headers/InputFrame.h"

#include <string>

namespace AstroPhotoStacker {
    Metadata read_metadata_rgb_image(const std::string &input_file);

    Metadata read_metadata_video(const std::string &input_file);

    Metadata read_metadata(const InputFrame &input_frame);
}