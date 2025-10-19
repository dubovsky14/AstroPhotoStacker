#include "../headers/CometSelectionFrame.h"
#include "../headers/FloatingPointSlider.h"
#include "../headers/IndividualColorStretchingBlackCorrectionWhite.h"

#include "../../headers/InputFrameReader.h"

using namespace std;
using namespace AstroPhotoStacker;

CometSelectionFrame::CometSelectionFrame(AlignmentFrame *parent, std::map<InputFrame, std::pair<float,float>> *comet_positions_storage, std::vector<AstroPhotoStacker::InputFrame> frames_to_select_from) :
    wxDialog(parent, wxID_ANY, "Select comet position in frame (right-click)",  wxDefaultPosition, wxSize(700, 800))   {

    m_comet_positions_storage = comet_positions_storage;
    m_frames_to_select_from = frames_to_select_from;


    m_main_vertical_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(m_main_vertical_sizer);

    add_image_preview();

    add_exposure_correction_spin_ctrl();

    add_summary_of_selected_positions();

    add_frames_dropdown_menu();

    add_control_buttons();


};


void CometSelectionFrame::add_image_preview()    {
    m_image_preview_comet_selection_tool = make_unique<ImagePreviewCometSelectionTool>(this, 600, 400, 255, true);
    m_image_preview_comet_selection_tool->set_max_zoom_factor(32);

    m_main_vertical_sizer->Add(m_image_preview_comet_selection_tool->get_image_preview_bitmap(), 1, wxCENTER, 0);
};


void CometSelectionFrame::add_exposure_correction_spin_ctrl()   {
    m_color_stretcher.add_luminance_stretcher(std::make_shared<IndividualColorStretchingBlackCorrectionWhite>());
    m_image_preview_comet_selection_tool->set_stretcher(&m_color_stretcher);

    m_exposure_correction_slider = make_unique<FloatingPointSlider>(
        this,
        "Exposure correction: ",
        -7.0,
        7.0,
        0.0,
        0.1,
        1,
        [this](float value){
            IndividualColorStretchingToolBase &luminance_stretcher = m_color_stretcher.get_luminance_stretcher(0);
            (dynamic_cast<IndividualColorStretchingBlackCorrectionWhite&>(luminance_stretcher)).set_stretching_parameters(0,value,1);
            m_image_preview_comet_selection_tool->update_preview_bitmap();
        }
    );
    m_exposure_correction_slider->add_sizer(m_main_vertical_sizer, 0, wxEXPAND | wxTOP, 5);
};

void CometSelectionFrame::add_summary_of_selected_positions() {
    m_summary_wx_static_text = new wxStaticText(this, wxID_ANY, build_summary_text());
    m_main_vertical_sizer->Add(m_summary_wx_static_text, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);
};

std::string CometSelectionFrame::build_summary_text() const {
    const int total_frames = static_cast<int>(m_frames_to_select_from.size());
    const int frames_with_positions = static_cast<int>(m_comet_positions_storage->size());
    return c_summary_text_template + std::to_string(frames_with_positions) + " / " + std::to_string(total_frames);
};

void CometSelectionFrame::update_summary_text() {
    m_summary_wx_static_text->SetLabel(build_summary_text());
};

void CometSelectionFrame::add_frames_dropdown_menu() {
    wxStaticText* select_file_text = new wxStaticText(this, wxID_ANY, "All available frames:");
    select_file_text->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    std::vector<wxString> frame_names;
    for (const InputFrame &frame : m_frames_to_select_from) {
        frame_names.push_back(frame.to_gui_string());
    }
    wxChoice* choice_box_frame = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, frame_names.size(), frame_names.data());
    choice_box_frame->SetSelection(0);
    update_image_preview(0);
    choice_box_frame->Bind(wxEVT_CHOICE, [this, choice_box_frame](wxCommandEvent&){
        int current_selection = choice_box_frame->GetSelection();
        update_image_preview(current_selection);
    });
    m_main_vertical_sizer->Add(select_file_text, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);
    m_main_vertical_sizer->Add(choice_box_frame, 0,  wxEXPAND, 5);

};

