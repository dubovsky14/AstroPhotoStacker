#pragma once

#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/CalibrationFrameBase.h"
#include "../headers/HotPixelIdentifier.h"
#include "../headers/TimeLapseVideoSettings.h"
#include "../headers/InputFrame.h"
#include "../headers/StackerBase.h"
#include "../headers/StackSettings.h"

#include <functional>
#include <string>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

namespace AstroPhotoStacker {
    struct GroupToStack {
        std::vector<InputFrame>                 input_frames;
        std::vector<FileAlignmentInformation>   alignment_info;
        StackSettings                           stack_settings;
    };
    class AlignedImagesProducer {
        public:
            AlignedImagesProducer(int n_cpu = 1, int memory_usage_limit_in_mb = 8192);

            void limit_output_image_size(int top_left_corner_x, int top_left_corner_y, int width, int height);

            void add_calibration_frame_handler(std::shared_ptr<const CalibrationFrameBase> calibration_frame_handler);

            void add_image(const InputFrame &input_frame, const FileAlignmentInformation &alignment_info = FileAlignmentInformation());

            void add_image_group_to_stack(const std::vector<InputFrame> &input_frames, const std::vector<FileAlignmentInformation> &alignment_info, const StackSettings &stack_settings);

            void set_add_datetime(bool add_datetime)    {
                m_add_datetime = add_datetime;
            };

            bool get_add_datetime() const {
                return m_add_datetime;
            };

            void set_datetime_position(float x_frac, float y_frac);

            void produce_aligned_images(const std::string &output_folder_address);

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

            void set_hot_pixel_identifier(const HotPixelIdentifier *hot_pixel_identifier) {
                m_hot_pixel_identifier = hot_pixel_identifier;
            };

            std::pair<float,float> get_position_of_datetime() const {
                return {m_datetime_pos_frac_x, m_datetime_pos_frac_y};
            };

            void set_maximal_output_image_size(int width, int height) {
                m_max_width = width;
                m_max_height = height;
            };


            static std::string get_output_file_name(const InputFrame &input_frame);

            static void scale_down_image(std::vector<std::vector<unsigned short>> *image, unsigned int origianal_max, unsigned int new_max);

            void produce_video(const std::string &output_video_address)   const;

            TimeLapseVideoSettings *get_timelapse_video_settings();

        private:
            int m_top_left_corner_x = 0;
            int m_top_left_corner_y = 0;
            int m_width             = -1;
            int m_height            = -1;

            int m_max_width         = -1;
            int m_max_height        = -1;

            int m_n_cpu             = 1;
            int m_memory_usage_limit_in_mb = 8192;

            bool m_add_datetime     = false;
            float m_datetime_pos_frac_x   = 0.6;
            float m_datetime_pos_frac_y   = 0.9;

            int m_timestamp_offset = 0;

            std::function<void(std::vector<std::vector<unsigned short>>*, unsigned short max_value)> m_image_stretching_function = nullptr;

            std::vector<InputFrame>                 m_frames_to_align;
            std::vector<FileAlignmentInformation>   m_alignment_info;

            std::vector<std::shared_ptr<const CalibrationFrameBase> > m_calibration_frame_handlers;
            const HotPixelIdentifier *m_hot_pixel_identifier = nullptr;
            TimeLapseVideoSettings m_timelapse_video_settings;

            void produce_aligned_image( const InputFrame &input_frame, const std::string &output_file_address, const FileAlignmentInformation &alignment_info);

            void produce_aligned_image( const GroupToStack &group_to_stack, const std::string &output_file_address);

            void process_and_save_image(std::vector<std::vector<unsigned short>> *stacked_image,
                                        int width,
                                        int height,
                                        const std::string &output_file_address,
                                        int unix_time) const;

            mutable std::atomic<int> m_n_tasks_processed = 0;
            std::vector<std::tuple<std::string, int>>   m_output_adresses_and_unix_times;
            std::mutex                                  m_output_adresses_and_unix_times_mutex;


            std::vector<GroupToStack> m_groups_to_stack;

    };
}