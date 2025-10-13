#pragma once

#include <memory>

#include "../headers/NonRawFrameReaderBase.h"
#include "../headers/InputFrame.h"

namespace AstroPhotoStacker {
    class NonRawFrameReaderFactory {
        public:
            static std::unique_ptr<NonRawFrameReaderBase> get_non_raw_frame_reader(const InputFrame &input_frame);
    };
}