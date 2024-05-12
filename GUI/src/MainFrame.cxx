#include "../headers/MainFrame.h"
#include "../headers/AlignmentFrame.h"
#include "../headers/ListFrame.h"
#include "../headers/StackerConfigureTool.h"
#include "../headers/ThreePointSlider.h"

#include "../../headers/Common.h"
#include "../../headers/raw_file_reader.h"
#include "../../headers/thread_pool.h"

#include <wx/spinctrl.h>
#include <wx/progdlg.h>
#include <wx/artprov.h>

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;

bool MyApp::OnInit()    {
    MyFrame *frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "AstroPhotoStacker GUI") {

    // full screen
    SetSize(wxGetDisplaySize());


    m_sizer_main_frame = new wxBoxSizer(wxVERTICAL);
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
    m_file_menu->Append(id, "Save stacked file", "Save stacked file");
    Bind(wxEVT_MENU, &MyFrame::on_save_stacked, this, id);

    m_file_menu->Append(wxID_EXIT);
    Bind(wxEVT_MENU, &MyFrame::on_exit,  this, wxID_EXIT);

    m_menu_bar->Append(m_file_menu, "&File");

};

void MyFrame::add_alignment_menu()  {
    wxMenu *alignment_menu = new wxMenu;

    int id = unique_counter();
    alignment_menu->Append(id, "Save alignment info", "Save alignment info");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        const std::string default_path = m_recent_paths_handler.get_recent_file_path(FileTypes::LIGHT, "");
        wxFileDialog dialog(this, "Save alignment info", "", default_path, "*['.txt']", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dialog.ShowModal() == wxID_OK) {
            const std::string file_address = dialog.GetPath().ToStdString();
            m_filelist_handler.save_alignment_to_file(file_address);
        }
    }, id);

    id = unique_counter();
    alignment_menu->Append(id, "Load alignment info", "Load alignment info");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        const std::string default_path = m_recent_paths_handler.get_recent_file_path(FileTypes::LIGHT, "");
        wxFileDialog dialog(this, "Load alignment info", "", default_path, "*['.txt']", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dialog.ShowModal() == wxID_OK) {
            const std::string file_address = dialog.GetPath().ToStdString();
            m_filelist_handler.load_alignment_from_file(file_address);
            update_alignment_status();
        }
    }, id);

    m_menu_bar->Append(alignment_menu, "&Alignment");
};

