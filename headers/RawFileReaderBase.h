#pragma once

#include "../headers/InputFrame.h"
#include "../headers/Metadata.h"

#include <string>
#include <vector>
#include <array>

namespace AstroPhotoStacker {
    class RawFileReaderBase {
        public:
            virtual ~RawFileReaderBase() = default;

            RawFileReaderBase(const InputFrame &input_frame);

            virtual std::vector<short int> read_raw_file(int *width, int *height, std::array<char, 4> *bayer_pattern = nullptr) = 0;

            virtual void get_photo_resolution(int *width, int *height) = 0;

            virtual Metadata read_metadata() = 0;

        protected:
            InputFrame m_input_frame;
    };
}