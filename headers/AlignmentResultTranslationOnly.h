#pragma once

#include "../headers/AlignmentResultBase.h"

#include <memory>

namespace AstroPhotoStacker {
    /**
     * @class AlignmentResultTranslationOnly
     * @brief Class for storing alignment results based on translation only (no rotation).
     */
    class AlignmentResultTranslationOnly : public AlignmentResultBase {
        public:
            /**
             * @brief Default constructor for the AlignmentResultTranslationOnly class.
             * */
            AlignmentResultTranslationOnly();

            AlignmentResultTranslationOnly(const std::string &description_string);

            AlignmentResultTranslationOnly( float shift_x,
                                            float shift_y);

            AlignmentResultTranslationOnly(const AlignmentResultTranslationOnly &other);

            /**
             * @brief Virtual destructor for the AlignmentResultTranslationOnly class.
             */
            virtual ~AlignmentResultTranslationOnly() = default;

            virtual void transform_from_reference_to_shifted_frame(float *x, float *y) const override;

            virtual void transform_to_reference_frame(float *x, float *y) const override;

            virtual std::string get_method_specific_description_string() const override;

            void get_shift( float *shift_x,
                            float *shift_y) const;

            void set_shift( float shift_x,
                            float shift_y);

            inline static const std::string s_type_name = "translation_only";

            virtual std::unique_ptr<AlignmentResultBase> clone() const override {
                return std::make_unique<AlignmentResultTranslationOnly>(*this);
            };

            virtual const std::string& get_type_name() const override {
                return s_type_name;
            };

        protected:
            const std::string c_separator_in_description = " | ";

            float m_shift_x = 0.0f;
            float m_shift_y = 0.0f;
    };
}
