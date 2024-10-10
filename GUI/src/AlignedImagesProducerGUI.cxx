#include "../headers/AlignedImagesProducerGUI.h"
#include "../headers/IndividualColorStretchingBlackCorrectionWhite.h"

#include "../headers/StackSettings.h"

#include "../../headers/AlignedImagesProducer.h"
#include "../../headers/CalibrationFrameBase.h"
#include "../../headers/DarkFrameHandler.h"
#include "../../headers/FlatFrameHandler.h"
#include "../../headers/thread_pool.h"

#include "../headers/MainFrame.h"
#include "../headers/ProgressBarWindow.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/progdlg.h>

#include <vector>
#include <string>



using namespace std;
using namespace AstroPhotoStacker;

AlignedImagesProducerGUI::AlignedImagesProducerGUI(MyFrame *parent) :
        wxFrame(parent, wxID_ANY, "Produce aligned images", wxDefaultPosition, wxSize(1000, 800)),
        m_parent(parent)    {

    m_color_stretcher.add_luminance_stretcher(std::make_shared<IndividualColorStretchingBlackCorrectionWhite>());

    m_main_vertical_sizer = new wxBoxSizer(wxVERTICAL);

    m_image_preview_crop_tool = make_unique<ImagePreviewCropTool>(this, 600, 400, 255, true);
    m_image_preview_crop_tool->set_stretcher(&m_color_stretcher);

    const std::string reference_file = get_reference_file_address();
    if (reference_file != "") {
        m_image_preview_crop_tool->read_preview_from_file(reference_file);
        m_image_preview_crop_tool->update_preview_bitmap();
    }


    m_image_preview_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_main_vertical_sizer->Add(m_image_preview_sizer, 1, wxCENTER, 5);

    m_image_preview_sizer->Add(m_image_preview_crop_tool->get_image_preview_bitmap(), 1, wxCENTER, 0);
    add_exposure_correction_spin_ctrl();

    add_checkboxes();

    wxBoxSizer *bottom_horizontal_sizer = new wxBoxSizer(wxHORIZONTAL);

    m_main_vertical_sizer->Add(bottom_horizontal_sizer, 1, wxEXPAND | wxALL, 5);

    wxButton *button_select_output_folder      = new wxButton(this, wxID_ANY, "Select output folder", wxDefaultPosition, wxSize(200, 50));
    button_select_output_folder->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        wxDirDialog dialog(this, "Select folder for output images", "");
        if (dialog.ShowModal() == wxID_OK) {
            m_output_folder_address = dialog.GetPath().ToStdString();
        }
    });
    bottom_horizontal_sizer->Add(button_select_output_folder, 1, wxALL, 5);

    wxButton *drop_crop_button = new wxButton(this, wxID_ANY, "Drop crop", wxDefaultPosition, wxSize(200, 50));
    drop_crop_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        m_image_preview_crop_tool->drop_crop();
        m_image_preview_crop_tool->update_preview_bitmap();
    });
    bottom_horizontal_sizer->Add(drop_crop_button, 1, wxALL, 5);


    wxButton *button_produce_images      = new wxButton(this, wxID_ANY, "Produce images", wxDefaultPosition, wxSize(200, 50));
    button_produce_images->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        if (m_output_folder_address == "") {
            wxMessageBox("Please select the output folder first.", "Error", wxICON_ERROR);
            return;
        }


        this->initialize_aligned_images_producer();

        const int tasks_total = m_aligned_images_producer->get_tasks_total();
        const std::atomic<int> &tasks_processed = m_aligned_images_producer->get_tasks_processed();

        int crop_top_left_x, crop_top_left_y, crop_width, crop_height;
        m_image_preview_crop_tool->get_crop_coordinates(&crop_top_left_x, &crop_top_left_y, &crop_width, &crop_height);
        cout << "Crop params: " << crop_top_left_x << " " << crop_top_left_y << " " << crop_width << " " << crop_height << endl;
        if (crop_width > 0 && crop_height > 0) {
            m_aligned_images_producer->limit_output_image_size(crop_top_left_x, crop_top_left_y, crop_width, crop_height);
        }

        run_task_with_progress_dialog("Producing aligned images", "Finished", "", tasks_processed, tasks_total, [this](){
            if (m_apply_color_stretcher) {
                m_aligned_images_producer->set_image_stretching_function([this](std::vector<std::vector<unsigned short>> *image, unsigned short max_value){
                    m_color_stretcher.stretch_image(image, max_value, true);
                });
            }
            m_aligned_images_producer->produce_aligned_images(m_output_folder_address);
        });
    });
    bottom_horizontal_sizer->Add(button_produce_images, 1, wxALL, 5);


    SetSizer(m_main_vertical_sizer);
};

