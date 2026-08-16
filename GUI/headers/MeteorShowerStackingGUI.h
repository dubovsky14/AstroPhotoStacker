#pragma once

#include "../headers/ImagePreview.h"
#include "../headers/MainFrame.h"
#include "../headers/FilelistHandlerGUIInterface.h"

#include "../../headers/InputFrame.h"


#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <memory>
#include <vector>
#include <string>


/**
 * @brief Frame (dialog window) for meteor shower stacking

*/
class MeteorShowerStackingGUI : public wxFrame  {
    public:
        /**
         * @brief Construct a new Meteor Shower Stacking GUI object
         *
         * @param parent pointer to the parent frame (main frame)
         * @param aligned_images_producer pointer to the aligned images producer object
         */
        MeteorShowerStackingGUI(MyFrame *parent);


    private:
        void add_exposure_correction_spin_ctrl();

        AstroPhotoStacker::InputFrame get_reference_frame() const;

        // buttons
        void add_buttons();
        wxBoxSizer *m_buttons_sizer = nullptr;
        wxButton *m_button_check_all = nullptr;
        wxButton *m_button_remove_checked = nullptr;
        wxButton *m_button_stack = nullptr;
        wxButton *m_button_show_stacked_image = nullptr;
        wxButton *m_button_save_stacked_image = nullptr;

        // background frame selection
        void add_background_frame_selector();
        std::vector<int>                            m_indices_frames_to_align;
        std::vector<AstroPhotoStacker::InputFrame>  m_available_light_frames;
        std::vector<wxString>                       m_available_light_frames_strings;

        // filelist
        FilelistHandlerGUIInterface m_filelist_handler_gui_interface;
        wxCheckListBox *m_files_checkbox = nullptr;

        void add_list_of_files();
        void update_image_preview_file(size_t frame_index);
        bool update_checked_files_in_filelist();
        void update_files_to_stack_checkbox();


        MyFrame *m_parent = nullptr;
        wxSize m_window_size;
        int m_image_preview_width = 600;
        int m_image_preview_height = 400;

        wxBoxSizer *m_main_vertical_sizer = nullptr;

        wxBoxSizer *m_upper_part_sizer_horizontal = nullptr;

        wxBoxSizer *m_image_preview_sizer = nullptr;

        std::unique_ptr<ImagePreview> m_image_preview = nullptr;
        std::unique_ptr<FloatingPointSlider> m_exposure_correction_slider   = nullptr;
        CombinedColorStrecherTool m_exposure_stretcher; // for exposure correction

        AstroPhotoStacker::InputFrame m_background_frame;


};
