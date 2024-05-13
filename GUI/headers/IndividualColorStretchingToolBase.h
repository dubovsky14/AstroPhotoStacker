#pragma once

class IndividualColorStretchingToolBase   {
    public:
        IndividualColorStretchingToolBase() = default;

        virtual ~IndividualColorStretchingToolBase() = default;

        /**
         * @brief Stretch the value
         *
         * @param value the value to stretch
         * @return float the stretched value
        */
        virtual float stretch(float value, float full_scale_max) const  = 0;
};