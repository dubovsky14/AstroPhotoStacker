#pragma once

#include "../headers/MetadataReader.h"
#include "../headers/Metadata.h"
#include "../headers/InputFrame.h"
#include "../headers/Common.h"
#include "../headers/VideoReaderSer.h"

#include <string>
#include <map>
#include <shared_mutex>


namespace AstroPhotoStacker {
    class VideoMetadataManager {
        public:
            VideoMetadataManager() = default;

            Metadata get_metadata(const InputFrame &input_frame);

        private:
            std::shared_mutex m_metadata_mutex;
            std::map<std::string,std::pair<float, Metadata>> m_video_name_to_fps_and_metadata;

            Metadata read_video_metadata(const std::string &video_address) const {
                if (is_ser_file(video_address)) {
                    return read_ser_video_metadata(video_address);
                }
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