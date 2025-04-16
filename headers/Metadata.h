#pragma once

#include <string>
#include <optional>
#include <array>

namespace AstroPhotoStacker {
    struct Metadata {
        float aperture          = 0;
        float exposure_time     = 0;
        int iso                 = 0;
        float focal_length      = 0;
        std::string date_time   = "";
        int timestamp           = 0;
        int max_value           = 0;
        bool monochrome         = false;
        bool is_raw             = false;

        std::optional<std::array<char, 4>> bayer_matrix;

    };
}