#include "../headers/HistogramDataTool.h"

#include <iostream>

using namespace std;


HistogramDataTool::HistogramDataTool(int max_value, int number_of_colors) {
    m_max_value = max_value;
    m_number_of_colors = number_of_colors;
    m_histogram_data_colors = std::vector<std::vector<int>>(m_number_of_colors, std::vector<int>(m_max_value + 1, 0));
    m_histogram_data_luminance = std::vector<int>(m_max_value + 1, 0);
};

std::vector<int> HistogramDataTool::rebin_data(const std::vector<int> &original_histogram, unsigned int new_n_bins) {
    if (new_n_bins == original_histogram.size()) {
        return original_histogram;
    }
    std::vector<int> result(new_n_bins, 0);
    const float step = float(original_histogram.size()) / float(new_n_bins);
    for (unsigned int i = 0; i < original_histogram.size(); i++) {
        result[min(unsigned(i / step), new_n_bins-1)] += original_histogram[i];
    }
    return result;
};

vector<float> HistogramDataTool::get_mean_values(const CombinedColorStrecherTool *color_stretcher) const  {
    vector<float> result;

    for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
        float sum = 0;
        int number_of_pixels = 0;
        for (unsigned int i_value = 0; i_value < m_histogram_data_colors[i_color].size(); i_value++) {
            sum += i_value * m_histogram_data_colors[i_color][i_value];
            number_of_pixels += m_histogram_data_colors[i_color][i_value];
        }
        result.push_back(sum/number_of_pixels);
    }
    return result;
};

std::vector<float> HistogramDataTool::get_median_values(const CombinedColorStrecherTool *color_stretcher) const    {
    vector<float> result;

    for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
        int number_of_pixels = 0;
        for (unsigned int i_value = 0; i_value < m_histogram_data_colors[i_color].size(); i_value++) {
            number_of_pixels += m_histogram_data_colors[i_color][i_value];
        }
        int median_index = number_of_pixels/2;
        int sum = 0;
        for (unsigned int i_value = 0; i_value < m_histogram_data_colors[i_color].size(); i_value++) {
            sum += m_histogram_data_colors[i_color][i_value];
            if (sum >= median_index) {
                result.push_back(i_value);
                break;
            }
        }
    }
    return result;
};

const std::vector<int>& HistogramDataTool::get_histogram_data_colors(int i_color)   const {
    return m_histogram_data_colors[i_color];
};

std::vector<std::vector<int>> HistogramDataTool::get_stretched_color_data(const CombinedColorStrecherTool &color_stretcher) const {
    std::vector<std::vector<int>> result(m_number_of_colors, std::vector<int>(m_max_value + 1, 0));
    for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
        for (unsigned int i_value = 0; i_value < m_histogram_data_colors[i_color].size(); i_value++) {
            const int stretched_value = color_stretcher.stretch(i_value, m_max_value,i_color);
            result[i_color][stretched_value] += m_histogram_data_colors[i_color][i_value];
        }
    }

    return result;
};

const std::vector<std::vector<int>>& HistogramDataTool::get_histogram_data_colors() const {
    return m_histogram_data_colors;
};