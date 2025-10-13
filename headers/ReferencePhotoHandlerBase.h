#pragma once

#include "../headers/StarFinder.h"
#include "../headers/PixelType.h"
#include "../headers/KDTree.h"
#include "../headers/PlateSolver.h"
#include "../headers/MonochromeImageData.h"
#include "../headers/InputFrame.h"
#include "../headers/PlateSolvingResult.h"

#include <memory>
#include <string>
#include <vector>
#include <tuple>

namespace AstroPhotoStacker   {
    /**
     * @brief Class responsible for handling the reference photo. It provides also methods for plate-solving another photo - how it should be rotated and shifted to match the reference photo
    */
    class ReferencePhotoHandlerBase {
        public:
            ReferencePhotoHandlerBase(const ReferencePhotoHandlerBase&) = delete;

            virtual ~ReferencePhotoHandlerBase()    {};

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param input_frame - input frame data
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerBase(const InputFrame& input_frame, float threshold_fraction = 0.0005)   {};

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param width - width of the photo
             * @param height - height of the photo
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerBase(const PixelType *brightness, int width, int height, float threshold_fraction = 0.0005)    {

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
             * @brief Calculate how the photo should be rotated and shifted to match the reference photo
             *
             * @param file_address - path to the file to be plate-solved
             * @param ranking - pointer to the variable where the ranking of the plate will be stored
             *
             * @return PlateSolvingResult
            */
            virtual PlateSolvingResult calculate_alignment(const InputFrame &input_frame, float *ranking = nullptr) const = 0;

        protected:
            ReferencePhotoHandlerBase() {};

            int m_width;
            int m_height;

            virtual void initialize(const PixelType *brightness, int width, int height, float threshold_fraction = 0.0005) = 0;

            std::vector<PixelType>  read_image_monochrome(const InputFrame &input_frame, int *width, int *height)   const;

    };
}