void MyFrame::add_hot_pixel_menu()  {
    wxMenu *hot_pixel_menu = new wxMenu;

    int id = unique_counter();
    hot_pixel_menu->Append(id, "Save hot pixel info", "Save hot pixel info");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        if (m_hot_pixel_identifier == nullptr)  {
            return;
        }
        const std::string default_path = m_recent_paths_handler.get_recent_file_path(FileTypes::LIGHT, "");
        wxFileDialog dialog(this, "Save hot pixel info", "", default_path, "*['.txt']", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dialog.ShowModal() == wxID_OK) {
            const std::string file_address = dialog.GetPath().ToStdString();
            m_hot_pixel_identifier->save_hot_pixels_to_file(file_address);
        }
    }, id);

    id = unique_counter();
    hot_pixel_menu->Append(id, "Load hot pixel info", "Load hot pixel info");
    Bind(wxEVT_MENU, [this](wxCommandEvent&){
        const std::string default_path = m_recent_paths_handler.get_recent_file_path(FileTypes::LIGHT, "");
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

void MyFrame::add_menu_bar()    {
    m_menu_bar = new wxMenuBar;

    add_file_menu();
    add_alignment_menu();
    add_hot_pixel_menu();

    SetMenuBar(m_menu_bar);
};

void MyFrame::add_files_to_stack_checkbox()  {
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY);

    // Create a sizer for the panel
    wxBoxSizer* headerSizer = new wxBoxSizer(wxHORIZONTAL);
    headerPanel->SetSizer(headerSizer);

    // Create the static texts
    wxStaticText* sortByNameText = new wxStaticText(headerPanel, wxID_ANY, "Sort by Name",wxDefaultPosition);
    wxStaticText* sortByScoreText = new wxStaticText(headerPanel, wxID_ANY, "Sort by Score",wxDefaultPosition);

    wxBitmap arrow_down_bitmap("../data/png/arrows/arrow_down_20x10.png", wxBITMAP_TYPE_PNG);
    wxBitmap arrow_up_bitmap("../data/png/arrows/arrow_up_20x10.png", wxBITMAP_TYPE_PNG);
    wxSize arrow_size(40, 20);
    wxBitmapButton* sortByNameArrowUpButton = new wxBitmapButton(headerPanel, wxID_ANY, arrow_up_bitmap, wxDefaultPosition, arrow_size);
    wxBitmapButton* sortByNameArrowDownButton = new wxBitmapButton(headerPanel, wxID_ANY, arrow_down_bitmap, wxDefaultPosition, arrow_size);
    wxBitmapButton* sortByScoreArrowUpButton = new wxBitmapButton(headerPanel, wxID_ANY, arrow_up_bitmap, wxDefaultPosition, arrow_size);
    wxBitmapButton* sortByScoreArrowDownButton = new wxBitmapButton(headerPanel, wxID_ANY, arrow_down_bitmap, wxDefaultPosition, arrow_size);

    // Create a vertical sizer for the name arrows
    wxBoxSizer* nameArrowSizer = new wxBoxSizer(wxVERTICAL);
    nameArrowSizer->Add(sortByNameArrowUpButton, 0, wxTOP | wxLEFT | wxRIGHT, 5);
    nameArrowSizer->Add(sortByNameArrowDownButton, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);

    // Create a vertical sizer for the score arrows
    wxBoxSizer* scoreArrowSizer = new wxBoxSizer(wxVERTICAL);
    scoreArrowSizer->Add(sortByScoreArrowUpButton, 0, wxTOP | wxLEFT | wxRIGHT, 5);
    scoreArrowSizer->Add(sortByScoreArrowDownButton, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);

    // Add the static texts and arrow sizers to the header sizer
    headerSizer->Add(sortByNameText, 0, wxTOP, 5);
    headerSizer->Add(nameArrowSizer, 0, wxTOP, 5);
    headerSizer->Add(sortByScoreText, 0, wxTOP, 5);
    headerSizer->Add(scoreArrowSizer, 0, wxTOP, 5);


    sortByScoreArrowUpButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        this->m_filelist_handler.sort_by_alignment_ranking(true);
        update_files_to_stack_checkbox();
    });
    sortByScoreArrowDownButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        this->m_filelist_handler.sort_by_alignment_ranking(false);
        update_files_to_stack_checkbox();
    });

    sortByNameArrowUpButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        this->m_filelist_handler.sort_by_filename(true);
        update_files_to_stack_checkbox();
    });
    sortByNameArrowDownButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        this->m_filelist_handler.sort_by_filename(false);
        update_files_to_stack_checkbox();
    });


    wxArrayString files;
    m_files_to_stack_checkbox = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, files, wxLB_MULTIPLE);

    // Create a new sizer for the header and the checkbox list
    wxBoxSizer* checkboxSizer = new wxBoxSizer(wxVERTICAL);
    checkboxSizer->Add(headerPanel, 0, wxEXPAND | wxALL, 5);
    checkboxSizer->Add(m_files_to_stack_checkbox, 1, wxEXPAND | wxALL, 5);

    // Add the new sizer to the main frame's sizer
    m_sizer_main_frame->Add(checkboxSizer, 9, wxEXPAND | wxALL, 5);

    m_files_to_stack_checkbox->Bind(wxEVT_LISTBOX, [this](wxCommandEvent &event){
        int index = event.GetSelection();
        string text = m_files_to_stack_checkbox->GetString(index).ToStdString();
        const std::vector<std::string> elements = AstroPhotoStacker::split_and_strip_string(text, "\t\t");
        const std::string file = elements[1];
        const bool update_needed = update_checked_files_in_filelist();

        // Do not update the preview if the files was just checked/unchecked - it is slow
        if (!update_needed) {
            update_image_preview_file(file);
        }
    });
};

void MyFrame::update_files_to_stack_checkbox()   {
    m_files_to_stack_checkbox->Clear();
    for (FileTypes type : {FileTypes::LIGHT, FileTypes::DARK, FileTypes::FLAT, FileTypes::BIAS})   {
        const vector<string> &file_names = m_filelist_handler.get_files(type);
        for (unsigned int i_file = 0; i_file < file_names.size(); i_file++) {
            const std::string file = file_names[i_file];
            // aperture, exposure time, ISO, and focal length
            std::string metadata_string = "";
            if (type == FileTypes::LIGHT)   {
                const std::tuple<float, float, int, float> metadata = AstroPhotoStacker::read_metadata(file);
                const AlignmentFileInfo alignment_info = m_filelist_handler.get_alignment_info(FileTypes::LIGHT)[i_file];
                const float alignment_score = alignment_info.ranking;
                metadata_string =   "\t\t f/" + AstroPhotoStacker::round_and_convert_to_string(get<0>(metadata)) +
                                    "\t\t" + AstroPhotoStacker::round_and_convert_to_string(get<1>(metadata)) + " s"
                                    "\t\t" + to_string(get<2>(metadata)) + " ISO" +
                                    "\t\t\tscore: " + AstroPhotoStacker::round_and_convert_to_string(alignment_score, 3);
            }
            const std::string file_string = to_string(type) + "\t\t" + file + metadata_string;
            m_files_to_stack_checkbox->Append(file_string);

            if (m_filelist_handler.get_files_checked(type).at(i_file)) {
                m_files_to_stack_checkbox->Check(i_file);
            }
        }
    }
};

