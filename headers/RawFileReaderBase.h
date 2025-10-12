#pragma once

#include "../headers/FrameReaderBase.h"
#include "../headers/InputFrame.h"
#include "../headers/Metadata.h"
#include "../headers/ThreadSafeCacheSystem.h"
#include "../headers/PixelType.h"

#include <string>
#include <vector>
#include <array>
#include <map>

namespace AstroPhotoStacker {
    class RawFileReaderBase : public FrameReaderBase {
        public:
            RawFileReaderBase() = delete;

            RawFileReaderBase(const InputFrame &input_frame);

            virtual std::vector<PixelType> read_raw_file(int *width, int *height, std::array<char, 4> *bayer_pattern = nullptr) = 0;

            virtual void get_photo_resolution(int *width, int *height) = 0;
    };
}