void CometSelectionFrame::add_control_buttons() {

    // HELP button
    wxButton* button_help = new wxButton(this, wxID_ANY, "HELP");
    button_help->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        wxMessageDialog dialog(this, "Select the position of the comet in a frame by right-clicking on the preview image. Do this for at least two frames (ideally, the first and last), but more the better. The program will then run least squares fit on the frames with selected positions to extract the comet initial position and velocity.", "Help", wxOK | wxICON_INFORMATION);
        dialog.ShowModal();
    });
    m_main_vertical_sizer->Add(button_help, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 10);



    m_buttons_horizontal_sizer = new wxBoxSizer(wxHORIZONTAL);

    auto add_button_to_sizer = [this](const std::string &label, std::function<void(wxCommandEvent&)> callback) {
        wxButton* button = new wxButton(this, wxID_ANY, label);
        button->Bind(wxEVT_BUTTON, callback);
        m_buttons_horizontal_sizer->Add(button, 1, wxEXPAND, 5);
    };

    add_button_to_sizer("First frame", [this](wxCommandEvent&){
        update_image_preview(0);
    });

    add_button_to_sizer("Previous frame", [this](wxCommandEvent&){
        int new_index = (m_current_frame_index - 1 + m_frames_to_select_from.size()) % m_frames_to_select_from.size();
        update_image_preview(new_index);
    });

    add_button_to_sizer("Next frame", [this](wxCommandEvent&){
        int new_index = (m_current_frame_index + 1) % m_frames_to_select_from.size();
        update_image_preview(new_index);
    });

    add_button_to_sizer("Last frame", [this](wxCommandEvent&){
        update_image_preview(static_cast<int>(m_frames_to_select_from.size()) - 1);
    });

    m_main_vertical_sizer->Add(m_buttons_horizontal_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);


    // OK button
    wxButton* button_ok = new wxButton(this, wxID_ANY, "OK");
    button_ok->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        const std::pair<float,float> comet_position = m_image_preview_comet_selection_tool->get_comet_position();
        if (comet_position.first >= 0 && comet_position.second >= 0)   {
            // save last comet position
            (*m_comet_positions_storage)[m_frames_to_select_from[m_current_frame_index]] = comet_position;
        }

        if (m_comet_positions_storage->size() < 2) {
            wxMessageDialog dialog(this, "Please select at least two comet positions.", "Error", wxOK | wxICON_ERROR);
            dialog.ShowModal();
            return;
        }
        EndModal(wxID_OK);
    });
    button_ok->SetToolTip("Proceed with the selected comet positions. It is necessary to select at least two positions, ideally the first and last frame, but more the better.");

    m_main_vertical_sizer->Add(button_ok, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 10);
};

void CometSelectionFrame::sort_frames_by_timestamp() {
    map<InputFrame, int> frame_to_timestamp;
    for (const InputFrame &frame : m_frames_to_select_from) {
        InputFrameReader frame_reader(frame, false);
        frame_to_timestamp[frame] = frame_reader.get_metadata().timestamp;
    }

    std::sort(
        m_frames_to_select_from.begin(),
        m_frames_to_select_from.end(),
        [&frame_to_timestamp](const AstroPhotoStacker::InputFrame &a, const AstroPhotoStacker::InputFrame &b) {
            return frame_to_timestamp[a] < frame_to_timestamp[b];
        }
    );
};

void CometSelectionFrame::update_image_preview(int frame_index) {
    const InputFrame &frame = m_frames_to_select_from[frame_index];
    const std::pair<float,float> comet_position = m_image_preview_comet_selection_tool->get_comet_position();
    if (comet_position.first >= 0 && comet_position.second >= 0)   {
        // save previous comet position
        (*m_comet_positions_storage)[m_frames_to_select_from[m_current_frame_index]] = comet_position;
    }
    m_image_preview_comet_selection_tool->read_preview_from_frame(frame);
    if (m_comet_positions_storage->find(frame) != m_comet_positions_storage->end()) {
        const std::pair<float,float> &stored_comet_position = (*m_comet_positions_storage)[frame];
        m_image_preview_comet_selection_tool->set_comet_position(stored_comet_position.first, stored_comet_position.second);
    }
    m_current_frame_index = frame_index;
    update_summary_text();
};