#pragma once

#include "../headers/ImagePreview.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <memory>
#include <vector>
#include <string>

class ImageViewerApp : public wxApp  {
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

class ImageViewerFrame : public wxFrame  {
    public:
        ImageViewerFrame();

    private:
        void add_menu_bar();

        void on_open_file(wxCommandEvent& event);

        std::unique_ptr<ImagePreview> m_image_preview = nullptr;

        wxBoxSizer *m_sizer_main_frame = nullptr;

        std::string m_file_address = "";
        std::string m_current_folder = "";


    inline int unique_counter()    {
        static int counter = 1000;
        return ++counter;
    };

    void on_exit(wxCommandEvent& event);

};