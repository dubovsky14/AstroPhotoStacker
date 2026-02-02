#pragma once

#include <string>
#include <array>

namespace AstroPhotoStacker {
    struct Metadata {
        float aperture          = 0;
        float exposure_time     = 0;
        int iso                 = 0;
        float focal_length      = 0;
        int timestamp           = 0;
        int max_value           = 0;
        bool monochrome         = false;
        float temperature       = -300;
        int bit_depth           = 16;

        bool is_raw             = false;
        std::string camera_model = "";
        std::string bayer_matrix = "";
        float video_fps         = -1;

        std::string get_datetime() const;
    };
}