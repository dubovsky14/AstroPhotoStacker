#include "../headers/ConvertToFitFile.h"

#include "../headers/FitFileSaver.h"
#include "../headers/raw_file_reader.h"
#include "../headers/MetadataReader.h"

using namespace AstroPhotoStacker;
using namespace std;

void AstroPhotoStacker::convert_to_fit_file(const InputFrame &input_frame, const std::string &output_file, int output_bit_depth)   {
    int width, height;
    vector<unsigned short> image_data = read_raw_file(input_frame, &width, &height);
    const Metadata metadata_reader = read_metadata(input_frame);

    if (output_bit_depth == 16)    {
        if(input_frame.is_video_frame())    {
            std::transform(image_data.begin(), image_data.end(), image_data.begin(), [](unsigned short value) {
                return value * 256;
            });
        }
        else   {
            std::transform(image_data.begin(), image_data.end(), image_data.begin(), [](unsigned short value) {
                return value * 2;
            });
        }
    }

    FitFileSaver fit_file_saver(width, height);
    fit_file_saver.set_metadata(metadata_reader);
    fit_file_saver.set_bits_per_pixel(output_bit_depth);
    fit_file_saver.save(output_file, image_data);
};
