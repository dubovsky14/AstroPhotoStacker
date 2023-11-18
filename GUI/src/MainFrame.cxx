#include "../headers/MainFrame.h"
#include "../headers/AlignmentFrame.h"
#include "../headers/ImagePreview.h"
#include "../headers/ListFrame.h"
#include "../headers/StackerConfigureTool.h"

#include "../../headers/Common.h"
#include "../../headers/thread_pool.h"

#include <wx/spinctrl.h>
#include <wx/progdlg.h>

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
    //m_panel_top     = new wxPanel(this, wxID_ANY);
    //m_sizer_main_frame->Add(m_panel_top   , 1,wxEXPAND | wxALL, 5);
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

void MyFrame::add_menu_bar()    {
    m_menu_bar = new wxMenuBar;

    add_file_menu();

    SetMenuBar(m_menu_bar);
};

void MyFrame::add_files_to_stack_checkbox()  {
    wxArrayString files;
    m_files_to_stack_checkbox = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, files, wxLB_MULTIPLE);
    m_sizer_main_frame->Add(m_files_to_stack_checkbox, 9,wxEXPAND | wxALL, 5);

    m_files_to_stack_checkbox->Bind(wxEVT_LISTBOX, [this](wxCommandEvent &event){
        int index = event.GetSelection();
        string text = m_files_to_stack_checkbox->GetString(index).ToStdString();
        const std::string file = text.substr(text.find_last_of("\t") + 1);
        update_image_preview_file(file);
    });
};

void MyFrame::update_files_to_stack_checkbox()   {
    m_files_to_stack_checkbox->Clear();
    for (FileTypes type : {FileTypes::LIGHT, FileTypes::DARK, FileTypes::FLAT, FileTypes::BIAS})   {
        for (auto file : m_filelist_handler.get_files(type))   {
            m_files_to_stack_checkbox->Append(to_string(type) + "\t\t" + file);
        }
    }
};

void MyFrame::update_checked_files_in_filelist() {
    wxArrayInt checked_indices;
    m_files_to_stack_checkbox->GetCheckedItems(checked_indices);

    m_filelist_handler.set_checked_status_for_all_files(false);

    // loop over checked items:
    for (size_t i = 0; i < checked_indices.GetCount(); ++i) {
        int index = checked_indices[i];
        m_filelist_handler.set_file_checked(index, true);
    }
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
            return;
        }
        else {
            for (unsigned int i = 0; i < m_files_to_stack_checkbox->GetCount(); ++i) {
                m_files_to_stack_checkbox->Check(i);
            }
            button_check_all->SetLabel("Uncheck all");
        }
        update_checked_files_in_filelist();
        update_alignment_status();
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
        cout << "Hot pixels identified!" << endl;
        cout << "Processed " + std::to_string(n_processed) + " / " + std::to_string(files_total) + " files\n";
        for (const auto &hot_pixel : hot_pixels) {
            cout << get<0>(hot_pixel) << "\t" << get<1>(hot_pixel) << endl;
        }
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
            progress_bar.Update(tasks_processed, "Finished " + std::to_string(tasks_processed) + " / " + std::to_string(tasks_total));
            wxMilliSleep(100);
        }

        pool.wait_for_tasks();

        update_image_preview_with_stacked_image();

        progress_bar.Close();
        progress_bar.Destroy();
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
    //wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
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
    });

    int current_selection = choice_box_stacking_algorithm->GetSelection();
    m_stack_settings.set_stacking_algorithm(choice_box_stacking_algorithm->GetString(current_selection).ToStdString());

    m_sizer_top_left->Add(stacking_algorithm_text, 0, wxEXPAND, 5);
    m_sizer_top_left->Add(choice_box_stacking_algorithm, 0,  wxEXPAND, 5);

    add_kappa_sigma_options();
};

