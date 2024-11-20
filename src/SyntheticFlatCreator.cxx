#include "../headers/SyntheticFlatCreator.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/KDTree.h"
#include "../headers/Fitter.h"
#include "../headers/Common.h"
#include "../headers/ImageFilesInputOutput.h"

#include <algorithm>
#include <iostream>
#include <cmath>

using namespace AstroPhotoStacker;
using namespace std;

SyntheticFlatCreator::SyntheticFlatCreator(const InputFrame &input_frame) {
    load_data(input_frame);
};

void SyntheticFlatCreator::create_and_save_synthetic_flat(const std::string &output_file) {
    calculate_threshold();
    replace_values_above_threshold();
    create_gray_scale_image(m_original_gray_scale_data.data(), m_width, m_height, "original_gray_scale.tif", CV_16U);
    rebin_data(m_rescaled_square_size);
    fit_parameters();
    save_flat(output_file);
};

void SyntheticFlatCreator::load_data(const InputFrame &input_frame) {
    CalibratedPhotoHandler calibrated_photo_handler(input_frame, true);
    calibrated_photo_handler.define_alignment(0, 0, 0, 0, 0);
    calibrated_photo_handler.calibrate();

    m_height = calibrated_photo_handler.get_height();
    m_width = calibrated_photo_handler.get_width();

    vector<vector<short int>> calibrated_color_data = calibrated_photo_handler.get_calibrated_data_after_color_interpolation();

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
                const std::vector<std::tuple<std::array<unsigned,2>, unsigned>> neighbors = kdtree.get_k_nearest_neighbors(coordinate_buffer, 5);

                if (neighbors.size() > 0) {
                    unsigned sum = 0;
                    for (const auto &neighbor : neighbors) {
                        const unsigned brightness = std::get<1>(neighbor);
                        sum += brightness;
                    }
                    m_original_gray_scale_data[y * m_width + x] = sum / neighbors.size();
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
    m_center_x = fit_center(m_rebinned_data);
    m_center_y = fit_center(get_transponded_vector(m_rebinned_data));
    fit_function_of_distance();
};

void SyntheticFlatCreator::fit_function_of_distance()   {
    initialize_function_of_distance_and_its_parameters();
    const std::pair<vector<float>, vector<float>> data_for_fit = get_data_for_fit();
    const vector<float> &distances         = data_for_fit.first;
    const vector<float> &brightness_values = data_for_fit.second;

    auto objective_function = [this, &distances, &brightness_values](const double *parameters) {
        double sum = 0;
        for (unsigned int i = 0; i < distances.size(); i++) {
            const double y_fitted = m_function_of_distance(distances.at(i), parameters);
            const double diff = y_fitted - brightness_values.at(i);
            sum += diff * diff;
        }
        return sum;
    };


    m_brightnesses = brightness_values;
    m_distances = distances;

    Fitter fitter(&m_function_parameters, m_function_parameter_limits);
    fitter.fit_gradient(objective_function, 0.05, 0.8, 10000);

    cout << "Data for fitting:\n";
    for (unsigned int i = 0; i < distances.size(); i++) {
        cout << "\t" << distances.at(i) << "\t" << brightness_values.at(i) << "\t" << m_function_of_distance(distances.at(i), m_function_parameters.data()) << endl;
    }
};

std::pair<vector<float>, vector<float>> SyntheticFlatCreator::get_data_for_fit()    {
    const unsigned int step = 10;
    vector<vector<unsigned short>> data_to_sort; // first vector are all values in distance between 0 and step, second vector between step and 2*step, etc.

    for (unsigned int y = 0; y < m_height; y++) {
        for (unsigned int x = 0; x < m_width; x++) {
            const float distance = sqrt((x - m_center_x)*(x - m_center_x) + (y - m_center_y)*(y - m_center_y));
            const unsigned int bin = distance / step;
            if (bin >= data_to_sort.size()) {
                data_to_sort.resize(bin + 1);
            }
            data_to_sort[bin].push_back(m_original_gray_scale_data[y*m_width + x]);
        }
    }

    vector<float> result_x;
    vector<float> result_y;
    for (unsigned int i = 0; i < data_to_sort.size(); i++) {
        vector<unsigned short> &this_vector = data_to_sort[i];
        std::sort(this_vector.begin(), this_vector.end());
        result_x.push_back(i*step + 0.5*step);
        if (this_vector.size() != 0) {
            double mean = 0;
            int count = 0;
            for (unsigned int i = this_vector.size()/4; i <= 1.4*this_vector.size()/4; i++) {
                mean += this_vector.at(i);
                count++;
            }
            result_y.push_back(mean / count);
        }
        else if (i > 0) {
            result_y.push_back(result_y.at(i-1));
        }
        else {
            result_y.push_back(0);
        }
    }

    return {result_x, result_y};
};

float SyntheticFlatCreator::fit_center(const std::vector<std::vector<float>> &rebinned_data)  {
    const unsigned int width = m_rescaled_square_size * rebinned_data.at(0).size();

    vector<double> data_x;
    for (unsigned int i = 0; i < rebinned_data[0].size(); i++) {
        data_x.push_back((i+0.5)*m_rescaled_square_size);
    }
    vector<double> data_y(rebinned_data.at(0).size(), 0);

    vector<double> params;
    params.push_back(rebinned_data.at(0).at(0)); // normalization
    params.push_back(width/4);    // center
    params.push_back(width);  // sigma
    params.push_back(rebinned_data.at(0).at(0)); // background

    const unsigned int rebinned_half_size = 0.5*rebinned_data.size();
    vector<pair<double, double>> limits{
        {0, 3*rebinned_data.at(rebinned_half_size).at(rebinned_data.at(0).size()/2)},
        {0, width},
        {0, width*10},
        {0, rebinned_data.at(0).at(0)*2}
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
    for (unsigned int i_line = 0; i_line < rebinned_data.size(); i_line++) {
        for (unsigned int i_column = 0; i_column < rebinned_data.at(i_line).size(); i_column++) {
            data_y[i_column] = rebinned_data[i_line][i_column];
        }
        fitter.fit_gradient(objective_function, 0.02, 0.8, 1000);
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

void SyntheticFlatCreator::save_flat(const std::string &output_file)    {
    cout << "Saving flat\n";
    cout << "Center x = " << m_center_x << endl;
    cout << "Center y = " << m_center_y << endl;

    cout << "Function parameters: ";
    for (const auto &param : m_function_parameters) {
        cout << "\t" << param << "\n";
    }

    vector<float> flat_data(m_width*m_height, 0);
    for (unsigned int y = 0; y < m_height; y++) {
        for (unsigned int x = 0; x < m_width; x++) {
            const float distance = sqrt((x - m_center_x)*(x - m_center_x) + (y - m_center_y)*(y - m_center_y));
            //const float brightness = m_function_of_distance(distance, m_function_parameters.data());
            const float brightness = get_brightness(distance);
            flat_data[y*m_width + x] = brightness;
        }
    }

    const float max_value = *max_element(flat_data.begin(), flat_data.end());
    cout << "Current max value: " << max_value << endl;
    const float new_max = 1 << 15;
    if (max_value < new_max) {
        const float ratio = new_max / max_value;
        for (float &pixel : flat_data) {
            pixel *= ratio;
        }
    }

    //cout << "Middle line:\n";
    //for (unsigned int x = 0; x < m_width; x++) {
    //    cout << flat_data[m_height/2*m_width + x] << endl;
    //}

    //create_color_image(flat_data.data(), flat_data.data(), flat_data.data(), m_width, m_height, output_file, CV_16UC3);
    create_gray_scale_image(flat_data.data(), m_width, m_height, output_file, CV_16UC1);
};

void SyntheticFlatCreator::initialize_function_of_distance_and_its_parameters()  {
    const float brightness_in_center = m_rebinned_data.at(m_rebinned_data.size()/2).at(m_rebinned_data.at(0).size()/2);

    m_function_of_distance = [this](double r, const double *parameters) -> double {
        r /= m_width;
        return  parameters[0] +
                parameters[1] * r +
                parameters[2] * r * r +
                parameters[3] * r * r * r +
                parameters[4] * r * r * r * r +
                parameters[5] * r * r * r * r * r +
                parameters[6] * r * r * r * r * r * r +
                parameters[7] * r * r * r * r * r * r * r +
                parameters[8] * r * r * r * r * r * r * r  * r;
    };

    m_function_parameter_limits = {
        {-2*brightness_in_center, 2*brightness_in_center},
        {-2*brightness_in_center, 2*brightness_in_center},
        {-3*brightness_in_center, 3*brightness_in_center},
        {-3*brightness_in_center, 3*brightness_in_center},
        {-3*brightness_in_center, 3*brightness_in_center},
        {-3*brightness_in_center, 3*brightness_in_center},
        {-3*brightness_in_center, 3*brightness_in_center},
        {-3*brightness_in_center, 3*brightness_in_center},
        {-3*brightness_in_center, 3*brightness_in_center}
    };

    m_function_parameters = {brightness_in_center, -brightness_in_center, 0, 0, 0, 0, 0, 0, 0};

};

//vector<double> m_brightness_cache;
float SyntheticFlatCreator::get_brightness(int distance)   {
    if (m_brightness_cache.find(distance) != m_brightness_cache.end()) {
        return m_brightness_cache.at(distance);
    }

    int index_before = -1;
    int index_after  = -1;
    for (unsigned int i = 0; i < m_distances.size(); i++) {
        if (m_distances.at(i) < distance) {
            index_before = i;
        }
        else if (index_before != -1) {
            index_after = i;
            break;
        }
    }

    if (index_before == -1) {
        return m_brightnesses.at(0);
    }

    if (index_after == -1) {
        return m_brightnesses.at(m_brightnesses.size() - 1);
    }

    const double x1 = m_distances.at(index_before);
    const double x2 = m_distances.at(index_after);

    const double y1 = m_brightnesses.at(index_before);
    const double y2 = m_brightnesses.at(index_after);

    const double m = (y2 - y1) / (x2 - x1);
    const double b = y1 - m*x1;

    const double result = m*distance + b;

    m_brightness_cache[distance] = result;

    return m*distance + b;

};