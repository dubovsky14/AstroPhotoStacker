#pragma once

#include "../headers/PixelType.h"

#include <opencv2/opencv.hpp>

#include <vector>

class ImageRanker {
    public:
        //ImageRanker(const std::vector<PixelType> &image_brightness, int width, int height, int gaussian_kernel_size = 11, double gaussian_sigma = 6.0);

        ImageRanker(const std::vector<PixelType> &image_brightness, int width, int height, int gaussian_kernel_size, double gaussian_sigma);

        float get_sharpness_score() const;

    private:
        cv::Mat m_preprocessed_image;
        cv::Mat m_planet_mask;

};
