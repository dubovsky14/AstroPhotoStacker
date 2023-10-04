#pragma once

#include "../headers/GeometricTransformations.h"

#include <string>
#include <memory>
#include <vector>

namespace AstroPhotoStacker {
    class CalibratedPhotoHandler {
        public:
            CalibratedPhotoHandler(const std::string &raw_file_address);

            void define_alignment(float shift_x, float shift_y, float rotation_center_x, float rotation_center_y, float rotation);

            void calibrate();

            void add_flat_frame()   {}; // TODO: implement

            void add_dark_frame()   {}; // TODO: implement

            // if color is negative, the pixel coordinates are out of the image boundaries
            void get_value_by_reference_frame_coordinates(float x, float y, unsigned int *value, char *color) const;


        private:
            int m_width;
            int m_height;

            std::unique_ptr<GeometricTransformer> m_geometric_transformer   = nullptr;
            std::unique_ptr<short unsigned int[]> m_data_original           = nullptr;
            std::vector<short unsigned int> m_data_shifted;
            std::vector<char> m_colors_original;
            std::vector<char> m_colors_shifted;
            std::vector<char> m_color_conversion_table; // color number from the raw file usually can take values 0,1,2,3. 3 is usually green, but it is not guaranteed, so we need to convert it to 0,1,2
    };
}