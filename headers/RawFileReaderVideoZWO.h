#pragma once

#include "../headers/RawFileReaderBase.h"

namespace AstroPhotoStacker {

    class RawFileReaderVideoZWO : public RawFileReaderBase {
        public:
            RawFileReaderVideoZWO(const InputFrame &input_frame) : RawFileReaderBase(input_frame) {};

            virtual std::vector<PixelType> read_raw_file(int *width, int *height, std::array<char, 4> *bayer_pattern = nullptr) override;

            virtual void get_photo_resolution(int *width, int *height) override;

        protected:
            virtual Metadata read_metadata_without_cache() override;

        private:
            static float get_fps_of_video(const std::string &video_address);
    };
}