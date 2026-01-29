#pragma once

#include <string>

#include "../headers/TestUtils.h"
#include "../../headers/InputFrame.h"


namespace AstroPhotoStacker {
    TestResult test_metadata_reading(   const InputFrame &input_frame,
                                        float expected_aperture,
                                        float expected_exposure_time,
                                        int expected_iso,
                                        float expected_focal_length,
                                        const std::string &expected_bayer_matrix,
                                        const std::string &expected_camera_model,
                                        int expected_unix_time = -1,
                                        float expected_camera_temperature = -300);
}