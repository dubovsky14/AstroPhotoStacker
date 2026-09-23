#include "../headers/FWHMCalculator.h"
#include "../headers/StarFinder.h"
#include "../headers/Fitter.h"

#include <algorithm>
#include <cmath>

using namespace std;
using namespace AstroPhotoStacker;


std::vector<char> AstroPhotoStacker::FWHMCalculator::get_distance_from_cluster_mask(const std::vector<std::pair<int, int>> &pixels_in_cluster, int width, int height, char max_distance)  {
    vector<char> result(width*height, -1);
    for (const pair<int, int> &pixel : pixels_in_cluster) {
        int x = pixel.first;
        int y = pixel.second;
        int idx = y * width + x;
        if (idx >= 0 && idx < width*height) {
            result.at(idx) = 0;
        }
    }

    for (int i_current_distance = 1; i_current_distance <= max_distance; i_current_distance++) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = y * width + x;
                if (result.at(idx) == i_current_distance - 1) {
                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            int nx = x + dx;
                            int ny = y + dy;
                            int nidx = ny * width + nx;
                            if (nx >= 0 && nx < width && ny >= 0 && ny < height && result.at(nidx) == -1) {
                                result.at(nidx) = i_current_distance;
                            }
                        }
                    }
                }
            }
        }
    }

    return result;
};

std::vector<std::vector<PixelType>> AstroPhotoStacker::FWHMCalculator::get_pixel_values_in_given_distance_from_cluster(const std::vector<PixelType> &brightness, const std::vector<char> &distance_mask) {
    std::vector<std::vector<PixelType>> result;

    for (size_t i = 0; i < distance_mask.size(); i++) {
        char d = distance_mask.at(i);
        if (d < 0) {
            continue;
        }
        if (d >= int(result.size())) {
            result.resize(d + 1);
        }
        result.at(d).push_back(brightness.at(i));
    }
    return result;
};

std::vector<std::vector<PixelType>> AstroPhotoStacker::FWHMCalculator::get_pixel_values_in_given_distance_from_cluster( const std::vector<PixelType> &brightness,
                                                                                                                        int width,
                                                                                                                        int height,
                                                                                                                        const std::vector<std::pair<int, int>> &pixels_in_cluster)  {
    double center_x(0), center_y(0);
    for (const auto &p : pixels_in_cluster) {
        center_x += p.first;
        center_y += p.second;
    }
    if (!pixels_in_cluster.empty()) {
        center_x /= pixels_in_cluster.size();
        center_y /= pixels_in_cluster.size();
    }

    vector<float> distance_vector(width * height, 0.0f);
    for (float x = 0; x < width; x++) {
        for (float y = 0; y < height; y++) {
            const float dx = x - center_x;
            const float dy = y - center_y;
            const float distance = std::sqrt(dx * dx + dy * dy);
            const int idx = static_cast<int>(y) * width + static_cast<int>(x);
            distance_vector.at(idx) = distance;
        }
    }

    for (const pair<int, int> &p : pixels_in_cluster) {
        const int x = p.first;
        const int y = p.second;
        const int idx = y * width + x;
        distance_vector.at(idx) = 0;
    }

    vector<vector<PixelType>> result;

    for (int idx = 0; idx < width * height; idx++) {
        const int d = static_cast<int>(distance_vector.at(idx));
        if (d <= 0) {
            continue;
        }
        if (d >= int(result.size())) {
            result.resize(d + 1);
        }
        result.at(d).push_back(brightness.at(idx));
    }
    return result;
};

