#pragma once


#include <vector>
#include <string>
#include <memory>
#include <vector>

namespace AstroPhotoStacker {
    class SyntheticFlatCreator {
        public:
            SyntheticFlatCreator(const std::string &input_file);

            void create_and_save_synthetic_flat(const std::string &output_file);

        private:
            void load_data(const std::string &input_file);

            void calculate_threshold();

            void replace_values_above_threshold();

            void rebin_data(unsigned int rebin_factor);

            void fit_parameters();

            float fit_center_x();

            void get_flat_center(float *center_x, float *center_y);

            void save_flat(const std::string &output_file);

            std::vector<std::vector<float>> m_rebinned_data;

            std::vector<unsigned short> m_original_gray_scale_data;
            unsigned int m_width = 0;
            unsigned int m_height = 0;

            unsigned short m_threshold = 0;

            unsigned int m_rescaled_square_size = 10;

            float m_center_x = 0;
            float m_center_y = 0;

    };
}