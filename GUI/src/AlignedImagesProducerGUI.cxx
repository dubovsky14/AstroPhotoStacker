#include "../headers/AlignedImagesProducerGUI.h"
#include "../headers/IndividualColorStretchingBlackCorrectionWhite.h"
#include "../headers/Common.h"
#include "../headers/PhotoGroupingTool.h"
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

AlignedImagesProducerGUI::AlignedImagesProducerGUI(MyFrame *parent, const PostProcessingTool *post_processing_tool) :
        wxFrame(parent, wxID_ANY, "Produce aligned images", wxDefaultPosition, wxSize(1000, 1000)),
        m_parent(parent), m_post_processing_tool(post_processing_tool)    {

    m_color_stretcher.add_luminance_stretcher(std::make_shared<IndividualColorStretchingBlackCorrectionWhite>());

    m_main_vertical_sizer = new wxBoxSizer(wxVERTICAL);

    m_image_preview_crop_tool = make_unique<ImagePreviewCropTool>(this, 600, 400, 255, true);
    m_image_preview_crop_tool->set_stretcher(&m_color_stretcher);

    // TODO: add alignment preview
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
                m_aligned_images_producer->set_image_stretching_function([this](std::vector<std::vector<PixelType>> *image, PixelType max_value){
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
    const FilelistHandlerGUIInterface *filelist_handler_gui_interface = &m_parent->get_filelist_handler_gui_interface();
    m_aligned_images_producer = make_unique<AlignedImagesProducer>(stack_settings->get_n_cpus(), stack_settings->get_max_memory());
    m_aligned_images_producer->set_add_datetime(m_add_datetime);
    m_aligned_images_producer->set_post_processing_tool([this](const std::vector<std::vector<PixelType>> &image, int width, int height){
        return m_post_processing_tool->post_process_image(image, width, height);
    });
    m_aligned_images_producer->set_timestamp_offset(m_timestamp_offset);
    *m_aligned_images_producer->get_timelapse_video_settings() = m_timelapse_video_settings;

    // Light frames
    const vector<FrameInfo>    &light_frames = filelist_handler_gui_interface->get_checked_frames_of_type(FrameType::LIGHT);

    const std::map<int, std::vector<std::shared_ptr<const CalibrationFrameBase> > > calibration_handlers_map = get_calibration_frame_handlers_map();

    auto add_file_to_aligned_images_produced = [this, &light_frames, calibration_handlers_map](size_t i_file) {
        const InputFrame frame = light_frames[i_file].input_frame;
        const AlignmentResultBase &alignment_result = *light_frames[i_file].alignment_result;
        const int calibration_group_number = light_frames[i_file].group_number;
        std::vector<std::shared_ptr<const CalibrationFrameBase> > calibration_frame_handlers = {};

        if (calibration_handlers_map.find(i_file) != calibration_handlers_map.end()) {
            calibration_frame_handlers = calibration_handlers_map.at(calibration_group_number);
        }

        if (alignment_result.is_valid()) {
            m_aligned_images_producer->add_image(frame, alignment_result, calibration_frame_handlers);
        }
    };

    if (m_use_grouping) {
        PhotoGroupingTool photo_grouping_tool;
        for (const FrameInfo &frame_info : light_frames) {
            const InputFrame &frame = frame_info.input_frame;
            const AlignmentResultBase &alignment_result = *frame_info.alignment_result;
            const Metadata &metadata = frame_info.metadata;
            if (alignment_result.is_valid()) {
                photo_grouping_tool.add_file(frame, metadata.timestamp, alignment_result.get_ranking_score());
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
                    add_group_to_stack(group, light_frames, calibration_handlers_map);
                }
            }
        }
    }
    else {
        for (size_t i_file = 0; i_file < light_frames.size(); ++i_file) {
            add_file_to_aligned_images_produced(i_file);
        }
    }

    // Hot pixel identifier
    if (m_parent->get_hot_pixel_identifier()) {
        m_aligned_images_producer->set_hot_pixel_identifier(m_parent->get_hot_pixel_identifier());
    }
};

InputFrame AlignedImagesProducerGUI::get_reference_frame() const  {
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

void AlignedImagesProducerGUI::add_group_to_stack(  const vector<size_t> &group,
                                                    const vector<FrameInfo> &light_frames,
                                                    const std::map<int, std::vector<std::shared_ptr<const CalibrationFrameBase> > > &calibration_handlers_map) const   {

    const unsigned int n_selected_files = std::max<unsigned int>(1,group.size()*m_fraction_to_stack);
    vector<InputFrame> selected_frames;
    vector<unique_ptr<AlignmentResultBase>> selected_alignment_info;
    vector<std::vector<std::shared_ptr<const CalibrationFrameBase>>> selected_calibration_frame_handlers;
    for (unsigned int i_file = 0; i_file < n_selected_files; ++i_file) {
        const size_t i_file_group = group[i_file];
        const FrameInfo &frame_info = light_frames[i_file_group];
        const InputFrame &frame = frame_info.input_frame;
        unique_ptr<AlignmentResultBase> alignment_result = frame_info.alignment_result->clone();

        selected_frames.push_back(frame);
        selected_alignment_info.push_back(std::move(alignment_result));
        selected_calibration_frame_handlers.push_back(calibration_handlers_map.at(frame_info.group_number));
    }

    const StackSettings *stack_settings = m_parent->get_stack_settings();

    m_aligned_images_producer->add_image_group_to_stack(selected_frames, selected_alignment_info, *stack_settings, &selected_calibration_frame_handlers);
};

std::map<int, std::vector<std::shared_ptr<const CalibrationFrameBase> > > AlignedImagesProducerGUI::get_calibration_frame_handlers_map() const    {
    std::map<int, std::vector<std::shared_ptr<const CalibrationFrameBase> > > calibration_frame_handlers_map;

    const FilelistHandlerGUIInterface *filelist_handler_gui_interface = &m_parent->get_filelist_handler_gui_interface();
    const std::vector<int> group_indices = filelist_handler_gui_interface->get_group_numbers();

    for (const int group_index : group_indices) {
        calibration_frame_handlers_map[group_index] = std::vector<std::shared_ptr<const CalibrationFrameBase> >();
        for (const FrameType &file_type : {FrameType::DARK, FrameType::FLAT, FrameType::BIAS}) {
            const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &calibration_frames = filelist_handler_gui_interface->get_frames(file_type, group_index);
            if (calibration_frames.size() > 0) {
                shared_ptr<const CalibrationFrameBase> calibration_frame_handler = nullptr;
                switch (file_type) {
                    case FrameType::DARK:
                        calibration_frame_handler = make_shared<DarkFrameHandler>(calibration_frames.begin()->first);
                        break;
                    case FrameType::FLAT:
                        calibration_frame_handler = make_shared<FlatFrameHandler>(calibration_frames.begin()->first);
                        break;
                    default:
                        break;
                }
                if (calibration_frame_handler) {
                    calibration_frame_handlers_map[group_index].push_back(calibration_frame_handler);
                }
            }
            }
    }
    return calibration_frame_handlers_map;
};