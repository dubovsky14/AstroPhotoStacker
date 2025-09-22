#include "../headers/MainFrame.h"
#include "../headers/AlignmentFrame.h"
#include "../headers/ListFrame.h"
#include "../headers/StackerConfigureTool.h"
#include "../headers/ThreePointSlider.h"
#include "../headers/AlignedImagesProducerGUI.h"
#include "../headers/ProgressBarWindow.h"
#include "../headers/PostProcessingToolGUI.h"
#include "../headers/MetadataManager.h"


#include "../headers/IndividualColorStretchingBlackMidtoneWhite.h"
#include "../headers/IndividualColorStretchingBlackCorrectionWhite.h"

#include "../../headers/Common.h"
#include "../../headers/raw_file_reader.h"
#include "../../headers/thread_pool.h"
#include "../../headers/StackerBase.h"
#include "../../headers/VideoReader.h"
#include "../../headers/AlignmentPointBoxGrid.h"
#include "../../headers/ConvertToFitFile.h"

#include <wx/spinctrl.h>
#include <wx/progdlg.h>
#include <wx/artprov.h>

#include <iostream>
#include <filesystem>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace AstroPhotoStacker;

std::string MyFrame::s_gui_folder_path = std::filesystem::path(__FILE__).parent_path().parent_path().string() + "/";

bool MyApp::OnInit()    {
    MyFrame *frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "AstroPhotoStacker GUI") {

    // to capture keyboard events
    m_main_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

    m_recent_paths_handler = make_unique<RecentPathsHandler>(s_gui_folder_path + "data/recent_paths/");
    m_stack_settings = make_unique<StackSettingsSaver>(s_gui_folder_path + "data/stack_settings.txt");

    // full screen
    SetSize(wxGetDisplaySize());


    m_sizer_main_frame  = new wxBoxSizer(wxVERTICAL);
    m_sizer_top         = new wxBoxSizer(wxHORIZONTAL);
    m_sizer_button_bar  = new wxBoxSizer(wxHORIZONTAL);
    m_sizer_main_frame->Add(m_sizer_top,        9, wxEXPAND | wxALL, 5);
    m_sizer_main_frame->Add(m_sizer_button_bar, 1, wxEXPAND | wxALL, 5);

    m_sizer_top_left    = new wxBoxSizer(wxVERTICAL);
    m_sizer_top_center  = new wxBoxSizer(wxVERTICAL);
    m_sizer_top_right   = new wxBoxSizer(wxVERTICAL);

    m_sizer_top->Add(m_sizer_top_left,   1, wxEXPAND | wxALL, 5);
    m_sizer_top->Add(m_sizer_top_center, 1, wxEXPAND | wxALL, 5);
    m_sizer_top->Add(m_sizer_top_right,  1, wxEXPAND | wxALL, 5);

    SetSizer(m_sizer_main_frame);
    //SetSizer(m_sizer_top);

    m_color_stretcher.add_luminance_stretcher(std::make_shared<IndividualColorStretchingBlackMidtoneWhite>());
    m_color_stretcher.add_luminance_stretcher(std::make_shared<IndividualColorStretchingBlackCorrectionWhite>());

    add_files_to_stack_checkbox();

    add_button_bar();

    add_stack_settings_preview();
    add_upper_middle_panel();
    add_image_settings();

    add_menu_bar();

    CreateStatusBar();
    SetStatusText("Author: Michal Dubovsky");

}

void MyFrame::add_file_menu()  {
    m_file_menu = new wxMenu;

    int id = unique_counter();
    m_file_menu->Append(id, "Open light frames", "Open light frames");
    Bind(wxEVT_MENU, &MyFrame::on_open_lights, this, id);

    id = unique_counter();
    m_file_menu->Append(id, "Open flat frames", "Open flat frames");
    Bind(wxEVT_MENU, &MyFrame::on_open_flats, this, id);

    id = unique_counter();
    m_file_menu->Append(id, "Open dark frames", "Open dark frames");
    Bind(wxEVT_MENU, &MyFrame::on_open_darks, this, id);

    id = unique_counter();
    m_file_menu->Append(id, "Save stacked file", "Save stacked file");
    Bind(wxEVT_MENU, &MyFrame::on_save_stacked, this, id);

    id = unique_counter();
    m_file_menu->Append(id, "Save selected files as FIT", "Save selected files as FIT");
    Bind(wxEVT_MENU, &MyFrame::on_save_selected_as_fit, this, id);

    m_file_menu->Append(wxID_EXIT);
    Bind(wxEVT_MENU, &MyFrame::on_exit,  this, wxID_EXIT);

    m_menu_bar->Append(m_file_menu, "&File");

};


void MyFrame::add_filelist_menu()    {
    wxMenu *filelist_menu = new wxMenu;

    int id = unique_counter();
    filelist_menu->Append(id, "Save filelist", "Save filelist");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        const std::string default_path = m_recent_paths_handler->get_recent_file_path(FrameType::LIGHT, "");
        wxFileDialog dialog(this, "Save filelist", "", default_path + "/filelist.txt", "*['.txt']", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dialog.ShowModal() == wxID_OK) {
            const std::string file_address = dialog.GetPath().ToStdString();
            m_filelist_handler_gui_interface.save_filelist_to_file(file_address);
        }
    }, id);

    id = unique_counter();
    filelist_menu->Append(id, "Load filelist", "Load filelist");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        const std::string default_path = m_recent_paths_handler->get_recent_file_path(FrameType::LIGHT, "");
        wxFileDialog dialog(this, "Load filelist", "", default_path, "*['.txt']", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dialog.ShowModal() == wxID_OK) {
            const std::string file_address = dialog.GetPath().ToStdString();
            m_filelist_handler_gui_interface.load_filelist_from_file(file_address);
            update_alignment_status();
            update_files_to_stack_checkbox();
            update_input_numbers_overview();
        }
    }, id);

    m_menu_bar->Append(filelist_menu, "&Filelist");
};

void MyFrame::add_alignment_menu()  {
    wxMenu *alignment_menu = new wxMenu;

    int id = unique_counter();
    alignment_menu->Append(id, "Save alignment info", "Save alignment info");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        const std::string default_path = m_recent_paths_handler->get_recent_file_path(FrameType::LIGHT, "");
        wxFileDialog dialog(this, "Save alignment info", "", default_path + "/alignment.txt", "*['.txt']", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dialog.ShowModal() == wxID_OK) {
            const std::string file_address = dialog.GetPath().ToStdString();
            m_filelist_handler_gui_interface.save_alignment_to_file(file_address);
        }
    }, id);

    id = unique_counter();
    alignment_menu->Append(id, "Load alignment info", "Load alignment info");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        const std::string default_path = m_recent_paths_handler->get_recent_file_path(FrameType::LIGHT, "");
        wxFileDialog dialog(this, "Load alignment info", "", default_path, "*['.txt']", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dialog.ShowModal() == wxID_OK) {
            const std::string file_address = dialog.GetPath().ToStdString();
            m_filelist_handler_gui_interface.load_alignment_from_file(file_address);
            update_alignment_status();
        }
    }, id);


    id = unique_counter();
    alignment_menu->Append(id, "Enable stack without alignment", "Enable stack without alignment");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        m_filelist_handler_gui_interface.set_dummy_alignment_for_all_frames();
        update_alignment_status();
    }, id);


    const std::string alignment_boxes_preview_name = "alignment_boxes";
    id = unique_counter();
    alignment_menu->Append(id, "Show alignment boxes", "Show alignment boxes");
    Bind(wxEVT_MENU, [this, alignment_boxes_preview_name](wxCommandEvent&){
        auto draw_boxes_lambda = [this, alignment_boxes_preview_name](std::vector<std::vector<short int>> *image_data, int width, int height) {
            cout << "Drawing " << m_alignment_box_vector_storage.size() <<  " alignment boxes" << endl;
            AstroPhotoStacker::AlignmentPointBoxGrid::draw_boxes_into_image(
                m_alignment_box_vector_storage,
                image_data,
                width,
                height,
                {0, 255, 0},
                {255,0,0});
        };
        m_current_preview->add_layer(alignment_boxes_preview_name, draw_boxes_lambda);
    }, id);

    id = unique_counter();
    alignment_menu->Append(id, "Hide alignment boxes", "Hide alignment boxes");
    Bind(wxEVT_MENU, [this, alignment_boxes_preview_name](wxCommandEvent&){
        m_current_preview->remove_layer(alignment_boxes_preview_name);
    }, id);


    m_menu_bar->Append(alignment_menu, "&Alignment");
};

