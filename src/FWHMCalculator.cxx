#include "../headers/FWHMCalculator.h"
#include "../headers/StarFinder.h"
#include "../headers/Fitter.h"

#include <algorithm>
#include <cmath>

using namespace std;
using namespace AstroPhotoStacker;
using namespace AstroPhotoStacker::FWHMCalculator;



float AstroPhotoStacker::FWHMCalculator::calculate_fwhm(const std::vector<PixelType> &brightness, int width, int height, PixelType threshold) {
    vector<vector<tuple<int,int>>> clusters = get_clusters(brightness.data(), width, height, threshold);
    if (clusters.empty()) {
        return 0.0f;
    }

    // drop clusters smaller than a certain size
    std::remove_if(clusters.begin(), clusters.end(), [](const vector<tuple<int,int>> &cluster) {return cluster.size() < 12;});

    std::sort(clusters.begin(), clusters.end(), [](const vector<tuple<int,int>> &a, const vector<tuple<int,int>> &b) {
        return a.size() > b.size();
    });


    // Brief description of the algorithm:
    // 1. Take a random cluster
    // 1. Find min and max values of x and y coordinates for the cluster
    // 2. Calculate mean background brightness around the cluster, subrtract if from data
    // 3. Take the lines x_min-1 and x_max+1 and get the pixel brightness values along these lines around center
    // 4. Apply piece-wise polynomial fit on these data around the peak, extract value of maximum and x-positions where brightness is half of the maximum
    // 5. Calculate the Full Width at Half Maximum (FWHM) along x-axis as their average
    // 6. Repeat the same for the y-axis
    // 7. Star's FWHM is obtained as the average of the FWHM along x and y axes
    // 8. Repeat for N clusters and get median - this is the overall FWHM of the image

    const int clusters_to_use = 50;
    const int edge_margin = 20;
    vector<float> fwhm_values;
    for (int i_cluster_selection = 0; i_cluster_selection < clusters_to_use; i_cluster_selection++) {
        const size_t random_index = rand() % clusters.size();
        vector<tuple<int,int>> cluster = clusters.at(random_index);

        std::array<int, 4> cluster_bounds = get_cluster_bounds(cluster); // {min_x, min_y, max_x, max_y}

        if ((cluster_bounds[0] - edge_margin) < 0) continue;
        if ((cluster_bounds[1] - edge_margin) < 0) continue;
        if ((cluster_bounds[2] + edge_margin) >= width) continue;
        if ((cluster_bounds[3] + edge_margin) >= height) continue;
        ImageView2D<PixelType> cluster_view(
            brightness.data(),
            width,
            height,
            cluster_bounds[0] - edge_margin,
            cluster_bounds[1] - edge_margin,
            (cluster_bounds[2] - cluster_bounds[0]) + 1 + 2 * edge_margin,
            (cluster_bounds[3] - cluster_bounds[1]) + 1 + 2 * edge_margin
        );

        const float background_median = get_background_median(cluster_view, edge_margin);
        const tuple<int,int> top_left = make_tuple(cluster_bounds[0] - edge_margin, cluster_bounds[1] - edge_margin);

        // recalculate the cluster coordinates relative to the top-left corner of the cluster view
        for (auto &point : cluster) {
            std::get<0>(point) -= std::get<0>(top_left);
            std::get<1>(point) -= std::get<1>(top_left);
        }



    }


};

std::array<int, 4> AstroPhotoStacker::FWHMCalculator::get_cluster_bounds(const std::vector<tuple<int,int>> &cluster)    {
    if (cluster.empty()) {
        return {0, 0, 0, 0};
    }
    int min_x = std::get<0>(cluster[0]);
    int min_y = std::get<1>(cluster[0]);
    int max_x = min_x;
    int max_y = min_y;

    for (const auto &point : cluster) {
        min_x = std::min(min_x, static_cast<int>(std::get<0>(point)));
        min_y = std::min(min_y, static_cast<int>(std::get<1>(point)));
        max_x = std::max(max_x, static_cast<int>(std::get<0>(point)));
        max_y = std::max(max_y, static_cast<int>(std::get<1>(point)));
    }

    return {min_x, min_y, max_x, max_y};
};


