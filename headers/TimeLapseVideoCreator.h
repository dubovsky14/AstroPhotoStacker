#pragma once

#include "../headers/TimeLapseVideoSettings.h"

#include <string>
#include <vector>
#include <tuple>


namespace AstroPhotoStacker   {
    class TimeLapseVideoCreator {
        public:
            TimeLapseVideoCreator() = default;

            TimeLapseVideoCreator(const TimeLapseVideoSettings &settings) :
                m_settings(settings)    {
            };

            void add_image(const std::string &file_address, int unix_time);

            void clear();

            TimeLapseVideoSettings* get_video_settings()    {
                return &m_settings;
            };

            void create_video(const std::string &video_address, bool sort_by_time = true) const;

        private:
            std::vector<std::tuple<std::string, int>>  m_input_files;

            TimeLapseVideoSettings m_settings;
    };
}