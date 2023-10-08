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

        protected:
            std::vector<std::vector<unsigned short>> m_number_of_stacked_pixels;
            std::vector<std::unique_ptr<short[]>> m_values_to_stack;

            void add_photo_to_stack(unsigned int file_index, int y_min, int y_max);

            int get_height_range_limit()    const;

            virtual void calculate_median_at_line(int y_index_final_array, int y_index_values_to_stack_array, int i_color);
    };
}