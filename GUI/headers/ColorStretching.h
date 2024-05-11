#pragma once
#include "../../headers/Common.h"

#include <vector>

/**
 * @brief The class for individual stretching - applying one color curve to the image
*/
class IndividualColorStretchingTool {
    public:
        IndividualColorStretchingTool() = default;

        /**
         * @brief Construct a new Individual Color Stretching Tool object
         *
         * @param black_point the black point - as a fraction - from interval [0, 1]
         * @param midtone the midtone - as a fraction - from interval [0, 1]
         * @param white_point the white point - as a fraction - from interval [0, 1]
        */
        IndividualColorStretchingTool(float black_point, float midtone, float white_point)  {
            set_stretching_parameters(black_point, midtone, white_point);
        };

        /**
         * @brief Set straching parameters
         *
         * @param black_point the black point - as a fraction - from interval [0, 1]
         * @param midtone the midtone - as a fraction - from interval [0, 1]
         * @param white_point the white point - as a fraction - from interval [0, 1]
        */
        void set_stretching_parameters(float black_point, float midtone, float white_point) {
            m_black_point   = black_point;
            m_midtone       = midtone;
            m_white_point   = white_point;

            m_one_over_range  = 1 / (m_white_point - m_black_point);

            // quadratic stretching
            m_a = 2 - 4 * m_midtone;
            m_b = 4 * m_midtone - 1;
        };

        /**
         * @brief Stretch the value
         *
         * @param value the value to stretch
         * @return float the stretched value
        */
        float stretch(float value, float full_scale_max) const    {
            // firstly, normalize the value
            value = value / full_scale_max;
            float normalized_value =  (value - m_black_point) * m_one_over_range;
            normalized_value = AstroPhotoStacker::force_range<float>(normalized_value, 0, 1);


            normalized_value = m_a*normalized_value*normalized_value + m_b*normalized_value;

            // apply the full scale
            const float result = normalized_value * full_scale_max;
            return AstroPhotoStacker::force_range<float>(result, 0,   full_scale_max);
        };

    private:
        float m_black_point     = 0;
        float m_midtone         = 0.5;
        float m_white_point     = 1;
        float m_one_over_range  = 1;

        // quadratic stretching
        float m_a = 2 - 4 * m_midtone;
        float m_b = 4 * m_midtone - 1;
};

/**
 * @brief The class for combined stretching - applying multiple color curves to the image, either to luminance or to each color channel
*/
class CombinedColorStrecherTool {
    public:
        CombinedColorStrecherTool(unsigned int n_colors = 3) {
            m_n_colors = n_colors;
            m_color_stretchers.resize(n_colors);
        };

        /**
         * @brief Add stretcher for luminance - it will be applied to all color channels
         *
         * @param stretcher the stretcher to add
        */
        void add_luminance_stretcher(const IndividualColorStretchingTool& stretcher) {
            m_luminance_stretchers.push_back(stretcher);
        };

        /**
         * @brief Add stretcher for color channel
         *
         * @param stretcher the stretcher to add
         * @param color the color index
        */
        void add_color_stretcher(const IndividualColorStretchingTool& stretcher, unsigned int color) {
            if (color >= m_n_colors) {
                throw std::invalid_argument("CombinedColorStrecherTool::add_color_stretcher: Color index out of range");
            }
            m_color_stretchers[color].push_back(stretcher);
        };

        /**
         * @brief Stretch the value
         *
         * @param value the value to stretch
         * @param color the color index
         * @return float the stretched value
        */
        float stretch(float value, float max_value, unsigned int color) const {
            if (color >= m_n_colors) {
                throw std::invalid_argument("CombinedColorStrecherTool::stretch: Color index out of range");
            }
            float result = value;
            for (const IndividualColorStretchingTool &stretcher : m_luminance_stretchers) {
                result = stretcher.stretch(result, max_value);
            }
            for (const IndividualColorStretchingTool &stretcher : m_color_stretchers[color]) {
                result = stretcher.stretch(result, max_value);
            }
            return result;
        };

        /**
         * @brief Stretch luminance
         *
         * @param value the value to stretch
         * @return float the stretched value
        */
        float stretch_luminance(float value, float max_value) const {
            float result = value;
            for (const IndividualColorStretchingTool &stretcher : m_luminance_stretchers) {
                result = stretcher.stretch(result, max_value);
            }
            return result;
        };

        void remove_all_luminance_stretchers() {
            m_luminance_stretchers.clear();
        };

        void remove_all_stretchers_for_given_color(unsigned int color) {
            if (color >= m_n_colors) {
                throw std::invalid_argument("CombinedColorStrecherTool::remove_all_stretchers_for_given_color: Color index out of range");
            }
            m_color_stretchers[color].clear();
        };

        void remove_luminance_stretcher(unsigned int index) {
            if (index >= m_luminance_stretchers.size()) {
                throw std::invalid_argument("CombinedColorStrecherTool::remove_luminance_stretcher: Index out of range");
            }
            m_luminance_stretchers.erase(m_luminance_stretchers.begin() + index);
        };

        void remove_stretcher_for_given_color(unsigned int color, unsigned int index) {
            if (color >= m_n_colors) {
                throw std::invalid_argument("CombinedColorStrecherTool::remove_stretcher_for_given_color: Color index out of range");
            }
            if (index >= m_color_stretchers[color].size()) {
                throw std::invalid_argument("CombinedColorStrecherTool::remove_stretcher_for_given_color: Index out of range");
            }
            m_color_stretchers[color].erase(m_color_stretchers[color].begin() + index);
        };

        unsigned int get_n_colors() const {
            return m_n_colors;
        };

        unsigned int get_n_luminance_stretchers() const {
            return m_luminance_stretchers.size();
        };

        unsigned int get_n_color_stretchers(unsigned int color) const {
            if (color >= m_n_colors) {
                throw std::invalid_argument("CombinedColorStrecherTool::get_n_color_stretchers: Color index out of range");
            }
            return m_color_stretchers[color].size();
        };

        IndividualColorStretchingTool& get_luminance_stretcher(unsigned int index) {
            if (index >= m_luminance_stretchers.size()) {
                throw std::invalid_argument("CombinedColorStrecherTool::get_luminance_stretcher: Index out of range");
            }
            return m_luminance_stretchers[index];
        };

        IndividualColorStretchingTool& get_color_stretcher(unsigned int color, unsigned int index) {
            if (color >= m_n_colors) {
                throw std::invalid_argument("CombinedColorStrecherTool::get_color_stretcher: Color index out of range");
            }
            if (index >= m_color_stretchers[color].size()) {
                throw std::invalid_argument("CombinedColorStrecherTool::get_color_stretcher: Index out of range");
            }
            return m_color_stretchers[color][index];
        };

        /**
         * @brief Check if there are any stretchers
         * @return true if there are any stretchers, false otherwise
        */
        bool has_stretchers() const {
            return !m_luminance_stretchers.empty() || !m_color_stretchers.empty();
        };

    private:
        unsigned m_n_colors = 3;
        std::vector<IndividualColorStretchingTool>                  m_luminance_stretchers;
        std::vector<std::vector<IndividualColorStretchingTool>>     m_color_stretchers;
};