#pragma once


#include <cmath>

namespace AstroPhotoStacker {

    /**
     * @brief Class responsible for transforming coordinates between different frames - frame of the reference photo and frame of the shifted photo
    */
    class GeometricTransformer {
        public:
            GeometricTransformer() = delete;

            /**
             * @brief Construct a new Geometric Transformer object
             *
             * @param shift_x - horizontal shift
             * @param shift_y - vertical shift
             * @param rotation_center_x - x coordinate of the rotation center
             * @param rotation_center_y - y coordinate of the rotation center
             * @param rotation - rotation angle
            */
            GeometricTransformer(float shift_x, float shift_y, float rotation_center_x, float rotation_center_y, float rotation, float zoom = 1.0f) :
                m_shift_x(shift_x),
                m_shift_y(shift_y),
                m_rotation(rotation),
                m_rotation_center_x(rotation_center_x),
                m_rotation_center_y(rotation_center_y),
                m_zoom(zoom) {
                    m_cosx = cos(m_rotation);
                    m_sinx = sin(m_rotation);
                };

            /**
             * @brief Transform coordinates from the shifted frame to the reference frame
             *
             * @param x - pointer to the x coordinate in the shifted frame, it will be overwritten with the x coordinate in the reference frame
             * @param y - pointer to the y coordinate in the shifted frame, it will be overwritten with the x coordinate in the reference frame
            */
            void transform_to_reference_frame(float *x, float *y)   const   {
                float x_new = (*x - m_rotation_center_x) / m_zoom;
                float y_new = (*y - m_rotation_center_y) / m_zoom;
                *x = x_new*m_cosx - y_new*m_sinx + m_rotation_center_x + m_shift_x;
                *y = x_new*m_sinx + y_new*m_cosx + m_rotation_center_y + m_shift_y;
            };

            /**
             * @brief Transform coordinates from the reference frame to the shifted frame
             *
             * @param x - pointer to the x coordinate in the reference frame, it will be overwritten with the x coordinate in the shifted frame
             * @param y - pointer to the y coordinate in the reference frame, it will be overwritten with the x coordinate in the shifted frame
            */
            void transform_from_reference_to_shifted_frame(float *x, float *y)   const   {
                float x_new = (*x - m_rotation_center_x - m_shift_x) * m_zoom;
                float y_new = (*y - m_rotation_center_y - m_shift_y) * m_zoom;
                *x = x_new*m_cosx + y_new*m_sinx + m_rotation_center_x;
                *y = -x_new*m_sinx + y_new*m_cosx + m_rotation_center_y;
            };

            void get_parameters(float *shift_x, float *shift_y, float *rotation_center_x, float *rotation_center_y, float *rotation, float *zoom = nullptr) const {
                *shift_x = m_shift_x;
                *shift_y = m_shift_y;
                *rotation_center_x = m_rotation_center_x;
                *rotation_center_y = m_rotation_center_y;
                *rotation = m_rotation;
                if (zoom != nullptr)    {
                    *zoom = m_zoom;
                }
            };

        private:
            float m_shift_x;
            float m_shift_y;
            float m_rotation;
            float m_rotation_center_x;
            float m_rotation_center_y;
            float m_zoom = 1.0f;
            float m_cosx, m_sinx;
    };
}