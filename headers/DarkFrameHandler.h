#pragma once

#include "../headers/CalibrationFrameBase.h"

#include <string>
#include <memory>
#include <vector>


namespace AstroPhotoStacker {

    /**
     * @brief A class that handles a flat frame data - it can read both raw files and tif/jpg images
     *
    */
    class DarkFrameHandler : public CalibrationFrameBase  {
        public:
            DarkFrameHandler() = delete;

            /**
             * @brief Construct a new DarkFrameHandler object
             *
             * @param input_file The file to read the flat frame data from
            */
            DarkFrameHandler(const std::string &input_file);


            /**
             * @brief Copy constructor
            */
            DarkFrameHandler(const DarkFrameHandler &other);

            /**
             * @brief Get value of a pixel after the calibration
             *
             * @param pixel_value The value of the pixel
             * @param x The x coordinate of the pixel
             * @param y The y coordinate of the pixel
             *
             * @return float The value of the pixel after calibration
            */
            virtual float get_updated_pixel_value(float pixel_value, int x, int y) const override;

        private:
            virtual void calibrate() override;

    };
}