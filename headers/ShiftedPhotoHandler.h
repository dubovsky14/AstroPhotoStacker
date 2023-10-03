#pragma once

#include "../headers/GeometricTransformations.h"

#include <string>
#include <memory>
#include <vector>

namespace AstroPhotoStacker {
    class ShiftedPhotoHandler {
        public:
            ShiftedPhotoHandler(float shift_x, float shift_y, float rotation_center_x, float rotation_center_y, float rotation);

            void add_raw_data(const std::string &raw_file_address);

            // if coloro is negative, the pixel coordinates are out of the image boundaries
            void get_value_by_reference_frame_coordinates(float x, float y, unsigned int *value, char *color) const;


        private:
            int m_width;
            int m_height;

            std::unique_ptr<GeometricTransformer> m_geometric_transformer   = nullptr;
            std::unique_ptr<short unsigned int[]> m_data                    = nullptr;
            std::vector<char> m_colors;
    };

}