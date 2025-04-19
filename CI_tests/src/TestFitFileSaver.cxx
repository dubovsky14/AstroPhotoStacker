#include "../headers/TestFitFileSaver.h"

#include "../../headers/ConvertToFitFile.h"
#include "../../headers/raw_file_reader.h"
#include "../../headers/MetadataReader.h"

using namespace std;
using namespace AstroPhotoStacker;


TestResult AstroPhotoStacker::test_metadata_match(const Metadata &metadata_original, const Metadata &metadata_fit)  {
    string error_message;

    if (metadata_original.aperture != metadata_fit.aperture)   {
        error_message += "Aperture mismatch: original = " + std::to_string(metadata_original.aperture) + ", fit = " + std::to_string(metadata_fit.aperture) + "\n";
    }
    if (metadata_original.exposure_time != metadata_fit.exposure_time)   {
        error_message += "Exposure time mismatch: original = " + std::to_string(metadata_original.exposure_time) + ", fit = " + std::to_string(metadata_fit.exposure_time) + "\n";
    }
    if (metadata_original.iso != metadata_fit.iso)   {
        error_message += "ISO mismatch: original = " + std::to_string(metadata_original.iso) + ", fit = " + std::to_string(metadata_fit.iso) + "\n";
    }
    if (metadata_original.focal_length != metadata_fit.focal_length)   {
        error_message += "Focal length mismatch: original = " + std::to_string(metadata_original.focal_length) + ", fit = " + std::to_string(metadata_fit.focal_length) + "\n";
    }
    //if (metadata_original.timestamp != metadata_fit.timestamp)   {
    //    error_message += "Timestamp mismatch: original = " + std::to_string(metadata_original.timestamp) + ", fit = " + std::to_string(metadata_fit.timestamp) + "\n";
    //}
    if (metadata_original.bayer_matrix != metadata_fit.bayer_matrix)   {
        error_message += "Bayer matrix mismatch: original = " + metadata_original.bayer_matrix + ", fit = " + metadata_fit.bayer_matrix + "\n";
    }

    if (error_message.empty())   {
        return TestResult(true, "");
    }
    else    {
        return TestResult(false, error_message);
    }
};

TestResult AstroPhotoStacker::test_metadata_fit_file_saver( const InputFrame &input_frame,
                                                            const std::string &output_file,
                                                            int output_bit_depth) {

    string error_message;
    convert_to_fit_file(input_frame, output_file, output_bit_depth);

    int width_original, height_original;
    vector<unsigned short> image_data_original = read_raw_file(input_frame, &width_original, &height_original);
    const Metadata metadata_original = read_metadata(input_frame);


    int width_fit, height_fit;
    vector<unsigned short> image_data_fit = read_raw_file(InputFrame(output_file), &width_fit, &height_fit);
    const Metadata metadata_fit = read_metadata(InputFrame(output_file));

    if (width_original != width_fit)    {
        error_message += "Width mismatch: original = " + std::to_string(width_original) + ", fit = " + std::to_string(width_fit) + "\n";
    }
    if (height_original != height_fit)   {
        error_message += "Height mismatch: original = " + std::to_string(height_original) + ", fit = " + std::to_string(height_fit) + "\n";
    }

    if (image_data_original.size() != image_data_fit.size())   {
        error_message += "Image data size mismatch: original = " + std::to_string(image_data_original.size()) + ", fit = " + std::to_string(image_data_fit.size()) + "\n";
    }
    else    {
        for (size_t i = 0; i < image_data_original.size(); i++)   {
            if (image_data_original[i] != image_data_fit[i])   {
                error_message += "Image data mismatch at index " + std::to_string(i) + ": original = " + std::to_string(image_data_original[i]) + ", fit = " + std::to_string(image_data_fit[i]) + "\n";
                break;
            }
        }
    }

    const TestResult metadata_test_result = test_metadata_match(metadata_original, metadata_fit);

    if (error_message.empty())   {
        return metadata_test_result + TestResult(true, "");
    }
    else    {
        return metadata_test_result + TestResult(false, error_message);
    }
};
