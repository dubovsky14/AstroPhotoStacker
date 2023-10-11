#pragma once

#include <cmath>

namespace AstroPhotoStacker {
    class GeometricTransformer {
        public:
            GeometricTransformer() = delete;
            GeometricTransformer(float shift_x, float shift_y, float rotation_center_x, float rotation_center_y, float rotation) :
                m_shift_x(shift_x),
                m_shift_y(shift_y),
                m_rotation(rotation),
                m_rotation_center_x(rotation_center_x),
                m_rotation_center_y(rotation_center_y) {
                    cosx = cos(m_rotation);
                    sinx = sin(m_rotation);
                };

            void transform_to_reference_frame(float *x, float *y)   const   {
                float x_new = *x - m_rotation_center_x;
                float y_new = *y - m_rotation_center_y;
                *x = x_new*cosx - y_new*sinx + m_rotation_center_x + m_shift_x;
                *y = x_new*sinx + y_new*cosx + m_rotation_center_y + m_shift_y;
            };

            void transform_from_reference_to_shifted_frame(float *x, float *y)   const   {
                float x_new = *x - m_rotation_center_x - m_shift_x;
                float y_new = *y - m_rotation_center_y - m_shift_y;
                *x = x_new*cosx + y_new*sinx + m_rotation_center_x;
                *y = -x_new*sinx + y_new*cosx + m_rotation_center_y;
            };

        private:
            float m_shift_x;
            float m_shift_y;
            float m_rotation;
            float m_rotation_center_x;
            float m_rotation_center_y;

            float cosx, sinx;
    };
}