void MyFrame::add_kappa_sigma_options() {
    wxBoxSizer* kappa_sigma_sizer  = new wxBoxSizer(wxHORIZONTAL);
    m_sizer_top_left->Add(kappa_sigma_sizer,   1, wxEXPAND | wxALL, 5);

    wxBoxSizer* kappa_sizer  = new wxBoxSizer(wxVERTICAL);
    kappa_sigma_sizer->Add(kappa_sizer, 0, wxEXPAND, 5);
    m_kappa_text = new wxStaticText(this, wxID_ANY, "Kappa:");
    m_spin_ctrl_kappa = new wxSpinCtrlDouble(this, wxID_ANY, "3.0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 6, 2, 0.1);

    m_spin_ctrl_kappa->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&){
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

void MyFrame::add_max_memory_spin_ctrl() {
    wxStaticText* memory_usage_text = new wxStaticText(this, wxID_ANY, "Maximum memory usage (MB):");

    wxSpinCtrl* spin_ctrl_max_memory = new wxSpinCtrl(this, wxID_ANY, "8000", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100000, 8000);
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
};

void MyFrame::add_upper_middle_panel()   {
    add_image_preview();
    add_step_control_part();
};

void MyFrame::add_image_preview()    {
    // Create a wxImage
    wxImage image(m_preview_size[0], m_preview_size[1]);

    for (int x = 0; x < m_preview_size[0]; ++x) {
        for (int y = 0; y < m_preview_size[1]; ++y) {
            image.SetRGB(x, y, 0,0,0);
        }
    }

    // Convert the wxImage to a wxBitmap
    wxBitmap bitmap(image);

    // Create a wxStaticBitmap to display the image
    m_preview_bitmap = new wxStaticBitmap(this, wxID_ANY, bitmap);

    // Add the wxStaticBitmap to a sizer
    m_sizer_top_center->Add(m_preview_bitmap, 1, wxCENTER, 0);
};

void MyFrame::update_image_preview_file(const std::string& file_address)  {
    m_current_preview = get_preview(file_address, m_preview_size[0],m_preview_size[1], &m_current_max_value);
    m_current_preview_is_raw_file = true;

    update_image_preview();
    update_alignment_status();
};

void MyFrame::update_image_preview_with_stacked_image()  {
    const vector<vector<double>> stacked_image = m_stacker->get_stacked_image();
    const int width = m_stacker->get_width();
    const int height = m_stacker->get_height();
    m_current_preview = get_preview_from_stacked_picture(stacked_image, width, height, m_preview_size[0],m_preview_size[1], &m_current_max_value);
    m_current_preview_is_raw_file = false;
    update_image_preview();
};

void MyFrame::update_image_preview()  {
    const int width = m_preview_size[0];
    const int height = m_preview_size[1];
    wxImage image_wx(width, height);

    if (m_current_preview_is_raw_file) {
        const float scale_factor = pow(2,m_current_exposure_correction)*2*255.0 / m_current_max_value;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {

                const int index = x + y*width;
                image_wx.SetRGB(x, y,   min<int>(255,scale_factor*m_current_preview[0][index]),
                                        min<int>(255,0.5*scale_factor*m_current_preview[1][index]),
                                        min<int>(255,scale_factor*m_current_preview[2][index]));
            }
        }
    }
    else {
        const float scale_factor = pow(2,m_current_exposure_correction)*255.0 / m_current_max_value;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const int index = x + y*width;
                image_wx.SetRGB(x, y,   min<int>(255,scale_factor*m_current_preview[0][index]),
                                        min<int>(255,scale_factor*m_current_preview[1][index]),
                                        min<int>(255,scale_factor*m_current_preview[2][index]));
            }
        }
    }

    // Convert the wxImage to a wxBitmap
    wxBitmap bitmap(image_wx);

    // Create a wxStaticBitmap to display the image
    m_preview_bitmap->SetBitmap(bitmap);
    m_sizer_top_center->Add(m_preview_bitmap, 1, wxCENTER, 0);
};