bool MyFrame::update_checked_files_in_filelist() {
    wxArrayInt checked_indices;
    m_files_to_stack_checkbox->GetCheckedItems(checked_indices);
    bool updated = false;
    for (int i = 0; i < m_filelist_handler.get_number_of_all_files(); i++) {
        const bool file_checked_in_checkbox = m_files_to_stack_checkbox->IsChecked(i);
        const bool file_checked_in_filelist = m_filelist_handler.file_is_checked(i);
        if (file_checked_in_checkbox != file_checked_in_filelist) {
            m_filelist_handler.set_file_checked(i, file_checked_in_checkbox);
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
        FilelistHandler checked_filelist = this->m_filelist_handler.get_filelist_with_checked_files();
        if (checked_filelist.get_files(FileTypes::LIGHT).empty()) {
            wxMessageDialog *dialog = new wxMessageDialog(this, "No light frames have been checked. Please check them first!", "Frames alignment warning.");
            dialog->ShowModal();
            return;
        }
        AlignmentFrame *select_alignment_window = new AlignmentFrame(this, &m_filelist_handler, &m_stack_settings);
        select_alignment_window->Show(true);
    });

    button_remove_checked->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        // get checked files:
        wxArrayInt checked_indices;
        m_files_to_stack_checkbox->GetCheckedItems(checked_indices);

        // remove checked files from m_filelist_handler
        for (int i = checked_indices.GetCount() - 1; i >= 0; --i) {
            const std::string option = m_files_to_stack_checkbox->GetString(checked_indices[i]).ToStdString();
            m_filelist_handler.remove_file(checked_indices[i]);
        }

        // update m_files_to_stack_checkbox
        update_files_to_stack_checkbox();
    });

    button_hot_pixel_id->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        update_checked_files_in_filelist();
        vector<string> files;
        for (FileTypes type : {FileTypes::LIGHT, FileTypes::DARK, FileTypes::FLAT, FileTypes::BIAS})   {
            for (unsigned int i = 0; i < m_filelist_handler.get_files(type).size(); i++)   {
                if (m_filelist_handler.get_files_checked(type)[i]) {
                    files.push_back(m_filelist_handler.get_files(type)[i]);
                }
            }
        }

        if (files.empty()) {
            wxMessageDialog *dialog = new wxMessageDialog(this, "No files have been selected. Please select them first!", "Hot pixel identification warning.");
            dialog->ShowModal();
            return;
        }

        m_hot_pixel_identifier = make_unique<AstroPhotoStacker::HotPixelIdentifier>();
        m_hot_pixel_identifier->set_n_cpu(m_stack_settings.get_n_cpus());
        const std::atomic<int> &n_processed = m_hot_pixel_identifier->get_number_of_processed_photos();
        const int files_total = files.size();

        wxProgressDialog progress_bar("Hot pixel identification", "Processed 0 / " + std::to_string(files_total) + " files", files_total, nullptr, wxPD_AUTO_HIDE | wxPD_APP_MODAL);
        progress_bar.Update(n_processed);

        thread_pool pool(1);
        pool.submit([this, files](){
            m_hot_pixel_identifier->add_photos(files);
        });

        while (pool.get_tasks_total()) {
            progress_bar.Update(n_processed, "Processed " + std::to_string(n_processed) + " / " + std::to_string(files_total) + " files");
            wxMilliSleep(100);
        }
        progress_bar.Close();
        progress_bar.Destroy();

        pool.wait_for_tasks();

        m_hot_pixel_identifier->compute_hot_pixels();

        std::vector<std::tuple<int,int>> hot_pixels = m_hot_pixel_identifier->get_hot_pixels();

        update_status_icon(m_hot_pixel_status_icon, true);
    });

    button_stack_files->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        update_checked_files_in_filelist();
        const bool files_aligned = m_filelist_handler.all_checked_files_are_aligned();
        if (!files_aligned) {
            wxMessageDialog dialog(this, "Please align the files first!", "Files not aligned");
            if (dialog.ShowModal() == wxID_YES) {
                AlignmentFrame *select_alignment_window = new AlignmentFrame(this, &m_filelist_handler, &m_stack_settings);
                select_alignment_window->Show(true);
            }
            else {
                return;
            }
        }

        m_stacker = get_configured_stacker(m_stack_settings, m_filelist_handler);
        if (this->m_stack_settings.use_hot_pixel_correction()) {
            m_stacker->set_hot_pixels(m_hot_pixel_identifier->get_hot_pixels());
        }
        const int tasks_total = m_stacker->get_tasks_total();
        const std::atomic<int> &tasks_processed = m_stacker->get_tasks_processed();

        wxProgressDialog progress_bar("File stacking", "Finished 0 / " + std::to_string(tasks_total), tasks_total, nullptr, wxPD_AUTO_HIDE | wxPD_APP_MODAL);
        progress_bar.Update(tasks_processed);

        thread_pool pool(1);
        pool.submit([this](){
            m_stacker->calculate_stacked_photo();
        });

        while (pool.get_tasks_total()) {
            if (tasks_processed < tasks_total) {
                progress_bar.Update(tasks_processed, "Finished " + std::to_string(tasks_processed) + " / " + std::to_string(tasks_total));
            }
            else {
                progress_bar.Update(tasks_total-1, "Calculating final image ...");
            }
            wxMilliSleep(100);
        }
        pool.wait_for_tasks();

        progress_bar.Close();
        progress_bar.Destroy();


        update_image_preview_with_stacked_image();

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
};

