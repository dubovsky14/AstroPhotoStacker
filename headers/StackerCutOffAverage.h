#pragma once

#include "../headers/StackerMedian.h"

class StackerCutOffAverage : public AstroPhotoStacker::StackerMedian {
    public:
        StackerCutOffAverage(int number_of_colors, int width, int height, bool interpolate_colors);

        void set_tail_fraction_to_cut_off(float tail_fraction_to_cut_off) {m_tail_fraction_to_cut_off = tail_fraction_to_cut_off;};

        float get_tail_fraction_to_cut_off() const {return m_tail_fraction_to_cut_off;};

    protected:
        float m_tail_fraction_to_cut_off = 0.1;

        virtual double get_stacked_value_from_pixel_array(short int *ordered_array_begin, unsigned int number_of_stacked_pixels) override;
};