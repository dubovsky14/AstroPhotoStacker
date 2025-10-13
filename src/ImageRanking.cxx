#include "../headers/ImageRanking.h"

#include <iostream>

using namespace std;

ImageRanker::ImageRanker(const vector<PixelType> &image_brightness, int width, int height) {
    // Preprocess the image and create the planet mask
    // 1) Wrap raw data into cv::Mat (16-bit signed, single channel)

    // Check if PixelType is short int
    static_assert(sizeof(PixelType) == sizeof(short int), "PixelType must be short int");
    cv::Mat img16(height, width, CV_16UC1, (void*)image_brightness.data());

    // Convert to float for math
    cv::Mat img;
    img16.convertTo(img, CV_32F);

    cv::Mat mask;
    cv::threshold(img16, mask, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    mask.convertTo(mask, CV_8U);


    // 3) Keep only the largest connected component (planet)
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    int best_idx = -1, best_area = 0;
    for (int i = 0; i < (int)contours.size(); i++) {
        int a = (int)cv::contourArea(contours[i]);
        if (a > best_area) {
            best_area = a;
            best_idx = i;
        }
    }

    m_planet_mask = cv::Mat::zeros(mask.size(), CV_8U);
    if (best_idx >= 0) {
        cv::drawContours(m_planet_mask, contours, best_idx, cv::Scalar(255), cv::FILLED);
    }

    // Optionally erode a bit to avoid limb artifacts
    cv::erode(m_planet_mask, m_planet_mask, cv::Mat(), cv::Point(-1,-1), 1);

    // 4) Light denoise with small Gaussian blur
    cv::GaussianBlur(img, m_preprocessed_image, cv::Size(11,11), 4);
}

float ImageRanker::get_sharpness_score() const {
    cv::Mat lap;
    cv::Laplacian(m_preprocessed_image, lap, CV_32F);

    // Masked variance
    cv::Scalar mean, stddev;
    cv::meanStdDev(lap, mean, stddev, m_planet_mask);
    return stddev[0] * stddev[0]; // variance
}