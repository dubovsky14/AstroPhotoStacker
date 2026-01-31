#include "../headers/InputFrame.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/LocalShiftsHandler.h"
#include "../headers/ReferencePhotoHandlerSurface.h"
#include "../headers/ImageFilesInputOutput.h"

#include <opencv2/opencv.hpp>

#include <iostream>
#include <vector>

using namespace std;
using namespace AstroPhotoStacker;


void scale_to_8_bits(std::vector<std::vector<unsigned short> > *image, int width, int height, bool downscale_green = true) {
    const int n_colors = image->size();
    // get maximum
    unsigned short max_value = 0;
    for (int i_color = 0; i_color < n_colors; i_color++) {
        for (int i_pixel = 0; i_pixel < width*height; i_pixel++) {
            if (image->at(i_color)[i_pixel] > max_value) {
                max_value = image->at(i_color)[i_pixel];
            }
        }
    }

    // scale
    const double scale_factor = 255.0/max_value;
    for (int i_color = 0; i_color < n_colors; i_color++) {
        for (int i_pixel = 0; i_pixel < width*height; i_pixel++) {
            image->at(i_color)[i_pixel] *= scale_factor;

            //downscale green
            if ((i_color == 1 || i_color == 3) && downscale_green) {
                image->at(i_color)[i_pixel] /= 2;
            }
            else {
                image->at(i_color)[i_pixel] = min<int>(image->at(i_color)[i_pixel], 128);
            }
        }
    }
}

int main(int argc, const char **argv)   {
    if (argc != 3)   {
        std::cerr << "Usage: test_local_shifts <reference file> <file to align>\n";
        return 1;
    }

    const std::string reference_file = argv[1];
    const std::string file_to_align = argv[2];

    const InputFrame reference_frame(reference_file);
    const InputFrame alternative_frame(file_to_align);

    PhotoAlignmentHandler photo_alignment_handler;
    photo_alignment_handler.set_alignment_method("surface", ConfigurableAlgorithmSettingsMap());
    photo_alignment_handler.align_files(reference_frame, {alternative_frame});

    std::unique_ptr<AlignmentResultBase> alignment_result = photo_alignment_handler.get_alignment_parameters(alternative_frame);


    CalibratedPhotoHandler calibrated_photo_handler(alternative_frame, true);
    calibrated_photo_handler.define_alignment(*alignment_result);
    calibrated_photo_handler.calibrate();


    const int width = calibrated_photo_handler.get_width();
    const int height = calibrated_photo_handler.get_height();

    std::vector<vector<unsigned short>> output_image(3, vector<unsigned short>(width*height, 0));

    for (int color = 0; color < 3; color++) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int value = calibrated_photo_handler.get_value_by_reference_frame_index(x + width*y, color);
                if (value >= 0) {
                    output_image[color][x + width*y] = value;
                }
            }
        }
    }

    scale_to_8_bits(&output_image, width, height);
    const std::string output_file_address = "aligned_image.jpg";
    create_color_image(output_image.at(0).data(), output_image.at(1).data(), output_image.at(2).data(), width, height, output_file_address);
}