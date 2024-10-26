#pragma once

#include <string>
#include <array>

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

        private:
            float m_fps = 25;
            int m_n_repeat = 1;
            std::array<char, 4> m_codec = {'M', 'J', 'P', 'G'};
    };
}