#pragma once

#include "../headers/ColorStretching.h"

#include <vector>


class HistogramDataTool {
    public:
        HistogramDataTool() = delete;

        HistogramDataTool(int max_value, int number_of_colors);

        template<class datatype>
        void extract_data_from_image(const std::vector<std::vector<datatype>> &image_data) {
            for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
                for (int i_pixel = 0; i_pixel < image_data[i_color].size(); i_pixel++) {
                    m_histogram_data_colors[i_color][image_data[i_color][i_pixel]]++;
                }
            }
            for (int i_pixel = 0; i_pixel < image_data[0].size(); i_pixel++) {
                int luminance = 0;
                for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
                    luminance += image_data[i_color][i_pixel];
                }
                luminance /= m_number_of_colors;
                m_histogram_data_luminance[luminance]++;
            }
        };

        static std::vector<int> rebin_data(const std::vector<int> &original_histogram, unsigned int new_n_bins);

        const std::vector<int>& get_histogram_data_colors(int i_color)  const;

        std::vector<std::vector<int>> get_stretched_color_data(const CombinedColorStrecherTool &color_stretcher) const;

    private:
        int m_max_value;
        int m_number_of_colors;
        std::vector<std::vector<int>>   m_histogram_data_colors;
        std::vector<int>                m_histogram_data_luminance;
};