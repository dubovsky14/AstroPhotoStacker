#include "../headers/ReferencePhotoHandlerSurface.h"
#include "../headers/GaussianBlur.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/MonochromeImageData.h"
#include "../headers/ImageFilesInputOutput.h"
#include "../headers/Common.h"
#include "../headers/ImageRanking.h"

#include "../headers/AlignmentSettingsSurface.h"
#include "../headers/AlignmentResultSurface.h"

#include <opencv2/features2d.hpp>

#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;


ReferencePhotoHandlerSurface::ReferencePhotoHandlerSurface(const InputFrame &reference_frame, float threshold_fraction) : ReferencePhotoHandlerBase(reference_frame, threshold_fraction) {
    m_threshold_fraction = threshold_fraction;
    CalibratedPhotoHandler calibrated_photo_handler(reference_frame, true);
    calibrated_photo_handler.calibrate();

    m_width  = calibrated_photo_handler.get_width();
    m_height = calibrated_photo_handler.get_height();
    const vector<vector<PixelType>> &calibrated_data = calibrated_photo_handler.get_calibrated_data_after_color_interpolation();

    vector<PixelType> brightness(m_width*m_height);
    for (int i_pixel = 0; i_pixel < m_width*m_height; i_pixel++) {
        float value = 0;
        int n_points = 0;
        for (unsigned int color = 0; color < calibrated_data.size(); color++) {
            if (calibrated_data[color][i_pixel] >= 0) {
                value += calibrated_data[color][i_pixel];
                n_points++;
            }
        }
        value /= n_points;
        brightness.at(i_pixel) = value;
    }
    initialize(brightness.data(), m_width, m_height, threshold_fraction);
};


ReferencePhotoHandlerSurface::ReferencePhotoHandlerSurface(const PixelType *brightness, int width, int height, float threshold_fraction) : ReferencePhotoHandlerBase(brightness, width, height, threshold_fraction) {
    initialize(brightness, width, height, threshold_fraction);
};

std::unique_ptr<AlignmentResultBase> ReferencePhotoHandlerSurface::calculate_alignment(const InputFrame &input_frame) const {

    InputFrameReader reader(input_frame);
    std::vector<PixelType> brightness = reader.get_monochrome_data();
    const int width = reader.get_width();
    const int height = reader.get_height();

    MonochromeImageData image_data;
    image_data.brightness = brightness.data();
    image_data.width = width;
    image_data.height = height;

    ImageRanker image_ranker(brightness, width, height);
    const double sharpness = image_ranker.get_sharpness_score();
    const float ranking = 100./sharpness;

    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;

    get_keypoints_and_descriptors(brightness.data(), width, height, &keypoints, &descriptors);

    // Match features
    cv::BFMatcher matcher(cv::NORM_L2);
    std::vector<cv::DMatch> matches;
    matcher.match(m_reference_descriptors, descriptors, matches);

    std::vector<float> shift_sizes_x, shift_sizes_y;

    std::vector<LocalShift> local_shifts;
    const float match_distance_threshold = AlignmentSettingsSurface::get_instance()->get_match_distance_threshold();
    for (const cv::DMatch &match : matches) {
        if (match.distance > match_distance_threshold) {
            continue;
        }

        const cv::KeyPoint &ref_kp = m_reference_keypoints[match.queryIdx];
        const cv::KeyPoint &img_kp = keypoints[match.trainIdx];

        LocalShift shift;
        shift.x = img_kp.pt.x;
        shift.y = img_kp.pt.y;
        shift.dx = img_kp.pt.x - ref_kp.pt.x;
        shift.dy = img_kp.pt.y - ref_kp.pt.y;
        shift.valid_ap = true;
        shift.score = 1.0f - (match.distance / (match_distance_threshold+100));

        local_shifts.push_back(shift);

        shift_sizes_x.push_back(shift.dx);
        shift_sizes_y.push_back(shift.dy);
    }

    if (local_shifts.size() == 0) {
        return std::make_unique<AlignmentResultSurface>();
    }

    std::sort(shift_sizes_x.begin(), shift_sizes_x.end());
    std::sort(shift_sizes_y.begin(), shift_sizes_y.end());


    const float median_shift_x = shift_sizes_x[shift_sizes_x.size()/2];
    const float median_shift_y = shift_sizes_y[shift_sizes_y.size()/2];

    const float max_allowed_deviation = AlignmentSettingsSurface::get_instance()->get_maximal_allowed_distance_in_pixels();
    const float max_allowed_deviation_squared = max_allowed_deviation * max_allowed_deviation;
    std::vector<LocalShift> selected_local_shifts;
    std::vector<std::pair<int,int>> keypoint_positions;
    for (LocalShift shift : local_shifts) {
        const float distance_squared = (shift.dx - median_shift_x)*(shift.dx - median_shift_x) + (shift.dy - median_shift_y)*(shift.dy - median_shift_y);
        if (distance_squared < max_allowed_deviation_squared) {
            selected_local_shifts.push_back(shift);
            keypoint_positions.push_back( std::make_pair(shift.x + shift.dx, shift.y + shift.dy) );
        }
    }

    if (selected_local_shifts.size() < 10) {
        return std::make_unique<AlignmentResultSurface>();
    }

    std::vector<float> local_sharpness_values = evalulate_local_sharpness(image_data, keypoint_positions);
    for (unsigned int i_shift = 0; i_shift < selected_local_shifts.size(); i_shift++) {
        selected_local_shifts[i_shift].score = local_sharpness_values[i_shift];
    }

    return make_unique<AlignmentResultSurface>( selected_local_shifts,
                                                ranking);
};

