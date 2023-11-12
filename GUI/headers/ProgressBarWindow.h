#pragma once


#include <wx/wx.h>
#include <wx/progdlg.h>

#include <atomic>
#include <string>

class ProgressBarWindow : public wxFrame  {
    public:
        ProgressBarWindow(wxFrame *parent, const std::atomic<int> &n_processed, int n_total, const std::string &window_title, const std::string &label_text, int window_width = 400, int window_height = 200);

        void update_gauge();
    private:

        wxProgressDialog *m_progress_bar = nullptr;
        const std::atomic<int> *m_n_processed;
        int m_n_total;
        std::string m_label_text;
};
