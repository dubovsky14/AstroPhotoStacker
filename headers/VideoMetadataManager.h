#pragma once

#include "../headers/MetadataReader.h"
#include "../headers/Metadata.h"
#include "../headers/InputFrame.h"
#include "../headers/Common.h"
#include "../headers/InputFormatTypes.h"
#include "../headers/ThreadSafeCacheSystem.h"
#include <string>


namespace AstroPhotoStacker {
    class VideoMetadataManager {
        public:
            VideoMetadataManager() = default;

            Metadata get_metadata(const InputFrame &input_frame);

        private:
            ThreadSafeCacheSystem<std::string, Metadata> m_metadata_cache; // map from video file name to pair of (fps, metadata)

            Metadata read_video_metadata(const std::string &video_address) const {
                const bool raw_file = is_raw_file(video_address);
                if (raw_file) {
                    return read_metadata_from_raw_file(video_address);
                }
                return read_metadata_video(video_address);
            }

            Metadata read_metadata_video_non_raw(const std::string &input_file) const;

            Metadata read_metadata_video(const std::string &input_file) const;


    };
}