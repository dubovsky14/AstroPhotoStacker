#include "../headers/VideoReader.h"
#include "../headers/Common.h"

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
    const string extension = video_address.substr(video_address.find_last_of(".") + 1);
    vector<string> supported_extensions = {"avi", "mp4", "mov"};
    bool supported_extension = false;
    for (string supported_extension : supported_extensions) {
        if (compare_case_insensitive(supported_extension, extension)) {
            supported_extension = true;
            break;
        }
    }
    if (!supported_extension) {
        return false;
    }

    // for some reason only this is not enough - isOpened returns true even for canon raw files ...
    cv::VideoCapture video(video_address);
    return video.isOpened();
};