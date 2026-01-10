#pragma once

#include "../headers/StarFinder.h"
#include "../headers/KDTree.h"
#include "../headers/PlateSolver.h"
#include "../headers/ReferencePhotoHandlerBase.h"
#include "../headers/AlignmentResultPlateSolving.h"

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
             * @param input_frame - input frame data
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerStars(const InputFrame &input_frame, float threshold_fraction = 0.0005, bool variable_zoom = false);

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param width - width of the photo
             * @param height - height of the photo
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerStars(const PixelType *brightness, int width, int height, float threshold_fraction = 0.0005, bool variable_zoom = false)  :
                ReferencePhotoHandlerBase(brightness, width, height, threshold_fraction) {
                m_variable_zoom = variable_zoom;
                initialize(brightness, width, height, threshold_fraction);
            };

            /**
             * @brief Get the number of precalculated hashes
             *
             * @return unsigned int - number of hashes
            */
            unsigned int get_number_of_hashes() const { return m_kd_tree->get_n_nodes(); };

            /**
             * @brief Calculate how the photo should be rotated and shifted to match the reference photo
             *
             * @param file_address - path to the file to be plate-solved
             * @param ranking - pointer to the variable where the ranking of the plate will be stored
             *
             * @return std::unique_ptr<AlignmentResultBase>
            */
            virtual std::unique_ptr<AlignmentResultBase> calculate_alignment(const InputFrame &input_frame) const override;

            /**
             * @brief Plate-solve a photo - calculate how it should be rotated and shifted to match the reference photo
             *
             * @param std::vector<std::tuple<float, float, int> > stars - x-coordinate, y-coordinate, number of pixels forming the star
             *
             * @return std::unique_ptr<AlignmentResultBase>
            */
            std::unique_ptr<AlignmentResultPlateSolving> plate_solve(const std::vector<std::tuple<float, float, int> > &stars) const;

        protected:

            std::vector<std::tuple<float, float, int> > m_stars;
            std::unique_ptr<KDTree<float, 4, std::tuple<unsigned, unsigned, unsigned, unsigned>>> m_kd_tree = nullptr;
            std::unique_ptr<PlateSolver> m_plate_solver = nullptr;
            bool m_variable_zoom = false;

            int m_minimal_number_of_pixels_per_star = -1;

            virtual void initialize(const PixelType *brightness, int width, int height, float threshold_fraction = 0.0005) override;

            void initialize(const std::vector<std::tuple<float, float, int> > &stars, int width, int height);

            void calculate_and_store_hashes();

    };
}