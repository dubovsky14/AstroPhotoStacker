#pragma once

#include "../headers/FilelistHandlerGUIInterface.h"
#include "../headers/PostProcessingTool.h"
#include "../../headers/Metadata.h"
#include "../../headers/StackSettings.h"

#include <map>
#include <string>
#include <vector>

class SummaryYamlCreator {
    public:
        SummaryYamlCreator(const FilelistHandlerGUIInterface &filelist_handler_gui, const AstroPhotoStacker::StackSettings &stack_settings);

        std::string get_yaml_summary(const PostProcessingTool *post_processing_tool = nullptr) const;

        void create_and_save_yaml_file(const std::string &output_address, const PostProcessingTool *post_processing_tool = nullptr) const;

        void add_as_exif_metadata(const std::string &output_address) const;

    private:

        std::vector<std::string> get_overall_frames_summary() const;

        std::vector<std::string> get_group_summary(int group_number) const;

        std::vector<std::string> get_group_and_type_summary(int group_number, FrameType frame_type) const;

        std::vector<std::string> get_post_processing_summary(const PostProcessingTool *post_processing_tool) const;

        std::vector<std::string> get_stack_settings_summary() const;

        FilelistHandlerGUIInterface m_filelist_handler_gui;
        AstroPhotoStacker::StackSettings m_stack_settings;

        std::string block_to_string(const std::vector<std::string> &block, const std::string &indent) const;

        std::vector<int> m_group_numbers;

        static const std::string s_indent;
        static const std::string s_indent_new_item;

        AstroPhotoStacker::Metadata get_metadata_of_first_light()  const;
};