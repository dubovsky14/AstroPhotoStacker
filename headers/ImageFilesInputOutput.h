#pragma once

#include <opencv2/opencv.hpp>

#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>

namespace AstroPhotoStacker {

    template <typename pixel_values_type>
    void create_gray_scale_image(const pixel_values_type* arr, int width, int height, const std::string& filename, int image_settings = CV_8UC1) {
        cv::Mat image(height, width, image_settings);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if ((image_settings & CV_8U) == CV_8U) {
                    image.at<uchar>(y, x) = arr[y*width + x];
                }
                else if ((image_settings & CV_16U) == CV_16U) {
                    image.at<ushort>(y, x) = arr[y*width + x];
                }
                else if ((image_settings & CV_8S) == CV_8S) {
                    image.at<schar>(y, x) = arr[y*width + x];
                }
                else if ((image_settings & CV_16S) == CV_16S) {
                    image.at<short>(y, x) = arr[y*width + x];
                }
                else if ((image_settings & CV_32S) == CV_32S) {
                    image.at<int>(y, x) = arr[y*width + x];
                }
                else if ((image_settings & CV_32F) == CV_32F) {
                    image.at<float>(y, x) = arr[y*width + x];
                }
                else if ((image_settings & CV_64F) == CV_64F) {
                    image.at<double>(y, x) = arr[y*width + x];
                }
                else if ((image_settings & CV_16F) == CV_16F) {
                    image.at<ushort>(y, x) = arr[y*width + x];
                }
                else    {
                    throw std::runtime_error("Unsupported image type");
                }
            }
        }
        cv::imwrite(filename, image);
    }


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

    template <typename pixel_values_type>
    void crate_color_image( const pixel_values_type* arr_red, const pixel_values_type* arr_green, const pixel_values_type* arr_blue,
                            int width, int height, const std::string& filename, int image_settings = CV_8UC3) {

        if ((image_settings & CV_8U) == CV_8U) {
            crate_color_image_3d_template<pixel_values_type, cv::Vec3b>(arr_red, arr_green, arr_blue, width, height, filename, image_settings);
        }
        else if ((image_settings & CV_16U) == CV_16U) {
            crate_color_image_3d_template<pixel_values_type, cv::Vec3w>(arr_red, arr_green, arr_blue, width, height, filename, image_settings);
        }
        else if ((image_settings & CV_8S) == CV_8S) {
            crate_color_image_3d_template<pixel_values_type, cv::Vec3s>(arr_red, arr_green, arr_blue, width, height, filename, image_settings);
        }
        else if ((image_settings & CV_16S) == CV_16S) {
            crate_color_image_3d_template<pixel_values_type, cv::Vec3s>(arr_red, arr_green, arr_blue, width, height, filename, image_settings);
        }
        else if ((image_settings & CV_32S) == CV_32S) {
            crate_color_image_3d_template<pixel_values_type, cv::Vec3i>(arr_red, arr_green, arr_blue, width, height, filename, image_settings);
        }
        else if ((image_settings & CV_32F) == CV_32F) {
            crate_color_image_3d_template<pixel_values_type, cv::Vec3f>(arr_red, arr_green, arr_blue, width, height, filename, image_settings);
        }
        else if ((image_settings & CV_64F) == CV_64F) {
            crate_color_image_3d_template<pixel_values_type, cv::Vec3d>(arr_red, arr_green, arr_blue, width, height, filename, image_settings);
        }
        else if ((image_settings & CV_16F) == CV_16F) {
            crate_color_image_3d_template<pixel_values_type, cv::Vec3w>(arr_red, arr_green, arr_blue, width, height, filename, image_settings);
        }
        else    {
            throw std::runtime_error("Unsupported image type");
        }
    }

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
                this_pixel_rgb[1] /= 3;

                // save the result
                for (int color = 0; color < 3; color++) {
                    result[color][y*width + x] = this_pixel_rgb[color];
                }
            }
        }

        return result;
    }

}