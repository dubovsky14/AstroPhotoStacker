#include "../headers/FramesToSERVideoConvertor.h"

#include "../headers/MetadataReader.h"
#include "../headers/InputFrameReader.h"

#include <utility>
#include <stdexcept>
#include <algorithm>

using namespace std;
using namespace AstroPhotoStacker;

FramesToSERVideoConvertor::FramesToSERVideoConvertor(const std::vector<InputFrame> &input_frames, float fps)  {
    m_input_frames = input_frames;
    m_fps = fps;
};

void FramesToSERVideoConvertor::save_to_file(const std::string &output_address)    {
    m_tasks_processed = 0;
    if (m_input_frames.empty()) {
        return;
    }

    vector<pair<InputFrame, Metadata>> input_frames_and_timestamps;
    for (const InputFrame &input_frame : m_input_frames)    {
        const Metadata metadata = read_metadata(input_frame);
        input_frames_and_timestamps.emplace_back(input_frame, metadata);
    }

    // sort by timestamp
    std::sort(input_frames_and_timestamps.begin(), input_frames_and_timestamps.end(), [](const std::pair<InputFrame, Metadata> &a, const std::pair<InputFrame, Metadata> &b) {
            return a.second.timestamp < b.second.timestamp;
    });

    // check consistency
    const Metadata metadata_first = input_frames_and_timestamps.at(0).second;
    for (const pair<InputFrame, Metadata> &frame_and_timestamp : input_frames_and_timestamps)   {
        const Metadata &metadata = frame_and_timestamp.second;
        if (!metadata.is_raw)   {
            throw std::runtime_error("One of the input frames is not raw frame");
        }

        if (metadata.bayer_matrix != metadata_first.bayer_matrix)   {
            throw std::runtime_error("Inconsistent bayer matrices!");
        }

        if (metadata.bit_depth != metadata_first.bit_depth)   {
            throw std::runtime_error("Inconsistent bit depths!");
        }
    }
    InputFrameReader input_frame_reader(input_frames_and_timestamps[0].first);
    int width, height;
    input_frame_reader.get_photo_resolution(&width, &height);
    float fps = m_fps > 0 ? m_fps : metadata_first.video_fps;
    if (fps <= 0) fps = 25;
    VideoWriterSER video_writter(output_address, metadata_first, width, height, fps, metadata_first.bit_depth);
    const int  output_bit_depth = metadata_first.bit_depth;
    for (const pair<InputFrame, Metadata> &frame_and_timestamp : input_frames_and_timestamps)   {
        const InputFrame &input_frame = frame_and_timestamp.first;
        InputFrameReader frame_reader(input_frame);
        std::vector<PixelType> frame_data = frame_reader.get_raw_data();

        if (output_bit_depth == 8) {
            std::vector<unsigned char> frame_data_8bit(frame_data.size(), 0);
            std::transform(frame_data.begin(), frame_data.end(), frame_data_8bit.begin(), [](PixelType x) -> unsigned char { return static_cast<unsigned char>(x / 128); });
            video_writter.write_frame(frame_data_8bit);
        }
        else if (output_bit_depth == 16) {
            std::vector<unsigned short> frame_data_16bit(frame_data.size(), 0);
            std::transform(frame_data.begin(), frame_data.end(), frame_data_16bit.begin(), [](PixelType x) -> unsigned short { return 2*static_cast<unsigned short>(x); });
            video_writter.write_frame(frame_data_16bit);
        }
        else {
            throw runtime_error("Unsupported output bit depth: " + to_string(output_bit_depth));
        }
        m_tasks_processed++;
    }
};