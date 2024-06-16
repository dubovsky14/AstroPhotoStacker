#include "../headers/ImageFilesInputOutput.h"

#include "../headers/raw_file_reader.h"

#include <opencv2/opencv.hpp>
#include <string>



bool AstroPhotoStacker::get_photo_resolution(const std::string &input_file, int *width, int *height) {
    const bool raw_file = is_raw_file(input_file);
    if (raw_file) {
        return get_photo_resolution_raw_file(input_file, width, height);
    }
    else {
        cv::Mat image = cv::imread(input_file, -1);
        *width = image.cols;
        *height = image.rows;
        return true;
    }
};