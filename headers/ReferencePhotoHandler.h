#pragma once

#include "../headers/StarFinder.h"
#include "../headers/KDTree.h"
#include "../headers/PlateSolver.h"

#include <memory>
#include <string>
#include <vector>
#include <tuple>

namespace AstroPhotoStacker   {

    /**
     * @brief Class responsible for handling the reference photo. It provides also methods for plate-solving another photo - how it should be rotated and shifted to match the reference photo
    */
    class ReferencePhotoHandler {
        public:
            ReferencePhotoHandler()                             = delete;
            ReferencePhotoHandler(const ReferencePhotoHandler&) = delete;

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param raw_file_address - path to the raw file
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandler(const std::string &raw_file_address, float threshold_fraction = 0.0005);

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param width - width of the photo
             * @param height - height of the photo
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            template<typename pixel_brightness_type = unsigned short>
            ReferencePhotoHandler(const pixel_brightness_type *brightness, int width, int height, float threshold_fraction = 0.0005)    {
                initialize(brightness, width, height, threshold_fraction);
            };

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param stars - vector of tuples containing the x and y coordinates of the stars and number of their pixels together with the numbers of the pixels in individual stars
             * @param width - width of the photo in pixels
             * @param height - height of the photo in pixels
            */
            ReferencePhotoHandler(const std::vector<std::tuple<float, float, int> > &stars, int width, int height)  {
                initialize(stars, width, height);
            };

            /**
             * @brief Get the width of the reference photo
             *
             * @return int - width of the reference photo
            */
            int get_width()     const   { return m_width; };

            /**
             * @brief Get the height of the reference photo
             *
             * @return int - height of the reference photo
            */
            int get_height()    const   { return m_height; };

            /**
             * @brief Get the number of precalculated hashes
             *
             * @return unsigned int - number of hashes
            */
            unsigned int get_number_of_hashes() const { return m_kd_tree->get_number_of_points_in_tree(); };

            /**
             * @brief Plate-solve a photo - calculate how it should be rotated and shifted to match the reference photo
             *
             * @param file_address - path to the file to be plate-solved
             * @param shift_x - pointer to the variable where the horizontal shift will be stored
             * @param shift_y - pointer to the variable where the vertical shift will be stored
             * @param rot_center_x - pointer to the variable where the x coordinate of the rotation center will be stored
             * @param rot_center_y - pointer to the variable where the y coordinate of the rotation center will be stored
             * @param rotation - pointer to the variable where the rotation angle will be stored
             * @return true - if the plate was solved
             * @return false - if the plate was not solved
            */
            bool plate_solve(const std::string &file_address, float *shift_x, float *shift_y, float *rot_center_x, float *rot_center_y, float *rotation) const;

            /**
             * @brief Plate-solve a photo - calculate how it should be rotated and shifted to match the reference photo
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param shift_x - pointer to the variable where the horizontal shift will be stored
             * @param shift_y - pointer to the variable where the vertical shift will be stored
             * @param rot_center_x - pointer to the variable where the x coordinate of the rotation center will be stored
             * @param rot_center_y - pointer to the variable where the y coordinate of the rotation center will be stored
             * @param rotation - pointer to the variable where the rotation angle will be stored
            */
            bool plate_solve(const std::vector<std::tuple<float, float, int> > &stars, float *shift_x, float *shift_y, float *rot_center_x, float *rot_center_y, float *rotation) const;

            /**
             * @brief Get the hash with a given index from the reference photo
             *
             * @param hash_index - index of the hash
             * @return std::tuple<std::tuple<float,float,float,float>,unsigned int, unsigned int, unsigned int, unsigned int> - tuple containing the hash (as tuple), followed by indices of the stars forming the hash
            */
            std::tuple<std::tuple<float,float,float,float>,unsigned int, unsigned int, unsigned int, unsigned int> get_hash(unsigned int hash_index) const;

        private:
            int m_width;
            int m_height;

            std::vector<std::tuple<float, float, int> > m_stars;
            std::unique_ptr<KDTree> m_kd_tree = nullptr;
            std::unique_ptr<PlateSolver> m_plate_solver = nullptr;

            template<typename pixel_brightness_type = unsigned short>
            void initialize(const pixel_brightness_type *brightness, int width, int height, float threshold_fraction = 0.0005)   {
                const unsigned short threshold = get_threshold_value<unsigned short>(&brightness[0], width*height, threshold_fraction);
                std::vector<std::tuple<float, float, int> > stars = get_stars(&brightness[0], width, height, threshold);
                keep_only_stars_above_size(&stars, 9);
                sort_stars_by_size(&stars);

                initialize(stars, width, height);
            };

            void initialize(const std::vector<std::tuple<float, float, int> > &stars, int width, int height);

            void calculate_and_store_hashes();

    };
}