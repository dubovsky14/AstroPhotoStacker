// Start of wxWidgets "Hello World" Program
#pragma once

#include "../headers/FilelistHandler.h"
#include "../headers/StackSettings.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <memory>

class MyApp : public wxApp  {
    public:
        bool OnInit() override;
};

class MyFrame : public wxFrame  {
    public:
        MyFrame();

    private:
        void add_file_menu();

        void add_menu_bar();

        void add_files_to_stack_checkbox();
        void update_files_to_stack_checkbox();

        void add_button_bar();

        void add_stack_settings_preview();
        void add_image_preview();
        void update_image_preview(const std::string& file_address);

        void add_image_settings();
        void add_n_cpu_slider();
        void add_max_memory_spin_ctrl();
        void add_stacking_algorithm_choice_box();
        void add_kappa_sigma_options();
        void update_kappa_sigma_visibility();


        wxPanel     *m_panel_top        = nullptr;
        wxBoxSizer  *m_sizer_main_frame = nullptr;
        wxBoxSizer  *m_sizer_button_bar = nullptr;
        wxBoxSizer  *m_sizer_top        = nullptr;
        wxMenuBar   *m_menu_bar         = nullptr;
        wxMenu      *m_file_menu        = nullptr;

        // upper part: stack settings, image preview and image settings
        wxBoxSizer  *m_sizer_top_left   = nullptr;
        wxBoxSizer  *m_sizer_top_center = nullptr;
        wxBoxSizer  *m_sizer_top_right  = nullptr;


        wxStaticText        *m_kappa_text                   = nullptr;
        wxSpinCtrlDouble    *m_spin_ctrl_kappa              = nullptr;
        wxStaticText        *m_kappa_sigma_iter_text        = nullptr;
        wxSpinCtrl          *m_spin_ctrl_kappa_sigma_iter   = nullptr;

        int                 m_preview_size[2]               = {600, 400};
        wxStaticBitmap      *m_preview_bitmap               = nullptr;


        wxCheckListBox *m_files_to_stack_checkbox = nullptr;



        FilelistHandler m_filelist_handler;
        StackSettings   m_stack_settings;

        void on_open_frames(wxCommandEvent& event, FileTypes type, const std::string& title);
        void on_open_lights(wxCommandEvent& event);
        void on_open_flats (wxCommandEvent& event);
        void on_exit(wxCommandEvent& event);


};

inline int unique_counter()    {
    static int counter = 1000;
    return ++counter;
};

enum    {
    ID_Hello = 1
};
