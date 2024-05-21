#pragma once

#include <string>
#include <memory>
#include <vector>


namespace AstroPhotoStacker {

    /**
     * @brief A class that handles a flat frame data - it can read both raw files and tif/jpg images
     *
    */
    class FlatFrameHandler  {
        public:
            FlatFrameHandler() = delete;

            /**
             * @brief Construct a new FlatFrameHandler object
             *
             * @param input_file The file to read the flat frame data from
            */
            FlatFrameHandler(const std::string &input_file);


            /**
             * @brief Copy constructor
            */
            FlatFrameHandler(const FlatFrameHandler &other);

            /**
             * @brief Get value of a pixel in the flat frame
             *
             * @param x The x coordinate of the pixel
             * @param y The y coordinate of the pixel
             * @return float The value of the pixel
            */
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