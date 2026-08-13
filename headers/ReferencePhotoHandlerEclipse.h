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
#include <utility>
#include <set>

namespace AstroPhotoStacker   {

    /**
     * @brief Class responsible for handling the reference photo, providing methods for alignement of planetary photos
     */
    class ReferencePhotoHandlerEclipse : public ReferencePhotoHandlerBase {
        public:
            friend class ReferencePhotoHandlerFactory;

            ReferencePhotoHandlerEclipse(const ReferencePhotoHandlerEclipse&) = delete;

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param input_frame - path to the raw file
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerEclipse(const InputFrame &input_frame, const ConfigurableAlgorithmSettingsMap &configuration_map = ConfigurableAlgorithmSettingsMap());

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param width - width of the photo
             * @param height - height of the photo
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerEclipse(const PixelType *brightness, int width, int height, const ConfigurableAlgorithmSettingsMap &configuration_map = ConfigurableAlgorithmSettingsMap());

            /**
             * @brief Calculate how the photo should be rotated and shifted to match the reference photo
             *
             * @param file_address - path to the file to be plate-solved
             * @param ranking - pointer to the variable where the ranking of the plate will be stored
             *
             * @return std::unique_ptr<AlignmentResultBase>
            */
            virtual std::unique_ptr<AlignmentResultBase> calculate_alignment(const InputFrame &input_frame) const override;

        protected:
            ReferencePhotoHandlerEclipse() : ReferencePhotoHandlerBase() { define_configuration_settings(); };

            virtual void initialize(const PixelType *brightness, int width, int height, const ConfigurableAlgorithmSettingsMap &configuration_map)   override;

            virtual void define_configuration_settings() override;

            std::tuple<float,float,float> get_center_coordinates_and_radius(const MonochromeImageData &image_data) const;

            static std::vector<unsigned char> threshold_image(const MonochromeImageData &image_data, PixelType threshold);

            std::vector<std::pair<int,int>> get_edge_pixels(const std::vector<unsigned char> &binary_image, int width, int height, int minimal_number_of_neighbors_outside = 3) const;

            double m_center_x = 0;
            double m_center_y = 0;
            double m_radius = 0;
            float  m_threshold_fraction = 0.5;

            double m_gaussian_sigma = 6.0;
            bool   m_use_number_of_pixels_above_otsu_threshold_for_ranking = false;
            bool   m_zero_rotation = true;


    };
}