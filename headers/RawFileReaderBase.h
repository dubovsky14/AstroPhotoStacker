#pragma once

#include "../headers/InputFrame.h"
#include "../headers/Metadata.h"

#include <string>
#include <vector>
#include <array>
#include <map>
#include <shared_mutex>

namespace AstroPhotoStacker {
    class RawFileReaderBase {
        public:
            virtual ~RawFileReaderBase() = default;

            RawFileReaderBase() = delete;

            RawFileReaderBase(const InputFrame &input_frame);

            virtual std::vector<short int> read_raw_file(int *width, int *height, std::array<char, 4> *bayer_pattern = nullptr) = 0;

            virtual void get_photo_resolution(int *width, int *height) = 0;

            Metadata read_metadata();

        protected:
            InputFrame m_input_frame;

            virtual Metadata read_metadata_without_cache() = 0;

            static std::map<std::string, Metadata> s_metadata_cache;
            static std::shared_mutex s_metadata_mutex;
    };
}