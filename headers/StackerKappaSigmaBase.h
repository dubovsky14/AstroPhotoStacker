#pragma once

#include "../headers/StackerMedian.h"

namespace AstroPhotoStacker {
    class StackerKappaSigmaBase : public StackerMedian {
    public:
            StackerKappaSigmaBase(int number_of_colors, int width, int height) :
                StackerMedian(number_of_colors, width, height)    {};

            void set_kappa(float kappa)                         {m_kappa = kappa;};

            void set_number_of_iterations(int n_iterations)     {m_n_iterations = n_iterations;};

    protected:
            virtual void process_line(int y_index_final_array, int y_index_values_to_stack_array, int i_color)  = 0;

            float   m_kappa       = 3.0;
            int     m_n_iterations  = 3;
    };
}