#include "../headers/ReferencePhotoHandlerSurface.h"
#include "../headers/GaussianBlur.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/MonochromeImageData.h"
#include "../headers/ImageFilesInputOutput.h"
#include "../headers/Common.h"
#include "../headers/ImageRanking.h"
#include "../headers/LocalShiftsClusteringTool.h"

#include "../headers/AlignmentResultSurface.h"

#include <opencv2/features2d.hpp>

#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;


ReferencePhotoHandlerSurface::ReferencePhotoHandlerSurface(const InputFrame &reference_frame, const ConfigurableAlgorithmSettingsMap &configuration_map) : ReferencePhotoHandlerBase(reference_frame, configuration_map) {
    define_configuration_settings();
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
    initialize(brightness.data(), m_width, m_height, configuration_map);
};


ReferencePhotoHandlerSurface::ReferencePhotoHandlerSurface(const PixelType *brightness, int width, int height, const ConfigurableAlgorithmSettingsMap &configuration_map) : ReferencePhotoHandlerBase(brightness, width, height, configuration_map) {
    define_configuration_settings();
    initialize(brightness, width, height);
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

    const int gaussian_kernel_size = 2 *int(m_gaussian_sigma + 0.5) + 1; // we need this to be odd
    ImageRanker image_ranker(brightness, width, height, gaussian_kernel_size, m_gaussian_sigma);
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
    for (const cv::DMatch &match : matches) {
        if (match.distance > m_match_distance_threshold) {
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
        shift.score = 1.0f - (match.distance / (m_match_distance_threshold+100));

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

    const float max_allowed_deviation_squared = m_maximal_allowed_shift_in_pixels * m_maximal_allowed_shift_in_pixels;
    std::vector<LocalShift> selected_local_shifts;
    for (LocalShift shift : local_shifts) {
        const float distance_squared = (shift.dx - median_shift_x)*(shift.dx - median_shift_x) + (shift.dy - median_shift_y)*(shift.dy - median_shift_y);
        if (distance_squared < max_allowed_deviation_squared) {
            selected_local_shifts.push_back(shift);
        }
    }
    if (selected_local_shifts.size() < 10) {
        return std::make_unique<AlignmentResultSurface>();
    }

    LocalShiftsClusteringTool clustering_tool(0.01 * sqrt(width*width + height*height));
    selected_local_shifts = clustering_tool.cluster_local_shifts(selected_local_shifts);

    return make_unique<AlignmentResultSurface>(selected_local_shifts, ranking);
};

void ReferencePhotoHandlerSurface::initialize(const PixelType *brightness, int width, int height, const ConfigurableAlgorithmSettingsMap &configuration_map)  {
    m_width = width;
    m_height = height;
    m_configurable_algorithm_settings.set_values_from_configuration_map(configuration_map);
    initialize_reference_features(brightness);
};


void ReferencePhotoHandlerSurface::initialize_reference_features(const PixelType *brightness_original) {
    MonochromeImageData image_data;
    image_data.brightness = brightness_original;
    image_data.width = m_width;
    image_data.height = m_height;

    get_keypoints_and_descriptors(brightness_original, m_width, m_height, &m_reference_keypoints, &m_reference_descriptors);
};

void ReferencePhotoHandlerSurface::define_configuration_settings()   {
    m_configurable_algorithm_settings.add_additional_setting_numerical("Gaussian sigma for denoising", &m_gaussian_sigma, 0.1, 15.0, 0.2);
    m_configurable_algorithm_settings.add_additional_setting_numerical("Maximal allowed shift in pixels", &m_maximal_allowed_shift_in_pixels, 1, 30, 0.2);
    m_configurable_algorithm_settings.add_additional_setting_numerical("Match distance threshold", &m_match_distance_threshold, 50, 1000, 1);
    m_configurable_algorithm_settings.add_additional_setting_numerical("Number of features to detect", &m_n_features_to_detect, 100, 10000, 10);
    m_configurable_algorithm_settings.add_additional_setting_bool("Use SIFT features detector", &m_use_sift_features_detector);
};

void ReferencePhotoHandlerSurface::get_keypoints_and_descriptors(   const PixelType *brightness,
                                                                    int width,
                                                                    int height,
                                                                    std::vector<cv::KeyPoint> *keypoints,
                                                                    cv::Mat *descriptors) const    {

    vector<unsigned char> normalized_data(width * height);
    const PixelType max_pixel_value = *std::max_element(brightness, brightness + width * height);
    for (int i = 0; i < width * height; i++) {
        normalized_data[i] = static_cast<unsigned char>( (static_cast<float>(brightness[i]) / max_pixel_value) * 255.0f );
    }
    cv::Mat cv_image_normalized(height, width, CV_8UC1, normalized_data.data());

    if (m_use_sift_features_detector) {
        cv::Ptr<cv::SIFT> detector = cv::SIFT::create(m_n_features_to_detect);
        detector->detectAndCompute(cv_image_normalized, cv::noArray(), *keypoints, *descriptors);
        return;
    }
    cv::Ptr<cv::ORB> detector = cv::ORB::create(m_n_features_to_detect, 1.05f, 50, 27, 0, 2, cv::ORB::HARRIS_SCORE, 27, 15);
    detector->detectAndCompute(cv_image_normalized, cv::noArray(), *keypoints, *descriptors);
};
