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
            ReferencePhotoHandlerSurface()                             = delete;
            ReferencePhotoHandlerSurface(const ReferencePhotoHandlerSurface&) = delete;

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param reference_frame    - reference frame - local shifts will be calculated to match this frame
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerSurface(const InputFrame &reference_frame, float threshold_fraction);

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param width - width of the photo
             * @param height - height of the photo
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerSurface(const PixelType *brightness, int width, int height, float threshold_fraction);

            std::vector<LocalShift> get_local_shifts(const InputFrame &input_frame) const;


            const std::vector<std::pair<float,float>> get_alignment_points() const;

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
            void initialize_reference_features(const PixelType *brightness_original);

            void get_keypoints_and_descriptors(const PixelType *brightness, int width, int height, std::vector<cv::KeyPoint> *keypoints, cv::Mat *descriptors) const;

            virtual void initialize(const PixelType *brightness, int width, int height, float threshold_fraction = 0.0005) override;


            float m_blur_sigma = 1.;
            int   m_blur_window_size = 5;
            float m_threshold_fraction = 0.0005;

            mutable ThreadSafeCacheSystem<InputFrame, std::tuple<std::vector<LocalShift>, PlateSolvingResult, float>> m_local_shifts_cache;

            std::tuple<std::vector<LocalShift>, PlateSolvingResult, float> compute_local_shifts_and_alignment(const InputFrame &input_frame) const;

            std::vector<cv::KeyPoint> m_reference_keypoints;
            cv::Mat                   m_reference_descriptors;
    };
}