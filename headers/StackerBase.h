#pragma once

#include "../headers/FlatFrameHandler.h"
#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/ImageStretching.h"


#include <memory>
#include <string>
#include <vector>


namespace AstroPhotoStacker {
    class StackerBase   {
        public:
            StackerBase()                   = delete;
            StackerBase(const StackerBase&) = delete;

            StackerBase(int number_of_colors, int width, int height);

            void set_memory_usage_limit(int memory_usage_limit_in_mb);

            void add_alignment_text_file(const std::string &alignment_file_address);

            virtual void add_photo(const std::string &file_address);

            virtual void add_flat_frame(const std::string &file_address);

            virtual void save_stacked_photo(const std::string &file_address, int image_options = 18) const;

            virtual void set_number_of_cpu_threads(unsigned int n_cpu) = 0;

            virtual void calculate_stacked_photo() = 0;

            virtual void stretch_stacked_photo(StretchingType stretching_type, unsigned int n_bits = 14);

            virtual void apply_black_point(double black_pixels_fraction);

        protected:
            int m_number_of_colors;
            int m_width;
            int m_height;

            unsigned int m_n_cpu = 1;

            int m_memory_usage_limit_in_mb = -1;

            std::string m_alignment_file_address;

            std::vector<std::string> m_files_to_stack;
            std::vector<std::vector<double> > m_stacked_image;
            std::unique_ptr<FlatFrameHandler> m_flat_frame_handler              = nullptr;
            std::unique_ptr<PhotoAlignmentHandler> m_photo_alignment_handler    = nullptr;
            std::unique_ptr<ImageStretcher> m_image_stretcher                   = nullptr;
    };
}