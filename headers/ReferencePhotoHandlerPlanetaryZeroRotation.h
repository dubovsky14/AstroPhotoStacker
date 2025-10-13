#pragma once

#include "../headers/StarFinder.h"
#include "../headers/KDTree.h"
#include "../headers/PlateSolver.h"
#include "../headers/ReferencePhotoHandlerPlanetary.h"

#include <memory>
#include <string>
#include <vector>
#include <tuple>

namespace AstroPhotoStacker   {

    /**
     * @brief Class responsible for handling the reference photo, providing methods for alignement of planetary photos, without trying to fit the rotation angle
     */
    class ReferencePhotoHandlerPlanetaryZeroRotation : public ReferencePhotoHandlerPlanetary {
        public:
            ReferencePhotoHandlerPlanetaryZeroRotation(const ReferencePhotoHandlerPlanetaryZeroRotation&) = delete;

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param input_frame - info about the light frame
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerPlanetaryZeroRotation(const InputFrame &input_frame, float threshold_fraction);

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param width - width of the photo
             * @param height - height of the photo
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerPlanetaryZeroRotation(const PixelType *brightness, int width, int height, float threshold_fraction);

             /**
             * @brief Calculate how the photo should be rotated and shifted to match the reference photo
             *
             * @param file_address - path to the file to be plate-solved
             * @param ranking - pointer to the variable where the ranking of the plate will be stored
             *
             * @return PlateSolvingResult
            */
           virtual PlateSolvingResult calculate_alignment(const InputFrame &input_frame, float *ranking = nullptr) const override;
    };
}