void MyFrame::add_n_cpu_slider()    {
    const int max_cpu = m_stack_settings.get_max_threads();
    const int default_value = std::max<int>(max_cpu/2,1);

    // Create a wxStaticText to display the current value
    wxStaticText* n_cpu_text = new wxStaticText(this, wxID_ANY, wxString::Format(wxT("Number of CPUs: %d"), default_value ));
    m_stack_settings.set_n_cpus(default_value);

    // Create the wxSlider
    wxSlider* slider_ncpu = new wxSlider(this, wxID_ANY, default_value, 1, max_cpu, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);

    // Bind the slider's wxEVT_SLIDER event to a lambda function that updates the value text
    slider_ncpu->Bind(wxEVT_SLIDER, [n_cpu_text, slider_ncpu, this](wxCommandEvent&){
        int current_value = slider_ncpu->GetValue();
        (this->m_stack_settings).set_n_cpus(current_value);
        n_cpu_text->SetLabel(wxString::Format(wxT("Number of CPUs: %d"), current_value));
    });

    // Add the controls to a sizer
    m_sizer_top_left->Add(n_cpu_text, 0,   wxEXPAND, 5);
    m_sizer_top_left->Add(slider_ncpu, 0,  wxEXPAND, 5);
};

void MyFrame::add_stacking_algorithm_choice_box()  {
    wxStaticText* stacking_algorithm_text = new wxStaticText(this, wxID_ANY, "Stacking algorithm:");

    wxString stacking_algorithms[m_stack_settings.get_stacking_algorithms().size()];
    for (unsigned int i = 0; i < m_stack_settings.get_stacking_algorithms().size(); ++i) {
        stacking_algorithms[i] = m_stack_settings.get_stacking_algorithms()[i];
    }
    wxChoice* choice_box_stacking_algorithm = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_stack_settings.get_stacking_algorithms().size(), stacking_algorithms);
    choice_box_stacking_algorithm->SetSelection(0);
    choice_box_stacking_algorithm->Bind(wxEVT_CHOICE, [choice_box_stacking_algorithm, this](wxCommandEvent&){
        int current_selection = choice_box_stacking_algorithm->GetSelection();
        (this->m_stack_settings).set_stacking_algorithm(choice_box_stacking_algorithm->GetString(current_selection).ToStdString());
        update_kappa_sigma_visibility();
        update_cut_off_average_visibility();
    });

    int current_selection = choice_box_stacking_algorithm->GetSelection();
    m_stack_settings.set_stacking_algorithm(choice_box_stacking_algorithm->GetString(current_selection).ToStdString());

    m_sizer_top_left->Add(stacking_algorithm_text, 0, wxEXPAND, 5);
    m_sizer_top_left->Add(choice_box_stacking_algorithm, 0,  wxEXPAND, 5);

    add_kappa_sigma_options();
    add_cut_off_average_options();



    m_luminance_stretching_slider = new ThreePointSlider(this, wxID_ANY, wxDefaultPosition, wxSize(300, 50));
    m_sizer_top_left->Add(m_luminance_stretching_slider, 0, wxEXPAND, 5);
    m_luminance_stretching_slider->set_thumbs_positions(vector<float>({0., 0.5, 1.}));
    m_current_preview->set_stretcher(&m_color_stretcher);
    m_color_stretcher.add_luminance_stretcher(IndividualColorStretchingTool());
    m_luminance_stretching_slider->register_on_change_callback([this](){
        const float thumb1 = m_luminance_stretching_slider->get_value(0);
        const float thumb2 = m_luminance_stretching_slider->get_value(1);
        const float thumb3 = m_luminance_stretching_slider->get_value(2);
        IndividualColorStretchingTool &luminance_stretcher = m_color_stretcher.get_luminance_stretcher(0);
        luminance_stretcher.set_stretching_parameters(thumb1, thumb2, thumb3);
        update_image_preview();
    });

};

