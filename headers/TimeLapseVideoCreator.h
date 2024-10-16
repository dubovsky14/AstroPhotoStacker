#pragma once

#include <string>
#include <vector>
#include <tuple>


namespace AstroPhotoStacker   {
    class TimeLapseVideoCreator {
        public:
            TimeLapseVideoCreator() = default;

            void add_image(const std::string &file_address, int unix_time);

            void set_fps(int fps);

            int get_fps()   const;

            void set_n_repeat(int n_repeat);

            int get_n_repeat()  const;

            void create_video(const std::string &video_address, bool sort_by_time = true) const;

        private:
            std::vector<std::tuple<std::string, int>>  m_input_files;

            int m_fps = 25;
            int m_n_repeat = 1;


    };
}