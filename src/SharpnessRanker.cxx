#include "../headers/SharpnessRanker.h"

using namespace AstroPhotoStacker;


float AstroPhotoStacker::get_sharpness_for_file(const InputFrame &input_frame, const AlignmentWindow &alignment_window)    {
    CalibratedPhotoHandler calibrated_photo_handler(input_frame, true);
    calibrated_photo_handler.define_alignment(0, 0, 0, 0, 0);
    calibrated_photo_handler.calibrate();

    const std::vector<std::vector<PixelType>> &data = calibrated_photo_handler.get_calibrated_data_after_color_interpolation();
    const int width = calibrated_photo_handler.get_width();
    const int height = calibrated_photo_handler.get_height();

    float average_sharpness = 0;
    for (unsigned int i_color = 0; i_color < data.size(); i_color++)    {
        const float sharpness = AstroPhotoStacker::get_sharpness_factor(data[i_color].data(), width, height, alignment_window);
        average_sharpness += sharpness;
    }
    average_sharpness /= data.size();

    return average_sharpness;
}
