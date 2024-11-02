#pragma once

#include <string>

namespace AstroPhotoStacker {
    /**
     * @class InputFrame
     * @brief Provies abstraction over possibile types of input frames - they can be either individual photos, or frames fromo video.
     */
    class InputFrame    {
        public:
            InputFrame() = default;

            InputFrame(const std::string &file_address) {
                m_file_address = file_address;
            };

            InputFrame(const std::string &video_address, int frame_number)  {
                m_file_address = video_address;
                m_frame_number = frame_number;
            };

            bool is_video_frame() const {
                return m_frame_number >= 0;
            };

            bool is_still_image() const {
                return !is_video_frame();
            };

            const std::string &get_file_address() const    {
                return m_file_address;
            };

            int get_frame_number() const    {
                return m_frame_number;
            };

            std::string to_string() const {
                return m_file_address + "|" + std::to_string(m_frame_number);
            };

            // needed to use InputFrame as a key in a map
            bool operator<(const InputFrame &other) const {
                if (m_file_address != other.m_file_address) {
                    return m_file_address < other.m_file_address;
                }
                return m_frame_number < other.m_frame_number;
            };

            bool operator==(const InputFrame &other) const {
                return m_file_address == other.m_file_address && m_frame_number == other.m_frame_number;
            };

            bool operator!=(const InputFrame &other) const {
                return !(*this == other);
            };

        private:
            std::string m_file_address = "";
            int         m_frame_number = -1;

    };
}