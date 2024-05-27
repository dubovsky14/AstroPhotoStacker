#pragma once

#include "../headers/GeometricTransformations.h"
#include "../headers/FlatFrameHandler.h"
#include "../headers/HotPixelIdentifier.h"

#include <string>
#include <memory>
#include <vector>

namespace AstroPhotoStacker {
    /**
     * @class CalibratedPhotoHandler
     *
     * @brief A class for handling calibrated photo data. It is responsible for applying calibration data to raw photo data, such as flat frames and dark frames and for applying geometric transformations to the photo data.
    */
    class CalibratedPhotoHandler {
        public:
            CalibratedPhotoHandler() = delete;

            /**
             * @brief Constructor that initializes the CalibratedPhotoHandler object.
             * @param raw_file_address The address of the raw file to be calibrated.
             * @param use_color_interpolation Whether to use color interpolation.
            */
            CalibratedPhotoHandler(const std::string &raw_file_address, bool use_color_interpolation = false);

            /**
             * @brief Add alignment data to the CalibratedPhotoHandler object.
             *
             * @param shift_x The shift in the x direction.
             * @param shift_y The shift in the y direction.
             * @param rotation_center_x The x coordinate of the rotation center.
             * @param rotation_center_y The y coordinate of the rotation center.
             * @param rotation The rotation angle in radians.
            */
            void define_alignment(float shift_x, float shift_y, float rotation_center_x, float rotation_center_y, float rotation);

            /**
             * @brief Set the bit depth of the raw file.
             *
             * @param bit_depth The bit depth of the raw file.
            */
            void set_bit_depth(unsigned short int bit_depth);

            /**
             * @brief Apply calibration frames to the raw photo, shift and rotate the photo according to the alignment and run color interpolation if requested in the constructor.
            */
            void calibrate();

            /**
             * @brief Set the range of y values to be used in the calibrated data. This is useful to safe memory when stacking large number of images.
             *
             * @param y_min The minimum y value in pixels.
             * @param y_max The maximum y value in pixels.
            */
            void limit_y_range(int y_min, int y_max);

            /**
             * @brief Register calibration frame which will then be applied to the photo.
             *
             * @param calibration_frame_handler The calibration frame handler to be registered.
            */
            void register_calibration_frame(std::shared_ptr<const CalibrationFrameBase> calibration_frame_handler);

            /**
             * @brief Register a flat frame handler to the CalibratedPhotoHandler object.
             *
             * @param flat_frame_handler The flat frame handler to be registered.
            */
            void register_flat_frame(const FlatFrameHandler *flat_frame_handler);

            /**
             * @brief Register a hot pixel identifier to the CalibratedPhotoHandler object.
             *
             * @param hot_pixel_identifier The hot pixel identifier to be registered.
            */
            void register_hot_pixel_identifier(const HotPixelIdentifier *hot_pixel_identifier);

            void apply_dark_frame()   {}; // TODO: implement

            /**
             * @brief Get the value of the pixel and its color at the given coordinates in the reference frame. If color is negative, the pixel coordinates are out of the image boundaries
             *
             * @param x The x coordinate in the reference frame.
             * @param y The y coordinate in the reference frame.
             * @param value The value of the pixel at the given coordinates.
             * @param color The color of the pixel at the given coordinates.
            */
            void get_value_by_reference_frame_coordinates(float x, float y, unsigned int *value, char *color) const;


            /**
             * @brief Get the value of the pixel at the given coordinates in the reference frame, when color interpolation is used.
             *
             * @param x The x coordinate in the reference frame.
             * @param y The y coordinate in the reference frame.
             * @param color The color of the pixel at the given coordinates.
             * @param value The value of the pixel at the given coordinates.
            */
            void get_value_by_reference_frame_coordinates(float x, float y, int color, unsigned int *value) const;

        private:
            int m_width;
            int m_height;

            int m_y_min = -1;
            int m_y_max = -1;
            unsigned int m_max_allowed_pixel_value = 1 << 14;

            const FlatFrameHandler *m_flat_frame                = nullptr;
            const HotPixelIdentifier *m_hot_pixel_identifier    = nullptr;
            std::vector<std::shared_ptr<const CalibrationFrameBase>> m_calibration_frames;

            std::unique_ptr<GeometricTransformer> m_geometric_transformer   = nullptr;
            std::unique_ptr<short unsigned int[]> m_data_original           = nullptr;

            bool m_use_color_interpolation = false;
            std::vector<std::vector<short unsigned int>> m_data_original_color_interpolation;
            std::vector<std::vector<short unsigned int>> m_data_shifted_color_interpolation;

            std::vector<short unsigned int> m_data_shifted;
            std::vector<char> m_colors_original;
            std::vector<char> m_colors_shifted;
            std::vector<char> m_color_conversion_table; // color number from the raw file usually can take values 0,1,2,3. 3 is usually green, but it is not guaranteed, so we need to convert it to 0,1,2

            void fix_hot_pixel(int x, int y);

            void run_color_interpolation();
    };
}