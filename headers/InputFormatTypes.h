#pragma once

#include "../headers/InputFrame.h"
#include "../headers/ThreadSafeCacheSystem.h"

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

    /**
     * @brief Check if the file is a raw file
     *
     * @param file_address - path to the file
     * @return true - if the file is a raw file
     * @return false - if the file is not a raw file
    */
    bool is_raw_file(const std::string &file_address);

    class InputFormatTypeGetter {
        public:
            static InputFormatType get_input_format_type(const InputFrame &input_frame);

            static InputFormatType get_input_format_type(const std::string &input_address);

        private:

            static InputFormatType get_input_format_type_without_cache(const std::string &file_address);

            static ThreadSafeCacheSystem<std::string, InputFormatType> s_format_type_cache;
    };
}