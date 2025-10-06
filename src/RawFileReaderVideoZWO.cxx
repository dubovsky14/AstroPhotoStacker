#include "../headers/RawFileReaderVideoZWO.h"
#include "../headers/ZWOVideoTextFileInfo.h"

#include <opencv2/opencv.hpp>


using namespace AstroPhotoStacker;
using namespace std;

std::vector<short int> RawFileReaderVideoZWO::read_raw_file(int *width, int *height, std::array<char, 4> *bayer_pattern) {

};

void RawFileReaderVideoZWO::get_photo_resolution(int *width, int *height) {

};

Metadata RawFileReaderVideoZWO::read_metadata() {
    ZWOVideoTextFileInfo info(m_input_frame.get_file_address());
    Metadata metadata = info.get_metadata();
    metadata.video_fps = get_fps_of_video(m_input_frame.get_file_address());
    return metadata;
};

float RawFileReaderVideoZWO::get_fps_of_video(const std::string &video_address)    {
    cv::VideoCapture video(video_address);
    if (!video.isOpened()) {
        throw std::runtime_error("Unable to open video file: " + video_address);
    }
    return video.get(cv::CAP_PROP_FPS);
};