#pragma once

#include "../headers/InputFrame.h"

#include <string>
#include <map>

namespace AstroPhotoStacker   {
    enum class InputFormatType {
        DSLR_OR_SLR_RAW_FILE,
        FIT_FILE,
        RGB_FILE,
        VIDEO_FRAME_RGB,
        VIDEO_FRAME_RAW_ZWO,
        VIDEO_FRAME_RAW_SER,
        NOT_SUPPORTED
    };

    class InputFormatTypeGetter {
        public:
            static InputFormatType get_input_format_type(const InputFrame &input_frame);

        private:

            static InputFormatType get_input_format_type_without_cache(const std::string &file_address);

            static std::map<std::string, InputFormatType> s_format_type_cache;
    };
}