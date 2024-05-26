#pragma once

#include <string>
#include <vector>
#include <memory>

namespace AstroPhotoStacker {
    /**
     * @class CalibrationFrameBase
     *
     * @brief A purely virtual base class for handling calibration frame data. It is responsible for reading calibration frame data and applying it to raw photo data.
    */
    class CalibrationFrameBase{
        public:
            CalibrationFrameBase() = delete;

            /**
             * @brief Constructor that initializes the CalibrationFrameBase object.
             *
             * @param input_file The address of the calibration frame file.
            */
            CalibrationFrameBase(const std::string &input_file);

            /**
             * @brief Copy constructor
            */
            CalibrationFrameBase(const CalibrationFrameBase &other);

            virtual ~CalibrationFrameBase() = default;

            /**
             * @brief Get value of a pixel after the calibration
             *
             * @param pixel_value The value of the pixel
             * @param x The x coordinate of the pixel
             * @param y The y coordinate of the pixel
             *
             * @return float The value of the pixel after calibration
            */
            virtual float get_updated_pixel_value(float pixel_value, int x, int y) const = 0;

        protected:
            virtual void calibrate() = 0;

            int m_width, m_height;
            std::unique_ptr<unsigned short int[]>   m_data_original;
            std::vector<float>                      m_data_calibrated;
            std::vector<char>                       m_colors;

    };
}