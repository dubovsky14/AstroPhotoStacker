#pragma once

#include "../headers/FilelistHandlerGUIInterface.h"
#include "../headers/StackSettingsSaver.h"
#include "../headers/ImagePreview.h"
#include "../headers/RecentPathsHandler.h"
#include "../headers/CombinedColorStrecherTool.h"
#include "../headers/ThreePointSlider.h"
#include "../headers/FloatingPointSlider.h"
#include "../headers/SummaryYamlCreator.h"

#include "../headers/HistogramDataTool.h"
#include "../headers/HistogramDataToolGUI.h"
#include "../headers/PostProcessingTool.h"

#include "../../headers/PhotoAlignmentHandler.h"
#include "../../headers/HotPixelIdentifier.h"
#include "../../headers/StackerBase.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/generic/statbmpg.h>

#include <memory>

class MyApp : public wxApp  {
    public:
        bool OnInit() override;


    void OnUnhandledException() override       {
        try {
            throw; // Rethrow the current exception to handle it
        }
        catch (const std::exception& e)     {
            wxMessageBox(e.what(), "Error", wxOK | wxICON_ERROR);
        }
        catch (...)     {
            wxMessageBox("An unknown error occurred.", "Error", wxOK | wxICON_ERROR);
        }
    }
};

class MyFrame : public wxFrame  {
    public:
        MyFrame();

        void update_alignment_status();
        void update_files_to_stack_checkbox();

        const FilelistHandlerGUIInterface& get_filelist_handler_gui_interface() const {
            return m_filelist_handler_gui_interface;
        };

        const StackSettingsSaver *get_stack_settings() const {
            return m_stack_settings.get();
        };

        void stack_calibration_frames();

        const AstroPhotoStacker::HotPixelIdentifier *get_hot_pixel_identifier() const {
            if (m_hot_pixel_identifier == nullptr) {
                return nullptr;
            }
            return m_hot_pixel_identifier.get();
        };

    private:

        void add_file_menu();
        void add_filelist_menu();
        void add_alignment_menu();
        void add_group_menu();
        void add_hot_pixel_menu();
        void add_aligned_images_producer_menu();
        void add_postprocessing_menu();

        void add_menu_bar();

        void add_files_to_stack_checkbox();

        /**
         * @brief Update the list of files to stack in the GUI
         *
         * @return true if the list was updated, false otherwise
        */
        bool update_checked_files_in_filelist();

        void add_button_bar();

        void add_stack_settings_preview();

        void add_upper_middle_panel();
        void add_image_preview();
        void update_image_preview_file(size_t frame_index);
        void update_image_preview_with_stacked_image();
        void update_image_preview();


        /**
         * @brief Add pannel with the checkmarks and buttons for alignment, hot pixel correction and stacking status
        */
        void add_step_control_part();

        wxGenericStaticBitmap *m_alignment_status_icon = nullptr;
        wxGenericStaticBitmap *m_hot_pixel_status_icon = nullptr;
        wxGenericStaticBitmap *m_stacked_status_icon   = nullptr;

        void add_n_cpu_slider();
        void add_max_memory_spin_ctrl();
        void add_stacking_algorithm_choice_box();

        std::vector<wxStaticText*>      m_algorithm_specific_settings_texts;
        std::vector<wxSpinCtrlDouble*>  m_algorithm_specific_settings_spin_ctrls;
        wxBoxSizer  *m_sizer_algorithm_specific_settings   = nullptr;
        void update_algorithm_specific_settings_gui();

        void add_hot_pixel_correction_checkbox();
        void add_color_interpolation_checkbox();
        void add_color_stretching_checkbox();
        void add_calibrated_preview_checkbox();

        void add_image_settings();
        void add_exposure_correction_spin_ctrl();
        void add_input_numbers_overview();
        void add_histogram_and_rgb_sliders();
        void update_input_numbers_overview();
        std::map<FrameType, std::pair<wxStaticText*, wxStaticText*>>  m_frames_numbers_overview_texts;

