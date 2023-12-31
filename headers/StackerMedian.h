#pragma once

#include "../headers/StackerBase.h"

#include <vector>
#include <memory>

namespace AstroPhotoStacker {
    class StackerMedian : public StackerBase {
        public:
            StackerMedian(int number_of_colors, int width, int height);

            virtual void calculate_stacked_photo() override;

            virtual void set_number_of_cpu_threads(unsigned int n_cpu) override;

            virtual int get_tasks_total() const override;

        protected:
            std::vector<std::vector<unsigned short>> m_number_of_stacked_pixels;
            std::vector<std::unique_ptr<short[]>> m_values_to_stack;

            void add_photo_to_stack(unsigned int file_index, int y_min, int y_max);

            int get_height_range_limit()    const;

            virtual void process_line(int y_index_final_array, int y_index_values_to_stack_array, int i_color);

            /**
             * @brief The method defines how to stack "number_of_stacked pixels" values from pixels in individual photos into one file. In case of this class it is a median.
             *
             * @param ordered_array_begin       - pointer ot the first element of the ordered array
             * @param number_of_stacked_pixels  - number of pixels to stack
             */
            virtual double get_stacked_value_from_pixel_array(short int *ordered_array_begin, unsigned int number_of_stacked_pixels);
    };
}