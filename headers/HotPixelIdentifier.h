#pragma once

#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <atomic>
#include <tuple>

namespace AstroPhotoStacker {
    class HotPixelIdentifier    {
        public:
            void add_photos(const std::vector<std::string> &photo_addresses);

            void add_photo(const std::string &photo_address);

            void add_photo(const unsigned short int *pixel_value_array, int width, int height, int image_bit_depth = 14);

            static std::map<std::tuple<int,int>,int> get_hot_pixel_candidates_from_photo(const unsigned short int *pixel_value_array, int width, int height, int image_bit_depth);

            void compute_hot_pixels();

            const std::vector<std::tuple<int,int>>& get_hot_pixels() const;

            void save_hot_pixels_to_file(const std::string &file_address) const;

            void load_hot_pixels_from_file(const std::string &file_address);

            void set_n_cpu(unsigned int n_cpu);

            void set_hot_pixels(const std::vector<std::tuple<int,int>> &hot_pixels);

            bool is_hot_pixel(int x, int y) const;

            const std::atomic<int> &get_number_of_processed_photos() const;

        private:
            unsigned int m_n_cpu = 1;
            std::map<std::tuple<int,int>, int>  m_hot_pixel_candidates;
            std::vector<std::tuple<int,int>>    m_hot_pixels;
            std::map<std::tuple<int,int>,char>  m_hot_pixels_map;
            std::mutex m_mutex;
            std::atomic<int> m_n_photos_processed = 0;

    };
}