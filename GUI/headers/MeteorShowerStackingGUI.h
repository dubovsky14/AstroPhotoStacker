#pragma once

#include "../headers/ImagePreview.h"
#include "../headers/MainFrame.h"
#include "../headers/FilelistHandlerGUIInterface.h"
#include "../../headers/MeteorShowerStackingTool.h"
#include "../../headers/InputFrame.h"
#include "../../headers/KDTree.h"


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
        MeteorShowerStackingGUI(MyFrame *parent, int n_cpus);


    private:

        MyFrame *m_parent = nullptr;
        wxSize m_window_size;
        int m_image_preview_width = 600;
        int m_image_preview_height = 400;

        AstroPhotoStacker::MeteorShowerStackingTool m_meteor_shower_stacking_tool;

        wxBoxSizer *m_main_vertical_sizer = nullptr;
        wxBoxSizer *m_upper_part_sizer_horizontal = nullptr;

        AstroPhotoStacker::InputFrame get_reference_frame() const;



        // Image preview
        wxBoxSizer *m_image_preview_sizer = nullptr;
        std::unique_ptr<ImagePreview> m_image_preview = nullptr;
        std::unique_ptr<FloatingPointSlider> m_exposure_correction_slider   = nullptr;
        CombinedColorStrecherTool m_exposure_stretcher; // for exposure correction
        void add_exposure_correction_spin_ctrl();
        AstroPhotoStacker::InputFrame m_currently_displayed_frame;
        void update_clusters_in_preview();
        bool select_unselect_cluster_from_preview(int index_in_cluster_info);

        wxBoxSizer *m_top_right_sizer = nullptr;

        // cluster list
        void add_list_of_clusters();
        void update_cluster_list();
        std::vector<std::pair<unsigned int,unsigned int>> m_cluster_id_to_index_in_gui;
        wxCheckListBox *m_clusters_checkbox = nullptr;

         // for fast search of clusters in the image in on-click events. Key point (x,y) of pixel, value is cluster index in cluster_info.clusters vector
        std::unique_ptr<AstroPhotoStacker::KDTree<int,2,int>> m_cluster_kd_tree = nullptr;

        // cluster buttons
        void add_cluster_buttons();
        wxBoxSizer *m_cluster_buttons_sizer = nullptr;
        wxButton *m_button_show_cluster = nullptr;
        wxButton *m_button_recalculate_clusters = nullptr;
        wxButton *m_button_recalculate_clusters_for_all_images = nullptr;
        bool m_show_clusters = true;

        // cluster setttings
        void add_cluster_settings();
        std::unique_ptr<FloatingPointSlider> m_cluster_threshold_slider   = nullptr;
        std::unique_ptr<FloatingPointSlider> m_cluster_excentricity_slider   = nullptr;
        std::unique_ptr<FloatingPointSlider> m_cluster_correlation_slider   = nullptr;
        float m_cluster_threshold = 0.01;
        float m_cluster_excentricity = 10.0;
        float m_cluster_correlation = 0.5;


        // buttons
        void add_buttons();
        wxBoxSizer *m_buttons_sizer = nullptr;
        wxButton *m_button_check_all = nullptr;
        wxButton *m_button_remove_checked = nullptr;
        wxButton *m_button_stack = nullptr;
        wxButton *m_button_show_stacked_image = nullptr;
        wxButton *m_button_save_stacked_image = nullptr;

        // background frame selection
        AstroPhotoStacker::InputFrame m_background_frame;
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




};
