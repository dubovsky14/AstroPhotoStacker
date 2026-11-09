#include "../headers/FramesSelectionTool.h"
#include "../headers/ProgressBarWindow.h"
#include "../headers/CometSelectionFrame.h"
#include "../headers/SettingsCustomization.h"

#include "../../headers/PhotoAlignmentHandler.h"
#include "../../headers/InputFrame.h"
#include "../../headers/ReferencePhotoHandlerFactory.h"
#include "../../headers/LensCorrectionTool.h"
#include "../../headers/Common.h"

#include <wx/progdlg.h>

#include <vector>
#include <iostream>
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
    add_value_type_dropdown();
    add_selection_line();
    add_apply_button();

    this->SetSizer(m_main_sizer);
};

void FramesSelectionTool::add_value_type_dropdown() {
    wxStaticText* select_value_type_text = new wxStaticText(this, wxID_ANY, "Select value used for selection:");
    select_value_type_text->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    vector<wxString> value_types;
    for (const auto& selection_function : m_selection_functions) {
        value_types.push_back(selection_function.first);
    }

    wxChoice* choice_box_value_type = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, value_types.size(), value_types.data());
    choice_box_value_type->SetSelection(0);
    choice_box_value_type->Bind(wxEVT_CHOICE, [this, choice_box_value_type](wxCommandEvent&){
        m_value_type_selection = choice_box_value_type->GetSelection();
    });
    m_main_sizer->Add(select_value_type_text, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);
    m_main_sizer->Add(choice_box_value_type, 0,  wxEXPAND, 5);
}



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
};

void FramesSelectionTool::add_selection_line() {
    wxSizer *selection_line_sizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* value_text = new wxStaticText(this, wxID_ANY, "Value");
    selection_line_sizer->Add(value_text, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    vector<wxString> comparison_options;
    for (const auto& option : s_comparison_options) {
        comparison_options.push_back(option.first);
    }

    wxChoice* comparison_operator_choice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, comparison_options.size(), comparison_options.data());
    comparison_operator_choice->SetSelection(0);
    selection_line_sizer->Add(comparison_operator_choice, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    comparison_operator_choice->Bind(wxEVT_CHOICE, [this, comparison_operator_choice](wxCommandEvent&){
        m_comparison_operator_selection = comparison_operator_choice->GetSelection();
    });

    // text field input for the value
    m_value_input = new wxTextCtrl(this, wxID_ANY);
    selection_line_sizer->Add(m_value_input, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    m_main_sizer->Add(selection_line_sizer, 0, wxEXPAND, 5);
};

void FramesSelectionTool::add_apply_button() {
    wxButton* apply_button = new wxButton(this, wxID_ANY, "Apply");

    apply_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        const std::function<bool(const FrameInfo&, const std::string&, ComparisonEnum)>& selection_function = m_selection_functions[m_value_type_selection].second;
        const std::string value = m_value_input->GetValue().ToStdString();
        const ComparisonEnum comparison = s_comparison_options[m_comparison_operator_selection].second;
        auto final_lambda = [selection_function, value, comparison](const FrameInfo& frame_info) -> bool {
            return selection_function(frame_info, value, comparison);
        };
        m_filelist_handler_gui_interface->check_frames_based_on_condition(final_lambda);
        m_parent_my_frame->update_files_to_stack_checkbox();
    });

    m_main_sizer->Add(apply_button, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
};