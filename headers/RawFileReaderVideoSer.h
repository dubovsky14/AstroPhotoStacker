#pragma once

#include "../headers/RawFileReaderBase.h"

namespace AstroPhotoStacker {
    class RawFileReaderVideoSer : public RawFileReaderBase {
        public:
            virtual std::vector<short int> read_raw_file(int *width, int *height, std::array<char, 4> *bayer_pattern) override;

            virtual void get_photo_resolution(int *width, int *height) override;

            virtual Metadata read_metadata() override;

            static unsigned int microsoft_to_unix_time(unsigned long long int microsoft_time);


        private:
            unsigned int read_uint_from_file(std::ifstream *file, size_t position_in_file) const;

            unsigned long long int read_ulonglong_from_file(std::ifstream *file, size_t position_in_file, bool little_endian = true) const;

    };
}