#pragma once

#include "../headers/StackerSimpleBase.h"

/**
 * @brief Class for stacking photos using maximal value. This can be useful to stack star trails or thunderstorms.
 *
 */

namespace AstroPhotoStacker {
    class StackerMaximum : public StackerSimpleBase {
        public:
            /**
             * @brief Construct a new Stacker Mean Value object
             *
             * @param number_of_colors - number of colors in the stacked photo
             * @param width - width of the photo
             * @param height - height of the photo
             * @param interpolate_colors - not implemented yet, it's there just for compatibility with other stacking algorithms
            */
            StackerMaximum(int number_of_colors, int width, int height, bool interpolate_colors);

        protected:

            std::vector<std::vector<std::vector<int>>>                  m_max_values_individual_threads;  // [thread][color][pixel]

            /**
             * @brief Put all the partial results together
            */
            virtual void calculate_final_image() override;

            /**
             * @brief Allocate arrays for stacking where partial results are stored
            */
            virtual void allocate_arrays_for_stacking() override;

            /**
             * @brief Clean up the arrays for stacking where partial results were stored
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
    };
}