#pragma once

#include "../headers/StackerKappaSigmaBase.h"

namespace AstroPhotoStacker {
    class StackerKappaSigmaClipping : public StackerKappaSigmaBase {
        public:
                StackerKappaSigmaClipping(int number_of_colors, int width, int height);

        protected:
                virtual void process_line(int y_index_final_array, int y_index_values_to_stack_array, int i_color) override;
    };
}