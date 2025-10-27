#pragma once


#include "../headers/TestUtils.h"
#include "../../headers/InputFrame.h"

#include <string>
#include <vector>
#include <tuple>


namespace AstroPhotoStacker {
    TestResult test_predefined_alignment_boxes( const InputFrame &reference_frame,
                                                const InputFrame &alternative_frame,
                                                const std::vector<std::tuple<int,int,int,int>> &expected_shifts);
}