void MyFrame::add_group_menu()   {
    wxMenu *group_menu = new wxMenu;

    int id = unique_counter();
    group_menu->Append(id, "Show current group", "Show current group");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        const std::string current_group_string = "Current group number: " + std::to_string(m_current_group);
        wxMessageDialog dialog(this, current_group_string, "Current group");
        dialog.ShowModal();
    }, id);

    id = unique_counter();
    group_menu->Append(id, "Create new group", "Create new group");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        m_current_group++;
    }, id);

    id = unique_counter();
    group_menu->Append(id, "Change current group", "Change current group");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        const vector<int> group_numbers = m_filelist_handler_gui_interface.get_group_numbers();
        wxArrayString group_names;
        for (const int group_number : group_numbers) {
            group_names.Add(std::to_string(group_number));
        }
        wxSingleChoiceDialog dialog(this, "Select group number", "Group number", group_names);
        if (dialog.ShowModal() == wxID_OK) {
            const int selection = dialog.GetSelection();
            if (selection != -1) {
                m_current_group = group_numbers[selection];
            }
        }
    }, id);

    id = unique_counter();
    group_menu->Append(id, "Remove group", "Remove group");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        const vector<int> group_numbers = m_filelist_handler_gui_interface.get_group_numbers();
        wxArrayString group_names;
        for (const int group_number : group_numbers) {
            group_names.Add(std::to_string(group_number));
        }
        wxSingleChoiceDialog dialog(this, "Select group to delete", "Group number", group_names);
        if (dialog.ShowModal() == wxID_OK) {
            const int selection = dialog.GetSelection();
            if (selection != -1) {
                m_filelist_handler_gui_interface.remove_group(group_numbers[selection]);
            }
            update_files_to_stack_checkbox();
        }
    }, id);

    id = unique_counter();
    group_menu->Append(id, "What are groups?", "What are groups?");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        string message = "Groups allow a user to combine light frames taken in different conditions, ";
        message += " where different calibration frames are used for different groups.";
        message += " A typical example: stacking an object photographed on a telescope over multiple nights, each night with different flat frames.";
        wxMessageDialog dialog(this, message, "");
        dialog.ShowModal();
    }, id);

    m_menu_bar->Append(group_menu, "&Groups");
};

void MyFrame::add_hot_pixel_menu()  {
    wxMenu *hot_pixel_menu = new wxMenu;

    int id = unique_counter();
    hot_pixel_menu->Append(id, "Save hot pixel info", "Save hot pixel info");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        if (m_hot_pixel_identifier == nullptr)  {
            return;
        }
        const std::string default_path = m_recent_paths_handler->get_recent_file_path(FrameType::LIGHT, "");
        wxFileDialog dialog(this, "Save hot pixel info", "", default_path, "*['.txt']", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dialog.ShowModal() == wxID_OK) {
            const std::string file_address = dialog.GetPath().ToStdString();
            m_hot_pixel_identifier->save_hot_pixels_to_file(file_address);
        }
    }, id);

    id = unique_counter();
    hot_pixel_menu->Append(id, "Load hot pixel info", "Load hot pixel info");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        const std::string default_path = m_recent_paths_handler->get_recent_file_path(FrameType::LIGHT, "");
        wxFileDialog dialog(this, "Load hot pixel info", "", default_path, "*['.txt']", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dialog.ShowModal() == wxID_OK) {
            const std::string file_address = dialog.GetPath().ToStdString();
            if (m_hot_pixel_identifier == nullptr)  {
                m_hot_pixel_identifier = make_unique<AstroPhotoStacker::HotPixelIdentifier>();
            }
            m_hot_pixel_identifier->load_hot_pixels_from_file(file_address);
            update_status_icon(m_hot_pixel_status_icon, true);
        }
    }, id);

    m_menu_bar->Append(hot_pixel_menu, "&Hot pixels");
};

void MyFrame::add_aligned_images_producer_menu()  {
    wxMenu *produce_aligned_images_menu = new wxMenu;


    int id = unique_counter();
    produce_aligned_images_menu->Append(id, "Produce aligned images", "Produce aligned images");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        const bool frames_aligned = m_filelist_handler_gui_interface.all_checked_frames_are_aligned();
        stack_calibration_frames();
        if (!frames_aligned) {
            wxMessageDialog dialog(this, "Please align the frames first!", "Frames not aligned");
            if (dialog.ShowModal() == wxID_YES) {
                AlignmentFrame *select_alignment_window = new AlignmentFrame(this, &m_filelist_handler_gui_interface, static_cast<StackSettings *>(m_stack_settings.get()), &m_alignment_box_vector_storage);
                select_alignment_window->Show(true);
            }
            else {
                return;
            }
        }

        AlignedImagesProducerGUI *aligned_images_producer_gui = new AlignedImagesProducerGUI(this, &m_post_processing_tool);
        aligned_images_producer_gui->Show(true);

    }, id);

    m_menu_bar->Append(produce_aligned_images_menu, "&Produce aligned images");
}


void MyFrame::add_postprocessing_menu() {
    wxMenu *postprocessing_menu = new wxMenu;

    int id = unique_counter();
    postprocessing_menu->Append(id, "Open post-processing editor", "Open post-processing editor");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        if (m_stacker == nullptr) {
            wxMessageDialog dialog(this, "Files have not been stacked yet!", "Frames not stacked");
            dialog.ShowModal();
            return;
        }

        const vector<vector<double>> &stacked_image = m_stacker->get_stacked_image();
        const int width = m_stacker->get_width();
        const int height = m_stacker->get_height();

        PostProcessingToolGUI *aligned_images_producer_gui = new PostProcessingToolGUI(this, stacked_image, width, height, &m_post_processing_tool);
        aligned_images_producer_gui->Show(true);
    }, id);

    m_menu_bar->Append(postprocessing_menu, "&Postprocessing");
};

void MyFrame::add_menu_bar()    {
    m_menu_bar = new wxMenuBar;

    add_file_menu();
    add_filelist_menu();
    add_alignment_menu();
    add_group_menu();
    add_hot_pixel_menu();
    add_aligned_images_producer_menu();
    add_postprocessing_menu();

    SetMenuBar(m_menu_bar);
};

