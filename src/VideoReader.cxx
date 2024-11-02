#include "../headers/VideoReader.h"

using namespace std;
using namespace AstroPhotoStacker;

int AstroPhotoStacker::get_number_of_frames_in_video(const std::string &video_address) {
    cv::VideoCapture video(video_address);
    if (!video.isOpened()) {
        throw std::runtime_error("Unable to open video file: " + video_address);
    }
    return video.get(cv::CAP_PROP_FRAME_COUNT);
}

std::vector<InputFrame> AstroPhotoStacker::get_video_frames(const std::string &video_address) {
    int n_frames = get_number_of_frames_in_video(video_address);
    std::vector<InputFrame> result;
    for (int i_frame = 0; i_frame < n_frames; i_frame++) {
        result.emplace_back(video_address, i_frame);
    }
    return result;
}

bool AstroPhotoStacker::is_valid_video_file(const std::string &video_address)   {
    cv::VideoCapture video(video_address);
    return video.isOpened();
};