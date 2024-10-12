#pragma once

#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/CalibrationFrameBase.h"

#include <functional>
#include <string>
#include <vector>
#include <atomic>


namespace AstroPhotoStacker {

    class AlignedImagesProducer {
        public:
            AlignedImagesProducer(int n_cpu = 1);

            void limit_output_image_size(int top_left_corner_x, int top_left_corner_y, int width, int height);

            void add_calibration_frame_handler(std::shared_ptr<const CalibrationFrameBase> calibration_frame_handler);

            void add_image(const std::string &file_address, const FileAlignmentInformation &alignment_info = FileAlignmentInformation());

            void set_add_datetime(bool add_datetime)    {
                m_add_datetime = add_datetime;
            };

            bool get_add_datetime() const {
                return m_add_datetime;
            };

            void set_datetime_position(float x_frac, float y_frac);

            void produce_aligned_images(const std::string &output_folder_address) const;

            const std::atomic<int>& get_tasks_processed() const;

            int get_tasks_total() const;

            void set_image_stretching_function(std::function<void(std::vector<std::vector<unsigned short>>*, unsigned short max_value)> image_stretching_function) {
                m_image_stretching_function = image_stretching_function;
            };

            void set_timestamp_offset(int timestamp_offset) {
                m_timestamp_offset = timestamp_offset;
            };

            int get_timestamp_offset() const {
                return m_timestamp_offset;
            };

            std::pair<float,float> get_position_of_datetime() const {
                return {m_datetime_pos_frac_x, m_datetime_pos_frac_y};
            };


            static std::string get_output_file_name(const std::string &input_file_address);

            static void scale_down_image(std::vector<std::vector<unsigned short>> *image, unsigned int origianal_max, unsigned int new_max);

            static void apply_green_correction(std::vector<std::vector<unsigned short>> *image, unsigned short max_value);

        private:
            int m_top_left_corner_x = 0;
            int m_top_left_corner_y = 0;
            int m_width             = -1;
            int m_height            = -1;

            int m_n_cpu             = 1;

            bool m_add_datetime     = false;
            float m_datetime_pos_frac_x   = 0.6;
            float m_datetime_pos_frac_y   = 0.9;

            int m_timestamp_offset = 0;

            std::function<void(std::vector<std::vector<unsigned short>>*, unsigned short max_value)> m_image_stretching_function = nullptr;

            std::vector<std::string>                m_files_to_align;
            std::vector<FileAlignmentInformation>   m_alignment_info;

            std::vector<std::shared_ptr<const CalibrationFrameBase> > m_calibration_frame_handlers;


            void produce_aligned_image( const std::string &input_file_address,
                                        const std::string &output_file_address,
                                        const FileAlignmentInformation &alignment_info) const;


            mutable std::atomic<int> m_n_tasks_processed = 0;
    };
}