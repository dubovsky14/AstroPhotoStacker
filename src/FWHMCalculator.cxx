#include "../headers/FWHMCalculator.h"
#include "../headers/StarFinder.h"
#include "../headers/Fitter.h"

#include <algorithm>
#include <cmath>

using namespace std;
using namespace AstroPhotoStacker;



FWHMCalculator::FWHMCalculator(const std::vector<PixelType> &brightness, int width, int height) {
    m_width = width;
    m_height = height;
    m_brightness = &brightness;

    m_star_threshold = get_threshold_value(brightness.data(), width * height, 0.0005);
    m_background_threshold = get_threshold_value(brightness.data(), width * height, 0.7);
    m_clusters = get_stars(brightness.data(), width, height, m_star_threshold);

    // drop clusters smaller than a certain size
    m_clusters.erase(
        std::remove_if(m_clusters.begin(), m_clusters.end(), [](const tuple<float,float,int> &cluster) {return get<2>(cluster) < 3;}),
        m_clusters.end()
    );

    std::sort(m_clusters.begin(), m_clusters.end(), [](const tuple<float,float,int> &a, const tuple<float,float,int> &b) {
        return get<2>(a) > get<2>(b);
    });

};

FWHMCalculator::FWHMCalculator(const std::vector<PixelType> &brightness, int width, int height, std::vector<std::tuple<float, float, int>> &clusters, PixelType threshold) {
    m_width = width;
    m_height = height;
    m_brightness = &brightness;
    m_clusters = clusters;
    m_star_threshold = threshold;
    m_background_threshold = get_threshold_value(brightness.data(), width * height, 0.7);
};



float FWHMCalculator::calculate_fwhm(int stars_to_use) const {
    vector<float> fwhm_values;

    // to get consistent results for CI tests
    FWHMCalculatorPRNG prng(static_cast<unsigned int>(m_clusters.size()) ^ (static_cast<unsigned int>(m_star_threshold) << 16) ^ (static_cast<unsigned int>(m_background_threshold)));
    for (int i_cluster_selection = 0; i_cluster_selection < stars_to_use; i_cluster_selection++) {
        const size_t random_index = prng.rand() % m_clusters.size();
        const tuple<float,float,int> &cluster = m_clusters.at(random_index);

        const std::array<int, 2> star_position = {static_cast<int>(get<0>(cluster)), static_cast<int>(get<1>(cluster))};
        const float fwhm_along_x = calculate_fwhm_for_star(star_position, std::array<int, 2>{1, 0});
        const float fwhm_along_y = calculate_fwhm_for_star(star_position, std::array<int, 2>{0, 1});
        const float fwhm_usual_orientation = (fwhm_along_x > 0 && fwhm_along_y > 0) ? (fwhm_along_x + fwhm_along_y) / 2.0 : -1.0f;


        const float fwhm_along_xy = calculate_fwhm_for_star(star_position, std::array<int, 2>{1, 1});
        const float fwhm_along_yx = calculate_fwhm_for_star(star_position, std::array<int, 2>{-1, 1});
        const float fwhm_diagonal_orientation = (fwhm_along_xy > 0 && fwhm_along_yx > 0) ? (fwhm_along_xy + fwhm_along_yx) / 2.0 : -1.0f;

        if (fwhm_usual_orientation > 0 && fwhm_diagonal_orientation > 0) {
            fwhm_values.push_back(std::min(fwhm_usual_orientation, fwhm_diagonal_orientation));
        } else if (fwhm_usual_orientation > 0) {
            fwhm_values.push_back(fwhm_usual_orientation);
        } else if (fwhm_diagonal_orientation > 0) {
            fwhm_values.push_back(fwhm_diagonal_orientation);
        }
    }

    if (fwhm_values.empty()) {
        return -1;
    }

    std::sort(fwhm_values.begin(), fwhm_values.end());
    const size_t median_index = fwhm_values.size() / 2;
    return fwhm_values.at(median_index);

};


