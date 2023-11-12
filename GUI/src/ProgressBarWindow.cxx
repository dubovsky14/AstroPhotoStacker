#include "../headers/ProgressBarWindow.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <atomic>
#include <string>
#include <iostream>

using namespace std;

ProgressBarWindow::ProgressBarWindow(const std::atomic<int> &n_processed, int n_total, const std::string &window_title, const std::string &label_text, int window_width, int window_height)
    :  wxFrame(nullptr, wxID_ANY, window_title)      {

        SetSize(window_width, window_height);
        cout << "window_width: " << window_width << endl;
        cout << "window_height: " << window_height << endl;

        m_n_processed = &n_processed;
        m_n_total = n_total;
        m_label_text = label_text;



        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        wxStaticText* select_file_text = new wxStaticText(this, wxID_ANY, label_text);


        m_gauge = new wxGauge(this, wxID_ANY, n_total, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL);
        m_gauge->SetValue(n_processed);
        m_gauge->SetRange(n_total);
        m_gauge->SetSize(400, 50);

        sizer->Add(select_file_text, 1, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
        sizer->Add(m_gauge, 1, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

        SetSizer(sizer);

        cout << "Progress bar window constructed" << endl;
};


void ProgressBarWindow::update_gauge()  {
    m_gauge->SetValue(int(*m_n_processed));
    std::cout << "n_processed: " << *m_n_processed << std::endl;
};

