#include "../headers/FrameRankingTool.h"

#include "../headers/StarFinder.h"
#include "../headers/ClusterAlgebra.h"
#include "../headers/CommonImageOperations.h"
#include "../headers/FWHMCalculator.h"

#include <stdexcept>
#include <algorithm>
#include <tuple>
#include <opencv2/opencv.hpp>

using namespace AstroPhotoStacker;
using namespace std;


FrameScore FrameRankingTool::get_ranking_for_deep_sky_objects(const std::vector<PixelType>& brightness, int width, int height, PixelType cluster_threshold) {
    std::vector<std::vector<std::tuple<int,int>>> clusters = get_clusters(brightness.data(), width, height, cluster_threshold);

    vector<float> cluster_excentricities;
    for (const auto &cluster : clusters) {
        if (cluster.size() < 20)    continue;
        cluster_excentricities.push_back(ClusterAlgebra::get_cluster_excentricity(cluster));
    }

    if (cluster_excentricities.size() == 0) {
        throw std::runtime_error("No clusters found in frame");
    }

    sort(cluster_excentricities.begin(), cluster_excentricities.end());

    vector<tuple<float,float,int>> stars(clusters.size(), std::make_tuple(0.0f, 0.0f, 0));
    for (size_t i_cluster = 0; i_cluster < clusters.size(); ++i_cluster) {
        for (const std::tuple<int,int> &pixel : clusters.at(i_cluster)) {
            get<0>(stars.at(i_cluster)) += std::get<0>(pixel);
            get<1>(stars.at(i_cluster)) += std::get<1>(pixel);
        }
        get<0>(stars.at(i_cluster)) /= clusters.at(i_cluster).size();
        get<1>(stars.at(i_cluster)) /= clusters.at(i_cluster).size();
        get<2>(stars.at(i_cluster)) = static_cast<int>(clusters.at(i_cluster).size());
    }

    FWHMCalculator fwhm_calculator(brightness, width, height, stars, cluster_threshold);
    const float fwhm = fwhm_calculator.calculate_fwhm();

    FrameScore result;
    result.stars_excentricity = cluster_excentricities.at(cluster_excentricities.size() / 5);
    result.stars_fwhm = fwhm;
    result = add_brighness_info(brightness, result);
    return result;
};


FrameScore FrameRankingTool::get_ranking_for_planetary_objects(const std::vector<PixelType>& brightness, int width, int height, int gaussian_kernel_size, double gaussian_sigma) {
    // Preprocess the image and create the planet mask
    // 1) Wrap raw data into cv::Mat (16-bit signed, single channel)

    // Check if PixelType is short int
    static_assert(sizeof(PixelType) == sizeof(short int), "PixelType must be short int");
    cv::Mat img16(height, width, CV_16UC1, (void*)brightness.data());

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

    cv::Mat planet_mask = cv::Mat::zeros(mask.size(), CV_8U);
    if (best_idx >= 0) {
        cv::drawContours(planet_mask, contours, best_idx, cv::Scalar(255), cv::FILLED);
    }

    // Optionally erode a bit to avoid limb artifacts
    cv::erode(planet_mask, planet_mask, cv::Mat(), cv::Point(-1,-1), 1);

    // 4) Light denoise with small Gaussian blur
    cv::Mat preprocessed_image;
    cv::GaussianBlur(img, preprocessed_image, cv::Size(gaussian_kernel_size,gaussian_kernel_size), gaussian_sigma);

    cv::Mat lap;
    cv::Laplacian(preprocessed_image, lap, CV_32F);

    // Masked variance
    cv::Scalar mean, stddev;
    cv::meanStdDev(lap, mean, stddev, planet_mask);

    FrameScore result;
    result.sharpness_score = 100./(stddev[0] * stddev[0]);
    result = add_brighness_info(brightness, result);

    return result;

};


FrameScore FrameRankingTool::get_ranking_otsu_based(const std::vector<PixelType>& brightness, int width, int height)   {
    // Create a copy of the brightness data as unsigned short
    std::vector<unsigned short> brightness_copy(width*height);
    for (int i = 0; i < width*height; i++) {
        brightness_copy[i] = max<short>(brightness.at(i), 0);
    }

    const unsigned short otsu_threshold = get_otsu_threshold(brightness_copy.data(), width*height);

    int count_above_threshold = 0;
    for (int i = 0; i < width*height; i++) {
        if (brightness_copy[i] > otsu_threshold) {
            count_above_threshold++;
        }
    }

    FrameScore result;
    result.sharpness_score = 100. * static_cast<float>(count_above_threshold) / static_cast<float>(width*height);
    result = add_brighness_info(brightness, result);
    return result;
}


FrameScore FrameRankingTool::add_brighness_info(const std::vector<PixelType>& brightness, FrameScore frame_score)  {
    unsigned long long sum_brightness           = 0;
    unsigned long long sum_brightness_squared   = 0;
    for (const PixelType &pixel : brightness) {
        frame_score.brightness_min = std::min<unsigned int>(frame_score.brightness_min, pixel);
        frame_score.brightness_max = std::max<unsigned int>(frame_score.brightness_max, pixel);
        sum_brightness += pixel;
        sum_brightness_squared += pixel * static_cast<unsigned long long>(pixel);
    }

    const double pixel_count = brightness.size();

    frame_score.brightness_mean = static_cast<float>(sum_brightness) / pixel_count;
    frame_score.brightness_std  = static_cast<float>(std::sqrt(sum_brightness_squared / pixel_count - frame_score.brightness_mean * frame_score.brightness_mean));

    return frame_score;
};

