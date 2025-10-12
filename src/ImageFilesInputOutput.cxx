#include "../headers/ImageFilesInputOutput.h"

#include "../headers/raw_file_reader.h"

#include <opencv2/opencv.hpp>
#include <string>



bool AstroPhotoStacker::get_photo_resolution(const InputFrame &input_frame, int *width, int *height) {
    const bool raw_file = is_raw_file(input_frame.get_file_address());
    if (raw_file) {
        return get_photo_resolution_raw_file(input_frame.get_file_address(), width, height);
    }
    else {
        read_rgb_image<unsigned short>(input_frame, width, height);
        return true;
    }
    return false;
};

std::vector<short> AstroPhotoStacker::read_image_monochrome(const InputFrame &input_frame, int *width, int *height)    {
    InputFrameReader reader(input_frame);
    reader.load_input_frame_data();
    reader.get_photo_resolution(width, height);
    return reader.get_monochrome_data();
};