void AlignedImagesProducerGUI::initialize_aligned_images_producer()   {
    const StackSettings *stack_settings = m_parent->get_stack_settings();
    const FilelistHandler *filelist_handler = &m_parent->get_filelist_handler();
    m_aligned_images_producer = make_unique<AlignedImagesProducer>(stack_settings->get_n_cpus());
    m_aligned_images_producer->set_add_datetime(m_add_datetime);

    // Light frames
    const vector<string>    &light_frames = filelist_handler->get_files(FileTypes::LIGHT);
    const vector<bool>      &files_are_checked = filelist_handler->get_files_checked(FileTypes::LIGHT);
    const vector<AlignmentFileInfo> &alignment_info_vec = filelist_handler->get_alignment_info(FileTypes::LIGHT);
    for (size_t i_file = 0; i_file < light_frames.size(); ++i_file) {
        if (files_are_checked[i_file]) {
            const string &file = light_frames[i_file];
            const AlignmentFileInfo &alignment_info_gui = alignment_info_vec[i_file];

            FileAlignmentInformation alignment_info;
            alignment_info.shift_x = alignment_info_gui.shift_x;
            alignment_info.shift_y = alignment_info_gui.shift_y;
            alignment_info.rotation_center_x = alignment_info_gui.rotation_center_x;
            alignment_info.rotation_center_y = alignment_info_gui.rotation_center_y;
            alignment_info.rotation = alignment_info_gui.rotation;
            alignment_info.ranking = alignment_info_gui.ranking;
            alignment_info.local_shifts_handler = alignment_info_gui.local_shifts_handler;

            m_aligned_images_producer->add_image(file, alignment_info);
        }
    }

    // Calibration frames
    for (const FileTypes &file_type : {FileTypes::DARK, FileTypes::FLAT, FileTypes::BIAS}) {
        const vector<string> &calibration_frames = filelist_handler->get_files(file_type);
        if (calibration_frames.size() > 0) {
            shared_ptr<const CalibrationFrameBase> calibration_frame_handler = nullptr;
            switch (file_type) {
                case FileTypes::DARK:
                    calibration_frame_handler = make_shared<DarkFrameHandler>(calibration_frames[0]);
                    break;
                case FileTypes::FLAT:
                    calibration_frame_handler = make_shared<FlatFrameHandler>(calibration_frames[0]);
                    break;
                default:
                    break;
            }
            m_aligned_images_producer->add_calibration_frame_handler(calibration_frame_handler);
        }
    }
};

std::string AlignedImagesProducerGUI::get_reference_file_address() const  {
    const FilelistHandler *filelist_handler = &m_parent->get_filelist_handler();

    // Light frames
    const vector<string>    &light_frames = filelist_handler->get_files(FileTypes::LIGHT);
    const vector<bool>      &files_are_checked = filelist_handler->get_files_checked(FileTypes::LIGHT);
    const vector<AlignmentFileInfo> &alignment_info_vec = filelist_handler->get_alignment_info(FileTypes::LIGHT);
    for (size_t i_file = 0; i_file < light_frames.size(); ++i_file) {
        if (files_are_checked[i_file]) {
            const string &file = light_frames[i_file];
            const AlignmentFileInfo &alignment_info_gui = alignment_info_vec[i_file];

            if (alignment_info_gui.ranking != 0 && alignment_info_gui.shift_x == 0 && alignment_info_gui.shift_y == 0 && alignment_info_gui.rotation == 0) {
                return file;
            }
        }
    }
    return "";
};

void AlignedImagesProducerGUI::add_exposure_correction_spin_ctrl()   {
    // Create a wxStaticText to display the current value
    wxStaticText* exposure_correction_text = new wxStaticText(this, wxID_ANY, "Exposure correction: 0.0");

    // Create the wxSlider
    wxSlider* slider_exposure = new wxSlider(this, wxID_ANY, 0, -70, 70, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);

    // Bind the slider's wxEVT_SLIDER event to a lambda function that updates the value text
    slider_exposure->Bind(wxEVT_SLIDER, [exposure_correction_text, slider_exposure, this](wxCommandEvent&){
        const float exposure_correction = slider_exposure->GetValue()/10.;
        //m_current_preview->set_exposure_correction( exposure_correction );

        IndividualColorStretchingToolBase &luminance_stretcher = m_color_stretcher.get_luminance_stretcher(0);
        (dynamic_cast<IndividualColorStretchingBlackCorrectionWhite&>(luminance_stretcher)).set_stretching_parameters(0,exposure_correction,1);

        m_image_preview_crop_tool->update_preview_bitmap();
        const std::string new_label = "Exposure correction: " + to_string(exposure_correction+0.0001).substr(0,4);
        exposure_correction_text->SetLabel(new_label);
    });

    // Add the controls to a sizer
    //wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    m_main_vertical_sizer->Add(exposure_correction_text, 0,   wxEXPAND, 5);
    m_main_vertical_sizer->Add(slider_exposure, 0,  wxEXPAND, 5);

};


void AlignedImagesProducerGUI::add_checkboxes()   {
    wxCheckBox* add_datetime_checkbox = new wxCheckBox(this, wxID_ANY, "Add datetime to the output image");
    add_datetime_checkbox->SetValue(m_add_datetime);
    add_datetime_checkbox->SetToolTip("If checked, the datetime from photo's metadata will be shown in the image.");
    add_datetime_checkbox->Bind(wxEVT_CHECKBOX, [add_datetime_checkbox, this](wxCommandEvent&){
        const bool is_checked = add_datetime_checkbox->GetValue();
        m_add_datetime = is_checked;
    });
    m_main_vertical_sizer->Add(add_datetime_checkbox, 0, wxEXPAND, 5);

    wxCheckBox* apply_color_stretcher = new wxCheckBox(this, wxID_ANY, "Apply exposure correction to the output image");
    apply_color_stretcher->SetValue(m_apply_color_stretcher);
    apply_color_stretcher->SetToolTip("If checked, the exposure correction used for preview will be used also for output images.");
    apply_color_stretcher->Bind(wxEVT_CHECKBOX, [apply_color_stretcher, this](wxCommandEvent&){
        const bool is_checked = apply_color_stretcher->GetValue();
        m_apply_color_stretcher = is_checked;
    });
    m_main_vertical_sizer->Add(apply_color_stretcher, 0, wxEXPAND, 5);
};