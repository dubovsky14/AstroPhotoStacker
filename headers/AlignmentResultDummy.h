#pragma once

#include "../headers/AlignmentResultBase.h"

#include <memory>

namespace AstroPhotoStacker {
    /**
     * @class AlignmentResultDummy
     * @brief Class for storing dummy alignment result (mapping coordinates to themselves).
     */
    class AlignmentResultDummy : public AlignmentResultBase {
        public:
            /**
             * @brief Default constructor for the AlignmentResultDummy class.
             * */
            AlignmentResultDummy()  : AlignmentResultBase() {m_is_valid = false;};

            AlignmentResultDummy(const std::string &description_string) : AlignmentResultBase() {m_is_valid = false;};

            AlignmentResultDummy(const AlignmentResultDummy &other) = default;

            /**
             * @brief Virtual destructor for the AlignmentResultDummy class.
             */
            virtual ~AlignmentResultDummy() = default;

            virtual void transform_from_reference_to_shifted_frame(float *x, float *y) const override {};

            virtual void transform_to_reference_frame(float *x, float *y) const override {};

            virtual std::string get_method_specific_description_string() const override {return s_type_name;};

            inline static const std::string s_type_name = "translation_only";

            virtual std::unique_ptr<AlignmentResultBase> clone() const override {
                return std::make_unique<AlignmentResultDummy>(*this);
            };

            virtual const std::string& get_type_name() const override {
                return s_type_name;
            };

    };
}
