#include "../headers/AlignedImagesProducerGUI.h"

#include "../headers/StackSettings.h"

#include "../../headers/AlignedImagesProducer.h"
#include "../../headers/CalibrationFrameBase.h"
#include "../../headers/DarkFrameHandler.h"
#include "../../headers/FlatFrameHandler.h"

#include "../headers/MainFrame.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <vector>
#include <string>



using namespace std;
using namespace AstroPhotoStacker;

AlignedImagesProducerGUI::AlignedImagesProducerGUI(MyFrame *parent) :
        wxFrame(parent, wxID_ANY, "Produce aligned images", wxDefaultPosition, wxSize(1000, 800)),
        m_parent(parent)    {

    m_main_vertical_sizer = new wxBoxSizer(wxVERTICAL);

    wxButton *button_select_output_folder      = new wxButton(this, wxID_ANY, "Select output folder");
    button_select_output_folder->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        wxDirDialog dialog(this, "Select folder for output images", "");
        if (dialog.ShowModal() == wxID_OK) {
            m_output_folder_address = dialog.GetPath().ToStdString();
        }
    });
    m_main_vertical_sizer->Add(button_select_output_folder, 1, wxALL, 5);


    wxButton *button_produce_images      = new wxButton(this, wxID_ANY, "Produce images");
    button_produce_images->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        this->initialize_aligned_images_producer();
        m_aligned_images_producer->produce_aligned_images(m_output_folder_address);
    });
    m_main_vertical_sizer->Add(button_produce_images, 1, wxALL, 5);

    SetSizer(m_main_vertical_sizer);
};

void AlignedImagesProducerGUI::initialize_aligned_images_producer()   {
    const StackSettings *stack_settings = m_parent->get_stack_settings();
    const FilelistHandler *filelist_handler = &m_parent->get_filelist_handler();
    m_aligned_images_producer = make_unique<AlignedImagesProducer>(stack_settings->get_n_cpus());
    m_aligned_images_producer->set_add_datetime(true);

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