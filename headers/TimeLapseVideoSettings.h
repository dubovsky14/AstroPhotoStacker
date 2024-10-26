#pragma once

#include "../headers/TimeLapseVideoCreator.h"

#include <string>

namespace AstroPhotoStacker {
    class TimeLapseVideoSettings    {
        public:
            TimeLapseVideoSettings() = default;

            void set_fps(float fps);

            float get_fps()   const;

            void set_n_repeat(int n_repeat);

            int get_n_repeat()  const;

            void set_codec(const char codec[4]);

            void set_codec(const std::string &codec);

            const char* get_codec() const;

            void configure_timelapse_video_creator(TimeLapseVideoCreator *settings) const;

        private:
            float m_fps = 25;
            int m_n_repeat = 1;
            char m_codec[4] = {'M', 'J', 'P', 'G'};
    };
}