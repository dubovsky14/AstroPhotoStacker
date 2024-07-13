#pragma once

#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/CalibrationFrameBase.h"

#include <string>
#include <vector>


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

            void set_datetime_position(float x_frac, float y_frac);

            void produce_aligned_images(const std::string &output_folder_address) const;

        private:
            int m_top_left_corner_x = 0;
            int m_top_left_corner_y = 0;
            int m_width             = -1;
            int m_height            = -1;

            int m_n_cpu             = 1;

            bool m_add_datetime     = false;
            float m_datetime_pos_frac_x   = 0.7;
            float m_datetime_pos_frac_y   = 0.9;

            std::vector<std::string>                m_files_to_align;
            std::vector<FileAlignmentInformation>   m_alignment_info;

            std::vector<std::shared_ptr<const CalibrationFrameBase> > m_calibration_frame_handlers;

            static std::string get_file_name(const std::string &file_address);

            void produce_aligned_image( const std::string &input_file_address,
                                        const std::string &output_file_address,
                                        const FileAlignmentInformation &alignment_info) const;



    };
}