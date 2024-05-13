#pragma once
#include "../headers/IndividualColorStretchingToolBase.h"

#include "../../headers/Common.h"

#include <vector>

/**
 * @brief The class for individual stretching - applying one color curve to the image
*/
class IndividualColorStretchingBlackMidtoneWhite : public IndividualColorStretchingToolBase {
    public:
        IndividualColorStretchingBlackMidtoneWhite() = default;

        /**
         * @brief Construct a new Individual Color Stretching Tool object
         *
         * @param black_point the black point - as a fraction - from interval [0, 1]
         * @param midtone the midtone - as a fraction - from interval [0, 1]
         * @param white_point the white point - as a fraction - from interval [0, 1]
        */
        IndividualColorStretchingBlackMidtoneWhite(float black_point, float midtone, float white_point)  {
            set_stretching_parameters(black_point, midtone, white_point);
        };

        /**
         * @brief Set straching parameters
         *
         * @param black_point the black point - as a fraction - from interval [0, 1]
         * @param midtone the midtone - as a fraction - from interval [0, 1]
         * @param white_point the white point - as a fraction - from interval [0, 1]
        */
        void set_stretching_parameters(float black_point, float midtone, float white_point) {
            m_black_point   = black_point;
            m_midtone       = midtone;
            m_white_point   = white_point;

            m_one_over_range  = 1 / (m_white_point - m_black_point);

            // quadratic stretching
            m_a = 2 - 4 * m_midtone;
            m_b = 4 * m_midtone - 1;
        };

        /**
         * @brief Stretch the value
         *
         * @param value the value to stretch
         * @return float the stretched value
        */
        virtual float stretch(float value, float full_scale_max) const override    {
            // firstly, normalize the value
            value = value / full_scale_max;
            float normalized_value =  (value - m_black_point) * m_one_over_range;
            normalized_value = AstroPhotoStacker::force_range<float>(normalized_value, 0, 1);


            normalized_value = m_a*normalized_value*normalized_value + m_b*normalized_value;

            // apply the full scale
            const float result = normalized_value * full_scale_max;
            return AstroPhotoStacker::force_range<float>(result, 0,   full_scale_max);
        };

    private:
        float m_black_point     = 0;
        float m_midtone         = 0.5;
        float m_white_point     = 1;
        float m_one_over_range  = 1;

        // quadratic stretching
        float m_a = 2 - 4 * m_midtone;
        float m_b = 4 * m_midtone - 1;
};
