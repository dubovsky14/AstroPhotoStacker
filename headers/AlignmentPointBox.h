#pragma once

#include <vector>

#include "../headers/MonochromeImageData.h"

namespace AstroPhotoStacker {
    class AlignmentPointBox    {
        public:
            AlignmentPointBox(const MonochromeImageData &image_data, int x_center, int y_center, unsigned int box_width, unsigned int box_height, unsigned short max_value);

            float get_chi2(const MonochromeImageData &image_data, int window_pos_x, int window_pos_y) const;

            static bool is_valid_ap(const MonochromeImageData &image_data, int x_center, int y_center, unsigned int box_width, unsigned int box_height, unsigned short max_value);

            static void  set_contrast_threshold(float threshold);

            static float get_contrast_threshold();

            float get_relative_rms() const;

            unsigned int get_box_width() const   {
                return m_box_width;
            };

            unsigned int get_box_height() const   {
                return m_box_height;
            };

            unsigned short get_max_value() const    {
                return m_max_value;
            };

            unsigned int get_center_x() const   {
                return m_x_center;
            };

            unsigned int get_center_y() const   {
                return m_y_center;
            };

            bool good_match(float chi2) const;

            static float get_sharpness_factor(const MonochromeImageData &image_data, int x_center, int y_center, unsigned int box_width, unsigned int box_height);

        private:
            std::vector<unsigned short> m_brightness;
            int m_x_center;
            int m_y_center;
            unsigned int m_box_width;
            unsigned int m_box_height;
            unsigned short m_max_value;
            float m_max_acceptable_chi2 = 10e100;

            static float s_contrast_threshold;

            float calculate_acceptable_chi2(const MonochromeImageData &image_data) const;
    };
}