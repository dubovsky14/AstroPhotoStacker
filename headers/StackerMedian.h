#pragma once

#include "../headers/StackerBase.h"

#include <vector>
#include <memory>

namespace AstroPhotoStacker {

    /**
     * @brief Class for stacking photos using median stacking algorithm. It serves also as a base class for all algorithms that stack require values from all photos to be stored in memory.
     */
    class StackerMedian : public StackerBase {
        public:
            StackerMedian(int number_of_colors, int width, int height, bool interpolate_colors);

            /**
             * @brief Get the number of tasks to be done by the stacker
             *
             * @return int - number of tasks
            */
            virtual int get_tasks_total() const override;

        protected:
            /**
             * Calculate the stacked photo from the photos added to the stack
            */
            virtual void calculate_stacked_photo_internal() override;

            std::vector<std::vector<PixelType>> m_values_to_stack;

            virtual void add_photo_to_stack(unsigned int file_index, int y_min, int y_max) override;

            virtual int get_height_range_limit() const override;

            virtual void process_line(int y_index_final_array, int y_index_values_to_stack_array, int i_color);

            /**
             * @brief The method defines how to stack "number_of_stacked pixels" values from pixels in individual photos into one file. In case of this class it is a median. Can be overriden in derived classes to implement different stacking algorithms.
             *
             * @param ordered_array_begin       - pointer ot the first element of the ordered array
             * @param number_of_stacked_pixels  - number of pixels to stack
             */
            virtual double get_stacked_value_from_pixel_array(PixelType *ordered_array_begin, unsigned int number_of_stacked_pixels);

            /**
             * @brief Get maximal memory usage, considering the number of frames and their resolution
             *
             * @return unsigned long long - maximal memory usage
            */
            virtual unsigned long long get_maximal_memory_usage(int number_of_frames) const override;
    };
}