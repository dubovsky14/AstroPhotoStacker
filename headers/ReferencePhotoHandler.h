#pragma once

#include "../headers/StarFinder.h"

#include <memory>
#include <string>
#include <vector>
#include <tuple>

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

    private:
        int m_width;
        int m_height;

        std::vector<std::tuple<float, float, int> > m_stars;

        template<typename pixel_brightness_type = unsigned short>
        void Initialize(const pixel_brightness_type *brightness, int width, int height, float threshold_fraction = 0.0005)   {
            const unsigned short threshold = get_threshold_value<unsigned short>(&brightness[0], width*height, threshold_fraction);
            std::vector<std::tuple<float, float, int> > stars = get_stars(&brightness[0], width, height, threshold);
            keep_only_stars_above_size(&stars, 9);
            sort_stars_by_size(&stars);

            Initialize(stars, width, height);
        };

        void Initialize(const std::vector<std::tuple<float, float, int> > &stars, int width, int height);



};