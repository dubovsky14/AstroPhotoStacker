#pragma once

#include "../headers/ReferencePhotoHandlerPlanetary.h"
#include "../headers/AlignmentPointBoxGrid.h"

#include <memory>
#include <string>
#include <vector>
#include <tuple>


namespace AstroPhotoStacker   {

    /**
     * @brief Class responsible for handling the reference photo, providing methods for alignement of planetary photos
     */
    class ReferencePhotoHandlerSurface : public ReferencePhotoHandlerPlanetary {
        public:
            ReferencePhotoHandlerSurface()                             = delete;
            ReferencePhotoHandlerSurface(const ReferencePhotoHandlerSurface&) = delete;

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param raw_file_address - path to the raw file
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerSurface(const std::string &raw_file_address, float threshold_fraction);

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param brightness - pointer to the array containing the brightness of the pixels
             * @param width - width of the photo
             * @param height - height of the photo
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerSurface(const unsigned short *brightness, int width, int height, float threshold_fraction);

            std::vector<std::tuple<int,int,int,int,bool>> get_local_shifts( const std::string &file_address,
                                                                            float shift_x,
                                                                            float shift_y,
                                                                            float rotation_center_x,
                                                                            float rotation_center_y,
                                                                            float rotation) const;

        protected:

            std::unique_ptr<AlignmentPointBoxGrid> m_alignment_point_box_grid = nullptr;

            void initialize_alignment_grid(const unsigned short *brightness_original);

            float m_blur_sigma = 1.;
            int   m_blur_window_size = 5;
    };
}