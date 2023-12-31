#pragma once

#include "../headers/FilelistHandler.h"
#include "../headers/StackSettings.h"

#include "../headers/MainFrame.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <vector>
#include <string>

class AlignmentFrame : public wxFrame  {
    public:
        AlignmentFrame(MyFrame *parent, FilelistHandler *filelist_handler, StackSettings *stack_settings);

    private:
        StackSettings *m_stack_settings = nullptr;
        FilelistHandler *m_filelist_handler = nullptr;
};
