#include "../headers/ReferencePhotoHandlerSurface.h"
#include "../headers/GaussianBlur.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/MonochromeImageData.h"
#include "../headers/ImageFilesInputOutput.h"
#include "../headers/Common.h"
#include "../headers/ImageRanking.h"

#include "../headers/AlignmentSettingsSurface.h"

#include <opencv2/features2d.hpp>

#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;


ReferencePhotoHandlerSurface::ReferencePhotoHandlerSurface(const InputFrame &reference_frame, float threshold_fraction) : ReferencePhotoHandlerBase(reference_frame, threshold_fraction) {
    m_threshold_fraction = threshold_fraction;
    CalibratedPhotoHandler calibrated_photo_handler(reference_frame, true);
    calibrated_photo_handler.define_alignment(0,0,0,0,0);
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


PlateSolvingResult ReferencePhotoHandlerSurface::calculate_alignment(const InputFrame &input_frame, float *ranking) const {
    std::tuple<std::vector<LocalShift>, PlateSolvingResult, float> result = m_local_shifts_cache.get(input_frame, [this, &input_frame]() {
        return compute_local_shifts_and_alignment(input_frame);
    });

    if (ranking) {
        *ranking = std::get<2>(result);
    }

    return std::get<1>(result);
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

    cv::Ptr<cv::ORB> detector = cv::ORB::create(2000, 1.2f, 8, 31, 0, 2, cv::ORB::HARRIS_SCORE, 31, 20);
    detector->detectAndCompute(cv_image_normalized, cv::noArray(), *keypoints, *descriptors);
};

std::vector<LocalShift> ReferencePhotoHandlerSurface::get_local_shifts( const InputFrame &input_frame) const   {
    std::tuple<std::vector<LocalShift>, PlateSolvingResult, float> result = m_local_shifts_cache.get(input_frame, [this, &input_frame]() {
        return compute_local_shifts_and_alignment(input_frame);
    });

    return std::get<0>(result);
};

const std::vector<std::pair<float,float>> ReferencePhotoHandlerSurface::get_alignment_points() const {
    vector<std::pair<float,float>> alignment_points;
    for (const cv::KeyPoint &kp : m_reference_keypoints) {
        alignment_points.push_back({kp.pt.x, kp.pt.y});
    }
    return alignment_points;
};

std::tuple<std::vector<LocalShift>, PlateSolvingResult, float> ReferencePhotoHandlerSurface::compute_local_shifts_and_alignment(const InputFrame &input_frame) const {
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
    for (const cv::DMatch &match : matches) {
        if (match.distance > 200.0f) {
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
        shift.score = 1.0f - (match.distance / 300.0f);

        local_shifts.push_back(shift);

        shift_sizes_x.push_back(shift.dx);
        shift_sizes_y.push_back(shift.dy);
    }

    if (local_shifts.size() == 0) {
        PlateSolvingResult plate_solving_result;
        plate_solving_result.is_valid = false;
        return {local_shifts, plate_solving_result, ranking};
    }

    std::sort(shift_sizes_x.begin(), shift_sizes_x.end());
    std::sort(shift_sizes_y.begin(), shift_sizes_y.end());


    PlateSolvingResult plate_solving_result;
    plate_solving_result.is_valid = true;

    const float median_shift_x = shift_sizes_x[shift_sizes_x.size()/2];
    const float median_shift_y = shift_sizes_y[shift_sizes_y.size()/2];

    plate_solving_result.shift_x = -median_shift_x;
    plate_solving_result.shift_y = -median_shift_y;

    const float max_allowed_deviation = 20.0f;
    std::vector<LocalShift> selected_local_shifts;
    for (LocalShift shift : local_shifts) {
        const float distance_squared = (shift.dx - median_shift_x)*(shift.dx - median_shift_x) + (shift.dy - median_shift_y)*(shift.dy - median_shift_y);
        if (distance_squared < max_allowed_deviation * max_allowed_deviation) {
            shift.dx -= median_shift_x;
            shift.dy -= median_shift_y;

            shift.x -= shift.dx;
            shift.y -= shift.dy;

            selected_local_shifts.push_back(shift);
        }
    }

    return {selected_local_shifts, plate_solving_result, ranking};
};