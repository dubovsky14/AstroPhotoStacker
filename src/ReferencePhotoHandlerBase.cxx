#include "../headers/ReferencePhotoHandlerBase.h"
#include "../headers/InputFrameReader.h"

using namespace AstroPhotoStacker;
using namespace std;

std::vector<PixelType> ReferencePhotoHandlerBase::read_image_monochrome(const InputFrame &input_frame, int *width, int *height) const {
    InputFrameReader input_frame_reader(input_frame);
    *width = input_frame_reader.get_width();
    *height = input_frame_reader.get_height();
    return input_frame_reader.get_monochrome_data();
};