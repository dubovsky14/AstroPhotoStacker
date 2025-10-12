#pragma once

#include "../headers/RawFileReaderBase.h"

#include <libraw/libraw.h>

namespace AstroPhotoStacker {
    class RawFileReaderDSLR : public RawFileReaderBase {
        public:
            RawFileReaderDSLR(const InputFrame &input_frame) : RawFileReaderBase(input_frame) {};

            virtual std::vector<PixelType> read_raw_file(int *width, int *height, std::array<char, 4> *bayer_pattern = nullptr) override;

            virtual void get_photo_resolution(int *width, int *height) override;

        protected:
            virtual Metadata read_metadata_without_cache() override;

        private:
            LibRaw get_libraw_processor();
    };

    bool is_raw_file_dslr_slr(const std::string &file_address);
}