void MyFrame::add_kappa_sigma_options() {
    wxBoxSizer* kappa_sigma_sizer  = new wxBoxSizer(wxHORIZONTAL);
    m_sizer_top_left->Add(kappa_sigma_sizer,   1, wxEXPAND | wxALL, 5);

    wxBoxSizer* kappa_sizer  = new wxBoxSizer(wxVERTICAL);
    kappa_sigma_sizer->Add(kappa_sizer, 0, wxEXPAND, 5);
    m_kappa_text = new wxStaticText(this, wxID_ANY, "Kappa:");
    m_spin_ctrl_kappa = new wxSpinCtrlDouble(this, wxID_ANY, "3.0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 6, 2, 0.1);

    m_spin_ctrl_kappa->Bind(wxEVT_SPINCTRLDOUBLE, [this](wxCommandEvent&){
        double current_value = m_spin_ctrl_kappa->GetValue();
        (this->m_stack_settings).set_kappa(current_value);
    });
    double current_kappa = m_spin_ctrl_kappa->GetValue();
    m_stack_settings.set_kappa(current_kappa);

    kappa_sizer->Add(m_kappa_text, 0, wxEXPAND, 5);
    kappa_sizer->Add(m_spin_ctrl_kappa, 0,  wxEXPAND, 5);

    wxBoxSizer* kappa_sigma_iter_sizer  = new wxBoxSizer(wxVERTICAL);
    kappa_sigma_sizer->Add(kappa_sigma_iter_sizer, 0, wxEXPAND, 5);
    m_kappa_sigma_iter_text = new wxStaticText(this, wxID_ANY, "Iterations:");
    m_spin_ctrl_kappa_sigma_iter = new wxSpinCtrl(this, wxID_ANY, "3", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 3);

    m_spin_ctrl_kappa_sigma_iter->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&){
        int current_value = m_spin_ctrl_kappa_sigma_iter->GetValue();
        (this->m_stack_settings).set_kappa_sigma_iter(current_value);
    });
    int current_n_iter = m_spin_ctrl_kappa_sigma_iter->GetValue();
    m_stack_settings.set_kappa_sigma_iter(current_n_iter);

    kappa_sigma_iter_sizer->Add(m_kappa_sigma_iter_text, 0, wxEXPAND, 5);
    kappa_sigma_iter_sizer->Add(m_spin_ctrl_kappa_sigma_iter, 0,  wxEXPAND, 5);
};

void MyFrame::update_kappa_sigma_visibility()   {
    if (!m_stack_settings.is_kappa_sigma()) {
        m_kappa_text->Hide();
        m_spin_ctrl_kappa->Hide();
        m_kappa_sigma_iter_text->Hide();
        m_spin_ctrl_kappa_sigma_iter->Hide();
    }
    else    {
        m_kappa_text->Show();
        m_spin_ctrl_kappa->Show();
        m_kappa_sigma_iter_text->Show();
        m_spin_ctrl_kappa_sigma_iter->Show();
    }
};

void MyFrame::add_cut_off_average_options()  {
    wxBoxSizer* main_sizer  = new wxBoxSizer(wxHORIZONTAL);
    m_sizer_top_left->Add(main_sizer,   1, wxEXPAND | wxALL, 5);

    wxBoxSizer* cut_off_sizer  = new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(cut_off_sizer, 0, wxEXPAND, 5);
    m_cut_off_average_text = new wxStaticText(this, wxID_ANY, "Cut-off fraction:");
    m_spin_ctrl_cut_off_average = new wxSpinCtrlDouble(this, wxID_ANY, "0.2", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0.49, 0.2, 0.01);

    m_spin_ctrl_cut_off_average->Bind(wxEVT_SPINCTRLDOUBLE, [this](wxCommandEvent&){
        double current_value = m_spin_ctrl_cut_off_average->GetValue();
        (this->m_stack_settings).set_cut_off_tail_fraction(current_value);
    });
    double current_cut_off = m_spin_ctrl_cut_off_average->GetValue();
    m_stack_settings.set_cut_off_tail_fraction(current_cut_off);

    cut_off_sizer->Add(m_cut_off_average_text, 0, wxEXPAND, 5);
    cut_off_sizer->Add(m_spin_ctrl_cut_off_average, 0,  wxEXPAND, 5);

};