void MyFrame::add_files_to_stack_checkbox()  {
    wxPanel* header_panel = new wxPanel(this, wxID_ANY);

    // Create a sizer for the panel
    wxBoxSizer* header_sizer = new wxBoxSizer(wxHORIZONTAL);
    header_panel->SetSizer(header_sizer);

    wxBitmap arrow_down_bitmap(s_gui_folder_path + "data/png/arrows/arrow_down_20x10.png", wxBITMAP_TYPE_PNG);
    wxBitmap arrow_up_bitmap(s_gui_folder_path + "data/png/arrows/arrow_up_20x10.png", wxBITMAP_TYPE_PNG);

    auto add_sorting_option = [this, &arrow_down_bitmap, &arrow_up_bitmap, header_panel, header_sizer]
        (const std::string &text, void (FilelistHandlerGUIInterface::*sorting_method)(bool)) {
        wxStaticText* static_text = new wxStaticText(header_panel, wxID_ANY, text, wxDefaultPosition);
        wxBitmapButton* arrow_up_button = new wxBitmapButton(header_panel, wxID_ANY, arrow_up_bitmap, wxDefaultPosition, wxSize(40, 20));
        wxBitmapButton* arrow_down_button = new wxBitmapButton(header_panel, wxID_ANY, arrow_down_bitmap, wxDefaultPosition, wxSize(40, 20));

        // Create a vertical sizer for the arrows
        wxBoxSizer* arrow_sizer = new wxBoxSizer(wxVERTICAL);
        arrow_sizer->Add(arrow_up_button,   0, wxTOP    | wxLEFT | wxRIGHT, 5);
        arrow_sizer->Add(arrow_down_button, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);

        arrow_up_button->Bind(wxEVT_BUTTON, [this, sorting_method](wxCommandEvent&) {
            (m_filelist_handler_gui_interface.*sorting_method)(true);
            update_files_to_stack_checkbox();
        });
        arrow_down_button->Bind(wxEVT_BUTTON, [this, sorting_method](wxCommandEvent&) {
            (m_filelist_handler_gui_interface.*sorting_method)(false);
            update_files_to_stack_checkbox();
        });

        // Add the static texts and arrow sizers to the header sizer
        header_sizer->Add(static_text, 0, wxTOP, 5);
        header_sizer->Add(arrow_sizer, 0, wxTOP, 5);
    };
    add_sorting_option("Sort by Name",          &FilelistHandlerGUIInterface::sort_by_name);
    add_sorting_option("Sort by Score",         &FilelistHandlerGUIInterface::sort_by_ranking);
    add_sorting_option("Sort by Group",         &FilelistHandlerGUIInterface::sort_by_group);
    add_sorting_option("Sort by Brightness",    &FilelistHandlerGUIInterface::sort_by_mean_brightness);

    // button for keeping only best N files
    wxButton *button_keep_best = new wxButton(header_panel, wxID_ANY, "Keep best N", wxDefaultPosition, wxDefaultSize);
    button_keep_best->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        const int n_files = m_filelist_handler_gui_interface.get_number_of_all_frames();
        const wxString default_value = wxString::Format(wxT("%d"), n_files);
        wxTextEntryDialog dialog(this, "Enter the number of best files to keep", "Keep best N files", default_value);
        if (dialog.ShowModal() == wxID_OK) {
            const wxString value = dialog.GetValue();
            const int n_files_to_keep = stoi(value.ToStdString());
            m_filelist_handler_gui_interface.keep_best_n_frames(n_files_to_keep);
            update_files_to_stack_checkbox();
        }
    });

    header_sizer->Add(button_keep_best, 0, wxTOP, 5);

    // show metadata checkbox
    wxCheckBox *checkbox_show_metadata = new wxCheckBox(header_panel, wxID_ANY, "Show Metadata");
    checkbox_show_metadata->SetValue(m_filelist_handler_gui_interface.show_metadata());
    checkbox_show_metadata->Bind(wxEVT_CHECKBOX, [this, checkbox_show_metadata](wxCommandEvent&){
        m_filelist_handler_gui_interface.set_show_metadata(checkbox_show_metadata->IsChecked());
        update_files_to_stack_checkbox();
    });
    header_sizer->Add(checkbox_show_metadata, 0, wxRIGHT, 5);

    // show group checkbox
    wxCheckBox *checkbox_show_group = new wxCheckBox(header_panel, wxID_ANY, "Show Group");
    checkbox_show_group->SetValue(m_filelist_handler_gui_interface.show_group());
    checkbox_show_group->Bind(wxEVT_CHECKBOX, [this, checkbox_show_group](wxCommandEvent&){
        m_filelist_handler_gui_interface.set_show_group(checkbox_show_group->IsChecked());
        update_files_to_stack_checkbox();
    });
    header_sizer->Add(checkbox_show_group, 0, wxRIGHT, 5);

    // show frame statistics checkbox
    wxCheckBox *checkbox_show_frame_stats = new wxCheckBox(header_panel, wxID_ANY, "Show Frame Statistics");
    checkbox_show_frame_stats->SetValue(m_filelist_handler_gui_interface.show_statistics());
    checkbox_show_frame_stats->Bind(wxEVT_CHECKBOX, [this, checkbox_show_frame_stats](wxCommandEvent&){
        const bool checked = checkbox_show_frame_stats->IsChecked();
        const int n_frames_without_stats = m_filelist_handler_gui_interface.get_number_of_frames_without_statistics();
        if (n_frames_without_stats > 0 && checked) {
            std::atomic<int> n_processed{0};
            const unsigned int n_cpu = m_stack_settings->get_n_cpus();
            run_task_with_progress_dialog(  "Calculating frame statistics",
                                            "Processed",
                                            "frames",
                                            n_processed,
                                            n_frames_without_stats,
                                            [this, &n_frames_without_stats, &n_processed, n_cpu](){
                                                m_filelist_handler_gui_interface.calculate_frame_statistics(n_cpu, &n_processed);
                                            });

        }
        m_filelist_handler_gui_interface.set_show_statistics(checkbox_show_frame_stats->IsChecked());
        update_files_to_stack_checkbox();
    });
    header_sizer->Add(checkbox_show_frame_stats, 0, wxRIGHT, 5);

    wxArrayString files;
    m_files_to_stack_checkbox = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, files, wxLB_MULTIPLE);

    // set font to the one with fixed width characters
    m_files_to_stack_checkbox->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));


    // Create a new sizer for the header and the checkbox list
    wxBoxSizer* checkboxSizer = new wxBoxSizer(wxVERTICAL);
    checkboxSizer->Add(header_panel, 0, wxEXPAND | wxALL, 5);
    checkboxSizer->Add(m_files_to_stack_checkbox, 1, wxEXPAND | wxALL, 5);

    // Add the new sizer to the main frame's sizer
    m_sizer_main_frame->Add(checkboxSizer, 9, wxEXPAND | wxALL, 5);

    m_files_to_stack_checkbox->Bind(wxEVT_LISTBOX, [this](wxCommandEvent &event){
        const int index = event.GetSelection();
        m_filelist_handler_gui_interface.set_selected_frame_index(index);
        const bool update_needed = update_checked_files_in_filelist();

        // Do not update the preview if the files was just checked/unchecked - it is slow
        if (!update_needed) {
            update_image_preview_file(index);
        }
    });

    m_files_to_stack_checkbox->Bind(wxEVT_CHAR_HOOK, [this](wxKeyEvent& event) {
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
            if (index > int(m_files_to_stack_checkbox->GetCount()) - 1) {
                index = int(m_files_to_stack_checkbox->GetCount()) - 1;
            }
            m_filelist_handler_gui_interface.set_selected_frame_index(index);
            m_files_to_stack_checkbox->SetSelection(index);
            if (old_index != index) {
                m_files_to_stack_checkbox->Deselect(old_index);
                update_image_preview_file(index);
            }
        }
        if (event.GetKeyCode() == WXK_UP) {
            const int old_index = m_filelist_handler_gui_interface.selected_frame_index();
            int index = old_index - 1;
            if (index < 0) index = 0;
            m_filelist_handler_gui_interface.set_selected_frame_index(index);
            m_files_to_stack_checkbox->SetSelection(index);
            if (old_index != index) {
                m_files_to_stack_checkbox->Deselect(old_index);
                update_image_preview_file(index);
            }
        }
    });
};

void MyFrame::update_files_to_stack_checkbox()   {
    unsigned int index = m_filelist_handler_gui_interface.selected_frame_index() > 0 ? m_filelist_handler_gui_interface.selected_frame_index() : 0;
    m_files_to_stack_checkbox->Clear();
    vector<string> rows;
    vector<bool>   rows_checked;

    m_filelist_handler_gui_interface.update_shown_frames();
    const vector<std::pair<std::string, FrameID>> &shown_frames = m_filelist_handler_gui_interface.get_shown_frames();
    for (const auto &frame : shown_frames)   {
        const std::string file_string = frame.first;
        const bool is_checked = m_filelist_handler_gui_interface.frame_is_checked(frame.second);
        rows.push_back(file_string);
        rows_checked.push_back(is_checked);
    }

    wxArrayString rows_wx;
    for (const string &row : rows) {
        rows_wx.Add(row);
    }

    m_files_to_stack_checkbox->Append(rows_wx);
    for (unsigned int i = 0; i < rows_checked.size(); i++) {
        m_files_to_stack_checkbox->Check(i, rows_checked[i]);
    }

    if (index >= rows_checked.size()) {
        index = rows_checked.size() ? rows_checked.size() - 1 : 0;
        m_filelist_handler_gui_interface.set_selected_frame_index(index);
    }

    m_files_to_stack_checkbox->SetSelection(index);
};

