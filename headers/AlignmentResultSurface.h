#pragma once

#include "../headers/AlignmentResultBase.h"
#include "../headers/GeometricTransformations.h"
#include "../headers/LocalShift.h"
#include "../headers/LocalShiftsHandler.h"

#include <memory>

namespace AstroPhotoStacker {
    /**
     * @class AlignmentResultSurface
     * @brief Class for storing alignment results based on calculating local shifts.
     */
    class AlignmentResultSurface : public AlignmentResultBase {
        public:
            /**
             * @brief Default constructor for the AlignmentResultSurface class.
             * */
            AlignmentResultSurface();

            AlignmentResultSurface(const std::string &description_string);

            AlignmentResultSurface( float shift_x,
                                    float shift_y,
                                    float rotation_center_x,
                                    float rotation_center_y,
                                    float rotation,
                                    const std::vector<LocalShift> &local_shifts);

            AlignmentResultSurface(const AlignmentResultSurface &other);

            /**
             * @brief Virtual destructor for the AlignmentResultSurface class.
             */
            virtual ~AlignmentResultSurface() = default;

            virtual void transform_from_reference_to_shifted_frame(float *x, float *y) const override;

            virtual void transform_to_reference_frame(float *x, float *y) const override;

            virtual std::string get_method_specific_description_string() const override;

            void set_parameters(float shift_x,
                                float shift_y,
                                float rotation_center_x,
                                float rotation_center_y,
                                float rotation,
                                const std::vector<LocalShift> &local_shifts);

            inline static const std::string s_type_name = "surface";

        protected:
            std::unique_ptr<GeometricTransformer> m_geometric_transformer = nullptr;
            std::unique_ptr<LocalShiftsHandler> m_local_shifts_handler = nullptr;

            const std::string c_separator_in_description = " | ";
    };
}
