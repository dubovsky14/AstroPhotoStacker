#pragma once
#include "../headers/StackerSimpleBase.h"

#include <vector>
#include <mutex>

namespace AstroPhotoStacker {

    /**
     * @brief Class for stacking photos using standard deviation -> this can be used to get weights for comet alignment to get both stars and comet sharp
     */
    class StackerRMS : public StackerSimpleBase {
        public:

            /**
             * @brief Construct a new Stacker Mean Value object
             *
             * @param number_of_colors - number of colors in the stacked photo
             * @param width - width of the photo
             * @param height - height of the photo
             * @param interpolate_colors - not implemented yet, it's there just for compatibility with other stacking algorithms
            */
            StackerRMS(int number_of_colors, int width, int height, bool interpolate_colors);

        protected:

            std::vector<std::vector<std::vector<int>>>                  m_values_to_stack_individual_threads_sum;   // [thread][color][pixel]
            std::vector<std::vector<std::vector<double>>>               m_values_to_stack_individual_threads_sum2;  // [thread][color][pixel]
            std::vector<std::vector<std::vector<short unsigned int>>>   m_counts_to_stack_individual_threads;       // [thread][color][pixel]

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

            /**
             * @brief Get number of pixel lines that we can proces at once (limited by memory usage)
             *
             * @return int - number of pixel lines that we can proces at once
            */
            virtual int get_height_range_limit() const override;

            virtual unsigned long long get_maximal_memory_usage(int number_of_frames) const override    {
                return static_cast<unsigned long long>(m_number_of_colors) * static_cast<unsigned long long>(m_width) * static_cast<unsigned long long>(m_height) * (sizeof(int) + sizeof(PixelType) + sizeof(double));
            };


        };
}