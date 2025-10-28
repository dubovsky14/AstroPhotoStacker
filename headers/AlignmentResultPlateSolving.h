#pragma once

#include "../headers/AlignmentResultBase.h"
#include "../headers/GeometricTransformations.h"

#include <memory>

namespace AstroPhotoStacker {
    /**
     * @class AlignmentResultPlateSolving
     * @brief Class for storing alignment results based on plate solving, without scale transformation.
     */
    class AlignmentResultPlateSolving : public AlignmentResultBase {
        public:
            /**
             * @brief Default constructor for the AlignmentResultPlateSolving class.
             * */
            AlignmentResultPlateSolving();

            AlignmentResultPlateSolving(const std::string &description_string);

            AlignmentResultPlateSolving(float shift_x,
                                        float shift_y,
                                        float rotation_center_x,
                                        float rotation_center_y,
                                        float rotation);

            AlignmentResultPlateSolving(const AlignmentResultPlateSolving &other);

            /**
             * @brief Virtual destructor for the AlignmentResultPlateSolving class.
             */
            virtual ~AlignmentResultPlateSolving() = default;

            virtual void transform_from_reference_to_shifted_frame(float *x, float *y) const override;

            virtual void transform_to_reference_frame(float *x, float *y) const override;

            virtual std::string get_method_specific_description_string() const override;

            void set_parameters(float shift_x,
                                float shift_y,
                                float rotation_center_x,
                                float rotation_center_y,
                                float rotation);

            inline static const std::string s_type_name = "plate_solving";

        protected:
            std::unique_ptr<GeometricTransformer> m_geometric_transformer = nullptr;

            const std::string c_separator_in_description = " | ";
    };
}