bool MyFrame::update_checked_files_in_filelist() {
    wxArrayInt checked_indices;
    m_files_to_stack_checkbox->GetCheckedItems(checked_indices);
    bool updated = false;
    for (int i = 0; i < m_filelist_handler_gui_interface.get_number_of_all_frames(); i++) {
        const bool file_checked_in_checkbox = m_files_to_stack_checkbox->IsChecked(i);
        const bool file_checked_in_filelist = m_filelist_handler_gui_interface.frame_is_checked(i);
        if (file_checked_in_checkbox != file_checked_in_filelist) {
            m_filelist_handler_gui_interface.set_frame_checked(i, file_checked_in_checkbox);
            updated = true;
        }
    }
    update_input_numbers_overview();
    update_alignment_status();
    return updated;
};

void MyFrame::add_button_bar()   {
    wxButton *button_check_all      = new wxButton(this, wxID_ANY, "Check all");
    wxButton *button_remove_checked = new wxButton(this, wxID_ANY, "Remove checked");
    wxButton *button_align_files    = new wxButton(this, wxID_ANY, "Align files");
    wxButton *button_hot_pixel_id   = new wxButton(this, wxID_ANY, "Identify hot pixels");
    wxButton *button_stack_files    = new wxButton(this, wxID_ANY, "Stack files");

    button_check_all->Bind(wxEVT_BUTTON, [this, button_check_all](wxCommandEvent&){
        update_checked_files_in_filelist();
        if (button_check_all->GetLabel() == "Uncheck all") {
            for (unsigned int i = 0; i < m_files_to_stack_checkbox->GetCount(); ++i) {
                m_files_to_stack_checkbox->Check(i, false);
            }
            button_check_all->SetLabel("Check all");
        }
        else {
            for (unsigned int i = 0; i < m_files_to_stack_checkbox->GetCount(); ++i) {
                m_files_to_stack_checkbox->Check(i);
            }
            button_check_all->SetLabel("Uncheck all");
        }
        update_checked_files_in_filelist();
    });

    button_align_files->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        // pop-up window with wxChoice of added light frames
        update_checked_files_in_filelist();
        if (m_filelist_handler_gui_interface.get_number_of_checked_frames(FrameType::LIGHT) == 0) {
            wxMessageDialog *dialog = new wxMessageDialog(this, "No light frames have been checked. Please check them first!", "Frames alignment warning.");
            dialog->ShowModal();
            return;
        }
        AlignmentFrame *select_alignment_window = new AlignmentFrame(this, &m_filelist_handler_gui_interface, m_stack_settings.get(), &m_alignment_box_vector_storage);
        select_alignment_window->Show(true);
    });

    button_remove_checked->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        // get checked files:
        wxArrayInt checked_indices;
        m_files_to_stack_checkbox->GetCheckedItems(checked_indices);

        // remove checked files from m_filelist_handler_gui_interface
        for (int i = checked_indices.GetCount() - 1; i >= 0; --i) {
            const std::string option = m_files_to_stack_checkbox->GetString(checked_indices[i]).ToStdString();
            m_filelist_handler_gui_interface.remove_frame(checked_indices[i]);
        }

            // update m_files_to_stack_checkbox
            update_files_to_stack_checkbox();
    });

    button_hot_pixel_id->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        update_checked_files_in_filelist();
        vector<InputFrame> frames;

        const vector<int> group_indices = m_filelist_handler_gui_interface.get_group_numbers();
        for (int group_index : group_indices) {
            for (FrameType type : {FrameType::LIGHT, FrameType::DARK, FrameType::FLAT, FrameType::BIAS})   {
                const std::map<AstroPhotoStacker::InputFrame, FrameInfo> &frames_map = m_filelist_handler_gui_interface.get_frames(type, group_index);
                for (const auto &frame : frames_map)   {
                    if (frame.second.is_checked)   {
                        frames.push_back(frame.first);
                    }
                }
            }
        }

        if (frames.empty()) {
            wxMessageDialog *dialog = new wxMessageDialog(this, "No frames have been selected. Please select them first!", "Hot pixel identification warning.");
            dialog->ShowModal();
            return;
        }

        m_hot_pixel_identifier = make_unique<AstroPhotoStacker::HotPixelIdentifier>();
        m_hot_pixel_identifier->set_n_cpu(m_stack_settings->get_n_cpus());
        const std::atomic<int> &n_processed = m_hot_pixel_identifier->get_number_of_processed_photos();
        const int files_total = frames.size();

        run_task_with_progress_dialog(  "Hot pixel identification",
                                        "Processed",
                                        "frames",
                                        n_processed,
                                        files_total,
                                        [this, frames](){
                                            m_hot_pixel_identifier->add_photos(frames);
                                        });

        m_hot_pixel_identifier->compute_hot_pixels();

        std::vector<std::tuple<int,int>> hot_pixels = m_hot_pixel_identifier->get_hot_pixels();

        update_status_icon(m_hot_pixel_status_icon, true);
    });

    button_stack_files->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        update_checked_files_in_filelist();
        const bool frames_aligned = m_filelist_handler_gui_interface.all_checked_frames_are_aligned();
        stack_calibration_frames();
        if (!frames_aligned) {
            wxMessageDialog dialog(this, "Please align the files first!", "Files not aligned");
            if (dialog.ShowModal() == wxID_YES) {
                AlignmentFrame *select_alignment_window = new AlignmentFrame(this, &m_filelist_handler_gui_interface, static_cast<StackSettings *>(m_stack_settings.get()), &m_alignment_box_vector_storage);
                select_alignment_window->Show(true);
            }
            else {
                return;
            }
        }


        if (m_filelist_handler_gui_interface.get_number_of_checked_frames(FrameType::LIGHT) == 0) {
            wxMessageDialog dialog(this, "No light frames have been checked. Please check them first!", "No frames checked.");
            if (dialog.ShowModal() == wxID_YES) {
                return;
            }
            else {
                return;
            }
        }

        m_stacker = get_configured_stacker(*m_stack_settings, m_filelist_handler_gui_interface);
        if (this->m_stack_settings->use_hot_pixel_correction()) {
            m_stacker->set_hot_pixels(m_hot_pixel_identifier->get_hot_pixels());
        }
        const int tasks_total = m_stacker->get_tasks_total();
        const std::atomic<int> &tasks_processed = m_stacker->get_tasks_processed();

        run_task_with_progress_dialog(  "File stacking",
                                "Finished",
                                "",
                                tasks_processed,
                                tasks_total,
                                [this](){
                                    m_stacker->calculate_stacked_photo();
                                },
                                "Calculating final image ...");

        update_image_preview_with_stacked_image();
        m_summary_yaml_creator = make_unique<SummaryYamlCreator>(m_filelist_handler_gui_interface);

        update_status_icon(m_stacked_status_icon, true);
    });

    m_sizer_button_bar->Add(button_check_all, 1, wxALL, 5);
    m_sizer_button_bar->Add(button_remove_checked,  1, wxALL, 5);
    m_sizer_button_bar->Add(button_align_files, 1, wxALL, 5);
    m_sizer_button_bar->Add(button_hot_pixel_id,1, wxALL, 5);
    m_sizer_button_bar->Add(button_stack_files, 1, wxALL, 5);
};

void MyFrame::add_stack_settings_preview()   {
    add_n_cpu_slider();
    add_max_memory_spin_ctrl();
    add_stacking_algorithm_choice_box();
    add_hot_pixel_correction_checkbox();
    add_color_interpolation_checkbox();
    add_color_stretching_checkbox();
};

