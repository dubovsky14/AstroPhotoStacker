#include "../headers/RawFileReaderBase.h"

#include "../headers/CustomSharedMutex.h"

using namespace AstroPhotoStacker;
using namespace std;


ThreadSafeCacheSystem<std::string, Metadata> RawFileReaderBase::s_metadata_cache;

RawFileReaderBase::RawFileReaderBase(const InputFrame &input_frame) : m_input_frame(input_frame)   {
}

Metadata RawFileReaderBase::read_metadata() {
    const string input_file = m_input_frame.get_file_address();
    const int frame_number = m_input_frame.get_frame_number();

    auto get_metadata_lambda = [this]() {
        return read_metadata_without_cache();
    };

    Metadata metadata = s_metadata_cache.get(input_file, get_metadata_lambda);
    if (frame_number < 0) {
        return metadata; // still image, no need to adjust timestamp
    }

    const float fps = metadata.video_fps;
    metadata.timestamp += int(frame_number / fps); // add time in video to to the original timestamp
    return metadata;
};