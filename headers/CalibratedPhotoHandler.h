#pragma once

#include "../headers/GeometricTransformations.h"
#include "../headers/FlatFrameHandler.h"
#include "../headers/HotPixelIdentifier.h"

#include <string>
#include <memory>
#include <vector>

namespace AstroPhotoStacker {
    class CalibratedPhotoHandler {
        public:
            CalibratedPhotoHandler() = delete;

            CalibratedPhotoHandler(const std::string &raw_file_address);

            void define_alignment(float shift_x, float shift_y, float rotation_center_x, float rotation_center_y, float rotation);

            void set_bit_depth(unsigned short int bit_depth);

            void calibrate();

            void limit_y_range(int y_min, int y_max);

            void register_flat_frame(const FlatFrameHandler *flat_frame_handler);

            void register_hot_pixel_identifier(const HotPixelIdentifier *hot_pixel_identifier);

            void apply_dark_frame()   {}; // TODO: implement

            // if color is negative, the pixel coordinates are out of the image boundaries
            void get_value_by_reference_frame_coordinates(float x, float y, unsigned int *value, char *color) const;


        private:
            int m_width;
            int m_height;

            int m_y_min = -1;
            int m_y_max = -1;
            unsigned int m_max_allowed_pixel_value = 1 << 14;

            const FlatFrameHandler *m_flat_frame                = nullptr;
            const HotPixelIdentifier *m_hot_pixel_identifier    = nullptr;

            std::unique_ptr<GeometricTransformer> m_geometric_transformer   = nullptr;
            std::unique_ptr<short unsigned int[]> m_data_original           = nullptr;
            std::vector<short unsigned int> m_data_shifted;
            std::vector<char> m_colors_original;
            std::vector<char> m_colors_shifted;
            std::vector<char> m_color_conversion_table; // color number from the raw file usually can take values 0,1,2,3. 3 is usually green, but it is not guaranteed, so we need to convert it to 0,1,2

            void fix_hot_pixel(int x, int y);
    };
}