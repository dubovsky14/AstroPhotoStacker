// Start of wxWidgets "Hello World" Program
#pragma once

#include "../headers/FilelistHandler.h"

#include <wx/wx.h>

#include <memory>

class MyApp : public wxApp  {
    public:
        bool OnInit() override;
};

class MyFrame : public wxFrame  {
    public:
        MyFrame();

    private:
        void add_file_menu();

        void add_menu_bar();

        wxMenuBar   *m_menu_bar     = nullptr;
        wxMenu      *m_file_menu    = nullptr;





        FilelistHandler m_filelist_handler;

        void on_open_lights(wxCommandEvent& event);
        void on_open_flats (wxCommandEvent& event);
        void on_exit(wxCommandEvent& event);


};

inline int unique_counter()    {
    static int counter = 1000;
    return ++counter;
};

enum    {
    ID_Hello = 1
};
