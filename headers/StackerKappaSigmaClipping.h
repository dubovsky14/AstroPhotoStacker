#pragma once

#include "../headers/StackerMedian.h"

namespace AstroPhotoStacker {
    class StackerKappaSigmaClipping : public StackerMedian {
    public:
            StackerKappaSigmaClipping(int number_of_colors, int width, int height);

            void set_kappa_and_n_iterations(float kappa, int n_iterations);

    protected:
            virtual void calculate_median_at_line(int y_index_final_array, int y_index_values_to_stack_array, int i_color) override;

            float   m_kappa       = 3.0;
            int     m_n_iterations  = 3;
    };
}