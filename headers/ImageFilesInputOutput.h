#pragma once

#include "../headers/Metadata.h"

#include <opencv2/opencv.hpp>

#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <type_traits>

namespace AstroPhotoStacker {


    /**
     * @brief Create a gray scale image from an array of pixel values
     *
     * @tparam pixel_values_type The type of the pixel values
     * @param arr The array of pixel values
     * @param width The width of the image
     * @param height The height of the image
     * @param filename The filename of the image
     * @param image_settings The settings of the image
    */
    template <typename pixel_values_type>
    void create_gray_scale_image(const pixel_values_type* arr, int width, int height, const std::string& filename, int image_settings = CV_8UC1) {
        cv::Mat image(height, width, image_settings);

        const int mask = 0b111;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if ((image_settings & mask) == CV_8U) {
                    image.at<uchar>(y, x) = arr[y*width + x];
                }
                else if ((image_settings & mask) == CV_16U) {
                    image.at<ushort>(y, x) = arr[y*width + x];
                }
                else if ((image_settings & mask) == CV_8S) {
                    image.at<schar>(y, x) = arr[y*width + x];
                }
                else if ((image_settings & mask) == CV_16S) {
                    image.at<short>(y, x) = arr[y*width + x];
                }
                else if ((image_settings & mask) == CV_32S) {
                    image.at<int>(y, x) = arr[y*width + x];
                }
                else if ((image_settings & mask) == CV_32F) {
                    image.at<float>(y, x) = arr[y*width + x];
                }
                else if ((image_settings & mask) == CV_64F) {
                    image.at<double>(y, x) = arr[y*width + x];
                }
                else if ((image_settings & mask) == CV_16F) {
                    image.at<ushort>(y, x) = arr[y*width + x];
                }
                else    {
                    throw std::runtime_error("Unsupported image type");
                }
            }
        }
        cv::imwrite(filename, image);
    }

    /**
     * @brief Create a color image from an array of pixel values
     *
     * @tparam pixel_values_type The type of the pixel values
     * @tparam pixel_3d_type The type of the pixel values in 3D
     * @param arr_red The array of red pixel values
     * @param arr_green The array of green pixel values
     * @param arr_blue The array of blue pixel values
     * @param width The width of the image
     * @param height The height of the image
     * @param filename The filename of the image
     * @param image_settings The settings of the image
    */
    template <typename pixel_values_type, typename pixel_3d_type = cv::Vec3b>
    void crate_color_image_3d_template( const pixel_values_type* arr_red, const pixel_values_type* arr_green, const pixel_values_type* arr_blue,
                            int width, int height, const std::string& filename, int image_settings = CV_8UC3) {

        cv::Mat image(height, width, image_settings);
        for (int y = 0; y < height-1; y++) {
            for (int x = 0; x < width-1; x++) {
                pixel_3d_type& pixel = image.at<pixel_3d_type>(y, x);
                const unsigned int index_pixel = y*width + x;
                pixel[0] = arr_blue [index_pixel];
                pixel[1] = arr_green[index_pixel];
                pixel[2] = arr_red  [index_pixel];
            }
        }
        cv::imwrite(filename, image);
    }

    /**
     * @brief Create a color image from an array of pixel values
     *
     * @tparam pixel_values_type The type of the pixel values
     * @param arr_red The array of red pixel values
     * @param arr_green The array of green pixel values
     * @param arr_blue The array of blue pixel values
     * @param width The width of the image
     * @param height The height of the image
     * @param filename The filename of the image
     * @param image_settings The settings of the image
    */
    template <typename pixel_values_type>
    void crate_color_image( const pixel_values_type* arr_red, const pixel_values_type* arr_green, const pixel_values_type* arr_blue,
                            int width, int height, const std::string& filename, int image_settings = CV_8UC3) {

        const int mask = 0b111;
        if ((image_settings & mask) == CV_8U) {
            crate_color_image_3d_template<pixel_values_type, cv::Vec3b>(arr_red, arr_green, arr_blue, width, height, filename, image_settings);
        }
        else if ((image_settings & mask) == CV_16U) {
            crate_color_image_3d_template<pixel_values_type, cv::Vec3w>(arr_red, arr_green, arr_blue, width, height, filename, image_settings);
        }
        else if ((image_settings & mask) == CV_8S) {
            crate_color_image_3d_template<pixel_values_type, cv::Vec3s>(arr_red, arr_green, arr_blue, width, height, filename, image_settings);
        }
        else if ((image_settings & mask) == CV_16S) {
            crate_color_image_3d_template<pixel_values_type, cv::Vec3s>(arr_red, arr_green, arr_blue, width, height, filename, image_settings);
        }
        else if ((image_settings & mask) == CV_32S) {
            crate_color_image_3d_template<pixel_values_type, cv::Vec3i>(arr_red, arr_green, arr_blue, width, height, filename, image_settings);
        }
        else if ((image_settings & mask) == CV_32F) {
            crate_color_image_3d_template<pixel_values_type, cv::Vec3f>(arr_red, arr_green, arr_blue, width, height, filename, image_settings);
        }
        else if ((image_settings & mask) == CV_64F) {
            crate_color_image_3d_template<pixel_values_type, cv::Vec3d>(arr_red, arr_green, arr_blue, width, height, filename, image_settings);
        }
        else if ((image_settings & mask) == CV_16F) {
            crate_color_image_3d_template<pixel_values_type, cv::Vec3w>(arr_red, arr_green, arr_blue, width, height, filename, image_settings);
        }
        else    {
            throw std::runtime_error("Unsupported image type");
        }
    }

    /**
     * @brief Convert raw data to RGB image
     *
     * @tparam pixel_values_type The type of the pixel values
     * @param arr The array of pixel values
     * @param colors The colors of the pixels
     * @param width The width of the image
     * @param height The height of the image
     * @return std::vector<std::vector<pixel_values_type>> The RGB image, first index is the color, second index is the pixel
    */
    template <typename pixel_values_type>
    std::vector<std::vector<pixel_values_type> > convert_raw_data_to_rgb_image(const pixel_values_type* arr, const char *colors, int width, int height) {
        std::vector<std::vector<pixel_values_type> > result = std::vector<std::vector<pixel_values_type> >(3, std::vector<pixel_values_type>(width*height, 0));

        for (int y = 0; y < height-1; y++)  {
            for (int x = 0; x < width-1; x++)   {
                pixel_values_type this_pixel_rgb[3] = {0, 0, 0};

                // average out 2x2 pixels
                for (int y2 = 0; y2 < 2; y2++)  {
                    for (int x2 = 0; x2 < 2; x2++)   {
                        const unsigned int index = (y+y2)*width + (x+x2);
                        const unsigned int color = colors[index] != 3 ? colors[index] : 1;
                        this_pixel_rgb[color] += arr[index];
                    }
                }

                // usually there are 2 green pixels
                this_pixel_rgb[1] /= 2;

                // save the result
                for (int color = 0; color < 3; color++) {
                    result[color][y*width + x] = this_pixel_rgb[color];
                }
            }
        }

        return result;
    }

    template<class ValueType>
    std::vector<std::vector<ValueType> > read_rgb_image(const std::string &input_file, int *width, int *height) {
        cv::Mat image = cv::imread(input_file, cv::IMREAD_COLOR);
        *width = image.cols;
        *height = image.rows;
        std::vector<std::vector<ValueType>> result(3, std::vector<ValueType>(*width*(*height)));

        for (int y = 0; y < *height; y++) {
            for (int x = 0; x < *width; x++) {
                for (int color = 0; color < 3; color++) {
                    result[color][y*(*width) + x] = image.at<cv::Vec3b>(y, x)[color];
                }
            }
        }
        return result;
    };

    template<class ValueType>
    std::vector<ValueType> read_rgb_image_as_gray_scale(const std::string &input_file, int *width, int *height) {
        cv::Mat image = cv::imread(input_file, -1);
        *width = image.cols;
        *height = image.rows;
        std::vector<ValueType> result((*width)*(*height));
        for (int y = 0; y < (*height); y++) {
            for (int x = 0; x < (*width); x++) {
                result[y*(*width) + x] = image.at<unsigned short>(y, x);
            }
        }
        return result;
    };


    bool get_photo_resolution(const std::string &input_file, int *width, int *height);

    template <class ValueType>
    void decrease_image_bit_depth(ValueType *data, size_t data_size, int bits_to_drop) {
        static_assert(std::is_integral<ValueType>::value, "The data type must be an integral type");
        for (size_t i = 0; i < data_size; i++) {
            data[i] = data[i] >> bits_to_drop;
        }
    };
}