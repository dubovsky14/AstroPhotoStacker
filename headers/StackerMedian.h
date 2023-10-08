#include "../headers/StackerBase.h"

#include <vector>
#include <memory>

namespace AstroPhotoStacker {
    class StackerMedian : public StackerBase {
        public:
            StackerMedian(int number_of_colors, int width, int height);

            virtual void calculate_stacked_photo() override;

        protected:
            std::vector<std::vector<unsigned short>> m_number_of_stacked_pixels;
            std::vector<std::unique_ptr<short[]>> m_values_to_stack;

            void add_photo_to_stack(unsigned int file_index, int y_min, int y_max);

            int get_height_range_limit()    const;
    };
}