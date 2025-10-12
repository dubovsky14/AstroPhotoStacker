#include "../headers/NonRawFrameReaderBase.h"

using namespace AstroPhotoStacker;
using namespace std;

NonRawFrameReaderBase::NonRawFrameReaderBase(const InputFrame &input_frame) : FrameReaderBase(input_frame) {
};

std::vector<std::vector<PixelType>> NonRawFrameReaderBase::opencv_rgb_image_to_vector_vector_short(const cv::Mat &image, int *width, int *height) {
    const int bit_depth = image.depth();
    const int n_colors = image.channels();
    std::vector<std::vector<PixelType>> result(n_colors, std::vector<PixelType>(image.cols*image.rows));
    if (n_colors == 3)  {
        for (int y = 0; y < image.rows; y++) {
            for (int x = 0; x < image.cols; x++) {
                for (int color = 0; color < n_colors; color++) {
                    if (bit_depth == CV_8U) {
                        result[2-color][y*(image.cols) + x] = image.at<cv::Vec3b>(y, x)[color];
                    }
                    else if (bit_depth == CV_16U) {
                        result[2-color][y*(image.cols) + x] = image.at<cv::Vec3w>(y, x)[color]/2;
                    }
                    else if (bit_depth == CV_16S) {
                        result[2-color][y*(image.cols) + x] = image.at<cv::Vec3s>(y, x)[color];
                    }
                    else {
                        throw std::runtime_error("Unsupported bit depth");
                    }
                }
            }
        }
    }
    else if (n_colors == 1) {
        for (int y = 0; y < image.rows; y++) {
            for (int x = 0; x < image.cols; x++) {
                if (bit_depth == CV_8U) {
                    result[0][y*(image.cols) + x] = image.at<unsigned char>(y, x);
                }
                else if (bit_depth == CV_16U) {
                    result[0][y*(image.cols) + x] = image.at<unsigned short>(y, x)/2;
                }
                else if (bit_depth == CV_16S) {
                    result[0][y*(image.cols) + x] = image.at<short>(y, x);
                }
                else {
                    throw std::runtime_error("Unsupported bit depth");
                }
            }
        }
    }
    else {
        throw std::runtime_error("Unsupported number of channels: " + std::to_string(n_colors));
    }

    if (width != nullptr) {
        *width = image.cols;
    }
    if (height != nullptr) {
        *height = image.rows;
    }
    return result;
};


std::vector<PixelType> NonRawFrameReaderBase::opencv_grayscale_image_to_vector_short(const cv::Mat &image, int *width, int *height) {
    const int bit_depth = image.depth();
    std::vector<PixelType> result(image.cols * image.rows, 0);
    for (int y = 0; y < (image.rows); y++) {
        for (int x = 0; x < (image.cols); x++) {
            if (bit_depth == CV_8U) {
                result[y*(image.cols) + x] = image.at<unsigned char>(y, x);
            }
            else if (bit_depth == CV_16U) {
                result[y*(image.cols) + x] = image.at<ushort>(y, x)/2;
            }
            else if (bit_depth == CV_16S) {
                result[y*(image.cols) + x] = image.at<short>(y, x);
            }
            else {
                throw std::runtime_error("Unsupported bit depth");
            }
        }
    }
    if (width != nullptr) {
        *width = image.cols;
    }
    if (height != nullptr) {
        *height = image.rows;
    }
    return result;
};