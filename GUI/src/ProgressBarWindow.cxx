#include "../headers/ProgressBarWindow.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <atomic>
#include <string>
#include <iostream>

using namespace std;

ProgressBarWindow::ProgressBarWindow(wxFrame *parent, const std::atomic<int> &n_processed, int n_total, const std::string &window_title, const std::string &label_text, int window_width, int window_height)
    :  wxFrame(parent, wxID_ANY, window_title)      {

        SetSize(window_width, window_height);
        cout << "window_width: " << window_width << endl;
        cout << "window_height: " << window_height << endl;

        m_n_processed = &n_processed;
        m_n_total = n_total;
        m_label_text = label_text;



        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        wxStaticText* select_file_text = new wxStaticText(this, wxID_ANY, label_text);


        m_progress_bar = new wxProgressDialog(label_text, "Aligning files...", n_total, this, wxPD_AUTO_HIDE | wxPD_APP_MODAL);
        m_progress_bar->Update(n_processed);
        m_progress_bar->SetRange(n_total);
        m_progress_bar->SetSize(400, 50);
        m_progress_bar->SetSizeHints(400, 50);
        m_progress_bar->Show(true);

        sizer->Add(select_file_text, 1, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
        sizer->Add(m_progress_bar, 1, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

        SetSizer(sizer);

        cout << "Progress bar window constructed" << endl;
};


void ProgressBarWindow::update_gauge()  {
    m_progress_bar->Update(int(*m_n_processed));
    std::cout << "n_processed: " << *m_n_processed << std::endl;
};

