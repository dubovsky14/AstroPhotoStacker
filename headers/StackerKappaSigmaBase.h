#pragma once

#include "../headers/StackerMedian.h"

namespace AstroPhotoStacker {
    class StackerKappaSigmaBase : public StackerMedian {
    public:
            StackerKappaSigmaBase(int number_of_colors, int width, int height, bool interpolate_colors) :
                StackerMedian(number_of_colors, width, height, interpolate_colors)    {};

            void set_kappa(float kappa)                         {m_kappa = kappa;};

            void set_number_of_iterations(int n_iterations)     {m_n_iterations = n_iterations;};

    protected:
            virtual double get_stacked_value_from_pixel_array(short int *ordered_array_begin, unsigned int number_of_stacked_pixels)  = 0;

            void apply_kappa_sigma_clipping(short int **ordered_array_begin, unsigned int *number_of_stacked_pixels);

            float   m_kappa       = 3.0;
            int     m_n_iterations  = 3;
    };
}