void MyFrame::add_n_cpu_slider()    {
    const int max_cpu = m_stack_settings->get_max_threads();
    const int default_value = m_stack_settings->get_n_cpus();

    // Create a wxStaticText to display the current value
    wxStaticText* n_cpu_text = new wxStaticText(this, wxID_ANY, wxString::Format(wxT("Number of CPUs: %d"), default_value ));
    m_stack_settings->set_n_cpus(default_value);

    // Create the wxSlider
    wxSlider* slider_ncpu = new wxSlider(this, wxID_ANY, default_value, 1, max_cpu, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);

    // Bind the slider's wxEVT_SLIDER event to a lambda function that updates the value text
    slider_ncpu->Bind(wxEVT_SLIDER, [n_cpu_text, slider_ncpu, this](wxCommandEvent&){
        int current_value = slider_ncpu->GetValue();
        m_stack_settings->set_n_cpus(current_value);
        n_cpu_text->SetLabel(wxString::Format(wxT("Number of CPUs: %d"), current_value));
    });

    // Add the controls to a sizer
    m_sizer_top_left->Add(n_cpu_text, 0,   wxEXPAND, 5);
    m_sizer_top_left->Add(slider_ncpu, 0,  wxEXPAND, 5);
};

void MyFrame::add_stacking_algorithm_choice_box()  {
    wxStaticText* stacking_algorithm_text = new wxStaticText(this, wxID_ANY, "Stacking algorithm:");

    wxString stacking_algorithms[m_stack_settings->get_stacking_algorithms().size()];
    for (unsigned int i = 0; i < m_stack_settings->get_stacking_algorithms().size(); ++i) {
        stacking_algorithms[i] = m_stack_settings->get_stacking_algorithms()[i];
    }

    const vector<string> &stacking_algorithms_vector = m_stack_settings->get_stacking_algorithms();
    int defaut_choice = 0;
    for (unsigned int i = 0; i < stacking_algorithms_vector.size(); ++i) {
        if (stacking_algorithms_vector[i] == m_stack_settings->get_stacking_algorithm()) {
            defaut_choice = i;
            break;
        }
    }

    wxChoice* choice_box_stacking_algorithm = new wxChoice( this,
                                                            wxID_ANY,
                                                            wxDefaultPosition,
                                                            wxDefaultSize,
                                                            m_stack_settings->get_stacking_algorithms().size(),
                                                            stacking_algorithms);

    choice_box_stacking_algorithm->SetSelection(defaut_choice);
    //choice_box_stacking_algorithm->SetSelection(0);
    choice_box_stacking_algorithm->Bind(wxEVT_CHOICE, [choice_box_stacking_algorithm, this](wxCommandEvent&){
        int current_selection = choice_box_stacking_algorithm->GetSelection();
        m_stack_settings->set_stacking_algorithm(choice_box_stacking_algorithm->GetString(current_selection).ToStdString());
        update_algorithm_specific_settings_gui();
    });

    int current_selection = choice_box_stacking_algorithm->GetSelection();
    m_stack_settings->set_stacking_algorithm(choice_box_stacking_algorithm->GetString(current_selection).ToStdString());

    m_sizer_top_left->Add(stacking_algorithm_text, 0, wxEXPAND, 5);
    m_sizer_top_left->Add(choice_box_stacking_algorithm, 0,  wxEXPAND, 5);

    m_sizer_algorithm_specific_settings = new wxBoxSizer(wxVERTICAL);
    update_algorithm_specific_settings_gui();
    m_sizer_top_left->Add(m_sizer_algorithm_specific_settings, 0, wxEXPAND, 5);

    m_sizer_bottom_left = new wxBoxSizer(wxVERTICAL);

    m_sizer_top_left->AddStretchSpacer(1);
    m_sizer_top_left->Add(m_sizer_bottom_left, 0, wxALIGN_BOTTOM | wxEXPAND , 5);
    m_luminance_stretching_slider = new ThreePointSlider(this, wxID_ANY, wxDefaultPosition, wxSize(300, 50));
    m_sizer_bottom_left->Add(m_luminance_stretching_slider, 0, wxEXPAND, 5);
    m_luminance_stretching_slider->set_thumbs_positions(vector<float>({0., 0.5, 1.}));
    m_current_preview->set_stretcher(&m_color_stretcher);
    m_luminance_stretching_slider->register_on_change_callback([this](){
        const float thumb1 = m_luminance_stretching_slider->get_value(0);
        const float thumb2 = m_luminance_stretching_slider->get_value(1);
        const float thumb3 = m_luminance_stretching_slider->get_value(2);
        const float midtone = thumb1 == thumb3 ? 0.5 : thumb2/(thumb3-thumb1);
        IndividualColorStretchingToolBase &luminance_stretcher = m_color_stretcher.get_luminance_stretcher(0);
        (dynamic_cast<IndividualColorStretchingBlackMidtoneWhite&>(luminance_stretcher)).set_stretching_parameters(thumb1, midtone, thumb3);
        update_image_preview();
    });

};

void MyFrame::update_algorithm_specific_settings_gui()  {
    vector<AdditionalStackerSetting> specific_settings = m_stack_settings->get_algorithm_specific_settings_defaults();

    for (wxStaticText* text : m_algorithm_specific_settings_texts) {
        m_sizer_algorithm_specific_settings->Detach(text);
        text->Destroy();        // cannot call delete on this, because of a bug in wxWidgets desctructor, so we are going to leak some memory here instead ...
    }
    m_algorithm_specific_settings_texts.clear();

    for (wxSpinCtrlDouble* spin_ctrl : m_algorithm_specific_settings_spin_ctrls) {
        m_sizer_algorithm_specific_settings->Detach(spin_ctrl);
        spin_ctrl->Destroy();   // cannot call delete on this, because of a bug in wxWidgets desctructor, so we are going to leak some memory here instead ...
    }
    m_algorithm_specific_settings_spin_ctrls.clear();

    m_stack_settings->clear_algorithm_specific_settings();


    for (const AstroPhotoStacker::AdditionalStackerSetting &setting : specific_settings) {
        wxStaticText* label = new wxStaticText(this, wxID_ANY, setting.get_name() + ":");
        const double current_value = setting.get_value();
        m_stack_settings->set_algorithm_specific_setting(setting.get_name(), current_value);
        const std::pair<double, double> min_max = setting.get_range();
        const double step = setting.get_step();

        wxSpinCtrlDouble* spin_ctrl = new wxSpinCtrlDouble(this, wxID_ANY, wxString::Format("%.2f", current_value), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min_max.first, min_max.second, current_value, step);

        spin_ctrl->Bind(wxEVT_SPINCTRLDOUBLE, [spin_ctrl, setting, this](wxCommandEvent&){
            m_stack_settings->set_algorithm_specific_setting(setting.get_name(), spin_ctrl->GetValue());
        });

        m_algorithm_specific_settings_texts.push_back(label);
        m_algorithm_specific_settings_spin_ctrls.push_back(spin_ctrl);

        m_sizer_algorithm_specific_settings->Add(label, 0, wxEXPAND, 5);
        m_sizer_algorithm_specific_settings->Add(spin_ctrl, 0, wxEXPAND, 5);
    }

    m_sizer_algorithm_specific_settings->Layout();
    m_sizer_top_left->Layout();
};

void MyFrame::add_hot_pixel_correction_checkbox()    {
    wxCheckBox* checkbox_hot_pixel_correction = new wxCheckBox(this, wxID_ANY, "Hot pixel correction");
    checkbox_hot_pixel_correction->Bind(wxEVT_CHECKBOX, [checkbox_hot_pixel_correction, this](wxCommandEvent&){
        if (m_hot_pixel_identifier == nullptr && checkbox_hot_pixel_correction->GetValue())    {
            wxMessageDialog *dialog = new wxMessageDialog(this, "Hot pixel identification not performed. Please run it first!", "Hot pixel identification");
            dialog->ShowModal();
            checkbox_hot_pixel_correction->SetValue(false);
            return;
        }
        bool is_checked = checkbox_hot_pixel_correction->GetValue();
        m_stack_settings->set_hot_pixel_correction(is_checked);
    });
    bool is_checked = checkbox_hot_pixel_correction->GetValue();
    m_stack_settings->set_hot_pixel_correction(is_checked);

    m_sizer_bottom_left->Add(checkbox_hot_pixel_correction, 0, wxEXPAND, 5);
};

