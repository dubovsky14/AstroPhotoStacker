#pragma once

#include "../headers/FilelistHandlerGUIInterface.h"
#include "../headers/StackSettings.h"
#include "../headers/ImagePreviewCropTool.h"
#include "../headers/CombinedColorStrecherTool.h"
#include "../headers/TimeLapseVideoSettings.h"
#include "../headers/FloatingPointSlider.h"
#include "../headers/PostProcessingTool.h"

#include "../../headers/AlignedImagesProducer.h"
#include "../../headers/InputFrame.h"

#include "../headers/MainFrame.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <memory>
#include <vector>
#include <string>


/**
 * @brief Frame (dialog window) for selecting the reference photo for alignment

*/
class AlignedImagesProducerGUI : public wxFrame  {
    public:
        /**
         * @brief Construct a new Alignment Frame object
         *
         * @param parent pointer to the parent frame (main frame)
         * @param aligned_images_producer pointer to the aligned images producer object
         */
        AlignedImagesProducerGUI(MyFrame *parent, const PostProcessingTool *post_processing_tool);

        void set_color_stretcher(const CombinedColorStrecherTool &color_stretcher) {
            m_color_stretcher = color_stretcher;
        };

        void set_default_path(const std::string &path) {
            m_default_path = path;
        };

    private:
        MyFrame *m_parent = nullptr;
        const PostProcessingTool *m_post_processing_tool = nullptr;
        wxBoxSizer *m_main_vertical_sizer = nullptr;
        wxBoxSizer *m_basic_settings_sizer = nullptr;
        wxBoxSizer *m_image_preview_sizer = nullptr;

        std::unique_ptr<FloatingPointSlider> m_fraction_to_stack_slider     = nullptr;
        std::unique_ptr<FloatingPointSlider> m_exposure_correction_slider   = nullptr;
        std::string m_default_path = "";

        std::unique_ptr<AstroPhotoStacker::AlignedImagesProducer> m_aligned_images_producer = nullptr;
        std::unique_ptr<ImagePreviewCropTool> m_image_preview_crop_tool = nullptr;
        CombinedColorStrecherTool m_exposure_stretcher; // for exposure correction
        CombinedColorStrecherTool m_color_stretcher;

        bool m_save_also_tif_files  = false;
        bool m_group_by_files       = false;

        std::string m_output_folder_address;

        void initialize_aligned_images_producer();

        AstroPhotoStacker::InputFrame get_reference_frame() const;

        void add_exposure_correction_spin_ctrl();

        void add_checkboxes();

        void add_advanced_settings();

        void add_video_settings();

        void add_group_to_stack(const std::vector<size_t> &group,
                                const std::vector<FrameInfo> &light_frames,
                                const std::map<int, std::vector<std::shared_ptr<const AstroPhotoStacker::CalibrationFrameBase> > > &calibration_handlers_map)   const;

        std::map<int, std::vector<std::shared_ptr<const AstroPhotoStacker::CalibrationFrameBase> > > get_calibration_frame_handlers_map() const;

        bool m_add_datetime = false;
        bool m_apply_color_stretcher = false;

        bool m_use_grouping = false;
        int  m_grouping_time_interval = 0;
        int  m_timestamp_offset = 0;

        bool  m_stack_images = false;
        float m_fraction_to_stack = 0.5;

        AstroPhotoStacker::TimeLapseVideoSettings m_timelapse_video_settings;
};
