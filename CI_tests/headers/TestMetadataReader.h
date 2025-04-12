#pragma once

#include <string>

#include "../headers/TestUtils.h"

namespace AstroPhotoStacker {
    TestResult test_metadata_reading(   const std::string &input_file,
                                        float expected_aperture,
                                        float expected_exposure_time,
                                        int expected_iso,
                                        float expected_focal_length,
                                        int expected_unix_time = -1);
}