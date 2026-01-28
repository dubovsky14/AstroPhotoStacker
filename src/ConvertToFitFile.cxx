#include "../headers/ConvertToFitFile.h"

#include "../headers/FitFileSaver.h"
#include "../headers/InputFrameReader.h"
#include "../headers/MetadataReader.h"

#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;

void AstroPhotoStacker::convert_to_fit_file(const InputFrame &input_frame, const std::string &output_file, int output_bit_depth)   {
    int width, height;
    InputFrameReader input_frame_reader(input_frame);
    input_frame_reader.get_photo_resolution(&width, &height);
    vector<PixelType> image_data = input_frame_reader.get_raw_data();
    const Metadata metadata_reader = read_metadata(input_frame);

    if (output_bit_depth == 16)    {
        std::transform(image_data.begin(), image_data.end(), image_data.begin(), [](PixelType value) {
            return value * 2;
        });
    }
    else if (output_bit_depth == 8)    {
        std::transform(image_data.begin(), image_data.end(), image_data.begin(), [](PixelType value) {
            return static_cast<PixelType>(value / 128);
        });
    }

    FitFileSaver fit_file_saver(width, height);
    fit_file_saver.set_metadata(metadata_reader);
    fit_file_saver.set_bits_per_pixel(output_bit_depth);
    fit_file_saver.save(output_file, image_data);
};
