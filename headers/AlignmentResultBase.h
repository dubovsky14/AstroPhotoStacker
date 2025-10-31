#pragma once

#include <string>
#include <memory>

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

            virtual std::unique_ptr<AlignmentResultBase> clone() const = 0;

            virtual const std::string& get_type_name() const  = 0;

        protected:
            void copy_base_data(const AlignmentResultBase &other) {
                m_ranking_score = other.m_ranking_score;
                m_is_valid = other.m_is_valid;
            };

            float   m_ranking_score = 0.0f;
            bool    m_is_valid = false;

    };
}