#pragma once

#include "../headers/PlateSolvingResult.h"

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
            GeometricTransformer(float shift_x, float shift_y, float rotation_center_x, float rotation_center_y, float rotation) :
                m_shift_x(shift_x),
                m_shift_y(shift_y),
                m_rotation(rotation),
                m_rotation_center_x(rotation_center_x),
                m_rotation_center_y(rotation_center_y) {
                    cosx = cos(m_rotation);
                    sinx = sin(m_rotation);
                };

            /**
             * @brief Construct a new Geometric Transformer object
             *
             * @param PlateSolvingResult
            */
            GeometricTransformer(const PlateSolvingResult &plate_solving_result) :
            m_shift_x           (plate_solving_result.shift_x),
            m_shift_y           (plate_solving_result.shift_y),
            m_rotation          (plate_solving_result.rotation),
            m_rotation_center_x (plate_solving_result.rotation_center_x),
            m_rotation_center_y (plate_solving_result.rotation_center_y) {
                cosx = cos(m_rotation);
                sinx = sin(m_rotation);
            };


            /**
             * @brief Transform coordinates from the shifted frame to the reference frame
             *
             * @param x - pointer to the x coordinate in the shifted frame, it will be overwritten with the x coordinate in the reference frame
             * @param y - pointer to the y coordinate in the shifted frame, it will be overwritten with the x coordinate in the reference frame
            */
            void transform_to_reference_frame(float *x, float *y)   const   {
                float x_new = *x - m_rotation_center_x;
                float y_new = *y - m_rotation_center_y;
                *x = x_new*cosx - y_new*sinx + m_rotation_center_x + m_shift_x;
                *y = x_new*sinx + y_new*cosx + m_rotation_center_y + m_shift_y;
            };

            /**
             * @brief Transform coordinates from the reference frame to the shifted frame
             *
             * @param x - pointer to the x coordinate in the reference frame, it will be overwritten with the x coordinate in the shifted frame
             * @param y - pointer to the y coordinate in the reference frame, it will be overwritten with the x coordinate in the shifted frame
            */
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