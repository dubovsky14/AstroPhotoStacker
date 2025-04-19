#pragma once

#include "../headers/TestUtils.h"

#include "../..//headers/InputFrame.h"
#include "../../headers/Metadata.h"

#include <string>

namespace AstroPhotoStacker {
    TestResult test_if_two_light_input_frames_match(const InputFrame &frame1, const InputFrame &frame2);

    TestResult test_metadata_match(const Metadata &metadata_original, const Metadata &metadata_fit);

    TestResult test_metadata_fit_file_saver(const InputFrame &input_frame, const std::string &output_file, int output_bit_depth);
}