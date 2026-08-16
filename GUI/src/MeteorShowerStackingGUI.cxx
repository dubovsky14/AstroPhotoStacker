#include "../headers/MeteorShowerStackingGUI.h"
#include "../headers/IndividualColorStretchingBlackCorrectionWhite.h"
#include "../headers/Common.h"
#include "../headers/PhotoGroupingTool.h"
#include "../headers/StackerConfigureTool.h"

#include "../headers/StackSettings.h"

#include "../../headers/AlignedImagesProducer.h"
#include "../../headers/CalibrationFrameBase.h"
#include "../../headers/DarkFrameHandler.h"
#include "../../headers/FlatFrameHandler.h"
#include "../../headers/ImageFilesInputOutput.h"
#include "../../headers/TimeLapseVideoCreator.h"
#include "../../headers/PhotoAlignmentHandler.h"


#include "../headers/MainFrame.h"
#include "../headers/ProgressBarWindow.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/progdlg.h>

#include <vector>
#include <string>



using namespace std;
using namespace AstroPhotoStacker;

MeteorShowerStackingGUI::MeteorShowerStackingGUI(MyFrame *parent) :
        wxFrame(parent, wxID_ANY, "Meteor shower stacking", wxDefaultPosition, wxSize(1000, 1000), wxDEFAULT_FRAME_STYLE | wxMAXIMIZE),
        m_parent(parent)    {

    m_window_size = wxGetDisplaySize();
    m_filelist_handler_gui_interface = m_parent->get_filelist_handler_gui_interface();


    m_exposure_stretcher.add_luminance_stretcher(std::make_shared<IndividualColorStretchingBlackCorrectionWhite>());

    m_main_vertical_sizer = new wxBoxSizer(wxVERTICAL);



    m_image_preview_width = m_window_size.GetWidth() * 0.45;
    m_image_preview_height = m_image_preview_width * 2. / 3.; // 3:2 aspect ratio

    m_image_preview = make_unique<ImagePreview>(this, m_image_preview_width, m_image_preview_height, 255, true);
    m_image_preview->set_stretcher(&m_exposure_stretcher);

    m_reference_frame = get_reference_frame();
    if (m_reference_frame != InputFrame()) {
        m_image_preview->read_preview_from_frame(m_reference_frame);
        m_image_preview->update_preview_bitmap();
    }



    m_upper_part_sizer_horizontal = new wxBoxSizer(wxHORIZONTAL);
    m_main_vertical_sizer->Add(m_upper_part_sizer_horizontal, 0, wxEXPAND | wxALL, 5);


    m_image_preview_sizer = new wxBoxSizer(wxVERTICAL);
    m_main_vertical_sizer->Add(m_image_preview_sizer, 2, wxLEFT, 5);

    m_image_preview_sizer->Add(m_image_preview->get_image_preview_bitmap(), 1, wxCENTER, 0);
    add_exposure_correction_spin_ctrl();


    m_basic_settings_sizer = new wxBoxSizer(wxVERTICAL);
    m_main_vertical_sizer->Add(m_basic_settings_sizer, 1, wxEXPAND | wxALL, 5);


    // Add the list of files to stack
    add_list_of_files();


    SetSizer(m_main_vertical_sizer);
};

InputFrame MeteorShowerStackingGUI::get_reference_frame() const  {
    const FilelistHandlerGUIInterface *filelist_handler_gui_interface = &m_parent->get_filelist_handler_gui_interface();

    // Light frames
    const vector<FrameInfo>    light_frames = filelist_handler_gui_interface->get_checked_frames_of_type(FrameType::LIGHT);
    double min_distance = 1e20;
    const float x_orig(3000), y_orig(2000);
    InputFrame best_frame;
    for (const FrameInfo &frame_info : light_frames) {
        const InputFrame &frame                     = frame_info.input_frame;
        const AlignmentResultBase &alignment_result = *frame_info.alignment_result;
        float x(x_orig), y(y_orig);
        alignment_result.transform_from_reference_to_shifted_frame(&x, &y);

        const double distance = (x - x_orig) * (x - x_orig) + (y - y_orig) * (y - y_orig);
        if (distance < min_distance) {
            min_distance = distance;
            best_frame = frame;
        }
    }
    if (min_distance < 1e19) {
        return best_frame;
    }

    return InputFrame();
};

void MeteorShowerStackingGUI::add_exposure_correction_spin_ctrl()   {
    m_exposure_correction_slider = make_unique<FloatingPointSlider>(
        this,
        "Exposure correction: ",
        -7.0,
        7.0,
        0.0,
        0.1,
        1,
        [this](float value){
            IndividualColorStretchingToolBase &luminance_stretcher = m_exposure_stretcher.get_luminance_stretcher(0);
            (dynamic_cast<IndividualColorStretchingBlackCorrectionWhite&>(luminance_stretcher)).set_stretching_parameters(0,value,1);
            m_image_preview->update_preview_bitmap();
        }
    );
    m_exposure_correction_slider->add_sizer(m_image_preview_sizer, 0, wxEXPAND, 1);
};


