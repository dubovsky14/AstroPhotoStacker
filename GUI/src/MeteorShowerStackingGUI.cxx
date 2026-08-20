#include "../headers/MeteorShowerStackingGUI.h"
#include "../headers/IndividualColorStretchingBlackCorrectionWhite.h"
#include "../headers/Common.h"
#include "../headers/SettingsCustomization.h"


#include "../headers/StackSettings.h"

#include "../../headers/AlignedImagesProducer.h"
#include "../../headers/CalibrationFrameBase.h"
#include "../../headers/PhotoAlignmentHandler.h"
#include "../../headers/ImageFilesInputOutput.h"
#include "../../headers/Common.h"


#include "../headers/MainFrame.h"
#include "../headers/ProgressBarWindow.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/progdlg.h>

#include <vector>
#include <string>



using namespace std;
using namespace AstroPhotoStacker;

MeteorShowerStackingGUI::MeteorShowerStackingGUI(MyFrame *parent, int n_cpus) :
        wxFrame(parent, wxID_ANY, "Meteor shower stacking", wxDefaultPosition, wxSize(1000, 1000), wxDEFAULT_FRAME_STYLE | wxMAXIMIZE) {

    m_parent = parent;
    m_meteor_shower_stacking_tool.set_n_cpus(n_cpus);
    m_window_size = wxGetDisplaySize();
    m_filelist_handler_gui_interface = m_parent->get_filelist_handler_gui_interface().get_filelist_with_checked_frames();


    m_exposure_stretcher.add_luminance_stretcher(std::make_shared<IndividualColorStretchingBlackCorrectionWhite>());

    m_main_vertical_sizer = new wxBoxSizer(wxVERTICAL);



    m_image_preview_width = m_window_size.GetWidth() * 0.45;
    m_image_preview_height = m_image_preview_width * 2. / 3.; // 3:2 aspect ratio

    m_image_preview = make_unique<ImagePreview>(this, m_image_preview_width, m_image_preview_height, 255, true);
    m_image_preview->set_stretcher(&m_exposure_stretcher);

    m_background_frame = get_reference_frame();
    if (m_background_frame != FrameAndGroup()) {
        m_image_preview->read_preview_from_frame(m_background_frame.input_frame);
        m_image_preview->update_preview_bitmap();
        m_currently_displayed_frame = m_background_frame;
    }

    // bind select/unselect cluster on click event
    m_image_preview->bind_right_click_event([this](int x, int y) {
        if (m_cluster_kd_tree == nullptr) {
            return;
        }
        const array<int,2> query_point = {x, y};
        vector<tuple<array<int, 2>, int>> nearest_neighbors = m_cluster_kd_tree->get_k_nearest_neighbors(query_point.data(), 1);
        if (nearest_neighbors.empty()) {
            return;
        }
        const int cluster_index = std::get<1>(nearest_neighbors[0]);

        const std::array<int, 2> cluster_pixel = std::get<0>(nearest_neighbors[0]);
        const int dx = cluster_pixel[0] - x;
        const int dy = cluster_pixel[1] - y;
        const int distance = sqrt(dx * dx + dy * dy);
        if (distance > 20) {
            return;
        }

        select_unselect_cluster_from_preview(cluster_index);
    });


    m_upper_part_sizer_horizontal = new wxBoxSizer(wxHORIZONTAL);
    m_main_vertical_sizer->Add(m_upper_part_sizer_horizontal, 0, wxEXPAND | wxALL, 5);


    m_image_preview_sizer = new wxBoxSizer(wxVERTICAL);
    m_upper_part_sizer_horizontal->Add(m_image_preview_sizer, 5, wxLEFT, 5);

    m_image_preview_sizer->Add(m_image_preview->get_image_preview_bitmap(), 1, wxCENTER, 0);
    add_exposure_correction_spin_ctrl();

    m_top_right_sizer = new wxBoxSizer(wxVERTICAL);
    m_upper_part_sizer_horizontal->Add(m_top_right_sizer, 4, wxEXPAND | wxALL, 5);
    add_list_of_clusters();
    add_cluster_buttons();
    add_cluster_settings();

    // bottom part with buttons and file list
    add_buttons();
    add_background_frame_selector();
    add_list_of_files();

    SetSizer(m_main_vertical_sizer);
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

void MeteorShowerStackingGUI::update_clusters_in_preview()   {
    m_image_preview->update_additional_layers_data();
    m_image_preview->update_preview_bitmap();
};

bool MeteorShowerStackingGUI::select_unselect_cluster_from_preview(int index_in_cluster_info) {
    if (m_cluster_id_to_index_in_gui.find(index_in_cluster_info) == m_cluster_id_to_index_in_gui.end()) {
        return false;
    }
    const int index_gui = m_cluster_id_to_index_in_gui[index_in_cluster_info];
    const bool was_checked = m_clusters_checkbox->IsChecked(index_gui);
    m_meteor_shower_stacking_tool.set_cluster_selected(m_currently_displayed_frame, index_in_cluster_info, !was_checked);
    m_clusters_checkbox->Check(index_gui, !was_checked);
    m_clusters_checkbox->SetSelection(index_gui);
    update_clusters_in_preview();
    return !was_checked;
};

void MeteorShowerStackingGUI::add_list_of_clusters() {
    m_clusters_checkbox = new wxCheckListBox(this, wxID_ANY);

    // set font to the one with fixed width characters
    m_clusters_checkbox->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    m_top_right_sizer->Add(m_clusters_checkbox, 1, wxEXPAND | wxALL, 5);
};

void MeteorShowerStackingGUI::update_cluster_list() {
    m_clusters_checkbox->Clear();
    m_cluster_id_to_index_in_gui.clear();
    m_index_in_gui_to_cluster_id.clear();

    if (m_currently_displayed_frame == FrameAndGroup()) {
        return;
    }

    FrameClusterInfo cluster_info = m_meteor_shower_stacking_tool.get_cluster_info(m_currently_displayed_frame);
    vector<vector<string>> cluster_labels_cells;
    vector<bool> cluster_selected;
    m_cluster_kd_tree = make_unique<KDTree<int,2,int>>();
    for (unsigned int i = 0; i < cluster_info.clusters.size(); ++i) {
        const unsigned int cluster_size = cluster_info.clusters[i].size();
        const float excentricity = cluster_info.clusters_excentricity[i];
        const bool is_selected = cluster_info.clusters_selected[i];

        if (excentricity < m_cluster_excentricity) {
            continue; // skip clusters with excentricity above the threshold
        }

        if (abs(cluster_info.clusters_correlation[i]) < m_cluster_correlation) {
            continue; // skip clusters with correlation below the threshold
        }

        const std::string cluster_id_label = "Cluster #" + std::to_string(i);
        const std::string cluster_size_label = "Size: " + std::to_string(cluster_size);
        const std::string cluster_excentricity_label = "Excentricity: " + std::to_string(excentricity);
        const std::string cluster_correlation_label = "Correlation: " + std::to_string(cluster_info.clusters_correlation[i]);

        vector<string> cluster_labels = {cluster_id_label, cluster_size_label, cluster_excentricity_label, cluster_correlation_label};
        cluster_labels_cells.push_back(cluster_labels);
        cluster_selected.push_back(is_selected);
        m_cluster_id_to_index_in_gui[i] = cluster_labels_cells.size() - 1;
        m_index_in_gui_to_cluster_id[cluster_labels_cells.size() - 1] = i;

        for (const std::tuple<int,int> &pixel : cluster_info.clusters[i]) {
            const int x = std::get<0>(pixel);
            const int y = std::get<1>(pixel);
            m_cluster_kd_tree->add_point({x,y}, i);
        }
    }
    m_cluster_kd_tree->build_tree_structure();

    vector<string> cluster_labels_formated = get_formated_table(cluster_labels_cells, 4*" "s);
    vector<wxString> cluster_labels_wx;
    for (const std::string &label : cluster_labels_formated) {
        cluster_labels_wx.push_back(label);
    }

    m_clusters_checkbox->Append(cluster_labels_wx);
    for (unsigned int i = 0; i < cluster_selected.size(); ++i) {
        m_clusters_checkbox->Check(i, cluster_selected[i]);
    }

    // on check/uncheck, update the cluster_info in m_meteor_shower_stacking_tool
    m_clusters_checkbox->Bind(wxEVT_CHECKLISTBOX, [this](wxCommandEvent& event) {
        const int index_gui = event.GetInt();
        if (m_index_in_gui_to_cluster_id.find(index_gui) == m_index_in_gui_to_cluster_id.end()) {
            return;
        }
        const int cluster_index = m_index_in_gui_to_cluster_id[index_gui];

        FrameClusterInfo cluster_info = m_meteor_shower_stacking_tool.get_cluster_info(m_currently_displayed_frame);
        const bool is_checked = m_clusters_checkbox->IsChecked(index_gui);
        const bool was_checked = cluster_info.clusters_selected[cluster_index];
        if (is_checked == was_checked) {
            return; // no change
        }
        m_meteor_shower_stacking_tool.set_cluster_selected(m_currently_displayed_frame, cluster_index, is_checked);
        update_clusters_in_preview();
    });
};


void MeteorShowerStackingGUI::add_cluster_buttons()  {
    m_cluster_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_top_right_sizer->Add(m_cluster_buttons_sizer, 0, wxEXPAND | wxALL, 5);

    auto add_button = [this](const std::string &label, const std::function<void()> &on_click) {
        wxButton *button = new wxButton(this, wxID_ANY, label);
        button->Bind(wxEVT_BUTTON, [on_click](wxCommandEvent&) {
            on_click();
        });
        m_cluster_buttons_sizer->Add(button, 1, wxEXPAND | wxALL, 5);
        return button;
    };
    m_button_show_cluster = add_button(m_show_clusters ? "Hide clusters" : "Show clusters", [this]() {
        m_show_clusters = !m_show_clusters;
        if (m_show_clusters) {
            m_button_show_cluster->SetLabel("Hide clusters");
        }
        else {
            m_button_show_cluster->SetLabel("Show clusters");
        }
        update_clusters_in_preview();
    });

    m_button_recalculate_clusters = add_button("Recalculate clusters", [this]() {
        m_meteor_shower_stacking_tool.recalculate_clusters(m_currently_displayed_frame, m_cluster_threshold, true);
        update_cluster_list();
        update_image_preview_file(m_previously_selected_frame_index);
    });

    m_button_recalculate_clusters_for_all_images = add_button("Recalculate clusters for all images", [this]() {
        vector<FrameAndGroup> frames_to_process;
        for (const FrameInfo &frame_info : m_filelist_handler_gui_interface.get_checked_frames_of_type(FrameType::LIGHT)) {
            FrameAndGroup frame_and_group;
            frame_and_group.input_frame = frame_info.input_frame;
            frame_and_group.group_number = frame_info.group_number;
            frames_to_process.push_back(frame_and_group);
        }
        const int tasks_total = frames_to_process.size();
        const std::atomic<int> &tasks_processed = m_meteor_shower_stacking_tool.get_tasks_processed();
        run_task_with_progress_dialog(  "Recalculating clusters for all images...",
                                "Finished",
                                "",
                                tasks_processed,
                                tasks_total,
                                [this, frames_to_process](){
                                    m_meteor_shower_stacking_tool.recalculate_clusters(frames_to_process, m_cluster_threshold);
                                },
                                "");
        update_cluster_list();
        update_image_preview_file(m_previously_selected_frame_index);
    });
};

void MeteorShowerStackingGUI::add_cluster_settings() {
    // spacer
    m_top_right_sizer->AddSpacer(10);

    m_cluster_threshold_slider = make_unique<FloatingPointSlider>(
        this,
        "Cluster threshold: ",
        0.0,
        0.3,
        m_cluster_threshold,
        0.0002,
        4,
        [this](float value){
            m_cluster_threshold = value;
        }
    );
    m_cluster_threshold_slider->add_sizer(m_top_right_sizer, 0, wxEXPAND, 1);

    m_cluster_excentricity_slider = make_unique<FloatingPointSlider>(
        this,
        "Cluster excentricity: ",
        0.0,
        50,
        m_cluster_excentricity,
        1,
        1,
        [this](float value){
            m_cluster_excentricity = value;
        }
    );
    m_cluster_excentricity_slider->add_sizer(m_top_right_sizer, 0, wxEXPAND, 1);

    m_cluster_correlation_slider = make_unique<FloatingPointSlider>(
        this,
        "Cluster correlation: ",
        0.0,
        1.0,
        m_cluster_correlation,
        0.001,
        3,
        [this](float value){
            m_cluster_correlation = value;
        }
    );
    m_cluster_correlation_slider->add_sizer(m_top_right_sizer, 0, wxEXPAND, 1);
};

void MeteorShowerStackingGUI::add_buttons()  {
    m_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_main_vertical_sizer->Add(m_buttons_sizer, 0, wxEXPAND | wxALL, 5);

    auto add_button = [this](const std::string &label, const std::function<void()> &on_click) {
        wxButton *button = new wxButton(this, wxID_ANY, label);
        button->Bind(wxEVT_BUTTON, [on_click](wxCommandEvent&) {
            on_click();
        });
        m_buttons_sizer->Add(button, 1, wxEXPAND | wxALL, 5);
        return button;
    };

    m_button_check_all = add_button("Check all", [this]() {
        update_checked_files_in_filelist();
        if (m_button_check_all->GetLabel() == "Uncheck all") {
            for (unsigned int i = 0; i < m_files_checkbox->GetCount(); ++i) {
                m_files_checkbox->Check(i, false);
            }
            m_button_check_all->SetLabel("Check all");
        }
        else {
            for (unsigned int i = 0; i < m_files_checkbox->GetCount(); ++i) {
                m_files_checkbox->Check(i);
            }
            m_button_check_all->SetLabel("Uncheck all");
        }
        update_checked_files_in_filelist();
    });

    m_button_remove_checked = add_button("Remove checked", [this]() {
        // get checked files:
        wxArrayInt checked_indices;
        m_files_checkbox->GetCheckedItems(checked_indices);

        // remove checked files from m_filelist_handler_gui_interface
        for (int i = checked_indices.GetCount() - 1; i >= 0; --i) {
            const std::string option = m_files_checkbox->GetString(checked_indices[i]).ToStdString();
            m_filelist_handler_gui_interface.remove_frame(checked_indices[i]);
        }

        // update m_files_checkbox
        update_files_to_stack_checkbox();
    });

    m_button_stack = add_button("Stack files", [this]() {
        m_meteor_shower_stacking_tool.stack_frames(m_filelist_handler_gui_interface, m_background_frame);
    });

    m_button_show_stacked_image = add_button("Show stacked image", [this]() {
        if (m_current_preview_is_stack) {
            if (m_previously_selected_frame_index < 0)  {
                return;
            }
            update_image_preview_file(m_previously_selected_frame_index);
            m_current_preview_is_stack = false;
            m_button_show_stacked_image->SetLabel("Show stacked image");
        }
        else {
            int width, height;
            const std::vector<std::vector<float>> &stacked_image = m_meteor_shower_stacking_tool.get_stacked_image(&width, &height);
            const vector<vector<double>> stacked_image_double = convert_vector_2d<float,double>(stacked_image);
            m_image_preview->read_preview_from_stacked_image(stacked_image_double, width, height);
            m_image_preview->update_preview_bitmap();
            m_currently_displayed_frame = FrameAndGroup();
            m_current_preview_is_stack = true;
            m_button_show_stacked_image->SetLabel("Show original image");
        }
    });

    m_button_save_stacked_image = add_button("Save stacked image", [this]() {
        int width, height;
        const std::vector<std::vector<float>> &stacked_image_float = m_meteor_shower_stacking_tool.get_stacked_image(&width, &height);

        const vector<vector<double>> stacked_image_double = convert_vector_2d<float,double>(stacked_image_float);
        const std::string default_path = m_parent->get_recent_paths_handler().get_recent_file_path(FrameType::LIGHT, "");
        wxFileDialog dialog(this, "Save stacked file", "", default_path, "*['.tif']", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dialog.ShowModal() == wxID_OK) {
            std::string file_address = dialog.GetPath().ToStdString();

            // if the extension is not .tif, add it
            if (file_address.substr(file_address.size()-4) != ".tif") {
                file_address += ".tif";
            }

            AstroPhotoStacker::StackerBase::save_stacked_photo(file_address,
                                            stacked_image_double,
                                            width,
                                            height,
                                            CV_16UC3);
        }
    });
};

FrameAndGroup MeteorShowerStackingGUI::get_reference_frame() const  {
    const FilelistHandlerGUIInterface *filelist_handler_gui_interface = &m_parent->get_filelist_handler_gui_interface();

    // Light frames
    const vector<FrameInfo>    light_frames = filelist_handler_gui_interface->get_checked_frames_of_type(FrameType::LIGHT);
    double min_distance = 1e20;
    const float x_orig(3000), y_orig(2000);
    FrameAndGroup best_frame;
    for (const FrameInfo &frame_info : light_frames) {
        const InputFrame &frame                     = frame_info.input_frame;
        const AlignmentResultBase &alignment_result = *frame_info.alignment_result;
        float x(x_orig), y(y_orig);
        alignment_result.transform_from_reference_to_shifted_frame(&x, &y);

        const double distance = (x - x_orig) * (x - x_orig) + (y - y_orig) * (y - y_orig);
        if (distance < min_distance) {
            min_distance = distance;
            best_frame.input_frame = frame;
            best_frame.group_number = frame_info.group_number;
        }
    }
    if (min_distance < 1e19) {
        return best_frame;
    }

    return FrameAndGroup();
};

void MeteorShowerStackingGUI::add_background_frame_selector() {
    m_indices_frames_to_align.clear();
    m_available_light_frames.clear();
    m_available_light_frames_strings.clear();

    const std::vector<std::pair<std::string, FrameID>> &all_frames = m_filelist_handler_gui_interface.get_shown_frames();
    std::vector<std::pair<std::string, FrameID>> light_frames;
    for (size_t i = 0; i < all_frames.size(); ++i) {
        const pair<string,FrameID> &frame = all_frames[i];
        if (frame.second.type != FrameType::LIGHT)  continue;
        light_frames.push_back(frame);
    }

    const unsigned int max_number_of_frames_for_gui = 2000; // without this, it would freeze for planetary videos
    const bool show_full_frame_paths = SettingsCustomization::get_instance().other_settings_customization.show_full_frame_paths;
    int current_selection = 0;
    for (unsigned int i = 0; i < light_frames.size(); ++i) {
        if (i < max_number_of_frames_for_gui) {
            m_available_light_frames_strings.push_back(light_frames[i].second.input_frame.to_gui_string(show_full_frame_paths));
        }
        m_indices_frames_to_align.push_back(i);
        FrameAndGroup frame_and_group;
        frame_and_group.input_frame = light_frames[i].second.input_frame;
        frame_and_group.group_number = light_frames[i].second.group_number;
        m_available_light_frames.push_back(frame_and_group);

        if (frame_and_group == m_background_frame) {
            current_selection = i;
        }
    }

    wxStaticText* select_background_frame_text = new wxStaticText(this, wxID_ANY, "Background frame:");
    select_background_frame_text->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    wxChoice* choice_box_background_frame = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_available_light_frames_strings.size(), m_available_light_frames_strings.data());
    choice_box_background_frame->SetSelection(current_selection);
    m_background_frame = m_available_light_frames[current_selection];
    choice_box_background_frame->Bind(wxEVT_CHOICE, [this, choice_box_background_frame](wxCommandEvent&){
        int current_selection = choice_box_background_frame->GetSelection();
        m_background_frame = m_available_light_frames[current_selection];
    });
    m_main_vertical_sizer->Add(select_background_frame_text, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);
    m_main_vertical_sizer->Add(choice_box_background_frame, 0,  wxEXPAND, 5);
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
    FrameAndGroup frame_and_group;
    frame_and_group.input_frame = m_filelist_handler_gui_interface.get_frame_by_index(frame_index).input_frame;
    frame_and_group.group_number = m_filelist_handler_gui_interface.get_frame_by_index(frame_index).group_number;

    m_currently_displayed_frame = frame_and_group;
    update_cluster_list();

    m_image_preview->add_layer("cluster_mask",
                            [this, frame_and_group](std::vector<std::vector<PixelType>> *image_data, int width, int height) {
                                if (!m_show_clusters) return;

                                FrameClusterInfo cluster_info = m_meteor_shower_stacking_tool.get_cluster_info(frame_and_group);
                                for (const auto &[i_cluster, i_cluster_gui] : m_cluster_id_to_index_in_gui) {

                                    for (const auto &pixel : cluster_info.clusters[i_cluster]) {
                                        const int x = std::get<0>(pixel);
                                        const int y = std::get<1>(pixel);
                                        const size_t pixel_index = y * width + x;
                                        if (x >= 0 && x < width && y >= 0 && y < height) {
                                            (*image_data)[0][pixel_index] = cluster_info.clusters_selected[i_cluster] ? 0   : 255;
                                            (*image_data)[1][pixel_index] = cluster_info.clusters_selected[i_cluster] ? 255 : 0;
                                            (*image_data)[2][pixel_index] = 0;
                                        }
                                    }
                                }
                            });

    m_image_preview->read_preview_from_frame(frame_and_group.input_frame);
    m_image_preview->update_additional_layers_data();
    m_image_preview->update_preview_bitmap();

    FrameClusterInfo cluster_info = m_meteor_shower_stacking_tool.get_cluster_info(frame_and_group);
    const float threshold = cluster_info.cluster_fraction_threshold;
    if (threshold > 0)  {
        m_cluster_threshold_slider->set_value(threshold);
    }
    m_previously_selected_frame_index = frame_index;
    m_button_show_stacked_image->SetLabel("Show stacked image");
    m_current_preview_is_stack = false;
};


bool MeteorShowerStackingGUI::update_checked_files_in_filelist() {
    wxArrayInt checked_indices;
    m_files_checkbox->GetCheckedItems(checked_indices);
    bool updated = false;
    for (int i = 0; i < m_filelist_handler_gui_interface.get_number_of_all_frames(); i++) {
        const bool file_checked_in_checkbox = m_files_checkbox->IsChecked(i);
        const bool file_checked_in_filelist = m_filelist_handler_gui_interface.frame_is_checked(i);
        const FrameID frame_info = m_filelist_handler_gui_interface.get_frame_by_index(i);
        if (frame_info.type != FrameType::LIGHT) {
            continue;
        }
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
