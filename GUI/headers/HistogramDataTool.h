#pragma once

#include "../headers/ColorStretching.h"

#include <vector>


/**
 * @class HistogramDataTool
 * @brief A class for extracting and manipulating histogram data from an image.
 *
 * The HistogramDataTool class provides functionality to extract histogram data from an image,
 * including color and luminance histograms. It also supports rebinning of histogram data and
 * obtaining stretched color data. The histogram data are binned from 0 to max_value, with one integer value per bin.
 */
class HistogramDataTool {
    public:
        /**
         * @brief Default constructor is deleted.
         */
        HistogramDataTool() = delete;

        /**
         * @brief Constructor that initializes the HistogramDataTool object.
         * @param max_value The maximum value of the histogram data (maximal brightness of input image).
         * @param number_of_colors The number of colors in the image.
         */
        HistogramDataTool(int max_value, int number_of_colors);

        /**
         * Default copy constructor.
         */
        HistogramDataTool(const HistogramDataTool&) = default;

        /**
         * @brief Extracts histogram data from the given image data.
         * @tparam datatype The data type of the image dat
         * @param image_data The image data from which to extract the histogram data - first dimension is color, second dimension is pixel index.
         */
        template<class datatype>
        void extract_data_from_image(const std::vector<std::vector<datatype>> &image_data) {
            // Implementation details...
        };

        /**
         * @brief Rebins the original histogram data to a new number of bins.
         * @param original_histogram The original histogram data.
         * @param new_n_bins The new number of bins.
         * @return The rebinned histogram data.
         */
        static std::vector<int> rebin_data(const std::vector<int> &original_histogram, unsigned int new_n_bins);

        /**
         * @brief Gets the histogram data for a specific color.
         * @param i_color The index of the color.
         * @return The histogram data for the specified color.
         */
        const std::vector<int>& get_histogram_data_colors(int i_color)  const;

        /**
         * @brief Gets the stretched color data using the given color stretcher.
         * @param color_stretcher The color stretcher tool.
         * @return The stretched color data - one value per bin, i.e. size of all vectors is max_value + 1.
         */
        std::vector<std::vector<int>> get_stretched_color_data(const CombinedColorStrecherTool &color_stretcher) const;

        /**
         * @brief Gets histogram data for color channels
         * @return The histogram data for color channels - one value per bin, i.e. size of all vectors is max_value + 1.
         */
        const std::vector<std::vector<int>>& get_histogram_data_colors() const;

        /**
         * @brief get number of colors
         * @return The number of colors in the image.
         */
        int get_number_of_colors() const { return m_number_of_colors; };

    private:
        int m_max_value;                                            ///< The maximum value of the histogram data.
        int m_number_of_colors;                                     ///< The number of colors in the image.
        std::vector<std::vector<int>>   m_histogram_data_colors;    ///< The histogram data for each color.
        std::vector<int>                m_histogram_data_luminance; ///< The histogram data for luminance.
};