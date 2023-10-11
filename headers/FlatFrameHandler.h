#pragma once

#include <string>
#include <memory>
#include <vector>


namespace AstroPhotoStacker {
    class FlatFrameHandler  {
        public:
            FlatFrameHandler() = delete;

            FlatFrameHandler(const std::string &input_file);

            FlatFrameHandler(const FlatFrameHandler &other);

            float get_pixel_value_inverted(int x, int y) const;


        private:
            void calibrate();

            int m_width, m_height;
            std::unique_ptr<unsigned short int[]>   m_data_original;
            std::vector<float>                      m_data_calibrated;
            std::vector<char>                       m_colors;

            bool                                    m_read_from_raw_file = false;

    };
}