#pragma once

#include "../headers/InputFrame.h"

#include <opencv2/opencv.hpp>

#include <string>
#include <vector>
#include <stdexcept>

namespace AstroPhotoStacker {
    template<class ValueType>
    std::vector<std::vector<ValueType> > read_video_frame(const std::string &video_address, int frame_id, int *width, int *height) {
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

        for (int y = 0; y < *height; y++) {
            for (int x = 0; x < *width; x++) {
                for (int color = 0; color < 3; color++) {
                    result[color][y*(*width) + x] = frame.at<cv::Vec3b>(y, x)[color];
                }
            }
        }
        return result;
    };

    int get_number_of_frames_in_video(const std::string &video_address);

    std::vector<InputFrame> get_video_frames(const std::string &video_address);
}