void MyFrame::add_color_interpolation_checkbox()    {
    wxCheckBox* checkbox_color_interpolation = new wxCheckBox(this, wxID_ANY, "Color interpolation");
    const bool is_checked = m_stack_settings->use_color_interpolation();
    checkbox_color_interpolation->SetValue(is_checked);
    checkbox_color_interpolation->SetToolTip("This will calculate all 3 color channels for each pixels, using information from its neighbors. It helps to further suppress the noise and avoid weird color artifacts, but it also leads to approx. 3 times slower stacking and might result in slightly less detailed stacked image.");
    checkbox_color_interpolation->Bind(wxEVT_CHECKBOX, [checkbox_color_interpolation, this](wxCommandEvent&){
        const bool is_checked = checkbox_color_interpolation->GetValue();
        m_stack_settings->set_use_color_interpolation(is_checked);
    });
    m_sizer_bottom_left->Add(checkbox_color_interpolation, 0, wxEXPAND, 5);
};

void MyFrame::add_color_stretching_checkbox()   {
    wxCheckBox* checkbox_color_stretching = new wxCheckBox(this, wxID_ANY, "Apply color stretching to output file");
    const bool is_checked = m_stack_settings->apply_color_stretching();
    checkbox_color_stretching->SetValue(is_checked);
    checkbox_color_stretching->SetToolTip("If checked, the color stretching from RGB sliders will be used in the output image.");
    checkbox_color_stretching->Bind(wxEVT_CHECKBOX, [checkbox_color_stretching, this](wxCommandEvent&){
        const bool is_checked = checkbox_color_stretching->GetValue();
        m_stack_settings->set_apply_color_stretching(is_checked);
    });
    m_sizer_bottom_left->Add(checkbox_color_stretching, 0, wxEXPAND, 5);
};

void MyFrame::add_max_memory_spin_ctrl() {
    wxStaticText* memory_usage_text = new wxStaticText(this, wxID_ANY, "Maximum memory usage (MB):");

    const int memory_defeault = m_stack_settings->get_max_memory();
    wxSpinCtrl* spin_ctrl_max_memory = new wxSpinCtrl(this, wxID_ANY, std::to_string(memory_defeault), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100000, memory_defeault);
    spin_ctrl_max_memory->Bind(wxEVT_SPINCTRL, [spin_ctrl_max_memory, this](wxCommandEvent&){
        int current_value = spin_ctrl_max_memory->GetValue();
        m_stack_settings->set_max_memory(current_value);
    });
    int current_value = spin_ctrl_max_memory->GetValue();
    m_stack_settings->set_max_memory(current_value);

    m_sizer_top_left->Add(memory_usage_text, 0, wxEXPAND, 5);
    m_sizer_top_left->Add(spin_ctrl_max_memory, 0,  wxEXPAND, 5);
};

void MyFrame::add_image_settings()   {
    add_exposure_correction_spin_ctrl();
    add_input_numbers_overview();
    add_histogram_and_rgb_sliders();
};

void MyFrame::add_upper_middle_panel()   {
    add_image_preview();
    add_step_control_part();
};


void MyFrame::add_image_preview()    {
    // Add the wxGenericStaticBitmap to a sizer
    m_image_preview_size = new wxBoxSizer(wxVERTICAL);
    m_sizer_top_center->Add(m_image_preview_size, 1, wxEXPAND | wxALL, 5);

    m_image_preview_size->Add(m_current_preview->get_image_preview_bitmap(), 1, wxCENTER, 0);
};

void MyFrame::update_image_preview_file(size_t frame_index)  {
    const InputFrame frame = m_filelist_handler_gui_interface.get_frame_by_index(frame_index).input_frame;
    m_current_preview->read_preview_from_frame(frame);
    update_image_preview();
    update_alignment_status();
};

void MyFrame::update_image_preview_with_stacked_image()  {
    const vector<vector<double>> &stacked_image = m_stacker->get_stacked_image();
    const int width = m_stacker->get_width();
    const int height = m_stacker->get_height();
    m_current_preview->read_preview_from_stacked_image(stacked_image, width, height);

    m_current_preview->update_preview_bitmap();

    update_histogram();
    update_color_channels_mean_and_median_values_text();
};

void MyFrame::update_image_preview()  {
    m_current_preview->update_preview_bitmap();
    update_histogram();
    update_color_channels_mean_and_median_values_text();
};

