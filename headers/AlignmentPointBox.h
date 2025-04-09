#pragma once

#include <vector>

#include "../headers/MonochromeImageData.h"
#include "../headers/AlignmentWindow.h"

namespace AstroPhotoStacker {

    /**
     * @brief Class representing a box of pixels in the alignment window. It is used to find patterns in the image
     *        by comparing the brightness values of the pixels in the box (reference image) and the pixels in the
     *        input image which is being alifned..
     */
    class AlignmentPointBox    {
        public:
            AlignmentPointBox() = delete;

            /**
             * @brief Construct a new Alignment Point Box object
             *
             * @param scaled_data_vector The scaled values in the pixels of the reference image, inside the alignment window
             * @param alignment_window The alignment window xmin, ymin, xmax, ymax
             * @param x_center The x coordinate of the center of the box in the original image coordinates
             * @param y_center The y coordinate of the center of the box in the original image coordinates
             * @param box_width The width of the box
             * @param box_height The height of the box
             * @param max_value The maximum value in the input image
             */
            AlignmentPointBox(  const std::vector<float> *scaled_data_vector,
                                const AlignmentWindow &alignment_window,
                                int x_center, int y_center,
                                int box_width, int box_height, float max_value);


            // coordinates are in alignment window coordinates [0,0] is the top left corner of the alignment window
            float get_chi2(const std::vector<float> *scaled_data_vector, int window_pos_x, int window_pos_y) const;

            // coordinates are in alignment window coordinates [0,0] is the top left corner of the alignment window
            static bool is_valid_ap(const std::vector<float> *scaled_data_vector, const AlignmentWindow &alignment_window, int x_center, int y_center, int box_width, int box_height, float max_value);

            static void  set_contrast_threshold(float threshold);

            static float get_contrast_threshold();

            float get_relative_rms() const;

            unsigned int get_box_width() const   {
                return m_box_width;
            };

            unsigned int get_box_height() const   {
                return m_box_height;
            };

            float get_max_value() const    {
                return m_max_value;
            };

            int get_center_x() const   {
                return m_x_center;
            };

            int get_center_y() const   {
                return m_y_center;
            };

            int get_center_x_in_alignment_window_coordinates() const   {
                return m_x_center - m_alignment_window.x_min;
            };

            int get_center_y_in_alignment_window_coordinates() const   {
                return m_y_center - m_alignment_window.y_min;
            };

            bool good_match(float chi2) const;

            float acceptable_chi2() const {
                return m_max_acceptable_chi2;
            };

            // coordinates are in alignment window coordinates [0,0] is the top left corner of the alignment window
            static float get_sharpness_factor(const std::vector<float> *scaled_data_vector, const AlignmentWindow &alignment_window, int x_center, int y_center, int box_width, int box_height);

        private:
            bool valid_box_coordinates(int x_center, int y_center, int box_width, int box_height) const;

            const std::vector<float> *m_scaled_data_vector = nullptr; //    scaled data in the alignment window
            AlignmentWindow m_alignment_window;
            int m_x_min_in_alignment_window; //    x_min in the alignment window coordinates
            int m_y_min_in_alignment_window; //    y_min in the alignment window coordinates
            int m_alignment_window_width;    //    width of the alignment window
            int m_alignment_window_height;   //    height of the alignment window

            int m_x_center;
            int m_y_center;
            int m_box_width;
            int m_box_height;
            float m_max_value;
            float m_max_acceptable_chi2 = 10e100;

            static float s_contrast_threshold;

            float calculate_acceptable_chi2() const;

            inline float get_value_of_reference_frame_by_alignment_window_coordinates(int x, int y) const {
                return m_scaled_data_vector->at(x*m_alignment_window_width + y);
            };

            inline float get_value_of_reference_frame_by_reference_frame_coordinates(int x, int y) const {
                return get_value_of_reference_frame_by_alignment_window_coordinates(x - m_alignment_window.x_min, y - m_alignment_window.y_min);
            };
    };
}