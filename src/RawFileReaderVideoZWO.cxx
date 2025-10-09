#include "../headers/RawFileReaderVideoZWO.h"
#include "../headers/ZWOVideoTextFileInfo.h"
#include "../headers/VideoReader.h"
#include "../headers/MetadataCommon.h"

#include <opencv2/opencv.hpp>


using namespace AstroPhotoStacker;
using namespace std;

std::vector<short int> RawFileReaderVideoZWO::read_raw_file(int *width, int *height, std::array<char, 4> *bayer_pattern) {
    std::vector<short int> result = read_one_channel_from_video_frame<short int>(m_input_frame.get_file_address(), m_input_frame.get_frame_number(), width, height, 0);
    if (bayer_pattern != nullptr)   {
        Metadata metadata = read_metadata();
        *bayer_pattern = convert_bayer_string_to_int_array(metadata.bayer_matrix);
    }
    return result;
};

void RawFileReaderVideoZWO::get_photo_resolution(int *width, int *height) {
    read_one_channel_from_video_frame<short int>(m_input_frame.get_file_address(), m_input_frame.get_frame_number(), width, height, 0);
};

Metadata RawFileReaderVideoZWO::read_metadata_without_cache() {
    ZWOVideoTextFileInfo info(m_input_frame.get_file_address() + ".txt");
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