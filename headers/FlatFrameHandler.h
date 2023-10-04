#pragma once

#include <string>
#include <memory>
#include <vector>


namespace AstroPhotoStacker {
    class FlatFrameHandler  {
        public:
            FlatFrameHandler(const std::string &input_file);

            float get_pixel_value(int x, int y) const;


        private:
            void calibrate();

            int m_width, m_height;
            std::unique_ptr<unsigned short int[]>   m_data_original;
            std::vector<float>                      m_data_calibrated;
            std::vector<char>                       m_colors;

    };
}