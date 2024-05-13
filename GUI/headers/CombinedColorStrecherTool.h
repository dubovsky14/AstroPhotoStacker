#pragma once

#include "../headers/IndividualColorStretchingToolBase.h"

#include <vector>
#include <memory>
#include <stdexcept>



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
        void add_luminance_stretcher(std::shared_ptr<IndividualColorStretchingToolBase> stretcher) {
            m_luminance_stretchers.push_back(stretcher);
        };

        /**
         * @brief Add stretcher for color channel
         *
         * @param stretcher the stretcher to add
         * @param color the color index
        */
        void add_color_stretcher(std::shared_ptr<IndividualColorStretchingToolBase> stretcher, unsigned int color) {
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
            for (const std::shared_ptr<IndividualColorStretchingToolBase> &stretcher : m_luminance_stretchers) {
                result = stretcher->stretch(result, max_value);
            }
            for (const std::shared_ptr<IndividualColorStretchingToolBase> &stretcher : m_color_stretchers[color]) {
                result = stretcher->stretch(result, max_value);
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
            for (const std::shared_ptr<IndividualColorStretchingToolBase> &stretcher : m_luminance_stretchers) {
                result = stretcher->stretch(result, max_value);
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

        IndividualColorStretchingToolBase& get_luminance_stretcher(unsigned int index) {
            if (index >= m_luminance_stretchers.size()) {
                throw std::invalid_argument("CombinedColorStrecherTool::get_luminance_stretcher: Index out of range");
            }
            return *m_luminance_stretchers[index];
        };

        IndividualColorStretchingToolBase& get_color_stretcher(unsigned int color, unsigned int index) {
            if (color >= m_n_colors) {
                throw std::invalid_argument("CombinedColorStrecherTool::get_color_stretcher: Color index out of range");
            }
            if (index >= m_color_stretchers[color].size()) {
                throw std::invalid_argument("CombinedColorStrecherTool::get_color_stretcher: Index out of range");
            }
            return *m_color_stretchers[color][index];
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
        std::vector<std::shared_ptr<IndividualColorStretchingToolBase>>                 m_luminance_stretchers;
        std::vector<std::vector<std::shared_ptr<IndividualColorStretchingToolBase>>>    m_color_stretchers;
};