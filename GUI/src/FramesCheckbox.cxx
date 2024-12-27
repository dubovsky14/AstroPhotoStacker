#include "../headers/FramesCheckbox.h"
#include "../headers/StackerConfigureTool.h"

using namespace AstroPhotoStacker;
using namespace std;


FramesCheckbox::FramesCheckbox( wxWindow *parent,
                                const wxArrayString& choices,
                                wxWindowID id,
                                const wxPoint& pos,
                                const wxSize& size,
                                long style)   {
    m_files_to_stack_checkbox = new wxCheckListBox(parent, id, pos, size, choices, style);

    m_files_to_stack_checkbox->Bind(wxEVT_LISTBOX, [this](wxCommandEvent &event){
        int index = event.GetSelection();
        const RowInfo &row_info = m_row_info_vector[index];
        if (row_info.frame_index == -1) {  // unroll / hide frame sequence
            m_input_files_are_unrolled.at(FileTypes::LIGHT)[row_info.file_index] = !m_input_files_are_unrolled.at(FileTypes::LIGHT)[row_info.file_index];
            update_checkbox();
        }
        else {
            const InputFile &input_file = m_input_files.at(FileTypes::LIGHT).at(row_info.file_index);
            const InputFrame &input_frame = input_file.get_frames_info().at(row_info.frame_index).input_frame;
            if (m_on_click_callback) {
                m_on_click_callback(input_frame);
            }
        }
    });
};

void FramesCheckbox::add_sizer(wxBoxSizer *sizer, int proportion, int flag, int border)  {
    sizer->Add(m_files_to_stack_checkbox, proportion, flag, border);
};

void FramesCheckbox::add_files(const std::vector<std::string> &files, FileTypes file_type)  {
    if (m_input_files.find(file_type) == m_input_files.end()) {
        m_input_files[file_type] = vector<InputFile>();
        m_input_files_are_unrolled[file_type] = vector<bool>();
    }

    for (const string &file : files)    {
        m_input_files[file_type].push_back(InputFile(file));
        m_input_files_are_unrolled[file_type].push_back(false);
    }
    update_checkbox();
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

    for (unsigned int i_row = 0; i_row < m_files_to_stack_checkbox->GetCount(); ++i_row) {
        m_files_to_stack_checkbox->Check(i_row, checked);
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

void FramesCheckbox::remove_checked_frames()    {
    update_checked_elements();
    for (auto &[frame_type, input_files] : m_input_files) {
        for (unsigned int i_file = 0; i_file < input_files.size(); i_file++) {
            InputFile &input_file = input_files[i_file];
            input_file.remove_checked_frames();

            if (input_file.get_frames_info().empty()) {
                input_files.erase(input_files.begin() + i_file);
                m_input_files_are_unrolled[frame_type].erase(m_input_files_are_unrolled[frame_type].begin() + i_file);
                i_file--;
            }
        }

    }
    update_checkbox();
};

void FramesCheckbox::update_checked_elements()    {
    for (unsigned int i = 0; i < m_files_to_stack_checkbox->GetCount(); i++) {
        const bool file_checked_in_checkbox = m_files_to_stack_checkbox->IsChecked(i);
        const RowInfo &row_info = m_row_info_vector[i];
        if (row_info.frame_index == -1) {
            // TODO
        }
        else {
            InputFile &input_file = m_input_files.at(row_info.file_type)[row_info.file_index];
            input_file.set_frame_is_checked(input_file.get_frames_info().at(row_info.frame_index).input_frame, file_checked_in_checkbox);
        }
    }
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

void FramesCheckbox::update_checkbox()  {
    m_files_to_stack_checkbox->Clear();
    m_row_info_vector.clear();

    unsigned int i_row = 0;
    for (FileTypes type : {FileTypes::LIGHT, FileTypes::DARK, FileTypes::FLAT, FileTypes::BIAS})   {
        if (m_input_files.find(type) == m_input_files.end()) continue;

        const vector<InputFile> &files = m_input_files[type];
        const vector<bool> &files_are_unrolled = m_input_files_are_unrolled[type];
        for (int i_file = 0; i_file < int(files.size()); i_file++) {
            const InputFile &file = files[i_file];
            const bool has_multiple_frames = file.has_multiple_frames();
            if (!has_multiple_frames)   {
                const InputFrameInfoGUI frame_info = file.get_frames_info().at(0);
                const string frame_description = to_string(type) + "\t" + frame_info.gui_string;
                if (frame_info.is_checked)  {
                    m_files_to_stack_checkbox->Check(i_row);
                }
                m_files_to_stack_checkbox->Append(frame_description);
                m_row_info_vector.push_back({type, i_file, 0});
                i_row++;
            }
            else {
                const string file_string = to_string(type) + "\t" + file.get_gui_string();
                m_files_to_stack_checkbox->Append(file_string);
                m_files_to_stack_checkbox->Check(i_row);
                m_row_info_vector.push_back({type, i_file, -1});
                i_row++;

                if (files_are_unrolled[i_file]) {
                    const vector<InputFrameInfoGUI> &frames = file.get_frames_info();
                    for (int i_frame = 0; i_frame < int(frames.size()); i_frame++) {
                        const InputFrameInfoGUI &frame_info = frames[i_frame];
                        const string frame_description = "  " + to_string(type) + "\t" + frame_info.gui_string;
                        m_files_to_stack_checkbox->Append(frame_description);
                        if (frame_info.is_checked)  {
                            m_files_to_stack_checkbox->Check(i_row, true);
                        }
                        m_row_info_vector.push_back({type, i_file, i_frame});
                        i_row++;
                    }
                }
            }
        }
    }
};
