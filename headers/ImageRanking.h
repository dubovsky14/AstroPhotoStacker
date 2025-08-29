#pragma once

#include <opencv2/opencv.hpp>

#include <vector>

class ImageRanker {
    public:
        ImageRanker(const std::vector<unsigned short int> &image_brightness, int width, int height);

        float get_sharpness_score() const;

    private:
        cv::Mat m_preprocessed_image;
        cv::Mat m_planet_mask;

};
