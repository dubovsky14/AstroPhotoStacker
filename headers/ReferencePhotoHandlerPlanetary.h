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
     * @brief Class responsible for handling the reference photo, providing methods for alignement of planetary photos
     */
    class ReferencePhotoHandlerPlanetary : public ReferencePhotoHandlerBase {
        public:
            ReferencePhotoHandlerPlanetary()                             = delete;
            ReferencePhotoHandlerPlanetary(const ReferencePhotoHandlerPlanetary&) = delete;

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param raw_file_address - path to the raw file
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerPlanetary(const std::string &raw_file_address, float threshold_fraction = 0.5);

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param width - width of the photo
             * @param height - height of the photo
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerPlanetary(const unsigned short *brightness, int width, int height, float threshold_fraction = 0.5);

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

        protected:

            /**
             * @brief Get coordinates of the window where the alignment should be calculated - surrounding the planet
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param width - width of the photo
             * @param height - height of the photo
             * @param threshold - threshold value for the pixels
             * @return std::tuple<int,int,int,int> - coordinates of the window x0, y0, x1, y1
            */
            std::tuple<int,int,int,int> get_alignment_window(const MonochromeImageData &image_data, unsigned short threshold) const;

            std::vector<std::vector<double>> get_covariance_matrix(const MonochromeImageData &image_data, const std::tuple<double,double> &center_of_mass, unsigned short threshold, const std::tuple<int,int,int,int> &window_coordinates) const;

            /**
             * @brief get the center of mass of the planet
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param width - width of the photo
             * @param height - height of the photo
             * @param threshold - threshold value for the pixels
             * @param window_coordinates - coordinates of the window x0, y0, x1, y1
             *
             * @return std::tuple<double,double> - x and y coordinates of the center of mass
             */
            std::tuple<double,double> get_center_of_mass(const MonochromeImageData &image_data, unsigned short threshold, const std::tuple<int,int,int,int> &window_coordinates) const;

            std::tuple<float,float,std::vector<std::vector<double>>,std::vector<double>> get_center_of_mass_eigenvectors_and_eigenvalues(const MonochromeImageData &image_data, float threshold_fraction) const;

            virtual void initialize(const unsigned short *brightness, int width, int height, float threshold_fraction = 0.5) override;

            static void calculate_eigenvectors_and_eigenvalues(const std::vector<std::vector<double>> &covariance_matrix, std::vector<double> *eigenvalues, std::vector<std::vector<double>> *eigenvectors);

            static double scalar_product(const std::vector<double> &v1, const std::vector<double> &v2);

            double m_center_of_mass_x = 0;
            double m_center_of_mass_y = 0;

            std::vector<std::vector<double>> m_covariance_eigen_vectors;
            std::vector<double> m_covariance_eigen_values;


    };
}