#pragma once

#include "../headers/RawFileReaderBase.h"

namespace AstroPhotoStacker {

    class RawFileReaderVideoZWO : public RawFileReaderBase {
        public:
            virtual std::vector<short int> read_raw_file(int *width, int *height, std::array<char, 4> *bayer_pattern) override;

            virtual void get_photo_resolution(int *width, int *height) override;

            virtual Metadata read_metadata() override;

        private:
            static float get_fps_of_video(const std::string &video_address);
    };
}