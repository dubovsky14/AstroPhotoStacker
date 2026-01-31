#pragma once

#include "../headers/ReferencePhotoHandlerStars.h"

#include <string>
#include <vector>
#include <tuple>
#include <map>

namespace AstroPhotoStacker   {

    /**
     * @brief Class responsible for handling the reference photo of a comet. It provides also methods for plate-solving another photo - how it should be rotated and shifted to match the reference photo
    */
    class ReferencePhotoHandlerComet : public ReferencePhotoHandlerStars {
        public:
            friend class ReferencePhotoHandlerFactory;

            ReferencePhotoHandlerComet(const ReferencePhotoHandlerComet&) = delete;

            /**
             * @brief Construct a new Reference Photo Handler object
             *
             * @param input_frame - input frame data
             * @param threshold_fraction - fraction of the brightest pixels that will be considered as stars
            */
            ReferencePhotoHandlerComet(const InputFrame &input_frame, const ConfigurableAlgorithmSettingsMap &configuration_map = ConfigurableAlgorithmSettingsMap());

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
             * @brief Add comet position in the given photo
             *
             * @param input_frame - input frame data
             * @param x - x position of the comet in pixel coordinates of this photo
             * @param y - y position of the comet in pixel coordinates of this photo
             */
            bool add_comet_position(const InputFrame &input_frame, float x, float y);

            /**
             * @brief Fit the comet path through the given positions, use linear least squares fitting
             *
             * @return true if the fitting was successful, false otherwise
            */
            bool fit_comet_path();

        protected:
            ReferencePhotoHandlerComet() : ReferencePhotoHandlerStars() { define_configuration_settings(); };

            InputFrame  m_reference_input_frame;
            std::pair<float,float> calculate_expected_comet_position(int timestamp) const;

            std::map<InputFrame, std::pair<float, float>> m_comet_positions; // comet positions in pixel coordinates of the reference photo

            int m_minimal_timestamp = 0;
            std::pair<double,double> m_comet_velocity = std::make_pair(0.0, 0.0); // in pixels per second
            std::pair<double,double> m_comet_initial_position = std::make_pair(0.0, 0.0); // in pixels at time = 0

            std::pair<float,float> m_comet_position_reference_frame = std::make_pair(-1.0f, -1.0f);

    };
}