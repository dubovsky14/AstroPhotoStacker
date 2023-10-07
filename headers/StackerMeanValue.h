#include "../headers/StackerBase.h"

#include <vector>

namespace AstroPhotoStacker {
    class StackerMeanValue : public StackerBase {
        public:
            StackerMeanValue(int number_of_colors, int width, int height);

            virtual void calculate_stacked_photo() override;

            void save_stacked_photo_as_png(const std::string &file_address) const;

        protected:
            std::vector<std::vector<unsigned short>> m_number_of_stacked_pixels;

            void add_photo_to_stack(const std::string &file_address);
    };
}