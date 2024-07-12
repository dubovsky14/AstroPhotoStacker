#pragma once

#include "../headers/FilelistHandler.h"
#include "../headers/StackSettings.h"

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
        std::unique_ptr<AstroPhotoStacker::AlignedImagesProducer> m_aligned_images_producer = nullptr;
};