float AstroPhotoStacker::FWHMCalculator::get_background_median(const ImageView2D<PixelType> &image_view, int edge_margin) {
    const int view_width = image_view.get_view_width();
    const int view_height = image_view.get_view_height();
    const int n_pixels_to_use = edge_margin*view_width*2 + edge_margin*view_height*2 - edge_margin*edge_margin*4;

    PixelType edge_pixels[n_pixels_to_use];
    int current_index = 0;
    for (int y = 0; y < edge_margin; y++) {
        for (int x = 0; x < view_width; x++) {
            edge_pixels[current_index++] = image_view.at(x, y);
        }
    }

    for (int y = view_height - edge_margin; y < view_height; y++) {
        for (int x = 0; x < view_width; x++) {
            edge_pixels[current_index++] = image_view.at(x, y);
        }
    }

    for (int x = 0; x < edge_margin; x++) {
        for (int y = edge_margin; y < view_height - edge_margin; y++) {
            edge_pixels[current_index++] = image_view.at(x, y);
        }
    }

    for (int x = view_width - edge_margin; x < view_width; x++) {
        for (int y = edge_margin; y < view_height - edge_margin; y++) {
            edge_pixels[current_index++] = image_view.at(x, y);
        }
    }

    std::sort(edge_pixels, edge_pixels + n_pixels_to_use);
    if (n_pixels_to_use % 2 == 0) {
        return (edge_pixels[n_pixels_to_use / 2 - 1] + edge_pixels[n_pixels_to_use / 2]) / 2.0f;
    } else {
        return edge_pixels[n_pixels_to_use / 2];
    }
};


std::pair<std::vector<PixelType>, std::vector<PixelType>> AstroPhotoStacker::FWHMCalculator::get_line_above_and_below(
        const ImageView2D<PixelType> &image_view,
        const std::vector<tuple<int,int>> &cluster,
        int cluster_y_min,
        int cluster_y_max,
        float background_median,
        int edge_margin) {

    std::vector<PixelType> line_above;
    std::vector<PixelType> line_below;

    float first_line_center_x(0), last_line_center_x(0);
    int   first_line_n_pixels(0), last_line_n_pixels(0);

    for (const auto &pixel : cluster) {
        const int x = std::get<0>(pixel);
        const int y = std::get<1>(pixel);

        if (y == cluster_y_min) {
            first_line_center_x += x;
            first_line_n_pixels++;
        }
        if (y == cluster_y_max) {
            last_line_center_x += x;
            last_line_n_pixels++;
        }
    }
    if (first_line_n_pixels > 0) {
        first_line_center_x /= first_line_n_pixels;
    }
    if (last_line_n_pixels > 0) {
        last_line_center_x /= last_line_n_pixels;
    }

    const std::pair<int, int> line_above_range = {
        std::max<int>(first_line_center_x - edge_margin, 0),
        std::min<int>(first_line_center_x + edge_margin, image_view.get_view_width() - 1)
    };

    const std::pair<int, int> line_below_range = {
        std::max<int>(last_line_center_x - edge_margin, 0),
        std::min<int>(last_line_center_x + edge_margin, image_view.get_view_width() - 1)
    };


    if (cluster_y_min-1 > 0)    {
        for (int x = line_above_range.first; x < line_above_range.second; x++) {
            line_above.push_back(image_view.at(x, cluster_y_min-1));
        }
    }
    if (cluster_y_max + 1 < image_view.get_view_height()) {
        for (int x = line_below_range.first; x < line_below_range.second; x++) {
            line_below.push_back(image_view.at(x, cluster_y_max+1));
        }
    }

    return {line_above, line_below};
}

FWHMFitter::FWHMFitter(const std::vector<float> &x_data, const std::vector<float> &y_data) {
    m_x_data = x_data;
    m_y_data = y_data;

    for (unsigned int i_point = 0; i_point+1 < m_x_data.size(); i_point++) {
        float slope = (m_y_data[i_point+1] - m_y_data[i_point]) / (m_x_data[i_point+1] - m_x_data[i_point]);
        float offset = m_y_data[i_point] - slope * m_x_data[i_point];
        m_slopes_and_offsets.push_back({slope, offset});
    }
}


