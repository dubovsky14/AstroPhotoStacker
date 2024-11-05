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

            explicit InputFrame(const std::string &file_address) {
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

            std::string to_gui_string() const {
                if (is_video_frame()) {
                    return m_file_address + " | frame #" + std::to_string(m_frame_number);
                }
                return m_file_address;
            };

            static InputFrame build_from_gui_string(const std::string &input_string) {
                const size_t separator_position = input_string.find("|");
                if (separator_position == std::string::npos) {
                    return InputFrame(input_string);
                }
                const std::string file_address = input_string.substr(0, separator_position-1);
                const int frame_number = std::stoi(input_string.substr(separator_position + 9));
                return InputFrame(file_address, frame_number);
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