#include "../headers/FramesSelectionTool.h"

#include "../../headers/InputFrame.h"
#include "../../headers/Common.h"

#include <vector>
#include <functional>
#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;

const std::vector<std::pair<std::string, ComparisonEnum>> FramesSelectionTool::s_comparison_options = {
    {" < ", ComparisonEnum::Less},
    {" <= ", ComparisonEnum::LessEqual},
    {" == ", ComparisonEnum::Equal},
    {" != ", ComparisonEnum::NotEqual},
    {" >= ", ComparisonEnum::GreaterEqual},
    {" > ", ComparisonEnum::Greater}
};

FramesSelectionTool::FramesSelectionTool(MyFrame *parent, FilelistHandlerGUIInterface *filelist_handler_gui_interface)
    :  wxFrame(parent, wxID_ANY, "Frames Selection Tool")      {

    m_parent_my_frame = parent;
    SetSize(m_window_size);

    m_filelist_handler_gui_interface = filelist_handler_gui_interface;
    m_main_sizer = new wxBoxSizer(wxVERTICAL);

    initialize_comparison_functions_vector();
    add_frame_types_checkboxes();

    m_main_sizer->AddSpacer(10);
    wxStaticText* set_selection_text = new wxStaticText(this, wxID_ANY, "Set frames selection condition:");
    set_selection_text->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    m_main_sizer->Add(set_selection_text, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);

    add_selection_line();

    m_main_sizer->AddSpacer(10);
    add_apply_button();

    this->SetSizer(m_main_sizer);
};

void FramesSelectionTool::add_frame_types_checkboxes() {
    wxBoxSizer* frame_types_sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* frame_types_text = new wxStaticText(this, wxID_ANY, "Select frame types:");
    frame_types_text->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    m_main_sizer->Add(frame_types_text, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);


    auto add_checkbox = [this, frame_types_sizer](const std::string& label, AstroPhotoStacker::FrameType frame_type) {
        wxCheckBox* check_box = new wxCheckBox(this, wxID_ANY, label);
        check_box->SetValue(m_frames_checked[frame_type]);
        check_box->Bind(wxEVT_CHECKBOX, [this, frame_type](wxCommandEvent&){
            m_frames_checked[frame_type] = !m_frames_checked[frame_type];
        });
        frame_types_sizer->Add(check_box, 0, wxALL, 5);
    };

    add_checkbox("Light", AstroPhotoStacker::FrameType::LIGHT);

    add_checkbox("Dark", AstroPhotoStacker::FrameType::DARK);

    add_checkbox("Flat", AstroPhotoStacker::FrameType::FLAT);

    m_main_sizer->Add(frame_types_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);
};

