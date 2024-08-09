#include "../headers/SyntheticFlatCreator.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/KDTree.h"
#include "../headers/Fitter.h"

#include <algorithm>
#include <iostream>
#include <cmath>

using namespace AstroPhotoStacker;
using namespace std;

SyntheticFlatCreator::SyntheticFlatCreator(const std::string &input_file) {
    load_data(input_file);
};

void SyntheticFlatCreator::create_and_save_synthetic_flat(const std::string &output_file) {
    calculate_threshold();
    replace_values_above_threshold();
    rebin_data(m_rescaled_square_size);
    fit_parameters();
    save_flat(output_file);
};

void SyntheticFlatCreator::load_data(const std::string &input_file) {
    CalibratedPhotoHandler calibrated_photo_handler(input_file, true);
    calibrated_photo_handler.define_alignment(0, 0, 0, 0, 0);
    calibrated_photo_handler.calibrate();

    m_height = calibrated_photo_handler.get_height();
    m_width = calibrated_photo_handler.get_width();

    vector<vector<short unsigned int>> calibrated_color_data = calibrated_photo_handler.get_calibrated_data_after_color_interpolation();

    m_original_gray_scale_data.resize(calibrated_color_data[0].size(), 0);
    for (unsigned int i = 0; i < calibrated_color_data[0].size(); i++) {
        int mean = 0;    // ushort is 16 bits, so it could overflow
        for (unsigned int i_color = 0; i_color < calibrated_color_data.size(); i_color++) {
            mean += calibrated_color_data[i_color][i];
        }
        mean /= calibrated_color_data.size();
        m_original_gray_scale_data[i] = mean;
    }
}

void SyntheticFlatCreator::calculate_threshold()    {
    vector<int> histogram(1 << 16, 0);
    for (unsigned short pixel_value : m_original_gray_scale_data) {
        histogram[pixel_value]++;
    }

    const float fraction = 0.9;
    const int total_pixels = m_width * m_height;
    int sum = 0;
    for (unsigned short pixel_value = 0; pixel_value < histogram.size(); pixel_value++) {
        sum += histogram[pixel_value];
        if (sum > total_pixels*fraction) {
            m_threshold = pixel_value;
            break;
        }
    }
};

void SyntheticFlatCreator::replace_values_above_threshold() {
    unsigned int coordinate_buffer[2];

    // build kd-tree for pixels below threshold
    KDTree<unsigned, 2, unsigned> kdtree;
    for (unsigned int y = 0; y < m_height; y++) {
        coordinate_buffer[1] = y;
        for (unsigned int x = 0; x < m_width; x++) {
            coordinate_buffer[0] = x;
            const unsigned short pixel_value = m_original_gray_scale_data[y * m_width + x];
            if (pixel_value < m_threshold) {
                kdtree.add_point(coordinate_buffer, pixel_value);
            }
        }
    }
    kdtree.build_tree_structure();

    // replace values above threshold by average of N nearest neighbors below threshold
    for (unsigned int y = 0; y < m_height; y++) {
        coordinate_buffer[1] = y;
        for (unsigned int x = 0; x < m_width; x++) {
            const unsigned short pixel_value = m_original_gray_scale_data[y * m_width + x];
            coordinate_buffer[0] = x;
            if (pixel_value >= m_threshold) {
                const std::vector<std::tuple<std::vector<unsigned>, unsigned>> neighbors = kdtree.get_k_nearest_neighbors(coordinate_buffer, 5);

                if (neighbors.size() > 0) {
                    unsigned sum = 0;
                    unsigned sum_weights = 0;
                    for (const auto &neighbor : neighbors) {
                        const unsigned weight = std::get<1>(neighbor);
                        sum += weight;
                        sum_weights += weight;
                    }
                    m_original_gray_scale_data[y * m_width + x] = sum / sum_weights;
                }
            }
        }
    }
};

void SyntheticFlatCreator::rebin_data(unsigned int new_bin_size)    {
    const unsigned int rounded_width = (m_width / new_bin_size) * new_bin_size;
    const unsigned int rounded_height = (m_height / new_bin_size) * new_bin_size;
    m_rebinned_data.resize(rounded_height / new_bin_size, vector<float>(rounded_width / new_bin_size, 0));
    for (unsigned int y = 0; y < rounded_height; y++) {
        for (unsigned int x = 0; x < rounded_width; x++) {
            const unsigned int x_new = x / new_bin_size;
            const unsigned int y_new = y / new_bin_size;
            m_rebinned_data[y_new][x_new] += m_original_gray_scale_data[y * m_width + x];
        }
    }

    for (unsigned int y = 0; y < m_rebinned_data.size(); y++) {
        for (unsigned int x = 0; x < m_rebinned_data[0].size(); x++) {
            m_rebinned_data[y][x] /= new_bin_size * new_bin_size;
        }
    }
};

void SyntheticFlatCreator::fit_parameters() {
    m_center_x = fit_center_x();

};

float SyntheticFlatCreator::fit_center_x()  {
    vector<double> data_x;
    for (unsigned int i = 0; i < m_rebinned_data[0].size(); i++) {
        data_x.push_back((i+0.5)*m_rescaled_square_size);
    }
    vector<double> data_y(m_rebinned_data.at(0).size(), 0);

    vector<double> params;
    params.push_back(m_rebinned_data.at(0).at(0)); // normalization
    params.push_back(m_width/4);    // center
    params.push_back(m_width);  // sigma
    params.push_back(m_rebinned_data.at(0).at(0)); // background

    vector<pair<double, double>> limits{
        {0, 3*m_rebinned_data.at(0.5*m_height/m_rescaled_square_size).at(0.5*m_width/m_rescaled_square_size)},
        {0, m_width},
        {0, m_width*10},
        {0, m_rebinned_data.at(0).at(0)*2}
    };

    auto gauss_on_flat_background = [](const double *parameters, double x) {
        return parameters[0] * exp(-0.5 * (x - parameters[1]) * (x - parameters[1]) / (parameters[2] * parameters[2])) + parameters[3];
    };

    auto objective_function = [&data_x, &data_y, &gauss_on_flat_background](const double *parameters) {
        double sum = 0;
        for (unsigned int i = 0; i < data_x.size(); i++) {
            const double y_fitted = gauss_on_flat_background(parameters, data_x.at(i));
            const double diff = y_fitted - data_y.at(i);
            sum += diff * diff;
        }
        return sum;
    };

    Fitter fitter(&params, limits);
    vector<float> partial_results;
    for (unsigned int i_line = 0; i_line < m_rebinned_data.size(); i_line++) {
        for (unsigned int i_column = 0; i_column < m_rebinned_data.at(i_line).size(); i_column++) {
            data_y[i_column] = m_rebinned_data[i_line][i_column];
        }
        fitter.fit_gradient(objective_function, 0.02, 0.99, 1000);
        partial_results.push_back(params.at(1));
        cout << "Line " << i_line << " center x = " << params.at(1) << endl;
    }

    if (partial_results.size() == 0) {
        return -1;
    }

    std::sort(partial_results.begin(), partial_results.end());
    const float center_position = partial_results.at(partial_results.size()/2);
    return center_position;
};

void SyntheticFlatCreator::get_flat_center(float *center_x, float *center_y)    {
    *center_x = fit_center_x();

};

void SyntheticFlatCreator::save_flat(const std::string &output_file)    {
    cout << "Saving flat\n";
    cout << "Center x = " << m_center_x << endl;
};