void MyFrame::update_cut_off_average_visibility()   {
    if (m_stack_settings.get_stacking_algorithm() != "cut-off average") {
        m_cut_off_average_text->Hide();
        m_spin_ctrl_cut_off_average->Hide();
    }
    else    {
        m_cut_off_average_text->Show();
        m_spin_ctrl_cut_off_average->Show();
    }
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
        (this->m_stack_settings).set_hot_pixel_correction(is_checked);
    });
    bool is_checked = checkbox_hot_pixel_correction->GetValue();
    m_stack_settings.set_hot_pixel_correction(is_checked);

    m_sizer_top_left->Add(checkbox_hot_pixel_correction, 0, wxEXPAND, 5);
};

void MyFrame::add_color_interpolation_checkbox()    {
    wxCheckBox* checkbox_color_interpolation = new wxCheckBox(this, wxID_ANY, "Color interpolation");
    const bool is_checked = m_stack_settings.use_color_interpolation();
    checkbox_color_interpolation->SetValue(is_checked);
    checkbox_color_interpolation->SetToolTip("This will calculate all 3 color channels for each pixels, using information from its neighbors. It helps to further suppress the noise and avoid weird color artifacts, but it also leads to approx. 3 times slower stacking and might result in slightly less detailed stacked image.");
    checkbox_color_interpolation->Bind(wxEVT_CHECKBOX, [checkbox_color_interpolation, this](wxCommandEvent&){
        const bool is_checked = checkbox_color_interpolation->GetValue();
        (this->m_stack_settings).set_use_color_interpolation(is_checked);
    });
    m_sizer_top_left->Add(checkbox_color_interpolation, 0, wxEXPAND, 5);
};


void MyFrame::add_max_memory_spin_ctrl() {
    wxStaticText* memory_usage_text = new wxStaticText(this, wxID_ANY, "Maximum memory usage (MB):");

    wxSpinCtrl* spin_ctrl_max_memory = new wxSpinCtrl(this, wxID_ANY, "16000", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100000, 16000);
    spin_ctrl_max_memory->Bind(wxEVT_SPINCTRL, [spin_ctrl_max_memory, this](wxCommandEvent&){
        int current_value = spin_ctrl_max_memory->GetValue();
        (this->m_stack_settings).set_max_memory(current_value);
    });
    int current_value = spin_ctrl_max_memory->GetValue();
    m_stack_settings.set_max_memory(current_value);

    m_sizer_top_left->Add(memory_usage_text, 0, wxEXPAND, 5);
    m_sizer_top_left->Add(spin_ctrl_max_memory, 0,  wxEXPAND, 5);
};

void MyFrame::add_image_settings()   {
    add_exposure_correction_spin_ctrl();
    add_input_numbers_overview();
};

void MyFrame::add_upper_middle_panel()   {
    add_image_preview();
    add_step_control_part();
};

void MyFrame::on_mouse_wheel(wxMouseEvent& event) {
    // Get the mouse position in screen coordinates
    wxPoint screen_pos = event.GetPosition();
    screen_pos += wxPoint(0, 0.155*m_preview_size[1]);   // shift the position to the center of the image - wxStaticBitmap is buggy ...

    // Convert the mouse position to client coordinates relative to the wxStaticBitmap
    wxPoint client_position = m_preview_bitmap->ScreenToClient(screen_pos);

    // Check if the mouse is over the wxStaticBitmap
    if (wxRect(m_preview_bitmap->GetSize()).Contains(client_position)) {
        // Get the amount of rotation
        int rotation = event.GetWheelRotation();

        // Calculate the relative position of the mouse within the wxStaticBitmap
        wxSize bitmapSize = m_preview_bitmap->GetSize();
        float relative_x = static_cast<float>(client_position.x) / bitmapSize.GetWidth();
        float relative_y = static_cast<float>(client_position.y) / bitmapSize.GetHeight();

        // Check the direction of the rotation
        if (rotation > 0) {
            m_current_preview->zoom_in(relative_x, relative_y);
            update_image_preview();
        } else if (rotation < 0) {
            m_current_preview->zoom_out(relative_x, relative_y);
            update_image_preview();
        }
    }
};