float FWHMFitter::operator()(float x, int segment_index) const {
    if (segment_index < 0 || segment_index >= static_cast<int>(m_slopes_and_offsets.size())) {
        return 0.0f; // Default return value if segment_index is out of range
    }

    const std::pair<float, float> &slope_and_offset_this_segment = m_slopes_and_offsets.at(segment_index);
    const std::pair<float, float> &slope_and_offset_previus_segment = segment_index > 0 ? m_slopes_and_offsets.at(segment_index - 1) : slope_and_offset_this_segment;
    const std::pair<float, float> &slope_and_offset_next_segment = segment_index + 1 < static_cast<int>(m_slopes_and_offsets.size()) ? m_slopes_and_offsets.at(segment_index + 1) : slope_and_offset_this_segment;

    const float fx_this = slope_and_offset_this_segment.first * x + slope_and_offset_this_segment.second;
    const float fx_previous = slope_and_offset_previus_segment.first * x + slope_and_offset_previus_segment.second;
    const float fx_next = slope_and_offset_next_segment.first * x + slope_and_offset_next_segment.second;

    const float segment_length = m_x_data[segment_index + 1] - m_x_data[segment_index];
    const float distance_to_previous_normalized = (x - m_x_data[segment_index]) / segment_length;
    const float distance_to_next_normalized = (m_x_data[segment_index + 1] - x) / segment_length;

    return fx_this * 0.5 + fx_previous * (1 - distance_to_previous_normalized) + fx_next * (1 - distance_to_next_normalized);
}


std::pair<float, float> FWHMFitter::get_segment_maximum(int segment_index) const {
    if (segment_index < 0 || segment_index >= static_cast<int>(m_slopes_and_offsets.size())) {
        throw std::out_of_range("Segment index out of range");
    }

    const float xmin = m_x_data.at(segment_index);
    const float xmax = m_x_data.at(segment_index + 1);
    const float delta = xmax - xmin;

    const std::pair<float, float> &kq_this = m_slopes_and_offsets.at(segment_index);
    const std::pair<float, float> &kq_previous = segment_index > 0 ? m_slopes_and_offsets.at(segment_index - 1) : kq_this;
    const std::pair<float, float> &kq_next = segment_index + 1 < static_cast<int>(m_slopes_and_offsets.size()) ? m_slopes_and_offsets.at(segment_index + 1) : kq_this;

    const float numerator = kq_this.first * kq_previous.first*(1 + xmin/delta) + kq_next.first*(1 - xmax/delta) + (kq_previous.second + kq_next.second)/delta;
    const float denominator = (2.0/delta) * (kq_previous.first - kq_next.first);

    const float maximum_x = denominator != 0.0f ? numerator / denominator : xmin-1; // Avoid division by zero

    const bool maximum_in_range = maximum_x >= xmin && maximum_x <= xmax;

    if (!maximum_in_range) {
        const float y_at_xmin = this->operator()(xmin, segment_index);
        const float y_at_xmax = this->operator()(xmax, segment_index);
        return y_at_xmin > y_at_xmax ? std::make_pair(xmin, y_at_xmin) : std::make_pair(xmax, y_at_xmax);
    }

    const float y_at_maximum = this->operator()(maximum_x, segment_index);
    return std::make_pair(maximum_x, y_at_maximum);
};


std::array<double, 3> AstroPhotoStacker::FWHMCalculator::fit_by_1d_gaussian(  const std::vector<double> &data_x,
                                                                            const std::vector<double> &data_y,
                                                                            const std::array<double, 3> &initial_guess,
                                                                            double learning_rate,
                                                                            double decay_rate,
                                                                            double beta,
                                                                            int max_iterations) {


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
            accumulated_gradients2.at(i_param) = beta_grad2 * accumulated_gradients2.at(i_param) + (1 - beta_grad2) * gradients.at(i_param) * gradients.at(i_param);
            accumulated_gradients.at(i_param) = beta * accumulated_gradients.at(i_param) + (1 - beta) * gradients.at(i_param);


            fitted_parameters.at(i_param) -= learning_rate * accumulated_gradients.at(i_param) / (std::sqrt(accumulated_gradients2.at(i_param)) + epsilon);
        }
        //learning_rate *= decay_rate;

    }

    return fitted_parameters;
}