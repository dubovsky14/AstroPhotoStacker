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

    vector<float> values_for_gaussian_fit;
    for (int i = 0; i < n_neighbors_for_gaussian_fit && i < median_values_by_distance.size(); i++) {
        values_for_gaussian_fit.push_back(median_values_by_distance.at(i));
    }
    vector<float> values_for_background;
    for (int i = n_neighbors_for_gaussian_fit; i < n_nearest && i < median_values_by_distance.size(); i++) {
        values_for_background.push_back(median_values_by_distance.at(i));
    }
    std::sort(values_for_background.begin(), values_for_background.end());
    float background_median = 0.0f;
    if (!values_for_background.empty()) {
        size_t mid = values_for_background.size() / 2;
        background_median = (values_for_background.size() % 2 == 0) ? (values_for_background[mid - 1] + values_for_background[mid]) / 2.0f : values_for_background[mid];
    }


    unsigned int n_terms_to_use = 0;
    for (unsigned int i = 1; i < values_for_gaussian_fit.size(); i++) {
        if (values_for_gaussian_fit.at(i) > 0.0f && values_for_gaussian_fit.at(i) < values_for_gaussian_fit.at(i - 1)) {
            n_terms_to_use++;
        } else {
            break;
        }
    }
    values_for_gaussian_fit.resize(n_terms_to_use);


    // Subtract background median from values for Gaussian fit
    for (float &value : values_for_gaussian_fit) {
        value -= background_median;
    }

    cout << "Values to fit and ratio to previous: \n";
    for (size_t i = 0; i < values_for_gaussian_fit.size(); i++) {
        cout << values_for_gaussian_fit.at(i);
        if (i > 0) {
            cout << " (" << values_for_gaussian_fit.at(i) / values_for_gaussian_fit.at(i - 1) << ")";
        }
        cout << "\n";
    }

    // Now values_for_gaussian_fit contains the background-subtracted values ready for Gaussian fitting
    vector<double> initial_params = {0.0, 3.0}; // mean, sigma
    const vector<pair<double, double>> limits = {{-2 * cluser_radius, 2.0}, {0.1, 20.0}}; // mean, sigma


    auto get_gaussian_loss = [values_for_gaussian_fit](const double *params) -> double {
        const double mean = params[0];
        const double sigma = params[1];

        double sum_values = 0.0;
        for (const float &value : values_for_gaussian_fit) {
            sum_values += static_cast<double>(value);
        }

        vector<double> model_probabilities;
        double sum_model_probabilities = 0.0;
        for (size_t i = 0; i < values_for_gaussian_fit.size(); i++) {
            const double x = static_cast<double>(i) - mean;
            const double y = std::exp(-x * x / (2.0f * sigma * sigma));
            model_probabilities.push_back(y);
            sum_model_probabilities += y;
        }

        // Normalize model probabilities
        for (double &prob : model_probabilities) {
            prob *= sum_values /sum_model_probabilities;
        }

        double loss = 0.0f;
        for (size_t i = 0; i < values_for_gaussian_fit.size(); i++) {
            const double value = values_for_gaussian_fit.at(i);
            const double y = model_probabilities.at(i);
            const double diff = value - y;
            loss += diff * diff;
        }
        return loss;
    };

    Fitter fitter(&initial_params, limits);
    //fitter.set_debug(true);
    fitter.fit_gradient(get_gaussian_loss, 0.5, 0.9998, 10000);

    cout << "Cluster size: " << largest_cluster_pixels.size() << "\n";
    cout << "Fitted parameters (mean, sigma): " << initial_params.at(0) << ", " << initial_params.at(1) << "\n";
    cout << "Limits: ";
    for (const auto &limit : limits) {
        cout << "(" << limit.first << ", " << limit.second << ") ";
    }
    cout << "\n";

    return initial_params.at(1);
};