void MyFrame::add_step_control_part()    {


    // alignment of the files
    wxGridSizer *grid_sizer = new wxGridSizer(2,3, 0, 0);
    wxStaticText* text_aligned = new wxStaticText(this, wxID_ANY, "Files aligned: ");
    wxFont font = text_aligned->GetFont();
    font.SetPointSize(14);
    text_aligned->SetFont(font);

    m_alignment_status_icon = new wxStaticBitmap(this, wxID_ANY, wxBitmap("../data/png/checkmarks/20px/red_cross.png", wxBITMAP_TYPE_PNG));

    wxButton *button_show_alignment = new wxButton(this, wxID_ANY, "Show alignment");
    button_show_alignment->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        vector<vector<string>> tabular_data;
        vector<string> description;
        m_filelist_handler.get_alignment_info_tabular_data(&tabular_data, &description);
        if (tabular_data.empty())   {
            return;
        }
        ListFrame *alignment_list_frame = new ListFrame(this, "Alignment info", "Alignment info", description, tabular_data);
        alignment_list_frame->Show(true);
    });

    grid_sizer->Add(text_aligned, 0,             wxALIGN_CENTER_VERTICAL | wxALL, 5);
    grid_sizer->Add(m_alignment_status_icon, 0,  wxALIGN_CENTER_VERTICAL | wxALL, 5);
    grid_sizer->Add(button_show_alignment, 0,    wxALIGN_CENTER_VERTICAL | wxALL, 5);

    // hot pixel identification
    wxStaticText* text_hot_pixels = new wxStaticText(this, wxID_ANY, "Hot pixels identified: ");
    text_hot_pixels->SetFont(font);
    m_hot_pixel_status_icon = new wxStaticBitmap(this, wxID_ANY, wxBitmap("../data/png/checkmarks/20px/red_cross.png", wxBITMAP_TYPE_PNG));

    wxButton *button_show_hot_pixels = new wxButton(this, wxID_ANY, "Show hot pixels");
    button_show_hot_pixels->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
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
    });

    grid_sizer->Add(text_hot_pixels, 0,           wxALIGN_CENTER_VERTICAL | wxALL, 5);
    grid_sizer->Add(m_hot_pixel_status_icon, 0,   wxALIGN_CENTER_VERTICAL | wxALL, 5);
    grid_sizer->Add(button_show_hot_pixels, 0,    wxALIGN_CENTER_VERTICAL | wxALL, 5);

    // is the stacked image available?
    wxStaticText* text_stacked = new wxStaticText(this, wxID_ANY, "Stacking finished: ");
    text_stacked->SetFont(font);
    m_stacked_status_icon = new wxStaticBitmap(this, wxID_ANY, wxBitmap("../data/png/checkmarks/20px/red_cross.png", wxBITMAP_TYPE_PNG));

    wxButton *button_stacking_finished = new wxButton(this, wxID_ANY, "Show stacked file");
    button_stacking_finished->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        if (m_stacker == nullptr)    {
            return;
        }
        update_image_preview_with_stacked_image();

    });
    grid_sizer->Add(text_stacked, 0,                wxALIGN_CENTER_VERTICAL | wxALL, 5);
    grid_sizer->Add(m_stacked_status_icon, 0,       wxALIGN_CENTER_VERTICAL | wxALL, 5);
    grid_sizer->Add(button_stacking_finished, 0,    wxALIGN_CENTER_VERTICAL | wxALL, 5);

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
        m_current_exposure_correction = slider_exposure->GetValue()/10.;
        update_image_preview();
        const std::string new_label = "Exposure correction: " + to_string(m_current_exposure_correction+0.0001).substr(0,4);
        exposure_correction_text->SetLabel(new_label);
    });

    // Add the controls to a sizer
    //wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    m_sizer_top_right->Add(exposure_correction_text, 0,   wxEXPAND, 5);
    m_sizer_top_right->Add(slider_exposure, 0,  wxEXPAND, 5);

};

void MyFrame::on_exit(wxCommandEvent& event)     {
    Close(true);
}

void MyFrame::on_open_frames(wxCommandEvent& event, FileTypes type, const std::string& title)    {
    const std::string default_path = m_recent_paths_handler.get_recent_file_path(type, "");
    wxFileDialog dialog(this, title, "", default_path, "*[!'.txt']", wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dialog.ShowModal() == wxID_OK) {
        wxArrayString paths;
        dialog.GetPaths(paths);
        for (auto path : paths) {
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
        const std::string file_address = dialog.GetPath().ToStdString();
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