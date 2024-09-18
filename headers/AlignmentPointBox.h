#pragma once

#include <vector>

#include "../headers/MonochromeImageData.h"

namespace AstroPhotoStacker {
    class AlignmentPointBox    {
        public:
            AlignmentPointBox(const MonochromeImageData &image_data, int x_center, int y_center, unsigned int box_size, unsigned short max_value);

            float get_chi2(const MonochromeImageData &image_data, int window_pos_x, int window_pos_y) const;

            static bool is_valid_ap(const MonochromeImageData &image_data, int x_center, int y_center, unsigned int box_size, unsigned short max_value);


        private:
            std::vector<unsigned short> m_brightness;
            int m_x_center;
            int m_y_center;
            unsigned int m_box_size;
            unsigned short m_max_value;
    };
}