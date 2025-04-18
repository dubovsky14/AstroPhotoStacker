#pragma once

#include "../headers/StarFinder.h"
#include "../headers/KDTree.h"
#include "../headers/PlateSolver.h"
#include "../headers/ReferencePhotoHandlerBase.h"
#include "../headers/AlignmentWindow.h"

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
            ReferencePhotoHandlerPlanetary(const ReferencePhotoHandlerPlanetary&) = delete;

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param input_frame - path to the raw file
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerPlanetary(const InputFrame &input_frame, float threshold_fraction);

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param width - width of the photo
             * @param height - height of the photo
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerPlanetary(const unsigned short *brightness, int width, int height, float threshold_fraction);

            /**
             * @brief Calculate how the photo should be rotated and shifted to match the reference photo
             *
             * @param file_address - path to the file to be plate-solved
             * @param ranking - pointer to the variable where the ranking of the plate will be stored
             *
             * @return PlateSolvingResult
            */
            virtual PlateSolvingResult calculate_alignment(const InputFrame &input_frame, float *ranking = nullptr) const override;

        protected:
            ReferencePhotoHandlerPlanetary() : ReferencePhotoHandlerBase() {};

            /**
             * @brief Get coordinates of the window where the alignment should be calculated - surrounding the planet
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param width - width of the photo
             * @param height - height of the photo
             * @param threshold - threshold value for the pixels
             * @return AlignmentWindow - coordinates of the window x0, y0, x1, y1
            */
            AlignmentWindow get_alignment_window(const MonochromeImageData &image_data, unsigned short threshold) const;

            std::vector<std::vector<double>> get_covariance_matrix(const MonochromeImageData &image_data, const std::tuple<double,double> &center_of_mass, unsigned short threshold, const AlignmentWindow &window_coordinates) const;

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
            std::tuple<double,double> get_center_of_mass(const MonochromeImageData &image_data, unsigned short threshold, const AlignmentWindow &window_coordinates) const;

            std::tuple<float,float,std::vector<std::vector<double>>,std::vector<double>> get_center_of_mass_eigenvectors_and_eigenvalues(const MonochromeImageData &image_data, float threshold_fraction, AlignmentWindow *window_coordinates = nullptr) const;

            virtual void initialize(const unsigned short *brightness, int width, int height, float threshold_fraction) override;

            static void calculate_eigenvectors_and_eigenvalues(const std::vector<std::vector<double>> &covariance_matrix, std::vector<double> *eigenvalues, std::vector<std::vector<double>> *eigenvectors);

            double m_center_of_mass_x = 0;
            double m_center_of_mass_y = 0;
            float  m_threshold_fraction = 0.5;

            std::vector<std::vector<double>> m_covariance_eigen_vectors;
            std::vector<double> m_covariance_eigen_values;

            AlignmentWindow m_alignment_window;


    };
}