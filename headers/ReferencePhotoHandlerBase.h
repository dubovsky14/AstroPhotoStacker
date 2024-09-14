#pragma once

#include "../headers/StarFinder.h"
#include "../headers/KDTree.h"
#include "../headers/PlateSolver.h"

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
             * @param raw_file_address - path to the raw file
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerBase(const std::string &raw_file_address, float threshold_fraction = 0.0005)   {};

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param width - width of the photo
             * @param height - height of the photo
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerBase(const unsigned short *brightness, int width, int height, float threshold_fraction = 0.0005)    {

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
             * @param shift_x - pointer to the variable where the horizontal shift will be stored
             * @param shift_y - pointer to the variable where the vertical shift will be stored
             * @param rot_center_x - pointer to the variable where the x coordinate of the rotation center will be stored
             * @param rot_center_y - pointer to the variable where the y coordinate of the rotation center will be stored
             * @param rotation - pointer to the variable where the rotation angle will be stored
             * @return true - if the plate was solved
             * @return false - if the plate was not solved
            */
            virtual bool calculate_alignment(const std::string &file_address, float *shift_x, float *shift_y, float *rot_center_x, float *rot_center_y, float *rotation) const = 0;

        protected:
            ReferencePhotoHandlerBase() {};

            int m_width;
            int m_height;

            virtual void initialize(const unsigned short *brightness, int width, int height, float threshold_fraction = 0.0005) = 0;

    };
}