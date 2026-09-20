#pragma once

#include "../headers/PixelType.h"

#include "../headers/FrameScore.h"

#include <string>
#include <memory>
#include <vector>
#include <array>

namespace AstroPhotoStacker {
    /**
     * @class AlignmentResultBase
     * @brief Base class for alignment results.
     *
     * The AlignmentResultBase class serves as a base for storing alignment results.
     */
    class AlignmentResultBase {
        public:
            /**
             * @brief Default constructor for the AlignmentResultBase class.
             */
            AlignmentResultBase() = default;

            AlignmentResultBase(const AlignmentResultBase &other) = default;

            /**
             * @brief Virtual destructor for the AlignmentResultBase class.
             */
            virtual ~AlignmentResultBase() = default;

            virtual void transform_from_reference_to_shifted_frame(float *x, float *y) const = 0;

            virtual void transform_to_reference_frame(float *x, float *y) const = 0;

            virtual std::string get_description_string() const;

            virtual std::string get_method_specific_description_string() const = 0;

            virtual float get_local_score(float x, float y) const {
                return 1;
            };

            virtual bool has_local_scores() const {
                return false;
            };

            void set_frame_score(const FrameScore &frame_score) {
                m_frame_score = frame_score;
            };

            void set_frame_score(const std::string &frame_score) {
                m_frame_score = FrameScore(frame_score);
            };

            const FrameScore& get_frame_score() const {
                return m_frame_score;
            };

            // for backward compatibility (to be removed later)
            void set_ranking_score(float ranking_score) {
                m_frame_score.stars_excentricity = ranking_score;
            };

            // for backward compatibility (to be removed later)
            float get_ranking_score() const {
                return m_frame_score.stars_excentricity;
            };


            void set_is_valid(bool is_valid) {
                m_is_valid = is_valid;
            };

            bool is_valid() const {
                return m_is_valid;
            };

            inline static const std::string s_type_separator = " | ";

            static std::array<std::string, 3> split_type_and_description_and_score(const std::string &description_string);

            virtual std::unique_ptr<AlignmentResultBase> clone() const = 0;

            virtual const std::string& get_type_name() const  = 0;

            virtual void draw_on_image(std::vector<std::vector<PixelType>> *image_data, int width, int height, bool image_in_reference_frame = false) const {};

        protected:
            void copy_base_data(const AlignmentResultBase &other) {
                m_frame_score = other.m_frame_score;
                m_is_valid = other.m_is_valid;
            };

            FrameScore   m_frame_score;
            bool    m_is_valid = false;

    };
}