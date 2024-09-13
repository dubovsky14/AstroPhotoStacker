#pragma once

#include "../headers/StarFinder.h"
#include "../headers/KDTree.h"
#include "../headers/PlateSolver.h"
#include "../headers/ReferencePhotoHandlerBase.h"

#include <memory>
#include <string>
#include <vector>
#include <tuple>

namespace AstroPhotoStacker   {

    /**
     * @brief Class responsible for handling the reference photo. It provides also methods for plate-solving another photo - how it should be rotated and shifted to match the reference photo
    */
    class ReferencePhotoHandlerStars : public ReferencePhotoHandlerBase {
        public:
            ReferencePhotoHandlerStars()                             = delete;
            ReferencePhotoHandlerStars(const ReferencePhotoHandlerStars&) = delete;

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param raw_file_address - path to the raw file
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerStars(const std::string &raw_file_address, float threshold_fraction = 0.0005);

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param width - width of the photo
             * @param height - height of the photo
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerStars(const unsigned short *brightness, int width, int height, float threshold_fraction = 0.0005)  :
                ReferencePhotoHandlerBase(brightness, width, height, threshold_fraction) {
                initialize(brightness, width, height, threshold_fraction);
            };

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param stars - vector of tuples containing the x and y coordinates of the stars and number of their pixels together with the numbers of the pixels in individual stars
             * @param width - width of the photo in pixels
             * @param height - height of the photo in pixels
            */
            ReferencePhotoHandlerStars(const std::vector<std::tuple<float, float, int> > &stars, int width, int height) :
                ReferencePhotoHandlerBase(stars, width, height) {
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
            unsigned int get_number_of_hashes() const { return m_kd_tree->get_n_nodes(); };

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
            virtual bool calculate_alignment(const std::string &file_address, float *shift_x, float *shift_y, float *rot_center_x, float *rot_center_y, float *rotation) const override;

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

        private:
            int m_width;
            int m_height;

            std::vector<std::tuple<float, float, int> > m_stars;
            std::unique_ptr<KDTree<float, 4, std::tuple<unsigned, unsigned, unsigned, unsigned>>> m_kd_tree = nullptr;
            std::unique_ptr<PlateSolver> m_plate_solver = nullptr;

            virtual void initialize(const unsigned short *brightness, int width, int height, float threshold_fraction = 0.0005) override   {
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