#pragma once

#include <vector>

#include "../headers/MonochromeImageData.h"

namespace AstroPhotoStacker {
    class AlignmentPointBox    {
        public:
            AlignmentPointBox(const MonochromeImageData &image_data, int x0, int y0, unsigned int size_x, unsigned int size_y, unsigned short max_value);

            float get_chi2(const MonochromeImageData &image_data, int window_pos_x, int window_pos_y) const;

            static bool is_valid_ap(const MonochromeImageData &image_data, int x0, int y0, unsigned int size_x, unsigned int size_y, unsigned short max_value);


        private:
            std::vector<unsigned short> m_brightness;
            int m_x0;
            int m_y0;
            unsigned int m_size_x;
            unsigned int m_size_y;
            unsigned short m_max_value;
    };
}