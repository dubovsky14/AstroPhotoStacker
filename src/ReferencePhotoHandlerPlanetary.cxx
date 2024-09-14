#include "../headers/StarFinder.h"
#include "../headers/ReferencePhotoHandlerPlanetary.h"
#include "../headers/raw_file_reader.h"
#include "../headers/ImageFilesInputOutput.h"
#include "../headers/StarFinder.h"

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <cmath>

using namespace AstroPhotoStacker;
using namespace std;

ReferencePhotoHandlerPlanetary::ReferencePhotoHandlerPlanetary(const std::string &raw_file_address, float threshold_fraction)   :
    ReferencePhotoHandlerBase(raw_file_address, threshold_fraction) {
    const bool raw_file = is_raw_file(raw_file_address);
    if (raw_file) {
        const vector<unsigned short int> brightness = read_raw_file<unsigned short int>(raw_file_address, &m_width, &m_height);
        initialize(brightness.data(), m_width, m_height, threshold_fraction);
    }
    else {
        const vector<unsigned short int> brightness = read_rgb_image_as_gray_scale<unsigned short int>(raw_file_address, &m_width, &m_height);
        initialize(brightness.data(), m_width, m_height, threshold_fraction);
    }
};

ReferencePhotoHandlerPlanetary::ReferencePhotoHandlerPlanetary(const unsigned short *brightness, int width, int height, float threshold_fraction)  :
    ReferencePhotoHandlerBase(brightness, width, height, threshold_fraction) {
    initialize(brightness, width, height, threshold_fraction);
};


bool ReferencePhotoHandlerPlanetary::calculate_alignment(const std::string &file_address, float *shift_x, float *shift_y, float *rot_center_x, float *rot_center_y, float *rotation) const{

    return false;
};

std::tuple<int,int,int,int> ReferencePhotoHandlerPlanetary::get_alignment_window(   const unsigned short *brightness,
                                                                                    unsigned short threshold) const    {

    const std::vector<std::tuple<float, float, int> > clusters = get_stars(&brightness[0], m_width, m_height, threshold);

    if (clusters.size() == 0) {
        throw runtime_error("No clusters found in the reference photo");
    }

    const float leading_cluster_radius = sqrt(get<2>(clusters[0]))/3.14;
    const float leading_cluster_x = get<0>(clusters[0]);
    const float leading_cluster_y = get<1>(clusters[0]);

    const float window_scale = 5;

    const int window_half_size = window_scale * leading_cluster_radius;

    const int window_x_min = max<int>(0, leading_cluster_x - window_half_size);
    const int window_x_max = min<int>(m_width, leading_cluster_x + window_half_size);
    const int window_y_min = max<int>(0, leading_cluster_y - window_half_size);
    const int window_y_max = min<int>(m_height, leading_cluster_y + window_half_size);

    return make_tuple(window_x_min, window_y_min, window_x_max, window_y_max);
};

std::tuple<double,double> ReferencePhotoHandlerPlanetary::get_center_of_mass(   const unsigned short *brightness,
                                                                                unsigned short threshold,
                                                                                const std::tuple<int,int,int,int> &window_coordinates) const    {
    const auto [window_x_min, window_y_min, window_x_max, window_y_max] = window_coordinates;
    double center_of_mass_x = 0;
    double center_of_mass_y = 0;
    double sum_weights = 0;
    for (int y = window_y_min; y < window_y_max; y++) {
        for (int x = window_x_min; x < window_x_max; x++) {
            const float weight = brightness[y*m_width + x];
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

std::vector<std::vector<double>> ReferencePhotoHandlerPlanetary::get_covariance_matrix( const unsigned short *brightness,
                                                                                        unsigned short threshold,
                                                                                        const std::tuple<int,int,int,int> &window_coordinates) const  {

    double x2 = 0;
    double y2 = 0;
    double xy = 0;
    double sum_weights = 0;
    const auto [window_x_min, window_y_min, window_x_max, window_y_max] = window_coordinates;
    for (int y = window_y_min; y < window_y_max; y++) {
        for (int x = window_x_min; x < window_x_max; x++) {
            const float weight = brightness[y*m_width + x];
            if (weight < threshold) {
                continue;
            }
            const float x_centered = x - m_center_of_mass_x;
            const float y_centered = y - m_center_of_mass_y;

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

void  ReferencePhotoHandlerPlanetary::initialize(const unsigned short *brightness, int width, int height, float threshold_fraction)   {
    m_width = width;
    m_height = height;
    const unsigned short max_value = *max_element(brightness, brightness + width*height);
    const unsigned short min_value = *min_element(brightness, brightness + width*height);

    const unsigned short threshold = min_value + threshold_fraction * (max_value - min_value);


    const tuple<int,int,int,int> alignment_window = get_alignment_window(brightness, threshold);
    const tuple<double,double> center_of_mass = get_center_of_mass(brightness, threshold, alignment_window);

    m_center_of_mass_x = get<0>(center_of_mass);
    m_center_of_mass_y = get<1>(center_of_mass);

    const vector<vector<double>> covariance_matrix = get_covariance_matrix(brightness, threshold, alignment_window);

    m_rotation_angle = atan(2*covariance_matrix[0][1]/(covariance_matrix[0][0] - covariance_matrix[1][1]))/2;
};

