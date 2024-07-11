#pragma once

#include "../headers/CalibrationFrameBase.h"
#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/HotPixelIdentifier.h"
#include "../headers/CalibrationFrameBase.h"
#include "../headers/CalibratedPhotoHandler.h"

#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <tuple>


namespace AstroPhotoStacker {

    /**
     * @brief Base class for stacking algorithms. Provides methods for adding photos, saving the stacked photo and setting the number of CPU threads and interfaces for other common functionalities
    */
    class StackerBase   {
        public:
            StackerBase()                   = delete;
            StackerBase(const StackerBase&) = delete;

            /**
             * @brief Construct a new Stacker Base object
             *
             * @param number_of_colors - number of colors in the stacked photo
             * @param width - width of the photo
             * @param height - height of the photo
             * @param interpolate_colors - if true, each color will be interpolated from the colors of the neighboring pixels
            */
            StackerBase(int number_of_colors, int width, int height, bool interpolate_colors);

            /**
             * @brief Set the memory usage limit
             * @param memory_usage_limit_in_mb - memory usage limit in MB
            */
            void set_memory_usage_limit(int memory_usage_limit_in_mb);

            /**
             * @brief Load alignment information from a text file
             *
             * @param alignment_file_address - path to the alignment file
            */
            void add_alignment_text_file(const std::string &alignment_file_address);

            /**
             * @brief Add alignment information for a file
             *
             * @param file_address - path to the file
             * @param x_shift - horizontal shift
             * @param y_shift - vertical shift
             * @param rotation_center_x - x-coordinate of the rotation center
             * @param rotation_center_y - y-coordinate of the rotation center
             * @param rotation - rotation angle
             * @param ranking - ranking of the alignment
            */
            void add_alignment_info(const std::string &file_address, float x_shift, float y_shift, float rotation_center_x, float rotation_center_y, float rotation, float ranking);

            /**
             * @brief Add a photo to the stack
             *
             * @param file_address - path to the file
             * @param apply_alignment - if true, the alignment will be applied - switching it off is usefull for calibration frames
            */
            virtual void add_photo(const std::string &file_address, bool apply_alignment = true);

            /**
             * @brief Add calibration frame handler
             *
             * @param calibration frame handler
            */
            void add_calibration_frame_handler(std::shared_ptr<const CalibrationFrameBase> calibration_frame_handler);

            /**
             * @brief Read hot pixels from a file
             *
             * @param hot_pixels_file - path to the file
            */
            virtual void register_hot_pixels_file(const std::string &hot_pixels_file);

            /**
             * @brief Save the stacked photo
             *
             * @param file_address - path to the file
             * @param image_options - options for saving the image. See OpenCV documentation for details
            */
            virtual void save_stacked_photo(const std::string &file_address, bool apply_color_correction = true, int image_options = 18) const;

            /**
             * @brief Save the stacked photo
             *
             * @param file_address - path to the file
             * @param stacked_image - stacked image as a vector of vectors
             * @param width - width of the image
             * @param height - height of the image
             * @param image_options - options for saving the image. See OpenCV documentation for details
            */
            static void save_stacked_photo(const std::string &file_address, const std::vector<std::vector<double> > &stacked_image, int width, int height, bool apply_color_correction = true, int image_options = 18);

            /**
             * @brief Set the number of CPU threads
             *
             * @param n_cpu - number of CPU threads
            */
            virtual void set_number_of_cpu_threads(unsigned int n_cpu);

            /**
             * @brief The method that calculates the stacked photo - pure virtual method in the base class
            */
            virtual void calculate_stacked_photo() = 0;

            /**
             * @brief Fix empty pixels (pixels without any value) in the stacked photo
            */
            virtual void fix_empty_pixels();

            /**
             * @brief Get the bit depth of the output image
             *
             * @param open_cv_image_type - OpenCV image type
             * @return int - bit depth of the output image
            */
            static int get_output_bit_depth(int open_cv_image_type);

            /**
             * @brief Set the hot pixels
             *
             * @param hot_pixels - vector of tuples containing the x and y coordinates of the hot pixels
            */
            void set_hot_pixels(const std::vector<std::tuple<int, int> > &hot_pixels);

            /**
             * @brief Get the total number of tasks to perform in the stacking. If all photos fit in the memory, it is the number of photos + 1 (for calculating final image)
             *
             * @return int - total number of tasks to perform in the stacking.
            */
            virtual int get_tasks_total() const = 0;

            /**
             * @brief Get the number of tasks processed so far
             *
             * @return const std::atomic<int>& - number of tasks processed so far
            */
            const std::atomic<int>& get_tasks_processed() const;

            /**
             * @brief Get the stacked image
             *
             * @return const std::vector<std::vector<double> >& - stacked image as a vector of vectors
            */
            const std::vector<std::vector<double> > &get_stacked_image() const;

            /**
             * @brief Get the width of the stacked image
             *
             * @return const int - width of the stacked image
            */
            const int get_width() const  { return m_width; };

            /**
             * @brief Get the height of the stacked image
             *
             * @return const int - height of the stacked image
            */
            const int get_height() const { return m_height; };

            /**
             * @brief Does the stack contain only raw RBG files (in order to apply green channel correction)
             *
             * @return bool - true if the stack contains only raw RGB files
             */
            bool contains_only_rgb_raw_files() const { return m_contain_only_rgb_raw_files; };

        protected:
            virtual void add_photo_to_stack(unsigned int file_index, int y_min, int y_max) = 0;

            /**
             * @brief Get number of pixel lines that we can proces at once (limited by memory usage)
             *
             * @return int - number of pixel lines that we can proces at once
            */
            virtual int get_height_range_limit() const = 0;

            /**
             * @brief Take i_file-th file from the stack, calibrate it and return the calibrated photo handler
             *
             * @param i_file - index of the file in the stack
             * @param y_min - minimal y-coordinate of the photo (for memory consumption limits)
             * @param y_max - maximal y-coordinate of the photo (for memory consumption limits)
            */
            virtual CalibratedPhotoHandler get_calibrated_photo(unsigned int i_file, int y_min, int y_max) const;

            int m_number_of_colors;
            int m_width;
            int m_height;
            bool m_interpolate_colors;

            // needed for color correction
            bool m_contain_only_rgb_raw_files = true;

            constexpr static short int c_empty_pixel_value = -1;

            unsigned int m_n_cpu = 1;

            int m_memory_usage_limit_in_mb = -1;

            std::string m_alignment_file_address;

            std::vector<std::string>    m_files_to_stack;
            std::vector<bool>           m_apply_alignment; // for calibration frames we just stack them
            std::vector<std::vector<double> > m_stacked_image;
            std::unique_ptr<PhotoAlignmentHandler> m_photo_alignment_handler    = nullptr;
            std::unique_ptr<HotPixelIdentifier> m_hot_pixel_identifier          = nullptr;

            std::vector<std::shared_ptr<const CalibrationFrameBase> > m_calibration_frame_handlers;

            std::atomic<int> m_n_tasks_processed = 0;
    };
}