void FramesSelectionTool::initialize_comparison_functions_vector()   {
    m_selection_functions.clear();

    m_selection_functions.push_back({
        "Score",
        [](const FrameInfo& frame_info, const std::string &value, ComparisonEnum comparison) -> bool {
            if (!string_is_float(value)) {
                throw std::invalid_argument("Expected float value for Score comparison");
            }
            const float score = std::stof(value);
            return FramesSelectionTool::compare<float>(frame_info.alignment_result->get_ranking_score(), score, comparison);
        }
    });

    m_selection_functions.push_back({
        "FWHM",
        [](const FrameInfo& frame_info, const std::string &value, ComparisonEnum comparison) -> bool {
            if (!string_is_float(value)) {
                throw std::invalid_argument("Expected float value for FWHM comparison");
            }
            const float fwhm = std::stof(value);

            if (frame_info.type == FrameType::LIGHT && frame_info.alignment_result->is_valid() && frame_info.alignment_result->get_frame_score().stars_fwhm > 0) {
                return FramesSelectionTool::compare<float>(frame_info.alignment_result->get_frame_score().stars_fwhm, fwhm, comparison);
            }
            return false;
        }
    });

    m_selection_functions.push_back({
        "Group number",
        [](const FrameInfo& frame_info, const std::string &value, ComparisonEnum comparison) -> bool {
            if (!string_is_int(value)) {
                throw std::invalid_argument("Expected int value for Group number comparison");
            }
            const int group_number = std::stoi(value);
            return FramesSelectionTool::compare<int>(frame_info.group_number, group_number, comparison);
        }
    });

    m_selection_functions.push_back({
        "Brightness mean",
        [](const FrameInfo& frame_info, const std::string &value, ComparisonEnum comparison) -> bool {
            if (!string_is_float(value)) {
                throw std::invalid_argument("Expected float value for Mean brightness comparison");
            }
            const float mean_brightness = std::stof(value);
            return FramesSelectionTool::compare<float>(frame_info.alignment_result->get_frame_score().brightness_mean, mean_brightness, comparison);
        }
    });

    m_selection_functions.push_back({
        "Brightness std",
        [](const FrameInfo& frame_info, const std::string &value, ComparisonEnum comparison) -> bool {
            if (!string_is_float(value)) {
                throw std::invalid_argument("Expected float value for Brightness std comparison");
            }
            const float brightness_std = std::stof(value);
            return FramesSelectionTool::compare<float>(frame_info.alignment_result->get_frame_score().brightness_std, brightness_std, comparison);
        }
    });

    m_selection_functions.push_back({
        "Exposure time [s]",
        [](const FrameInfo& frame_info, const std::string &value, ComparisonEnum comparison) -> bool {
            if (!string_is_float(value)) {
                throw std::invalid_argument("Expected float value for Exposure time comparison");
            }
            const float exposure_time = std::stof(value);
            return FramesSelectionTool::compare<float>(frame_info.metadata.exposure_time, exposure_time, comparison);
        }
    });

    m_selection_functions.push_back({
        "ISO (or gain)",
        [](const FrameInfo& frame_info, const std::string &value, ComparisonEnum comparison) -> bool {
            if (!string_is_float(value)) {
                throw std::invalid_argument("Expected float value for ISO (or gain) comparison");
            }
            const float iso = std::stof(value);
            return FramesSelectionTool::compare<float>(frame_info.metadata.iso, iso, comparison);
        }
    });

    m_selection_functions.push_back({
        "Frame number",
        [](const FrameInfo& frame_info, const std::string &value, ComparisonEnum comparison) -> bool {
            if (!string_is_int(value)) {
                throw std::invalid_argument("Expected int value for Frame number comparison");
            }
            const int frame_number = std::stoi(value);
            return FramesSelectionTool::compare<int>(frame_info.input_frame.get_frame_number(), frame_number, comparison);
        }
    });

};

void FramesSelectionTool::add_selection_line() {
    wxSizer *selection_line_sizer = new wxBoxSizer(wxHORIZONTAL);

    vector<wxString> value_types;
    for (const auto& selection_function : m_selection_functions) {
        value_types.push_back(selection_function.first);
    }

    wxChoice* choice_box_value_type = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, value_types.size(), value_types.data());
    choice_box_value_type->SetSelection(m_value_type_selection);
    choice_box_value_type->Bind(wxEVT_CHOICE, [this, choice_box_value_type](wxCommandEvent&){
        m_value_type_selection = choice_box_value_type->GetSelection();
    });
    selection_line_sizer->Add(choice_box_value_type, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);


    vector<wxString> comparison_options;
    for (const auto& option : s_comparison_options) {
        comparison_options.push_back(option.first);
    }

    wxChoice* comparison_operator_choice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, comparison_options.size(), comparison_options.data());
    comparison_operator_choice->SetSelection(m_comparison_operator_selection);
    selection_line_sizer->Add(comparison_operator_choice, 1, wxALIGN_CENTER_VERTICAL | wxALL, 2);
    comparison_operator_choice->Bind(wxEVT_CHOICE, [this, comparison_operator_choice](wxCommandEvent&){
        m_comparison_operator_selection = comparison_operator_choice->GetSelection();
    });

    // text field input for the value
    m_value_input = new wxTextCtrl(this, wxID_ANY);
    selection_line_sizer->Add(m_value_input, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_main_sizer->Add(selection_line_sizer, 0, wxEXPAND, 5);
};

void FramesSelectionTool::add_apply_button() {
    wxButton* apply_button = new wxButton(this, wxID_ANY, "Apply");

    apply_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        const std::function<bool(const FrameInfo&, const std::string&, ComparisonEnum)>& selection_function = m_selection_functions[m_value_type_selection].second;
        const std::string value = m_value_input->GetValue().ToStdString();
        const ComparisonEnum comparison = s_comparison_options[m_comparison_operator_selection].second;
        auto final_lambda = [selection_function, value, comparison, this](const FrameInfo& frame_info) -> bool {
            if (!m_frames_checked[frame_info.type]) {
                return false;
            }
            return selection_function(frame_info, value, comparison);
        };
        m_filelist_handler_gui_interface->check_frames_based_on_condition(final_lambda);
        m_parent_my_frame->update_files_to_stack_checkbox();
    });

    m_main_sizer->Add(apply_button, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
};