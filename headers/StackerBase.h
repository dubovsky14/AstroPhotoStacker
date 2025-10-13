#pragma once

#include "../headers/CalibrationFrameBase.h"
#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/HotPixelIdentifier.h"
#include "../headers/CalibrationFrameBase.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/LocalShiftsHandler.h"

#include "../headers/InputFrame.h"
#include "../headers/AdditionalStackerSetting.h"
#include "../headers/PixelType.h"

#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <tuple>
#include <map>


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
             * @param input frame - input frame data
             * @param x_shift - horizontal shift
             * @param y_shift - vertical shift
             * @param rotation_center_x - x-coordinate of the rotation center
             * @param rotation_center_y - y-coordinate of the rotation center
             * @param rotation - rotation angle
             * @param ranking - ranking of the alignment
            */
            void add_alignment_info(const InputFrame &input_frame, float x_shift, float y_shift, float rotation_center_x, float rotation_center_y, float rotation, float ranking, const LocalShiftsHandler &local_shifts_handler = LocalShiftsHandler());

            /**
             * @brief Add a photo to the stack
             *
             * @param input_frame - data about the input frame (either a photo or a frame from a video)
             * @param calibration_frame_handlers - vector of calibration frame handlers
             * @param apply_alignment - if true, the alignment will be applied - switching it off is usefull for calibration frames
            */
            virtual void add_photo( const InputFrame &input_frame,
                                    const std::vector<std::shared_ptr<const CalibrationFrameBase> > &calibration_frame_handlers = std::vector<std::shared_ptr<const CalibrationFrameBase> >(),
                                    bool apply_alignment = true);

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
            virtual void save_stacked_photo(const std::string &file_address, int image_options = 18) const;

            /**
             * @brief Save the stacked photo
             *
             * @param file_address - path to the file
             * @param stacked_image - stacked image as a vector of vectors
             * @param width - width of the image
             * @param height - height of the image
             * @param image_options - options for saving the image. See OpenCV documentation for details
            */
            static void save_stacked_photo(const std::string &file_address, const std::vector<std::vector<double> > &stacked_image, int width, int height, int image_options = 18);

            /**
             * @brief Set the number of CPU threads
             *
             * @param n_cpu - number of CPU threads
            */
            virtual void set_number_of_cpu_threads(unsigned int n_cpu);

            /**
             * @brief The method that alocates resources and calculates the stacked photo - should be implemented in the derived classes
            */
            void calculate_stacked_photo();

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
             * @brief Get maximal memory usage, considering the number of frames and their resolution
             *
             * @return unsigned long long - maximal memory usage
            */
            virtual unsigned long long get_maximal_memory_usage(int number_of_frames) const = 0;

            std::vector<std::string> get_additional_setting_keys() const;

            void set_additional_setting(const std::string &name, double value);

            AdditionalStackerSetting get_additional_setting(const std::string &name) const;

            void configure_stacker(std::map<std::string, double> settings);

        protected:
            /**
             * @brief Actual stacking method - should be implemented in the derived classes
            */
            virtual void calculate_stacked_photo_internal() = 0;

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

            template<typename ValueType>
            void add_additional_setting(const std::string &name, ValueType *default_value, double range_min, double range_max, double step) {
                // there is no default constructor for AdditionalStackerSetting, so we need this monstrosity with "at" and "insert" ...
                if (m_additional_settings.find(name) != m_additional_settings.end()) {
                    m_additional_settings.at(name) = AdditionalStackerSetting(name, default_value, range_min, range_max, step);
                }
                else {
                    m_additional_settings.insert({name, AdditionalStackerSetting(name, default_value, range_min, range_max, step)});
                }
            };

            int m_number_of_colors;
            int m_width;
            int m_height;
            bool m_interpolate_colors;

            constexpr static PixelType c_empty_pixel_value = -1;

            unsigned int m_n_cpu = 1;

            int m_memory_usage_limit_in_mb = -1;

            std::vector<InputFrame>     m_frames_to_stack;
            std::vector<bool>           m_apply_alignment; // for calibration frames we just stack them
            std::vector<std::vector<double> > m_stacked_image;
            std::unique_ptr<PhotoAlignmentHandler> m_photo_alignment_handler    = nullptr;
            std::unique_ptr<HotPixelIdentifier> m_hot_pixel_identifier          = nullptr;

            // 1st index = light frame index, 2nd index = calibration frame index
            std::vector<std::vector<std::shared_ptr<const CalibrationFrameBase> >> m_calibration_frame_handlers;

            std::atomic<int> m_n_tasks_processed = 0;

            int m_top_left_corner_x = 0;
            int m_top_left_corner_y = 0;

            std::map<std::string, AdditionalStackerSetting> m_additional_settings;
    };
}