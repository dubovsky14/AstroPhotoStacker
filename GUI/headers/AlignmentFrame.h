#pragma once

#include "../headers/FilelistHandler.h"
#include "../headers/MainFrame.h"

#include "../../headers/StackSettings.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <vector>
#include <string>


/**
 * @brief Frame (dialog window) for selecting the reference photo for alignment

*/
class AlignmentFrame : public wxFrame  {
    public:
        /**
         * @brief Construct a new Alignment Frame object
         *
         * @param parent pointer to the parent frame (main frame)
         * @param filelist_handler pointer to the filelist handler object
         * @param stack_settings pointer to the stack settings object
         */
        AlignmentFrame(MyFrame *parent, FilelistHandler *filelist_handler, AstroPhotoStacker::StackSettings *stack_settings);

    private:
        AstroPhotoStacker::StackSettings *m_stack_settings = nullptr;
        FilelistHandler *m_filelist_handler = nullptr;
};
