#pragma once

#include <string>

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

            std::string get_description_string(const std::string &type_name) const;

            virtual std::string get_method_specific_description_string() const = 0;

            void set_ranking_score(float ranking_score) {
                m_ranking_score = ranking_score;
            };

            float get_ranking_score() const {
                return m_ranking_score;
            };

            void set_is_valid(bool is_valid) {
                m_is_valid = is_valid;
            };

            bool is_valid() const {
                return m_is_valid;
            };

            inline static const std::string s_type_separator = " | ";

            static std::pair<std::string, std::string> split_type_and_description(const std::string &description_string);

        protected:
            float   m_ranking_score = 0.0f;
            bool    m_is_valid = false;

    };
}