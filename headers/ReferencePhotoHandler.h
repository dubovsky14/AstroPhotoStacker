#pragma once

#include "../headers/StarFinder.h"
#include "../headers/KDTree.h"
#include "../headers/PlateSolver.h"

#include <memory>
#include <string>
#include <vector>
#include <tuple>

namespace AstroPhotoStacker   {
    class ReferencePhotoHandler {
        public:
            ReferencePhotoHandler(const std::string &raw_file_address, float threshold_fraction = 0.0005);

            template<typename pixel_brightness_type = unsigned short>
            ReferencePhotoHandler(const pixel_brightness_type *brightness, int width, int height, float threshold_fraction = 0.0005)    {
                Initialize(brightness, width, height, threshold_fraction);
            };

            ReferencePhotoHandler(const std::vector<std::tuple<float, float, int> > &stars, int width, int height)  {
                Initialize(stars, width, height);
            };

            int get_width()     const   { return m_width; };
            int get_height()    const   { return m_height; };


            unsigned int get_number_of_hashes() const { return m_kd_tree->get_number_of_points_in_tree(); };

            bool plate_solve(const std::string &file_address, float *shift_x, float *shift_y, float *rot_center_x, float *rot_center_y, float *rotation) const;

            bool plate_solve(const std::vector<std::tuple<float, float, int> > &stars, float *shift_x, float *shift_y, float *rot_center_x, float *rot_center_y, float *rotation) const;

            std::tuple<std::tuple<float,float,float,float>,unsigned int, unsigned int, unsigned int, unsigned int> get_hash(unsigned int hash_index) const;

        private:
            int m_width;
            int m_height;

            std::vector<std::tuple<float, float, int> > m_stars;
            std::unique_ptr<KDTree> m_kd_tree = nullptr;
            std::unique_ptr<PlateSolver> m_plate_solver = nullptr;

            template<typename pixel_brightness_type = unsigned short>
            void Initialize(const pixel_brightness_type *brightness, int width, int height, float threshold_fraction = 0.0005)   {
                const unsigned short threshold = get_threshold_value<unsigned short>(&brightness[0], width*height, threshold_fraction);
                std::vector<std::tuple<float, float, int> > stars = get_stars(&brightness[0], width, height, threshold);
                keep_only_stars_above_size(&stars, 9);
                sort_stars_by_size(&stars);

                Initialize(stars, width, height);
            };

            void Initialize(const std::vector<std::tuple<float, float, int> > &stars, int width, int height);

            void CalculateAndStoreHashes();

    };
}