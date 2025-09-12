#pragma once

#include "../headers/FilelistHandlerGUIInterface.h"

#include <map>
#include <string>
#include <vector>

class SummaryYamlCreator {
    public:
        SummaryYamlCreator(const FilelistHandlerGUIInterface &filelist_handler_gui);

        std::string get_yaml_summary() const;

        void create_and_save_yaml_file(const std::string &output_address) const;

    private:

        std::vector<std::string> get_overall_frames_summary() const;

        std::vector<std::string> get_group_summary(int group_number) const;

        std::vector<std::string> get_group_and_type_summary(int group_number, FrameType frame_type) const;

        FilelistHandlerGUIInterface m_filelist_handler_gui;

        std::string block_to_string(const std::vector<std::string> &block, const std::string &indent) const;

        std::vector<int> m_group_numbers;

        static const std::string s_indent;
        static const std::string s_indent_new_item;
};