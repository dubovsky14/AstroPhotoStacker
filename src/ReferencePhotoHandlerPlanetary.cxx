#include "../headers/StarFinder.h"
#include "../headers/ReferencePhotoHandlerPlanetary.h"
#include "../headers/InputFrameReader.h"
#include "../headers/StarFinder.h"
#include "../headers/ImageRanking.h"
#include "../headers/Common.h"
#include "../headers/CommonImageOperations.h"
#include "../headers/AlignmentResultTranslationOnly.h"

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <cmath>
#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;

ReferencePhotoHandlerPlanetary::ReferencePhotoHandlerPlanetary(const InputFrame &input_frame, const ConfigurableAlgorithmSettingsMap &configuration_map)   :
    ReferencePhotoHandlerBase(input_frame, configuration_map) {

    define_configuration_settings();
    const vector<PixelType> brightness = read_image_monochrome(input_frame, &m_width, &m_height);
    initialize(brightness.data(), m_width, m_height, configuration_map);
};

ReferencePhotoHandlerPlanetary::ReferencePhotoHandlerPlanetary(const PixelType *brightness, int width, int height, const ConfigurableAlgorithmSettingsMap &configuration_map)  :
    ReferencePhotoHandlerBase(brightness, width, height, configuration_map) {
    define_configuration_settings();
    initialize(brightness, width, height, configuration_map);
};

std::unique_ptr<AlignmentResultBase> ReferencePhotoHandlerPlanetary::calculate_alignment(const InputFrame &input_frame) const{
    int width, height;
    const vector<PixelType> brightness = read_image_monochrome(input_frame, &width, &height);

    MonochromeImageData image_data;
    image_data.brightness = brightness.data();
    image_data.width = width;
    image_data.height = height;

    AlignmentWindow alignment_window;
    const auto [center_of_mass_x, center_of_mass_y, eigenvec, eigenval] = get_center_of_mass_eigenvectors_and_eigenvalues(image_data, &alignment_window);

    const float shift_x = m_center_of_mass_x - center_of_mass_x;
    const float shift_y = m_center_of_mass_y - center_of_mass_y;

    double sharpness_score = 0;
    if (m_use_number_of_pixels_above_otsu_threshold_for_ranking) {
        const float fraction_of_pixels_above_otsu_threshold = ImageRanker::get_fraction_of_pixels_above_otsu_threshold(brightness, width, height);
        sharpness_score = 100.f * fraction_of_pixels_above_otsu_threshold;
    }
    else {
        const int gaussian_kernel_size = 2 *int(m_gaussian_sigma + 0.5) + 1; // we need this to be odd
        ImageRanker image_ranker(brightness, width, height, gaussian_kernel_size, m_gaussian_sigma);
        sharpness_score = 100./image_ranker.get_sharpness_score();
    }

    if (m_zero_rotation) {
        std::unique_ptr<AlignmentResultTranslationOnly> plate_solving_result = std::make_unique<AlignmentResultTranslationOnly>(shift_x, shift_y);
        plate_solving_result->set_ranking_score(sharpness_score);
        return plate_solving_result;
    }



    const float rotation_center_x = center_of_mass_x;
    const float rotation_center_y = center_of_mass_y;
    const double sin_angle = m_covariance_eigen_vectors[0][0]*eigenvec[0][1] - m_covariance_eigen_vectors[0][1]*eigenvec[0][0];
    const float rotation = std::asin(sin_angle);

    std::unique_ptr<AlignmentResultPlateSolving> plate_solving_result = std::make_unique<AlignmentResultPlateSolving>(  shift_x,
                                                                                                                        shift_y,
                                                                                                                        rotation_center_x,
                                                                                                                        rotation_center_y,
                                                                                                                        rotation);
    plate_solving_result->set_ranking_score(sharpness_score);
    return plate_solving_result;
};

