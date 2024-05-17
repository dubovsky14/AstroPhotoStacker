#pragma once

#include "../headers/FlatFrameHandler.h"
#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/HotPixelIdentifier.h"

#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <tuple>


namespace AstroPhotoStacker {
    class StackerBase   {
        public:
            StackerBase()                   = delete;
            StackerBase(const StackerBase&) = delete;

            StackerBase(int number_of_colors, int width, int height, bool interpolate_colors);

            void set_memory_usage_limit(int memory_usage_limit_in_mb);

            void add_alignment_text_file(const std::string &alignment_file_address);

            void add_alignment_info(const std::string &file_address, float x_shift, float y_shift, float rotation_center_x, float rotation_center_y, float rotation, float ranking);

            virtual void add_photo(const std::string &file_address, bool apply_alignment = true);

            virtual void add_flat_frame(const std::string &file_address);

            virtual void register_hot_pixels_file(const std::string &hot_pixels_file);

            virtual void save_stacked_photo(const std::string &file_address, int image_options = 18) const;

            static void save_stacked_photo(const std::string &file_address, const std::vector<std::vector<double> > &stacked_image, int width, int height, int number_of_colors, int image_options = 18);

            virtual void set_number_of_cpu_threads(unsigned int n_cpu) = 0;

            virtual void calculate_stacked_photo() = 0;

            virtual void fix_empty_pixels();

            static int get_output_bit_depth(int open_cv_image_type);

            void set_hot_pixels(const std::vector<std::tuple<int, int> > &hot_pixels);

            virtual int get_tasks_total() const = 0;

            const std::atomic<int>& get_tasks_processed() const;

            const std::vector<std::vector<double> > &get_stacked_image() const;

            const int get_width() const  { return m_width; };
            const int get_height() const { return m_height; };

        protected:
            int m_number_of_colors;
            int m_width;
            int m_height;
            bool m_interpolate_colors;

            constexpr static short int c_empty_pixel_value = -1;

            unsigned int m_n_cpu = 1;

            int m_memory_usage_limit_in_mb = -1;

            std::string m_alignment_file_address;

            std::vector<std::string>    m_files_to_stack;
            std::vector<bool>           m_apply_alignment; // for calibration frames we just stack them
            std::vector<std::vector<double> > m_stacked_image;
            std::unique_ptr<FlatFrameHandler> m_flat_frame_handler              = nullptr;
            std::unique_ptr<PhotoAlignmentHandler> m_photo_alignment_handler    = nullptr;
            std::unique_ptr<HotPixelIdentifier> m_hot_pixel_identifier          = nullptr;

            std::atomic<int> m_n_tasks_processed = 0;
    };
}