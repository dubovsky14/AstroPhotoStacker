#pragma once

#include "../headers/PixelType.h"
#include "../headers/InputFrame.h"

#include <string>
#include <vector>
#include <memory>
#include <array>

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
             * @param input_frame Data about the input frame.
            */
            CalibrationFrameBase(const InputFrame &input_frame);

            /**
             * @brief Copy constructor
            */
            CalibrationFrameBase(const CalibrationFrameBase &other) = default;

            /**
             * @brief Construct a new CalibrationFrameBase object from staker frame data
            */
            CalibrationFrameBase(int width, int height, const std::vector<double> &image);

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

            virtual void apply_calibration(std::vector<PixelType> *data) const;

        protected:
            virtual void calibrate() {};

            int m_width, m_height;
            std::vector<PixelType>              m_data_original;
            std::vector<float>                  m_data_calibrated;
            std::array<char, 4>                 m_colors;

    };
}