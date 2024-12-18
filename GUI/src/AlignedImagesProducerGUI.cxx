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

AlignedImagesProducerGUI::AlignedImagesProducerGUI(MyFrame *parent) :
        wxFrame(parent, wxID_ANY, "Produce aligned images", wxDefaultPosition, wxSize(1000, 1000)),
        m_parent(parent)    {

    m_color_stretcher.add_luminance_stretcher(std::make_shared<IndividualColorStretchingBlackCorrectionWhite>());

    m_main_vertical_sizer = new wxBoxSizer(wxVERTICAL);

    m_image_preview_crop_tool = make_unique<ImagePreviewCropTool>(this, 600, 400, 255, true);
    m_image_preview_crop_tool->set_stretcher(&m_color_stretcher);

    const InputFrame reference_frame = get_reference_frame();
    if (reference_frame != InputFrame()) {
        m_image_preview_crop_tool->read_preview_from_frame(reference_frame);
        m_image_preview_crop_tool->update_preview_bitmap();
    }


    m_image_preview_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_main_vertical_sizer->Add(m_image_preview_sizer, 2, wxCENTER, 5);

    m_image_preview_sizer->Add(m_image_preview_crop_tool->get_image_preview_bitmap(), 1, wxCENTER, 0);


    m_basic_settings_sizer = new wxBoxSizer(wxVERTICAL);
    m_main_vertical_sizer->Add(m_basic_settings_sizer, 1, wxEXPAND | wxALL, 5);
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
        if (crop_width > 0 && crop_height > 0) {
            m_aligned_images_producer->limit_output_image_size(crop_top_left_x, crop_top_left_y, crop_width, crop_height);
        }
        m_aligned_images_producer->set_maximal_output_image_size(1920, 1080);

        run_task_with_progress_dialog("Producing aligned images", "Finished", "", tasks_processed, tasks_total, [this](){
            if (m_apply_color_stretcher) {
                m_aligned_images_producer->set_image_stretching_function([this](std::vector<std::vector<unsigned short>> *image, unsigned short max_value){
                    m_color_stretcher.stretch_image(image, max_value, true);
                });
            }
            m_aligned_images_producer->produce_aligned_images(m_output_folder_address);

        }, "Producing video");
    });
    bottom_horizontal_sizer->Add(button_produce_images, 1, wxALL, 5);


    add_advanced_settings();
    add_video_settings();

    SetSizer(m_main_vertical_sizer);
};