void ReferencePhotoHandlerSurface::initialize(const PixelType *brightness, int width, int height, float threshold_fraction)  {
    m_width = width;
    m_height = height;
    m_threshold_fraction = threshold_fraction;
    initialize_reference_features(brightness);
};


void ReferencePhotoHandlerSurface::initialize_reference_features(const PixelType *brightness_original) {
    MonochromeImageData image_data;
    image_data.brightness = brightness_original;
    image_data.width = m_width;
    image_data.height = m_height;

    get_keypoints_and_descriptors(brightness_original, m_width, m_height, &m_reference_keypoints, &m_reference_descriptors);
};

void ReferencePhotoHandlerSurface::get_keypoints_and_descriptors(   const PixelType *brightness,
                                                                    int width,
                                                                    int height,
                                                                    std::vector<cv::KeyPoint> *keypoints,
                                                                    cv::Mat *descriptors) const    {

    const cv::Mat cv_image(height, width, CV_16UC1, const_cast<PixelType*>(brightness)); // OpenCV needs non-const pointer, because if can moidify the data (it does not in this case though)
    const PixelType max_pixel_value = *std::max_element(brightness, brightness + width * height);
    cv::Mat cv_image_normalized;
    cv_image.convertTo(cv_image_normalized, CV_8UC1, 255.0 / max_pixel_value);

    const bool use_sift = AlignmentSettingsSurface::get_instance()->use_sift_detector();
    if (use_sift) {
        cv::Ptr<cv::SIFT> detector = cv::SIFT::create(2000);
        detector->detectAndCompute(cv_image_normalized, cv::noArray(), *keypoints, *descriptors);
        return;
    }
    cv::Ptr<cv::ORB> detector = cv::ORB::create(2000, 1.2f, 8, 31, 0, 2, cv::ORB::HARRIS_SCORE, 31, 20);
    detector->detectAndCompute(cv_image_normalized, cv::noArray(), *keypoints, *descriptors);
};

std::vector<float> ReferencePhotoHandlerSurface::evalulate_local_sharpness( const MonochromeImageData &image_data_input, const std::vector<std::pair<int,int>> &keypoint_positions) const {
    std::vector<float> local_sharpness_values;
    const int width = image_data_input.width;
    const int height = image_data_input.height;
    const PixelType *brightness = image_data_input.brightness;
    const cv::Mat cv_image(height, width, CV_16UC1, const_cast<PixelType*>(brightness)); // OpenCV needs non-const pointer, because if can moidify the data (it does not in this case though)
    const PixelType max_pixel_value = *std::max_element(brightness, brightness + width * height);
    const PixelType min_pixel_value = *std::min_element(brightness, brightness + width * height);
    if (max_pixel_value == min_pixel_value) {
        return local_sharpness_values;
    }
    cv::Mat cv_image_normalized;
    cv_image.convertTo(cv_image_normalized, CV_32F, 255.0 / (max_pixel_value - min_pixel_value), -255.0 * min_pixel_value / (max_pixel_value - min_pixel_value));

    cv::Mat cv_image_blurred;
    cv::GaussianBlur(cv_image_normalized, cv_image_blurred, cv::Size(11,11), 4);

    cv::Mat laplacian;
    cv::Laplacian(cv_image_blurred, laplacian, CV_32F);

    for (const std::pair<int,int> &position : keypoint_positions) {
        const int x = position.first;
        const int y = position.second;

        const int window_size = 30;
        const int x_min = std::max(0, x - window_size/2);
        const int x_max = std::min(width-1, x + window_size/2);
        const int y_min = std::max(0, y - window_size/2);
        const int y_max = std::min(height-1, y + window_size/2);

        cv::Mat laplacian_roi = laplacian(cv::Range(y_min, y_max+1), cv::Range(x_min, x_max+1));

        // Masked variance
        cv::Scalar mean, stddev;
        cv::meanStdDev(laplacian_roi, mean, stddev);
        local_sharpness_values.push_back(stddev[0] * stddev[0]); // variance
    }

    return local_sharpness_values;
};