std::array<double, 3> FWHMCalculator::fit_by_1d_gaussian(   const std::vector<double> &data_x,
                                                            const std::vector<double> &data_y,
                                                            const std::array<double, 3> &initial_guess,
                                                            double learning_rate,
                                                            double beta,
                                                            int max_iterations) {

    if (data_x.size() < 3) {
        throw std::invalid_argument("Insufficient data points for fitting.");
    }

    std::array<double, 3> fitted_parameters = initial_guess;
    std::array<double, 3> accumulated_gradients  = {0.0, 0.0, 0.0};

    const double beta_grad2 = 0.6;
    const double epsilon = 1e-8;
    std::array<double, 3> accumulated_gradients2 = {0.0, 0.0, 0.0};

    const double max_y = *std::max_element(data_y.begin(), data_y.end());

    for (int iteration = 0; iteration < max_iterations; iteration++) {
        std::array<double, 3> gradients = {0.0, 0.0, 0.0};
        double loss = 0.0;

        for (size_t i_point = 0; i_point < data_x.size(); i_point++) {
            double x = data_x.at(i_point);
            double y = data_y.at(i_point);
            double A = fitted_parameters.at(0) * max_y;
            double mu = fitted_parameters.at(1);
            double sigma = fitted_parameters.at(2);

            double exponential_term = std::exp(-((x - mu) * (x - mu)) / (2 * sigma * sigma));
            double fx = A * exponential_term;
            double dLdf = 2 * (fx - y);
            loss += (fx - y) * (fx - y);

            gradients.at(0) += dLdf * max_y * exponential_term;
            gradients.at(1) += dLdf * A * exponential_term * (x - mu) / (sigma * sigma);
            gradients.at(2) += dLdf * A * exponential_term * (x - mu) * (x - mu) / (sigma * sigma * sigma);
        }

        for (size_t i_param = 0; i_param < fitted_parameters.size(); i_param++) {
            accumulated_gradients2.at(i_param) = beta_grad2 * accumulated_gradients2.at(i_param) + (1 - beta_grad2) * gradients.at(i_param) * gradients.at(i_param);
            accumulated_gradients.at(i_param) = beta * accumulated_gradients.at(i_param) + (1 - beta) * gradients.at(i_param);


            fitted_parameters.at(i_param) -= learning_rate * accumulated_gradients.at(i_param) / (std::sqrt(accumulated_gradients2.at(i_param)) + epsilon);
        }

    }

    return fitted_parameters;
};


float FWHMCalculator::calculate_fwhm_for_star(std::array<int, 2> star_position, std::array<int, 2> direction) const {
    vector<double> values_x; // signed distance from cluster center
    vector<double> values_y; // intensity values along the direction

    const double step_size = sqrt(direction[0] * direction[0] + direction[1] * direction[1]);
    // Collect pixel values along the specified direction
    for (int i_direction = 0; i_direction < 2; i_direction++) {
        int n_used_pixels = 0;
        for (int i = i_direction; i <= 200; i++) {
            int x = star_position[0] + i * direction[0];
            int y = star_position[1] + i * direction[1];

            if (n_used_pixels >= 20) break;

            if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
                PixelType brighness = m_brightness->at(y * m_width + x);
                if (brighness <= m_background_threshold) break;
                values_x.push_back(i * step_size);
                values_y.push_back(static_cast<double>(brighness) - m_background_threshold);
                n_used_pixels++;
            }
            else {
                break;
            }
        }
        direction[0] = -direction[0];
        direction[1] = -direction[1];
    }

    if (values_x.size() < 4) {
        return -1.0f; // Not enough data points to calculate FWHM
    }

    // Fit a 1D Gaussian to the collected values
    std::array<double, 3> initial_guess = {1.0, 0.0, 1.0};
    std::array<double, 3> fitted_params = fit_by_1d_gaussian(values_x, values_y, initial_guess, 0.1, 0.9, 1000);

    // Calculate FWHM from the fitted sigma
    double sigma = fitted_params[2];
    return static_cast<float>(2.3548 * sigma);

};