void MyFrame::add_image_preview()    {
    // Create a wxImage
    wxImage image(m_preview_size[0], m_preview_size[1]);

    // Set the image to black
    for (int x = 0; x < m_preview_size[0]; ++x) {
        for (int y = 0; y < m_preview_size[1]; ++y) {
            image.SetRGB(x, y, 0,0,0);
        }
    }

    // Convert the wxImage to a wxBitmap
    wxBitmap bitmap(image);

    // Create a wxStaticBitmap to display the image
    m_preview_bitmap = new wxStaticBitmap(this, wxID_ANY, bitmap);
    this->Bind(wxEVT_MOUSEWHEEL, &MyFrame::on_mouse_wheel, this);

    // Add the wxStaticBitmap to a sizer
    m_sizer_top_center->Add(m_preview_bitmap, 1, wxCENTER, 0);
};

void MyFrame::update_image_preview_file(const std::string& file_address)  {
    m_current_preview->read_preview_from_file(file_address);
    update_image_preview();
    update_alignment_status();
};

void MyFrame::update_image_preview_with_stacked_image()  {
    const vector<vector<double>> &stacked_image = m_stacker->get_stacked_image();
    const int width = m_stacker->get_width();
    const int height = m_stacker->get_height();
    m_current_preview->read_preview_from_stacked_image(stacked_image, width, height);
    update_image_preview();
};

void MyFrame::update_image_preview()  {
    m_current_preview->update_preview_bitmap(m_preview_bitmap);
    m_sizer_top_center->Add(m_preview_bitmap, 1, wxCENTER, 0);
    update_histogram();
};