float AstroPhotoStacker::FWHMCalculator::calculate_fwhm(const std::vector<PixelType> &brightness, int width, int height, PixelType threshold) {
    vector<vector<tuple<int,int>>> clusters = get_clusters(brightness.data(), width, height, threshold);
    if (clusters.empty()) {
        return 0.0f;
    }

    std::sort(clusters.begin(), clusters.end(), [](const vector<tuple<int,int>> &a, const vector<tuple<int,int>> &b) {
        return a.size() > b.size();
    });

    vector<pair<int,int>> largest_cluster_pixels;
    for (const tuple<int,int> &t : clusters.front()) {
        largest_cluster_pixels.emplace_back(get<0>(t), get<1>(t));
    }
    const float cluser_radius = sqrt(static_cast<float>(largest_cluster_pixels.size()/M_PI));

    const int n_neighbors_for_gaussian_fit = 16;
    const int n_neighbors_for_background = 16;
    const int n_nearest = n_neighbors_for_gaussian_fit + n_neighbors_for_background + 1;
    const vector<char> distance_mask = get_distance_from_cluster_mask(largest_cluster_pixels, width, height, n_nearest);
    vector<vector<PixelType>> pixel_values_by_distance = get_pixel_values_in_given_distance_from_cluster(brightness, distance_mask);

    // drop 0th element as it corresponds to the cluster itself
    if (!pixel_values_by_distance.empty()) {
        pixel_values_by_distance.erase(pixel_values_by_distance.begin());
    }

    vector<float> median_values_by_distance;
    for (const auto &values : pixel_values_by_distance) {
        if (values.empty()) {
            median_values_by_distance.push_back(0.0f);
            continue;
        }
        std::vector<PixelType> sorted_values = values;
        std::sort(sorted_values.begin(), sorted_values.end());
        size_t mid = sorted_values.size() / 2;
        float median = (sorted_values.size() % 2 == 0) ? (sorted_values[mid - 1] + sorted_values[mid]) / 2.0f : sorted_values[mid];
        median_values_by_distance.push_back(median);
    }

    vector<double> values_for_gaussian_fit_y;
    for (int i = 0; i < n_neighbors_for_gaussian_fit && i < median_values_by_distance.size(); i++) {
        values_for_gaussian_fit_y.push_back(median_values_by_distance.at(i));
    }
    vector<double> values_for_background;
    for (int i = n_neighbors_for_gaussian_fit; i < n_nearest && i < median_values_by_distance.size(); i++) {
        values_for_background.push_back(median_values_by_distance.at(i));
    }
    std::sort(values_for_background.begin(), values_for_background.end());
    double background_median = 0.0;
    if (!values_for_background.empty()) {
        size_t mid = values_for_background.size() / 2;
        background_median = (values_for_background.size() % 2 == 0) ? (values_for_background[mid - 1] + values_for_background[mid]) / 2.0f : values_for_background[mid];
    }


    unsigned int n_terms_to_use = 0;
    for (unsigned int i = 1; i < values_for_gaussian_fit_y.size(); i++) {
        if (values_for_gaussian_fit_y.at(i) > 0.0f && values_for_gaussian_fit_y.at(i) < values_for_gaussian_fit_y.at(i - 1)) {
            n_terms_to_use++;
        } else {
            break;
        }
    }
    values_for_gaussian_fit_y.resize(n_terms_to_use);


    // Subtract background median from values for Gaussian fit
    for (double &value : values_for_gaussian_fit_y) {
        value -= background_median;
    }

    cout << "Values to fit and ratio to previous: \n";
    for (size_t i = 0; i < values_for_gaussian_fit_y.size(); i++) {
        cout << values_for_gaussian_fit_y.at(i);
        if (i > 0) {
            cout << " (" << values_for_gaussian_fit_y.at(i) / values_for_gaussian_fit_y.at(i - 1) << ")";
        }
        cout << "\n";
    }
    cout << "Cluster radius: " << cluser_radius << "\n";

    vector<double> values_for_gaussian_fit_x;
    for (size_t i = 0; i < values_for_gaussian_fit_y.size(); i++) {
        values_for_gaussian_fit_x.push_back(static_cast<double>(i));
    }

    vector<double> initial_guess = {values_for_gaussian_fit_y.at(0), -cluser_radius, 3};
    vector<double> fitted_parameters = AstroPhotoStacker::FWHMCalculator::fit_by_1d_gaussian(
        values_for_gaussian_fit_x,
        values_for_gaussian_fit_y,
        initial_guess,
        0.000000001,  // learning_rate
        0.99,  // decay_rate
        0.,   // beta
        2000   // max_iterations
    );
    return fitted_parameters.at(2);
}



std::vector<double> AstroPhotoStacker::FWHMCalculator::fit_by_1d_gaussian(  const std::vector<double> &data_x,
                                                                            const std::vector<double> &data_y,
                                                                            const std::vector<double> &initial_guess,
                                                                            double learning_rate,
                                                                            double decay_rate,
                                                                            double beta,
                                                                            int max_iterations) {


    std::vector<double> fitted_parameters = initial_guess;
    std::vector<double> accumulated_gradients(initial_guess.size(), 0.0);

    for (int iteration = 0; iteration < max_iterations; iteration++) {
        std::vector<double> gradients(initial_guess.size(), 0.0);
        double loss = 0.0;

        for (size_t i_point = 0; i_point < data_x.size(); i_point++) {
            double x = data_x.at(i_point);
            double y = data_y.at(i_point);
            double A = fitted_parameters.at(0);
            double mu = fitted_parameters.at(1);
            double sigma = fitted_parameters.at(2);

            double exponential_term = std::exp(-((x - mu) * (x - mu)) / (2 * sigma * sigma));
            double fx = A * exponential_term;
            double dLdf = 2 * (fx - y);
            loss += (fx - y) * (fx - y);

            gradients.at(0) += dLdf * exponential_term;
            gradients.at(1) += dLdf * A * exponential_term * (x - mu) / (sigma * sigma);
            gradients.at(2) += dLdf * A * exponential_term * (x - mu) * (x - mu) / (sigma * sigma * sigma);
        }

        //cout << "Iteration " << iteration << ": ";
        //cout << "A = " << fitted_parameters.at(0) << ", ";
        //cout << "mu = " << fitted_parameters.at(1) << ", ";
        //cout << "sigma = " << fitted_parameters.at(2) << endl;
        //cout << "Gradients: ";
        //for (size_t i_param = 0; i_param < gradients.size(); i_param++) {
        //    cout << gradients.at(i_param) << " ";
        //}
        //cout << endl << "Loss: " << loss << endl << endl;

        for (size_t i_param = 0; i_param < fitted_parameters.size(); i_param++) {
            accumulated_gradients.at(i_param) = beta * accumulated_gradients.at(i_param) + (1 - beta) * gradients.at(i_param);
            fitted_parameters.at(i_param) -= learning_rate * accumulated_gradients.at(i_param);
        }
        learning_rate *= decay_rate;

    }

    return fitted_parameters;
}