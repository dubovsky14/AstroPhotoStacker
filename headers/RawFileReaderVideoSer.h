#pragma once

#include "../headers/RawFileReaderBase.h"

namespace AstroPhotoStacker {
    class RawFileReaderVideoSer : public RawFileReaderBase {
        public:
            RawFileReaderVideoSer(const InputFrame &input_frame) : RawFileReaderBase(input_frame) {};

            virtual std::vector<PixelType> read_raw_file(int *width, int *height, std::array<char, 4> *bayer_pattern = nullptr) override;

            virtual void get_photo_resolution(int *width, int *height) override;

            static unsigned int microsoft_to_unix_time(unsigned long long int microsoft_time);

        protected:
            virtual Metadata read_metadata_without_cache() override;

        private:
            unsigned int read_uint_from_file(std::ifstream *file, size_t position_in_file) const;

            unsigned long long int read_ulonglong_from_file(std::ifstream *file, size_t position_in_file, bool little_endian = true) const;

            static std::string int_code_to_bayer_matrix(unsigned int code);
    };

    bool is_ser_file(const std::string &file_address);
}