#pragma once

#include "../headers/InputFrame.h"
#include "../headers/Common.h"

#include <opencv2/opencv.hpp>

#include <string>
#include <vector>
#include <stdexcept>

namespace AstroPhotoStacker {

    template<class ValueType>
    std::vector<ValueType> read_one_channel_from_video_frame(const std::string &video_address, int frame_id, int *width, int *height, int channel) {
        cv::VideoCapture video(video_address);
        if (!video.isOpened()) {
            throw std::runtime_error("Unable to open video file: " + video_address);
        }
        video.set(cv::CAP_PROP_POS_FRAMES, frame_id);
        cv::Mat frame;
        video.read(frame);
        *width = frame.cols;
        *height = frame.rows;
        std::vector<ValueType> result(*width*(*height));
        int bit_depth = frame.depth();
        for (int y = 0; y < *height; y++) {
            for (int x = 0; x < *width; x++) {
                if (bit_depth == CV_8U) {
                    result[y*(*width) + x] = frame.at<cv::Vec3b>(y, x)[channel];
                }
                else if (bit_depth == CV_16U) {
                    result[y*(*width) + x] = frame.at<cv::Vec3w>(y, x)[channel];
                }
                else if (bit_depth == CV_16S) {
                    result[y*(*width) + x] = frame.at<cv::Vec3s>(y, x)[channel];
                }
                else {
                    throw std::runtime_error("Unsupported bit depth");
                }
            }
        }
        return result;
    }

    template<class ValueType>
    std::vector<std::vector<ValueType> > read_video_frame_rgb(const std::string &video_address, int frame_id, int *width, int *height) {
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
        const int n_colors = 3;
        for (int y = 0; y < *height; y++) {
            for (int x = 0; x < *width; x++) {
                for (int color = 0; color < n_colors; color++) {
                    if (bit_depth == CV_8U) {
                        result[2-color][y*(*width) + x] = frame.at<cv::Vec3b>(y, x)[color];
                    }
                    else if (bit_depth == CV_16U) {
                        result[2-color][y*(*width) + x] = frame.at<cv::Vec3w>(y, x)[color];
                    }
                    else if (bit_depth == CV_16S) {
                        result[2-color][y*(*width) + x] = frame.at<cv::Vec3s>(y, x)[color];
                    }
                    else {
                        throw std::runtime_error("Unsupported bit depth");
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

    float get_fps_of_video(const std::string &video_address);

    std::vector<InputFrame> get_video_frames(const std::string &video_address);

    bool is_valid_video_file(const std::string &video_address);
}