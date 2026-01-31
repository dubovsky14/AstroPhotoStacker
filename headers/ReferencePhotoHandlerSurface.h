#pragma once

#include "../headers/ReferencePhotoHandlerBase.h"
#include "../headers/LocalShift.h"
#include "../headers/ThreadSafeCacheSystem.h"

#include <opencv2/opencv.hpp>

#include <memory>
#include <string>
#include <vector>
#include <tuple>


namespace AstroPhotoStacker   {

    /**
     * @brief Class responsible for handling the reference photo, providing methods for alignement for solar system photos, where effects of seeing are important
     */
    class ReferencePhotoHandlerSurface : public ReferencePhotoHandlerBase {
        public:

            friend class ReferencePhotoHandlerFactory;

            ReferencePhotoHandlerSurface(const ReferencePhotoHandlerSurface&) = delete;

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param reference_frame    - reference frame - local shifts will be calculated to match this frame
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerSurface(const InputFrame &reference_frame, const ConfigurableAlgorithmSettingsMap &configuration_map = ConfigurableAlgorithmSettingsMap());

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param width - width of the photo
             * @param height - height of the photo
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerSurface(const PixelType *brightness, int width, int height, const ConfigurableAlgorithmSettingsMap &configuration_map = ConfigurableAlgorithmSettingsMap());


            virtual std::unique_ptr<AlignmentResultBase> calculate_alignment(const InputFrame &input_frame) const override;

        protected:
            ReferencePhotoHandlerSurface() : ReferencePhotoHandlerBase() { define_configuration_settings(); };

            void initialize_reference_features(const PixelType *brightness_original);

            void get_keypoints_and_descriptors(const PixelType *brightness, int width, int height, std::vector<cv::KeyPoint> *keypoints, cv::Mat *descriptors) const;

            virtual void initialize(const PixelType *brightness, int width, int height, const ConfigurableAlgorithmSettingsMap &configuration_map = ConfigurableAlgorithmSettingsMap()) override;

            virtual void define_configuration_settings() override;

            double m_gaussian_sigma = 6.0;
            float m_threshold_fraction = 0.0005;

            std::vector<cv::KeyPoint> m_reference_keypoints;
            cv::Mat                   m_reference_descriptors;
    };
}