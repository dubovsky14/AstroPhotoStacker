#pragma once

#include "../headers/ReferencePhotoHandlerPlanetaryZeroRotation.h"
#include "../headers/AlignmentPointBoxGrid.h"
#include "../headers/LocalShift.h"

#include <memory>
#include <string>
#include <vector>
#include <tuple>


namespace AstroPhotoStacker   {

    /**
     * @brief Class responsible for handling the reference photo, providing methods for alignement for solar system photos, where effects of seeing are important
     */
    class ReferencePhotoHandlerSurface : public ReferencePhotoHandlerPlanetaryZeroRotation {
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

            std::vector<LocalShift> get_local_shifts(   const InputFrame &input_frame,
                                                        const PlateSolvingResult &plate_solving_result) const;

            const std::vector<AlignmentPointBox> &get_alignment_boxes() const {return m_alignment_point_box_grid->get_alignment_boxes();};

        protected:

            std::unique_ptr<AlignmentPointBoxGrid> m_alignment_point_box_grid = nullptr;

            void initialize_alignment_grid(const PixelType *brightness_original);

            float m_blur_sigma = 1.;
            int   m_blur_window_size = 5;
    };
}