AlignmentWindow ReferencePhotoHandlerPlanetary::get_alignment_window(   const MonochromeImageData &image_data,
                                                                        PixelType threshold) const    {

    const PixelType *brightness = image_data.brightness;
    const int width = image_data.width;
    const int height = image_data.height;

    std::vector< std::vector<std::tuple<int, int> > > clusters = get_clusters_non_recursive(brightness, width, height, threshold);

    if (clusters.size() == 0) {
        throw runtime_error("No clusters found in the reference photo");
    }
    if (clusters.at(0).size()  < 10) {
        throw runtime_error("No clusters found in the reference photo");
    }

    const vector<tuple<int,int>> &leading_cluster = *std::max_element(clusters.begin(), clusters.end(), []
                                                    (const std::vector<std::tuple<int, int> > &a, const std::vector<std::tuple<int, int> > &b)
                                                    {return a.size() < b.size();});

    int window_x_min = INT_MAX;
    int window_x_max = INT_MIN;
    int window_y_min = INT_MAX;
    int window_y_max = INT_MIN;

    for (const auto &[x,y] : leading_cluster) {
        window_x_min = min(window_x_min, x);
        window_x_max = max(window_x_max, x);
        window_y_min = min(window_y_min, y);
        window_y_max = max(window_y_max, y);
    }

    const int border_size = 10;
    AlignmentWindow alignment_window;
    alignment_window.x_min = max(0, window_x_min - border_size);
    alignment_window.x_max = min(width, window_x_max + border_size);
    alignment_window.y_min = max(0, window_y_min - border_size);
    alignment_window.y_max = min(height, window_y_max + border_size);

    return alignment_window;
};

std::tuple<double,double> ReferencePhotoHandlerPlanetary::get_center_of_mass(   const MonochromeImageData &image_data,
                                                                                PixelType threshold,
                                                                                const AlignmentWindow &window_coordinates) const    {

    const PixelType *brightness = image_data.brightness;
    const int width = image_data.width;

    double center_of_mass_x = 0;
    double center_of_mass_y = 0;
    double sum_weights = 0;
    for (int y = window_coordinates.y_min; y < window_coordinates.y_max; y++) {
        for (int x = window_coordinates.x_min; x < window_coordinates.x_max; x++) {
            const float weight = brightness[y*width + x];
            if (weight < threshold) {
                continue;
            }
            center_of_mass_x += x * weight;
            center_of_mass_y += y * weight;
            sum_weights += weight;
        }
    }
    center_of_mass_x /= sum_weights;
    center_of_mass_y /= sum_weights;
    return make_tuple(center_of_mass_x, center_of_mass_y);
};

std::vector<std::vector<double>> ReferencePhotoHandlerPlanetary::get_covariance_matrix( const MonochromeImageData &image_data,
                                                                                        const tuple<double,double> &center_of_mass,
                                                                                        PixelType threshold,
                                                                                        const AlignmentWindow &window_coordinates) const  {

    const PixelType *brightness = image_data.brightness;
    const int width = image_data.width;

    double x2 = 0;
    double y2 = 0;
    double xy = 0;
    double sum_weights = 0;
    for (int y = window_coordinates.y_min; y < window_coordinates.y_max; y++) {
        for (int x = window_coordinates.x_min; x < window_coordinates.x_max; x++) {
            const float weight = brightness[y*width + x];
            if (weight < threshold) {
                continue;
            }
            const float x_centered = x - get<0>(center_of_mass);
            const float y_centered = y - get<1>(center_of_mass);

            x2 += x_centered * x_centered * weight;
            y2 += y_centered * y_centered * weight;
            xy += x_centered * y_centered * weight;

            sum_weights += weight;
        }
    }

    x2 /= sum_weights;
    y2 /= sum_weights;
    xy /= sum_weights;

    return {{x2, xy}, {xy, y2}};
};


std::tuple<float,float,vector<vector<double>>,vector<double>> ReferencePhotoHandlerPlanetary::get_center_of_mass_eigenvectors_and_eigenvalues(const MonochromeImageData &image_data, AlignmentWindow *window_coordinates) const    {
    const PixelType *brightness = image_data.brightness;
    const int width = image_data.width;
    const int height = image_data.height;

    vector<unsigned short> brightness_copy(width*height);
    for (int i = 0; i < width*height; i++) {
        brightness_copy[i] = max<short>(brightness[i], 0);
    }

    const PixelType max_value = *max_element(brightness, brightness + width*height);
    const PixelType otsu_threshold = get_otsu_threshold(brightness_copy.data(), width*height);
    const PixelType threshold = max<PixelType>(0.05*max_value, otsu_threshold);

    const AlignmentWindow alignment_window = get_alignment_window(image_data, threshold);
    if (window_coordinates != nullptr) {
        *window_coordinates = alignment_window;
    }
    const tuple<double,double> center_of_mass = get_center_of_mass(image_data, threshold, alignment_window);

    const float center_of_mass_x = get<0>(center_of_mass);
    const float center_of_mass_y = get<1>(center_of_mass);

    if (m_zero_rotation) {
        return make_tuple(center_of_mass_x, center_of_mass_y, vector<vector<double>>{{1,0},{0,1}}, vector<double>{1,1});
    }

    const vector<vector<double>> covariance_matrix = get_covariance_matrix(image_data, center_of_mass, threshold, alignment_window);

    vector<double> eigenvalues;
    vector<vector<double>> eigenvectors;
    calculate_eigenvectors_and_eigenvalues(covariance_matrix, &eigenvalues, &eigenvectors);


    return make_tuple(center_of_mass_x, center_of_mass_y, eigenvectors, eigenvalues);
};


