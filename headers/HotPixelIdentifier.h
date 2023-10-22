#pragma once

#include <vector>
#include <map>
#include <string>


namespace AstroPhotoStacker {
    class HotPixelIdentifier    {
        public:
            void add_photo(const std::string &photo_address);

            void add_photo(const unsigned short int *pixel_value_array, int width, int height, int image_bit_depth = 14);

            static std::map<std::tuple<int,int>,int> get_hot_pixel_candidates_from_photo(const unsigned short int *pixel_value_array, int width, int height, int image_bit_depth);

            void compute_hot_pixels();

            std::vector<std::tuple<int,int>> get_hot_pixels() const;

            void save_hot_pixels_to_file(const std::string &file_address) const;

            void load_hot_pixels_from_file(const std::string &file_address);

            bool is_hot_pixel(int x, int y) const;

        private:
            unsigned int m_number_of_photos = 0;
            std::map<std::tuple<int,int>, int> m_hot_pixel_candidates;
            std::map<std::tuple<int,int>, bool> m_hot_pixels;


    };
}