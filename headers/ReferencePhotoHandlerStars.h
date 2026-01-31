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
            friend class ReferencePhotoHandlerFactory;

            ReferencePhotoHandlerStars(const ReferencePhotoHandlerStars&) = delete;

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param input_frame - input frame data
             * @param configuration_map - values of free parameters of the algorithm
            */
            ReferencePhotoHandlerStars(const InputFrame &input_frame, const ConfigurableAlgorithmSettingsMap &configuration_map = ConfigurableAlgorithmSettingsMap());

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param width - width of the photo
             * @param height - height of the photo
             * @param configuration_map - values of free parameters of the algorithm
            */
            ReferencePhotoHandlerStars(const PixelType *brightness, int width, int height, const ConfigurableAlgorithmSettingsMap &configuration_map = ConfigurableAlgorithmSettingsMap())  :
                ReferencePhotoHandlerBase(brightness, width, height, configuration_map) {
                initialize(brightness, width, height, configuration_map);
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
            ReferencePhotoHandlerStars() : ReferencePhotoHandlerBase() { define_configuration_settings(); };

            virtual void define_configuration_settings() override;


            std::vector<std::tuple<float, float, int> > m_stars;
            std::unique_ptr<KDTree<float, 4, std::tuple<unsigned, unsigned, unsigned, unsigned>>> m_kd_tree = nullptr;
            std::unique_ptr<PlateSolver> m_plate_solver = nullptr;
            bool m_variable_zoom = false;

            int m_minimal_number_of_pixels_per_star = -1;
            float m_threshold_fraction = 0.0005;

            virtual void initialize(const PixelType *brightness, int width, int height, const ConfigurableAlgorithmSettingsMap &configuration_map) override;

            void initialize(const std::vector<std::tuple<float, float, int> > &stars, int width, int height);

            void calculate_and_store_hashes();

    };
}