void MyFrame::add_step_control_part()    {
    wxGridSizer *grid_sizer = new wxGridSizer(2,3, 0, 0);

    auto add_button_and_checkmark = [this](auto on_button_function, const std::string &description_text, const std::string &button_text, wxStaticBitmap **status_icon, wxGridSizer *grid_sizer){
        wxStaticText* text_aligned = new wxStaticText(this, wxID_ANY, description_text);
        wxFont font = text_aligned->GetFont();
        font.SetPointSize(14);
        text_aligned->SetFont(font);
        *status_icon = new wxStaticBitmap(this, wxID_ANY, wxBitmap("../data/png/checkmarks/20px/red_cross.png", wxBITMAP_TYPE_PNG));

        wxButton *button = new wxButton(this, wxID_ANY, button_text);
        button->Bind(wxEVT_BUTTON, on_button_function);

        grid_sizer->Add(text_aligned, 0,    wxALIGN_CENTER_VERTICAL | wxALL, 5);
        grid_sizer->Add(*status_icon, 0,    wxALIGN_CENTER_VERTICAL | wxALL, 5);
        grid_sizer->Add(button, 0,          wxALIGN_CENTER_VERTICAL | wxALL, 5);
    };

    auto on_button_show_alignment = [this](wxCommandEvent&){
        vector<vector<string>> tabular_data;
        vector<string> description;
        m_filelist_handler.get_alignment_info_tabular_data(&tabular_data, &description);
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
    update_status_icon(m_alignment_status_icon, m_filelist_handler.all_checked_files_are_aligned());
};

void MyFrame::add_exposure_correction_spin_ctrl()   {
    // Create a wxStaticText to display the current value
    wxStaticText* exposure_correction_text = new wxStaticText(this, wxID_ANY, "Exposure correction: 0.0");

    // Create the wxSlider
    wxSlider* slider_exposure = new wxSlider(this, wxID_ANY, 0, -70, 70, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);

    // Bind the slider's wxEVT_SLIDER event to a lambda function that updates the value text
    slider_exposure->Bind(wxEVT_SLIDER, [exposure_correction_text, slider_exposure, this](wxCommandEvent&){
        const float exposure_correction = slider_exposure->GetValue()/10.;
        m_current_preview->set_exposure_correction( exposure_correction );
        update_image_preview();
        const std::string new_label = "Exposure correction: " + to_string(exposure_correction+0.0001).substr(0,4);
        exposure_correction_text->SetLabel(new_label);
    });

    // Add the controls to a sizer
    //wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    m_sizer_top_right->Add(exposure_correction_text, 0,   wxEXPAND, 5);
    m_sizer_top_right->Add(slider_exposure, 0,  wxEXPAND, 5);

};

void MyFrame::add_input_numbers_overview()  {
    wxGridSizer *grid_sizer = new wxGridSizer(5,3, 0, 0);
    wxStaticText* text_frame_type = new wxStaticText(this, wxID_ANY, "Frame type");
    wxStaticText* text_total = new wxStaticText(this, wxID_ANY, "Total");
    wxStaticText* text_checked = new wxStaticText(this, wxID_ANY, "Checked");

    grid_sizer->Add(text_frame_type, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    grid_sizer->Add(text_total, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    grid_sizer->Add(text_checked, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    auto add_summary_text = [this, grid_sizer](FileTypes type, const std::string& label) {
        wxStaticText* text = new wxStaticText(this, wxID_ANY, label);
        wxStaticText* text_number = new wxStaticText(this, wxID_ANY, std::to_string(m_filelist_handler.get_files(type).size()));
        wxStaticText* text_checked = new wxStaticText(this, wxID_ANY, std::to_string(m_filelist_handler.get_number_of_checked_files(type)));
        m_frames_numbers_overview_texts[type] = {text_number, text_checked};

        grid_sizer->Add(text, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        grid_sizer->Add(text_number, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        grid_sizer->Add(text_checked, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    };

    add_summary_text(FileTypes::LIGHT, "Light frames: ");
    add_summary_text(FileTypes::FLAT, "Flat frames: ");
    add_summary_text(FileTypes::BIAS, "Bias frames: ");
    add_summary_text(FileTypes::DARK, "Dark frames: ");

    m_sizer_top_right->Add(grid_sizer, 0, wxEXPAND, 5);

    m_histogram_data_tool_gui = std::make_unique<HistogramDataToolGUI>(this, m_sizer_top_right, wxDefaultPosition, wxSize(600,250));
    m_histogram_data_tool_gui->set_background_color(wxColour(200,200,200));
    m_histogram_data_tool_gui->set_line_colors({wxColour(255,0,0), wxColour(0,255,0), wxColour(0,0,255)});
};

void MyFrame::update_input_numbers_overview()   {
    for (FileTypes type : {FileTypes::LIGHT, FileTypes::DARK, FileTypes::FLAT, FileTypes::BIAS})   {
        m_frames_numbers_overview_texts[type].first->SetLabel(std::to_string(m_filelist_handler.get_files(type).size()));
        m_frames_numbers_overview_texts[type].second->SetLabel(std::to_string(m_filelist_handler.get_number_of_checked_files(type)));
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
    m_histogram_data_tool = std::make_unique<HistogramDataTool>(pow(2,13), 3);
    m_histogram_data_tool->extract_data_from_image(m_current_preview->get_original_image());
    m_histogram_data_tool_gui->set_color_stretcher(m_color_stretcher);

    m_histogram_data_tool_gui->set_histogram_data_colors(*m_histogram_data_tool);
};

void MyFrame::on_open_frames(wxCommandEvent& event, FileTypes type, const std::string& title)    {
    const std::string default_path = m_recent_paths_handler.get_recent_file_path(type, "");
    wxFileDialog dialog(this, title, "", default_path, "*[!'.txt']", wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dialog.ShowModal() == wxID_OK) {
        wxArrayString paths;
        dialog.GetPaths(paths);
        for (auto path : paths) {
            if (type == FileTypes::LIGHT) {
                const bool this_file_is_raw_file = AstroPhotoStacker::is_raw_file(path.ToStdString());
                if (!this_file_is_raw_file) {
                    wxMessageDialog *dialog = new wxMessageDialog(this, "The file " + path + " is not a raw file. Please select raw files only.", "File type error");
                    dialog->ShowModal();
                    continue;
                }
            }
            m_filelist_handler.add_file(path.ToStdString(), type);
            m_recent_paths_handler.set_recent_file_path_from_file(type, path.ToStdString());
        }
    }
    dialog.Destroy();
    update_checked_files_in_filelist();
    update_files_to_stack_checkbox();
};

void MyFrame::on_open_lights(wxCommandEvent& event)    {
    on_open_frames(event, FileTypes::LIGHT, "Open light frames");
}

void MyFrame::on_open_flats(wxCommandEvent& event)    {
    on_open_frames(event, FileTypes::FLAT, "Open flat frames");
}

void MyFrame::on_save_stacked(wxCommandEvent& event) {
    wxFileDialog dialog(this, "Save stacked file", "", "", "*['.tif']", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dialog.ShowModal() == wxID_OK) {
        std::string file_address = dialog.GetPath().ToStdString();

        // if the extension is not .tif, add it
        if (file_address.substr(file_address.size()-4) != ".tif") {
            file_address += ".tif";
        }
        m_stacker->save_stacked_photo(file_address, CV_16UC3);
    }
};

void MyFrame::update_status_icon(wxStaticBitmap *status_icon, bool is_ok)   {
    const std::string file_checkmark    = "../data/png/checkmarks/20px/checkmark.png";
    const std::string file_cross        = "../data/png/checkmarks/20px/red_cross.png";
    if (is_ok)  {
        status_icon->SetBitmap(wxBitmap(file_checkmark, wxBITMAP_TYPE_PNG));
    }
    else    {
        status_icon->SetBitmap(wxBitmap(file_cross, wxBITMAP_TYPE_PNG));
    }
};