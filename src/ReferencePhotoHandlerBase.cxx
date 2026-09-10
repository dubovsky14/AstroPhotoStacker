#include "../headers/ReferencePhotoHandlerBase.h"
#include "../headers/InputFrameReader.h"

#include <iostream>

using namespace AstroPhotoStacker;
using namespace std;

std::vector<PixelType> ReferencePhotoHandlerBase::read_image_monochrome(const InputFrame &input_frame, int *width, int *height) const {
    InputFrameReader input_frame_reader(input_frame);
    *width = input_frame_reader.get_width();
    *height = input_frame_reader.get_height();
    vector<PixelType> monochrome_data = input_frame_reader.get_monochrome_data();
    if (m_lens_correction_tool) {
        monochrome_data = m_lens_correction_tool->get_undistorted_image(monochrome_data, *width, *height);

        // now let's fix empty pixels if any
        for (PixelType &x : monochrome_data) {
            if (x < 0)  {
                x = 0;
            }
        }
    }
    return monochrome_data;
};
