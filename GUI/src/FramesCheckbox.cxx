#include "../headers/FramesCheckbox.h"
#include "../headers/StackerConfigureTool.h"

using namespace AstroPhotoStacker;
using namespace std;


FramesCheckbox::FramesCheckbox(wxWindow *parent)    {
    // TODO
};

void FramesCheckbox::add_files(const std::vector<std::string> &files, FileTypes file_type)  {
    if (m_input_files.find(file_type) == m_input_files.end()) {
        m_input_files[file_type] = vector<InputFile>();
    }

    for (const string &file : files)    {
        m_input_files[file_type].push_back(InputFile(file));
    }
};

vector<InputFrame> FramesCheckbox::get_checked_frames(FileTypes type) const {
    vector<InputFrame> result;
    for (const InputFile &input_file : m_input_files.at(type)) {
        const vector<InputFrame> checked_frames = input_file.get_checked_frames();
        result.insert(result.end(), checked_frames.begin(), checked_frames.end());
    }
    return result;
};

void FramesCheckbox::add_on_click_callback(const std::function<void(const InputFrame&)> &callback)  {
    m_on_click_callback = callback;
};

void FramesCheckbox::set_checked_status_for_all_frames(bool checked)    {
    for (auto &input_files : m_input_files) {
        for (InputFile &input_file : input_files.second) {
            input_file.set_check_for_all_frames(checked);
        }
    }
};

//std::unique_ptr<AstroPhotoStacker::StackerBase> FramesCheckbox::get_configured_stacker(const AstroPhotoStacker::StackSettings &stack_settings) const  {
//    return get_configured_stacker(stack_settings, m_filelist_handler);
//};

void FramesCheckbox::set_alignment_info(const InputFrame &frame, const AlignmentFileInfo& alignment_info)   {
    const int light_file_index = get_light_file_index(frame);
    if (light_file_index != -1) {
        InputFile &input_file = m_input_files[FileTypes::LIGHT][light_file_index];
        input_file.set_alignment_info(frame, alignment_info);
    }
};

const AlignmentFileInfo FramesCheckbox::get_alignment_info(const InputFrame &frame) const  {
    const int light_file_index = get_light_file_index(frame);
    if (light_file_index != -1) {
        const InputFile &input_file = m_input_files.at(FileTypes::LIGHT)[light_file_index];
        return input_file.get_alignment_info(frame);
    }
    return AlignmentFileInfo();
};

Metadata FramesCheckbox::get_metadata(const InputFrame &frame) const  {
    const int light_file_index = get_light_file_index(frame);
    if (light_file_index != -1) {
        const InputFile &input_file = m_input_files.at(FileTypes::LIGHT)[light_file_index];
        return input_file.get_metadata(frame);
    }
    return Metadata();
};

int FramesCheckbox::get_light_file_index(const InputFrame &input_frame) const   {
    const string input_frame_path = input_frame.get_file_address();
    if (m_input_files.find(FileTypes::LIGHT) == m_input_files.end()) {
        return -1;
    }
    for (unsigned int i = 0; i < m_input_files.at(FileTypes::LIGHT).size(); i++) {
        if (m_input_files.at(FileTypes::LIGHT)[i].get_file_path() == input_frame_path) {
            return i;
        }
    }
    return -1;
};