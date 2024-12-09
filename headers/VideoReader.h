#pragma once

#include "../headers/InputFrame.h"

#include <opencv2/opencv.hpp>

#include <string>
#include <vector>
#include <stdexcept>

namespace AstroPhotoStacker {
    template<class ValueType>
    std::vector<std::vector<ValueType> > read_video_frame(const std::string &video_address, int frame_id, int *width, int *height, bool debayer = true) {
        cv::VideoCapture video(video_address);
        if (!video.isOpened()) {
            throw std::runtime_error("Unable to open video file: " + video_address);
        }
        video.set(cv::CAP_PROP_POS_FRAMES, frame_id);
        cv::Mat frame;
        video.read(frame);
        *width = frame.cols;
        *height = frame.rows;
        std::vector<std::vector<ValueType>> result(3, std::vector<ValueType>(*width*(*height)));
        int bit_depth = frame.depth();
        const int n_colors = debayer ? 1 : 3;
        for (int y = 0; y < *height; y++) {
            for (int x = 0; x < *width; x++) {
                for (int color = 0; color < n_colors; color++) {
                    if (bit_depth == CV_8U) {
                        result[color][y*(*width) + x] = frame.at<cv::Vec3b>(y, x)[color];
                    }
                    else if (bit_depth == CV_16U) {
                        result[color][y*(*width) + x] = frame.at<cv::Vec3w>(y, x)[color];
                    }
                    else if (bit_depth == CV_16S) {
                        result[color][y*(*width) + x] = frame.at<cv::Vec3s>(y, x)[color];
                    }
                    else {
                        throw std::runtime_error("Unsupported bit depth");
                    }
                }
            }
        }

        if (debayer) {
            const std::array<char, 4> bayer_matrix = {0,1,1,2}; // RGGB
            auto get_color = [&bayer_matrix, width, height](int x, int y) {
                return bayer_matrix[(y%2)*2 + x%2];
            };

            for (int y = 0; y < *height-1; y++) {
                for (int x = 0; x < *width-1; x++) {
                    ValueType colors[3] = {0,0,0};
                    int n_pixels[3] = {0,0,0};
                    for (int y_offset = 0; y_offset < 2; y_offset++) {
                        for (int x_offset = 0; x_offset < 2; x_offset++) {
                            const int index = (y+y_offset)*(*width) + x + x_offset;
                            const unsigned int color = get_color(x+x_offset, y+y_offset);
                            colors[color] += result[0][index];
                            n_pixels[color]++;
                        }
                    }
                    for (int color = 0; color < 3; color++) {
                        if (n_pixels[color] == 0) {
                            result[color][y*(*width) + x] = 0;
                            continue;
                        }
                        result[color][y*(*width) + x] = colors[color]/n_pixels[color];
                    }
                }
            }
        }


        return result;
    };

    template<class ValueType>
    std::vector<ValueType> read_video_frame_as_gray_scale(const std::string &video_address, int frame_id, int *width, int *height) {
        cv::VideoCapture video(video_address);
        if (!video.isOpened()) {
            throw std::runtime_error("Unable to open video file: " + video_address);
        }
        video.set(cv::CAP_PROP_POS_FRAMES, frame_id);
        cv::Mat frame;
        video.read(frame);
        *width = frame.cols;
        *height = frame.rows;

        cv::Mat gray_frame;
        cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);

        int bit_depth = gray_frame.depth();
        std::vector<ValueType> result(std::vector<ValueType>(*width*(*height)));
        for (int y = 0; y < *height; y++) {
            for (int x = 0; x < *width; x++) {
                if (bit_depth == CV_8U) {
                    result[y*(*width) + x] = gray_frame.at<unsigned char>(y, x);
                }
                else if (bit_depth == CV_16U) {
                    result[y*(*width) + x] = gray_frame.at<unsigned short>(y, x);
                }
                else if (bit_depth == CV_16S) {
                    result[y*(*width) + x] = gray_frame.at<short>(y, x);
                }
                else {
                    throw std::runtime_error("Unsupported bit depth");
                }
                result[y*(*width) + x] = gray_frame.at<unsigned char>(y, x);
            }
        }
        return result;
    };

    int get_number_of_frames_in_video(const std::string &video_address);

    std::vector<InputFrame> get_video_frames(const std::string &video_address);

    bool is_valid_video_file(const std::string &video_address);
}