void AlignedImagesProducerGUI::initialize_aligned_images_producer()   {
    const StackSettings *stack_settings = m_parent->get_stack_settings();
    const FilelistHandler *filelist_handler = &m_parent->get_filelist_handler();
    m_aligned_images_producer = make_unique<AlignedImagesProducer>(stack_settings->get_n_cpus(), stack_settings->get_max_memory());
    m_aligned_images_producer->set_add_datetime(m_add_datetime);
    m_aligned_images_producer->set_timestamp_offset(m_timestamp_offset);
    *m_aligned_images_producer->get_timelapse_video_settings() = m_timelapse_video_settings;

    // Light frames
    const vector<InputFrame>    &light_frames = filelist_handler->get_frames(FileTypes::LIGHT);
    const vector<bool>          &files_are_checked = filelist_handler->get_frames_checked(FileTypes::LIGHT);
    const vector<AlignmentFileInfo> &alignment_info_vec = filelist_handler->get_alignment_info();
    const vector<Metadata> &metadata_vec = filelist_handler->get_metadata();

    auto add_file_to_aligned_images_produced = [this, &light_frames, &files_are_checked, &alignment_info_vec, &metadata_vec](size_t i_file) {
        const InputFrame frame = light_frames[i_file];
        const AlignmentFileInfo &alignment_info_gui = alignment_info_vec[i_file];

        FileAlignmentInformation alignment_info;
        alignment_info.shift_x = alignment_info_gui.shift_x;
        alignment_info.shift_y = alignment_info_gui.shift_y;
        alignment_info.rotation_center_x = alignment_info_gui.rotation_center_x;
        alignment_info.rotation_center_y = alignment_info_gui.rotation_center_y;
        alignment_info.rotation = alignment_info_gui.rotation;
        alignment_info.ranking = alignment_info_gui.ranking;
        alignment_info.local_shifts_handler = alignment_info_gui.local_shifts_handler;

        if (has_valid_alignment(alignment_info_gui)) {
            m_aligned_images_producer->add_image(frame, alignment_info);
        }
    };

    if (m_use_grouping) {
        PhotoGroupingTool photo_grouping_tool;
        for (size_t i_file = 0; i_file < light_frames.size(); ++i_file) {
            const InputFrame frame = light_frames[i_file];
            const AlignmentFileInfo &alignment_info_gui = alignment_info_vec[i_file];
            const Metadata &metadata = metadata_vec[i_file];
            if (files_are_checked[i_file]) {
                if (has_valid_alignment(alignment_info_gui)) {
                    photo_grouping_tool.add_file(frame, metadata.timestamp, alignment_info_gui.ranking);
                }
            }
        }
        photo_grouping_tool.define_maximum_time_difference_in_group(m_grouping_time_interval);
        photo_grouping_tool.run_grouping();
        const vector<vector<size_t>> &groups = photo_grouping_tool.get_groups_indices();
        for (const vector<size_t> &group : groups) {
            if (group.size() > 0) {
                if (!m_stack_images) {
                    add_file_to_aligned_images_produced(group[0]);
                }
                else {
                    add_group_to_stack(group);
                }
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
        const vector<InputFrame> &calibration_frames = filelist_handler->get_frames(file_type);
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

    // Hot pixel identifier
    if (m_parent->get_hot_pixel_identifier()) {
        m_aligned_images_producer->set_hot_pixel_identifier(m_parent->get_hot_pixel_identifier());
    }
};

InputFrame AlignedImagesProducerGUI::get_reference_frame() const  {
    const FilelistHandler *filelist_handler = &m_parent->get_filelist_handler();

    // Light frames
    const vector<InputFrame>    &light_frames = filelist_handler->get_frames(FileTypes::LIGHT);
    const vector<bool>          &files_are_checked = filelist_handler->get_frames_checked(FileTypes::LIGHT);
    const vector<AlignmentFileInfo> &alignment_info_vec = filelist_handler->get_alignment_info();
    for (size_t i_file = 0; i_file < light_frames.size(); ++i_file) {
        if (files_are_checked[i_file]) {
            const InputFrame frame = light_frames[i_file];
            const AlignmentFileInfo &alignment_info_gui = alignment_info_vec[i_file];

            if (alignment_info_gui.ranking != 0 && alignment_info_gui.shift_x == 0 && alignment_info_gui.shift_y == 0 && alignment_info_gui.rotation == 0) {
                return frame;
            }
        }
    }

    double min_distance = 1000000;
    size_t i_min_distance = 0;
    for (size_t i_file = 0; i_file < light_frames.size(); ++i_file) {
        if (files_are_checked[i_file]) {
            const InputFrame frame = light_frames[i_file];
            const AlignmentFileInfo &alignment_info_gui = alignment_info_vec[i_file];

            const double distance = alignment_info_gui.shift_x*alignment_info_gui.shift_x + alignment_info_gui.shift_y*alignment_info_gui.shift_y;
            if (distance < min_distance) {
                min_distance = distance;
                i_min_distance = i_file;
            }
        }
    }
    if (min_distance < 1000000) {
        return light_frames[i_min_distance];
    }

    return InputFrame();
};

void AlignedImagesProducerGUI::add_exposure_correction_spin_ctrl()   {
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
            m_image_preview_crop_tool->update_preview_bitmap();
        }
    );
    m_exposure_correction_slider->add_sizer(m_basic_settings_sizer, 0, wxEXPAND, 5);
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

    m_fraction_to_stack_slider = make_unique<FloatingPointSlider>(
        this,
        "Fraction of images to stack: ",
        0,
        1,
        m_fraction_to_stack,
        0.01,
        2,
        [this](float fraction_to_stack){
            m_fraction_to_stack = fraction_to_stack;
        }
    );
    m_fraction_to_stack_slider->set_tool_tip("Fraction of images from each group which will be stacked.");
    m_fraction_to_stack_slider->add_sizer(advanced_settings_sizer, 0, wxEXPAND, 5);


};

void AlignedImagesProducerGUI::add_video_settings()   {
    wxSizer *video_settings_sizer = new wxBoxSizer(wxVERTICAL);
    m_main_vertical_sizer->Add(video_settings_sizer, 0, wxEXPAND, 5);

    wxStaticText* video_settings_text = new wxStaticText(this, wxID_ANY, "Video settings:");
    set_text_size(video_settings_text, 20);
    video_settings_sizer->Add(video_settings_text, 0, wxCENTER, 5);

    // fps
    wxStaticText *fps_text = new wxStaticText(this, wxID_ANY, "Frame rate per second (fps):");
    video_settings_sizer->Add(fps_text, 0, wxEXPAND, 5);
    m_timelapse_video_settings.set_fps(25);
    wxSpinCtrl* spin_fps = new wxSpinCtrl(this, wxID_ANY, "25", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 60, 25);
    spin_fps->Bind(wxEVT_SPINCTRL, [spin_fps, this](wxCommandEvent&){
        int current_value = spin_fps->GetValue();
        m_timelapse_video_settings.set_fps(current_value);
    });
    video_settings_sizer->Add(spin_fps, 0, wxEXPAND, 5);

    // n_repeat
    wxStaticText *n_repeat_text = new wxStaticText(this, wxID_ANY, "Number of repeats:");
    video_settings_sizer->Add(n_repeat_text, 0, wxEXPAND, 5);
    m_timelapse_video_settings.set_n_repeat(3);
    wxSpinCtrl* spin_n_repeat = new wxSpinCtrl(this, wxID_ANY, "3", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 50, 3);
    spin_n_repeat->Bind(wxEVT_SPINCTRL, [spin_n_repeat, this](wxCommandEvent&){
        int current_value = spin_n_repeat->GetValue();
        m_timelapse_video_settings.set_n_repeat(current_value);
    });
    video_settings_sizer->Add(spin_n_repeat, 0, wxEXPAND, 5);
};

void AlignedImagesProducerGUI::add_group_to_stack(const vector<size_t> &group) const   {
    // TODO: Add calibration frames

    const FilelistHandler *filelist_handler = &m_parent->get_filelist_handler();

    // Light frames
    const vector<InputFrame>    &light_frames = filelist_handler->get_frames(FileTypes::LIGHT);
    const vector<AlignmentFileInfo> &alignment_info_vec = filelist_handler->get_alignment_info();

    const unsigned int n_selected_files = std::max<unsigned int>(1,group.size()*m_fraction_to_stack);
    bool contains_raw_files = false;
    vector<InputFrame> selected_frames;
    vector<FileAlignmentInformation> selected_alignment_info;
    for (unsigned int i_file = 0; i_file < n_selected_files; ++i_file) {
        const size_t i_file_group = group[i_file];
        const InputFrame frame = light_frames[i_file_group];
        const AlignmentFileInfo &alignment_info_gui = alignment_info_vec[i_file_group];

        FileAlignmentInformation alignment_info;
        alignment_info.shift_x = alignment_info_gui.shift_x;
        alignment_info.shift_y = alignment_info_gui.shift_y;
        alignment_info.rotation_center_x = alignment_info_gui.rotation_center_x;
        alignment_info.rotation_center_y = alignment_info_gui.rotation_center_y;
        alignment_info.rotation = alignment_info_gui.rotation;
        alignment_info.input_frame = frame;

        if (!contains_raw_files) {
            if (is_raw_file(frame.get_file_address())) {
                contains_raw_files = true;
            }
        }

        selected_frames.push_back(frame);
        selected_alignment_info.push_back(alignment_info);
    }

    const StackSettings *stack_settings = m_parent->get_stack_settings();

    m_aligned_images_producer->add_image_group_to_stack(selected_frames, selected_alignment_info, *stack_settings);
};


void AlignedImagesProducerGUI::process_and_save_stacked_image(  const std::vector<std::vector<double>> &stacked_image,
                                                                const std::string &output_file_address, int unix_time, int original_width, int original_height)  const   {

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

        cv::Mat opencv_image = get_opencv_color_image(&cropped_image_ushort[0][0], &cropped_image_ushort[1][0], &cropped_image_ushort[2][0], crop_width, crop_height);
        const int max_height = 1080;
        const int max_width = 1920;
        // rescale image if it exceeds maximal size
        if (crop_width > max_width || crop_height > max_height) {
            const float scale_factor_width = max_width/static_cast<float>(crop_width);
            const float scale_factor_height = max_height/static_cast<float>(crop_height);

            const float scale_factor = min(scale_factor_width, scale_factor_height);
            cv::resize(opencv_image, opencv_image, cv::Size(), scale_factor, scale_factor);
        }

        if (m_add_datetime) {
            const string datetime = unix_time_to_string(unix_time);

            const int current_width = opencv_image.cols;
            const int current_height = opencv_image.rows;

            const float font_size = current_width/1200.0;
            const float font_width = font_size*2;

            cv::putText(opencv_image, datetime, cv::Point(0.6*current_width, 0.9*current_height), cv::FONT_HERSHEY_SIMPLEX, font_size, CV_RGB(255, 0, 0), font_width);

        }
        cv::imwrite(output_file_address, opencv_image);

};


bool AlignedImagesProducerGUI::has_valid_alignment(const AlignmentFileInfo &alignment_info) const  {
    return fabs(alignment_info.ranking) > 0.001;
};