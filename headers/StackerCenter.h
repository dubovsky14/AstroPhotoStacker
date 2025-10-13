#pragma once

#include "../headers/StackerSimpleBase.h"

/**
 * @brief Class for stacking photos using the value closest to the middle of the range. Can be used to create hight dynamic range photos.
 *
 */

namespace AstroPhotoStacker {
    class StackerCenter : public StackerSimpleBase {
        public:
            /**
             * @brief Construct a new StackerCenter object
             *
             * @param number_of_colors - number of colors in the stacked photo
             * @param width - width of the photo
             * @param height - height of the photo
             * @param interpolate_colors - not implemented yet, it's there just for compatibility with other stacking algorithms
            */
            StackerCenter(int number_of_colors, int width, int height, bool interpolate_colors);

            /**
             * @brief Set the value which will be used for stacking -> pixel closest to this value will be selected
             */
            void set_central_value(PixelType central_value) {
                m_central_value = central_value;
            }

        protected:

            std::vector<std::vector<std::vector<int>>>                  m_stacked_values_individual_threads;  // [thread][color][pixel]

            /**
             * @brief Put all the partial results together
             *
             * @param y_min - minimal y-coordinate of the area to be processed
             * @param y_max - maximal y-coordinate of the area to be processed
            */
            virtual void calculate_final_image(int y_min, int y_max) override;

            /**
             * @brief Allocate arrays for stacking where partial results are stored
             *
             * @param dy - number of pixel lines to be processed at once
            */
            virtual void allocate_arrays_for_stacking(int dy) override;

            /**
             * @brief Set again default values to these arrays
             *
            */
            virtual void reset_values_in_arrays_for_stacking() override;

            /**
             * @brief Deallocate the arrays for stacking where partial results were stored
            */
            virtual void deallocate_arrays_for_stacking() override;

            /**
             * @brief Process the given pixel for given color
             *
             * @param i_color - color index
             * @param i_pixel - pixel index (it's the total index = y*width + x)
             * @param value - value of the pixel from individual photo
             * @param i_thread - thread index
            */
            virtual void process_pixel(int i_color, int i_pixel, int value, int i_thread) override;

            /**
             * @brief Get number of pixel lines that we can proces at once (limited by memory usage)
             *
             * @return int - number of pixel lines that we can proces at once
            */
            virtual int get_height_range_limit() const override;

            PixelType m_central_value = 16384;
    };
}