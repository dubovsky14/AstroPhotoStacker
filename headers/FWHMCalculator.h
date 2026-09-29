#pragma once

#include "../headers/PixelType.h"

#include <vector>
#include <utility>
#include <array>
#include <stdexcept>
#include <tuple>

namespace AstroPhotoStacker {
    class FWHMCalculator {
        public:
            FWHMCalculator() = delete;

            FWHMCalculator(const std::vector<PixelType> &brightness, int width, int height);

            FWHMCalculator(const std::vector<PixelType> &brightness, int width, int height, std::vector<std::tuple<float, float, int>> &clusters, PixelType threshold);

            float calculate_fwhm(int stars_to_use = 200) const;

            static std::array<double, 3> fit_by_1d_gaussian(const std::vector<double> &data_x,
                                                            const std::vector<double> &data_y,
                                                            const std::array<double, 3> &initial_guess,
                                                            double learning_rate,
                                                            double beta,
                                                            int max_iterations);

        private:
            PixelType m_star_threshold         = 32767;
            PixelType m_background_threshold   = 32767;
            const std::vector<PixelType> *m_brightness = nullptr;
            int m_width  = 0;
            int m_height = 0;
            std::vector<std::tuple<float,float,int>> m_clusters;

            float calculate_fwhm_for_star(std::array<int, 2> star_position, std::array<int, 2> direction) const;

    };
}