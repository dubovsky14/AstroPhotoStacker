#pragma once

#include "../headers/PixelType.h"

#include "../headers/FrameReaderBase.h"
#include "../headers/InputFrame.h"
#include "../headers/Metadata.h"

#include <opencv2/opencv.hpp>

namespace AstroPhotoStacker {
    class NonRawFrameReaderBase : public FrameReaderBase {
        public:
            virtual ~NonRawFrameReaderBase() = default;

            NonRawFrameReaderBase() = delete;

            NonRawFrameReaderBase(const InputFrame &input_frame);

            /**
             * @return N channels, each channel is width*height elements
             */
            virtual std::vector<std::vector<PixelType>> get_pixels_data(int *width, int *height) = 0;

            virtual std::vector<PixelType> get_pixels_data_monochrome(int *width, int *height) = 0;

            virtual void get_photo_resolution(int *width, int *height) = 0;


        protected:
            std::vector<std::vector<PixelType>> opencv_rgb_image_to_vector_vector_short(const cv::Mat &image, int *width, int *height, int *bith_depth = nullptr);

            std::vector<PixelType> opencv_grayscale_image_to_vector_short(const cv::Mat &image, int *width, int *height, int *bith_depth = nullptr);
    };
}