void MyFrame::add_step_control_part()    {
    wxGridSizer *grid_sizer = new wxGridSizer(2,3, 0, 0);

    auto add_button_and_checkmark = [this](auto on_button_function, const std::string &description_text, const std::string &button_text, wxGenericStaticBitmap **status_icon, wxGridSizer *grid_sizer){
        wxStaticText* text_aligned = new wxStaticText(this, wxID_ANY, description_text);
        wxFont font = text_aligned->GetFont();
        font.SetPointSize(14);
        text_aligned->SetFont(font);
        *status_icon = new wxGenericStaticBitmap(this, wxID_ANY, wxBitmap(s_gui_folder_path + "data/png/checkmarks/20px/red_cross.png", wxBITMAP_TYPE_PNG));

        wxButton *button = new wxButton(this, wxID_ANY, button_text);
        button->Bind(wxEVT_BUTTON, on_button_function);

        grid_sizer->Add(text_aligned, 0,    wxALIGN_CENTER_VERTICAL | wxALL, 5);
        grid_sizer->Add(*status_icon, 0,    wxALIGN_CENTER_VERTICAL | wxALL, 5);
        grid_sizer->Add(button, 0,          wxALIGN_CENTER_VERTICAL | wxALL, 5);
    };

    auto on_button_show_alignment = [this](wxCommandEvent&){
        vector<vector<string>> tabular_data;
        vector<string> description;
        m_filelist_handler_gui_interface.get_alignment_info_tabular_data(&tabular_data, &description);
        if (tabular_data.empty())   {
            return;
        }
        ListFrame *alignment_list_frame = new ListFrame(this, "Alignment info", "Alignment info", description, tabular_data);
        alignment_list_frame->Show(true);
    };

    auto on_button_hot_pixels = [this](wxCommandEvent&){
        if (m_hot_pixel_identifier == nullptr)    {
            return;
        }
        const std::vector<std::tuple<int,int>> &hot_pixels = m_hot_pixel_identifier->get_hot_pixels();
        if (hot_pixels.empty())   {
            return;
        }
        vector<vector<string>> tabular_data;
        vector<string> description = {"x", "y"};
        for (const auto &hot_pixel : hot_pixels) {
            tabular_data.push_back({to_string(get<0>(hot_pixel)), to_string(get<1>(hot_pixel))});
        }
        ListFrame *alignment_list_frame = new ListFrame(this, "Hot pixel info", "Found " + std::to_string(hot_pixels.size()) + " hot pixels", description, tabular_data);
        alignment_list_frame->Show(true);
    };

    auto on_button_show_stacked_image = [this](wxCommandEvent&){
        if (m_stacker == nullptr)    {
            return;
        }
        update_image_preview_with_stacked_image();
    };

    add_button_and_checkmark(on_button_show_alignment,      "Files aligned: ",          "Show alignment",       &m_alignment_status_icon,   grid_sizer);
    add_button_and_checkmark(on_button_hot_pixels,          "Hot pixels identified: ",  "Show hot pixels",      &m_hot_pixel_status_icon,   grid_sizer);
    add_button_and_checkmark(on_button_show_stacked_image,  "Stacking finished: ",      "Show stacked file",    &m_stacked_status_icon,     grid_sizer);

    m_sizer_top_center->Add(grid_sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
};

void MyFrame::update_alignment_status()  {
    update_status_icon(m_alignment_status_icon, m_filelist_handler_gui_interface.all_checked_frames_are_aligned());
};

void MyFrame::add_exposure_correction_spin_ctrl()   {
    m_exposure_correction_slider = make_unique<FloatingPointSlider>(
        this,
        "Exposure correction: ",
        -7.0,
        7.0,
        0.0,
        0.1,
        1,
        [this](float exposure_correction){
            IndividualColorStretchingToolBase &luminance_stretcher = m_color_stretcher.get_luminance_stretcher(1);
            (dynamic_cast<IndividualColorStretchingBlackCorrectionWhite&>(luminance_stretcher)).set_stretching_parameters(0,exposure_correction,1);
            update_image_preview();
        }
    );
    m_exposure_correction_slider->add_sizer(m_sizer_top_right, 0, wxEXPAND, 5);
};

void MyFrame::add_input_numbers_overview()  {
    wxGridSizer *grid_sizer = new wxGridSizer(5,3, 0, 0);
    wxStaticText* text_frame_type = new wxStaticText(this, wxID_ANY, "Frame type");
    wxStaticText* text_total = new wxStaticText(this, wxID_ANY, "Total");
    wxStaticText* text_checked = new wxStaticText(this, wxID_ANY, "Checked");

    grid_sizer->Add(text_frame_type, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    grid_sizer->Add(text_total, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    grid_sizer->Add(text_checked, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    auto add_summary_text = [this, grid_sizer](FrameType type, const std::string& label) {
        wxStaticText* text = new wxStaticText(this, wxID_ANY, label);
        wxStaticText* text_number = new wxStaticText(this, wxID_ANY, std::to_string(m_filelist_handler_gui_interface.get_number_of_all_frames(type)));
        wxStaticText* text_checked = new wxStaticText(this, wxID_ANY, std::to_string(m_filelist_handler_gui_interface.get_number_of_checked_frames(type)));
        m_frames_numbers_overview_texts[type] = {text_number, text_checked};

        grid_sizer->Add(text, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        grid_sizer->Add(text_number, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        grid_sizer->Add(text_checked, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    };

    add_summary_text(FrameType::LIGHT, "Light frames: ");
    add_summary_text(FrameType::FLAT, "Flat frames: ");
    add_summary_text(FrameType::BIAS, "Bias frames: ");
    add_summary_text(FrameType::DARK, "Dark frames: ");

    m_sizer_top_right->Add(grid_sizer, 0, wxEXPAND, 5);

};

void MyFrame::add_histogram_and_rgb_sliders()    {
    m_histogram_data_tool_gui = std::make_unique<HistogramDataToolGUI>(this, m_sizer_top_right, wxDefaultPosition, wxSize(600,250));
    m_histogram_data_tool_gui->set_background_color(wxColour(200,200,200));
    m_histogram_data_tool_gui->set_line_colors({wxColour(255,0,0), wxColour(0,255,0), wxColour(0,0,255)});


    m_text_color_channels_mean_values = new wxStaticText(this, wxID_ANY, "Mean values: R: 0.0, G: 0.0, B: 0.0");
    m_sizer_top_right->Add( m_text_color_channels_mean_values, 0, wxCENTER, 5);

    m_text_color_channels_median_values = new wxStaticText(this, wxID_ANY, "Median values: R: 0.0, G: 0.0, B: 0.0");
    m_sizer_top_right->Add( m_text_color_channels_median_values, 0, wxCENTER, 5);

    vector<vector<wxColour>> thumbs_colors = {
        vector<wxColour>({wxColour(50,0,0), wxColour(180,0,0), wxColour(255,0,0)}),
        vector<wxColour>({wxColour(0,50,0), wxColour(0,180,0), wxColour(0,255,0)}),
        vector<wxColour>({wxColour(0,0,50), wxColour(0,0,180), wxColour(0,0,255)})
    };

    auto add_color_slider = [this](int color_index, ThreePointSlider* stretching_slider, const vector<wxColour> &thumb_colors_vector)    {
        stretching_slider = new ThreePointSlider(this, wxID_ANY, wxDefaultPosition, wxSize(300, 50));
        stretching_slider->set_colors(thumb_colors_vector[0], thumb_colors_vector[1], thumb_colors_vector[2]);
        m_sizer_top_right->Add(stretching_slider, 0, wxEXPAND, 5);
        stretching_slider->set_thumbs_positions(vector<float>({0., 0.5, 1.}));
        stretching_slider->set_maintain_slide_ordering(false);
        m_current_preview->set_stretcher(&m_color_stretcher);
        m_color_stretcher.add_color_stretcher(std::make_shared<IndividualColorStretchingBlackCorrectionWhite>(),color_index);
        stretching_slider->register_on_change_callback([this, stretching_slider, color_index](){
            const float thumb1 = stretching_slider->get_value(0);
            const float thumb2 = stretching_slider->get_value(1);
            const float thumb3 = stretching_slider->get_value(2);
            const float exposure_correction = 2*(thumb2 - 0.5);
            IndividualColorStretchingToolBase &stretcher = m_color_stretcher.get_color_stretcher(color_index,0);
            (dynamic_cast<IndividualColorStretchingBlackCorrectionWhite&>(stretcher)).set_stretching_parameters(thumb1, exposure_correction, thumb3);
            update_image_preview();
        });
    };

    add_color_slider(0, m_stretching_slider_red, thumbs_colors[0]);
    add_color_slider(1, m_stretching_slider_green, thumbs_colors[1]);
    add_color_slider(2, m_stretching_slider_blue, thumbs_colors[2]);

};

void MyFrame::update_input_numbers_overview()   {
    for (FrameType type : {FrameType::LIGHT, FrameType::DARK, FrameType::FLAT, FrameType::BIAS})   {
        m_frames_numbers_overview_texts[type].first->SetLabel(std::to_string(m_filelist_handler_gui_interface.get_number_of_all_frames(type)));
        m_frames_numbers_overview_texts[type].second->SetLabel(std::to_string(m_filelist_handler_gui_interface.get_number_of_checked_frames(type)));
    }
};

void MyFrame::on_exit(wxCommandEvent& event)     {
    Close(true);
}

void MyFrame::update_histogram()    {
    if (!m_current_preview->image_loaded()) {
        return;
    }
    // TODO bit depth and number of channels are hardcoded - this should be changed
    m_histogram_data_tool = std::make_unique<HistogramDataTool>(pow(2,15)-1, 3);
    m_histogram_data_tool->extract_data_from_image(m_current_preview->get_original_image());
    m_histogram_data_tool_gui->set_color_stretcher(m_color_stretcher);

    m_histogram_data_tool_gui->set_histogram_data_colors(*m_histogram_data_tool);
};

void MyFrame::on_open_frames(wxCommandEvent& event, FrameType type, const std::string& title)    {
    const std::string default_path = m_recent_paths_handler->get_recent_file_path(type, "");
    std::vector<string> allowed_extensions = {"cr2", "cr3", "jpg", "jpeg", "png", "fit", "tif", "tiff", "nef", "dng"};
    if (type == FrameType::LIGHT) {
        allowed_extensions.push_back("avi");
        allowed_extensions.push_back("mov");
        allowed_extensions.push_back("mp4");
        allowed_extensions.push_back("ser");
    }

    string wildcard_string = "Image files |";
    for (const string &extension : allowed_extensions) {
        wildcard_string += "*." + extension + ";" + "*." + AstroPhotoStacker::to_upper_copy(extension) + ";";
    }

    wxFileDialog dialog(this, title, "", default_path, wildcard_string, wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dialog.ShowModal() == wxID_OK) {
        wxArrayString paths;
        dialog.GetPaths(paths);
        MetadataManager metadata_manager;
        for (auto path : paths) {
            const AstroPhotoStacker::Metadata metadata = metadata_manager.get_metadata(InputFrame(path.ToStdString()));
            const string str_path = path.ToStdString();
            m_filelist_handler_gui_interface.add_file(path.ToStdString(), type, m_current_group, true, AlignmentFileInfo(), metadata);
            m_recent_paths_handler->set_recent_file_path_from_file(type, path.ToStdString());
        }
    }
    dialog.Destroy();
    update_checked_files_in_filelist();
    update_files_to_stack_checkbox();
};

void MyFrame::on_open_lights(wxCommandEvent& event)    {
    on_open_frames(event, FrameType::LIGHT, "Open light frames");
}

void MyFrame::on_open_flats(wxCommandEvent& event)    {
    on_open_frames(event, FrameType::FLAT, "Open flat frames");
}

void MyFrame::on_open_darks(wxCommandEvent& event)    {
    on_open_frames(event, FrameType::DARK, "Open dark frames");
}

void MyFrame::on_save_stacked(wxCommandEvent& event) {
    const bool has_stacked_image = m_stacker != nullptr ? !m_stacker->get_stacked_image().empty() : false;
    if (!has_stacked_image) {
        wxMessageDialog dialog(this, "Files have not been stacked yet!", "Frames not stacked");
        if (dialog.ShowModal() == wxID_YES) {
            return;
        }
        else {
            return;
        }
    }
    const std::string default_path = m_recent_paths_handler->get_recent_file_path(FrameType::LIGHT, "");
    wxFileDialog dialog(this, "Save stacked file", "", default_path, "*['.tif']", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dialog.ShowModal() == wxID_OK) {
        std::string file_address = dialog.GetPath().ToStdString();

        // if the extension is not .tif, add it
        if (file_address.substr(file_address.size()-4) != ".tif") {
            file_address += ".tif";
        }


        std::vector<std::vector<double> > stacked_image = m_stacker->get_stacked_image();

        if (m_stack_settings->apply_color_stretching()) {
            m_color_stretcher.stretch_image(&stacked_image, pow(2,15)-1, false);
        }

        stacked_image = m_post_processing_tool.post_process_image(stacked_image, m_stacker->get_width(), m_stacker->get_height());

        AstroPhotoStacker::StackerBase::save_stacked_photo(file_address,
                                        stacked_image,
                                        m_stacker->get_width(),
                                        m_stacker->get_height(),
                                        CV_16UC3);

        if (m_summary_yaml_creator) {
            m_summary_yaml_creator->create_and_save_yaml_file(file_address + ".yaml", &m_post_processing_tool);
            m_summary_yaml_creator->add_as_exif_metadata(file_address);
        }
    }
};


void MyFrame::on_save_selected_as_fit(wxCommandEvent& event) {

    // ask user for bit depth
    wxString choices[] = {"8", "16"};
    int output_bit_depth = 16;
    wxSingleChoiceDialog dialog_bit_depth(this, "Select bit depth", "Bit depth", 2, choices);
    if (dialog_bit_depth.ShowModal() == wxID_OK) {
        wxString selected = dialog_bit_depth.GetStringSelection();
        output_bit_depth = std::stoi(selected.ToStdString());
    }

    wxDirDialog dialog_folder(this, "Select folder for output images", "");
    string output_folder = ".";
    if (dialog_folder.ShowModal() == wxID_OK) {
        output_folder = dialog_folder.GetPath().ToStdString();
    }

    const vector<InputFrame> selected_frames = m_filelist_handler_gui_interface.get_selected_frames();
    vector<string> output_names;
    for (const InputFrame &frame : selected_frames) {
        const string file_address = frame.get_file_address();
        const string frame_number_part = !frame.is_video_frame() ? "" : "_frame_" + std::to_string(frame.get_frame_number());
        const string output_file_name = get_filename_without_extension(file_address) + frame_number_part + ".fit";
        output_names.push_back(output_file_name);
    }

    std::atomic<int> tasks_processed = 0;
    auto convert_selected_to_fit = [this, selected_frames, output_names, output_folder, output_bit_depth, &tasks_processed](){
        for (size_t i = 0; i < selected_frames.size(); ++i) {
            const InputFrame &frame = selected_frames[i];
            const string output_file_address = output_folder + "/" + output_names[i];
            convert_to_fit_file(frame, output_file_address, output_bit_depth);
            tasks_processed++;
        }
    };


    run_task_with_progress_dialog(  "Saving into FIT",
        "Saving selected files as FIT:",
        "",
        tasks_processed,
        selected_frames.size(),
        convert_selected_to_fit,
        "Saving selected files as FIT ...");


};

void MyFrame::update_status_icon(wxGenericStaticBitmap *status_icon, bool is_ok)   {
    const std::string file_checkmark    = s_gui_folder_path + "data/png/checkmarks/20px/checkmark.png";
    const std::string file_cross        = s_gui_folder_path + "data/png/checkmarks/20px/red_cross.png";
    if (is_ok)  {
        status_icon->SetBitmap(wxBitmap(file_checkmark, wxBITMAP_TYPE_PNG));
    }
    else    {
        status_icon->SetBitmap(wxBitmap(file_cross, wxBITMAP_TYPE_PNG));
    }
};

void MyFrame::update_color_channels_mean_and_median_values_text()   {
    if (!m_current_preview->image_loaded() || m_histogram_data_tool_gui == nullptr) {
        return;
    }

    auto update_one_text = [this](wxStaticText* static_text, const std::string &prefix, const vector<float> &values) {
        if (values.size() != 3) {
            return;
        }
        static_text->SetLabel(prefix + " R: " + AstroPhotoStacker::round_and_convert_to_string(values[0], 1) +
                                        ", G: " + AstroPhotoStacker::round_and_convert_to_string(values[1], 1) +
                                        ", B: " + AstroPhotoStacker::round_and_convert_to_string(values[2], 1) );
    };

    update_one_text(m_text_color_channels_mean_values, "Mean values: ", m_histogram_data_tool_gui->get_mean_values());
    update_one_text(m_text_color_channels_median_values, "Median values: ", m_histogram_data_tool_gui->get_median_values());
};

void MyFrame::stack_calibration_frames() {
    const vector<int> calibration_group_numbers = m_filelist_handler_gui_interface.get_group_numbers();

    for (int group_number : calibration_group_numbers) {
        for (FrameType type : {FrameType::BIAS, FrameType::DARK, FrameType::FLAT}) {
            // get vector of checked files

            const std::map<InputFrame, FrameInfo> &calibrationf_frame_map = m_filelist_handler_gui_interface.get_frames(type, group_number);
            vector<InputFrame> frames_to_stack;
            for (const auto &frame : calibrationf_frame_map) {
                if (frame.second.is_checked) {
                    frames_to_stack.push_back(frame.first);
                }
            }

            // nothing to stack
            if (frames_to_stack.size() < 2) {
                continue;
            }

            // add frames to new filelist handler
            FilelistHandler calibration_frames_handler;
            for (const auto &frame : frames_to_stack) {
                // LIGHT is not a bug - we do not use frames as correction here, we are stacking them
                calibration_frames_handler.add_frame(frame, FrameType::LIGHT, 0, true);
            }

            // create a separate stacker
            StackSettings calibration_frames_settings = *m_stack_settings;
            calibration_frames_settings.set_use_color_interpolation(false);
            std::unique_ptr<AstroPhotoStacker::StackerBase> calibration_stacker = get_configured_stacker(calibration_frames_settings, calibration_frames_handler);

            const string file_type_name = to_string(type);
            const int tasks_total = calibration_stacker->get_tasks_total();
            const std::atomic<int> &tasks_processed = calibration_stacker->get_tasks_processed();

            run_task_with_progress_dialog(  "File stacking",
                                    "Stacking " + file_type_name + " frames:",
                                    "",
                                    tasks_processed,
                                    tasks_total,
                                    [this, &calibration_stacker](){
                                        calibration_stacker->calculate_stacked_photo();
                                    },
                                    "Calculating final "  + file_type_name + " frame ...");

            const std::string last_frame_name = frames_to_stack.back().get_file_address();
            const string master_frame_name = last_frame_name.substr(0, last_frame_name.find_last_of('.')) + "_master" + file_type_name + ".tif";
            calibration_stacker->save_stacked_photo(master_frame_name, CV_16U);

            // remove original calibration frames from filelist handler
            m_filelist_handler_gui_interface.remove_all_frames_of_type_and_group(type, group_number);
            m_filelist_handler_gui_interface.add_file(master_frame_name, type, group_number, true);
            update_files_to_stack_checkbox();
        }
    }
};