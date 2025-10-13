#pragma once

#include "../headers/PixelType.h"

#include "../headers/InputFrame.h"
#include "../headers/Metadata.h"

#include <string>
#include <vector>
#include <array>

namespace AstroPhotoStacker {

    /**
     * Class for reading image data and metadata from input frames (video or still, raw or non-raw).
     *
     * This is THE way how to read any input in the AstroPhotoStacker project.
     */
    class InputFrameReader {
        public:
            /**
             * Constructor for InputFrameReader. It will load image data immediately unless specified otherwise.
             *
             * @param input_frame The input frame to read data from.
             * @param load_image_data Whether to load image data immediately - set to false if you only need metadata.
             */
            InputFrameReader(const InputFrame &input_frame, bool load_image_data = true);

            InputFrameReader() = delete;

            void load_input_frame_data();

            bool data_are_loaded() const;

            Metadata get_metadata() const;

            bool is_raw_file() const;

            bool is_raw_file_before_debayering() const;

            void debayer();

            const std::vector<std::vector<PixelType>> &get_rgb_data();

            const std::vector<PixelType> &get_raw_data() const { return m_raw_data; };

            // we need this for hot pixel correction
            inline std::vector<PixelType>& get_raw_data_non_const() { return m_raw_data; };

            std::vector<PixelType> get_monochrome_data();

            void get_photo_resolution(int *width, int *height);

            int get_width() const { return m_width; };

            int get_height() const { return m_height; };

            PixelType get_pixel_value(int x, int y, int channel) const;

            PixelType get_pixel_value(int pixel_index, int channel) const;

            std::vector<std::vector<PixelType>*> get_all_data_for_calibration();

            char get_raw_color(int x, int y) const;

            std::array<char, 4> get_bayer_pattern() const { return m_bayer_pattern; };

        private:
            InputFrame m_input_frame;
            int m_width = 0;
            int m_height = 0;
            bool m_is_raw_before_debayering = false;
            bool m_is_raw_file = false;
            bool m_data_are_loaded = false;
            Metadata m_metadata;

            std::array<char, 4> m_bayer_pattern = {-1, -1, -1, -1};
            std::vector<PixelType> m_raw_data; // single channel, width*height elements

            std::vector<std::vector<PixelType>> m_rgb_data; // 3 channels, each channel is width*height elements

            void read_raw();

            void read_non_raw();
    };
}