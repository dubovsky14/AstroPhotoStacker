#pragma once

#include "../headers/PixelType.h"

#include <opencv2/opencv.hpp>

#include <vector>

class ImageRanker {
    public:
        ImageRanker(const std::vector<PixelType> &image_brightness, int width, int height);

        float get_sharpness_score() const;

    private:
        cv::Mat m_preprocessed_image;
        cv::Mat m_planet_mask;

};
