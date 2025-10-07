#include "../headers/RawFileReaderBase.h"

#include "../headers/CustomSharedMutex.h"

using namespace AstroPhotoStacker;
using namespace std;


std::map<std::string, Metadata> RawFileReaderBase::s_metadata_cache;
std::shared_mutex               RawFileReaderBase::s_metadata_mutex;

RawFileReaderBase::RawFileReaderBase(const InputFrame &input_frame) : m_input_frame(input_frame)   {
}

Metadata RawFileReaderBase::read_metadata() {
    const string input_file = m_input_frame.get_file_address();
    const int frame_number = m_input_frame.get_frame_number();

    {
        // Lock the mutex in shared (read-only) mode
        SharedMutexRAIIShared scoped_lock(&s_metadata_mutex);

        // Check if the metadata and fps are aleady in map
        if (s_metadata_cache.find(input_file) != s_metadata_cache.end()) {
            Metadata metadata = s_metadata_cache[input_file];
            if (frame_number < 0) {
                return metadata; // still image, no need to adjust timestamp
            }
            const float fps = metadata.video_fps;

            metadata.timestamp += int(frame_number / fps); // add time in video to to the original timestamp
            return metadata;
        }
    }

    // Video metadata and fps are not yet in the map, so we need to read them

    Metadata metadata = read_metadata_without_cache();
    const float fps = metadata.video_fps;

    // Lock the mutex in exclusive (write) mode. In principe, the value could have been written by another thread in the meantime, but we don't care
    SharedMutexRAIIExclusive scoped_lock(&s_metadata_mutex);
    s_metadata_cache[input_file] = metadata;

    if (frame_number < 0) {
        return metadata; // still image, no need to adjust timestamp
    }
    metadata.timestamp += int(frame_number / fps); // add time in video to to the original timestamp
    return metadata;
};