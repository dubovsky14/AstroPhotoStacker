#pragma once

#include "../headers/FrameReaderBase.h"
#include "../headers/InputFrame.h"
#include "../headers/Metadata.h"



namespace AstroPhotoStacker {
    class NonRawFrameReaderBase : public FrameReaderBase {
        public:
            virtual ~NonRawFrameReaderBase() = default;

            NonRawFrameReaderBase() = delete;

            NonRawFrameReaderBase(const InputFrame &input_frame);

            /**
             * @return N channels, each channel is width*height elements
             */
            virtual std::vector<std::vector<short int>> get_pixels_data(int *width, int *height) = 0;

            virtual void get_photo_resolution(int *width, int *height) = 0;
    };
}