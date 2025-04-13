#include "../headers/StarFinder.h"
#include "../headers/ReferencePhotoHandlerPlanetary.h"
#include "../headers/raw_file_reader.h"
#include "../headers/ImageFilesInputOutput.h"
#include "../headers/StarFinder.h"
#include "../headers/SharpnessRanker.h"

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <cmath>
#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;

ReferencePhotoHandlerPlanetary::ReferencePhotoHandlerPlanetary(const InputFrame &input_frame, float threshold_fraction)   :
    ReferencePhotoHandlerBase(input_frame, threshold_fraction) {

    m_threshold_fraction = threshold_fraction;
    const vector<unsigned short> brightness = read_image_monochrome<unsigned short>(input_frame, &m_width, &m_height);
    initialize(brightness.data(), m_width, m_height, threshold_fraction);
};

ReferencePhotoHandlerPlanetary::ReferencePhotoHandlerPlanetary(const unsigned short *brightness, int width, int height, float threshold_fraction)  :
    ReferencePhotoHandlerBase(brightness, width, height, threshold_fraction) {
    m_threshold_fraction = threshold_fraction;
    initialize(brightness, width, height, threshold_fraction);
};

PlateSolvingResult ReferencePhotoHandlerPlanetary::calculate_alignment(const InputFrame &input_frame, float *ranking) const{
    int width, height;
    const vector<unsigned short int> brightness = read_image_monochrome<unsigned short>(input_frame, &width, &height);

    MonochromeImageData image_data;
    image_data.brightness = brightness.data();
    image_data.width = width;
    image_data.height = height;

    AlignmentWindow alignment_window;
    const auto [center_of_mass_x, center_of_mass_y, eigenvec, eigenval] = get_center_of_mass_eigenvectors_and_eigenvalues(image_data, m_threshold_fraction, &alignment_window);
    const double sin_angle = m_covariance_eigen_vectors[0][0]*eigenvec[0][1] - m_covariance_eigen_vectors[0][1]*eigenvec[0][0];

    PlateSolvingResult plate_solving_result;
    plate_solving_result.shift_x = m_center_of_mass_x - center_of_mass_x;
    plate_solving_result.shift_y = m_center_of_mass_y - center_of_mass_y;
    plate_solving_result.rotation_center_x = center_of_mass_x;
    plate_solving_result.rotation_center_y = center_of_mass_y;
    plate_solving_result.rotation = std::asin(sin_angle);
    plate_solving_result.is_valid = true;

    if (ranking != nullptr) {
        const double sharpness = get_sharpness_factor(brightness.data(), width, height, alignment_window);
        *ranking = 100./sharpness;
    }

    return plate_solving_result;
};

AlignmentWindow ReferencePhotoHandlerPlanetary::get_alignment_window(   const MonochromeImageData &image_data,
                                                                        unsigned short threshold) const    {

    const unsigned short *brightness = image_data.brightness;
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
                                                                                unsigned short threshold,
                                                                                const AlignmentWindow &window_coordinates) const    {

    const unsigned short *brightness = image_data.brightness;
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
                                                                                        unsigned short threshold,
                                                                                        const AlignmentWindow &window_coordinates) const  {

    const unsigned short *brightness = image_data.brightness;
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


std::tuple<float,float,vector<vector<double>>,vector<double>> ReferencePhotoHandlerPlanetary::get_center_of_mass_eigenvectors_and_eigenvalues(const MonochromeImageData &image_data, float threshold_fraction, AlignmentWindow *window_coordinates) const    {
    const unsigned short *brightness = image_data.brightness;
    const int width = image_data.width;
    const int height = image_data.height;

    const unsigned short max_value = *max_element(brightness, brightness + width*height);
    const unsigned short min_value = *min_element(brightness, brightness + width*height);

    const unsigned short threshold = min_value + threshold_fraction * (max_value - min_value);

    const AlignmentWindow alignment_window = get_alignment_window(image_data, threshold);
    if (window_coordinates != nullptr) {
        *window_coordinates = alignment_window;
    }
    const tuple<double,double> center_of_mass = get_center_of_mass(image_data, threshold, alignment_window);

    const float center_of_mass_x = get<0>(center_of_mass);
    const float center_of_mass_y = get<1>(center_of_mass);

    const vector<vector<double>> covariance_matrix = get_covariance_matrix(image_data, center_of_mass, threshold, alignment_window);

    vector<double> eigenvalues;
    vector<vector<double>> eigenvectors;
    calculate_eigenvectors_and_eigenvalues(covariance_matrix, &eigenvalues, &eigenvectors);


    return make_tuple(center_of_mass_x, center_of_mass_y, eigenvectors, eigenvalues);
};


void  ReferencePhotoHandlerPlanetary::initialize(const unsigned short *brightness, int width, int height, float threshold_fraction)   {
    m_width = width;
    m_height = height;

    MonochromeImageData image_data;
    image_data.brightness = brightness;
    image_data.width = width;
    image_data.height = height;

    std::tuple<float,float,vector<vector<double>>,vector<double>> center_of_mass_and_eigenvec_vals = get_center_of_mass_eigenvectors_and_eigenvalues(image_data, threshold_fraction, &m_alignment_window);
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
