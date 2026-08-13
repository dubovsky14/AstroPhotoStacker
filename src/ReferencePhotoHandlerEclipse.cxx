#include "../headers/StarFinder.h"
#include "../headers/ReferencePhotoHandlerEclipse.h"
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

ReferencePhotoHandlerEclipse::ReferencePhotoHandlerEclipse(const InputFrame &input_frame, const ConfigurableAlgorithmSettingsMap &configuration_map)   :
    ReferencePhotoHandlerBase(input_frame, configuration_map) {

    define_configuration_settings();
    const vector<PixelType> brightness = read_image_monochrome(input_frame, &m_width, &m_height);
    initialize(brightness.data(), m_width, m_height, configuration_map);
};

ReferencePhotoHandlerEclipse::ReferencePhotoHandlerEclipse(const PixelType *brightness, int width, int height, const ConfigurableAlgorithmSettingsMap &configuration_map)  :
    ReferencePhotoHandlerBase(brightness, width, height, configuration_map) {
    define_configuration_settings();
    initialize(brightness, width, height, configuration_map);
};

std::unique_ptr<AlignmentResultBase> ReferencePhotoHandlerEclipse::calculate_alignment(const InputFrame &input_frame) const{
    int width, height;
    const vector<PixelType> brightness = read_image_monochrome(input_frame, &width, &height);

    MonochromeImageData image_data;
    image_data.brightness = brightness.data();
    image_data.width = width;
    image_data.height = height;

    const auto [center_x, center_y, radius] = get_center_coordinates_and_radius(image_data);
    const float shift_x = m_center_x - center_x;
    const float shift_y = m_center_y - center_y;


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

    std::unique_ptr<AlignmentResultTranslationOnly> plate_solving_result = std::make_unique<AlignmentResultTranslationOnly>(shift_x, shift_y);
    plate_solving_result->set_ranking_score(sharpness_score);
    return plate_solving_result;
};


void  ReferencePhotoHandlerEclipse::initialize(const PixelType *brightness, int width, int height, const ConfigurableAlgorithmSettingsMap &configuration_map)   {
    m_configurable_algorithm_settings.set_values_from_configuration_map(configuration_map);

    m_width = width;
    m_height = height;

    MonochromeImageData image_data;
    image_data.brightness = brightness;
    image_data.width = width;
    image_data.height = height;

    std::tuple<float,float,float> center_and_radius = get_center_coordinates_and_radius(image_data);
    m_center_x = get<0>(center_and_radius);
    m_center_y = get<1>(center_and_radius);
    m_radius   = get<2>(center_and_radius);
};

void ReferencePhotoHandlerEclipse::define_configuration_settings()    {
    m_configurable_algorithm_settings.add_additional_setting_numerical("gaussian sigma for denoising", &m_gaussian_sigma, 0.1, 15.0, 0.2);
    m_configurable_algorithm_settings.add_additional_setting_bool("use number of pixels above otsu threshold for ranking", &m_use_number_of_pixels_above_otsu_threshold_for_ranking);
}


vector<unsigned char> ReferencePhotoHandlerEclipse::threshold_image(const MonochromeImageData &image_data, PixelType threshold)    {
    vector<unsigned char> result(image_data.width * image_data.height);
    for (int i = 0; i < image_data.width * image_data.height; ++i) {
        result[i] = image_data.brightness[i] > threshold ? 255 : 0;
    }
    return result;
}