void  ReferencePhotoHandlerPlanetary::initialize(const PixelType *brightness, int width, int height, const ConfigurableAlgorithmSettingsMap &configuration_map)   {
    m_configurable_algorithm_settings.set_values_from_configuration_map(configuration_map);

    m_width = width;
    m_height = height;

    MonochromeImageData image_data;
    image_data.brightness = brightness;
    image_data.width = width;
    image_data.height = height;

    std::tuple<float,float,vector<vector<double>>,vector<double>> center_of_mass_and_eigenvec_vals = get_center_of_mass_eigenvectors_and_eigenvalues(image_data, &m_alignment_window);
    m_center_of_mass_x = get<0>(center_of_mass_and_eigenvec_vals);
    m_center_of_mass_y = get<1>(center_of_mass_and_eigenvec_vals);
    m_covariance_eigen_vectors = get<2>(center_of_mass_and_eigenvec_vals);
    m_covariance_eigen_values = get<3>(center_of_mass_and_eigenvec_vals);

};


void ReferencePhotoHandlerPlanetary::calculate_eigenvectors_and_eigenvalues(
                                            const std::vector<std::vector<double>> &covariance_matrix,
                                            std::vector<double> *eigenvalues,
                                            std::vector<std::vector<double>> *eigenvectors) {

    if (covariance_matrix.size() != 2) {
        throw runtime_error("Covariance matrix should be 2x2");
    }
    if (covariance_matrix[0].size() != 2 || covariance_matrix[1].size() != 2) {
        throw runtime_error("Covariance matrix should be 2x2");
    }

    const double a = 1;
    const double b = -covariance_matrix[0][0] - covariance_matrix[1][1];
    const double c = covariance_matrix[0][0] * covariance_matrix[1][1] - covariance_matrix[0][1] * covariance_matrix[1][0];

    const double delta = b*b - 4*a*c;

    if (delta < 0) {
        throw runtime_error("Delta is negative");
    }

    const double lambda1 = (-b + sqrt(delta))/(2*a);
    const double lambda2 = (-b - sqrt(delta))/(2*a);

    eigenvalues->clear();
    eigenvalues->push_back(lambda1);
    eigenvalues->push_back(lambda2);

    eigenvectors->clear();
    eigenvectors->push_back({covariance_matrix[0][1], covariance_matrix[0][0] - lambda1});
    eigenvectors->push_back({covariance_matrix[0][1], covariance_matrix[0][0] - lambda2});

    for (auto &eigenvector : *eigenvectors) {
        const double length = sqrt(eigenvector[0]*eigenvector[0] + eigenvector[1]*eigenvector[1]);
        eigenvector[0] /= length;
        eigenvector[1] /= length;

        if (eigenvector[0] < 0 && eigenvector[1] < 0) {
            eigenvector[0] = -eigenvector[0];
            eigenvector[1] = -eigenvector[1];
        }
    }

    if (eigenvalues->at(0) < eigenvalues->at(1)) {
        swap(eigenvalues->at(0), eigenvalues->at(1));
        swap(eigenvectors->at(0), eigenvectors->at(1));
    }
};

void ReferencePhotoHandlerPlanetary::define_configuration_settings()    {
    m_configurable_algorithm_settings.add_additional_setting_numerical("gaussian sigma for denoising", &m_gaussian_sigma, 0.1, 15.0, 0.2);
    m_configurable_algorithm_settings.add_additional_setting_bool("use number of pixels above otsu threshold for ranking", &m_use_number_of_pixels_above_otsu_threshold_for_ranking);
    m_configurable_algorithm_settings.add_additional_setting_bool("Drop rotation (use translation only)", &m_zero_rotation);
}