        wxPanel     *m_main_panel       = nullptr;
        wxBoxSizer  *m_sizer_main_frame = nullptr;
        wxBoxSizer  *m_sizer_button_bar = nullptr;
        wxBoxSizer  *m_sizer_top        = nullptr;
        wxMenuBar   *m_menu_bar         = nullptr;
        wxMenu      *m_file_menu        = nullptr;

        // upper part: stack settings, image preview and image settings
        wxBoxSizer  *m_sizer_top_left   = nullptr;
        wxBoxSizer  *m_sizer_top_center = nullptr;
        wxBoxSizer  *m_image_preview_size = nullptr;
        wxBoxSizer  *m_sizer_top_right  = nullptr;


        wxStaticText        *m_kappa_text                   = nullptr;
        wxSpinCtrlDouble    *m_spin_ctrl_kappa              = nullptr;
        wxStaticText        *m_kappa_sigma_iter_text        = nullptr;
        wxSpinCtrl          *m_spin_ctrl_kappa_sigma_iter   = nullptr;

        wxStaticText        *m_cut_off_average_text         = nullptr;
        wxSpinCtrlDouble    *m_spin_ctrl_cut_off_average    = nullptr;

        std::array<int, 2>   m_preview_size   = {600, 400};
        std::unique_ptr<ImagePreview>   m_current_preview = std::make_unique<ImagePreview>(this, m_preview_size[0], m_preview_size[1], 255, true);
        std::unique_ptr<FloatingPointSlider> m_exposure_correction_slider   = nullptr;

        std::unique_ptr<HistogramDataTool> m_histogram_data_tool            = nullptr;
        std::unique_ptr<HistogramDataToolGUI> m_histogram_data_tool_gui     = nullptr;

        void update_histogram();


        wxCheckListBox *m_files_to_stack_checkbox = nullptr;


        FilelistHandlerGUIInterface m_filelist_handler_gui_interface;
        std::unique_ptr<StackSettingsSaver>    m_stack_settings = nullptr;
        std::unique_ptr<AstroPhotoStacker::HotPixelIdentifier>  m_hot_pixel_identifier = nullptr;
        std::unique_ptr<AstroPhotoStacker::StackerBase>         m_stacker = nullptr;
        std::unique_ptr<SummaryYamlCreator> m_summary_yaml_creator = nullptr;

        void on_open_frames(wxCommandEvent& event, FrameType type, const std::string& title);
        void on_open_lights(wxCommandEvent& event);
        void on_open_flats (wxCommandEvent& event);
        void on_open_darks (wxCommandEvent& event);
        std::unique_ptr<RecentPathsHandler> m_recent_paths_handler = nullptr;

        void on_save_stacked(wxCommandEvent& event);
        void on_save_selected_as_fit(wxCommandEvent& event);
        void on_exit(wxCommandEvent& event);

        std::vector<std::shared_ptr<wxSizer>> m_sizers;  // this is such a mess ...

        static void update_status_icon(wxGenericStaticBitmap *status_icon, bool is_ok);

        // luminance streching slider:
        wxBoxSizer  *m_sizer_bottom_left  = nullptr;
        ThreePointSlider *m_luminance_stretching_slider = nullptr;
        CombinedColorStrecherTool m_color_stretcher;

        PostProcessingTool m_post_processing_tool;

        ThreePointSlider *m_stretching_slider_red = nullptr;
        ThreePointSlider *m_stretching_slider_green = nullptr;
        ThreePointSlider *m_stretching_slider_blue = nullptr;

        void update_color_channels_mean_and_median_values_text();
        wxStaticText* m_text_color_channels_mean_values = nullptr;
        wxStaticText* m_text_color_channels_median_values = nullptr;

        static std::string s_gui_folder_path;

        std::vector<std::pair<float,float>> m_alignment_point_vector_storage;

        int m_current_group = 0;
        bool m_show_calibrated_preview = false;
};

inline int unique_counter()    {
    static int counter = 1000;
    return ++counter;
};
