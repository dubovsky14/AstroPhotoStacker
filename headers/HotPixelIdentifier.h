#pragma once

#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <atomic>
#include <tuple>

namespace AstroPhotoStacker {


    /**
     * @class HotPixelIdentifier
     * @brief Class for identifying hot pixels in photos.
     *
     * The HotPixelIdentifier class provides methods for adding photos, identifying hot pixel candidates,
     * computing hot pixels, saving and loading hot pixels from a text file.
     * Hot pixels are defined as pixels that consistently have higher or lower values compared to their neighboring pixels.
     */
    class HotPixelIdentifier {
    public:
        /**
         * @brief Adds multiple photos to the hot pixel identifier.
         *
         * @param photo_addresses The addresses of the photos to add.
         */
        void add_photos(const std::vector<std::string> &photo_addresses);

        /**
         * @brief Adds a single photo to the hot pixel identifier.
         *
         * @param photo_address The address of the photo to add.
         */
        void add_photo(const std::string &photo_address);

        /**
         * @brief Adds a photo using a pixel value array.
         *
         * @param pixel_value_array The array of pixel values.
         * @param width The width of the photo.
         * @param height The height of the photo.
         * @param image_bit_depth The bit depth of the photo (default is 14).
         */
        void add_photo(const unsigned short int* pixel_value_array, int width, int height, int image_bit_depth = 14);

        /**
         * @brief Gets the hot pixel candidates from a photo.
         *
         * @param pixel_value_array The array of pixel values.
         * @param width The width of the photo.
         * @param height The height of the photo.
         * @param image_bit_depth The bit depth of the photo.
         * @return A map of hot pixel candidates, where the key is a tuple representing the pixel coordinates (x, y),
         *         and the value is 1.
         */
        static std::map<std::tuple<int, int>, int> get_hot_pixel_candidates_from_photo(const unsigned short int* pixel_value_array, int width, int height, int image_bit_depth);

        /**
         * @brief Computes the hot pixels from the added photos.
         */
        void compute_hot_pixels();

        /**
         * @brief Gets the hot pixels that have been computed.
         * @return A vector of tuples representing the coordinates (x, y) of the hot pixels.
         */
        const std::vector<std::tuple<int, int>>& get_hot_pixels() const;

        /**
         * @brief Saves the hot pixels to a file.
         * @param file_address The address of the file to save the hot pixels to.
         */
        void save_hot_pixels_to_file(const std::string &file_address) const;

        /**
         * @brief Loads the hot pixels from a file.
         * @param file_address The address of the file to load the hot pixels from.
         */
        void load_hot_pixels_from_file(const std::string &file_address);

        /**
         * @brief Sets the number of CPU cores to use for computation.
         * @param n_cpu The number of CPU cores.
         */
        void set_n_cpu(unsigned int n_cpu);

        /**
         * @brief Sets the hot pixels manually.
         * @param hot_pixels The vector of tuples representing the coordinates (x, y) of the hot pixels.
         */
        void set_hot_pixels(const std::vector<std::tuple<int, int>> &hot_pixels);

        /**
         * @brief Checks if a pixel is a hot pixel.
         * @param x The x-coordinate of the pixel.
         * @param y The y-coordinate of the pixel.
         * @return True if the pixel is a hot pixel, false otherwise.
         */
        bool is_hot_pixel(int x, int y) const;

        /**
         * @brief Gets the number of processed photos.
         * @return The number of processed photos as an atomic integer.
         */
        const std::atomic<int>& get_number_of_processed_photos() const;

    private:
        unsigned int m_n_cpu = 1;
        std::map<std::tuple<int, int>, int> m_hot_pixel_candidates;
        std::vector<std::tuple<int, int>> m_hot_pixels;
        std::map<std::tuple<int, int>, char> m_hot_pixels_map;
        std::mutex m_mutex;
        std::atomic<int> m_n_photos_processed = 0;
    };
}