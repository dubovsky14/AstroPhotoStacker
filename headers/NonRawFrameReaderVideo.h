#pragma once

#include "../headers/NonRawFrameReaderBase.h"
#include "../headers/InputFrame.h"

namespace AstroPhotoStacker {
    class NonRawFrameReaderVideo : public NonRawFrameReaderBase {
        public:
            NonRawFrameReaderVideo() = delete;

            NonRawFrameReaderVideo(const InputFrame &input_frame);

            virtual std::vector<std::vector<PixelType>> get_pixels_data(int *width, int *height) override;

            virtual std::vector<PixelType> get_pixels_data_monochrome(int *width, int *height) override;

            virtual void get_photo_resolution(int *width, int *height) override;

        protected:
            int m_width  = -1;
            int m_height = -1;

            virtual Metadata read_metadata_without_cache() override;
    };
}