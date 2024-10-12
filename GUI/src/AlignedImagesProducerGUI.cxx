#include "../headers/AlignedImagesProducerGUI.h"
#include "../headers/IndividualColorStretchingBlackCorrectionWhite.h"
#include "../headers/Common.h"
#include "../headers/PhotoGroupingTool.h"
#include "../headers/FilelistHandler.h"
#include "../headers/StackerConfigureTool.h"

#include "../headers/StackSettings.h"

#include "../../headers/AlignedImagesProducer.h"
#include "../../headers/CalibrationFrameBase.h"
#include "../../headers/DarkFrameHandler.h"
#include "../../headers/FlatFrameHandler.h"
#include "../../headers/thread_pool.h"
#include "../../headers/ImageFilesInputOutput.h"


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

        if (m_stack_images) {
            m_use_grouping = true;
            stack_images_in_groups();
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


    add_advanced_settings();

    SetSizer(m_main_vertical_sizer);
};

void AlignedImagesProducerGUI::initialize_aligned_images_producer()   {
    const StackSettings *stack_settings = m_parent->get_stack_settings();
    const FilelistHandler *filelist_handler = &m_parent->get_filelist_handler();
    m_aligned_images_producer = make_unique<AlignedImagesProducer>(stack_settings->get_n_cpus());
    m_aligned_images_producer->set_add_datetime(m_add_datetime);
    m_aligned_images_producer->set_timestamp_offset(m_timestamp_offset);

    // Light frames
    const vector<string>    &light_frames = filelist_handler->get_files(FileTypes::LIGHT);
    const vector<bool>      &files_are_checked = filelist_handler->get_files_checked(FileTypes::LIGHT);
    const vector<AlignmentFileInfo> &alignment_info_vec = filelist_handler->get_alignment_info();
    const vector<Metadata> &metadata_vec = filelist_handler->get_metadata();

    auto add_file_to_aligned_images_produced = [this, &light_frames, &files_are_checked, &alignment_info_vec, &metadata_vec](size_t i_file) {
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
    };

    if (m_use_grouping) {
        PhotoGroupingTool photo_grouping_tool;
        for (size_t i_file = 0; i_file < light_frames.size(); ++i_file) {
            const string &file = light_frames[i_file];
            const AlignmentFileInfo &alignment_info_gui = alignment_info_vec[i_file];
            const Metadata &metadata = metadata_vec[i_file];
            if (files_are_checked[i_file]) {
                photo_grouping_tool.add_file(file, metadata.timestamp, alignment_info_gui.ranking);
            }
        }
        photo_grouping_tool.define_maximum_time_difference_in_group(m_grouping_time_interval);
        photo_grouping_tool.run_grouping();
        const vector<vector<size_t>> &groups = photo_grouping_tool.get_groups_indices();
        for (const vector<size_t> &group : groups) {
            if (group.size() > 0) {
                add_file_to_aligned_images_produced(group[0]);
            }
        }
    }
    else {
        for (size_t i_file = 0; i_file < light_frames.size(); ++i_file) {
            if (files_are_checked[i_file]) {
                add_file_to_aligned_images_produced(i_file);
            }
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
    const vector<AlignmentFileInfo> &alignment_info_vec = filelist_handler->get_alignment_info();
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

void AlignedImagesProducerGUI::add_advanced_settings()    {
    wxSizer *advanced_settings_sizer = new wxBoxSizer(wxVERTICAL);
    m_main_vertical_sizer->Add(advanced_settings_sizer, 0, wxEXPAND, 5);

    wxStaticText* advanced_settings_text = new wxStaticText(this, wxID_ANY, "Advanced settings:");
    set_text_size(advanced_settings_text, 20);
    advanced_settings_sizer->Add(advanced_settings_text, 0, wxCENTER, 5);


    wxCheckBox* use_grouping_checkbox = new wxCheckBox(this, wxID_ANY, "Use grouping");
    use_grouping_checkbox->SetValue(m_use_grouping);
    use_grouping_checkbox->SetToolTip("If checked, the photos will be grouped based on their time stamps and only one photo (or stack) from each group will be produced.");
    use_grouping_checkbox->Bind(wxEVT_CHECKBOX, [use_grouping_checkbox, this](wxCommandEvent&){
        const bool is_checked = use_grouping_checkbox->GetValue();
        m_use_grouping = is_checked;
    });

    advanced_settings_sizer->Add(use_grouping_checkbox, 0, wxEXPAND, 5);

    wxStaticText* grouping_time_text = new wxStaticText(this, wxID_ANY, "Grouping time interval [s]:");
    advanced_settings_sizer->Add(grouping_time_text, 0, wxEXPAND, 5);

    wxSpinCtrl* spin_ctrl_grouping_time = new wxSpinCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 180, 0);
    spin_ctrl_grouping_time->SetToolTip("Maximum time difference between photos in a group.");
    spin_ctrl_grouping_time->Bind(wxEVT_SPINCTRL, [spin_ctrl_grouping_time, this](wxCommandEvent&){
        int current_value = spin_ctrl_grouping_time->GetValue();
        m_grouping_time_interval = current_value;
    });
    advanced_settings_sizer->Add(spin_ctrl_grouping_time, 0, wxEXPAND, 5);

    wxStaticText *timestemp_offset_text = new wxStaticText(this, wxID_ANY, "Timestamp offset [s]:");
    advanced_settings_sizer->Add(timestemp_offset_text, 0, wxEXPAND, 5);

    wxSpinCtrl* spin_ctrl_timestamp_offset = new wxSpinCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -24*3600, 24*3600, 0);
    spin_ctrl_timestamp_offset->SetToolTip("If datetime in metadata is wrong, you can set an offset to correct it (it will be added to time from metadata). ");
    spin_ctrl_timestamp_offset->Bind(wxEVT_SPINCTRL, [spin_ctrl_timestamp_offset, this](wxCommandEvent&){
        int current_value = spin_ctrl_timestamp_offset->GetValue();
        m_timestamp_offset = current_value;
    });
    advanced_settings_sizer->Add(spin_ctrl_timestamp_offset, 0, wxEXPAND, 5);



    // Stacking options
    wxStaticText* stack_settings_text = new wxStaticText(this, wxID_ANY, "Stack settings:");
    set_text_size(stack_settings_text, 20);
    advanced_settings_sizer->Add(stack_settings_text, 0, wxCENTER, 5);

    wxCheckBox* stack_images_checkbox = new wxCheckBox(this, wxID_ANY, "Stack images");
    stack_images_checkbox->SetValue(m_stack_images);
    stack_images_checkbox->SetToolTip("If checked, the images will be stacked before producing the output image.");
    stack_images_checkbox->Bind(wxEVT_CHECKBOX, [stack_images_checkbox, this](wxCommandEvent&){
        const bool is_checked = stack_images_checkbox->GetValue();
        m_stack_images = is_checked;
    });
    advanced_settings_sizer->Add(stack_images_checkbox, 0, wxEXPAND, 5);

    wxStaticText* fraction_to_stack_text = new wxStaticText(this, wxID_ANY, "Fraction of images to stack: " + to_string(m_fraction_to_stack+0.00001).substr(0,4));
    advanced_settings_sizer->Add(fraction_to_stack_text, 0, wxEXPAND, 5);

    wxSlider* slider_stack_fraction = new wxSlider(this, wxID_ANY, m_fraction_to_stack*100, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
    slider_stack_fraction->SetToolTip("Fraction of images from each group which will be stacked.");
    slider_stack_fraction->Bind(wxEVT_SLIDER, [fraction_to_stack_text, slider_stack_fraction, this](wxCommandEvent&){
        const float fraction_to_stack = slider_stack_fraction->GetValue()/100.;
        const std::string new_label = "Fraction of images to stack: " + to_string(fraction_to_stack+0.00001).substr(0,4);
        fraction_to_stack_text->SetLabel(new_label);
        m_fraction_to_stack = fraction_to_stack;
    });
    advanced_settings_sizer->Add(slider_stack_fraction, 0, wxEXPAND, 5);
};

void AlignedImagesProducerGUI::stack_images_in_groups() const   {
    // TODO: Add calibration frames

    const FilelistHandler *filelist_handler = &m_parent->get_filelist_handler();

    // Light frames
    const vector<string>    &light_frames = filelist_handler->get_files(FileTypes::LIGHT);
    const vector<bool>      &files_are_checked = filelist_handler->get_files_checked(FileTypes::LIGHT);
    const vector<AlignmentFileInfo> &alignment_info_vec = filelist_handler->get_alignment_info();
    const vector<Metadata> &metadata_vec = filelist_handler->get_metadata();

    PhotoGroupingTool photo_grouping_tool;
    for (size_t i_file = 0; i_file < light_frames.size(); ++i_file) {
        const string &file = light_frames[i_file];
        const AlignmentFileInfo &alignment_info_gui = alignment_info_vec[i_file];
        const Metadata &metadata = metadata_vec[i_file];
        if (files_are_checked[i_file]) {
            photo_grouping_tool.add_file(file, metadata.timestamp, alignment_info_gui.ranking);
        }
    }
    photo_grouping_tool.define_maximum_time_difference_in_group(m_grouping_time_interval);
    photo_grouping_tool.run_grouping();
    const vector<vector<size_t>> &groups = photo_grouping_tool.get_groups_indices();

    const int tasks_total = groups.size();
    std::atomic<int> tasks_processed = 0;

    auto stack_lambda = [this, &light_frames, &files_are_checked, &alignment_info_vec, &metadata_vec, &groups, &tasks_processed](){

        for (const vector<size_t> &group : groups) {
            if (group.size() == 0) {
                continue;
            }

            FilelistHandler filelist_handler_group;
            const unsigned int n_selected_files = std::max<unsigned int>(1,group.size()*m_fraction_to_stack);
            bool contains_raw_files = false;
            for (unsigned int i_file = 0; i_file < n_selected_files; ++i_file) {
                const size_t i_file_group = group[i_file];
                const string &file = light_frames[i_file_group];
                const AlignmentFileInfo &alignment_info_gui = alignment_info_vec[i_file_group];

                if (!contains_raw_files) {
                    if (is_raw_file(file))  {
                        contains_raw_files = true;
                    }
                }

                filelist_handler_group.add_file(file, FileTypes::LIGHT, true, alignment_info_gui);
            }

            const StackSettings *stack_settings = m_parent->get_stack_settings();
            std::unique_ptr<StackerBase> stacker = get_configured_stacker(*stack_settings, filelist_handler_group);
            stacker->calculate_stacked_photo();

            const vector<vector<double>> &stacked_image_double = stacker->get_stacked_image();

            const string output_file_address = m_output_folder_address + "/" + AlignedImagesProducer::get_output_file_name(light_frames.at(group[0]));
            const int unix_time = metadata_vec.at(group[0]).timestamp;
            const bool use_green_correction = contains_raw_files;
            process_and_save_stacked_image(stacked_image_double, output_file_address, unix_time, use_green_correction, stacker->get_width(), stacker->get_height());
            cout << "Finished processing and saving stacked image for group: " << group[0] << endl;
            tasks_processed++;
        }
    };

    run_task_with_progress_dialog("Creating stacked aligned images", "Finished", "", tasks_processed, tasks_total, stack_lambda);
};


void AlignedImagesProducerGUI::process_and_save_stacked_image(  const std::vector<std::vector<double>> &stacked_image,
                                                                const std::string &output_file_address, int unix_time, bool use_green_correction, int original_width, int original_height)  const   {

        vector<vector<unsigned short>> cropped_image_ushort(stacked_image.size());
        int crop_top_left_x, crop_top_left_y, crop_width, crop_height;
        m_image_preview_crop_tool->get_crop_coordinates(&crop_top_left_x, &crop_top_left_y, &crop_width, &crop_height);
        unsigned short max_value = 0;
        for (unsigned int i_color = 0; i_color < stacked_image.size(); i_color++)   {
            const vector<double> &color_channel = stacked_image[i_color];

            for (int y = crop_top_left_y; y < crop_top_left_y + crop_height; y++) {
                for (int x = crop_top_left_x; x < crop_top_left_x + crop_width; x++) {
                    const unsigned int index_original = x + original_width*y;
                    cropped_image_ushort[i_color].push_back(color_channel[index_original]);
                    max_value = max<unsigned short>(max_value, color_channel[index_original]);
                }
            }
        }
        if (m_apply_color_stretcher) {
            m_color_stretcher.stretch_image(&cropped_image_ushort, max_value, true);
        }

        if (max_value > 255) {
            AlignedImagesProducer::scale_down_image(&cropped_image_ushort, max_value, 255);
        }

        if (use_green_correction) {
            AlignedImagesProducer::apply_green_correction(&cropped_image_ushort, 255);
        }

        if (!m_add_datetime) {
            crate_color_image(&cropped_image_ushort[0][0], &cropped_image_ushort[1][0], &cropped_image_ushort[2][0], crop_width, crop_height, output_file_address);
        }
        else {

            const string datetime = unix_time_to_string(unix_time);

            cv::Mat opencv_image = get_opencv_color_image(&cropped_image_ushort[0][0], &cropped_image_ushort[1][0], &cropped_image_ushort[2][0], crop_width, crop_height);
            cout << "Line 432\n";
            const float font_size = crop_width/1200.0;
            const float font_width = font_size*2;

            std::pair<float,float> datetime_position = m_aligned_images_producer ? m_aligned_images_producer->get_position_of_datetime() : std::pair<float,float>({0.6, 0.9});
            cv::putText(opencv_image, datetime, cv::Point(datetime_position.first* crop_width, datetime_position.second* crop_height), cv::FONT_HERSHEY_SIMPLEX, font_size, CV_RGB(255, 0, 0), font_width);

            cv::imwrite(output_file_address, opencv_image);
        }

};

