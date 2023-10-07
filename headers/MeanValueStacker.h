#pragma once

#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/FlatFrameHandler.h"

#include <memory>
#include <string>
#include <vector>

namespace AstroPhotoStacker {
    class MeanValueStacker   {
        public:
            MeanValueStacker(int number_of_colors, int width, int height);
            virtual ~MeanValueStacker()  {};

            virtual void add_photo(const CalibratedPhotoHandler &photo);

            virtual void calculate_stacked_photo();

            void save_stacked_photo_as_png(const std::string &file_address) const;

        private:
            int m_number_of_colors;
            int m_width;
            int m_height;

            std::vector<std::vector<unsigned int>>      m_stacked_image;
            std::vector<std::vector<unsigned short>>    m_number_of_stacked_pixels;

            std::vector<std::vector<double>>            m_stacked_image_double;
            std::unique_ptr<FlatFrameHandler>           m_flat_frame_handler    = nullptr;
    };
}