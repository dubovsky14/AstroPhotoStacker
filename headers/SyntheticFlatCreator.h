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

            void get_flat_center(int *center_x, int *center_y);

            void save_flat(const std::string &output_file);




            std::vector<unsigned short> m_original_gray_scale_data;
            int m_width = 0;
            int m_height = 0;

            unsigned short m_threshold = 0;

    };
}