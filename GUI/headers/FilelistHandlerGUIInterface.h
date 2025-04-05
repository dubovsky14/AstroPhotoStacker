#pragma once

#include "../headers/FilelistHandler.h"

#include <string>
#include <vector>
#include <functional>
#include <exception>

struct FrameID  {
    AstroPhotoStacker::InputFrame   input_frame;
    FileTypes                       type;
    int                             group_number;

    FrameID(const FrameInfo &frame_info) {
        input_frame = frame_info.input_frame;
        type = frame_info.type;
        group_number = frame_info.group_number;
    };
};


class FilelistHandlerGUIInterface : public FilelistHandler  {
    enum class SortType {
        NAME,
        RANKING,
        GROUP
    };

    public:
        FilelistHandlerGUIInterface() = default;

        FilelistHandlerGUIInterface(const FilelistHandler &filelist_handler);

        ~FilelistHandlerGUIInterface() = default;

        void sort_by_name(bool ascending = true);

        void sort_by_ranking(bool ascending = true);

        void sort_by_group(bool ascending = true);

        std::string get_gui_string(const FrameID &frame_info);

        FrameID get_frame_by_index(size_t index) const {
            if (index >= m_shown_frames.size())   {
                throw std::out_of_range("FilelistHandlerGUIInterface::FrameID - Index out of range");
            }
            return m_shown_frames[index].second;
        };

        const std::vector<std::pair<std::string,FrameID>> &get_shown_frames() const {
            return m_shown_frames;
        };

        void keep_best_n_frames(unsigned int n);

        void remove_frame(size_t index);

        bool frame_is_checked(const FrameID &frame_info)    const {
            return FilelistHandler::frame_is_checked(frame_info.group_number, frame_info.input_frame, frame_info.type);
        };

        bool frame_is_checked(size_t index)    const;

        void set_frame_checked(size_t index, bool checked);

        FilelistHandlerGUIInterface get_filelist_with_checked_frames() const;

    private:
        void update_shown_frames();

        void sort_frames();

        void sort_by_name_internal();
        void sort_by_ranking_internal();
        void sort_by_group_internal();

        std::vector<std::pair<std::string,FrameID>> m_shown_frames;

        SortType m_sort_type = SortType::NAME;
        bool m_sort_ascending = true;
};