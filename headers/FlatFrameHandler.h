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
    class FlatFrameHandler : public CalibrationFrameBase  {
        public:
            FlatFrameHandler() = delete;

            /**
             * @brief Construct a new FlatFrameHandler object
             *
             * @param input_frame The file to read the flat frame data from
            */
            FlatFrameHandler(const InputFrame &input_frame);

            /**
             * @brief Construct a new FlatFrameHandler object from staker frame data
            */
            FlatFrameHandler(int width, int height, const std::vector<double> &image);

            /**
             * @brief Copy constructor
            */
            FlatFrameHandler(const FlatFrameHandler &other);

            /**
             * @brief Get value of a pixel after the calibration
             *
             * @param pixel_value The value of the pixel
             * @param x The x coordinate of the pixel
             * @param y The y coordinate of the pixel
             *
             * @return float The value of the pixel after calibration
            */
            virtual float get_updated_pixel_value(float pixel_value, int x, int y) const override {
                return pixel_value * m_data_calibrated[y*m_width + x];
            };

        private:
            virtual void calibrate() override;

    };
}