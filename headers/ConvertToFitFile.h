#pragma once

#include "../headers/InputFrame.h"

#include <string>

namespace AstroPhotoStacker {
    void convert_to_fit_file(const InputFrame &input_frame, const std::string &output_file, int output_bit_depth);
}