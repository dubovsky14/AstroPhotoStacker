#pragma once
#include "../headers/IndividualColorStretchingToolBase.h"

#include "../../headers/Common.h"

#include <vector>

/**
 * @brief The class for individual stretching - applying one color curve to the image
*/
class IndividualColorStretchingBlackCorrectionWhite : public IndividualColorStretchingToolBase {
    public:
        IndividualColorStretchingBlackCorrectionWhite() = default;

        /**
         * @brief Construct a new Individual Color Stretching Tool object
         *
         * @param black_point the black point - as a fraction - from interval [0, 1]
         * @param correction the correction - as a fraction - from interval [0, 1]
         * @param white_point the white point - as a fraction - from interval [0, 1]
        */
        IndividualColorStretchingBlackCorrectionWhite(float black_point, float correction, float white_point)  {
            set_stretching_parameters(black_point, correction, white_point);
        };

        /**
         * @brief Set straching parameters
         *
         * @param black_point the black point - as a fraction - from interval [0, 1]
         * @param exposure_correction the correction - as a fraction - from interval [0, 1]
         * @param white_point the white point - as a fraction - from interval [0, 1]
        */
        void set_stretching_parameters(float black_point, float exposure_correction, float white_point) {
            m_black_point           = black_point;
            m_exposure_correction   = exposure_correction;
            m_white_point           = white_point;
            m_one_over_range        = 1. / (m_white_point - m_black_point);

            m_scale_factor = pow(2., m_exposure_correction);
        };

        /**
         * @brief Stretch the value
         *
         * @param value the value to stretch
         * @return float the stretched value
        */
        virtual float stretch(float value, float full_scale_max) const override    {
            // firstly, normalize the value and apply black point and white point
            value = value / full_scale_max;
            float normalized_value =  (value - m_black_point) * m_one_over_range;
            normalized_value = AstroPhotoStacker::force_range<float>(normalized_value, 0, 1);

            // apply exposure correction and scale factor
            normalized_value *= m_scale_factor;
            normalized_value = AstroPhotoStacker::force_range<float>(normalized_value, 0, 1);
            return normalized_value*full_scale_max;
        };

    private:
        float m_black_point         = 0;
        float m_exposure_correction = 0;
        float m_white_point         = 1;
        float m_scale_factor        = 1;
        float m_one_over_range      = 1;

};
