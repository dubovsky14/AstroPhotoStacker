#pragma once

#include "../headers/InputFrame.h"
#include "../headers/Metadata.h"

#include <string>
#include <vector>
#include <array>

namespace AstroPhotoStacker {
    class InputFrameReader {
        public:
            InputFrameReader(const InputFrame &input_frame);

            void load_input_frame_data();

            Metadata get_metadata() const;

            bool is_raw_file() const;

            void debayer();

            std::vector<std::vector<short int>> get_rgb_data() const;

            void get_photo_resolution(int *width, int *height) const;

            short int get_pixel_value(int x, int y, int channel) const;

            short int get_pixel_value(int pixel_index, int channel) const;

            std::vector<char> get_used_colors() const;

        private:
            InputFrame m_input_frame;

            std::array<char, 4> m_bayer_pattern = {-1, -1, -1, -1};

            int m_width = 0;
            int m_height = 0;
            std::vector<std::vector<short int>> m_rgb_data; // 3 channels, each channel is width*height elements
            Metadata m_metadata;
            std::vector<short int> m_raw_data; // single channel, width*height elements

            bool m_is_raw_before_debayering;
    };
}