void MeteorShowerStackingGUI::add_list_of_files() {
    wxArrayString files;
    m_files_checkbox = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, files, wxLB_MULTIPLE);

    // set font to the one with fixed width characters
    m_files_checkbox->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    // Create a new sizer for the header and the checkbox list
    wxBoxSizer* checkboxSizer = new wxBoxSizer(wxVERTICAL);
    checkboxSizer->Add(m_files_checkbox, 1, wxEXPAND | wxALL, 5);

    // Add the new sizer to the main frame's sizer
    m_main_vertical_sizer->Add(checkboxSizer, 9, wxEXPAND | wxALL, 5);

    m_files_checkbox->Bind(wxEVT_LISTBOX, [this](wxCommandEvent &event){
        const int index = event.GetSelection();
        m_filelist_handler_gui_interface.set_selected_frame_index(index);
        const bool update_needed = !update_checked_files_in_filelist();

        // Do not update the preview if the files was just checked/unchecked - it is slow
        if (update_needed) {
            update_image_preview_file(index);
        }
    });

    m_files_checkbox->Bind(wxEVT_CHAR_HOOK, [this](wxKeyEvent& event) {
        if (event.GetKeyCode() == WXK_DELETE) {
            const int index = m_filelist_handler_gui_interface.selected_frame_index();
            m_filelist_handler_gui_interface.remove_frame(index);
            update_files_to_stack_checkbox();
            const int new_index = (index < int(m_filelist_handler_gui_interface.get_number_of_shown_frames())) ?
                                   index : m_filelist_handler_gui_interface.get_number_of_shown_frames() - 1;
            update_image_preview_file(new_index);
        }
        if (event.GetKeyCode() == WXK_DOWN) {
            const int old_index = m_filelist_handler_gui_interface.selected_frame_index();
            int index = old_index + 1;
            if (index > int(m_files_checkbox->GetCount()) - 1) {
                index = int(m_files_checkbox->GetCount()) - 1;
            }
            m_filelist_handler_gui_interface.set_selected_frame_index(index);
            m_files_checkbox->SetSelection(index);
            if (old_index != index) {
                m_files_checkbox->Deselect(old_index);
                update_image_preview_file(index);
            }
        }
        if (event.GetKeyCode() == WXK_UP) {
            const int old_index = m_filelist_handler_gui_interface.selected_frame_index();
            int index = old_index - 1;
            if (index < 0) index = 0;
            m_filelist_handler_gui_interface.set_selected_frame_index(index);
            m_files_checkbox->SetSelection(index);
            if (old_index != index) {
                m_files_checkbox->Deselect(old_index);
                update_image_preview_file(index);
            }
        }
    });

    update_files_to_stack_checkbox();
}


void MeteorShowerStackingGUI::update_image_preview_file(size_t frame_index)  {
    if (frame_index >= m_filelist_handler_gui_interface.get_number_of_shown_frames()) {
        return;
    }
    const InputFrame frame = m_filelist_handler_gui_interface.get_frame_by_index(frame_index).input_frame;

    m_image_preview->read_preview_from_frame(frame);
    m_image_preview->update_preview_bitmap();
};

bool MeteorShowerStackingGUI::update_checked_files_in_filelist() {
    wxArrayInt checked_indices;
    m_files_checkbox->GetCheckedItems(checked_indices);
    bool updated = false;
    for (int i = 0; i < m_filelist_handler_gui_interface.get_number_of_all_frames(); i++) {
        const bool file_checked_in_checkbox = m_files_checkbox->IsChecked(i);
        const bool file_checked_in_filelist = m_filelist_handler_gui_interface.frame_is_checked(i);
        if (file_checked_in_checkbox != file_checked_in_filelist) {
            m_filelist_handler_gui_interface.set_frame_checked(i, file_checked_in_checkbox);
            updated = true;
        }
    }
    return updated;
};


void MeteorShowerStackingGUI::update_files_to_stack_checkbox()   {
    unsigned int index = m_filelist_handler_gui_interface.selected_frame_index() > 0 ? m_filelist_handler_gui_interface.selected_frame_index() : 0;
    m_files_checkbox->Clear();
    vector<string> rows;
    vector<bool>   rows_checked;

    m_filelist_handler_gui_interface.update_shown_frames();
    const vector<std::pair<std::string, FrameID>> &shown_frames = m_filelist_handler_gui_interface.get_shown_frames();
    for (const auto &frame : shown_frames)   {
        // show only light frames:
        if (frame.second.type != FrameType::LIGHT) {
            continue;
        }

        const std::string file_string = frame.first;
        const bool is_checked = m_filelist_handler_gui_interface.frame_is_checked(frame.second);
        rows.push_back(file_string);
        rows_checked.push_back(is_checked);
    }

    wxArrayString rows_wx;
    for (const string &row : rows) {
        rows_wx.Add(row);
    }

    m_files_checkbox->Append(rows_wx);
    for (unsigned int i = 0; i < rows_checked.size(); i++) {
        m_files_checkbox->Check(i, rows_checked[i]);
    }

    if (index >= rows_checked.size()) {
        index = rows_checked.size() ? rows_checked.size() - 1 : 0;
        m_filelist_handler_gui_interface.set_selected_frame_index(index);
    }

    m_files_checkbox->SetSelection(index);
};