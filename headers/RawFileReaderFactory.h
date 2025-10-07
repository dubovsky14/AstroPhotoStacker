#pragma once

#include "../headers/RawFileReaderBase.h"

#include <string>
#include <memory>

namespace AstroPhotoStacker {
    class RawFileReaderFactory {
        public:
            static std::unique_ptr<RawFileReaderBase> get_raw_file_reader(const InputFrame &input_frame);
    };
}