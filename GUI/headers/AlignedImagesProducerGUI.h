#pragma once

#include "../headers/FilelistHandler.h"
#include "../headers/StackSettings.h"
#include "../headers/ImagePreviewCropTool.h"
#include "../headers/CombinedColorStrecherTool.h"

#include "../../headers/AlignedImagesProducer.h"

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
        AlignedImagesProducerGUI(MyFrame *parent);

    private:
        MyFrame *m_parent = nullptr;
        wxBoxSizer *m_main_vertical_sizer = nullptr;


        std::unique_ptr<AstroPhotoStacker::AlignedImagesProducer> m_aligned_images_producer = nullptr;
        std::unique_ptr<ImagePreviewCropTool> m_image_preview_crop_tool = nullptr;
        CombinedColorStrecherTool m_color_stretcher; // for exposure correction

        std::string m_output_folder_address;

        void initialize_aligned_images_producer();

        std::string get_reference_file_address() const;

        void add_exposure_correction_spin_ctrl();

        void add_checkboxes();
        bool m_add_datetime = false;
        bool m_apply_color_stretcher = false;
};
