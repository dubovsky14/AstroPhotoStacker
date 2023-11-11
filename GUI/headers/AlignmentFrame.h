// Start of wxWidgets "Hello World" Program
#pragma once

#include "../headers/FilelistHandler.h"
#include "../headers/StackSettings.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <vector>
#include <string>

class AlignmentFrame : public wxFrame  {
    public:
        AlignmentFrame(const std::vector<std::string> &available_light_frames, StackSettings *stack_settings);

    private:
        std::vector<wxString> m_available_files;
};
