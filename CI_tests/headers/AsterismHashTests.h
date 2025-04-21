#pragma once

#include "../headers/TestUtils.h"

#include <vector>
#include <tuple>

namespace AstroPhotoStacker {
    TestResult test_asterism_hasher(const std::vector<std::tuple<float,float,int>> &stars, const std::vector<float> &expected_hash,
                                    unsigned int expected_index_star_A, unsigned int expected_index_star_B,
                                    unsigned int expected_index_star_C, unsigned int expected_index_star_D);
                            }