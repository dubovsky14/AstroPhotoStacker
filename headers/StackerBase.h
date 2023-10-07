#pragma once

#include "../headers/FlatFrameHandler.h"
#include "../headers/PhotoAlignmentHandler.h"

#include <memory>
#include <string>
#include <vector>


namespace AstroPhotoStacker {
    class StackerBase   {
        public:
            StackerBase(int number_of_colors, int width, int height);

            void set_memory_usage_limit(int memory_usage_limit_in_mb);

            void add_alignment_text_file(const std::string &alignment_file_address);

            virtual void add_photo(const std::string &file_address);

            virtual void add_flat_frame(const std::string &file_address);

            virtual void save_stacked_photo(const std::string &file_address, int image_options = 18) const;

            virtual void calculate_stacked_photo() = 0;

        protected:
            int m_number_of_colors;
            int m_width;
            int m_height;

            int m_memory_usage_limit_in_mb = -1;

            std::string m_alignment_file_address;

            std::vector<std::string> m_files_to_stack;
            std::vector<std::vector<double> > m_stacked_image;
            std::unique_ptr<FlatFrameHandler> m_flat_frame_handler              = nullptr;
            std::unique_ptr<PhotoAlignmentHandler> m_photo_alignment_handler    = nullptr;
    };
}