tuple<float,float,float> ReferencePhotoHandlerEclipse::get_center_coordinates_and_radius(const MonochromeImageData &image_data) const   {
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

    vector<unsigned char> thresholded_image = threshold_image(image_data, threshold);
    std::vector< std::vector<std::tuple<int, int> > > clusters = get_clusters(thresholded_image.data(), width, height, 2);
    std::sort(clusters.begin(), clusters.end(), [](const std::vector<std::tuple<int,int>> &a, const std::vector<std::tuple<int,int>> &b) {
        return a.size() > b.size();
    });
    const unsigned int n_pixels_leading_cluster = clusters.empty() ? 0 : clusters[0].size();
    const unsigned int n_pixels_minimal = 0.1*n_pixels_leading_cluster;

    for (unsigned char &x : thresholded_image) {
        x = 0;
    }
    for (const auto &cluster : clusters) {
        if (cluster.size() < n_pixels_minimal) {
            break;
        }
        for (const auto &[x, y] : cluster) {
            thresholded_image[y*width + x] = 255;
        }
    }

    const vector<pair<int,int>> edge_pixels = get_edge_pixels(thresholded_image, width, height);

    std::vector<std::pair<int,int>> pixels_above_threshold;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (thresholded_image[y*width + x] > 0) {
                pixels_above_threshold.emplace_back(x, y);
            }
        }
    }

    const int n_iterations = 100;
    double best_center_x = 0;
    double best_center_y = 0;
    double best_radius = 0;
    int    best_pixels_in_circle = 0;

    array<pair<int,int>, 3> random_points;
    for (int i = 0; i < n_iterations; ++i) {
        for (int i_random_point = 0; i_random_point < 3; ++i_random_point) {
            const int random_index = rand() % edge_pixels.size();
            random_points[i_random_point] = edge_pixels[random_index];
        }

        // solve for the circle that passes through the three random points
        const auto &[x1, y1] = random_points[0];
        const auto &[x2, y2] = random_points[1];
        const auto &[x3, y3] = random_points[2];

        const double r1 = x1 * x1 + y1 * y1;
        const double r2 = x2 * x2 + y2 * y2;
        const double r3 = x3 * x3 + y3 * y3;

        if (x1 == x2) continue;

        const double center_x_denominator = (-2*(y1+y3) - 2*(x1-x3)*(y1-y2)/(x2-x1));

        if (std::abs(center_x_denominator) < 1e-6) {
            continue; // points are collinear, skip this iteration
        }

        const double center_y = ((x3+x1)*(r2-r1) / (x2-x1) - r3 -r1) / center_x_denominator;
        const double center_x = (r2 - r1 - 2*center_y*(y2-y1)) / (2*(x2-x1));
        const double C = r1 - 2*center_x*x1 - 2*center_y*y1;
        const double radius_squared = center_x*center_x + center_y*center_y - C;
        const double radius = sqrt(radius_squared);


//        const double a = x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2;
//        if (std::abs(a) < 1e-6) {
//            continue; // points are collinear, skip this iteration
//        }
//
//        const double b = (x1 * x1 + y1 * y1) * (y3 - y2) + (x2 * x2 + y2 * y2) * (y1 - y3) + (x3 * x3 + y3 * y3) * (y2 - y1);
//        const double c = (x1 * x1 + y1 * y1) * (x2 - x3) + (x2 * x2 + y2 * y2) * (x3 - x1) + (x3 * x3 + y3 * y3) * (x1 - x2);
//        const double d = (x1 *x1 + y1 * y1) * (x3 * y2 - x2 * y3) + (x2 * x2 + y2 * y2) * (x1 * y3 - x3 * y1) + (x3 * x3 + y3 * y3) * (x2 * y1 - x1 * y2);
//
//        const double center_x = -b / (2 * a);
//        const double center_y = -c / (2 * a);
//
//        const double radius = std::sqrt((center_x - x1) * (center_x - x1) + (center_y - y1) * (center_y - y1));
//        const double radius_squared = radius * radius;
//

        const double minimal_distance = sqrt(std::min(r1, std::min(r2, r3)));
        if (minimal_distance < 0.1*radius) {
            continue; // points are too close to each other, skip this iteration
        }


        int pixels_in_circle = 0;
        for (const auto &[x, y] : pixels_above_threshold) {
            const double distance2 = (x - center_x) * (x - center_x) + (y - center_y) * (y - center_y);
            if (distance2 - radius_squared < 1.0) {
                ++pixels_in_circle;
            }
        }

        cout << "Iteration " << i << ": center = (" << center_x << ", " << center_y << "), radius = " << radius  << " #edge points: " << edge_pixels.size() << "\tpoints in circle: " << pixels_in_circle << endl;
        cout << "Points: (" << x1 << ", " << y1 << "), (" << x2 << ", " << y2 << "), (" << x3 << ", " << y3 << ")" << endl << endl;


        if (pixels_in_circle > best_pixels_in_circle) {
            best_center_x = center_x;
            best_center_y = center_y;
            best_radius = radius;
            best_pixels_in_circle = pixels_in_circle;
        }
    }

    std::string debug_message = "Best circle: center = (" + std::to_string(best_center_x) + ", " + std::to_string(best_center_y) + "), radius = " + std::to_string(best_radius) + ", pixels in circle = " + std::to_string(best_pixels_in_circle);
    cout << debug_message << endl;

    return std::make_tuple(best_center_x, best_center_y, best_radius);
};

vector<pair<int,int>> ReferencePhotoHandlerEclipse::get_edge_pixels(const vector<unsigned char> &binary_image, int width, int height, int minimal_number_of_neighbors_outside) const {
    vector<pair<int,int>> edge_pixels;
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            if (binary_image[y * width + x] == 0) {
                continue;
            }

            int neighbors_outside = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) {
                        continue;
                    }
                    if (binary_image[(y + dy) * width + (x + dx)] == 0) {
                        ++neighbors_outside;
                    }
                }
            }
            if (neighbors_outside >= minimal_number_of_neighbors_outside) {
                edge_pixels.push_back(make_pair(x, y));
            }
        }
    }
    return edge_pixels;
}

/*
        const double a = x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2;
        if (std::abs(a) < 1e-6) {
            continue; // points are collinear, skip this iteration
        }

        const double b = (x1 * x1 + y1 * y1) * (y3 - y2) + (x2 * x2 + y2 * y2) * (y1 - y3) + (x3 * x3 + y3 * y3) * (y2 - y1);
        const double c = (x1 * x1 + y1 * y1) * (x2 - x3) + (x2 * x2 + y2 * y2) * (x3 - x1) + (x3 * x3 + y3 * y3) * (x1 - x2);
        const double d = (x1 *x1 + y1 * y1) * (x3 * y2 - x2 * y3) + (x2 * x2 + y2 * y2) * (x1 * y3 - x3 * y1) + (x3 * x3 + y3 * y3) * (x2 * y1 - x1 * y2);

        const double center_x = -b / (2 * a);
        const double center_y = -c / (2 * a);

        const double radius = std::sqrt((center_x - x1) * (center_x - x1) + (center_y - y1) * (center_y - y1));
        const double radius_squared = radius * radius;

*/